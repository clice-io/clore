module;

#include "kota/async/async.h"

export module generate:bridge;

import std;
import network;

export namespace clore::generate {

auto request_llm_async(std::string_view provider,
                       std::string_view model,
                       std::string_view system_prompt,
                       clore::net::PromptRequest request,
                       std::uint32_t retry_count,
                       std::uint32_t retry_initial_backoff_ms,
                       kota::event_loop& loop) -> kota::task<std::string, clore::net::LLMError>;

}  // namespace clore::generate

namespace clore::generate {

auto request_llm_async(std::string_view provider,
                       std::string_view model,
                       std::string_view system_prompt,
                       clore::net::PromptRequest request,
                       std::uint32_t retry_count,
                       std::uint32_t retry_initial_backoff_ms,
                       kota::event_loop& loop) -> kota::task<std::string, clore::net::LLMError> {
    co_return co_await clore::net::call_llm_async(provider,
                                                  model,
                                                  system_prompt,
                                                  std::move(request),
                                                  retry_count,
                                                  retry_initial_backoff_ms,
                                                  loop)
        .or_fail();
}

}  // namespace clore::generate
