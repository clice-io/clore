#include <coroutine>
#include <cstdlib>

#include "kota/async/async.h"
#include "kota/zest/zest.h"

import network;

using namespace clore::net;

TEST_SUITE(network_dispatch) {

TEST_CASE(call_completion_async_rejects_unknown_provider) {
    ::unsetenv("ANTHROPIC_BASE_URL");
    ::unsetenv("ANTHROPIC_API_KEY");
    ::unsetenv("OPENAI_BASE_URL");
    ::unsetenv("OPENAI_API_KEY");

    CompletionRequest request{};
    request.messages.push_back(UserMessage{.content = "ping"});

    auto result = detail::run_task_sync<CompletionResponse>(
        [&](auto& loop) { return call_completion_async(request, loop); });

    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("no supported llm provider environment found"),
              std::string::npos);
}

TEST_CASE(call_llm_async_rejects_unknown_provider) {
    ::unsetenv("ANTHROPIC_BASE_URL");
    ::unsetenv("ANTHROPIC_API_KEY");
    ::unsetenv("OPENAI_BASE_URL");
    ::unsetenv("OPENAI_API_KEY");

    auto result = detail::run_task_sync<std::string>([&](auto& loop) {
        return call_llm_async("model", "system", PromptRequest{.prompt = "hello"}, 0, 0, loop);
    });

    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("no supported llm provider environment found"),
              std::string::npos);
}

TEST_CASE(call_llm_async_rejects_empty_provider) {
    ::unsetenv("ANTHROPIC_BASE_URL");
    ::unsetenv("ANTHROPIC_API_KEY");
    ::unsetenv("OPENAI_BASE_URL");
    ::unsetenv("OPENAI_API_KEY");

    auto result = detail::run_task_sync<std::string>([&](auto& loop) {
        return call_llm_async("model", "system", PromptRequest{.prompt = "hello"}, 0, 0, loop);
    });

    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("no supported llm provider environment found"),
              std::string::npos);
}

};  // TEST_SUITE(network_dispatch)
