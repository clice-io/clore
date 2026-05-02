module;

#include "kota/async/async.h"

export module client;

import std;
import http;
import protocol;
import schema;
import support;

export namespace clore::net {

template <typename Protocol>
auto call_completion_async(clore::net::CompletionRequest request, kota::event_loop* loop = nullptr)
    -> kota::task<clore::net::CompletionResponse, clore::net::LLMError>;

template <typename Protocol>
auto call_llm_async(std::string_view model,
                    std::string_view system_prompt,
                    clore::net::PromptRequest request,
                    kota::event_loop* loop = nullptr)
    -> kota::task<std::string, clore::net::LLMError>;

template <typename Protocol>
auto call_llm_async(std::string_view model,
                    std::string_view system_prompt,
                    std::string_view prompt,
                    kota::event_loop* loop = nullptr)
    -> kota::task<std::string, clore::net::LLMError>;

template <typename Protocol, typename T>
auto call_structured_async(std::string_view model,
                           std::string_view system_prompt,
                           std::string_view prompt,
                           kota::event_loop* loop = nullptr) -> kota::task<T, clore::net::LLMError>;

}  // namespace clore::net

namespace clore::net {

namespace detail {

auto select_event_loop(kota::event_loop* loop) -> kota::event_loop& {
    if(loop != nullptr) {
        return *loop;
    }
    // PRECONDITION: kota::event_loop::current() must return a valid loop.
    // If no event loop is active on this thread, behavior is undefined.
    return kota::event_loop::current();
}

}  // namespace detail

template <typename Protocol>
auto call_completion_async(clore::net::CompletionRequest request, kota::event_loop* loop)
    -> kota::task<clore::net::CompletionResponse, clore::net::LLMError> {
    const bool needs_tools = !request.tools.empty();

    for(int probe_attempt = 0; probe_attempt < 4; ++probe_attempt) {
        auto environment = Protocol::read_environment();
        if(!environment.has_value()) {
            co_await kota::fail(std::move(environment.error()));
        }

        auto& caps =
            get_probed_capabilities(Protocol::capability_probe_key(*environment, request));
        auto sanitized = sanitize_request_for_capabilities(std::move(request), caps);
        const bool tools_stripped = needs_tools && sanitized.tools.empty();

        auto request_json = Protocol::build_request_json(sanitized);
        if(!request_json.has_value()) {
            co_await kota::fail(std::move(request_json.error()));
        }

        auto& active_loop = detail::select_event_loop(loop);
        auto raw_response_result = co_await clore::net::detail::perform_http_request_async(
            Protocol::build_url(*environment),
            Protocol::build_headers(*environment),
            std::move(*request_json),
            active_loop);

        if(!raw_response_result.has_value()) {
            co_await kota::fail(std::move(raw_response_result.error()));
        }

        auto& raw_response = *raw_response_result;

        if(raw_response.http_status >= 400 && raw_response.http_status < 500) {
            if(is_feature_rejection_error(raw_response.body)) {
                auto rejected = parse_rejected_feature_from_error(raw_response.body);
                if(rejected.has_value() && probe_attempt < 3) {
                    bool changed = false;
                    if(*rejected == "response_format") {
                        caps.supports_json_schema.store(false, std::memory_order_relaxed);
                        changed = true;
                    } else if(*rejected == "tool_choice") {
                        caps.supports_tool_choice.store(false, std::memory_order_relaxed);
                        changed = true;
                    } else if(*rejected == "parallel_tool_calls") {
                        caps.supports_parallel_tool_calls.store(false, std::memory_order_relaxed);
                        changed = true;
                    } else if(*rejected == "tools") {
                        caps.supports_tools.store(false, std::memory_order_relaxed);
                        changed = true;
                    }
                    if(changed) {
                        logging::warn("Provider '{}' rejected '{}', retrying with reduced features",
                                      Protocol::provider_name(),
                                      *rejected);
                        request = std::move(sanitized);
                        continue;
                    }
                }
            }
        }

        auto parsed = Protocol::parse_response(raw_response);
        if(!parsed.has_value()) {
            co_await kota::fail(std::move(parsed.error()));
        }

        if(tools_stripped) {
            co_await kota::fail(LLMError(
                std::format("Provider '{}' does not support tools (required for this request)",
                            Protocol::provider_name())));
        }

        co_return std::move(*parsed);
    }

    co_await kota::fail(LLMError(
        std::format("Provider '{}' capability probing exhausted", Protocol::provider_name())));
}

template <typename Protocol>
auto call_llm_async(std::string_view model,
                    std::string_view system_prompt,
                    clore::net::PromptRequest request,
                    kota::event_loop* loop) -> kota::task<std::string, clore::net::LLMError> {
    auto& active_loop = detail::select_event_loop(loop);
    co_return co_await clore::net::detail::unwrap_caught_result(
        co_await clore::net::detail::request_text_once_async(
            [](clore::net::CompletionRequest request, kota::event_loop& request_loop) {
                return call_completion_async<Protocol>(std::move(request), &request_loop);
            },
            model,
            system_prompt,
            std::move(request),
            active_loop)
            .catch_cancel(),
        "LLM request cancelled");
}

template <typename Protocol>
auto call_llm_async(std::string_view model,
                    std::string_view system_prompt,
                    std::string_view prompt,
                    kota::event_loop* loop) -> kota::task<std::string, clore::net::LLMError> {
    auto& active_loop = detail::select_event_loop(loop);
    co_return co_await clore::net::detail::request_text_once_async(
        [](clore::net::CompletionRequest request, kota::event_loop& request_loop) {
            return call_completion_async<Protocol>(std::move(request), &request_loop);
        },
        model,
        system_prompt,
        clore::net::PromptRequest{
            .prompt = std::string(prompt),
            .response_format = std::nullopt,
            .output_contract = clore::net::PromptOutputContract::Markdown,
        },
        active_loop)
        .or_fail();
}

template <typename Protocol, typename T>
auto call_structured_async(std::string_view model,
                           std::string_view system_prompt,
                           std::string_view prompt,
                           kota::event_loop* loop) -> kota::task<T, clore::net::LLMError> {
    auto format = clore::net::schema::response_format<T>();
    if(!format.has_value()) {
        co_await kota::fail(std::move(format.error()));
    }

    clore::net::CompletionRequest request{
        .model = std::string(model),
        .messages =
            {
                       clore::net::SystemMessage{.content = std::string(system_prompt)},
                       clore::net::UserMessage{.content = std::string(prompt)},
                       },
        .response_format = std::move(*format),
        .tools = {                                                               },
        .tool_choice = std::nullopt,
        .parallel_tool_calls = std::nullopt,
    };

    auto response = co_await call_completion_async<Protocol>(std::move(request), loop).or_fail();
    auto parsed = clore::net::protocol::parse_response_text<T>(response);
    if(!parsed.has_value()) {
        co_await kota::fail(std::move(parsed.error()));
    }
    co_return std::move(*parsed);
}

}  // namespace clore::net
