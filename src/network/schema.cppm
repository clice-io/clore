module;

#include "kota/codec/json/error.h"
#include "kota/codec/json/json.h"
#include "kota/meta/attrs.h"
#include "kota/meta/name.h"
#include "kota/meta/struct.h"

export module schema;

import std;
import http;
import protocol;
import support;

namespace clore::net::openai::schema::detail {

namespace json = kota::codec::json;
namespace meta = kota::meta;
namespace meta_attrs = kota::meta::attrs;

template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template <typename T>
constexpr bool is_optional_v = is_optional<std::remove_cvref_t<T>>::value;

template <typename T>
struct optional_inner;

template <typename T>
struct optional_inner<std::optional<T>> {
    using type = T;
};

template <typename T>
using optional_inner_t = typename optional_inner<std::remove_cvref_t<T>>::type;

template <typename T>
struct is_vector : std::false_type {};

template <typename T, typename Allocator>
struct is_vector<std::vector<T, Allocator>> : std::true_type {};

template <typename T>
constexpr bool is_vector_v = is_vector<std::remove_cvref_t<T>>::value;

template <typename T>
struct vector_inner;

template <typename T, typename Allocator>
struct vector_inner<std::vector<T, Allocator>> {
    using type = T;
};

template <typename T>
using vector_inner_t = typename vector_inner<std::remove_cvref_t<T>>::type;

template <typename T>
struct is_array : std::false_type {};

template <typename T, std::size_t N>
struct is_array<std::array<T, N>> : std::true_type {};

template <typename T>
constexpr bool is_array_v = is_array<std::remove_cvref_t<T>>::value;

template <typename T>
struct array_inner;

template <typename T, std::size_t N>
struct array_inner<std::array<T, N>> {
    using type = T;
};

template <typename T>
using array_inner_t = typename array_inner<std::remove_cvref_t<T>>::type;

template <typename T>
struct schema_subject {
    using type = std::remove_cvref_t<T>;
};

template <typename T>
    requires meta::annotated_type<std::remove_cvref_t<T>> &&
             (!meta::reflectable_class<std::remove_cvref_t<T>>)
struct schema_subject<T> {
    using type = meta::annotated_underlying_t<std::remove_cvref_t<T>>;
};

template <typename T>
using schema_subject_t = typename schema_subject<T>::type;

auto sanitize_schema_name(std::string_view raw_name) -> std::string {
    std::string sanitized;
    sanitized.reserve(raw_name.size());
    for(char ch: raw_name) {
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
    auto sanitized = sanitize_schema_name(meta::type_name<T>());
    if(sanitized.empty()) {
        return std::unexpected(LLMError("generated schema name is empty"));
    }
    return sanitized;
}

template <typename T>
auto make_schema_value() -> std::expected<json::Value, LLMError>;

template <typename T>
auto make_schema_object() -> std::expected<json::Object, LLMError> {
    auto value = make_schema_value<T>();
    if(!value.has_value()) {
        return std::unexpected(std::move(value.error()));
    }

    auto* object = value->get_object();
    if(object == nullptr) {
        return std::unexpected(LLMError("generated schema root is not an object"));
    }
    return json::Object(*object);
}

template <typename T>
auto make_scalar_type_schema(std::string_view type_name) -> std::expected<json::Value, LLMError> {
    auto object = clore::net::detail::make_empty_object("failed to create scalar schema object");
    if(!object.has_value()) {
        return std::unexpected(std::move(object.error()));
    }
    object->insert("type", std::string(type_name));
    return json::Value(std::move(*object));
}

template <typename T>
auto make_any_of_schema(std::vector<json::Value> choices) -> std::expected<json::Value, LLMError> {
    auto object = clore::net::detail::make_empty_object("failed to create anyOf schema object");
    if(!object.has_value()) {
        return std::unexpected(std::move(object.error()));
    }
    auto any_of = clore::net::detail::make_empty_array("failed to create anyOf schema array");
    if(!any_of.has_value()) {
        return std::unexpected(std::move(any_of.error()));
    }
    for(auto& choice: choices) {
        any_of->push_back(std::move(choice));
    }
    object->insert("anyOf", std::move(*any_of));
    return json::Value(std::move(*object));
}

template <typename Object, std::size_t... Indices>
auto populate_object_schema(json::Object& object, std::index_sequence<Indices...>)
    -> std::expected<void, LLMError> {
    constexpr bool fields_valid = meta_attrs::validate_field_schema<Object>();
    static_assert(fields_valid, "schema field names contain collisions or alias conflicts");

    auto properties =
        clore::net::detail::make_empty_object("failed to create schema properties object");
    if(!properties.has_value()) {
        return std::unexpected(std::move(properties.error()));
    }
    auto required = clore::net::detail::make_empty_array("failed to create schema required array");
    if(!required.has_value()) {
        return std::unexpected(std::move(required.error()));
    }

    auto append_field = [&](auto index_constant) -> std::expected<void, LLMError> {
        constexpr std::size_t index = decltype(index_constant)::value;
        constexpr auto field_schema = meta_attrs::resolve_field<Object, index>();
        if constexpr(field_schema.is_skipped) {
            return {};
        } else if constexpr(field_schema.is_flattened) {
            return std::unexpected(LLMError(
                "flatten fields are not supported by " "automatic OpenAI schema generation"));
        } else {
            using field_type = meta::field_type<Object, index>;
            auto field_value = make_schema_value<field_type>();
            if(!field_value.has_value()) {
                return std::unexpected(std::move(field_value.error()));
            }

            properties->insert(std::string(field_schema.canonical_name), std::move(*field_value));
            required->push_back(std::string(field_schema.canonical_name));
            return {};
        }
    };

    auto statuses = std::array<std::expected<void, LLMError>, sizeof...(Indices)>{
        append_field(std::integral_constant<std::size_t, Indices>{})...};
    for(auto& status: statuses) {
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
    }

    object.insert("type", "object");
    object.insert("properties", std::move(*properties));
    object.insert("required", std::move(*required));
    object.insert("additionalProperties", false);
    return {};
}

template <typename T>
auto make_schema_value() -> std::expected<json::Value, LLMError> {
    using schema_type = schema_subject_t<T>;

    if constexpr(std::same_as<schema_type, std::string> ||
                 std::same_as<schema_type, std::string_view>) {
        return make_scalar_type_schema<schema_type>("string");
    } else if constexpr(std::same_as<schema_type, bool>) {
        return make_scalar_type_schema<schema_type>("boolean");
    } else if constexpr(std::integral<schema_type> && !std::same_as<schema_type, bool>) {
        return make_scalar_type_schema<schema_type>("integer");
    } else if constexpr(std::floating_point<schema_type>) {
        return make_scalar_type_schema<schema_type>("number");
    } else if constexpr(is_optional_v<schema_type>) {
        auto inner = make_schema_value<optional_inner_t<schema_type>>();
        if(!inner.has_value()) {
            return std::unexpected(std::move(inner.error()));
        }
        auto null_value = make_scalar_type_schema<std::nullptr_t>("null");
        if(!null_value.has_value()) {
            return std::unexpected(std::move(null_value.error()));
        }
        return make_any_of_schema<schema_type>({std::move(*inner), std::move(*null_value)});
    } else if constexpr(is_vector_v<schema_type>) {
        auto item_schema = make_schema_value<vector_inner_t<schema_type>>();
        if(!item_schema.has_value()) {
            return std::unexpected(std::move(item_schema.error()));
        }
        auto object = clore::net::detail::make_empty_object("failed to create array schema object");
        if(!object.has_value()) {
            return std::unexpected(std::move(object.error()));
        }
        object->insert("type", "array");
        object->insert("items", std::move(*item_schema));
        return json::Value(std::move(*object));
    } else if constexpr(is_array_v<schema_type>) {
        constexpr auto fixed_size = std::tuple_size_v<schema_type>;
        auto item_schema = make_schema_value<array_inner_t<schema_type>>();
        if(!item_schema.has_value()) {
            return std::unexpected(std::move(item_schema.error()));
        }
        auto object =
            clore::net::detail::make_empty_object("failed to create fixed array schema object");
        if(!object.has_value()) {
            return std::unexpected(std::move(object.error()));
        }
        object->insert("type", "array");
        object->insert("items", std::move(*item_schema));
        object->insert("minItems", static_cast<std::uint64_t>(fixed_size));
        object->insert("maxItems", static_cast<std::uint64_t>(fixed_size));
        return json::Value(std::move(*object));
    } else if constexpr(meta::reflectable_class<schema_type>) {
        auto object = clore::net::detail::make_empty_object("failed to create object schema root");
        if(!object.has_value()) {
            return std::unexpected(std::move(object.error()));
        }
        auto status = populate_object_schema<schema_type>(
            *object,
            std::make_index_sequence<meta::field_count<schema_type>()>{});
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
        return json::Value(std::move(*object));
    } else {
        static_assert(sizeof(schema_type) == 0,
                  "automatic OpenAI schema generation only supports "
                  "reflectable objects, strings, booleans, numbers, "
                  "std::optional, std::vector, and std::array");
    }
}

auto validate_schema_array_of_types(const json::Array& array, std::string_view path, bool is_root)
    -> std::expected<void, LLMError> {
    std::optional<std::string_view> primary_type;
    bool saw_null = false;
    for(auto value: array) {
        auto type = clore::net::detail::expect_string(value, std::format("{}.type[]", path));
        if(!type.has_value()) {
            return std::unexpected(std::move(type.error()));
        }
        if(*type == "null") {
            saw_null = true;
            continue;
        }
        if(primary_type.has_value()) {
            return std::unexpected(LLMError(std::format(
                "{} uses an unsupported multi-type union; only " "a single non-null type plus null is supported",
                path)));
        }
        primary_type = *type;
    }

    if(is_root) {
        return std::unexpected(LLMError("root schema must be an object and cannot be nullable"));
    }

    if(!primary_type.has_value() || !saw_null) {
        return std::unexpected(LLMError(
            std::format("{} type union must contain exactly one concrete type and 'null'", path)));
    }

    return {};
}

auto validate_openai_schema(const json::Object& object, std::string_view path, bool is_root)
    -> std::expected<void, LLMError>;

auto validate_openai_schema_value(const json::Value& value, std::string_view path, bool is_root)
    -> std::expected<void, LLMError> {
    auto object = clore::net::detail::expect_object(value, path);
    if(!object.has_value()) {
        return std::unexpected(std::move(object.error()));
    }
    return validate_openai_schema(**object, path, is_root);
}

auto validate_openai_schema_value(json::Cursor value, std::string_view path, bool is_root)
    -> std::expected<void, LLMError> {
    auto object = clore::net::detail::expect_object(value, path);
    if(!object.has_value()) {
        return std::unexpected(std::move(object.error()));
    }
    return validate_openai_schema(**object, path, is_root);
}

auto validate_required_properties(clore::net::detail::ObjectView properties,
                                  clore::net::detail::ArrayView required,
                                  std::string_view path) -> std::expected<void, LLMError> {
    std::unordered_set<std::string> required_names;
    for(auto value: required) {
        auto name = clore::net::detail::expect_string(value, std::format("{}.required[]", path));
        if(!name.has_value()) {
            return std::unexpected(std::move(name.error()));
        }
        required_names.emplace(*name);
    }

    for(auto entry: properties) {
        if(!required_names.contains(std::string(entry.key))) {
            return std::unexpected(LLMError(std::format(
                "{} property '{}' must be listed in required " "when using strict structured output",
                path,
                entry.key)));
        }
    }

    return {};
}

auto validate_openai_schema(const json::Object& object, std::string_view path, bool is_root)
    -> std::expected<void, LLMError> {
    auto object_view = clore::net::detail::ObjectView{.value = &object};
    auto any_of_value = object_view.get("anyOf");
    if(any_of_value.has_value()) {
        if(is_root) {
            return std::unexpected(LLMError("root schema must not use anyOf"));
        }
        auto any_of =
            clore::net::detail::expect_array(*any_of_value, std::format("{}.anyOf", path));
        if(!any_of.has_value()) {
            return std::unexpected(std::move(any_of.error()));
        }
        if(any_of->empty()) {
            return std::unexpected(
                LLMError(std::format("{}.anyOf must contain at least one schema", path)));
        }
        for(std::size_t index = 0; index < any_of->size(); ++index) {
            auto status = validate_openai_schema_value((*any_of)[index],
                                                       std::format("{}.anyOf[{}]", path, index),
                                                       false);
            if(!status.has_value()) {
                return std::unexpected(std::move(status.error()));
            }
        }
        return {};
    }

    auto type_value = object_view.get("type");
    if(!type_value.has_value()) {
        return std::unexpected(LLMError(std::format("{} schema is missing a 'type' field", path)));
    }

    std::optional<std::string_view> schema_type;
    if(auto type_string = type_value->get_string(); type_string.has_value()) {
        schema_type = *type_string;
    } else if(auto type_array = type_value->get_array(); type_array != nullptr) {
        auto status = validate_schema_array_of_types(*type_array, path, is_root);
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
        for(auto value: *type_array) {
            auto type_name = value.get_string();
            if(type_name.has_value() && *type_name != "null") {
                schema_type = *type_name;
                break;
            }
        }
    } else {
        return std::unexpected(
            LLMError(std::format("{}.type must be a string or string array", path)));
    }

    if(!schema_type.has_value()) {
        return std::unexpected(LLMError(std::format("{} schema type could not be resolved", path)));
    }

    if(is_root && *schema_type != "object") {
        return std::unexpected(LLMError("root schema must be an object"));
    }

    if(*schema_type == "object") {
        auto properties_value = object_view.get("properties");
        if(!properties_value.has_value()) {
            return std::unexpected(
                LLMError(std::format("{} object schema is missing properties", path)));
        }
        auto properties = clore::net::detail::expect_object(*properties_value,
                                                            std::format("{}.properties", path));
        if(!properties.has_value()) {
            return std::unexpected(std::move(properties.error()));
        }

        auto required_value = object_view.get("required");
        if(!required_value.has_value()) {
            return std::unexpected(
                LLMError(std::format("{} object schema is missing required", path)));
        }
        auto required =
            clore::net::detail::expect_array(*required_value, std::format("{}.required", path));
        if(!required.has_value()) {
            return std::unexpected(std::move(required.error()));
        }

        auto additional_properties_value = object_view.get("additionalProperties");
        if(!additional_properties_value.has_value()) {
            return std::unexpected(LLMError(
                std::format("{} object schema must set additionalProperties to false", path)));
        }
        auto additional_properties = additional_properties_value->get_bool();
        if(!additional_properties.has_value() || *additional_properties) {
            return std::unexpected(LLMError(
                std::format("{} object schema must set additionalProperties to false", path)));
        }

        auto required_status = validate_required_properties(*properties, *required, path);
        if(!required_status.has_value()) {
            return std::unexpected(std::move(required_status.error()));
        }

        for(auto entry: *properties) {
            auto status =
                validate_openai_schema_value(entry.value,
                                             std::format("{}.properties.{}", path, entry.key),
                                             false);
            if(!status.has_value()) {
                return std::unexpected(std::move(status.error()));
            }
        }
    } else if(*schema_type == "array") {
        auto items_value = object_view.get("items");
        if(!items_value.has_value()) {
            return std::unexpected(LLMError(std::format("{} array schema is missing items", path)));
        }
        auto status =
            validate_openai_schema_value(*items_value, std::format("{}.items", path), false);
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
    }

    if(auto defs_value = object_view.get("$defs"); defs_value.has_value()) {
        auto defs = clore::net::detail::expect_object(*defs_value, std::format("{}.$defs", path));
        if(!defs.has_value()) {
            return std::unexpected(std::move(defs.error()));
        }
        for(auto entry: *defs) {
            auto status = validate_openai_schema_value(entry.value,
                                                       std::format("{}.$defs.{}", path, entry.key),
                                                       false);
            if(!status.has_value()) {
                return std::unexpected(std::move(status.error()));
            }
        }
    }

    return {};
}

}  // namespace clore::net::openai::schema::detail

export namespace clore::net::schema {

template <typename T>
auto response_format() -> std::expected<ResponseFormat, LLMError>;

template <typename T>
auto function_tool(std::string name, std::string description)
    -> std::expected<FunctionToolDefinition, LLMError>;

}  // namespace clore::net::schema

export namespace clore::net::detail {

auto validate_response_format(const ResponseFormat& format) -> std::expected<void, LLMError>;

auto validate_tool_definition(const FunctionToolDefinition& tool) -> std::expected<void, LLMError>;

}  // namespace clore::net::detail

namespace clore::net::detail {

auto validate_response_format(const ResponseFormat& format) -> std::expected<void, LLMError> {
    if(!format.schema.has_value()) {
        return {};
    }
    if(format.name.empty()) {
        return std::unexpected(LLMError("response_format.name must not be empty"));
    }
    return openai::schema::detail::validate_openai_schema(*format.schema, format.name, true);
}

auto validate_tool_definition(const FunctionToolDefinition& tool) -> std::expected<void, LLMError> {
    if(tool.name.empty()) {
        return std::unexpected(LLMError("tool name must not be empty"));
    }
    if(tool.description.empty()) {
        return std::unexpected(
            LLMError(std::format("tool '{}' description must not be empty", tool.name)));
    }
    return openai::schema::detail::validate_openai_schema(tool.parameters, tool.name, true);
}

}  // namespace clore::net::detail

namespace clore::net::schema {

template <typename T>
auto response_format() -> std::expected<ResponseFormat, LLMError> {
    using root_type = openai::schema::detail::schema_subject_t<T>;
    static_assert(kota::meta::reflectable_class<root_type>,
                  "automatic structured output schemas require a reflectable " "root object type");

    auto name = openai::schema::detail::schema_type_name<root_type>();
    if(!name.has_value()) {
        return std::unexpected(std::move(name.error()));
    }

    auto schema = openai::schema::detail::make_schema_object<root_type>();
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
    using root_type = openai::schema::detail::schema_subject_t<T>;
    static_assert(kota::meta::reflectable_class<root_type>,
                  "automatic function tool schemas require a reflectable root object type");

    if(name.empty()) {
        return std::unexpected(LLMError("tool name must not be empty"));
    }
    if(description.empty()) {
        return std::unexpected(LLMError("tool description must not be empty"));
    }

    auto schema = openai::schema::detail::make_schema_object<root_type>();
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
