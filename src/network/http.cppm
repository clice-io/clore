module;

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "kota/async/async.h"
#include "kota/http/http.h"

export module http;

import std;
import support;

export namespace clore::net {

namespace async = kota;

void initialize_llm_rate_limit(std::uint32_t rate_limit);

void shutdown_llm_rate_limit() noexcept;

struct LLMError {
    std::string message;

    LLMError() = default;

    explicit LLMError(std::string msg) : message(std::move(msg)) {}

    explicit LLMError(kota::error err) : message(err.message()) {}
};

}  // namespace clore::net

export namespace clore::net::detail {

struct EnvironmentConfig {
    std::string api_base;
    std::string api_key;
};

struct RawHttpResponse {
    long http_status = 0;
    std::string body;
};

extern std::unique_ptr<kota::semaphore> g_llm_semaphore;

auto read_environment(std::string_view base_env, std::string_view key_env)
    -> std::expected<EnvironmentConfig, LLMError>;

auto perform_http_request(const std::string& url,
                          std::span<const kota::http::header> headers,
                          std::string_view request_json)
    -> std::expected<RawHttpResponse, LLMError>;

auto perform_http_request_async(std::string url,
                                std::vector<kota::http::header> headers,
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

constexpr long kHttpConnectTimeoutMs = 5'000;
constexpr auto kHttpRequestTimeout = std::chrono::milliseconds(120'000);

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

auto configure_request(kota::http::request& request,
                       std::span<const kota::http::header> headers,
                       std::string request_json) -> void {
    for(const auto& header: headers) {
        request.header(header.name, header.value);
    }

    request.body(std::move(request_json));
    request.curl_option(CURLOPT_CONNECTTIMEOUT_MS, kHttpConnectTimeoutMs);
    request.curl_option(CURLOPT_NOSIGNAL, 1L);
    request.curl_option(CURLOPT_TCP_KEEPALIVE, 1L);
}

auto perform_http_request(const std::string& url,
                          std::span<const kota::http::header> headers,
                          std::string_view request_json)
    -> std::expected<RawHttpResponse, LLMError> {
    async::event_loop loop;
    auto operation =
        perform_http_request_async(std::string(url),
                                   std::vector<kota::http::header>(headers.begin(), headers.end()),
                                   std::string(request_json),
                                   loop)
            .catch_cancel();

    loop.schedule(operation);
    loop.run();

    auto result = operation.result();
    if(result.is_cancelled()) {
        return std::unexpected(LLMError(std::format("request cancelled: {}", url)));
    }
    if(result.has_error()) {
        return std::unexpected(std::move(result).error());
    }

    return std::move(*result);
}

auto perform_http_request_async(std::string url,
                                std::vector<kota::http::header> headers,
                                std::string request_json,
                                async::event_loop& loop) -> async::task<RawHttpResponse, LLMError> {
    if(g_llm_semaphore) {
        co_await g_llm_semaphore->acquire();
    }

    struct SemaphoreGuard {
        ~SemaphoreGuard() {
            if(g_llm_semaphore) {
                g_llm_semaphore->release();
            }
        }
    } sem_guard;

    auto request_number = g_llm_request_counter.fetch_add(1, std::memory_order_relaxed) + 1;
    auto request_url = url;
    logging::info("calling LLM #{}: {}", request_number, request_url);

    kota::http::client client;
    client.timeout(kHttpRequestTimeout);

    auto request = client.on(loop).post(std::move(url));
    configure_request(request, headers, std::move(request_json));

    auto response_result = co_await request.send().catch_cancel();
    if(response_result.is_cancelled()) {
        co_await async::fail(
            LLMError(std::format("LLM request #{} cancelled: {}", request_number, request_url)));
    }
    if(response_result.has_error()) {
        auto error = LLMError(std::format("request failed: {}", response_result.error().message()));
        logging::warn("LLM request #{} failed: {} ({})",
                      request_number,
                      request_url,
                      error.message);
        co_await async::fail(std::move(error));
    }

    auto response = std::move(*response_result);
    auto raw_response = RawHttpResponse{
        .http_status = response.status,
        .body = response.text_copy(),
    };

    logging::info("completed LLM #{}: {} status={} bytes={}",
                  request_number,
                  request_url,
                  raw_response.http_status,
                  raw_response.body.size());
    co_return raw_response;
}

}  // namespace clore::net::detail

namespace clore::net {

void shutdown_llm_rate_limit() noexcept {
    detail::g_llm_semaphore.reset();
}

}  // namespace clore::net
