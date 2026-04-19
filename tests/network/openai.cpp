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
#include "kota/codec/json/json.h"
#include "kota/zest/zest.h"

import network;
import support;

using namespace clore::net;

namespace {

namespace json = kota::codec::json;

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

auto parse_object(std::string_view text) -> std::expected<json::Object, std::string> {
    auto parsed = json::Object::parse(text);
    if(!parsed.has_value()) {
        return std::unexpected(
            std::string(json::error_message(json::make_read_error(parsed.error()))));
    }
    return *parsed;
}

auto required_contains(json::ArrayRef required, std::string_view name) -> bool {
    return std::ranges::any_of(required, [name](json::ValueRef value) {
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

// Wrap a synchronous request handler into the async shape required by
// request_text_with_retries.  Uses a free-function coroutine instead of a
// coroutine-lambda to avoid Apple-clang coroutine-lambda capture bugs.
template <typename Fn>
auto invoke_sync_handler(Fn fn, const PromptRequest& req) -> kota::task<std::string, LLMError> {
    auto result = fn(req);
    if(result.has_value()) {
        co_return std::move(*result);
    }
    co_await kota::fail(std::move(result.error()));
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

template <typename Fn>
auto make_async_requester(Fn fn) {
    return [fn](std::string, std::string, const PromptRequest& req, kota::event_loop&) {
        return invoke_sync_handler(fn, req);
    };
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

    auto parsed = parse_object(*json);
    ASSERT_TRUE(parsed.has_value());

    auto model = parsed->get("model");
    ASSERT_TRUE(model.has_value());
    EXPECT_EQ(model->get_string().value_or(""), "deepseek-chat");

    auto messages_value = parsed->get("messages");
    ASSERT_TRUE(messages_value.has_value());
    auto messages = messages_value->get_array();
    ASSERT_TRUE(messages.has_value());
    ASSERT_EQ(messages->size(), 2u);

    auto system_message = (*messages)[0].get_object();
    auto user_message = (*messages)[1].get_object();
    ASSERT_TRUE(system_message.has_value());
    ASSERT_TRUE(user_message.has_value());
    EXPECT_EQ(system_message->get("content")->get_string().value_or(""), "你是一名中文文档作者。");
    EXPECT_EQ(user_message->get("content")->get_string().value_or(""),
              "请说明 `LLMClient` 的职责。");
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

    auto parsed = parse_object(*json);
    ASSERT_TRUE(parsed.has_value());

    auto messages_value = parsed->get("messages");
    ASSERT_TRUE(messages_value.has_value());
    auto messages = messages_value->get_array();
    ASSERT_TRUE(messages.has_value());
    auto user_message = (*messages)[1].get_object();
    ASSERT_TRUE(user_message.has_value());
    EXPECT_EQ(user_message->get("content")->get_string().value_or(""),
              clore::support::ensure_utf8(invalid_prompt));
}

TEST_CASE(response_format_generates_strict_schema) {
    auto format = schema::response_format<SummaryPayload>();

    ASSERT_TRUE(format.has_value());
    EXPECT_TRUE(format->strict);
    EXPECT_FALSE(format->name.empty());
    ASSERT_TRUE(format->schema.has_value());

    auto type = format->schema->get("type");
    ASSERT_TRUE(type.has_value());
    EXPECT_EQ(type->get_string().value_or(""), "object");

    auto additional_properties = format->schema->get("additionalProperties");
    ASSERT_TRUE(additional_properties.has_value());
    EXPECT_EQ(additional_properties->get_bool().value_or(true), false);

    auto properties_value = format->schema->get("properties");
    ASSERT_TRUE(properties_value.has_value());
    auto properties = properties_value->get_object();
    ASSERT_TRUE(properties.has_value());
    EXPECT_TRUE(properties->get("title").has_value());
    EXPECT_TRUE(properties->get("notes").has_value());
    EXPECT_TRUE(properties->get("tags").has_value());

    auto required_value = format->schema->get("required");
    ASSERT_TRUE(required_value.has_value());
    auto required = required_value->get_array();
    ASSERT_TRUE(required.has_value());
    EXPECT_TRUE(required_contains(*required, "title"));
    EXPECT_TRUE(required_contains(*required, "notes"));
    EXPECT_TRUE(required_contains(*required, "tags"));

    auto notes_schema = properties->get("notes");
    ASSERT_TRUE(notes_schema.has_value());
    auto notes_object = notes_schema->get_object();
    ASSERT_TRUE(notes_object.has_value());
    auto any_of_value = notes_object->get("anyOf");
    ASSERT_TRUE(any_of_value.has_value());
    auto any_of = any_of_value->get_array();
    ASSERT_TRUE(any_of.has_value());
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

    auto parsed = parse_object(*json);
    ASSERT_TRUE(parsed.has_value());

    auto response_format_value = parsed->get("response_format");
    ASSERT_TRUE(response_format_value.has_value());
    auto response_format_object = response_format_value->get_object();
    ASSERT_TRUE(response_format_object.has_value());
    EXPECT_EQ(response_format_object->get("type")->get_string().value_or(""), "json_schema");

    auto json_schema_value = response_format_object->get("json_schema");
    ASSERT_TRUE(json_schema_value.has_value());
    auto json_schema = json_schema_value->get_object();
    ASSERT_TRUE(json_schema.has_value());
    EXPECT_EQ(json_schema->get("strict")->get_bool().value_or(false), true);

    auto tools_value = parsed->get("tools");
    ASSERT_TRUE(tools_value.has_value());
    auto tools = tools_value->get_array();
    ASSERT_TRUE(tools.has_value());
    ASSERT_EQ(tools->size(), 1u);
    auto tool_object = (*tools)[0].get_object();
    ASSERT_TRUE(tool_object.has_value());
    EXPECT_EQ(tool_object->get("type")->get_string().value_or(""), "function");

    auto function_value = tool_object->get("function");
    ASSERT_TRUE(function_value.has_value());
    auto function = function_value->get_object();
    ASSERT_TRUE(function.has_value());
    EXPECT_EQ(function->get("name")->get_string().value_or(""), "search_repo");
    EXPECT_EQ(function->get("strict")->get_bool().value_or(false), true);

    auto tool_choice_value = parsed->get("tool_choice");
    ASSERT_TRUE(tool_choice_value.has_value());
    EXPECT_EQ(tool_choice_value->get_string().value_or(""), "required");

    auto parallel_tool_calls_value = parsed->get("parallel_tool_calls");
    ASSERT_TRUE(parallel_tool_calls_value.has_value());
    EXPECT_EQ(parallel_tool_calls_value->get_bool().value_or(true), false);
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

TEST_CASE(request_text_with_retries_preserves_response_format_on_retry) {
    auto response_format = schema::response_format<SummaryPayload>();

    ASSERT_TRUE(response_format.has_value());

    struct Log {
        std::size_t count = 0;
        bool first_had_format = false;
        bool second_had_format = false;
    } log;

    auto* log_state = &log;

    auto result = detail::run_task_sync<std::string>([response_format = *response_format,
                                                      log_state](kota::event_loop& loop) {
        return detail::request_text_with_retries(
            "deepseek-chat",
            "You return JSON.",
            PromptRequest{
                .prompt = "Summarize this symbol.",
                .response_format = response_format,
                .output_contract = PromptOutputContract::Json,
            },
            1,
            0,
            loop,
            make_async_requester([log_state](const PromptRequest& request)
                                     -> std::expected<std::string, LLMError> {
                ++log_state->count;
                if(log_state->count == 1) {
                    log_state->first_had_format = request.response_format.has_value();
                    return std::unexpected(LLMError(
                        "LLM request failed with HTTP 400: {\"error\":{\"message\":" "\"This response_format type is unavailable now\"}}"));
                }
                log_state->second_had_format = request.response_format.has_value();
                return std::string(R"({"title":"demo","notes":null,"tags":["core"]})");
            }),
            "OpenAI");
    });

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(log.count, 2u);
    EXPECT_TRUE(log.first_had_format);
    EXPECT_TRUE(log.second_had_format);
}

TEST_CASE(request_text_with_retries_allows_json_object_contract_without_schema) {
    CompletionRequest captured_request;
    bool captured = false;

    auto result = detail::run_task_sync<std::string>([&](kota::event_loop& loop) {
        return detail::request_text_with_retries(
            "deepseek-chat",
            "You return JSON.",
            PromptRequest{
                .prompt = "Summarize this symbol.",
                .output_contract = PromptOutputContract::Json,
            },
            0,
            0,
            loop,
            [&](std::string_view model,
                std::string_view system_prompt,
                PromptRequest request,
                kota::event_loop& request_loop) {
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
                                    .message =
                                        AssistantOutput{.text = std::string(R"({"ok":true})")},
                                    .raw_json = "{}",
                                };
                            },
                            std::move(completion_request),
                            completion_loop);
                    },
                    model,
                    system_prompt,
                    std::move(request),
                    request_loop);
            },
            "OpenAI");
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

TEST_CASE(request_text_with_retries_retries_when_markdown_contains_code_fence) {
    auto result = detail::run_task_sync<std::string>([](kota::event_loop& loop) {
        return detail::request_text_with_retries(
            "deepseek-chat",
            "You return markdown.",
            PromptRequest{
                .prompt = "Summarize this symbol.",
                .output_contract = PromptOutputContract::Markdown,
            },
            1,
            0,
            loop,
            make_async_requester(
                [](const PromptRequest& request) -> std::expected<std::string, LLMError> {
                    if(request.prompt.find("IMPORTANT: Return markdown fragment text only.") ==
                       std::string::npos) {
                        return std::string("Paragraph\n```cpp\nint x = 1;\n```\n");
                    }
                    return std::string("Recovered markdown fragment.");
                }),
            "OpenAI");
    });

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "Recovered markdown fragment.");
}

TEST_CASE(request_text_with_retries_retries_when_markdown_json_is_invalid) {
    std::size_t call_count = 0;
    auto* call_count_state = &call_count;

    auto result = detail::run_task_sync<std::string>([call_count_state](kota::event_loop& loop) {
        return detail::request_text_with_retries(
            "deepseek-chat",
            "You return markdown.",
            PromptRequest{
                .prompt = "Summarize this symbol.",
                .output_contract = PromptOutputContract::Markdown,
            },
            1,
            0,
            loop,
            make_async_requester([call_count_state](const PromptRequest& request)
                                     -> std::expected<std::string, LLMError> {
                ++*call_count_state;
                if(*call_count_state == 1) {
                    return std::string(R"({"markdown":123})");
                }
                EXPECT_NE(request.prompt.find("Do not return JSON"), std::string::npos);
                return std::string("Recovered markdown fragment.");
            }),
            "OpenAI");
    });

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "Recovered markdown fragment.");
    EXPECT_EQ(call_count, 2u);
}

TEST_CASE(request_text_with_retries_rejects_unspecified_output_contract) {
    auto result = detail::run_task_sync<std::string>([](kota::event_loop& loop) {
        return detail::request_text_with_retries(
            "deepseek-chat",
            "You return text.",
            PromptRequest{
                .prompt = "Summarize this symbol.",
            },
            0,
            0,
            loop,
            make_async_requester([](const PromptRequest&) -> std::expected<std::string, LLMError> {
                return std::string("unreachable");
            }),
            "OpenAI");
    });

    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("must be explicitly set to json or markdown"),
              std::string::npos);
}

TEST_CASE(response_format_fixed_arrays_use_exact_bounds) {
    auto format = schema::response_format<FixedLabelPayload>();

    ASSERT_TRUE(format.has_value());
    ASSERT_TRUE(format->schema.has_value());

    auto encoded = format->schema->to_json_string();
    ASSERT_TRUE(encoded.has_value());
    EXPECT_NE(encoded->find(R"("labels":{"type":"array")"), std::string::npos);
    EXPECT_NE(encoded->find(R"("minItems":2)"), std::string::npos);
    EXPECT_NE(encoded->find(R"("maxItems":2)"), std::string::npos);
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

};  // TEST_SUITE(llm)
