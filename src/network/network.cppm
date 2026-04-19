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

auto call_llm_async(std::string_view provider,
                    std::string_view model,
                    std::string_view system_prompt,
                    PromptRequest request,
                    std::uint32_t retry_count,
                    std::uint32_t retry_initial_backoff_ms,
                    kota::event_loop& loop = kota::event_loop::current())
    -> kota::task<std::string, LLMError>;

auto call_completion_async(std::string_view provider,
                           CompletionRequest request,
                           kota::event_loop& loop = kota::event_loop::current())
    -> kota::task<CompletionResponse, LLMError>;

}  // namespace clore::net

namespace clore::net {

namespace {

auto unsupported_provider_error(std::string_view provider) -> LLMError {
    return LLMError(std::format("unsupported llm provider '{}'", provider));
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

auto dispatch_completion(std::string_view provider,
                         CompletionRequest request,
                         kota::event_loop& loop)
    -> std::optional<kota::task<CompletionResponse, LLMError>> {
    if(provider == "openai") {
        return clore::net::call_completion_async(std::move(request), loop);
    }
    if(provider == "anthropic") {
        return clore::net::anthropic::call_completion_async(std::move(request), loop);
    }
    return std::nullopt;
}

auto provider_label(std::string_view provider) -> std::string_view {
    if(provider == "openai")
        return "OpenAI";
    if(provider == "anthropic")
        return "Anthropic";
    return provider;
}

}  // namespace

auto call_llm_async(std::string_view provider,
                    std::string_view model,
                    std::string_view system_prompt,
                    PromptRequest request,
                    std::uint32_t retry_count,
                    std::uint32_t retry_initial_backoff_ms,
                    kota::event_loop& loop) -> kota::task<std::string, LLMError> {
    // Validate provider early before entering retry machinery.
    if(!dispatch_completion(provider, CompletionRequest{}, loop).has_value()) {
        co_await kota::fail(unsupported_provider_error(provider));
    }
    auto label = provider_label(provider);
    co_return co_await request_provider_text_async(
        label,
        [provider](CompletionRequest req,
                   kota::event_loop& req_loop) -> kota::task<CompletionResponse, LLMError> {
            auto task = dispatch_completion(provider, std::move(req), req_loop);
            if(!task.has_value()) {
                co_await kota::fail(unsupported_provider_error(provider));
            }
            co_return co_await std::move(*task).or_fail();
        },
        model,
        system_prompt,
        std::move(request),
        retry_count,
        retry_initial_backoff_ms,
        loop)
        .or_fail();
}

auto call_completion_async(std::string_view provider,
                           CompletionRequest request,
                           kota::event_loop& loop) -> kota::task<CompletionResponse, LLMError> {
    auto task = dispatch_completion(provider, std::move(request), loop);
    if(!task.has_value()) {
        co_await kota::fail(unsupported_provider_error(provider));
    }
    co_return co_await detail::unwrap_caught_result(co_await std::move(*task).catch_cancel(),
                                                    "LLM completion request cancelled");
}

}  // namespace clore::net
