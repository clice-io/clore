module;

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <curl/curl.h>
#include <openssl/crypto.h>
#include <uv.h>

#include "kota/async/async.h"
#include "kota/codec/json/error.h"

export module http;

import std;
import support;

// ── error type ─────────────────────────────────────────────────────────────

export namespace clore::net {

namespace async = kota;

/// Initialize the global LLM request concurrency limit.
/// Must be called before any LLM requests are issued.
void initialize_llm_rate_limit(std::uint32_t rate_limit);

/// Shutdown the HTTP/OpenSSL runtime used by libcurl-backed LLM requests.
/// This must only be called once no further HTTP requests will be issued.
void shutdown_http_runtime() noexcept;

struct LLMError {
    std::string message;

    LLMError() = default;

    explicit LLMError(std::string msg) : message(std::move(msg)) {}

    explicit LLMError(kota::error err) : message(err.message()) {}
};

}  // namespace clore::net

// ── HTTP transport ─────────────────────────────────────────────────────────

export namespace clore::net::detail {

struct EnvironmentConfig {
    std::string api_base;
    std::string api_key;
};

struct RawHttpResponse {
    long http_status = 0;
    std::string body;
};

using HttpHeaders = std::vector<std::string>;

extern std::unique_ptr<kota::semaphore> g_llm_semaphore;

auto read_environment(std::string_view base_env, std::string_view key_env)
    -> std::expected<EnvironmentConfig, LLMError>;

auto read_environment() -> std::expected<EnvironmentConfig, LLMError>;

auto build_chat_completions_url(std::string_view api_base) -> std::string;

auto perform_http_request(const std::string& url,
                          const HttpHeaders& custom_headers,
                          std::string_view request_json)
    -> std::expected<RawHttpResponse, LLMError>;

auto perform_http_request(const std::string& url,
                          std::string_view api_key,
                          std::string_view request_json)
    -> std::expected<RawHttpResponse, LLMError>;

auto perform_http_request_async(std::string url,
                                HttpHeaders headers,
                                std::string request_json,
                                kota::event_loop& loop) -> kota::task<RawHttpResponse, LLMError>;

auto perform_http_request_async(std::string url,
                                std::string api_key,
                                std::string request_json,
                                kota::event_loop& loop) -> kota::task<RawHttpResponse, LLMError>;

template <typename R>
auto unwrap_caught_result(R result, std::string_view cancel_message)
    -> kota::task<typename R::value_type, LLMError> {
    if(result.is_cancelled()) {
        co_await kota::fail(LLMError(std::string(cancel_message)));
    }
    if(result.has_error()) {
        co_await kota::fail(std::move(result).error());
    }
    co_return std::move(*result);
}

}  // namespace clore::net::detail

namespace clore::net {

void initialize_llm_rate_limit(std::uint32_t rate_limit) {
    if(rate_limit == 0) {
        detail::g_llm_semaphore.reset();
    } else {
        detail::g_llm_semaphore =
            std::make_unique<kota::semaphore>(static_cast<std::ptrdiff_t>(rate_limit));
    }
}

}  // namespace clore::net

namespace clore::net::detail {

namespace async = kota;

std::unique_ptr<kota::semaphore> g_llm_semaphore;
std::atomic<std::uint64_t> g_llm_request_counter = 0;

constexpr std::string_view kOpenAIBaseUrlEnv = "OPENAI_BASE_URL";
constexpr std::string_view kOpenAIApiKeyEnv = "OPENAI_API_KEY";
constexpr long kCurlConnectTimeoutMs = 5'000;
constexpr long kCurlRequestTimeoutMs = 120'000;

auto read_required_env(std::string_view name) -> std::expected<std::string, LLMError> {
    auto* value = std::getenv(std::string(name).c_str());
    if(value == nullptr || value[0] == '\0') {
        return std::unexpected(
            LLMError(std::format("required environment variable {} is not set", name)));
    }
    return std::string(value);
}

auto read_environment(std::string_view base_env, std::string_view key_env)
    -> std::expected<EnvironmentConfig, LLMError> {
    auto api_base = read_required_env(base_env);
    if(!api_base.has_value()) {
        return std::unexpected(std::move(api_base.error()));
    }

    auto api_key = read_required_env(key_env);
    if(!api_key.has_value()) {
        return std::unexpected(std::move(api_key.error()));
    }

    return EnvironmentConfig{
        .api_base = std::move(*api_base),
        .api_key = std::move(*api_key),
    };
}

auto read_environment() -> std::expected<EnvironmentConfig, LLMError> {
    return read_environment(kOpenAIBaseUrlEnv, kOpenAIApiKeyEnv);
}

auto build_chat_completions_url(std::string_view api_base) -> std::string {
    std::string url(api_base);
    if(!url.empty() && url.back() != '/') {
        url += '/';
    }
    url += "chat/completions";
    return url;
}

auto write_response_body(char* ptr, std::size_t size, std::size_t nmemb, void* userdata)
    -> std::size_t {
    auto* response = static_cast<std::string*>(userdata);
    auto bytes = size * nmemb;
    response->append(ptr, bytes);
    return bytes;
}

auto append_header(curl_slist*& headers, const char* value) -> std::expected<void, LLMError> {
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

struct CurlGlobalState {
    std::once_flag once;
    std::optional<LLMError> init_error;
    bool curl_initialized = false;
    bool openssl_initialized = false;
    bool shut_down = false;
};

auto curl_global_state() -> CurlGlobalState& {
    static CurlGlobalState state;
    return state;
}

auto ensure_curl_global_init() -> std::expected<void, LLMError> {
    auto& state = curl_global_state();

    std::call_once(state.once, [&state] {
        if(OPENSSL_init_crypto(OPENSSL_INIT_NO_ATEXIT, nullptr) != 1) {
            state.init_error = LLMError("OPENSSL_init_crypto failed");
            return;
        }
        state.openssl_initialized = true;

        auto code = curl_global_init(CURL_GLOBAL_DEFAULT);
        if(code != CURLE_OK) {
            state.init_error =
                LLMError(std::format("curl_global_init failed: {}", curl_easy_strerror(code)));
            return;
        }
        state.curl_initialized = true;
    });

    if(state.init_error.has_value()) {
        return std::unexpected(*state.init_error);
    }
    if(state.shut_down) {
        return std::unexpected(LLMError("HTTP runtime has already been shut down"));
    }

    return {};
}

auto perform_http_request(const std::string& url,
                          const HttpHeaders& headers,
                          std::string_view request_json)
    -> std::expected<RawHttpResponse, LLMError> {
    auto init = ensure_curl_global_init();
    if(!init.has_value()) {
        return std::unexpected(std::move(init.error()));
    }

    std::unique_ptr<CURL, CurlEasyCleanup> easy_handle(curl_easy_init());
    if(!easy_handle) {
        return std::unexpected(LLMError("curl_easy_init failed"));
    }
    CURL* easy = easy_handle.get();

    curl_slist* raw_headers = nullptr;
    for(const auto& header: headers) {
        auto header_status = append_header(raw_headers, header.c_str());
        if(!header_status.has_value()) {
            return std::unexpected(std::move(header_status.error()));
        }
    }

    std::unique_ptr<curl_slist, CurlHeaderCleanup> request_headers(raw_headers);
    std::string response_body;
    long http_status = 0;
    std::string post_data(request_json);

    (void)curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
    (void)curl_easy_setopt(easy, CURLOPT_POST, 1L);
    (void)curl_easy_setopt(easy, CURLOPT_HTTPHEADER, request_headers.get());
    (void)curl_easy_setopt(easy, CURLOPT_POSTFIELDS, post_data.c_str());
    (void)curl_easy_setopt(easy,
                           CURLOPT_POSTFIELDSIZE_LARGE,
                           static_cast<curl_off_t>(post_data.size()));
    (void)curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, &write_response_body);
    (void)curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response_body);
    (void)curl_easy_setopt(easy, CURLOPT_CONNECTTIMEOUT_MS, kCurlConnectTimeoutMs);
    (void)curl_easy_setopt(easy, CURLOPT_TIMEOUT_MS, kCurlRequestTimeoutMs);
    (void)curl_easy_setopt(easy, CURLOPT_NOSIGNAL, 1L);
    (void)curl_easy_setopt(easy, CURLOPT_TCP_KEEPALIVE, 1L);

    auto perform_status = curl_easy_perform(easy);
    if(perform_status != CURLE_OK) {
        return std::unexpected(
            LLMError(std::format("curl request failed: {}", curl_easy_strerror(perform_status))));
    }

    auto info_status = curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_status);
    if(info_status != CURLE_OK) {
        return std::unexpected(
            LLMError(std::format("curl_easy_getinfo(CURLINFO_RESPONSE_CODE) failed: {}",
                                 curl_easy_strerror(info_status))));
    }

    return RawHttpResponse{
        .http_status = http_status,
        .body = std::move(response_body),
    };
}

auto perform_http_request(const std::string& url,
                          std::string_view api_key,
                          std::string_view request_json)
    -> std::expected<RawHttpResponse, LLMError> {
    HttpHeaders headers{
        "Content-Type: application/json; charset=utf-8",
        std::format("Authorization: Bearer {}", api_key),
    };
    return perform_http_request(url, headers, request_json);
}

auto perform_http_request_async(std::string url,
                                HttpHeaders headers,
                                std::string request_json,
                                async::event_loop& loop) -> async::task<RawHttpResponse, LLMError> {
    if(detail::g_llm_semaphore) {
        co_await detail::g_llm_semaphore->acquire();
    }

    struct SemaphoreGuard {
        ~SemaphoreGuard() {
            if(detail::g_llm_semaphore) {
                detail::g_llm_semaphore->release();
            }
        }
    } sem_guard;

    auto request_number = g_llm_request_counter.fetch_add(1, std::memory_order_relaxed) + 1;
    logging::info("calling LLM #{}: {}", request_number, url);

    auto queued_result =
        co_await async::queue(
            [request_url = std::move(url),
             request_headers = std::move(headers),
             request_body = std::move(request_json)]() mutable {
                return perform_http_request(request_url, request_headers, request_body);
            },
            loop)
            .catch_cancel();

    if(queued_result.is_cancelled()) {
        co_await async::fail(
            LLMError(std::format("LLM request #{} cancelled: {}", request_number, url)));
    }
    if(queued_result.has_error()) {
        co_await async::fail(LLMError(std::move(queued_result.error())));
    }

    auto result = std::move(*queued_result);
    if(!result.has_value()) {
        logging::warn("LLM request #{} failed: {} ({})",
                      request_number,
                      url,
                      result.error().message);
        co_await async::fail(std::move(result.error()));
    }

    logging::info("completed LLM #{}: {} status={} bytes={}",
                  request_number,
                  url,
                  result->http_status,
                  result->body.size());
    co_return std::move(*result);
}

auto perform_http_request_async(std::string url,
                                std::string api_key,
                                std::string request_json,
                                async::event_loop& loop) -> async::task<RawHttpResponse, LLMError> {
    HttpHeaders headers{
        "Content-Type: application/json; charset=utf-8",
        std::format("Authorization: Bearer {}", api_key),
    };
    co_return co_await perform_http_request_async(std::move(url),
                                                  std::move(headers),
                                                  std::move(request_json),
                                                  loop)
        .or_fail();
}

}  // namespace clore::net::detail

namespace clore::net {

void shutdown_http_runtime() noexcept {
    auto& state = detail::curl_global_state();
    if(state.shut_down) {
        return;
    }

    state.shut_down = true;
    detail::g_llm_semaphore.reset();

    if(state.curl_initialized) {
        curl_global_cleanup();
        state.curl_initialized = false;
    }
    if(state.openssl_initialized) {
        OPENSSL_cleanup();
        state.openssl_initialized = false;
    }
}

}  // namespace clore::net
