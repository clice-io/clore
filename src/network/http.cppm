module;

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <cstdint>
#include <cstdlib>
#include <expected>
#include <format>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include <curl/curl.h>
#include <uv.h>

#include "eventide/async/async.h"

export module network:http;

import :async;
import support;

export namespace clore::net::detail {

struct EnvironmentConfig {
    std::string api_base;
    std::string api_key;
};

struct RawHttpResponse {
    long http_status = 0;
    std::string body;
};

auto read_environment() -> std::expected<EnvironmentConfig, LLMError>;

auto build_chat_completions_url(std::string_view api_base) -> std::string;

auto perform_http_request(const std::string& url,
                          std::string_view api_key,
                          std::string_view request_json)
    -> std::expected<RawHttpResponse, LLMError>;

auto perform_http_request_async(std::string url,
                                std::string api_key,
                                std::string request_json,
                                eventide::event_loop& loop)
    -> eventide::task<RawHttpResponse, LLMError>;

}  // namespace clore::net::detail

namespace clore::net::detail {

namespace async = eventide;

constexpr std::string_view kOpenAIBaseUrlEnv = "OPENAI_BASE_URL";
constexpr std::string_view kOpenAIApiKeyEnv = "OPENAI_API_KEY";
constexpr long kCurlConnectTimeoutMs = 5'000;
constexpr long kCurlRequestTimeoutMs = 120'000;

auto read_required_env(std::string_view name)
    -> std::expected<std::string, LLMError> {
    auto* value = std::getenv(std::string(name).c_str());
    if(value == nullptr || value[0] == '\0') {
        return std::unexpected(
            LLMError(std::format("required environment variable {} is not set", name)));
    }
    return std::string(value);
}

auto read_environment() -> std::expected<EnvironmentConfig, LLMError> {
    auto api_base = read_required_env(kOpenAIBaseUrlEnv);
    if(!api_base.has_value()) {
        return std::unexpected(std::move(api_base.error()));
    }

    auto api_key = read_required_env(kOpenAIApiKeyEnv);
    if(!api_key.has_value()) {
        return std::unexpected(std::move(api_key.error()));
    }

    return EnvironmentConfig{
        .api_base = std::move(*api_base),
        .api_key = std::move(*api_key),
    };
}

auto build_chat_completions_url(std::string_view api_base) -> std::string {
    std::string url(api_base);
    if(!url.empty() && url.back() != '/') {
        url += '/';
    }
    url += "chat/completions";
    return url;
}

auto ensure_curl_global_init() -> std::expected<void, LLMError> {
    static std::once_flag once;
    static std::optional<LLMError> init_error;

    std::call_once(once, [] {
        auto code = curl_global_init(CURL_GLOBAL_DEFAULT);
        if(code != CURLE_OK) {
            init_error = LLMError(
                std::format("curl_global_init failed: {}", curl_easy_strerror(code)));
            return;
        }

        if(std::atexit(&curl_global_cleanup) != 0) {
            curl_global_cleanup();
            init_error = LLMError("std::atexit(curl_global_cleanup) failed");
        }
    });

    if(init_error.has_value()) {
        return std::unexpected(*init_error);
    }

    return {};
}

auto write_response_body(char* ptr, std::size_t size,
                         std::size_t nmemb, void* userdata) -> std::size_t {
    auto* response = static_cast<std::string*>(userdata);
    auto bytes = size * nmemb;
    response->append(ptr, bytes);
    return bytes;
}

template <typename Value>
auto curl_setopt_checked(CURL* easy, CURLoption option, Value value,
                         std::string_view option_name)
    -> std::expected<void, LLMError> {
    auto code = curl_easy_setopt(easy, option, value);
    if(code != CURLE_OK) {
        return std::unexpected(LLMError(std::format(
            "curl_easy_setopt({}) failed: {}", option_name, curl_easy_strerror(code))));
    }
    return {};
}

template <typename Value>
auto curl_multi_setopt_checked(CURLM* multi, CURLMoption option, Value value,
                               std::string_view option_name)
    -> std::expected<void, LLMError> {
    auto code = curl_multi_setopt(multi, option, value);
    if(code != CURLM_OK) {
        return std::unexpected(LLMError(std::format(
            "curl_multi_setopt({}) failed: {}", option_name, curl_multi_strerror(code))));
    }
    return {};
}

auto append_header(curl_slist*& headers, const char* value)
    -> std::expected<void, LLMError> {
    auto* next = curl_slist_append(headers, value);
    if(next == nullptr) {
        return std::unexpected(
            LLMError(std::format("curl_slist_append failed for header '{}'", value)));
    }
    headers = next;
    return {};
}

struct CurlEasyCleanup {
    void operator()(CURL* easy) const noexcept {
        if(easy != nullptr) {
            curl_easy_cleanup(easy);
        }
    }
};

struct CurlHeaderCleanup {
    void operator()(curl_slist* headers) const noexcept {
        if(headers != nullptr) {
            curl_slist_free_all(headers);
        }
    }
};

auto perform_http_request(const std::string& url,
                          std::string_view api_key,
                          std::string_view request_json)
    -> std::expected<RawHttpResponse, LLMError> {
    auto init = ensure_curl_global_init();
    if(!init.has_value()) {
        return std::unexpected(std::move(init.error()));
    }

    std::unique_ptr<CURL, CurlEasyCleanup> easy(curl_easy_init());
    if(easy == nullptr) {
        return std::unexpected(LLMError("curl_easy_init failed"));
    }

    curl_slist* raw_headers = nullptr;
    auto content_type = append_header(raw_headers,
        "Content-Type: application/json; charset=utf-8");
    if(!content_type.has_value()) {
        return std::unexpected(std::move(content_type.error()));
    }

    auto authorization = std::format("Authorization: Bearer {}", api_key);
    auto auth_status = append_header(raw_headers, authorization.c_str());
    if(!auth_status.has_value()) {
        return std::unexpected(std::move(auth_status.error()));
    }

    std::unique_ptr<curl_slist, CurlHeaderCleanup> headers(raw_headers);
    std::string response_body;
    long http_status = 0;
    std::string post_data(request_json);

    // libcurl requires a stable NUL-terminated URL string here.
    if(auto status = curl_setopt_checked(easy.get(), CURLOPT_URL, url.c_str(), "CURLOPT_URL");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    if(auto status = curl_setopt_checked(easy.get(), CURLOPT_POST, 1L, "CURLOPT_POST");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    if(auto status = curl_setopt_checked(easy.get(), CURLOPT_HTTPHEADER, headers.get(),
                                         "CURLOPT_HTTPHEADER");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    if(auto status = curl_setopt_checked(easy.get(), CURLOPT_POSTFIELDS,
                                         post_data.c_str(),
                                         "CURLOPT_POSTFIELDS");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    if(auto status = curl_setopt_checked(
            easy.get(), CURLOPT_POSTFIELDSIZE_LARGE,
            static_cast<curl_off_t>(post_data.size()),
            "CURLOPT_POSTFIELDSIZE_LARGE");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    if(auto status = curl_setopt_checked(easy.get(), CURLOPT_WRITEFUNCTION,
                                         &write_response_body,
                                         "CURLOPT_WRITEFUNCTION");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    if(auto status = curl_setopt_checked(easy.get(), CURLOPT_WRITEDATA,
                                         &response_body,
                                         "CURLOPT_WRITEDATA");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    if(auto status = curl_setopt_checked(easy.get(), CURLOPT_CONNECTTIMEOUT_MS,
                                         kCurlConnectTimeoutMs,
                                         "CURLOPT_CONNECTTIMEOUT_MS");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    if(auto status = curl_setopt_checked(easy.get(), CURLOPT_TIMEOUT_MS,
                                         kCurlRequestTimeoutMs,
                                         "CURLOPT_TIMEOUT_MS");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    if(auto status = curl_setopt_checked(easy.get(), CURLOPT_NOSIGNAL, 1L,
                                         "CURLOPT_NOSIGNAL");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }

    auto perform_status = curl_easy_perform(easy.get());
    if(perform_status != CURLE_OK) {
        return std::unexpected(LLMError(std::format(
            "curl request failed: {}",
            curl_easy_strerror(perform_status))));
    }

    auto info_status = curl_easy_getinfo(easy.get(), CURLINFO_RESPONSE_CODE, &http_status);
    if(info_status != CURLE_OK) {
        return std::unexpected(LLMError(std::format(
            "curl_easy_getinfo(CURLINFO_RESPONSE_CODE) failed: {}",
            curl_easy_strerror(info_status))));
    }

    return RawHttpResponse{
        .http_status = http_status,
        .body = std::move(response_body),
    };
}

auto perform_http_request_async(std::string url,
                                std::string api_key,
                                std::string request_json,
                                async::event_loop& loop)
    -> async::task<RawHttpResponse, LLMError> {
    struct ThreadedRequestState {
        async::event_loop& loop;
        async::event completion;
        uv_work_t work{};
        std::string url;
        std::string api_key;
        std::string request_json;
        std::optional<LLMError> error;
        std::optional<RawHttpResponse> response;

        ThreadedRequestState(std::string request_url,
                             std::string request_api_key,
                             std::string request_body,
                             async::event_loop& event_loop)
            : loop(event_loop),
              url(std::move(request_url)),
              api_key(std::move(request_api_key)),
              request_json(std::move(request_body)) {
            work.data = this;
        }

        static void on_uv_work(uv_work_t* request) {
            auto* current = static_cast<ThreadedRequestState*>(request->data);
            if(current == nullptr) {
                return;
            }

            auto result = perform_http_request(
                current->url, current->api_key, current->request_json);
            if(result.has_value()) {
                logging::info("completed LLM request: {} status={} bytes={}",
                              current->url, result->http_status, result->body.size());
                current->response = std::move(*result);
                return;
            }

            logging::warn("LLM request failed: {} ({})", current->url, result.error().message);
            current->error = std::move(result.error());
        }

        static void on_uv_after_work(uv_work_t* request, int status) {
            auto* current = static_cast<ThreadedRequestState*>(request->data);
            if(current == nullptr) {
                return;
            }

            if(status < 0 && !current->error.has_value() && !current->response.has_value()) {
                current->error = LLMError(std::format(
                    "uv_queue_work completion failed: {}", uv_strerror(status)));
            }

            current->completion.set();
        }

        auto start() -> std::expected<void, LLMError> {
            auto queue_status = uv_queue_work(&static_cast<uv_loop_t&>(loop),
                                              &work,
                                              &on_uv_work,
                                              &on_uv_after_work);
            if(queue_status < 0) {
                return std::unexpected(LLMError(std::format(
                    "uv_queue_work failed: {}", uv_strerror(queue_status))));
            }

            logging::info("calling LLM: {}", url);

            return {};
        }
    };

    auto state = std::make_unique<ThreadedRequestState>(
        std::move(url), std::move(api_key), std::move(request_json), loop);
    auto started = state->start();
    if(!started.has_value()) {
        co_await async::fail(std::move(started.error()));
    }

    co_await state->completion.wait();

    if(state->error.has_value()) {
        co_await async::fail(std::move(*state->error));
    }
    if(!state->response.has_value()) {
        co_await async::fail(LLMError("LLM request completed without a response payload"));
    }
    co_return std::move(*state->response);
}

}  // namespace clore::net::detail