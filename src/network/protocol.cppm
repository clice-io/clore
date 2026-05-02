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

auto get_probed_capabilities(std::string_view cache_key) -> ProbedCapabilities&;

auto make_capability_probe_key(std::string_view provider,
                               std::string_view api_base,
                               std::string_view model) -> std::string;

auto sanitize_request_for_capabilities(CompletionRequest request, const ProbedCapabilities& caps)
    -> CompletionRequest;

auto is_feature_rejection_error(std::string_view error_message) -> bool;

auto parse_rejected_feature_from_error(std::string_view error_message)
    -> std::optional<std::string>;

}  // namespace clore::net

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

struct ObjectView {
    const kota::codec::json::Object* value = nullptr;

    auto get(std::string_view key) const -> std::optional<kota::codec::json::Cursor>;

    auto begin() const noexcept {
        return value->begin();
    }

    auto end() const noexcept {
        return value->end();
    }

    auto operator->() const noexcept -> const kota::codec::json::Object* {
        return value;
    }

    auto operator*() const noexcept -> const kota::codec::json::Object& {
        return *value;
    }
};

struct ArrayView {
    const kota::codec::json::Array* value = nullptr;

    auto empty() const noexcept -> bool {
        return value->empty();
    }

    auto size() const noexcept -> std::size_t {
        return value->size();
    }

    auto begin() const noexcept {
        return value->begin();
    }

    auto end() const noexcept {
        return value->end();
    }

    auto operator[](std::size_t index) const -> const kota::codec::json::Value& {
        return (*value)[index];
    }

    auto operator->() const noexcept -> const kota::codec::json::Array* {
        return value;
    }

    auto operator*() const noexcept -> const kota::codec::json::Array& {
        return *value;
    }
};

auto unexpected_json_error(std::string_view context, const kota::codec::json::error& err)
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

auto expect_object(const kota::codec::json::Value& value, std::string_view context)
    -> std::expected<ObjectView, LLMError>;

auto expect_object(kota::codec::json::Cursor value, std::string_view context)
    -> std::expected<ObjectView, LLMError>;

auto expect_array(const kota::codec::json::Value& value, std::string_view context)
    -> std::expected<ArrayView, LLMError>;

auto expect_array(kota::codec::json::Cursor value, std::string_view context)
    -> std::expected<ArrayView, LLMError>;

auto expect_string(const kota::codec::json::Value& value, std::string_view context)
    -> std::expected<std::string_view, LLMError>;

auto expect_string(kota::codec::json::Cursor value, std::string_view context)
    -> std::expected<std::string_view, LLMError>;

auto clone_object(const kota::codec::json::Object& source, std::string_view context)
    -> std::expected<kota::codec::json::Object, LLMError>;

auto clone_object(ObjectView source, std::string_view context)
    -> std::expected<kota::codec::json::Object, LLMError>;

auto clone_array(ArrayView source, std::string_view context)
    -> std::expected<kota::codec::json::Array, LLMError>;

auto clone_value(const kota::codec::json::Value& source, std::string_view context)
    -> std::expected<kota::codec::json::Value, LLMError>;

}  // namespace clore::net::detail

namespace clore::net::detail {

namespace json = kota::codec::json;

auto ObjectView::get(std::string_view key) const -> std::optional<json::Cursor> {
    auto* item = value->find(key);
    if(item == nullptr) {
        return std::nullopt;
    }
    return item->cursor();
}

auto unexpected_json_error(std::string_view context, const json::error& err)
    -> std::unexpected<LLMError> {
    return std::unexpected(LLMError(std::format("{}: {}", context, err.to_string())));
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
    object.insert(std::string(key), std::string(value));
    return {};
}

template <typename Status>
auto to_llm_unexpected(Status error, std::string_view context) -> std::unexpected<LLMError> {
    return std::unexpected(LLMError(std::format("{}: {}", context, error.to_string())));
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
    auto parsed = json::parse<json::Object>("{}");
    if(!parsed.has_value()) {
        return unexpected_json_error(context, parsed.error());
    }
    return *parsed;
}

auto make_empty_array(std::string_view context) -> std::expected<json::Array, LLMError> {
    auto parsed = json::parse<json::Array>("[]");
    if(!parsed.has_value()) {
        return unexpected_json_error(context, parsed.error());
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
    auto raw = json::to_string(value);
    if(!raw.has_value()) {
        return unexpected_json_error(std::format("failed to serialize {} for parsing", context),
                                     raw.error());
    }
    return parse_json_value<T>(*raw, context);
}

auto serialize_value_to_string(const json::Value& value, std::string_view context)
    -> std::expected<std::string, LLMError> {
    auto encoded = json::to_string(value);
    if(!encoded.has_value()) {
        return unexpected_json_error(std::format("failed to serialize {}", context),
                                     encoded.error());
    }
    return *encoded;
}

auto expect_object(const json::Value& value, std::string_view context)
    -> std::expected<ObjectView, LLMError> {
    auto* object = value.get_object();
    if(object == nullptr) {
        return std::unexpected(LLMError(std::format("{} is not a JSON object", context)));
    }
    return ObjectView{.value = object};
}

auto expect_object(json::Cursor value, std::string_view context)
    -> std::expected<ObjectView, LLMError> {
    auto* object = value.get_object();
    if(object == nullptr) {
        return std::unexpected(LLMError(std::format("{} is not a JSON object", context)));
    }
    return ObjectView{.value = object};
}

auto expect_array(const json::Value& value, std::string_view context)
    -> std::expected<ArrayView, LLMError> {
    auto* array = value.get_array();
    if(array == nullptr) {
        return std::unexpected(LLMError(std::format("{} is not a JSON array", context)));
    }
    return ArrayView{.value = array};
}

auto expect_array(json::Cursor value, std::string_view context)
    -> std::expected<ArrayView, LLMError> {
    auto* array = value.get_array();
    if(array == nullptr) {
        return std::unexpected(LLMError(std::format("{} is not a JSON array", context)));
    }
    return ArrayView{.value = array};
}

auto expect_string(const json::Value& value, std::string_view context)
    -> std::expected<std::string_view, LLMError> {
    auto text = value.get_string();
    if(!text.has_value()) {
        return std::unexpected(LLMError(std::format("{} is not a JSON string", context)));
    }
    return *text;
}

auto expect_string(json::Cursor value, std::string_view context)
    -> std::expected<std::string_view, LLMError> {
    auto text = value.get_string();
    if(!text.has_value()) {
        return std::unexpected(LLMError(std::format("{} is not a JSON string", context)));
    }
    return *text;
}

auto clone_array(ArrayView source, std::string_view) -> std::expected<json::Array, LLMError> {
    return json::Array(*source.value);
}

auto clone_object(const json::Object& source, std::string_view)
    -> std::expected<json::Object, LLMError> {
    return json::Object(source);
}

auto clone_object(ObjectView source, std::string_view) -> std::expected<json::Object, LLMError> {
    return json::Object(*source.value);
}

auto clone_value(const json::Value& source, std::string_view context)
    -> std::expected<json::Value, LLMError> {
    static_cast<void>(context);
    return json::Value(source);
}

}  // namespace clore::net::detail

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
    auto parsed = kota::codec::json::parse<kota::codec::json::Value>(content);
    if(!parsed.has_value()) {
        return std::unexpected(
            LLMError(std::format("LLM output is not valid JSON: {}", parsed.error().to_string())));
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
    auto encoded = kota::codec::json::to_string(call.arguments);
    if(!encoded.has_value()) {
        return std::unexpected(
            LLMError(std::format("failed to serialize tool arguments for '{}': {}",
                                 call.name,
                                 encoded.error().to_string())));
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

// ── prompt output handling
// ──────────────────────────────────────────────────────────
export namespace clore::net::detail {

auto infer_output_contract(const PromptRequest& request)
    -> std::expected<PromptOutputContract, LLMError>;

auto validate_prompt_output(std::string_view content, PromptOutputContract contract)
    -> std::expected<void, LLMError>;

template <typename CompletionRequester>
auto request_text_once_async(CompletionRequester request_completion,
                             std::string_view model,
                             std::string_view system_prompt,
                             PromptRequest request,
                             kota::event_loop& loop) -> kota::task<std::string, LLMError>;

}  // namespace clore::net::detail

namespace clore::net::detail {

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

template <typename CompletionRequester>
auto request_text_once_async(CompletionRequester request_completion,
                             std::string_view model,
                             std::string_view system_prompt,
                             PromptRequest request,
                             kota::event_loop& loop) -> kota::task<std::string, LLMError> {
    auto contract = infer_output_contract(request);
    if(!contract.has_value()) {
        co_await kota::fail(std::move(contract.error()));
    }

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
    auto validation = validate_prompt_output(*text, *contract);
    if(!validation.has_value()) {
        co_await kota::fail(std::move(validation.error()));
    }

    co_return std::move(*text);
}

}  // namespace clore::net::detail

// ── provider capability probing
// ──────────────────────────────────────────────────────
namespace clore::net {

auto get_probed_capabilities(std::string_view cache_key) -> ProbedCapabilities& {
    static std::mutex mutex;
    static std::unordered_map<std::string, std::unique_ptr<ProbedCapabilities>> cache;

    std::lock_guard lock(mutex);
    auto it = cache.find(std::string(cache_key));
    if(it != cache.end()) {
        return *it->second;
    }
    auto [inserted_it, _] =
        cache.emplace(std::string(cache_key), std::make_unique<ProbedCapabilities>());
    return *inserted_it->second;
}

auto make_capability_probe_key(std::string_view provider,
                               std::string_view api_base,
                               std::string_view model) -> std::string {
    return std::format("{}|{}|{}", provider, api_base, model);
}

auto sanitize_request_for_capabilities(CompletionRequest request, const ProbedCapabilities& caps)
    -> CompletionRequest {
    if(!caps.supports_json_schema.load(std::memory_order_relaxed)) {
        if(request.response_format.has_value() && request.response_format->schema.has_value()) {
            request.response_format->schema = std::nullopt;
        }
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

auto icontains(std::string_view haystack, std::string_view needle) -> bool {
    if(needle.size() > haystack.size()) {
        return false;
    }
    for(std::size_t i = 0; i <= haystack.size() - needle.size(); ++i) {
        bool match = true;
        for(std::size_t j = 0; j < needle.size(); ++j) {
            if(std::tolower(static_cast<unsigned char>(haystack[i + j])) !=
               std::tolower(static_cast<unsigned char>(needle[j]))) {
                match = false;
                break;
            }
        }
        if(match) {
            return true;
        }
    }
    return false;
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
    for(auto pattern: patterns) {
        if(icontains(error_message, pattern)) {
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

    for(const auto& [keyword, field]: field_patterns) {
        if(icontains(error_message, keyword)) {
            return std::string(field);
        }
    }
    return std::nullopt;
}

}  // namespace clore::net
