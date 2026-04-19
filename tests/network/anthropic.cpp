#include <string>

#include "kota/codec/json/json.h"
#include "kota/zest/zest.h"

import network;

using namespace clore::net;

namespace {

namespace json = kota::codec::json;

auto parse_object(std::string_view text) -> std::expected<json::Object, std::string> {
    auto parsed = json::Object::parse(text);
    if(!parsed.has_value()) {
        return std::unexpected(
            std::string(json::error_message(json::make_read_error(parsed.error()))));
    }
    return *parsed;
}

}  // namespace

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
    auto parsed = parse_object(*encoded);
    ASSERT_TRUE(parsed.has_value());

    EXPECT_EQ(parsed->get("model")->get_string().value_or(""), "claude-sonnet-4-5");
    EXPECT_EQ(parsed->get("system")->get_string().value_or(""), "You are a writer.");

    auto messages_value = parsed->get("messages");
    ASSERT_TRUE(messages_value.has_value());
    auto messages = messages_value->get_array();
    ASSERT_TRUE(messages.has_value());
    ASSERT_EQ(messages->size(), 1u);
    auto user_message = (*messages)[0].get_object();
    ASSERT_TRUE(user_message.has_value());
    EXPECT_EQ(user_message->get("role")->get_string().value_or(""), "user");
    EXPECT_EQ(user_message->get("content")->get_string().value_or(""), "Summarize this module.");
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

TEST_CASE(call_llm_async_rejects_unknown_provider) {
    auto result = detail::run_task_sync<std::string>([](auto& loop) {
        return call_llm_async("invalid",
                              "claude-sonnet-4-5",
                              "system",
                              PromptRequest{
                                  .prompt = "ping",
                                  .response_format = std::nullopt,
                                  .output_contract = PromptOutputContract::Markdown,
                              },
                              0,
                              0,
                              loop);
    });

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().message, "unsupported llm provider 'invalid'");
}

};  // TEST_SUITE(anthropic_llm)
