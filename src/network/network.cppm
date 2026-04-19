module;

#include "kota/async/async.h"

export module network;

import std;
export import anthropic;
export import client;
export import http;
export import openai;
export import protocol;
export import schema;

export namespace clore::net {

auto call_llm_async(std::string_view model,
                    std::string_view system_prompt,
                    PromptRequest request,
                    std::uint32_t retry_count,
                    std::uint32_t retry_initial_backoff_ms,
                    kota::event_loop& loop = kota::event_loop::current())
    -> kota::task<std::string, LLMError>;

auto call_completion_async(CompletionRequest request,
                           kota::event_loop& loop = kota::event_loop::current())
    -> kota::task<CompletionResponse, LLMError>;

}  // namespace clore::net

namespace clore::net {

namespace {

enum class Provider {
    Anthropic,
    OpenAI,
};

constexpr std::string_view kAnthropicBaseUrlEnv = "ANTHROPIC_BASE_URL";
constexpr std::string_view kAnthropicApiKeyEnv = "ANTHROPIC_API_KEY";
constexpr std::string_view kOpenAIBaseUrlEnv = "OPENAI_BASE_URL";
constexpr std::string_view kOpenAIApiKeyEnv = "OPENAI_API_KEY";

auto has_nonempty_env(std::string_view name) -> bool {
    auto key = std::string(name);
    auto* value = std::getenv(key.c_str());
    return value != nullptr && value[0] != '\0';
}

auto has_provider_env(std::string_view base_env, std::string_view key_env) -> bool {
    return has_nonempty_env(base_env) && has_nonempty_env(key_env);
}

template <typename CompletionRequester>
auto request_provider_text_async(std::string_view provider_label,
                                 CompletionRequester request_completion,
                                 std::string_view model,
                                 std::string_view system_prompt,
                                 PromptRequest request,
                                 std::uint32_t retry_count,
                                 std::uint32_t retry_initial_backoff_ms,
                                 kota::event_loop& loop) -> kota::task<std::string, LLMError> {
    auto result = co_await detail::request_text_with_retries(
                      std::string(model),
                      std::string(system_prompt),
                      std::move(request),
                      retry_count,
                      retry_initial_backoff_ms,
                      loop,
                      [request_completion](std::string_view current_model,
                                           std::string_view current_system_prompt,
                                           PromptRequest current_request,
                                           kota::event_loop& current_loop) {
                          return detail::request_text_once_async(request_completion,
                                                                 current_model,
                                                                 current_system_prompt,
                                                                 std::move(current_request),
                                                                 current_loop);
                      },
                      provider_label)
                      .catch_cancel();

    if(result.is_cancelled()) {
        co_await kota::fail(LLMError("LLM request cancelled"));
    }
    if(result.has_error()) {
        co_await kota::fail(std::move(result).error());
    }

    co_return std::move(*result);
}

}  // namespace

namespace {

auto detect_provider_from_environment() -> std::expected<Provider, LLMError> {
    if(has_provider_env(kAnthropicBaseUrlEnv, kAnthropicApiKeyEnv)) {
        return Provider::Anthropic;
    }
    if(has_provider_env(kOpenAIBaseUrlEnv, kOpenAIApiKeyEnv)) {
        return Provider::OpenAI;
    }

    return std::unexpected(LLMError(
        "no supported llm provider environment found; set ANTHROPIC_BASE_URL and " "ANTHROPIC_API_KEY, or OPENAI_BASE_URL and OPENAI_API_KEY"));
}

auto dispatch_completion(Provider provider, CompletionRequest request, kota::event_loop& loop)
    -> kota::task<CompletionResponse, LLMError> {
    switch(provider) {
        case Provider::OpenAI:
            co_return co_await clore::net::openai::call_completion_async(std::move(request), loop)
                .or_fail();
        case Provider::Anthropic:
            co_return co_await clore::net::anthropic::call_completion_async(std::move(request),
                                                                            loop)
                .or_fail();
    }
}

auto provider_label(Provider provider) -> std::string_view {
    switch(provider) {
        case Provider::OpenAI: return "OpenAI";
        case Provider::Anthropic: return "Anthropic";
    }
}

}  // namespace

auto call_llm_async(std::string_view model,
                    std::string_view system_prompt,
                    PromptRequest request,
                    std::uint32_t retry_count,
                    std::uint32_t retry_initial_backoff_ms,
                    kota::event_loop& loop) -> kota::task<std::string, LLMError> {
    auto provider_result = detect_provider_from_environment();
    if(!provider_result.has_value()) {
        co_await kota::fail(std::move(provider_result.error()));
    }
    auto provider = std::move(*provider_result);

    auto label = provider_label(provider);
    co_return co_await request_provider_text_async(
        label,
        [provider](CompletionRequest req,
                   kota::event_loop& req_loop) -> kota::task<CompletionResponse, LLMError> {
            co_return co_await dispatch_completion(provider, std::move(req), req_loop).or_fail();
        },
        model,
        system_prompt,
        std::move(request),
        retry_count,
        retry_initial_backoff_ms,
        loop)
        .or_fail();
}

auto call_completion_async(CompletionRequest request, kota::event_loop& loop)
    -> kota::task<CompletionResponse, LLMError> {
    auto provider_result = detect_provider_from_environment();
    if(!provider_result.has_value()) {
        co_await kota::fail(std::move(provider_result.error()));
    }
    auto provider = std::move(*provider_result);

    co_return co_await detail::unwrap_caught_result(
        co_await dispatch_completion(provider, std::move(request), loop).catch_cancel(),
        "LLM completion request cancelled");
}

}  // namespace clore::net
