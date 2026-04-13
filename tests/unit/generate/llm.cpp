#include "eventide/zest/zest.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <expected>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "eventide/serde/json/json.h"

import network;
import support;

using namespace clore::net;

namespace {

namespace json = eventide::serde::json;

struct ScopedEnvVar {
    std::string name;
    std::optional<std::string> original_value;

    explicit ScopedEnvVar(std::string var_name) : name(std::move(var_name)) {
        if(auto* value = std::getenv(name.c_str()); value != nullptr) {
            original_value = value;
        }
    }

    ~ScopedEnvVar() { restore(); }

    void set(std::string_view value) const {
#ifdef _WIN32
        _putenv_s(name.c_str(), std::string(value).c_str());
#else
        setenv(name.c_str(), std::string(value).c_str(), 1);
#endif
    }

    void unset() const {
#ifdef _WIN32
        _putenv_s(name.c_str(), "");
#else
        unsetenv(name.c_str());
#endif
    }

    void restore() const {
        if(original_value.has_value()) {
            set(*original_value);
            return;
        }
        unset();
    }
};

struct SummaryPayload {
    std::string title;
    std::optional<std::string> notes;
    std::vector<std::string> tags;
};

struct SearchToolArgs {
    std::string query;
    std::int64_t limit = 0;
};

auto parse_object(std::string_view text) -> std::expected<json::Object, std::string> {
    auto parsed = json::Object::parse(text);
    if(!parsed.has_value()) {
        return std::unexpected(std::string(
            json::error_message(json::make_read_error(parsed.error()))));
    }
    return *parsed;
}

auto required_contains(json::ArrayRef required, std::string_view name) -> bool {
    return std::ranges::any_of(required, [name](json::ValueRef value) {
        auto current = value.get_string();
        return current.has_value() && *current == name;
    });
}

auto extract_text_response(std::string_view payload)
    -> std::expected<std::string, LLMError> {
    auto response = protocol::parse_response(payload);
    if(!response.has_value()) {
        return std::unexpected(std::move(response.error()));
    }
    return protocol::text_from_response(*response);
}

}  // namespace

TEST_SUITE(llm) {
    TEST_CASE(build_request_json_uses_requested_model) {
        CompletionRequest request{
            .model = "deepseek-chat",
            .messages = {
                SystemMessage{.content = "You are a writer."},
                UserMessage{.content = "Document this function."},
            },
        };

        auto json = protocol::build_request_json(request);

        ASSERT_TRUE(json.has_value());
        EXPECT_NE(json->find(R"("model":"deepseek-chat")"), std::string::npos);
        EXPECT_NE(json->find("Document this function."), std::string::npos);
    }

    TEST_CASE(build_request_json_preserves_utf8_content) {
        CompletionRequest request{
            .model = "deepseek-chat",
            .messages = {
                SystemMessage{.content = "你是一名中文文档作者。"},
                UserMessage{.content = "请说明 `LLMClient` 的职责。"},
            },
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
        EXPECT_EQ(system_message->get("content")->get_string().value_or(""),
                  "你是一名中文文档作者。");
        EXPECT_EQ(user_message->get("content")->get_string().value_or(""),
                  "请说明 `LLMClient` 的职责。");
    }

    TEST_CASE(build_request_json_repairs_invalid_utf8_content) {
        std::string invalid_prompt = "bad";
        invalid_prompt.push_back(static_cast<char>(0xFF));
        invalid_prompt += "prompt";

        CompletionRequest request{
            .model = "deepseek-chat",
            .messages = {
                SystemMessage{.content = "系统提示"},
                UserMessage{.content = invalid_prompt},
            },
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

        auto type = format->schema.get("type");
        ASSERT_TRUE(type.has_value());
        EXPECT_EQ(type->get_string().value_or(""), "object");

        auto additional_properties = format->schema.get("additionalProperties");
        ASSERT_TRUE(additional_properties.has_value());
        EXPECT_EQ(additional_properties->get_bool().value_or(true), false);

        auto properties_value = format->schema.get("properties");
        ASSERT_TRUE(properties_value.has_value());
        auto properties = properties_value->get_object();
        ASSERT_TRUE(properties.has_value());
        EXPECT_TRUE(properties->get("title").has_value());
        EXPECT_TRUE(properties->get("notes").has_value());
        EXPECT_TRUE(properties->get("tags").has_value());

        auto required_value = format->schema.get("required");
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
        auto tool = schema::function_tool<SearchToolArgs>(
            "search_repo", "Search the repository for matching symbols");

        ASSERT_TRUE(response_format.has_value());
        ASSERT_TRUE(tool.has_value());

        CompletionRequest request{
            .model = "deepseek-chat",
            .messages = {
                SystemMessage{.content = "You produce structured results."},
                UserMessage{.content = "Summarize the repository."},
            },
            .response_format = *response_format,
            .tools = {*tool},
            .tool_choice = ToolChoiceRequired{},
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
        EXPECT_EQ(response_format_object->get("type")->get_string().value_or(""),
                  "json_schema");

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

    TEST_CASE(parse_response_success) {
        auto result = extract_text_response(
            R"({"id":"resp_1","model":"deepseek-chat","choices":[{"message":{"content":"# Title\nGenerated docs"}}]})");

        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, "# Title\nGenerated docs");
    }

    TEST_CASE(parse_response_api_error) {
        auto result = protocol::parse_response(
            R"({"error":{"message":"bad request"}})");

        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error().message, "LLM API error: bad request");
    }

    TEST_CASE(parse_response_supports_utf8_content) {
        auto result = extract_text_response(
            R"({"id":"resp_utf8","model":"deepseek-chat","choices":[{"message":{"content":"中文摘要：负责生成文档。"}}]})");

        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, "中文摘要：负责生成文档。");
    }

    TEST_CASE(parse_response_supports_content_parts) {
        auto result = extract_text_response(
            R"({"id":"resp_parts","model":"deepseek-chat","choices":[{"message":{"content":[{"type":"text","text":"第一段。"},{"type":"output_text","text":"第二段。"}]}}]})");

        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, "第一段。第二段。");
    }

    TEST_CASE(parse_response_supports_tool_calls_and_typed_arguments) {
        auto response = protocol::parse_response(
            R"({"id":"resp_tool","model":"deepseek-chat","choices":[{"message":{"content":null,"tool_calls":[{"id":"call_1","type":"function","function":{"name":"search_repo","arguments":"{\"query\":\"llm\",\"limit\":2}"}}]}}]})");

        ASSERT_TRUE(response.has_value());
        ASSERT_EQ(response->message.tool_calls.size(), 1u);
        EXPECT_EQ(response->message.tool_calls[0].name, "search_repo");
        EXPECT_EQ(response->message.tool_calls[0].arguments_json,
                  R"({"query":"llm","limit":2})");

        auto args = protocol::parse_tool_arguments<SearchToolArgs>(
            response->message.tool_calls[0]);
        ASSERT_TRUE(args.has_value());
        EXPECT_EQ(args->query, "llm");
        EXPECT_EQ(args->limit, 2);
    }

    TEST_CASE(append_tool_outputs_builds_follow_up_history) {
        std::vector<Message> history{
            SystemMessage{.content = "system"},
            UserMessage{.content = "Find the llm module."},
        };

        auto response = protocol::parse_response(
            R"({"id":"resp_tool","model":"deepseek-chat","choices":[{"message":{"content":"Looking it up.","tool_calls":[{"id":"call_1","type":"function","function":{"name":"search_repo","arguments":"{\"query\":\"llm\",\"limit\":1}"}}]}}]})");
        ASSERT_TRUE(response.has_value());

        std::vector<ToolOutput> outputs{
            ToolOutput{
                .tool_call_id = "call_1",
                .output = R"({"matches":["src/llm/llm.cppm"]})",
            },
        };

        auto merged = protocol::append_tool_outputs(
            history, *response, std::span<const ToolOutput>(outputs));

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
        EXPECT_EQ(tool_result->content, R"({"matches":["src/llm/llm.cppm"]})");
    }

    TEST_CASE(call_llm_requires_openai_base_url_env) {
        ScopedEnvVar base_url("OPENAI_BASE_URL");
        ScopedEnvVar api_key("OPENAI_API_KEY");

        base_url.unset();
        api_key.set("test-key");

        auto result = call_llm("deepseek-chat", "system", "ping");

        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error().message,
                  "required environment variable OPENAI_BASE_URL is not set");
    }

    TEST_CASE(call_llm_requires_openai_api_key_env) {
        ScopedEnvVar base_url("OPENAI_BASE_URL");
        ScopedEnvVar api_key("OPENAI_API_KEY");

        base_url.set("https://example.invalid/v1");
        api_key.unset();

        auto result = call_llm("deepseek-chat", "system", "ping");

        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error().message,
                  "required environment variable OPENAI_API_KEY is not set");
    }

    TEST_CASE(llm_client_requires_openai_base_url_env) {
        ScopedEnvVar base_url("OPENAI_BASE_URL");
        ScopedEnvVar api_key("OPENAI_API_KEY");

        base_url.unset();
        api_key.set("test-key");

        LLMClient client("deepseek-chat", "system", 4, 3, 250);
        (void)client.submit(0, "ping");

        auto result = client.run([](std::uint64_t, auto) {});

        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error().message,
                  "required environment variable OPENAI_BASE_URL is not set");
    }

    TEST_CASE(llm_client_requires_openai_api_key_env) {
        ScopedEnvVar base_url("OPENAI_BASE_URL");
        ScopedEnvVar api_key("OPENAI_API_KEY");

        base_url.set("https://example.invalid/v1");
        api_key.unset();

        LLMClient client("deepseek-chat", "system", 4, 3, 250);
        (void)client.submit(0, "ping");

        auto result = client.run([](std::uint64_t, auto) {});

        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error().message,
                  "required environment variable OPENAI_API_KEY is not set");
    }

    TEST_CASE(llm_client_run_with_no_work_succeeds) {
        LLMClient client("deepseek-chat", "system", 4, 3, 250);

        // run() with nothing submitted should return immediately
        auto result = client.run([](std::uint64_t, auto) {});

        // No pending work → no env required, early exit
        EXPECT_TRUE(result.has_value());
    }

    TEST_CASE(llm_client_rejects_zero_max_concurrent) {
        LLMClient client("deepseek-chat", "system", 0, 3, 250);

        auto submit_result = client.submit(0, "ping");
        EXPECT_FALSE(submit_result.has_value());
        EXPECT_EQ(submit_result.error().message,
                  "max_concurrent must be greater than 0");

        auto run_result = client.run([](std::uint64_t, auto) {});

        EXPECT_FALSE(run_result.has_value());
        EXPECT_EQ(run_result.error().message,
                  "max_concurrent must be greater than 0");
    }

    TEST_CASE(llm_client_runs_scheduled_requests_and_reports_network_failures) {
        ScopedEnvVar base_url("OPENAI_BASE_URL");
        ScopedEnvVar api_key("OPENAI_API_KEY");

        base_url.set("http://127.0.0.1:1");
        api_key.set("test-key");

        LLMClient client("deepseek-chat", "system", 1, 0, 1);
        auto submit_result = client.submit(7, "ping");
        ASSERT_TRUE(submit_result.has_value());

        auto callback_count = 0u;
        std::optional<std::uint64_t> callback_tag;
        std::optional<bool> callback_has_value;
        std::string callback_error_message;

        auto run_result = client.run([&](std::uint64_t tag, auto result) {
            callback_tag = tag;
            callback_has_value = result.has_value();
            if(!result.has_value()) {
                callback_error_message = std::move(result.error().message);
            }
            ++callback_count;
        });

        EXPECT_TRUE(run_result.has_value());
        EXPECT_EQ(callback_count, 1u);
        ASSERT_TRUE(callback_tag.has_value());
        EXPECT_EQ(*callback_tag, 7u);
        ASSERT_TRUE(callback_has_value.has_value());
        EXPECT_FALSE(*callback_has_value);
        EXPECT_NE(callback_error_message.find("curl request failed"),
                  std::string::npos);
    }
};
