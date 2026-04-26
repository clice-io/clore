#include <algorithm>
#include <array>
#include <coroutine>
#include <cstdint>
#include <cstdlib>
#include <expected>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "kota/async/async.h"
#include "kota/zest/zest.h"

import network;
import support;

using namespace clore::net;

namespace {

struct SummaryPayload {
    std::string title;
    std::optional<std::string> notes;
    std::vector<std::string> tags;
};

struct SearchToolArgs {
    std::string query;
    std::int64_t limit = 0;
};

struct FixedLabelPayload {
    std::array<std::string, 2> labels;
};

auto required_contains(const auto& required, std::string_view name) -> bool {
    return std::ranges::any_of(required, [name](const auto& value) {
        auto current = value.get_string();
        return current.has_value() && *current == name;
    });
}

auto extract_text_response(std::string_view payload) -> std::expected<std::string, LLMError> {
    auto response = protocol::parse_response(payload);
    if(!response.has_value()) {
        return std::unexpected(std::move(response.error()));
    }
    return protocol::text_from_response(*response);
}

template <typename Fn>
auto invoke_completion_handler(Fn fn, CompletionRequest req, kota::event_loop& loop)
    -> kota::task<CompletionResponse, LLMError> {
    auto result = fn(std::move(req), loop);
    if(result.has_value()) {
        co_return std::move(*result);
    }
    co_await kota::fail(std::move(result.error()));
}

}  // namespace

TEST_SUITE(llm) {

TEST_CASE(build_request_json_uses_requested_model) {
    CompletionRequest request{
        .model = "deepseek-chat",
        .messages =
            {
                       SystemMessage{.content = "You are a writer."},
                       UserMessage{.content = "Document this function."},
                       },
        .response_format = std::nullopt,
        .tools = {                                            },
        .tool_choice = std::nullopt,
        .parallel_tool_calls = std::nullopt,
    };

    auto json = protocol::build_request_json(request);

    ASSERT_TRUE(json.has_value());
    EXPECT_NE(json->find(R"("model":"deepseek-chat")"), std::string::npos);
    EXPECT_NE(json->find("Document this function."), std::string::npos);
}

TEST_CASE(build_request_json_preserves_utf8_content) {
    CompletionRequest request{
        .model = "deepseek-chat",
        .messages =
            {
                       SystemMessage{.content = "你是一名中文文档作者。"},
                       UserMessage{.content = "请说明 `LLMClient` 的职责。"},
                       },
        .response_format = std::nullopt,
        .tools = {                                                 },
        .tool_choice = std::nullopt,
        .parallel_tool_calls = std::nullopt,
    };

    auto json = protocol::build_request_json(request);

    ASSERT_TRUE(json.has_value());
    EXPECT_NE(json->find(R"("model":"deepseek-chat")"), std::string::npos);
    EXPECT_NE(json->find("你是一名中文文档作者。"), std::string::npos);
    EXPECT_NE(json->find("请说明 `LLMClient` 的职责。"), std::string::npos);
}

TEST_CASE(build_request_json_repairs_invalid_utf8_content) {
    std::string invalid_prompt = "bad";
    invalid_prompt.push_back(static_cast<char>(0xFF));
    invalid_prompt += "prompt";

    CompletionRequest request{
        .model = "deepseek-chat",
        .messages =
            {
                       SystemMessage{.content = "系统提示"},
                       UserMessage{.content = invalid_prompt},
                       },
        .response_format = std::nullopt,
        .tools = {                                   },
        .tool_choice = std::nullopt,
        .parallel_tool_calls = std::nullopt,
    };

    auto json = protocol::build_request_json(request);

    ASSERT_TRUE(json.has_value());
    EXPECT_NE(json->find(clore::support::ensure_utf8(invalid_prompt)), std::string::npos);
}

TEST_CASE(response_format_generates_strict_schema) {
    auto format = schema::response_format<SummaryPayload>();

    ASSERT_TRUE(format.has_value());
    EXPECT_TRUE(format->strict);
    EXPECT_FALSE(format->name.empty());
    ASSERT_TRUE(format->schema.has_value());

    auto* type = format->schema->find("type");
    ASSERT_TRUE(type != nullptr);
    EXPECT_EQ(type->get_string().value_or(""), "object");

    auto* additional_properties = format->schema->find("additionalProperties");
    ASSERT_TRUE(additional_properties != nullptr);
    EXPECT_EQ(additional_properties->get_bool().value_or(true), false);

    auto* properties_value = format->schema->find("properties");
    ASSERT_TRUE(properties_value != nullptr);
    auto* properties = properties_value->get_object();
    ASSERT_TRUE(properties != nullptr);
    EXPECT_TRUE(properties->find("title") != nullptr);
    EXPECT_TRUE(properties->find("notes") != nullptr);
    EXPECT_TRUE(properties->find("tags") != nullptr);

    auto* required_value = format->schema->find("required");
    ASSERT_TRUE(required_value != nullptr);
    auto* required = required_value->get_array();
    ASSERT_TRUE(required != nullptr);
    EXPECT_TRUE(required_contains(*required, "title"));
    EXPECT_TRUE(required_contains(*required, "notes"));
    EXPECT_TRUE(required_contains(*required, "tags"));

    auto* notes_schema = properties->find("notes");
    ASSERT_TRUE(notes_schema != nullptr);
    auto* notes_object = notes_schema->get_object();
    ASSERT_TRUE(notes_object != nullptr);
    auto* any_of_value = notes_object->find("anyOf");
    ASSERT_TRUE(any_of_value != nullptr);
    auto* any_of = any_of_value->get_array();
    ASSERT_TRUE(any_of != nullptr);
    ASSERT_EQ(any_of->size(), 2u);
}

TEST_CASE(build_request_json_serializes_structured_output_and_tools) {
    auto response_format = schema::response_format<SummaryPayload>();
    auto tool = schema::function_tool<SearchToolArgs>("search_repo",
                                                      "Search the repository for matching symbols");

    ASSERT_TRUE(response_format.has_value());
    ASSERT_TRUE(tool.has_value());

    CompletionRequest request{
        .model = "deepseek-chat",
        .messages =
            {
                       SystemMessage{.content = "You produce structured results."},
                       UserMessage{.content = "Summarize the repository."},
                       },
        .response_format = *response_format,
        .tools = {*tool                                                          },
        .tool_choice = ToolChoiceRequired{                                                      },
        .parallel_tool_calls = false,
    };

    auto json = protocol::build_request_json(request);

    ASSERT_TRUE(json.has_value());
    EXPECT_NE(json->find(R"("response_format":{"type":"json_schema")"), std::string::npos);
    EXPECT_NE(json->find(R"("strict":true)"), std::string::npos);
    EXPECT_NE(json->find(R"("tools":[{"type":"function")"), std::string::npos);
    EXPECT_NE(json->find(R"("name":"search_repo")"), std::string::npos);
    EXPECT_NE(json->find(R"("tool_choice":"required")"), std::string::npos);
    EXPECT_NE(json->find(R"("parallel_tool_calls":false)"), std::string::npos);
}

TEST_CASE(validate_markdown_fragment_output_rejects_h1) {
    auto result = protocol::validate_markdown_fragment_output("# Title\n");

    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("H1"), std::string::npos);
}

TEST_CASE(validate_markdown_fragment_output_rejects_code_fence) {
    auto result =
        protocol::validate_markdown_fragment_output("Paragraph\n```cpp\nint x = 1;\n```\n");

    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("code fence"), std::string::npos);
}

TEST_CASE(validate_markdown_fragment_output_accepts_plain_fragment) {
    auto result =
        protocol::validate_markdown_fragment_output("A short paragraph with `inline_code`.\n");

    EXPECT_TRUE(result.has_value());
}

TEST_CASE(make_markdown_fragment_request_marks_markdown_contract) {
    auto request = make_markdown_fragment_request("Summarize this symbol.");

    EXPECT_EQ(request.prompt, "Summarize this symbol.");
    EXPECT_EQ(request.output_contract, PromptOutputContract::Markdown);
}

TEST_CASE(request_text_once_preserves_response_format) {
    auto response_format = schema::response_format<SummaryPayload>();

    ASSERT_TRUE(response_format.has_value());

    bool had_format = false;

    auto result = detail::run_task_sync<std::string>(
        [response_format = *response_format, &had_format](kota::event_loop& loop) {
            return detail::request_text_once_async(
                [&](CompletionRequest completion_request, kota::event_loop& completion_loop) {
                    return invoke_completion_handler(
                        [&](CompletionRequest request,
                            kota::event_loop&) -> std::expected<CompletionResponse, LLMError> {
                            had_format = request.response_format.has_value();
                            return CompletionResponse{
                                .id = "resp_json",
                                .model = "deepseek-chat",
                                .message =
                                    AssistantOutput{
                                        .text = std::string(
                                            R"({"title":"demo","notes":null,"tags":["core"]})")},
                                .raw_json = "{}",
                            };
                        },
                        std::move(completion_request),
                        completion_loop);
                },
                "deepseek-chat",
                "You return JSON.",
                PromptRequest{
                    .prompt = "Summarize this symbol.",
                    .response_format = response_format,
                    .output_contract = PromptOutputContract::Json,
                },
                loop);
        });

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(had_format);
}

TEST_CASE(request_text_once_allows_json_object_contract_without_schema) {
    CompletionRequest captured_request;
    bool captured = false;

    auto result = detail::run_task_sync<std::string>([&](kota::event_loop& loop) {
        return detail::request_text_once_async(
            [&](CompletionRequest completion_request, kota::event_loop& completion_loop) {
                return invoke_completion_handler(
                    [&](CompletionRequest request,
                        kota::event_loop&) -> std::expected<CompletionResponse, LLMError> {
                        captured_request = std::move(request);
                        captured = true;
                        return CompletionResponse{
                            .id = "resp_json",
                            .model = "deepseek-chat",
                            .message = AssistantOutput{.text = std::string(R"({"ok":true})")},
                            .raw_json = "{}",
                        };
                    },
                    std::move(completion_request),
                    completion_loop);
            },
            "deepseek-chat",
            "You return JSON.",
            PromptRequest{
                .prompt = "Summarize this symbol.",
                .output_contract = PromptOutputContract::Json,
            },
            loop);
    });

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, R"({"ok":true})");
    ASSERT_TRUE(captured);
    ASSERT_TRUE(captured_request.response_format.has_value());
    EXPECT_FALSE(captured_request.response_format->schema.has_value());

    auto encoded = protocol::build_request_json(captured_request);
    ASSERT_TRUE(encoded.has_value());
    EXPECT_NE(encoded->find(R"("response_format":{"type":"json_object"})"), std::string::npos);
}

TEST_CASE(request_text_once_rejects_markdown_code_fence) {
    auto result = detail::run_task_sync<std::string>([](kota::event_loop& loop) {
        return detail::request_text_once_async(
            [](CompletionRequest, kota::event_loop&) -> kota::task<CompletionResponse, LLMError> {
                co_return CompletionResponse{
                    .id = "resp_markdown",
                    .model = "deepseek-chat",
                    .message = AssistantOutput{.text = std::string(
                                                   "Paragraph\n```cpp\nint x = 1;\n```\n")},
                    .raw_json = "{}",
                };
            },
            "deepseek-chat",
            "You return markdown.",
            PromptRequest{
                .prompt = "Summarize this symbol.",
                .output_contract = PromptOutputContract::Markdown,
            },
            loop);
    });

    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("code fence"), std::string::npos);
}

TEST_CASE(request_text_once_rejects_markdown_json) {
    auto result = detail::run_task_sync<std::string>([](kota::event_loop& loop) {
        return detail::request_text_once_async(
            [](CompletionRequest, kota::event_loop&) -> kota::task<CompletionResponse, LLMError> {
                co_return CompletionResponse{
                    .id = "resp_markdown",
                    .model = "deepseek-chat",
                    .message = AssistantOutput{.text = std::string(R"({"markdown":123})")},
                    .raw_json = "{}",
                };
            },
            "deepseek-chat",
            "You return markdown.",
            PromptRequest{
                .prompt = "Summarize this symbol.",
                .output_contract = PromptOutputContract::Markdown,
            },
            loop);
    });

    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("JSON"), std::string::npos);
}

TEST_CASE(request_text_once_rejects_unspecified_output_contract) {
    auto result = detail::run_task_sync<std::string>([](kota::event_loop& loop) {
        return detail::request_text_once_async(
            [](CompletionRequest, kota::event_loop&) -> kota::task<CompletionResponse, LLMError> {
                co_return CompletionResponse{
                    .id = "resp_text",
                    .model = "deepseek-chat",
                    .message = AssistantOutput{.text = std::string("unreachable")},
                    .raw_json = "{}",
                };
            },
            "deepseek-chat",
            "You return text.",
            PromptRequest{
                .prompt = "Summarize this symbol.",
            },
            loop);
    });

    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("must be explicitly set to json or markdown"),
              std::string::npos);
}

TEST_CASE(response_format_fixed_arrays_use_exact_bounds) {
    auto format = schema::response_format<FixedLabelPayload>();

    ASSERT_TRUE(format.has_value());
    ASSERT_TRUE(format->schema.has_value());

    auto* properties_value = format->schema->find("properties");
    ASSERT_TRUE(properties_value != nullptr);
    auto* properties = properties_value->get_object();
    ASSERT_TRUE(properties != nullptr);

    auto* labels_value = properties->find("labels");
    ASSERT_TRUE(labels_value != nullptr);
    auto* labels = labels_value->get_object();
    ASSERT_TRUE(labels != nullptr);

    auto* type = labels->find("type");
    ASSERT_TRUE(type != nullptr);
    EXPECT_EQ(type->get_string().value_or(""), "array");

    auto* min_items = labels->find("minItems");
    auto* max_items = labels->find("maxItems");
    ASSERT_TRUE(min_items != nullptr);
    ASSERT_TRUE(max_items != nullptr);
    EXPECT_EQ(min_items->get_int().value_or(-1), 2);
    EXPECT_EQ(max_items->get_int().value_or(-1), 2);
}

TEST_CASE(parse_response_success) {
    auto result = extract_text_response(
        R"({"id":"resp_1","model":"deepseek-chat","choices":[{"finish_reason":"stop","message":{"content":"# Title\nGenerated docs"}}]})");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "# Title\nGenerated docs");
}

TEST_CASE(parse_response_requires_finish_reason) {
    auto result = protocol::parse_response(
        R"({"id":"resp_missing_finish","model":"deepseek-chat","choices":[{"message":{"content":"docs"}}]})");

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().message, "LLM response choice is missing finish_reason");
}

TEST_CASE(parse_response_rejects_truncated_finish_reason) {
    auto result = protocol::parse_response(
        R"({"id":"resp_length","model":"deepseek-chat","choices":[{"finish_reason":"length","message":{"content":"partial"}}]})");

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().message, "LLM response was truncated (finish_reason=length)");
}

TEST_CASE(parse_response_api_error) {
    auto result = protocol::parse_response(R"({"error":{"message":"bad request"}})");

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().message, "LLM API error: bad request");
}

TEST_CASE(parse_response_supports_utf8_content) {
    auto result = extract_text_response(
        R"({"id":"resp_utf8","model":"deepseek-chat","choices":[{"finish_reason":"stop","message":{"content":"中文摘要：负责生成文档。"}}]})");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "中文摘要：负责生成文档。");
}

TEST_CASE(parse_response_supports_content_parts) {
    auto result = extract_text_response(
        R"({"id":"resp_parts","model":"deepseek-chat","choices":[{"finish_reason":"stop","message":{"content":[{"type":"text","text":"第一段。"},{"type":"output_text","text":"第二段。"}]}}]})");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "第一段。第二段。");
}

TEST_CASE(parse_response_ignores_null_refusal) {
    auto response = protocol::parse_response(
        R"({"id":"resp_null_refusal","model":"deepseek-chat","choices":[{"finish_reason":"stop","message":{"content":"Generated docs","refusal":null}}]})");

    ASSERT_TRUE(response.has_value());
    ASSERT_TRUE(response->message.text.has_value());
    EXPECT_EQ(*response->message.text, "Generated docs");
    EXPECT_FALSE(response->message.refusal.has_value());
}

TEST_CASE(parse_response_supports_tool_calls_and_typed_arguments) {
    auto response = protocol::parse_response(
        R"({"id":"resp_tool","model":"deepseek-chat","choices":[{"finish_reason":"tool_calls","message":{"content":null,"tool_calls":[{"id":"call_1","type":"function","function":{"name":"search_repo","arguments":"{\"query\":\"llm\",\"limit\":2}"}}]}}]})");

    ASSERT_TRUE(response.has_value());
    ASSERT_EQ(response->message.tool_calls.size(), 1u);
    EXPECT_EQ(response->message.tool_calls[0].name, "search_repo");
    EXPECT_EQ(response->message.tool_calls[0].arguments_json, R"({"query":"llm","limit":2})");

    auto args = protocol::parse_tool_arguments<SearchToolArgs>(response->message.tool_calls[0]);
    ASSERT_TRUE(args.has_value());
    EXPECT_EQ(args->query, "llm");
    EXPECT_EQ(args->limit, 2);
}

TEST_CASE(append_tool_outputs_builds_follow_up_history) {
    std::vector<Message> history{
        SystemMessage{.content = "system"},
        UserMessage{.content = "Find the OpenAI client module."},
    };

    auto response = protocol::parse_response(
        R"({"id":"resp_tool","model":"deepseek-chat","choices":[{"finish_reason":"tool_calls","message":{"content":"Looking it up.","tool_calls":[{"id":"call_1","type":"function","function":{"name":"search_repo","arguments":"{\"query\":\"openai\",\"limit\":1}"}}]}}]})");
    ASSERT_TRUE(response.has_value());

    std::vector<ToolOutput> outputs{
        ToolOutput{
                   .tool_call_id = "call_1",
                   .output = R"({"matches":["src/network/openai.cppm"]})",
                   },
    };

    auto merged =
        protocol::append_tool_outputs(history, *response, std::span<const ToolOutput>(outputs));

    ASSERT_TRUE(merged.has_value());
    ASSERT_EQ(merged->size(), 4u);

    auto* assistant = std::get_if<AssistantToolCallMessage>(&(*merged)[2]);
    ASSERT_TRUE(assistant != nullptr);
    EXPECT_EQ(assistant->content.value_or(""), "Looking it up.");
    ASSERT_EQ(assistant->tool_calls.size(), 1u);
    EXPECT_EQ(assistant->tool_calls[0].id, "call_1");

    auto* tool_result = std::get_if<ToolResultMessage>(&(*merged)[3]);
    ASSERT_TRUE(tool_result != nullptr);
    EXPECT_EQ(tool_result->tool_call_id, "call_1");
    EXPECT_EQ(tool_result->content, R"({"matches":["src/network/openai.cppm"]})");
}

};  // TEST_SUITE(llm)
