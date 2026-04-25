#include <string>

#include "kota/zest/zest.h"

import network;

using namespace clore::net;

TEST_SUITE(anthropic_llm) {

TEST_CASE(build_messages_url_normalizes_base_url) {
    EXPECT_EQ(anthropic::protocol::build_messages_url("https://api.anthropic.com"),
              "https://api.anthropic.com/v1/messages");
    EXPECT_EQ(anthropic::protocol::build_messages_url("https://api.anthropic.com/v1"),
              "https://api.anthropic.com/v1/messages");
    EXPECT_EQ(anthropic::protocol::build_messages_url("https://api.anthropic.com/v1/"),
              "https://api.anthropic.com/v1/messages");
}

TEST_CASE(build_request_json_serializes_system_and_user_messages) {
    CompletionRequest request{
        .model = "claude-sonnet-4-5",
        .messages =
            {
                       SystemMessage{.content = "You are a writer."},
                       UserMessage{.content = "Summarize this module."},
                       },
        .response_format = std::nullopt,
        .tools = {                                            },
        .tool_choice = std::nullopt,
        .parallel_tool_calls = std::nullopt,
    };

    auto encoded = anthropic::protocol::build_request_json(request);

    ASSERT_TRUE(encoded.has_value());
    EXPECT_NE(encoded->find(R"("model":"claude-sonnet-4-5")"), std::string::npos);
    EXPECT_NE(encoded->find(R"("system":"You are a writer.")"), std::string::npos);
    EXPECT_NE(encoded->find(R"("messages":[{"role":"user","content":"Summarize this module."}])"),
              std::string::npos);
}

TEST_CASE(parse_response_supports_text_content) {
    auto response = anthropic::protocol::parse_response(
        R"({"id":"msg_1","model":"claude-sonnet-4-5","stop_reason":"end_turn","content":[{"type":"text","text":"Generated docs"}]})");

    ASSERT_TRUE(response.has_value());
    auto text = anthropic::protocol::text_from_response(*response);
    ASSERT_TRUE(text.has_value());
    EXPECT_EQ(*text, "Generated docs");
}

TEST_CASE(parse_response_supports_tool_use_blocks) {
    auto response = anthropic::protocol::parse_response(
        R"({"id":"msg_tool","model":"claude-sonnet-4-5","stop_reason":"tool_use","content":[{"type":"tool_use","id":"toolu_1","name":"search_repo","input":{"query":"llm","limit":1}}]})");

    ASSERT_TRUE(response.has_value());
    ASSERT_EQ(response->message.tool_calls.size(), 1u);
    EXPECT_EQ(response->message.tool_calls[0].id, "toolu_1");
    EXPECT_EQ(response->message.tool_calls[0].name, "search_repo");
    EXPECT_EQ(response->message.tool_calls[0].arguments_json, R"({"query":"llm","limit":1})");
}

};  // TEST_SUITE(anthropic_llm)
