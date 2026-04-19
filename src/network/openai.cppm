module;

#include "kota/async/async.h"
#include "kota/codec/json/error.h"
#include "kota/codec/json/json.h"

export module openai;

import std;
import http;
import protocol;
import schema;
import client;
import support;

// ── protocol serialization ──────────────────────────────────────────────
namespace clore::net::openai_protocol_detail {

namespace json = kota::codec::json;

template <typename T>
using remove_cvref_t = std::remove_cvref_t<T>;

auto validate_request(const CompletionRequest& request) -> std::expected<void, LLMError> {
    if(request.model.empty()) {
        return std::unexpected(LLMError("request model must not be empty"));
    }
    if(request.messages.empty()) {
        return std::unexpected(LLMError("request messages must not be empty"));
    }

    if(request.response_format.has_value()) {
        auto status = clore::net::detail::validate_response_format(*request.response_format);
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
    }

    for(auto& tool: request.tools) {
        auto status = clore::net::detail::validate_tool_definition(tool);
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
            for(auto& tool: request.tools) {
                if(tool.name == forced->name) {
                    exists = true;
                    break;
                }
            }
            if(!exists) {
                return std::unexpected(LLMError(
                    std::format("forced tool '{}' is not present in request.tools", forced->name)));
            }
        }
    }

    for(auto& message: request.messages) {
        auto status = std::visit(
            [](const auto& current) -> std::expected<void, LLMError> {
                using message_type = remove_cvref_t<decltype(current)>;
                if constexpr(std::same_as<message_type, AssistantToolCallMessage>) {
                    if(!current.content.has_value() && current.tool_calls.empty()) {
                        return std::unexpected(LLMError(
                            "assistant tool-call message must contain content " "or tool_calls"));
                    }
                    std::unordered_set<std::string> ids;
                    for(auto& call: current.tool_calls) {
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
            },
            message);
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
    }

    return {};
}

auto serialize_message(json::Document&, json::Array& out, const Message& message)
    -> std::expected<void, LLMError> {
    auto object = clore::net::detail::make_empty_object("failed to create request message object");
    if(!object.has_value()) {
        return std::unexpected(std::move(object.error()));
    }

    auto status = std::visit(
        [&](const auto& current) -> std::expected<void, LLMError> {
            using message_type = remove_cvref_t<decltype(current)>;
            if constexpr(std::same_as<message_type, SystemMessage>) {
                if(auto result = object->insert("role", "system"); !result.has_value()) {
                    return clore::net::detail::to_llm_unexpected(result.error(),
                                                                 "failed to serialize system role");
                }
                auto content =
                    clore::net::detail::normalize_utf8(current.content, "system content");
                auto result =
                    clore::net::detail::insert_string_field(*object,
                                                            "content",
                                                            std::string_view(content),
                                                            "failed to serialize system content");
                if(!result.has_value()) {
                    return std::unexpected(std::move(result.error()));
                }
            } else if constexpr(std::same_as<message_type, UserMessage>) {
                if(auto result = object->insert("role", "user"); !result.has_value()) {
                    return clore::net::detail::to_llm_unexpected(result.error(),
                                                                 "failed to serialize user role");
                }
                auto content = clore::net::detail::normalize_utf8(current.content, "user content");
                auto result =
                    clore::net::detail::insert_string_field(*object,
                                                            "content",
                                                            std::string_view(content),
                                                            "failed to serialize user content");
                if(!result.has_value()) {
                    return std::unexpected(std::move(result.error()));
                }
            } else if constexpr(std::same_as<message_type, AssistantMessage>) {
                if(auto result = object->insert("role", "assistant"); !result.has_value()) {
                    return clore::net::detail::to_llm_unexpected(
                        result.error(),
                        "failed to serialize assistant role");
                }
                auto content =
                    clore::net::detail::normalize_utf8(current.content, "assistant content");
                auto result = clore::net::detail::insert_string_field(
                    *object,
                    "content",
                    std::string_view(content),
                    "failed to serialize assistant content");
                if(!result.has_value()) {
                    return std::unexpected(std::move(result.error()));
                }
            } else if constexpr(std::same_as<message_type, AssistantToolCallMessage>) {
                if(auto result = object->insert("role", "assistant"); !result.has_value()) {
                    return clore::net::detail::to_llm_unexpected(
                        result.error(),
                        "failed to serialize assistant role");
                }
                if(current.content.has_value()) {
                    auto content =
                        clore::net::detail::normalize_utf8(*current.content, "assistant content");
                    auto result = clore::net::detail::insert_string_field(
                        *object,
                        "content",
                        std::string_view(content),
                        "failed to serialize assistant content");
                    if(!result.has_value()) {
                        return std::unexpected(std::move(result.error()));
                    }
                }
                if(!current.tool_calls.empty()) {
                    auto tool_calls = clore::net::detail::make_empty_array(
                        "failed to create assistant tool_calls array");
                    if(!tool_calls.has_value()) {
                        return std::unexpected(std::move(tool_calls.error()));
                    }
                    for(auto& tool_call: current.tool_calls) {
                        auto call_object = clore::net::detail::make_empty_object(
                            "failed to create tool call object");
                        if(!call_object.has_value()) {
                            return std::unexpected(std::move(call_object.error()));
                        }
                        auto function_object = clore::net::detail::make_empty_object(
                            "failed to create tool call function object");
                        if(!function_object.has_value()) {
                            return std::unexpected(std::move(function_object.error()));
                        }
                        auto call_id = clore::net::detail::insert_string_field(
                            *call_object,
                            "id",
                            std::string_view(tool_call.id),
                            "failed to serialize tool call id");
                        if(!call_id.has_value()) {
                            return std::unexpected(std::move(call_id.error()));
                        }
                        if(auto result = call_object->insert("type", "function");
                           !result.has_value()) {
                            return clore::net::detail::to_llm_unexpected(
                                result.error(),
                                "failed to serialize tool call type");
                        }
                        auto function_name = clore::net::detail::insert_string_field(
                            *function_object,
                            "name",
                            std::string_view(tool_call.name),
                            "failed to serialize tool call function name");
                        if(!function_name.has_value()) {
                            return std::unexpected(std::move(function_name.error()));
                        }
                        auto arguments =
                            clore::net::detail::normalize_utf8(tool_call.arguments_json,
                                                               "tool call arguments");
                        auto function_arguments = clore::net::detail::insert_string_field(
                            *function_object,
                            "arguments",
                            std::string_view(arguments),
                            "failed to serialize tool call arguments");
                        if(!function_arguments.has_value()) {
                            return std::unexpected(std::move(function_arguments.error()));
                        }
                        if(auto result =
                               call_object->insert("function", std::move(*function_object));
                           !result.has_value()) {
                            return clore::net::detail::to_llm_unexpected(
                                result.error(),
                                "failed to serialize tool call function payload");
                        }
                        if(auto result = tool_calls->push_back(std::move(*call_object));
                           !result.has_value()) {
                            return clore::net::detail::to_llm_unexpected(
                                result.error(),
                                "failed to append tool call");
                        }
                    }
                    if(auto result = object->insert("tool_calls", std::move(*tool_calls));
                       !result.has_value()) {
                        return clore::net::detail::to_llm_unexpected(
                            result.error(),
                            "failed to serialize assistant tool_calls");
                    }
                }
            } else if constexpr(std::same_as<message_type, ToolResultMessage>) {
                if(auto result = object->insert("role", "tool"); !result.has_value()) {
                    return clore::net::detail::to_llm_unexpected(result.error(),
                                                                 "failed to serialize tool role");
                }
                auto tool_call_id =
                    clore::net::detail::insert_string_field(*object,
                                                            "tool_call_id",
                                                            std::string_view(current.tool_call_id),
                                                            "failed to serialize tool_call_id");
                if(!tool_call_id.has_value()) {
                    return std::unexpected(std::move(tool_call_id.error()));
                }
                auto content = clore::net::detail::normalize_utf8(current.content, "tool content");
                auto result =
                    clore::net::detail::insert_string_field(*object,
                                                            "content",
                                                            std::string_view(content),
                                                            "failed to serialize tool content");
                if(!result.has_value()) {
                    return std::unexpected(std::move(result.error()));
                }
            }
            return {};
        },
        message);

    if(!status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }

    auto push_status = out.push_back(std::move(*object));
    if(!push_status.has_value()) {
        return clore::net::detail::to_llm_unexpected(push_status.error(),
                                                     "failed to append request message");
    }
    return {};
}

auto serialize_tool_choice(json::Object& root, const ToolChoice& choice)
    -> std::expected<void, LLMError> {
    return std::visit(
        [&](const auto& current) -> std::expected<void, LLMError> {
            using choice_type = remove_cvref_t<decltype(current)>;
            if constexpr(std::same_as<choice_type, ToolChoiceAuto>) {
                auto status = root.insert("tool_choice", "auto");
                if(!status.has_value()) {
                    return clore::net::detail::to_llm_unexpected(status.error(),
                                                                 "failed to serialize tool_choice");
                }
                return {};
            } else if constexpr(std::same_as<choice_type, ToolChoiceRequired>) {
                auto status = root.insert("tool_choice", "required");
                if(!status.has_value()) {
                    return clore::net::detail::to_llm_unexpected(status.error(),
                                                                 "failed to serialize tool_choice");
                }
                return {};
            } else if constexpr(std::same_as<choice_type, ToolChoiceNone>) {
                auto status = root.insert("tool_choice", "none");
                if(!status.has_value()) {
                    return clore::net::detail::to_llm_unexpected(status.error(),
                                                                 "failed to serialize tool_choice");
                }
                return {};
            } else {
                auto object = clore::net::detail::make_empty_object(
                    "failed to serialize forced tool choice object");
                if(!object.has_value()) {
                    return std::unexpected(std::move(object.error()));
                }
                auto function_object = clore::net::detail::make_empty_object(
                    "failed to serialize forced tool choice function object");
                if(!function_object.has_value()) {
                    return std::unexpected(std::move(function_object.error()));
                }
                if(auto status = object->insert("type", "function"); !status.has_value()) {
                    return clore::net::detail::to_llm_unexpected(
                        status.error(),
                        "failed to serialize forced tool choice");
                }
                auto status =
                    clore::net::detail::insert_string_field(*function_object,
                                                            "name",
                                                            std::string_view(current.name),
                                                            "failed to serialize forced tool name");
                if(!status.has_value()) {
                    return std::unexpected(std::move(status.error()));
                }
                if(auto root_status = object->insert("function", std::move(*function_object));
                   !root_status.has_value()) {
                    return clore::net::detail::to_llm_unexpected(
                        root_status.error(),
                        "failed to serialize forced tool payload");
                }
                auto root_status = root.insert("tool_choice", std::move(*object));
                if(!root_status.has_value()) {
                    return clore::net::detail::to_llm_unexpected(root_status.error(),
                                                                 "failed to serialize tool_choice");
                }
                return {};
            }
        },
        choice);
}

auto serialize_response_format(json::Document& document,
                               json::Object& root,
                               const ResponseFormat& format) -> std::expected<void, LLMError> {
    auto object = clore::net::detail::make_empty_object("failed to create response_format object");
    if(!object.has_value()) {
        return std::unexpected(std::move(object.error()));
    }
    auto schema_object =
        clore::net::detail::make_empty_object("failed to create response_format schema object");
    if(!schema_object.has_value()) {
        return std::unexpected(std::move(schema_object.error()));
    }

    if(!format.schema.has_value()) {
        if(auto status = object->insert("type", "json_object"); !status.has_value()) {
            return std::unexpected(
                LLMError(std::format("failed to serialize response_format.type: {}",
                                     json::error_message(status.error()))));
        }
    } else {
        if(auto status = object->insert("type", "json_schema"); !status.has_value()) {
            return std::unexpected(
                LLMError(std::format("failed to serialize response_format.type: {}",
                                     json::error_message(status.error()))));
        }
        auto name_status =
            clore::net::detail::insert_string_field(*schema_object,
                                                    "name",
                                                    std::string_view(format.name),
                                                    "failed to serialize response_format name");
        if(!name_status.has_value()) {
            return std::unexpected(std::move(name_status.error()));
        }
        if(auto status = schema_object->insert("strict", format.strict); !status.has_value()) {
            return std::unexpected(
                LLMError(std::format("failed to serialize response_format strict flag: {}",
                                     json::error_message(status.error()))));
        }
        auto copied_schema =
            clore::net::detail::clone_object(document,
                                             *format.schema,
                                             "failed to serialize response_format schema");
        if(!copied_schema.has_value()) {
            return std::unexpected(std::move(copied_schema.error()));
        }
        if(auto status = schema_object->insert("schema", std::move(*copied_schema));
           !status.has_value()) {
            return std::unexpected(
                LLMError(std::format("failed to serialize response_format schema: {}",
                                     json::error_message(status.error()))));
        }
        if(auto status = object->insert("json_schema", std::move(*schema_object));
           !status.has_value()) {
            return std::unexpected(
                LLMError(std::format("failed to serialize response_format json_schema payload: {}",
                                     json::error_message(status.error()))));
        }
    }

    auto root_status = root.insert("response_format", std::move(*object));
    if(!root_status.has_value()) {
        return clore::net::detail::to_llm_unexpected(root_status.error(),
                                                     "failed to serialize response_format");
    }
    return {};
}

auto serialize_tool_definition(json::Document& document,
                               json::Array& tools,
                               const FunctionToolDefinition& tool)
    -> std::expected<void, LLMError> {
    auto object = clore::net::detail::make_empty_object("failed to create tool definition object");
    if(!object.has_value()) {
        return std::unexpected(std::move(object.error()));
    }
    auto function_object =
        clore::net::detail::make_empty_object("failed to create function tool object");
    if(!function_object.has_value()) {
        return std::unexpected(std::move(function_object.error()));
    }

    if(auto status = object->insert("type", "function"); !status.has_value()) {
        return std::unexpected(LLMError(
            std::format("failed to serialize tool type: {}", json::error_message(status.error()))));
    }
    auto name_status = clore::net::detail::insert_string_field(*function_object,
                                                               "name",
                                                               std::string_view(tool.name),
                                                               "failed to serialize tool name");
    if(!name_status.has_value()) {
        return std::unexpected(std::move(name_status.error()));
    }
    auto description_status =
        clore::net::detail::insert_string_field(*function_object,
                                                "description",
                                                std::string_view(tool.description),
                                                "failed to serialize tool description");
    if(!description_status.has_value()) {
        return std::unexpected(std::move(description_status.error()));
    }
    auto copied_parameters =
        clore::net::detail::clone_object(document,
                                         tool.parameters,
                                         "failed to serialize tool parameters");
    if(!copied_parameters.has_value()) {
        return std::unexpected(std::move(copied_parameters.error()));
    }
    if(auto status = function_object->insert("parameters", std::move(*copied_parameters));
       !status.has_value()) {
        return std::unexpected(LLMError(std::format("failed to serialize tool parameters: {}",
                                                    json::error_message(status.error()))));
    }
    if(auto status = function_object->insert("strict", tool.strict); !status.has_value()) {
        return std::unexpected(LLMError(std::format("failed to serialize tool strict flag: {}",
                                                    json::error_message(status.error()))));
    }
    if(auto status = object->insert("function", std::move(*function_object)); !status.has_value()) {
        return std::unexpected(LLMError(std::format("failed to serialize function tool payload: {}",
                                                    json::error_message(status.error()))));
    }

    auto push_status = tools.push_back(std::move(*object));
    if(!push_status.has_value()) {
        return clore::net::detail::to_llm_unexpected(push_status.error(),
                                                     "failed to append request tool");
    }
    return {};
}

auto parse_content_parts(json::ArrayRef parts) -> std::expected<AssistantOutput, LLMError> {
    AssistantOutput output;
    std::string text;
    std::string refusal;
    bool saw_text = false;
    bool saw_refusal = false;

    for(auto value: parts) {
        auto part = clore::net::detail::expect_object(value, "message.content[]");
        if(!part.has_value()) {
            return std::unexpected(std::move(part.error()));
        }

        auto type_value = part->get("type");
        std::string_view type = "text";
        if(type_value.has_value()) {
            auto parsed_type =
                clore::net::detail::expect_string(*type_value, "message.content[].type");
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
            auto parsed_refusal =
                clore::net::detail::expect_string(*refusal_value, "message.content[].refusal");
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
            return std::unexpected(LLMError("text content part is missing the text payload"));
        }

        if(auto part_text = text_value->get_string(); part_text.has_value()) {
            text += *part_text;
            saw_text = true;
            continue;
        }

        auto text_object = clore::net::detail::expect_object(*text_value, "message.content[].text");
        if(!text_object.has_value()) {
            return std::unexpected(std::move(text_object.error()));
        }
        auto value_field = text_object->get("value");
        if(!value_field.has_value()) {
            return std::unexpected(LLMError("text content object is missing the value field"));
        }
        auto part_text =
            clore::net::detail::expect_string(*value_field, "message.content[].text.value");
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

auto parse_tool_calls(json::ArrayRef calls) -> std::expected<std::vector<ToolCall>, LLMError> {
    std::vector<ToolCall> parsed_calls;
    parsed_calls.reserve(calls.size());
    std::unordered_set<std::string> ids;

    for(auto value: calls) {
        auto call = clore::net::detail::expect_object(value, "message.tool_calls[]");
        if(!call.has_value()) {
            return std::unexpected(std::move(call.error()));
        }

        auto id_value = call->get("id");
        if(!id_value.has_value()) {
            return std::unexpected(LLMError("tool call is missing id"));
        }
        auto id = clore::net::detail::expect_string(*id_value, "message.tool_calls[].id");
        if(!id.has_value()) {
            return std::unexpected(std::move(id.error()));
        }
        if(!ids.emplace(std::string(*id)).second) {
            return std::unexpected(LLMError(std::format("duplicate tool call id '{}'", *id)));
        }

        auto type_value = call->get("type");
        if(!type_value.has_value()) {
            return std::unexpected(LLMError("tool call is missing type"));
        }
        auto type = clore::net::detail::expect_string(*type_value, "message.tool_calls[].type");
        if(!type.has_value()) {
            return std::unexpected(std::move(type.error()));
        }
        if(*type != "function") {
            return std::unexpected(LLMError(std::format("unsupported tool call type '{}'", *type)));
        }

        auto function_value = call->get("function");
        if(!function_value.has_value()) {
            return std::unexpected(LLMError("tool call is missing function payload"));
        }
        auto function =
            clore::net::detail::expect_object(*function_value, "message.tool_calls[].function");
        if(!function.has_value()) {
            return std::unexpected(std::move(function.error()));
        }

        auto name_value = function->get("name");
        if(!name_value.has_value()) {
            return std::unexpected(LLMError("tool call function is missing name"));
        }
        auto name =
            clore::net::detail::expect_string(*name_value, "message.tool_calls[].function.name");
        if(!name.has_value()) {
            return std::unexpected(std::move(name.error()));
        }

        auto arguments_value = function->get("arguments");
        if(!arguments_value.has_value()) {
            return std::unexpected(LLMError("tool call function is missing arguments"));
        }
        auto arguments_json =
            clore::net::detail::expect_string(*arguments_value,
                                              "message.tool_calls[].function.arguments");
        if(!arguments_json.has_value()) {
            return std::unexpected(std::move(arguments_json.error()));
        }
        auto parsed_arguments = json::Value::parse(*arguments_json);
        if(!parsed_arguments.has_value()) {
            return std::unexpected(LLMError(
                std::format("failed to parse tool call arguments for '{}': {}",
                            *name,
                            json::error_message(json::make_read_error(parsed_arguments.error())))));
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

}  // namespace clore::net::openai_protocol_detail

export namespace clore::net::protocol {

auto build_request_json(const CompletionRequest& request) -> std::expected<std::string, LLMError>;

auto parse_response(std::string_view json) -> std::expected<CompletionResponse, LLMError>;

}  // namespace clore::net::protocol

namespace clore::net::protocol {

auto build_request_json(const CompletionRequest& request) -> std::expected<std::string, LLMError> {
    auto validation = openai_protocol_detail::validate_request(request);
    if(!validation.has_value()) {
        return std::unexpected(std::move(validation.error()));
    }

    kota::codec::json::Document document;
    auto root = clore::net::detail::make_empty_object("failed to create request root object");
    if(!root.has_value()) {
        return std::unexpected(std::move(root.error()));
    }
    auto messages = clore::net::detail::make_empty_array("failed to create request messages array");
    if(!messages.has_value()) {
        return std::unexpected(std::move(messages.error()));
    }

    if(auto status = root->insert("model", std::string_view(request.model)); !status.has_value()) {
        return clore::net::detail::to_llm_unexpected(status.error(), "failed to serialize model");
    }

    for(auto& message: request.messages) {
        auto status = openai_protocol_detail::serialize_message(document, *messages, message);
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
    }
    if(auto status = root->insert("messages", std::move(*messages)); !status.has_value()) {
        return clore::net::detail::to_llm_unexpected(status.error(),
                                                     "failed to serialize request messages");
    }

    if(request.response_format.has_value()) {
        auto response_format =
            openai_protocol_detail::serialize_response_format(document,
                                                              *root,
                                                              *request.response_format);
        if(!response_format.has_value()) {
            return std::unexpected(std::move(response_format.error()));
        }
    }

    if(!request.tools.empty()) {
        auto tools = clore::net::detail::make_empty_array("failed to create tools array");
        if(!tools.has_value()) {
            return std::unexpected(std::move(tools.error()));
        }
        for(auto& tool: request.tools) {
            auto serialized =
                openai_protocol_detail::serialize_tool_definition(document, *tools, tool);
            if(!serialized.has_value()) {
                return std::unexpected(std::move(serialized.error()));
            }
        }
        if(auto status = root->insert("tools", std::move(*tools)); !status.has_value()) {
            return clore::net::detail::to_llm_unexpected(status.error(),
                                                         "failed to serialize tools array");
        }
    }

    if(request.tool_choice.has_value()) {
        auto tool_choice =
            openai_protocol_detail::serialize_tool_choice(*root, *request.tool_choice);
        if(!tool_choice.has_value()) {
            return std::unexpected(std::move(tool_choice.error()));
        }
    }

    if(request.parallel_tool_calls.has_value()) {
        if(auto status = root->insert("parallel_tool_calls", *request.parallel_tool_calls);
           !status.has_value()) {
            return clore::net::detail::to_llm_unexpected(status.error(),
                                                         "failed to serialize parallel_tool_calls");
        }
    }

    auto encoded = root->to_json_string();
    if(!encoded.has_value()) {
        return std::unexpected(
            LLMError(std::format("failed to serialize request JSON: {}",
                                 kota::codec::json::error_message(
                                     kota::codec::json::make_write_error(encoded.error())))));
    }
    return *encoded;
}

auto parse_response(std::string_view json_text) -> std::expected<CompletionResponse, LLMError> {
    auto parsed = kota::codec::json::Object::parse(json_text);
    if(!parsed.has_value()) {
        return std::unexpected(LLMError(std::format(
            "failed to parse LLM response JSON: {}",
            kota::codec::json::error_message(kota::codec::json::make_read_error(parsed.error())))));
    }

    auto error_value = parsed->get("error");
    if(error_value.has_value()) {
        auto error_object = clore::net::detail::expect_object(*error_value, "error");
        if(!error_object.has_value()) {
            return std::unexpected(std::move(error_object.error()));
        }
        auto message_value = error_object->get("message");
        if(message_value.has_value()) {
            auto message = clore::net::detail::expect_string(*message_value, "error.message");
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
    auto id = clore::net::detail::expect_string(*id_value, "id");
    if(!id.has_value()) {
        return std::unexpected(std::move(id.error()));
    }

    auto model_value = parsed->get("model");
    if(!model_value.has_value()) {
        return std::unexpected(LLMError("LLM response is missing model"));
    }
    auto model = clore::net::detail::expect_string(*model_value, "model");
    if(!model.has_value()) {
        return std::unexpected(std::move(model.error()));
    }

    auto choices_value = parsed->get("choices");
    if(!choices_value.has_value()) {
        return std::unexpected(LLMError("LLM response has no choices"));
    }
    auto choices = clore::net::detail::expect_array(*choices_value, "choices");
    if(!choices.has_value()) {
        return std::unexpected(std::move(choices.error()));
    }
    if(choices->empty()) {
        return std::unexpected(LLMError("LLM response choices array is empty"));
    }

    auto first_choice = clore::net::detail::expect_object((*choices)[0], "choices[0]");
    if(!first_choice.has_value()) {
        return std::unexpected(std::move(first_choice.error()));
    }

    auto finish_reason_value = first_choice->get("finish_reason");
    if(!finish_reason_value.has_value()) {
        return std::unexpected(LLMError("LLM response choice is missing finish_reason"));
    }
    auto finish_reason =
        clore::net::detail::expect_string(*finish_reason_value, "choices[0].finish_reason");
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
        return std::unexpected(
            LLMError(std::format("unsupported LLM finish_reason '{}'", *finish_reason)));
    }

    auto message_value = first_choice->get("message");
    if(!message_value.has_value()) {
        return std::unexpected(LLMError("LLM response choice has no message"));
    }
    auto message = clore::net::detail::expect_object(*message_value, "choices[0].message");
    if(!message.has_value()) {
        return std::unexpected(std::move(message.error()));
    }

    AssistantOutput output;

    if(auto refusal_value = message->get("refusal");
       refusal_value.has_value() && !refusal_value->is_null()) {
        auto refusal =
            clore::net::detail::expect_string(*refusal_value, "choices[0].message.refusal");
        if(!refusal.has_value()) {
            return std::unexpected(std::move(refusal.error()));
        }
        output.refusal = std::string(*refusal);
    }

    if(auto content_value = message->get("content"); content_value.has_value()) {
        if(auto content_text = content_value->get_string(); content_text.has_value()) {
            output.text = std::string(*content_text);
        } else if(auto content_parts = content_value->get_array(); content_parts.has_value()) {
            auto parsed_content = openai_protocol_detail::parse_content_parts(*content_parts);
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
        auto tool_calls =
            clore::net::detail::expect_array(*tool_calls_value, "choices[0].message.tool_calls");
        if(!tool_calls.has_value()) {
            return std::unexpected(std::move(tool_calls.error()));
        }
        auto parsed_calls = openai_protocol_detail::parse_tool_calls(*tool_calls);
        if(!parsed_calls.has_value()) {
            return std::unexpected(std::move(parsed_calls.error()));
        }
        output.tool_calls = std::move(*parsed_calls);
    }

    if(*finish_reason == "tool_calls" && output.tool_calls.empty()) {
        return std::unexpected(
            LLMError("LLM response finish_reason=tool_calls but " "no tool calls were returned"));
    }
    if(*finish_reason == "stop" && !output.tool_calls.empty()) {
        return std::unexpected(
            LLMError("LLM response returned tool calls with finish_reason=stop"));
    }

    if(!output.text.has_value() && !output.refusal.has_value() && output.tool_calls.empty()) {
        return std::unexpected(
            LLMError("LLM response message has neither text, refusal, nor tool calls"));
    }

    return CompletionResponse{
        .id = std::string(*id),
        .model = std::string(*model),
        .message = std::move(output),
        .raw_json = std::string(json_text),
    };
}

}  // namespace clore::net::protocol

// ── client wrapper ────────────────────────────────────────────────────────
namespace clore::net::openai_detail {

struct Protocol {
    static auto read_environment() -> std::expected<detail::EnvironmentConfig, LLMError> {
        return detail::read_environment();
    }

    static auto build_url(const detail::EnvironmentConfig& environment) -> std::string {
        return detail::build_chat_completions_url(environment.api_base);
    }

    static auto build_headers(const detail::EnvironmentConfig& environment) -> detail::HttpHeaders {
        return detail::HttpHeaders{
            "Content-Type: application/json; charset=utf-8",
            std::format("Authorization: Bearer {}", environment.api_key),
        };
    }

    static auto build_request_json(const CompletionRequest& request)
        -> std::expected<std::string, LLMError> {
        return clore::net::protocol::build_request_json(request);
    }

    static auto parse_response(const detail::RawHttpResponse& raw_response)
        -> std::expected<CompletionResponse, LLMError> {
        if(raw_response.body.empty()) {
            return std::unexpected(LLMError("empty response from LLM"));
        }
        if(raw_response.http_status >= 400) {
            return std::unexpected(
                LLMError(std::format("LLM request failed with HTTP {}: {}",
                                     raw_response.http_status,
                                     detail::excerpt_for_error(raw_response.body))));
        }

        return clore::net::protocol::parse_response(raw_response.body);
    }

    static auto provider_name() -> std::string_view {
        return "LLM";
    }
};

}  // namespace clore::net::openai_detail

export namespace clore::net::openai {

auto call_completion_async(CompletionRequest request,
                           kota::event_loop& loop = kota::event_loop::current())
    -> kota::task<CompletionResponse, LLMError>;

auto call_llm_async(std::string_view model,
                    std::string_view system_prompt,
                    PromptRequest request,
                    kota::event_loop& loop = kota::event_loop::current())
    -> kota::task<std::string, LLMError>;

auto call_llm_async(std::string_view model,
                    std::string_view system_prompt,
                    std::string_view prompt,
                    kota::event_loop& loop = kota::event_loop::current())
    -> kota::task<std::string, LLMError>;

template <typename T>
auto call_structured_async(std::string_view model,
                           std::string_view system_prompt,
                           std::string_view prompt,
                           kota::event_loop& loop = kota::event_loop::current())
    -> kota::task<T, LLMError>;

}  // namespace clore::net::openai

namespace clore::net::openai {

auto call_completion_async(CompletionRequest request, kota::event_loop& loop)
    -> kota::task<CompletionResponse, LLMError> {
    co_return co_await clore::net::call_completion_async<openai_detail::Protocol>(
        std::move(request),
        &loop)
        .or_fail();
}

auto call_llm_async(std::string_view model,
                    std::string_view system_prompt,
                    PromptRequest request,
                    kota::event_loop& loop) -> kota::task<std::string, LLMError> {
    co_return co_await clore::net::call_llm_async<openai_detail::Protocol>(model,
                                                                           system_prompt,
                                                                           std::move(request),
                                                                           &loop)
        .or_fail();
}

auto call_llm_async(std::string_view model,
                    std::string_view system_prompt,
                    std::string_view prompt,
                    kota::event_loop& loop) -> kota::task<std::string, LLMError> {
    co_return co_await clore::net::call_llm_async<openai_detail::Protocol>(model,
                                                                           system_prompt,
                                                                           prompt,
                                                                           &loop)
        .or_fail();
}

template <typename T>
auto call_structured_async(std::string_view model,
                           std::string_view system_prompt,
                           std::string_view prompt,
                           kota::event_loop& loop) -> kota::task<T, LLMError> {
    co_return co_await clore::net::call_structured_async<openai_detail::Protocol, T>(model,
                                                                                     system_prompt,
                                                                                     prompt,
                                                                                     &loop)
        .or_fail();
}

}  // namespace clore::net::openai
