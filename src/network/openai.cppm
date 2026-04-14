module;

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <array>
#include <chrono>
#include <condition_variable>
#include <concepts>
#include <cstdint>
#include <deque>
#include <expected>
#include <format>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "eventide/async/async.h"
#include "eventide/reflection/name.h"
#include "eventide/reflection/struct.h"
#include "eventide/serde/json/error.h"
#include "eventide/serde/json/json.h"
#include "eventide/serde/serde/attrs.h"
#include "eventide/serde/serde/attrs/schema.h"

export module network:openai;

import :async;
import :http;
import support;

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
    eventide::serde::json::Value arguments;
};

struct AssistantToolCallMessage {
    std::optional<std::string> content;
    std::vector<ToolCall> tool_calls;
};

struct ToolResultMessage {
    std::string tool_call_id;
    std::string content;
};

using Message = std::variant<SystemMessage, UserMessage,
                             AssistantToolCallMessage, ToolResultMessage>;

struct ResponseFormat {
    std::string name;
    eventide::serde::json::Object schema;
    bool strict = true;
};

struct FunctionToolDefinition {
    std::string name;
    std::string description;
    eventide::serde::json::Object parameters;
    bool strict = true;
};

struct ToolChoiceAuto {};
struct ToolChoiceRequired {};
struct ToolChoiceNone {};

struct ForcedFunctionToolChoice {
    std::string name;
};

using ToolChoice = std::variant<ToolChoiceAuto, ToolChoiceRequired,
                                ToolChoiceNone, ForcedFunctionToolChoice>;

struct CompletionRequest {
    std::string model;
    std::vector<Message> messages;
    std::optional<ResponseFormat> response_format;
    std::vector<FunctionToolDefinition> tools;
    std::optional<ToolChoice> tool_choice;
    std::optional<bool> parallel_tool_calls;
};

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

namespace schema {

template <typename T>
auto response_format() -> std::expected<ResponseFormat, LLMError>;

template <typename T>
auto function_tool(std::string name, std::string description)
    -> std::expected<FunctionToolDefinition, LLMError>;

}  // namespace schema

namespace protocol {

auto build_request_json(const CompletionRequest& request)
    -> std::expected<std::string, LLMError>;

auto parse_response(std::string_view json)
    -> std::expected<CompletionResponse, LLMError>;

auto text_from_response(const CompletionResponse& response)
    -> std::expected<std::string, LLMError>;

auto append_tool_outputs(std::span<const Message> history,
                         const CompletionResponse& response,
                         std::span<const ToolOutput> outputs)
    -> std::expected<std::vector<Message>, LLMError>;

template <typename T>
auto parse_response_text(const CompletionResponse& response)
    -> std::expected<T, LLMError>;

template <typename T>
auto parse_tool_arguments(const ToolCall& call)
    -> std::expected<T, LLMError>;

}  // namespace protocol

auto call_completion_async(CompletionRequest request,
                           eventide::event_loop& loop = eventide::event_loop::current())
    -> eventide::task<CompletionResponse, LLMError>;

auto call_completion(CompletionRequest request)
    -> std::expected<CompletionResponse, LLMError>;

auto call_llm_async(std::string_view model,
                    std::string_view system_prompt,
                    std::string_view prompt,
                    eventide::event_loop& loop = eventide::event_loop::current())
    -> eventide::task<std::string, LLMError>;

auto call_llm(std::string_view model,
              std::string_view system_prompt,
              std::string_view prompt)
    -> std::expected<std::string, LLMError>;

auto call_llm_with_retries(std::string_view model,
                           std::string_view system_prompt,
                           std::string_view prompt,
                           std::uint32_t retry_count,
                           std::uint32_t retry_initial_backoff_ms)
    -> std::expected<std::string, LLMError>;

template <typename T>
auto call_structured_async(std::string_view model,
                           std::string_view system_prompt,
                           std::string_view prompt,
                           eventide::event_loop& loop = eventide::event_loop::current())
    -> eventide::task<T, LLMError>;

template <typename T>
auto call_structured(std::string_view model,
                     std::string_view system_prompt,
                     std::string_view prompt)
    -> std::expected<T, LLMError>;

class LLMClient {
public:
    using Callback = std::function<void(std::uint64_t tag,
                                        std::expected<std::string, LLMError> result)>;

    LLMClient(std::string_view model, std::string_view system_prompt,
              std::uint32_t max_concurrent,
              std::uint32_t retry_count,
              std::uint32_t retry_initial_backoff_ms);

    auto submit(std::uint64_t tag, std::string prompt)
        -> std::expected<void, LLMError>;

    auto request_stop() noexcept -> void;

    auto run(Callback on_complete) -> std::expected<void, LLMError>;

private:
    struct PendingRequest {
        std::uint64_t tag = 0;
        std::string prompt;
    };

    struct CompletedRequest {
        std::uint64_t tag = 0;
        std::expected<std::string, LLMError> result = std::unexpected(LLMError("request not completed"));
    };

    struct RunState {
        std::mutex mutex;
        std::condition_variable condition;
        std::deque<PendingRequest> pending;
        std::deque<CompletedRequest> completed;
        std::size_t in_flight = 0;
        bool stop_requested = false;
        bool shutdown = false;
    };

    auto compute_retry_delay(std::uint32_t next_attempt) const
        -> std::expected<std::chrono::milliseconds, LLMError>;

    std::string model_;
    std::string system_prompt_;
    std::uint32_t max_concurrent_ = 0;
    std::uint32_t retry_count_ = 0;
    std::uint32_t retry_initial_backoff_ms_ = 0;
    std::deque<PendingRequest> pending_;
    std::mutex active_run_mutex_;
    std::shared_ptr<RunState> active_run_;
    std::optional<LLMError> configuration_error_;
};

}  // namespace clore::net

namespace clore::net::detail {

namespace json = eventide::serde::json;
namespace serde = eventide::serde;
namespace async = eventide;

template <typename T>
using remove_cvref_t = std::remove_cvref_t<T>;

template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template <typename T>
constexpr bool is_optional_v = is_optional<remove_cvref_t<T>>::value;

template <typename T>
struct optional_inner;

template <typename T>
struct optional_inner<std::optional<T>> {
    using type = T;
};

template <typename T>
using optional_inner_t = typename optional_inner<remove_cvref_t<T>>::type;

template <typename T>
struct is_vector : std::false_type {};

template <typename T, typename Allocator>
struct is_vector<std::vector<T, Allocator>> : std::true_type {};

template <typename T>
constexpr bool is_vector_v = is_vector<remove_cvref_t<T>>::value;

template <typename T>
struct vector_inner;

template <typename T, typename Allocator>
struct vector_inner<std::vector<T, Allocator>> {
    using type = T;
};

template <typename T>
using vector_inner_t = typename vector_inner<remove_cvref_t<T>>::type;

template <typename T>
struct is_array : std::false_type {};

template <typename T, std::size_t N>
struct is_array<std::array<T, N>> : std::true_type {};

template <typename T>
constexpr bool is_array_v = is_array<remove_cvref_t<T>>::value;

template <typename T>
struct array_inner;

template <typename T, std::size_t N>
struct array_inner<std::array<T, N>> {
    using type = T;
};

template <typename T>
using array_inner_t = typename array_inner<remove_cvref_t<T>>::type;

template <typename T>
struct schema_subject {
    using type = remove_cvref_t<T>;
};

template <typename T>
    requires serde::annotated_type<remove_cvref_t<T>> &&
             (!eventide::refl::reflectable_class<remove_cvref_t<T>>)
struct schema_subject<T> {
    using type = serde::annotated_underlying_t<remove_cvref_t<T>>;
};

template <typename T>
using schema_subject_t = typename schema_subject<T>::type;

auto normalize_utf8(std::string_view text, std::string_view field_name)
    -> std::string {
    auto normalized = clore::support::ensure_utf8(text);
    if(normalized != text) {
        logging::warn(
            "LLM {} contains invalid UTF-8; replaced invalid byte sequences before JSON serialization",
            field_name);
    }
    return normalized;
}

auto insert_string_field(json::Object& object,
                         std::string_view key,
                         std::string_view value,
                         std::string_view context)
    -> std::expected<void, LLMError> {
    auto status = object.insert(key, value);
    if(!status.has_value()) {
        return std::unexpected(LLMError(std::format(
            "{}: {}", context, json::error_message(status.error()))));
    }
    return {};
}

template <typename Status>
auto to_llm_unexpected(Status error, std::string_view context)
    -> std::unexpected<LLMError> {
    return std::unexpected(LLMError(
        std::format("{}: {}", context, json::error_message(error))));
}

auto excerpt_for_error(std::string_view body) -> std::string {
    constexpr std::size_t kMaxBytes = 200;
    return clore::support::truncate_utf8(body, kMaxBytes);
}

template <typename T>
auto run_task_sync(auto&& make_task) -> std::expected<T, LLMError> {
    async::event_loop loop;
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

auto sanitize_schema_name(std::string_view raw_name) -> std::string {
    std::string sanitized;
    sanitized.reserve(raw_name.size());
    for(char ch : raw_name) {
        auto unsigned_ch = static_cast<unsigned char>(ch);
        if((unsigned_ch >= 'a' && unsigned_ch <= 'z') ||
           (unsigned_ch >= 'A' && unsigned_ch <= 'Z') ||
           (unsigned_ch >= '0' && unsigned_ch <= '9')) {
            sanitized.push_back(ch);
        } else {
            sanitized.push_back('_');
        }
    }
    while(!sanitized.empty() && sanitized.front() == '_') {
        sanitized.erase(sanitized.begin());
    }
    while(!sanitized.empty() && sanitized.back() == '_') {
        sanitized.pop_back();
    }
    return sanitized;
}

template <typename T>
auto schema_type_name() -> std::expected<std::string, LLMError> {
    auto sanitized = sanitize_schema_name(eventide::refl::type_name<T>());
    if(sanitized.empty()) {
        return std::unexpected(LLMError("generated schema name is empty"));
    }
    return sanitized;
}

auto make_empty_object(std::string_view context) -> std::expected<json::Object, LLMError> {
    auto parsed = json::Object::parse("{}");
    if(!parsed.has_value()) {
        return std::unexpected(LLMError(std::format(
            "{}: {}", context,
            json::error_message(json::make_read_error(parsed.error())))));
    }
    return *parsed;
}

auto make_empty_array(std::string_view context) -> std::expected<json::Array, LLMError> {
    auto parsed = json::Array::parse("[]");
    if(!parsed.has_value()) {
        return std::unexpected(LLMError(std::format(
            "{}: {}", context,
            json::error_message(json::make_read_error(parsed.error())))));
    }
    return *parsed;
}

template <typename T>
auto make_schema_value(json::Document& document) -> std::expected<json::Value, LLMError>;

template <typename T>
auto make_schema_object() -> std::expected<json::Object, LLMError> {
    json::Document document;
    auto value = make_schema_value<T>(document);
    if(!value.has_value()) {
        return std::unexpected(std::move(value.error()));
    }

    auto object = value->get_object();
    if(!object.has_value()) {
        return std::unexpected(LLMError("generated schema root is not an object"));
    }
    return std::move(*object);
}

template <typename T>
auto make_scalar_type_schema(json::Document& document, std::string_view type_name)
    -> std::expected<json::Value, LLMError> {
    auto object = make_empty_object("failed to create scalar schema object");
    if(!object.has_value()) {
        return std::unexpected(std::move(object.error()));
    }
    auto status = object->insert("type", type_name);
    if(!status.has_value()) {
        return to_llm_unexpected(status.error(), "failed to build scalar schema");
    }
    return object->as_value();
}

template <typename T>
auto make_any_of_schema(json::Document& document, std::vector<json::Value> choices)
    -> std::expected<json::Value, LLMError> {
    auto object = make_empty_object("failed to create anyOf schema object");
    if(!object.has_value()) {
        return std::unexpected(std::move(object.error()));
    }
    auto any_of = make_empty_array("failed to create anyOf schema array");
    if(!any_of.has_value()) {
        return std::unexpected(std::move(any_of.error()));
    }
    for(auto& choice : choices) {
        auto push_status = any_of->push_back(std::move(choice));
        if(!push_status.has_value()) {
            return to_llm_unexpected(push_status.error(),
                                     "failed to append anyOf schema choice");
        }
    }
    auto status = object->insert("anyOf", std::move(*any_of));
    if(!status.has_value()) {
        return to_llm_unexpected(status.error(), "failed to build anyOf schema");
    }
    return object->as_value();
}

template <typename Object, std::size_t... Indices>
auto populate_object_schema(json::Document& document,
                            json::Object& object,
                            std::index_sequence<Indices...>)
    -> std::expected<void, LLMError> {
    constexpr bool fields_valid = serde::schema::validate_field_schema<Object>();
    static_assert(fields_valid, "schema field names contain collisions or alias conflicts");

    auto properties = make_empty_object("failed to create schema properties object");
    if(!properties.has_value()) {
        return std::unexpected(std::move(properties.error()));
    }
    auto required = make_empty_array("failed to create schema required array");
    if(!required.has_value()) {
        return std::unexpected(std::move(required.error()));
    }

    auto append_field = [&](auto index_constant) -> std::expected<void, LLMError> {
        constexpr std::size_t index = decltype(index_constant)::value;
        constexpr auto field_schema = serde::schema::resolve_field<Object, index>();
        if constexpr(field_schema.is_skipped) {
            return {};
        } else if constexpr(field_schema.is_flattened) {
            return std::unexpected(
                LLMError("flatten fields are not supported by automatic OpenAI schema generation"));
        } else {
            using field_type = eventide::refl::field_type<Object, index>;
            auto field_value = make_schema_value<field_type>(document);
            if(!field_value.has_value()) {
                return std::unexpected(std::move(field_value.error()));
            }

            auto property_status = properties->insert(field_schema.canonical_name,
                                                      std::move(*field_value));
            if(!property_status.has_value()) {
                return to_llm_unexpected(property_status.error(),
                    std::format("failed to add schema property '{}'",
                                field_schema.canonical_name));
            }

            auto required_status = required->push_back(field_schema.canonical_name);
            if(!required_status.has_value()) {
                return to_llm_unexpected(required_status.error(),
                    std::format("failed to add required field '{}'",
                                field_schema.canonical_name));
            }
            return {};
        }
    };

    auto statuses = std::array<std::expected<void, LLMError>, sizeof...(Indices)>{
        append_field(std::integral_constant<std::size_t, Indices>{})...
    };
    for(auto& status : statuses) {
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
    }

    if(auto status = object.insert("type", "object"); !status.has_value()) {
        return to_llm_unexpected(status.error(), "failed to set object schema type");
    }
    if(auto status = object.insert("properties", std::move(*properties)); !status.has_value()) {
        return to_llm_unexpected(status.error(), "failed to set object schema properties");
    }
    if(auto status = object.insert("required", std::move(*required)); !status.has_value()) {
        return to_llm_unexpected(status.error(), "failed to set object schema required keys");
    }
    if(auto status = object.insert("additionalProperties", false); !status.has_value()) {
        return to_llm_unexpected(status.error(),
                                 "failed to set object schema additionalProperties");
    }
    return {};
}

template <typename T>
auto make_schema_value(json::Document& document) -> std::expected<json::Value, LLMError> {
    using schema_type = schema_subject_t<T>;

    if constexpr(std::same_as<schema_type, std::string> ||
                 std::same_as<schema_type, std::string_view>) {
        return make_scalar_type_schema<schema_type>(document, "string");
    } else if constexpr(std::same_as<schema_type, bool>) {
        return make_scalar_type_schema<schema_type>(document, "boolean");
    } else if constexpr(std::integral<schema_type> && !std::same_as<schema_type, bool>) {
        return make_scalar_type_schema<schema_type>(document, "integer");
    } else if constexpr(std::floating_point<schema_type>) {
        return make_scalar_type_schema<schema_type>(document, "number");
    } else if constexpr(is_optional_v<schema_type>) {
        auto inner = make_schema_value<optional_inner_t<schema_type>>(document);
        if(!inner.has_value()) {
            return std::unexpected(std::move(inner.error()));
        }
        auto null_value = make_scalar_type_schema<std::nullptr_t>(document, "null");
        if(!null_value.has_value()) {
            return std::unexpected(std::move(null_value.error()));
        }
        return make_any_of_schema<schema_type>(document,
                                               {std::move(*inner), std::move(*null_value)});
    } else if constexpr(is_vector_v<schema_type>) {
        auto item_schema = make_schema_value<vector_inner_t<schema_type>>(document);
        if(!item_schema.has_value()) {
            return std::unexpected(std::move(item_schema.error()));
        }
        auto object = make_empty_object("failed to create array schema object");
        if(!object.has_value()) {
            return std::unexpected(std::move(object.error()));
        }
        if(auto status = object->insert("type", "array"); !status.has_value()) {
            return to_llm_unexpected(status.error(), "failed to set array schema type");
        }
        if(auto status = object->insert("items", std::move(*item_schema)); !status.has_value()) {
            return to_llm_unexpected(status.error(), "failed to set array schema items");
        }
        return object->as_value();
    } else if constexpr(is_array_v<schema_type>) {
        constexpr auto fixed_size = std::tuple_size_v<schema_type>;
        auto item_schema = make_schema_value<array_inner_t<schema_type>>(document);
        if(!item_schema.has_value()) {
            return std::unexpected(std::move(item_schema.error()));
        }
        auto object = make_empty_object("failed to create fixed array schema object");
        if(!object.has_value()) {
            return std::unexpected(std::move(object.error()));
        }
        if(auto status = object->insert("type", "array"); !status.has_value()) {
            return to_llm_unexpected(status.error(), "failed to set fixed array schema type");
        }
        if(auto status = object->insert("items", std::move(*item_schema)); !status.has_value()) {
            return to_llm_unexpected(status.error(), "failed to set fixed array schema items");
        }
        if(auto status = object->insert("minItems", static_cast<std::uint64_t>(fixed_size));
           !status.has_value()) {
            return to_llm_unexpected(status.error(), "failed to set fixed array schema minItems");
        }
        if(auto status = object->insert("maxItems", static_cast<std::uint64_t>(fixed_size));
           !status.has_value()) {
            return to_llm_unexpected(status.error(), "failed to set fixed array schema maxItems");
        }
        return object->as_value();
    } else if constexpr(eventide::refl::reflectable_class<schema_type>) {
        auto object = make_empty_object("failed to create object schema root");
        if(!object.has_value()) {
            return std::unexpected(std::move(object.error()));
        }
        auto status = populate_object_schema<schema_type>(
            document,
            *object,
            std::make_index_sequence<eventide::refl::field_count<schema_type>()>{});
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
        return object->as_value();
    } else {
        static_assert(sizeof(schema_type) == 0,
                      "automatic OpenAI schema generation only supports reflectable objects, strings, booleans, numbers, std::optional, std::vector, and std::array");
    }
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

auto validate_schema_array_of_types(json::ArrayRef array,
                                    std::string_view path,
                                    bool is_root)
    -> std::expected<void, LLMError> {
    std::optional<std::string_view> primary_type;
    bool saw_null = false;
    for(auto value : array) {
        auto type = expect_string(value, std::format("{}.type[]", path));
        if(!type.has_value()) {
            return std::unexpected(std::move(type.error()));
        }
        if(*type == "null") {
            saw_null = true;
            continue;
        }
        if(primary_type.has_value()) {
            return std::unexpected(LLMError(std::format(
                "{} uses an unsupported multi-type union; only a single non-null type plus null is supported",
                path)));
        }
        primary_type = *type;
    }

    if(is_root) {
        return std::unexpected(
            LLMError("root schema must be an object and cannot be nullable"));
    }

    if(!primary_type.has_value() || !saw_null) {
        return std::unexpected(LLMError(std::format(
            "{} type union must contain exactly one concrete type and 'null'", path)));
    }

    return {};
}

auto validate_openai_schema(json::ObjectRef object,
                            std::string_view path,
                            bool is_root)
    -> std::expected<void, LLMError>;

auto validate_openai_schema_value(json::ValueRef value,
                                  std::string_view path,
                                  bool is_root)
    -> std::expected<void, LLMError> {
    auto object = expect_object(value, path);
    if(!object.has_value()) {
        return std::unexpected(std::move(object.error()));
    }
    return validate_openai_schema(*object, path, is_root);
}

auto validate_required_properties(json::ObjectRef properties,
                                  json::ArrayRef required,
                                  std::string_view path)
    -> std::expected<void, LLMError> {
    std::unordered_set<std::string> required_names;
    for(auto value : required) {
        auto name = expect_string(value, std::format("{}.required[]", path));
        if(!name.has_value()) {
            return std::unexpected(std::move(name.error()));
        }
        required_names.emplace(*name);
    }

    for(auto entry : properties) {
        if(!required_names.contains(std::string(entry.key))) {
            return std::unexpected(LLMError(std::format(
                "{} property '{}' must be listed in required when using strict structured output",
                path, entry.key)));
        }
    }

    return {};
}

auto validate_openai_schema(json::ObjectRef object,
                            std::string_view path,
                            bool is_root)
    -> std::expected<void, LLMError> {
    auto any_of_value = object.get("anyOf");
    if(any_of_value.has_value()) {
        if(is_root) {
            return std::unexpected(LLMError("root schema must not use anyOf"));
        }
        auto any_of = expect_array(*any_of_value, std::format("{}.anyOf", path));
        if(!any_of.has_value()) {
            return std::unexpected(std::move(any_of.error()));
        }
        if(any_of->empty()) {
            return std::unexpected(
                LLMError(std::format("{}.anyOf must contain at least one schema", path)));
        }
        for(std::size_t index = 0; index < any_of->size(); ++index) {
            auto status = validate_openai_schema_value((*any_of)[index],
                std::format("{}.anyOf[{}]", path, index), false);
            if(!status.has_value()) {
                return std::unexpected(std::move(status.error()));
            }
        }
        return {};
    }

    auto type_value = object.get("type");
    if(!type_value.has_value()) {
        return std::unexpected(
            LLMError(std::format("{} schema is missing a 'type' field", path)));
    }

    std::optional<std::string_view> schema_type;
    if(auto type_string = type_value->get_string(); type_string.has_value()) {
        schema_type = *type_string;
    } else if(auto type_array = type_value->get_array(); type_array.has_value()) {
        auto status = validate_schema_array_of_types(*type_array, path, is_root);
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
        for(auto value : *type_array) {
            auto type_name = value.get_string();
            if(type_name.has_value() && *type_name != "null") {
                schema_type = *type_name;
                break;
            }
        }
    } else {
        return std::unexpected(LLMError(
            std::format("{}.type must be a string or string array", path)));
    }

    if(!schema_type.has_value()) {
        return std::unexpected(LLMError(
            std::format("{} schema type could not be resolved", path)));
    }

    if(is_root && *schema_type != "object") {
        return std::unexpected(LLMError("root schema must be an object"));
    }

    if(*schema_type == "object") {
        auto properties_value = object.get("properties");
        if(!properties_value.has_value()) {
            return std::unexpected(
                LLMError(std::format("{} object schema is missing properties", path)));
        }
        auto properties = expect_object(*properties_value,
                                        std::format("{}.properties", path));
        if(!properties.has_value()) {
            return std::unexpected(std::move(properties.error()));
        }

        auto required_value = object.get("required");
        if(!required_value.has_value()) {
            return std::unexpected(
                LLMError(std::format("{} object schema is missing required", path)));
        }
        auto required = expect_array(*required_value,
                                     std::format("{}.required", path));
        if(!required.has_value()) {
            return std::unexpected(std::move(required.error()));
        }

        auto additional_properties_value = object.get("additionalProperties");
        if(!additional_properties_value.has_value()) {
            return std::unexpected(LLMError(std::format(
                "{} object schema must set additionalProperties to false", path)));
        }
        auto additional_properties = additional_properties_value->get_bool();
        if(!additional_properties.has_value() || *additional_properties) {
            return std::unexpected(LLMError(std::format(
                "{} object schema must set additionalProperties to false", path)));
        }

        auto required_status = validate_required_properties(*properties, *required, path);
        if(!required_status.has_value()) {
            return std::unexpected(std::move(required_status.error()));
        }

        for(auto entry : *properties) {
            auto status = validate_openai_schema_value(
                entry.value,
                std::format("{}.properties.{}", path, entry.key),
                false);
            if(!status.has_value()) {
                return std::unexpected(std::move(status.error()));
            }
        }
    } else if(*schema_type == "array") {
        auto items_value = object.get("items");
        if(!items_value.has_value()) {
            return std::unexpected(
                LLMError(std::format("{} array schema is missing items", path)));
        }
        auto status = validate_openai_schema_value(*items_value,
            std::format("{}.items", path), false);
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
    }

    if(auto defs_value = object.get("$defs"); defs_value.has_value()) {
        auto defs = expect_object(*defs_value, std::format("{}.$defs", path));
        if(!defs.has_value()) {
            return std::unexpected(std::move(defs.error()));
        }
        for(auto entry : *defs) {
            auto status = validate_openai_schema_value(
                entry.value,
                std::format("{}.$defs.{}", path, entry.key),
                false);
            if(!status.has_value()) {
                return std::unexpected(std::move(status.error()));
            }
        }
    }

    return {};
}

auto validate_response_format(const ResponseFormat& format)
    -> std::expected<void, LLMError> {
    if(format.name.empty()) {
        return std::unexpected(LLMError("response_format.name must not be empty"));
    }
    return validate_openai_schema(format.schema, format.name, true);
}

auto validate_tool_definition(const FunctionToolDefinition& tool)
    -> std::expected<void, LLMError> {
    if(tool.name.empty()) {
        return std::unexpected(LLMError("tool name must not be empty"));
    }
    if(tool.description.empty()) {
        return std::unexpected(
            LLMError(std::format("tool '{}' description must not be empty", tool.name)));
    }
    return validate_openai_schema(tool.parameters, tool.name, true);
}

auto validate_request(const CompletionRequest& request)
    -> std::expected<void, LLMError> {
    if(request.model.empty()) {
        return std::unexpected(LLMError("request model must not be empty"));
    }
    if(request.messages.empty()) {
        return std::unexpected(LLMError("request messages must not be empty"));
    }

    if(request.response_format.has_value()) {
        auto status = validate_response_format(*request.response_format);
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
    }

    for(auto& tool : request.tools) {
        auto status = validate_tool_definition(tool);
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
    }

    if((request.tool_choice.has_value() || request.parallel_tool_calls.has_value()) &&
       request.tools.empty()) {
        return std::unexpected(
            LLMError("tool_choice and parallel_tool_calls require at least one tool"));
    }

    if(request.tool_choice.has_value()) {
        if(auto forced = std::get_if<ForcedFunctionToolChoice>(&*request.tool_choice)) {
            bool exists = false;
            for(auto& tool : request.tools) {
                if(tool.name == forced->name) {
                    exists = true;
                    break;
                }
            }
            if(!exists) {
                return std::unexpected(LLMError(std::format(
                    "forced tool '{}' is not present in request.tools", forced->name)));
            }
        }
    }

    for(auto& message : request.messages) {
        auto status = std::visit([](const auto& current)
            -> std::expected<void, LLMError> {
            using message_type = remove_cvref_t<decltype(current)>;
            if constexpr(std::same_as<message_type, AssistantToolCallMessage>) {
                if(!current.content.has_value() && current.tool_calls.empty()) {
                    return std::unexpected(LLMError(
                        "assistant tool-call message must contain content or tool_calls"));
                }
                std::unordered_set<std::string> ids;
                for(auto& call : current.tool_calls) {
                    if(call.id.empty()) {
                        return std::unexpected(LLMError("tool call id must not be empty"));
                    }
                    if(call.name.empty()) {
                        return std::unexpected(LLMError("tool call name must not be empty"));
                    }
                    if(!ids.emplace(call.id).second) {
                        return std::unexpected(
                            LLMError(std::format("duplicate tool call id '{}'", call.id)));
                    }
                }
            } else if constexpr(std::same_as<message_type, ToolResultMessage>) {
                if(current.tool_call_id.empty()) {
                    return std::unexpected(
                        LLMError("tool result message tool_call_id must not be empty"));
                }
            }
            return {};
        }, message);
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
    }

    return {};
}

template <typename T>
auto parse_json_value(std::string_view raw, std::string_view context)
    -> std::expected<T, LLMError> {
    auto parsed = json::from_json<T>(raw);
    if(!parsed.has_value()) {
        return std::unexpected(LLMError(std::format(
            "failed to parse {}: {}", context, parsed.error().to_string())));
    }
    return *parsed;
}

template <typename T>
auto parse_json_value(const json::Value& value, std::string_view context)
    -> std::expected<T, LLMError> {
    auto raw = value.to_json_string();
    if(!raw.has_value()) {
        return std::unexpected(LLMError(std::format(
            "failed to serialize {} for parsing: {}",
            context, json::error_message(json::make_write_error(raw.error())))));
    }
    return parse_json_value<T>(*raw, context);
}

auto serialize_value_to_string(const json::Value& value,
                               std::string_view context)
    -> std::expected<std::string, LLMError> {
    auto encoded = value.to_json_string();
    if(!encoded.has_value()) {
        return std::unexpected(LLMError(std::format(
            "failed to serialize {}: {}",
            context, json::error_message(json::make_write_error(encoded.error())))));
    }
    return *encoded;
}

auto clone_object(json::Document& document,
                  json::ObjectRef source,
                  std::string_view context)
    -> std::expected<json::Object, LLMError>;

auto clone_array(json::Document& document,
                 json::ArrayRef source,
                 std::string_view context)
    -> std::expected<json::Array, LLMError>;

auto append_cloned_value(json::Document& document,
                         json::Array& target,
                         json::ValueRef source,
                         std::string_view context)
    -> std::expected<void, LLMError> {
    if(auto object = source.get_object(); object.has_value()) {
        auto cloned = clone_object(document, *object, context);
        if(!cloned.has_value()) {
            return std::unexpected(std::move(cloned.error()));
        }
        auto status = target.push_back(std::move(*cloned));
        if(!status.has_value()) {
            return to_llm_unexpected(status.error(), context);
        }
        return {};
    }

    if(auto array = source.get_array(); array.has_value()) {
        auto cloned = clone_array(document, *array, context);
        if(!cloned.has_value()) {
            return std::unexpected(std::move(cloned.error()));
        }
        auto status = target.push_back(std::move(*cloned));
        if(!status.has_value()) {
            return to_llm_unexpected(status.error(), context);
        }
        return {};
    }

    if(auto text = source.get_string(); text.has_value()) {
        auto status = target.push_back(*text);
        if(!status.has_value()) {
            return to_llm_unexpected(status.error(), context);
        }
        return {};
    }

    if(auto boolean = source.get_bool(); boolean.has_value()) {
        auto status = target.push_back(*boolean);
        if(!status.has_value()) {
            return to_llm_unexpected(status.error(), context);
        }
        return {};
    }

    if(source.is_int()) {
        if(auto integer = source.get_int(); integer.has_value()) {
            auto status = target.push_back(*integer);
            if(!status.has_value()) {
                return to_llm_unexpected(status.error(), context);
            }
            return {};
        }
        if(auto integer = source.get_uint(); integer.has_value()) {
            auto status = target.push_back(*integer);
            if(!status.has_value()) {
                return to_llm_unexpected(status.error(), context);
            }
            return {};
        }
    }

    if(auto number = source.get_double(); number.has_value()) {
        auto status = target.push_back(*number);
        if(!status.has_value()) {
            return to_llm_unexpected(status.error(), context);
        }
        return {};
    }

    if(source.is_null()) {
        auto status = target.push_back(nullptr);
        if(!status.has_value()) {
            return to_llm_unexpected(status.error(), context);
        }
        return {};
    }

    return std::unexpected(LLMError(std::format(
        "{}: unsupported JSON value kind during DOM clone", context)));
}

auto insert_cloned_value(json::Document& document,
                         json::Object& target,
                         std::string_view key,
                         json::ValueRef source,
                         std::string_view context)
    -> std::expected<void, LLMError> {
    if(auto object = source.get_object(); object.has_value()) {
        auto cloned = clone_object(document, *object, context);
        if(!cloned.has_value()) {
            return std::unexpected(std::move(cloned.error()));
        }
        auto status = target.insert(key, std::move(*cloned));
        if(!status.has_value()) {
            return to_llm_unexpected(status.error(), context);
        }
        return {};
    }

    if(auto array = source.get_array(); array.has_value()) {
        auto cloned = clone_array(document, *array, context);
        if(!cloned.has_value()) {
            return std::unexpected(std::move(cloned.error()));
        }
        auto status = target.insert(key, std::move(*cloned));
        if(!status.has_value()) {
            return to_llm_unexpected(status.error(), context);
        }
        return {};
    }

    if(auto text = source.get_string(); text.has_value()) {
        return insert_string_field(target, key, *text, context);
    }

    if(auto boolean = source.get_bool(); boolean.has_value()) {
        auto status = target.insert(key, *boolean);
        if(!status.has_value()) {
            return to_llm_unexpected(status.error(), context);
        }
        return {};
    }

    if(source.is_int()) {
        if(auto integer = source.get_int(); integer.has_value()) {
            auto status = target.insert(key, *integer);
            if(!status.has_value()) {
                return to_llm_unexpected(status.error(), context);
            }
            return {};
        }
        if(auto integer = source.get_uint(); integer.has_value()) {
            auto status = target.insert(key, *integer);
            if(!status.has_value()) {
                return to_llm_unexpected(status.error(), context);
            }
            return {};
        }
    }

    if(auto number = source.get_double(); number.has_value()) {
        auto status = target.insert(key, *number);
        if(!status.has_value()) {
            return to_llm_unexpected(status.error(), context);
        }
        return {};
    }

    if(source.is_null()) {
        auto status = target.insert(key, nullptr);
        if(!status.has_value()) {
            return to_llm_unexpected(status.error(), context);
        }
        return {};
    }

    return std::unexpected(LLMError(std::format(
        "{}: unsupported JSON value kind during DOM clone", context)));
}

auto clone_array(json::Document& document,
                 json::ArrayRef source,
                 std::string_view context)
    -> std::expected<json::Array, LLMError> {
    auto cloned = make_empty_array(context);
    if(!cloned.has_value()) {
        return std::unexpected(std::move(cloned.error()));
    }
    for(auto value : source) {
        auto status = append_cloned_value(document, *cloned, value, context);
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
    }
    return std::move(*cloned);
}

auto clone_object(json::Document& document,
                  json::ObjectRef source,
                  std::string_view context)
    -> std::expected<json::Object, LLMError> {
    auto cloned = make_empty_object(context);
    if(!cloned.has_value()) {
        return std::unexpected(std::move(cloned.error()));
    }
    for(auto entry : source) {
        auto status = insert_cloned_value(document, *cloned, entry.key, entry.value, context);
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
    }
    return std::move(*cloned);
}

auto serialize_message(json::Document& document,
                       json::Array& out,
                       const Message& message)
    -> std::expected<void, LLMError> {
    auto object = make_empty_object("failed to create request message object");
    if(!object.has_value()) {
        return std::unexpected(std::move(object.error()));
    }

    auto status = std::visit([&](const auto& current)
        -> std::expected<void, LLMError> {
        using message_type = remove_cvref_t<decltype(current)>;
        if constexpr(std::same_as<message_type, SystemMessage>) {
            if(auto result = object->insert("role", "system"); !result.has_value()) {
                return to_llm_unexpected(result.error(), "failed to serialize system role");
            }
            auto content = normalize_utf8(current.content, "system content");
            auto result = insert_string_field(*object, "content", std::string_view(content),
                                              "failed to serialize system content");
            if(!result.has_value()) {
                return std::unexpected(std::move(result.error()));
            }
        } else if constexpr(std::same_as<message_type, UserMessage>) {
            if(auto result = object->insert("role", "user"); !result.has_value()) {
                return to_llm_unexpected(result.error(), "failed to serialize user role");
            }
            auto content = normalize_utf8(current.content, "user content");
            auto result = insert_string_field(*object, "content", std::string_view(content),
                                              "failed to serialize user content");
            if(!result.has_value()) {
                return std::unexpected(std::move(result.error()));
            }
        } else if constexpr(std::same_as<message_type, AssistantToolCallMessage>) {
            if(auto result = object->insert("role", "assistant"); !result.has_value()) {
                return to_llm_unexpected(result.error(), "failed to serialize assistant role");
            }
            if(current.content.has_value()) {
                auto content = normalize_utf8(*current.content, "assistant content");
                auto result = insert_string_field(*object, "content", std::string_view(content),
                                                  "failed to serialize assistant content");
                if(!result.has_value()) {
                    return std::unexpected(std::move(result.error()));
                }
            }
            if(!current.tool_calls.empty()) {
                auto tool_calls = make_empty_array("failed to create assistant tool_calls array");
                if(!tool_calls.has_value()) {
                    return std::unexpected(std::move(tool_calls.error()));
                }
                for(auto& tool_call : current.tool_calls) {
                    auto call_object = make_empty_object("failed to create tool call object");
                    if(!call_object.has_value()) {
                        return std::unexpected(std::move(call_object.error()));
                    }
                    auto function_object = make_empty_object(
                        "failed to create tool call function object");
                    if(!function_object.has_value()) {
                        return std::unexpected(std::move(function_object.error()));
                    }
                    auto call_id = insert_string_field(*call_object, "id",
                                                       std::string_view(tool_call.id),
                                                       "failed to serialize tool call id");
                    if(!call_id.has_value()) {
                        return std::unexpected(std::move(call_id.error()));
                    }
                    if(auto result = call_object->insert("type", "function"); !result.has_value()) {
                        return to_llm_unexpected(result.error(),
                                                 "failed to serialize tool call type");
                    }
                    auto function_name = insert_string_field(
                        *function_object, "name", std::string_view(tool_call.name),
                        "failed to serialize tool call function name");
                    if(!function_name.has_value()) {
                        return std::unexpected(std::move(function_name.error()));
                    }
                    auto arguments = normalize_utf8(tool_call.arguments_json,
                                                    "tool call arguments");
                    auto function_arguments = insert_string_field(
                        *function_object, "arguments", std::string_view(arguments),
                        "failed to serialize tool call arguments");
                    if(!function_arguments.has_value()) {
                        return std::unexpected(std::move(function_arguments.error()));
                    }
                    if(auto result = call_object->insert("function", std::move(*function_object));
                       !result.has_value()) {
                        return to_llm_unexpected(result.error(),
                                                 "failed to serialize tool call function payload");
                    }
                    if(auto result = tool_calls->push_back(std::move(*call_object));
                       !result.has_value()) {
                        return to_llm_unexpected(result.error(),
                                                 "failed to append tool call");
                    }
                }
                if(auto result = object->insert("tool_calls", std::move(*tool_calls));
                   !result.has_value()) {
                    return to_llm_unexpected(result.error(),
                                             "failed to serialize assistant tool_calls");
                }
            }
        } else if constexpr(std::same_as<message_type, ToolResultMessage>) {
            if(auto result = object->insert("role", "tool"); !result.has_value()) {
                return to_llm_unexpected(result.error(), "failed to serialize tool role");
            }
            auto tool_call_id = insert_string_field(
                *object, "tool_call_id", std::string_view(current.tool_call_id),
                "failed to serialize tool_call_id");
            if(!tool_call_id.has_value()) {
                return std::unexpected(std::move(tool_call_id.error()));
            }
            auto content = normalize_utf8(current.content, "tool content");
            auto result = insert_string_field(*object, "content", std::string_view(content),
                                              "failed to serialize tool content");
            if(!result.has_value()) {
                return std::unexpected(std::move(result.error()));
            }
        }
        return {};
    }, message);

    if(!status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }

    auto push_status = out.push_back(std::move(*object));
    if(!push_status.has_value()) {
        return to_llm_unexpected(push_status.error(), "failed to append request message");
    }
    return {};
}

auto serialize_tool_choice(json::Document& document,
                           json::Object& root,
                           const ToolChoice& choice)
    -> std::expected<void, LLMError> {
    return std::visit([&](const auto& current) -> std::expected<void, LLMError> {
        using choice_type = remove_cvref_t<decltype(current)>;
        if constexpr(std::same_as<choice_type, ToolChoiceAuto>) {
            auto status = root.insert("tool_choice", "auto");
            if(!status.has_value()) {
                return to_llm_unexpected(status.error(), "failed to serialize tool_choice");
            }
            return {};
        } else if constexpr(std::same_as<choice_type, ToolChoiceRequired>) {
            auto status = root.insert("tool_choice", "required");
            if(!status.has_value()) {
                return to_llm_unexpected(status.error(), "failed to serialize tool_choice");
            }
            return {};
        } else if constexpr(std::same_as<choice_type, ToolChoiceNone>) {
            auto status = root.insert("tool_choice", "none");
            if(!status.has_value()) {
                return to_llm_unexpected(status.error(), "failed to serialize tool_choice");
            }
            return {};
        } else {
            auto object = make_empty_object("failed to serialize forced tool choice object");
            if(!object.has_value()) {
                return std::unexpected(std::move(object.error()));
            }
            auto function_object = make_empty_object(
                "failed to serialize forced tool choice function object");
            if(!function_object.has_value()) {
                return std::unexpected(std::move(function_object.error()));
            }
            if(auto status = object->insert("type", "function"); !status.has_value()) {
                return to_llm_unexpected(status.error(), "failed to serialize forced tool choice");
            }
            auto status = insert_string_field(*function_object, "name",
                                              std::string_view(current.name),
                                              "failed to serialize forced tool name");
            if(!status.has_value()) {
                return std::unexpected(std::move(status.error()));
            }
            if(auto status = object->insert("function", std::move(*function_object));
               !status.has_value()) {
                return to_llm_unexpected(status.error(), "failed to serialize forced tool payload");
            }
            auto root_status = root.insert("tool_choice", std::move(*object));
            if(!root_status.has_value()) {
                return to_llm_unexpected(root_status.error(), "failed to serialize tool_choice");
            }
            return {};
        }
    }, choice);
}

auto serialize_response_format(json::Document& document,
                               json::Object& root,
                               const ResponseFormat& format)
    -> std::expected<void, LLMError> {
    auto object = make_empty_object("failed to create response_format object");
    if(!object.has_value()) {
        return std::unexpected(std::move(object.error()));
    }
    auto schema_object = make_empty_object("failed to create response_format schema object");
    if(!schema_object.has_value()) {
        return std::unexpected(std::move(schema_object.error()));
    }

    if(auto status = object->insert("type", "json_schema"); !status.has_value()) {
        return std::unexpected(LLMError(std::format(
            "failed to serialize response_format.type: {}",
            json::error_message(status.error()))));
    }
    auto name_status = insert_string_field(*schema_object, "name",
                                           std::string_view(format.name),
                                           "failed to serialize response_format name");
    if(!name_status.has_value()) {
        return std::unexpected(std::move(name_status.error()));
    }
    if(auto status = schema_object->insert("strict", format.strict); !status.has_value()) {
        return std::unexpected(LLMError(std::format(
            "failed to serialize response_format strict flag: {}",
            json::error_message(status.error()))));
    }
    auto copied_schema = clone_object(document, format.schema,
                                      "failed to serialize response_format schema");
    if(!copied_schema.has_value()) {
        return std::unexpected(std::move(copied_schema.error()));
    }
    if(auto status = schema_object->insert("schema", std::move(*copied_schema));
       !status.has_value()) {
        return std::unexpected(LLMError(std::format(
            "failed to serialize response_format schema: {}",
            json::error_message(status.error()))));
    }
    if(auto status = object->insert("json_schema", std::move(*schema_object));
       !status.has_value()) {
        return std::unexpected(LLMError(std::format(
            "failed to serialize response_format json_schema payload: {}",
            json::error_message(status.error()))));
    }

    auto root_status = root.insert("response_format", std::move(*object));
    if(!root_status.has_value()) {
        return to_llm_unexpected(root_status.error(), "failed to serialize response_format");
    }
    return {};
}

auto serialize_tool_definition(json::Document& document,
                               json::Array& tools,
                               const FunctionToolDefinition& tool)
    -> std::expected<void, LLMError> {
    auto object = make_empty_object("failed to create tool definition object");
    if(!object.has_value()) {
        return std::unexpected(std::move(object.error()));
    }
    auto function_object = make_empty_object("failed to create function tool object");
    if(!function_object.has_value()) {
        return std::unexpected(std::move(function_object.error()));
    }

    if(auto status = object->insert("type", "function"); !status.has_value()) {
        return std::unexpected(LLMError(std::format(
            "failed to serialize tool type: {}",
            json::error_message(status.error()))));
    }
    auto name_status = insert_string_field(*function_object, "name",
                                           std::string_view(tool.name),
                                           "failed to serialize tool name");
    if(!name_status.has_value()) {
        return std::unexpected(std::move(name_status.error()));
    }
    auto description_status = insert_string_field(*function_object, "description",
                                                  std::string_view(tool.description),
                                                  "failed to serialize tool description");
    if(!description_status.has_value()) {
        return std::unexpected(std::move(description_status.error()));
    }
    auto copied_parameters = clone_object(document, tool.parameters,
                                          "failed to serialize tool parameters");
    if(!copied_parameters.has_value()) {
        return std::unexpected(std::move(copied_parameters.error()));
    }
    if(auto status = function_object->insert("parameters", std::move(*copied_parameters));
       !status.has_value()) {
        return std::unexpected(LLMError(std::format(
            "failed to serialize tool parameters: {}",
            json::error_message(status.error()))));
    }
    if(auto status = function_object->insert("strict", tool.strict); !status.has_value()) {
        return std::unexpected(LLMError(std::format(
            "failed to serialize tool strict flag: {}",
            json::error_message(status.error()))));
    }
    if(auto status = object->insert("function", std::move(*function_object));
       !status.has_value()) {
        return std::unexpected(LLMError(std::format(
            "failed to serialize function tool payload: {}",
            json::error_message(status.error()))));
    }

    auto push_status = tools.push_back(std::move(*object));
    if(!push_status.has_value()) {
        return to_llm_unexpected(push_status.error(), "failed to append request tool");
    }
    return {};
}

auto parse_content_parts(json::ArrayRef parts)
    -> std::expected<AssistantOutput, LLMError> {
    AssistantOutput output;
    std::string text;
    std::string refusal;
    bool saw_text = false;
    bool saw_refusal = false;

    for(auto value : parts) {
        auto part = expect_object(value, "message.content[]");
        if(!part.has_value()) {
            return std::unexpected(std::move(part.error()));
        }

        auto type_value = part->get("type");
        std::string_view type = "text";
        if(type_value.has_value()) {
            auto parsed_type = expect_string(*type_value, "message.content[].type");
            if(!parsed_type.has_value()) {
                return std::unexpected(std::move(parsed_type.error()));
            }
            type = *parsed_type;
        }

        if(type == "refusal") {
            auto refusal_value = part->get("refusal");
            if(!refusal_value.has_value()) {
                return std::unexpected(
                    LLMError("refusal content part is missing the refusal field"));
            }
            auto parsed_refusal = expect_string(*refusal_value,
                                                "message.content[].refusal");
            if(!parsed_refusal.has_value()) {
                return std::unexpected(std::move(parsed_refusal.error()));
            }
            refusal += *parsed_refusal;
            saw_refusal = true;
            continue;
        }

        if(type != "text" && type != "output_text") {
            continue;
        }

        auto text_value = part->get("text");
        if(!text_value.has_value()) {
            return std::unexpected(
                LLMError("text content part is missing the text payload"));
        }

        if(auto part_text = text_value->get_string(); part_text.has_value()) {
            text += *part_text;
            saw_text = true;
            continue;
        }

        auto text_object = expect_object(*text_value, "message.content[].text");
        if(!text_object.has_value()) {
            return std::unexpected(std::move(text_object.error()));
        }
        auto value_field = text_object->get("value");
        if(!value_field.has_value()) {
            return std::unexpected(
                LLMError("text content object is missing the value field"));
        }
        auto part_text = expect_string(*value_field, "message.content[].text.value");
        if(!part_text.has_value()) {
            return std::unexpected(std::move(part_text.error()));
        }
        text += *part_text;
        saw_text = true;
    }

    if(saw_text) {
        output.text = std::move(text);
    }
    if(saw_refusal) {
        output.refusal = std::move(refusal);
    }
    return output;
}

auto parse_tool_calls(json::ArrayRef calls)
    -> std::expected<std::vector<ToolCall>, LLMError> {
    std::vector<ToolCall> parsed_calls;
    parsed_calls.reserve(calls.size());
    std::unordered_set<std::string> ids;

    for(auto value : calls) {
        auto call = expect_object(value, "message.tool_calls[]");
        if(!call.has_value()) {
            return std::unexpected(std::move(call.error()));
        }

        auto id_value = call->get("id");
        if(!id_value.has_value()) {
            return std::unexpected(LLMError("tool call is missing id"));
        }
        auto id = expect_string(*id_value, "message.tool_calls[].id");
        if(!id.has_value()) {
            return std::unexpected(std::move(id.error()));
        }
        if(!ids.emplace(std::string(*id)).second) {
            return std::unexpected(
                LLMError(std::format("duplicate tool call id '{}'", *id)));
        }

        auto type_value = call->get("type");
        if(!type_value.has_value()) {
            return std::unexpected(LLMError("tool call is missing type"));
        }
        auto type = expect_string(*type_value, "message.tool_calls[].type");
        if(!type.has_value()) {
            return std::unexpected(std::move(type.error()));
        }
        if(*type != "function") {
            return std::unexpected(LLMError(std::format(
                "unsupported tool call type '{}'", *type)));
        }

        auto function_value = call->get("function");
        if(!function_value.has_value()) {
            return std::unexpected(LLMError("tool call is missing function payload"));
        }
        auto function = expect_object(*function_value, "message.tool_calls[].function");
        if(!function.has_value()) {
            return std::unexpected(std::move(function.error()));
        }

        auto name_value = function->get("name");
        if(!name_value.has_value()) {
            return std::unexpected(LLMError("tool call function is missing name"));
        }
        auto name = expect_string(*name_value, "message.tool_calls[].function.name");
        if(!name.has_value()) {
            return std::unexpected(std::move(name.error()));
        }

        auto arguments_value = function->get("arguments");
        if(!arguments_value.has_value()) {
            return std::unexpected(
                LLMError("tool call function is missing arguments"));
        }
        auto arguments_json = expect_string(*arguments_value,
            "message.tool_calls[].function.arguments");
        if(!arguments_json.has_value()) {
            return std::unexpected(std::move(arguments_json.error()));
        }
        auto parsed_arguments = json::Value::parse(*arguments_json);
        if(!parsed_arguments.has_value()) {
            return std::unexpected(LLMError(std::format(
                "failed to parse tool call arguments for '{}': {}",
                *name, json::error_message(json::make_read_error(parsed_arguments.error())))));
        }

        parsed_calls.push_back(ToolCall{
            .id = std::string(*id),
            .name = std::string(*name),
            .arguments_json = std::string(*arguments_json),
            .arguments = std::move(*parsed_arguments),
        });
    }

    return parsed_calls;
}

auto compute_retry_delay(std::uint32_t next_attempt,
                         std::uint32_t retry_initial_backoff_ms)
    -> std::expected<std::chrono::milliseconds, LLMError> {
    if(next_attempt == 0) {
        return std::unexpected(LLMError("next_attempt must be greater than 0"));
    }

    std::uint64_t multiplier = 1;
    for(std::uint32_t attempt = 1; attempt < next_attempt; ++attempt) {
        if(multiplier > (std::numeric_limits<std::uint64_t>::max() / 2)) {
            return std::unexpected(LLMError(
                std::format("retry delay overflow at attempt {}", next_attempt)));
        }
        multiplier *= 2;
    }

    if(static_cast<std::uint64_t>(retry_initial_backoff_ms) >
       (std::numeric_limits<std::uint64_t>::max() / multiplier)) {
        return std::unexpected(LLMError(
            std::format("retry delay overflow at attempt {}", next_attempt)));
    }

    auto delay_ms = static_cast<std::uint64_t>(retry_initial_backoff_ms) * multiplier;
    if(delay_ms > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        return std::unexpected(LLMError(
            std::format("retry delay exceeds supported range: {}ms", delay_ms)));
    }

    return std::chrono::milliseconds(static_cast<std::int64_t>(delay_ms));
}

auto request_text_with_retries(std::string model,
                               std::string system_prompt,
                               std::string prompt,
                               std::uint32_t retry_count,
                               std::uint32_t retry_initial_backoff_ms,
                               async::event_loop& loop)
    -> async::task<std::string, LLMError> {
    for(std::uint32_t attempt = 0;; ++attempt) {
        auto result = co_await call_llm_async(model, system_prompt, prompt, loop).catch_cancel();
        if(!result.is_cancelled() && !result.has_error()) {
            co_return std::move(*result);
        }

        LLMError error = result.is_cancelled()
            ? LLMError("LLM request cancelled")
            : LLMError(std::move(result).error());

        if(attempt >= retry_count) {
            co_await async::fail(std::move(error));
        }

        auto delay = compute_retry_delay(attempt + 1, retry_initial_backoff_ms);
        if(!delay.has_value()) {
            co_await async::fail(std::move(delay.error()));
        }

        logging::warn(
            "LLM request failed: attempt={}/{} retry_in_ms={} reason={}",
            attempt + 1, retry_count + 1, delay->count(), error.message);
        co_await async::sleep(*delay, loop);
    }
}

auto call_completion_blocking(CompletionRequest request)
    -> std::expected<CompletionResponse, LLMError> {
    auto request_json = protocol::build_request_json(request);
    if(!request_json.has_value()) {
        return std::unexpected(std::move(request_json.error()));
    }

    auto environment = detail::read_environment();
    if(!environment.has_value()) {
        return std::unexpected(std::move(environment.error()));
    }

    auto url = detail::build_chat_completions_url(environment->api_base);
    logging::info("calling LLM: {}", url);

    auto raw_response = detail::perform_http_request(
        url,
        environment->api_key,
        *request_json);
    if(!raw_response.has_value()) {
        logging::warn("LLM request failed: {} ({})", url, raw_response.error().message);
        return std::unexpected(std::move(raw_response.error()));
    }

    logging::info("completed LLM request: {} status={} bytes={}",
                  url, raw_response->http_status, raw_response->body.size());

    if(raw_response->body.empty()) {
        return std::unexpected(LLMError("empty response from LLM"));
    }
    if(raw_response->http_status >= 400) {
        return std::unexpected(LLMError(std::format(
            "LLM request failed with HTTP {}: {}",
            raw_response->http_status,
            detail::excerpt_for_error(raw_response->body))));
    }

    auto parsed = protocol::parse_response(raw_response->body);
    if(!parsed.has_value()) {
        return std::unexpected(std::move(parsed.error()));
    }
    return std::move(*parsed);
}

auto call_llm_blocking(std::string_view model,
                       std::string_view system_prompt,
                       std::string_view prompt)
    -> std::expected<std::string, LLMError> {
    CompletionRequest request{
        .model = std::string(model),
        .messages = {
            SystemMessage{.content = std::string(system_prompt)},
            UserMessage{.content = std::string(prompt)},
        },
    };

    auto response = call_completion_blocking(std::move(request));
    if(!response.has_value()) {
        return std::unexpected(std::move(response.error()));
    }

    auto text = protocol::text_from_response(*response);
    if(!text.has_value()) {
        return std::unexpected(std::move(text.error()));
    }
    return std::move(*text);
}

auto wait_for_retry_delay_or_stop(std::chrono::milliseconds delay,
                                  const std::function<bool()>& stop_requested) -> bool {
    constexpr auto kPollInterval = std::chrono::milliseconds(50);

    auto remaining = delay;
    while(remaining.count() > 0) {
        if(stop_requested && stop_requested()) {
            return false;
        }

        auto slice = remaining < kPollInterval ? remaining : kPollInterval;
        std::this_thread::sleep_for(slice);
        remaining -= slice;
    }

    return !stop_requested || !stop_requested();
}

auto request_text_with_retries_blocking(std::string model,
                                        std::string system_prompt,
                                        std::string prompt,
                                        std::uint32_t retry_count,
                                        std::uint32_t retry_initial_backoff_ms,
                                        std::function<bool()> stop_requested = {})
    -> std::expected<std::string, LLMError> {
    for(std::uint32_t attempt = 0;; ++attempt) {
        if(stop_requested && stop_requested()) {
            return std::unexpected(LLMError("LLM request cancelled"));
        }

        auto result = call_llm_blocking(model, system_prompt, prompt);
        if(result.has_value()) {
            return result;
        }

        if(attempt >= retry_count) {
            return std::unexpected(std::move(result.error()));
        }

        auto delay = compute_retry_delay(attempt + 1, retry_initial_backoff_ms);
        if(!delay.has_value()) {
            return std::unexpected(std::move(delay.error()));
        }

        logging::warn(
            "LLM request failed: attempt={}/{} retry_in_ms={} reason={}",
            attempt + 1, retry_count + 1, delay->count(), result.error().message);
        if(!wait_for_retry_delay_or_stop(*delay, stop_requested)) {
            return std::unexpected(LLMError("LLM request cancelled"));
        }
    }
}

auto run_client_request(std::uint64_t tag,
                        std::string prompt,
                        std::string_view model,
                        std::string_view system_prompt,
                        std::uint32_t retry_count,
                        std::uint32_t retry_initial_backoff_ms,
                        async::event_loop& loop,
                        std::function<void(std::uint64_t,
                                           std::expected<std::string, LLMError>)>& on_complete,
                        std::function<void()>& on_finished)
    -> async::task<> {
    auto result = co_await request_text_with_retries(
        std::string(model), std::string(system_prompt), std::move(prompt),
        retry_count, retry_initial_backoff_ms, loop).catch_cancel();

    std::expected<std::string, LLMError> normalized_result;
    if(result.is_cancelled()) {
        normalized_result = std::unexpected(LLMError("LLM request cancelled"));
    } else if(result.has_error()) {
        normalized_result = std::unexpected(std::move(result).error());
    } else {
        normalized_result = std::move(*result);
    }

    on_complete(tag, std::move(normalized_result));
    on_finished();
}

}  // namespace clore::net::detail

namespace clore::net::schema {

template <typename T>
auto response_format() -> std::expected<ResponseFormat, LLMError> {
    using root_type = detail::schema_subject_t<T>;
    static_assert(eventide::refl::reflectable_class<root_type>,
                  "automatic structured output schemas require a reflectable root object type");

    auto name = detail::schema_type_name<root_type>();
    if(!name.has_value()) {
        return std::unexpected(std::move(name.error()));
    }

    auto schema = detail::make_schema_object<root_type>();
    if(!schema.has_value()) {
        return std::unexpected(std::move(schema.error()));
    }

    return ResponseFormat{
        .name = std::move(*name),
        .schema = std::move(*schema),
        .strict = true,
    };
}

template <typename T>
auto function_tool(std::string name, std::string description)
    -> std::expected<FunctionToolDefinition, LLMError> {
    using root_type = detail::schema_subject_t<T>;
    static_assert(eventide::refl::reflectable_class<root_type>,
                  "automatic function tool schemas require a reflectable root object type");

    if(name.empty()) {
        return std::unexpected(LLMError("tool name must not be empty"));
    }
    if(description.empty()) {
        return std::unexpected(LLMError("tool description must not be empty"));
    }

    auto schema = detail::make_schema_object<root_type>();
    if(!schema.has_value()) {
        return std::unexpected(std::move(schema.error()));
    }

    return FunctionToolDefinition{
        .name = std::move(name),
        .description = std::move(description),
        .parameters = std::move(*schema),
        .strict = true,
    };
}

}  // namespace clore::net::schema

namespace clore::net::protocol {

template <typename T>
auto parse_response_text(const CompletionResponse& response)
    -> std::expected<T, LLMError> {
    auto text = text_from_response(response);
    if(!text.has_value()) {
        return std::unexpected(std::move(text.error()));
    }
    return detail::parse_json_value<T>(*text, "structured LLM response");
}

template <typename T>
auto parse_tool_arguments(const ToolCall& call)
    -> std::expected<T, LLMError> {
    return detail::parse_json_value<T>(call.arguments, std::format(
        "tool arguments for '{}'", call.name));
}

auto build_request_json(const CompletionRequest& request)
    -> std::expected<std::string, LLMError> {
    auto validation = detail::validate_request(request);
    if(!validation.has_value()) {
        return std::unexpected(std::move(validation.error()));
    }

    detail::json::Document document;
    auto root = detail::make_empty_object("failed to create request root object");
    if(!root.has_value()) {
        return std::unexpected(std::move(root.error()));
    }
    auto messages = detail::make_empty_array("failed to create request messages array");
    if(!messages.has_value()) {
        return std::unexpected(std::move(messages.error()));
    }

    if(auto status = root->insert("model", std::string_view(request.model)); !status.has_value()) {
        return detail::to_llm_unexpected(status.error(), "failed to serialize model");
    }

    for(auto& message : request.messages) {
        auto status = detail::serialize_message(document, *messages, message);
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
    }
    if(auto status = root->insert("messages", std::move(*messages)); !status.has_value()) {
        return detail::to_llm_unexpected(status.error(),
                                         "failed to serialize request messages");
    }

    if(request.response_format.has_value()) {
        auto response_format = detail::serialize_response_format(
            document, *root, *request.response_format);
        if(!response_format.has_value()) {
            return std::unexpected(std::move(response_format.error()));
        }
    }

    if(!request.tools.empty()) {
        auto tools = detail::make_empty_array("failed to create tools array");
        if(!tools.has_value()) {
            return std::unexpected(std::move(tools.error()));
        }
        for(auto& tool : request.tools) {
            auto serialized = detail::serialize_tool_definition(document, *tools, tool);
            if(!serialized.has_value()) {
                return std::unexpected(std::move(serialized.error()));
            }
        }
        if(auto status = root->insert("tools", std::move(*tools)); !status.has_value()) {
            return detail::to_llm_unexpected(status.error(),
                                             "failed to serialize tools array");
        }
    }

    if(request.tool_choice.has_value()) {
        auto tool_choice = detail::serialize_tool_choice(document, *root, *request.tool_choice);
        if(!tool_choice.has_value()) {
            return std::unexpected(std::move(tool_choice.error()));
        }
    }

    if(request.parallel_tool_calls.has_value()) {
        if(auto status = root->insert("parallel_tool_calls", *request.parallel_tool_calls);
           !status.has_value()) {
            return detail::to_llm_unexpected(status.error(),
                                             "failed to serialize parallel_tool_calls");
        }
    }

    auto encoded = root->to_json_string();
    if(!encoded.has_value()) {
        return std::unexpected(LLMError(std::format(
            "failed to serialize request JSON: {}",
            detail::json::error_message(detail::json::make_write_error(encoded.error())))));
    }
    return *encoded;
}

auto parse_response(std::string_view json)
    -> std::expected<CompletionResponse, LLMError> {
    auto parsed = detail::json::Object::parse(json);
    if(!parsed.has_value()) {
        return std::unexpected(LLMError(std::format(
            "failed to parse LLM response JSON: {}",
            detail::json::error_message(detail::json::make_read_error(parsed.error())))));
    }

    auto error_value = parsed->get("error");
    if(error_value.has_value()) {
        auto error_object = detail::expect_object(*error_value, "error");
        if(!error_object.has_value()) {
            return std::unexpected(std::move(error_object.error()));
        }
        auto message_value = error_object->get("message");
        if(message_value.has_value()) {
            auto message = detail::expect_string(*message_value, "error.message");
            if(message.has_value()) {
                return std::unexpected(LLMError(std::format("LLM API error: {}", *message)));
            }
        }
        return std::unexpected(LLMError("LLM API returned an error payload"));
    }

    auto id_value = parsed->get("id");
    if(!id_value.has_value()) {
        return std::unexpected(LLMError("LLM response is missing id"));
    }
    auto id = detail::expect_string(*id_value, "id");
    if(!id.has_value()) {
        return std::unexpected(std::move(id.error()));
    }

    auto model_value = parsed->get("model");
    if(!model_value.has_value()) {
        return std::unexpected(LLMError("LLM response is missing model"));
    }
    auto model = detail::expect_string(*model_value, "model");
    if(!model.has_value()) {
        return std::unexpected(std::move(model.error()));
    }

    auto choices_value = parsed->get("choices");
    if(!choices_value.has_value()) {
        return std::unexpected(LLMError("LLM response has no choices"));
    }
    auto choices = detail::expect_array(*choices_value, "choices");
    if(!choices.has_value()) {
        return std::unexpected(std::move(choices.error()));
    }
    if(choices->empty()) {
        return std::unexpected(LLMError("LLM response choices array is empty"));
    }

    auto first_choice = detail::expect_object((*choices)[0], "choices[0]");
    if(!first_choice.has_value()) {
        return std::unexpected(std::move(first_choice.error()));
    }

    auto finish_reason_value = first_choice->get("finish_reason");
    if(!finish_reason_value.has_value()) {
        return std::unexpected(LLMError("LLM response choice is missing finish_reason"));
    }
    auto finish_reason = detail::expect_string(*finish_reason_value, "choices[0].finish_reason");
    if(!finish_reason.has_value()) {
        return std::unexpected(std::move(finish_reason.error()));
    }
    if(*finish_reason == "length") {
        return std::unexpected(LLMError("LLM response was truncated (finish_reason=length)"));
    }
    if(*finish_reason == "content_filter") {
        return std::unexpected(LLMError("LLM response was blocked by content filters"));
    }
    if(*finish_reason != "stop" && *finish_reason != "tool_calls") {
        return std::unexpected(LLMError(std::format(
            "unsupported LLM finish_reason '{}'", *finish_reason)));
    }

    auto message_value = first_choice->get("message");
    if(!message_value.has_value()) {
        return std::unexpected(LLMError("LLM response choice has no message"));
    }
    auto message = detail::expect_object(*message_value, "choices[0].message");
    if(!message.has_value()) {
        return std::unexpected(std::move(message.error()));
    }

    AssistantOutput output;

    if(auto refusal_value = message->get("refusal"); refusal_value.has_value()) {
        auto refusal = detail::expect_string(*refusal_value, "choices[0].message.refusal");
        if(!refusal.has_value()) {
            return std::unexpected(std::move(refusal.error()));
        }
        output.refusal = std::string(*refusal);
    }

    if(auto content_value = message->get("content"); content_value.has_value()) {
        if(auto content_text = content_value->get_string(); content_text.has_value()) {
            output.text = std::string(*content_text);
        } else if(auto content_parts = content_value->get_array(); content_parts.has_value()) {
            auto parsed_content = detail::parse_content_parts(*content_parts);
            if(!parsed_content.has_value()) {
                return std::unexpected(std::move(parsed_content.error()));
            }
            if(parsed_content->text.has_value()) {
                output.text = std::move(parsed_content->text);
            }
            if(parsed_content->refusal.has_value()) {
                output.refusal = std::move(parsed_content->refusal);
            }
        } else if(!content_value->is_null()) {
            return std::unexpected(
                LLMError("LLM response message content is neither string, array, nor null"));
        }
    }

    if(auto tool_calls_value = message->get("tool_calls"); tool_calls_value.has_value()) {
        auto tool_calls = detail::expect_array(*tool_calls_value,
                                               "choices[0].message.tool_calls");
        if(!tool_calls.has_value()) {
            return std::unexpected(std::move(tool_calls.error()));
        }
        auto parsed_calls = detail::parse_tool_calls(*tool_calls);
        if(!parsed_calls.has_value()) {
            return std::unexpected(std::move(parsed_calls.error()));
        }
        output.tool_calls = std::move(*parsed_calls);
    }

    if(*finish_reason == "tool_calls" && output.tool_calls.empty()) {
        return std::unexpected(LLMError(
            "LLM response finish_reason=tool_calls but no tool calls were returned"));
    }
    if(*finish_reason == "stop" && !output.tool_calls.empty()) {
        return std::unexpected(LLMError(
            "LLM response returned tool calls with finish_reason=stop"));
    }

    if(!output.text.has_value() && !output.refusal.has_value() && output.tool_calls.empty()) {
        return std::unexpected(
            LLMError("LLM response message has neither text, refusal, nor tool calls"));
    }

    return CompletionResponse{
        .id = std::string(*id),
        .model = std::string(*model),
        .message = std::move(output),
        .raw_json = std::string(json),
    };
}

auto text_from_response(const CompletionResponse& response)
    -> std::expected<std::string, LLMError> {
    if(response.message.refusal.has_value()) {
        return std::unexpected(LLMError(std::format(
            "LLM refused structured/text response: {}", *response.message.refusal)));
    }
    if(!response.message.tool_calls.empty()) {
        return std::unexpected(LLMError(
            "LLM response contains tool calls instead of a text completion"));
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
            LLMError("cannot append tool outputs for a response without tool calls"));
    }

    std::unordered_map<std::string, std::string> output_by_id;
    output_by_id.reserve(outputs.size());
    for(auto& output : outputs) {
        if(output.tool_call_id.empty()) {
            return std::unexpected(LLMError("tool output id must not be empty"));
        }
        if(!output_by_id.emplace(output.tool_call_id, output.output).second) {
            return std::unexpected(LLMError(std::format(
                "duplicate tool output for call '{}'", output.tool_call_id)));
        }
    }

    std::vector<Message> merged(history.begin(), history.end());
    merged.push_back(AssistantToolCallMessage{
        .content = response.message.text,
        .tool_calls = response.message.tool_calls,
    });

    for(auto& tool_call : response.message.tool_calls) {
        auto it = output_by_id.find(tool_call.id);
        if(it == output_by_id.end()) {
            return std::unexpected(LLMError(std::format(
                "missing tool output for call '{}'", tool_call.id)));
        }
        merged.push_back(ToolResultMessage{
            .tool_call_id = tool_call.id,
            .content = it->second,
        });
    }

    if(output_by_id.size() != response.message.tool_calls.size()) {
        return std::unexpected(
            LLMError("tool outputs contain unknown call ids not present in the response"));
    }

    return merged;
}

}  // namespace clore::net::protocol

namespace clore::net {

LLMClient::LLMClient(std::string_view model,
                     std::string_view system_prompt,
                     std::uint32_t max_concurrent,
                     std::uint32_t retry_count,
                     std::uint32_t retry_initial_backoff_ms)
    : model_(model),
      system_prompt_(system_prompt),
      max_concurrent_(max_concurrent),
      retry_count_(retry_count),
      retry_initial_backoff_ms_(retry_initial_backoff_ms) {
    if(model_.empty()) {
        configuration_error_ = LLMError("model must not be empty");
    } else if(max_concurrent_ == 0) {
        configuration_error_ = LLMError("max_concurrent must be greater than 0");
    }
}

auto LLMClient::submit(std::uint64_t tag, std::string prompt)
    -> std::expected<void, LLMError> {
    if(configuration_error_.has_value()) {
        return std::unexpected(*configuration_error_);
    }

    std::shared_ptr<RunState> active_run;
    {
        std::lock_guard guard(active_run_mutex_);
        active_run = active_run_;
    }

    if(active_run != nullptr) {
        {
            std::lock_guard guard(active_run->mutex);
            if(active_run->shutdown || active_run->stop_requested) {
                return {};
            }

            active_run->pending.push_back(PendingRequest{
                .tag = tag,
                .prompt = std::move(prompt),
            });
        }
        active_run->condition.notify_one();
        return {};
    }

    pending_.push_back(PendingRequest{
        .tag = tag,
        .prompt = std::move(prompt),
    });
    return {};
}

auto LLMClient::request_stop() noexcept -> void {
    std::shared_ptr<RunState> active_run;
    {
        std::lock_guard guard(active_run_mutex_);
        active_run = active_run_;
    }

    if(active_run != nullptr) {
        {
            std::lock_guard guard(active_run->mutex);
            active_run->stop_requested = true;
            active_run->pending.clear();
        }
        active_run->condition.notify_all();
        return;
    }

    pending_.clear();
}

auto LLMClient::compute_retry_delay(std::uint32_t next_attempt) const
    -> std::expected<std::chrono::milliseconds, LLMError> {
    return detail::compute_retry_delay(next_attempt, retry_initial_backoff_ms_);
}

auto LLMClient::run(Callback on_complete) -> std::expected<void, LLMError> {
    if(configuration_error_.has_value()) {
        return std::unexpected(*configuration_error_);
    }
    if(pending_.empty()) {
        return {};
    }

    auto environment = detail::read_environment();
    if(!environment.has_value()) {
        return std::unexpected(std::move(environment.error()));
    }

    auto run_state = std::make_shared<RunState>();
    run_state->pending = std::move(pending_);
    pending_.clear();

    {
        std::lock_guard guard(active_run_mutex_);
        active_run_ = run_state;
    }

    struct ActiveRunReset {
        LLMClient* owner;

        ~ActiveRunReset() {
            std::lock_guard guard(owner->active_run_mutex_);
            owner->active_run_.reset();
        }
    } active_run_reset{this};

    auto model = model_;
    auto system_prompt = system_prompt_;
    auto retry_count = retry_count_;
    auto retry_initial_backoff_ms = retry_initial_backoff_ms_;

    std::vector<std::jthread> workers;
    workers.reserve(max_concurrent_);
    for(std::uint32_t worker_index = 0; worker_index < max_concurrent_; ++worker_index) {
        workers.emplace_back([run_state,
                              model,
                              system_prompt,
                              retry_count,
                              retry_initial_backoff_ms](std::stop_token stop_token) mutable {
            while(true) {
                PendingRequest request;
                {
                    std::unique_lock lock(run_state->mutex);
                    run_state->condition.wait(lock, [&] {
                        return stop_token.stop_requested() || run_state->shutdown ||
                               run_state->stop_requested || !run_state->pending.empty();
                    });

                    if(stop_token.stop_requested() || run_state->shutdown) {
                        return;
                    }

                    if(run_state->pending.empty()) {
                        if(run_state->stop_requested) {
                            return;
                        }
                        continue;
                    }

                    request = std::move(run_state->pending.front());
                    run_state->pending.pop_front();
                    ++run_state->in_flight;
                }

                auto result = detail::request_text_with_retries_blocking(
                    model,
                    system_prompt,
                    std::move(request.prompt),
                    retry_count,
                    retry_initial_backoff_ms,
                    [stop_token, run_state] {
                        if(stop_token.stop_requested()) {
                            return true;
                        }
                        std::lock_guard guard(run_state->mutex);
                        return run_state->shutdown || run_state->stop_requested;
                    });

                {
                    std::lock_guard guard(run_state->mutex);
                    if(run_state->in_flight > 0) {
                        --run_state->in_flight;
                    }
                    run_state->completed.push_back(CompletedRequest{
                        .tag = request.tag,
                        .result = std::move(result),
                    });
                }
                run_state->condition.notify_all();
            }
        });
    }

    while(true) {
        std::optional<CompletedRequest> completed_request;
        {
            std::unique_lock lock(run_state->mutex);
            run_state->condition.wait(lock, [&] {
                return !run_state->completed.empty() ||
                       (run_state->pending.empty() && run_state->in_flight == 0);
            });

            if(!run_state->completed.empty()) {
                completed_request = std::move(run_state->completed.front());
                run_state->completed.pop_front();
            } else if(run_state->pending.empty() && run_state->in_flight == 0) {
                break;
            }
        }

        if(completed_request.has_value()) {
            on_complete(completed_request->tag, std::move(completed_request->result));
        }
    }

    {
        std::lock_guard guard(run_state->mutex);
        run_state->shutdown = true;
    }
    run_state->condition.notify_all();

    return {};
}

auto call_completion_async(CompletionRequest request, async::event_loop& loop)
    -> async::task<CompletionResponse, LLMError> {
    auto request_json = protocol::build_request_json(request);
    if(!request_json.has_value()) {
        co_await eventide::fail(std::move(request_json.error()));
    }

    auto environment = detail::read_environment();
    if(!environment.has_value()) {
        co_await eventide::fail(std::move(environment.error()));
    }

    auto raw_response = co_await detail::perform_http_request_async(
        detail::build_chat_completions_url(environment->api_base),
        environment->api_key,
        std::move(*request_json),
        loop).or_fail();

    if(raw_response.body.empty()) {
        co_await eventide::fail(LLMError("empty response from LLM"));
    }
    if(raw_response.http_status >= 400) {
        co_await eventide::fail(LLMError(std::format(
            "LLM request failed with HTTP {}: {}",
            raw_response.http_status,
            detail::excerpt_for_error(raw_response.body))));
    }

    auto parsed = protocol::parse_response(raw_response.body);
    if(!parsed.has_value()) {
        co_await eventide::fail(std::move(parsed.error()));
    }
    co_return std::move(*parsed);
}

auto call_completion(CompletionRequest request)
    -> std::expected<CompletionResponse, LLMError> {
    return detail::run_task_sync<CompletionResponse>(
        [request = std::move(request)](async::event_loop& loop) mutable {
            return call_completion_async(std::move(request), loop);
        });
}

auto call_llm_async(std::string_view model,
                    std::string_view system_prompt,
                    std::string_view prompt,
                    async::event_loop& loop)
    -> async::task<std::string, LLMError> {
    CompletionRequest request{
        .model = std::string(model),
        .messages = {
            SystemMessage{.content = std::string(system_prompt)},
            UserMessage{.content = std::string(prompt)},
        },
    };

    auto response = co_await call_completion_async(std::move(request), loop).or_fail();
    auto text = protocol::text_from_response(response);
    if(!text.has_value()) {
        co_await eventide::fail(std::move(text.error()));
    }
    co_return std::move(*text);
}

auto call_llm(std::string_view model,
              std::string_view system_prompt,
              std::string_view prompt)
    -> std::expected<std::string, LLMError> {
    return detail::run_task_sync<std::string>(
        [=](async::event_loop& loop) {
            return call_llm_async(model, system_prompt, prompt, loop);
        });
}

auto call_llm_with_retries(std::string_view model,
                           std::string_view system_prompt,
                           std::string_view prompt,
                           std::uint32_t retry_count,
                           std::uint32_t retry_initial_backoff_ms)
    -> std::expected<std::string, LLMError> {
    return detail::request_text_with_retries_blocking(
        std::string(model),
        std::string(system_prompt),
        std::string(prompt),
        retry_count,
        retry_initial_backoff_ms);
}

template <typename T>
auto call_structured_async(std::string_view model,
                           std::string_view system_prompt,
                           std::string_view prompt,
                           async::event_loop& loop)
    -> async::task<T, LLMError> {
    auto format = schema::response_format<T>();
    if(!format.has_value()) {
        co_await eventide::fail(std::move(format.error()));
    }

    CompletionRequest request{
        .model = std::string(model),
        .messages = {
            SystemMessage{.content = std::string(system_prompt)},
            UserMessage{.content = std::string(prompt)},
        },
        .response_format = std::move(*format),
    };

    auto response = co_await call_completion_async(std::move(request), loop).or_fail();
    auto parsed = protocol::parse_response_text<T>(response);
    if(!parsed.has_value()) {
        co_await eventide::fail(std::move(parsed.error()));
    }
    co_return std::move(*parsed);
}

template <typename T>
auto call_structured(std::string_view model,
                     std::string_view system_prompt,
                     std::string_view prompt)
    -> std::expected<T, LLMError> {
    return detail::run_task_sync<T>(
        [=](async::event_loop& loop) {
            return call_structured_async<T>(model, system_prompt, prompt, loop);
        });
}

}  // namespace clore::net