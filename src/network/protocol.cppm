module;

#include "kota/async/async.h"
#include "kota/codec/json/error.h"
#include "kota/codec/json/json.h"

export module protocol;

import std;
import http;
import support;

// ── types ──────────────────────────────────────────────────────────────────
export namespace clore::net {

struct SystemMessage {
    std::string content;
};

struct UserMessage {
    std::string content;
};

struct ToolCall {
    std::string id;
    std::string name;
    std::string arguments_json;
    kota::codec::json::Value arguments;
};

struct AssistantMessage {
    std::string content;
};

struct AssistantToolCallMessage {
    std::optional<std::string> content;
    std::vector<ToolCall> tool_calls;
};

struct ToolResultMessage {
    std::string tool_call_id;
    std::string content;
};

using Message = std::variant<SystemMessage,
                             UserMessage,
                             AssistantMessage,
                             AssistantToolCallMessage,
                             ToolResultMessage>;

struct ResponseFormat {
    std::string name;
    std::optional<kota::codec::json::Object> schema;
    bool strict = true;
};

struct FunctionToolDefinition {
    std::string name;
    std::string description;
    kota::codec::json::Object parameters;
    bool strict = true;
};

struct ToolChoiceAuto {};

struct ToolChoiceRequired {};

struct ToolChoiceNone {};

struct ForcedFunctionToolChoice {
    std::string name;
};

using ToolChoice =
    std::variant<ToolChoiceAuto, ToolChoiceRequired, ToolChoiceNone, ForcedFunctionToolChoice>;

struct CompletionRequest {
    std::string model{};
    std::vector<Message> messages{};
    std::optional<ResponseFormat> response_format{};
    std::vector<FunctionToolDefinition> tools{};
    std::optional<ToolChoice> tool_choice{};
    std::optional<bool> parallel_tool_calls{};
};

enum class PromptOutputContract : std::uint8_t {
    Unspecified,
    Json,
    Markdown,
};

struct PromptRequest {
    std::string prompt{};
    std::optional<ResponseFormat> response_format{};
    std::optional<ToolChoice> tool_choice{};
    PromptOutputContract output_contract = PromptOutputContract::Unspecified;
};

auto make_markdown_fragment_request(std::string prompt) -> PromptRequest;

struct AssistantOutput {
    std::optional<std::string> text;
    std::optional<std::string> refusal;
    std::vector<ToolCall> tool_calls;
};

struct CompletionResponse {
    std::string id;
    std::string model;
    AssistantOutput message;
    std::string raw_json;
};

struct ToolOutput {
    std::string tool_call_id;
    std::string output;
};

struct ProbedCapabilities {
    std::atomic<bool> supports_json_schema{true};
    std::atomic<bool> supports_tool_choice{true};
    std::atomic<bool> supports_parallel_tool_calls{true};
    std::atomic<bool> supports_tools{true};
};

auto get_probed_capabilities(std::string_view provider) -> ProbedCapabilities&;

auto sanitize_request_for_capabilities(CompletionRequest request, const ProbedCapabilities& caps)
    -> CompletionRequest;

auto is_feature_rejection_error(std::string_view error_message) -> bool;

auto parse_rejected_feature_from_error(std::string_view error_message)
    -> std::optional<std::string>;

}  // namespace clore::net

// ── json utilities ─────────────────────────────────────────────────────────
namespace clore::net::json_detail {

namespace json = kota::codec::json;

auto append_cloned_value(json::Document& document,
                         json::Array& target,
                         json::ValueRef source,
                         std::string_view context) -> std::expected<void, LLMError>;

auto insert_cloned_value(json::Document& document,
                         json::Object& target,
                         std::string_view key,
                         json::ValueRef source,
                         std::string_view context) -> std::expected<void, LLMError>;

}  // namespace clore::net::json_detail

namespace clore::net {

auto make_markdown_fragment_request(std::string prompt) -> PromptRequest {
    return PromptRequest{
        .prompt = std::move(prompt),
        .response_format = std::nullopt,
        .output_contract = PromptOutputContract::Markdown,
    };
}

}  // namespace clore::net

export namespace clore::net::detail {

template <typename T>
using remove_cvref_t = std::remove_cvref_t<T>;

auto unexpected_json_error(std::string_view context, yyjson_write_code err)
    -> std::unexpected<LLMError>;

auto unexpected_json_error(std::string_view context, kota::codec::json::error_kind err)
    -> std::unexpected<LLMError>;

auto normalize_utf8(std::string_view text, std::string_view field_name) -> std::string;

auto insert_string_field(kota::codec::json::Object& object,
                         std::string_view key,
                         std::string_view value,
                         std::string_view context) -> std::expected<void, LLMError>;

template <typename Status>
auto to_llm_unexpected(Status error, std::string_view context) -> std::unexpected<LLMError>;

auto excerpt_for_error(std::string_view body) -> std::string;

template <typename T>
auto run_task_sync(auto&& make_task) -> std::expected<T, LLMError>;

auto make_empty_object(std::string_view context)
    -> std::expected<kota::codec::json::Object, LLMError>;

auto make_empty_array(std::string_view context)
    -> std::expected<kota::codec::json::Array, LLMError>;

template <typename T>
auto parse_json_value(std::string_view raw, std::string_view context) -> std::expected<T, LLMError>;

template <typename T>
auto parse_json_value(const kota::codec::json::Value& value, std::string_view context)
    -> std::expected<T, LLMError>;

auto serialize_value_to_string(const kota::codec::json::Value& value, std::string_view context)
    -> std::expected<std::string, LLMError>;

auto expect_object(kota::codec::json::ValueRef value, std::string_view context)
    -> std::expected<kota::codec::json::ObjectRef, LLMError>;

auto expect_array(kota::codec::json::ValueRef value, std::string_view context)
    -> std::expected<kota::codec::json::ArrayRef, LLMError>;

auto expect_string(kota::codec::json::ValueRef value, std::string_view context)
    -> std::expected<std::string_view, LLMError>;

auto clone_object(kota::codec::json::Document& document,
                  kota::codec::json::ObjectRef source,
                  std::string_view context) -> std::expected<kota::codec::json::Object, LLMError>;

auto clone_array(kota::codec::json::Document& document,
                 kota::codec::json::ArrayRef source,
                 std::string_view context) -> std::expected<kota::codec::json::Array, LLMError>;

auto clone_object(const kota::codec::json::Object& source, std::string_view context)
    -> std::expected<kota::codec::json::Object, LLMError>;

auto clone_object(kota::codec::json::ObjectRef source, std::string_view context)
    -> std::expected<kota::codec::json::Object, LLMError>;

auto clone_value(const kota::codec::json::Value& source, std::string_view context)
    -> std::expected<kota::codec::json::Value, LLMError>;

}  // namespace clore::net::detail

namespace clore::net::detail {

namespace json = kota::codec::json;

auto unexpected_json_error(std::string_view context, yyjson_write_code err)
    -> std::unexpected<LLMError> {
    return std::unexpected(
        LLMError(std::format("{}: {}", context, json::error_message(json::make_write_error(err)))));
}

auto unexpected_json_error(std::string_view context, json::error_kind err)
    -> std::unexpected<LLMError> {
    return std::unexpected(LLMError(std::format("{}: {}", context, json::error_message(err))));
}

auto normalize_utf8(std::string_view text, std::string_view field_name) -> std::string {
    auto normalized = clore::support::ensure_utf8(text);
    if(normalized != text) {
        logging::warn(
            "LLM {} contains invalid UTF-8; replaced invalid byte " "sequences before JSON serialization",
            field_name);
    }
    return normalized;
}

auto insert_string_field(json::Object& object,
                         std::string_view key,
                         std::string_view value,
                         std::string_view context) -> std::expected<void, LLMError> {
    auto status = object.insert(key, value);
    if(!status.has_value()) {
        return std::unexpected(
            LLMError(std::format("{}: {}", context, json::error_message(status.error()))));
    }
    return {};
}

template <typename Status>
auto to_llm_unexpected(Status error, std::string_view context) -> std::unexpected<LLMError> {
    return std::unexpected(LLMError(std::format("{}: {}", context, json::error_message(error))));
}

auto excerpt_for_error(std::string_view body) -> std::string {
    constexpr std::size_t kMaxBytes = 200;
    return clore::support::truncate_utf8(body, kMaxBytes);
}

template <typename T>
auto run_task_sync(auto&& make_task) -> std::expected<T, LLMError> {
    kota::event_loop loop;
    auto operation = make_task(loop);
    loop.schedule(operation);
    loop.run();

    auto result = operation.result();
    if(result.has_error()) {
        return std::unexpected(std::move(result).error());
    }

    if constexpr(std::is_void_v<T>) {
        return {};
    } else {
        return std::move(*result);
    }
}

auto make_empty_object(std::string_view context) -> std::expected<json::Object, LLMError> {
    auto parsed = json::Object::parse("{}");
    if(!parsed.has_value()) {
        return std::unexpected(
            LLMError(std::format("{}: {}",
                                 context,
                                 json::error_message(json::make_read_error(parsed.error())))));
    }
    return *parsed;
}

auto make_empty_array(std::string_view context) -> std::expected<json::Array, LLMError> {
    auto parsed = json::Array::parse("[]");
    if(!parsed.has_value()) {
        return std::unexpected(
            LLMError(std::format("{}: {}",
                                 context,
                                 json::error_message(json::make_read_error(parsed.error())))));
    }
    return *parsed;
}

template <typename T>
auto parse_json_value(std::string_view raw, std::string_view context)
    -> std::expected<T, LLMError> {
    auto parsed = json::from_json<T>(raw);
    if(!parsed.has_value()) {
        return std::unexpected(
            LLMError(std::format("failed to parse {}: {}", context, parsed.error().to_string())));
    }
    return *parsed;
}

template <typename T>
auto parse_json_value(const json::Value& value, std::string_view context)
    -> std::expected<T, LLMError> {
    auto raw = value.to_json_string();
    if(!raw.has_value()) {
        return std::unexpected(
            LLMError(std::format("failed to serialize {} for parsing: {}",
                                 context,
                                 json::error_message(json::make_write_error(raw.error())))));
    }
    return parse_json_value<T>(*raw, context);
}

auto serialize_value_to_string(const json::Value& value, std::string_view context)
    -> std::expected<std::string, LLMError> {
    auto encoded = value.to_json_string();
    if(!encoded.has_value()) {
        return std::unexpected(
            LLMError(std::format("failed to serialize {}: {}",
                                 context,
                                 json::error_message(json::make_write_error(encoded.error())))));
    }
    return *encoded;
}

auto expect_object(json::ValueRef value, std::string_view context)
    -> std::expected<json::ObjectRef, LLMError> {
    auto object = value.get_object();
    if(!object.has_value()) {
        return std::unexpected(LLMError(std::format("{} is not a JSON object", context)));
    }
    return *object;
}

auto expect_array(json::ValueRef value, std::string_view context)
    -> std::expected<json::ArrayRef, LLMError> {
    auto array = value.get_array();
    if(!array.has_value()) {
        return std::unexpected(LLMError(std::format("{} is not a JSON array", context)));
    }
    return *array;
}

auto expect_string(json::ValueRef value, std::string_view context)
    -> std::expected<std::string_view, LLMError> {
    auto text = value.get_string();
    if(!text.has_value()) {
        return std::unexpected(LLMError(std::format("{} is not a JSON string", context)));
    }
    return *text;
}

auto clone_array(json::Document& document, json::ArrayRef source, std::string_view context)
    -> std::expected<json::Array, LLMError> {
    if(!source.valid()) {
        return std::unexpected(
            LLMError(std::format("{}: source is not a valid JSON array", context)));
    }

    auto cloned = make_empty_array(context);
    if(!cloned.has_value()) {
        return std::unexpected(std::move(cloned.error()));
    }
    for(auto value: source) {
        auto status = json_detail::append_cloned_value(document, *cloned, value, context);
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
    }
    return std::move(*cloned);
}

auto clone_object(json::Document& document, json::ObjectRef source, std::string_view context)
    -> std::expected<json::Object, LLMError> {
    if(!source.valid()) {
        return std::unexpected(
            LLMError(std::format("{}: source is not a valid JSON object", context)));
    }

    auto cloned = make_empty_object(context);
    if(!cloned.has_value()) {
        return std::unexpected(std::move(cloned.error()));
    }
    for(auto entry: source) {
        auto status =
            json_detail::insert_cloned_value(document, *cloned, entry.key, entry.value, context);
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
    }
    return std::move(*cloned);
}

auto clone_object(const json::Object& source, std::string_view context)
    -> std::expected<json::Object, LLMError> {
    if(!source.valid()) {
        return std::unexpected(
            LLMError(std::format("{}: source is not a valid JSON object", context)));
    }

    auto encoded = source.to_json_string();
    if(!encoded.has_value()) {
        return unexpected_json_error(context, encoded.error());
    }
    auto parsed = json::Object::parse(*encoded);
    if(!parsed.has_value()) {
        return std::unexpected(
            LLMError(std::format("{}: {}",
                                 context,
                                 json::error_message(json::make_read_error(parsed.error())))));
    }
    return *parsed;
}

auto clone_object(json::ObjectRef source, std::string_view context)
    -> std::expected<json::Object, LLMError> {
    json::Document document;
    return clone_object(document, source, context);
}

auto clone_value(const json::Value& source, std::string_view context)
    -> std::expected<json::Value, LLMError> {
    auto encoded = source.to_json_string();
    if(!encoded.has_value()) {
        return unexpected_json_error(context, encoded.error());
    }
    auto parsed = json::Value::parse(*encoded);
    if(!parsed.has_value()) {
        return std::unexpected(
            LLMError(std::format("{}: {}",
                                 context,
                                 json::error_message(json::make_read_error(parsed.error())))));
    }
    return *parsed;
}

}  // namespace clore::net::detail

namespace clore::net::json_detail {

auto append_cloned_value(json::Document& document,
                         json::Array& target,
                         json::ValueRef source,
                         std::string_view context) -> std::expected<void, LLMError> {
    static_cast<void>(document);
    auto copied = json::Value::copy_of(source);
    if(!copied.has_value()) {
        return std::unexpected(
            LLMError(std::format("{}: {}", context, json::error_message(copied.error()))));
    }
    auto status = target.push_back(std::move(*copied));
    if(!status.has_value()) {
        return detail::to_llm_unexpected(status.error(), context);
    }
    return {};
}

auto insert_cloned_value(json::Document& document,
                         json::Object& target,
                         std::string_view key,
                         json::ValueRef source,
                         std::string_view context) -> std::expected<void, LLMError> {
    static_cast<void>(document);
    auto copied = json::Value::copy_of(source);
    if(!copied.has_value()) {
        return std::unexpected(
            LLMError(std::format("{}: {}", context, json::error_message(copied.error()))));
    }
    auto status = target.insert(key, std::move(*copied));
    if(!status.has_value()) {
        return detail::to_llm_unexpected(status.error(), context);
    }
    return {};
}

}  // namespace clore::net::json_detail

// ── text validation
// ────────────────────────────────────────────────────────
export namespace clore::net::protocol {

auto validate_json_output(std::string_view content) -> std::expected<void, LLMError>;

auto validate_markdown_fragment_output(std::string_view content) -> std::expected<void, LLMError>;

auto text_from_response(const CompletionResponse& response) -> std::expected<std::string, LLMError>;

auto append_tool_outputs(std::span<const Message> history,
                         const CompletionResponse& response,
                         std::span<const ToolOutput> outputs)
    -> std::expected<std::vector<Message>, LLMError>;

template <typename T>
auto parse_response_text(const CompletionResponse& response) -> std::expected<T, LLMError>;

template <typename T>
auto parse_tool_arguments(const ToolCall& call) -> std::expected<T, LLMError>;

}  // namespace clore::net::protocol

namespace clore::net::protocol {

auto validate_json_output(std::string_view content) -> std::expected<void, LLMError> {
    auto parsed = kota::codec::json::Value::parse(content);
    if(!parsed.has_value()) {
        return std::unexpected(LLMError(std::format(
            "LLM output is not valid JSON: {}",
            kota::codec::json::error_message(kota::codec::json::make_read_error(parsed.error())))));
    }
    return {};
}

auto validate_markdown_fragment_output(std::string_view content) -> std::expected<void, LLMError> {
    if(content.empty()) {
        return std::unexpected(LLMError("LLM output is empty"));
    }

    bool has_non_whitespace = false;
    for(char ch: content) {
        if(std::isspace(static_cast<unsigned char>(ch)) == 0) {
            has_non_whitespace = true;
            break;
        }
    }
    if(!has_non_whitespace) {
        return std::unexpected(LLMError("LLM output contains only whitespace"));
    }

    if(content.starts_with("# ") || content.find("\n# ") != std::string::npos) {
        return std::unexpected(LLMError("LLM output contains H1 heading '# '"));
    }
    if(content.find("```") != std::string::npos) {
        return std::unexpected(LLMError("LLM output contains code fence"));
    }

    auto as_json = validate_json_output(content);
    if(as_json.has_value()) {
        return std::unexpected(LLMError("LLM output is JSON but markdown fragment was required"));
    }

    return {};
}

auto text_from_response(const CompletionResponse& response)
    -> std::expected<std::string, LLMError> {
    if(response.message.refusal.has_value()) {
        return std::unexpected(LLMError(
            std::format("LLM refused structured/text response: {}", *response.message.refusal)));
    }
    if(!response.message.tool_calls.empty()) {
        return std::unexpected(
            LLMError("LLM response contains tool calls " "instead of a text completion"));
    }
    if(!response.message.text.has_value()) {
        return std::unexpected(LLMError("LLM response has no text payload"));
    }
    return *response.message.text;
}

auto append_tool_outputs(std::span<const Message> history,
                         const CompletionResponse& response,
                         std::span<const ToolOutput> outputs)
    -> std::expected<std::vector<Message>, LLMError> {
    if(response.message.tool_calls.empty()) {
        return std::unexpected(
            LLMError("cannot append tool outputs for a " "response without tool calls"));
    }

    std::unordered_map<std::string, std::string> output_by_id;
    output_by_id.reserve(outputs.size());
    for(auto& output: outputs) {
        if(output.tool_call_id.empty()) {
            return std::unexpected(LLMError("tool output id must not be empty"));
        }
        if(!output_by_id.emplace(output.tool_call_id, output.output).second) {
            return std::unexpected(
                LLMError(std::format("duplicate tool output for call '{}'", output.tool_call_id)));
        }
    }

    std::vector<Message> merged(history.begin(), history.end());
    merged.push_back(AssistantToolCallMessage{
        .content = response.message.text,
        .tool_calls = response.message.tool_calls,
    });

    for(auto& tool_call: response.message.tool_calls) {
        auto it = output_by_id.find(tool_call.id);
        if(it == output_by_id.end()) {
            return std::unexpected(
                LLMError(std::format("missing tool output for call '{}'", tool_call.id)));
        }
        merged.push_back(ToolResultMessage{
            .tool_call_id = tool_call.id,
            .content = it->second,
        });
    }

    if(output_by_id.size() != response.message.tool_calls.size()) {
        return std::unexpected(
            LLMError("tool outputs contain unknown call " "ids not present in the response"));
    }

    return merged;
}

template <typename T>
auto parse_response_text(const CompletionResponse& response) -> std::expected<T, LLMError> {
    auto text = text_from_response(response);
    if(!text.has_value()) {
        return std::unexpected(std::move(text.error()));
    }

    auto parsed = kota::codec::json::from_json<T>(*text);
    if(!parsed.has_value()) {
        return std::unexpected(LLMError(std::format("failed to parse structured LLM response: {}",
                                                    parsed.error().to_string())));
    }
    return *parsed;
}

template <typename T>
auto parse_tool_arguments(const ToolCall& call) -> std::expected<T, LLMError> {
    auto encoded = call.arguments.to_json_string();
    if(!encoded.has_value()) {
        return std::unexpected(
            LLMError(std::format("failed to serialize tool arguments for '{}': {}",
                                 call.name,
                                 kota::codec::json::error_message(
                                     kota::codec::json::make_write_error(encoded.error())))));
    }

    auto parsed = kota::codec::json::from_json<T>(*encoded);
    if(!parsed.has_value()) {
        return std::unexpected(LLMError(std::format("failed to parse tool arguments for '{}': {}",
                                                    call.name,
                                                    parsed.error().to_string())));
    }
    return *parsed;
}

}  // namespace clore::net::protocol

// ── retry logic
// ──────────────────────────────────────────────────────────
export namespace clore::net::detail {

auto compute_retry_delay(std::uint32_t next_attempt, std::uint32_t retry_initial_backoff_ms)
    -> std::expected<std::chrono::milliseconds, LLMError>;

auto infer_output_contract(const PromptRequest& request)
    -> std::expected<PromptOutputContract, LLMError>;

auto validate_prompt_output(std::string_view content, PromptOutputContract contract)
    -> std::expected<void, LLMError>;

auto retry_prompt_reminder(PromptOutputContract contract) -> std::string_view;

template <typename CompletionRequester>
auto request_text_once_async(CompletionRequester request_completion,
                             std::string_view model,
                             std::string_view system_prompt,
                             PromptRequest request,
                             kota::event_loop& loop) -> kota::task<std::string, LLMError>;

template <typename AsyncRequester>
auto request_text_with_retries(std::string model,
                               std::string system_prompt,
                               PromptRequest request,
                               std::uint32_t retry_count,
                               std::uint32_t retry_initial_backoff_ms,
                               kota::event_loop& loop,
                               AsyncRequester request_async,
                               std::string_view provider_name) -> kota::task<std::string, LLMError>;

}  // namespace clore::net::detail

namespace clore::net::detail {

auto compute_retry_delay(std::uint32_t next_attempt, std::uint32_t retry_initial_backoff_ms)
    -> std::expected<std::chrono::milliseconds, LLMError> {
    if(next_attempt == 0) {
        return std::unexpected(LLMError("next_attempt must be greater than 0"));
    }

    if(next_attempt > 63) {
        return std::unexpected(
            LLMError(std::format("retry delay overflow at attempt {}", next_attempt)));
    }

    auto multiplier = std::uint64_t{1} << (next_attempt - 1);

    if(static_cast<std::uint64_t>(retry_initial_backoff_ms) >
       (std::numeric_limits<std::uint64_t>::max() / multiplier)) {
        return std::unexpected(
            LLMError(std::format("retry delay overflow at attempt {}", next_attempt)));
    }

    auto delay_ms = static_cast<std::uint64_t>(retry_initial_backoff_ms) * multiplier;
    if(delay_ms > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        return std::unexpected(
            LLMError(std::format("retry delay exceeds supported range: {}ms", delay_ms)));
    }

    return std::chrono::milliseconds(static_cast<std::int64_t>(delay_ms));
}

auto infer_output_contract(const PromptRequest& request)
    -> std::expected<PromptOutputContract, LLMError> {
    if(request.response_format.has_value()) {
        if(request.output_contract == PromptOutputContract::Markdown) {
            return std::unexpected(LLMError(
                "PromptRequest output_contract conflicts with response_format JSON contract"));
        }
        return PromptOutputContract::Json;
    }

    if(request.output_contract == PromptOutputContract::Unspecified) {
        return std::unexpected(
            LLMError("PromptRequest output_contract must be explicitly set to json or markdown"));
    }

    return request.output_contract;
}

auto validate_prompt_output(std::string_view content, PromptOutputContract contract)
    -> std::expected<void, LLMError> {
    switch(contract) {
        case PromptOutputContract::Json: return protocol::validate_json_output(content);
        case PromptOutputContract::Markdown:
            return protocol::validate_markdown_fragment_output(content);
        case PromptOutputContract::Unspecified:
            return std::unexpected(LLMError(
                "PromptRequest output_contract must be explicitly set to json or markdown"));
    }
    return std::unexpected(LLMError("unsupported prompt output contract"));
}

auto retry_prompt_reminder(PromptOutputContract contract) -> std::string_view {
    switch(contract) {
        case PromptOutputContract::Json:
            return "\n\nIMPORTANT: Return valid JSON only that strictly matches the provided schema. " "Do not include markdown, prose, or fenced code blocks.";
        case PromptOutputContract::Markdown:
            return "\n\nIMPORTANT: Return markdown fragment text only. Do not return JSON. " "Do not include H1 headings ('# ') or fenced code blocks.";
        case PromptOutputContract::Unspecified: return {};
    }
    return {};
}

template <typename CompletionRequester>
auto request_text_once_async(CompletionRequester request_completion,
                             std::string_view model,
                             std::string_view system_prompt,
                             PromptRequest request,
                             kota::event_loop& loop) -> kota::task<std::string, LLMError> {
    std::optional<ResponseFormat> response_format = std::move(request.response_format);
    if(request.output_contract == PromptOutputContract::Json && !response_format.has_value()) {
        response_format = ResponseFormat{
            .schema = std::nullopt,
        };
    }

    CompletionRequest completion_request{
        .model = std::string(model),
        .messages =
            {
                       SystemMessage{.content = std::string(system_prompt)},
                       UserMessage{.content = std::move(request.prompt)},
                       },
        .response_format = std::move(response_format),
        .tools = {                                                   },
        .tool_choice = std::move(request.tool_choice),
        .parallel_tool_calls = std::nullopt,
    };

    auto response = co_await request_completion(std::move(completion_request), loop).or_fail();
    auto text = protocol::text_from_response(response);
    if(!text.has_value()) {
        co_await kota::fail(std::move(text.error()));
    }
    co_return std::move(*text);
}

template <typename AsyncRequester>
auto request_text_with_retries(std::string model,
                               std::string system_prompt,
                               PromptRequest request,
                               std::uint32_t retry_count,
                               std::uint32_t retry_initial_backoff_ms,
                               kota::event_loop& loop,
                               AsyncRequester request_async,
                               std::string_view provider_name)
    -> kota::task<std::string, LLMError> {
    bool appended_reminder = false;
    auto label = provider_name.empty() ? std::string_view("LLM") : provider_name;
    auto contract = infer_output_contract(request);
    if(!contract.has_value()) {
        co_await kota::fail(std::move(contract.error()));
    }

    for(std::uint32_t attempt = 0;; ++attempt) {
        auto result = co_await request_async(model, system_prompt, request, loop).catch_cancel();

        std::optional<LLMError> error;
        if(result.is_cancelled()) {
            error = LLMError("LLM request cancelled");
        } else if(result.has_error()) {
            error = LLMError(std::move(result).error());
        } else {
            auto validation = validate_prompt_output(*result, *contract);
            if(validation.has_value()) {
                co_return std::move(*result);
            }
            error = std::move(validation.error());
        }

        if(attempt >= retry_count) {
            co_await kota::fail(std::move(*error));
        }

        auto delay = compute_retry_delay(attempt + 1, retry_initial_backoff_ms);
        if(!delay.has_value()) {
            co_await kota::fail(std::move(delay.error()));
        }

        logging::warn("{} request failed: attempt={}/{} retry_in_ms={} reason={}",
                      label,
                      attempt + 1,
                      retry_count + 1,
                      delay->count(),
                      error->message);

        auto reminder = retry_prompt_reminder(*contract);
        if(!appended_reminder && !reminder.empty()) {
            request.prompt += reminder;
            appended_reminder = true;
        }

        request.prompt +=
            std::format("\n\n[SYSTEM FEEDBACK - ATTEMPT {}]: {}", attempt + 1, error->message);

        co_await kota::sleep(*delay, loop);
    }
}

}  // namespace clore::net::detail

// ── provider capability probing
// ──────────────────────────────────────────────────────
namespace clore::net {

auto get_probed_capabilities(std::string_view provider) -> ProbedCapabilities& {
    static std::mutex mutex;
    static std::unordered_map<std::string, std::unique_ptr<ProbedCapabilities>> cache;

    std::lock_guard lock(mutex);
    auto it = cache.find(std::string(provider));
    if(it != cache.end()) {
        return *it->second;
    }
    auto [inserted_it, _] =
        cache.emplace(std::string(provider), std::make_unique<ProbedCapabilities>());
    return *inserted_it->second;
}

auto sanitize_request_for_capabilities(CompletionRequest request, const ProbedCapabilities& caps)
    -> CompletionRequest {
    if(!caps.supports_json_schema.load(std::memory_order_relaxed)) {
        request.response_format = std::nullopt;
    }
    if(!caps.supports_tool_choice.load(std::memory_order_relaxed)) {
        request.tool_choice = std::nullopt;
    }
    if(!caps.supports_parallel_tool_calls.load(std::memory_order_relaxed)) {
        request.parallel_tool_calls = std::nullopt;
    }
    if(!caps.supports_tools.load(std::memory_order_relaxed)) {
        request.tools.clear();
    }
    return request;
}

auto is_feature_rejection_error(std::string_view error_message) -> bool {
    constexpr static std::string_view patterns[] = {
        "unsupported parameter",
        "unknown field",
        "unexpected field",
        "invalid field",
        "does not support",
        "not supported",
        "unrecognized field",
        "invalid parameter",
    };
    auto lower = std::string(error_message);
    std::ranges::transform(lower, lower.begin(), ::tolower);
    for(auto pattern: patterns) {
        if(lower.find(pattern) != std::string::npos) {
            return true;
        }
    }
    return false;
}

auto parse_rejected_feature_from_error(std::string_view error_message)
    -> std::optional<std::string> {
    constexpr static std::pair<std::string_view, std::string_view> field_patterns[] = {
        {"response_format",     "response_format"    },
        {"json_schema",         "response_format"    },
        {"schema",              "response_format"    },
        {"tool_choice",         "tool_choice"        },
        {"parallel_tool_calls", "parallel_tool_calls"},
        {"tools",               "tools"              },
        {"functions",           "tools"              },
        {"temperature",         "temperature"        },
        {"max_tokens",          "max_tokens"         },
    };

    auto lower = std::string(error_message);
    std::ranges::transform(lower, lower.begin(), ::tolower);

    for(const auto& [keyword, field]: field_patterns) {
        if(lower.find(keyword) != std::string::npos) {
            return std::string(field);
        }
    }
    return std::nullopt;
}

}  // namespace clore::net
