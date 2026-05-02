module;

#include "kota/async/async.h"
#include "kota/codec/json/error.h"
#include "kota/codec/json/json.h"
#include "kota/http/http.h"

export module anthropic;

import std;
import http;
import protocol;
import schema;
import client;
import provider;
import support;

// ── protocol serialization ──────────────────────────────────────────────
namespace clore::net::anthropic::protocol::detail {

namespace json = kota::codec::json;

constexpr std::uint32_t kDefaultMaxTokens = 2048;

auto append_text_with_gap(std::string& target, std::string_view text) -> void {
    if(text.empty()) {
        return;
    }
    if(!target.empty()) {
        target += "\n\n";
    }
    target += text;
}

auto make_text_block(std::string_view text) -> std::expected<json::Object, LLMError> {
    auto block = clore::net::detail::make_empty_object("failed to create text block");
    if(!block.has_value()) {
        return std::unexpected(std::move(block.error()));
    }
    if(auto status = clore::net::detail::insert_string_field(*block,
                                                             "type",
                                                             "text",
                                                             "failed to set text block type");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    auto normalized = clore::net::detail::normalize_utf8(text, "text block text");
    if(auto status = clore::net::detail::insert_string_field(*block,
                                                             "text",
                                                             std::string_view(normalized),
                                                             "failed to set text block text");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    return *block;
}

auto make_tool_use_block(const ToolCall& call) -> std::expected<json::Object, LLMError> {
    if(!call.arguments.is_object()) {
        return std::unexpected(LLMError(
            std::format("Anthropic tool input for '{}' must be a JSON object", call.name)));
    }

    auto block = clore::net::detail::make_empty_object("failed to create tool_use block");
    if(!block.has_value()) {
        return std::unexpected(std::move(block.error()));
    }
    if(auto status = clore::net::detail::insert_string_field(*block,
                                                             "type",
                                                             "tool_use",
                                                             "failed to set tool_use block type");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    if(auto status = clore::net::detail::insert_string_field(*block,
                                                             "id",
                                                             call.id,
                                                             "failed to set tool_use block id");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    if(auto status = clore::net::detail::insert_string_field(*block,
                                                             "name",
                                                             call.name,
                                                             "failed to set tool_use block name");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }

    auto input = clore::net::detail::clone_value(call.arguments, "failed to clone tool_use input");
    if(!input.has_value()) {
        return std::unexpected(std::move(input.error()));
    }
    block->insert("input", std::move(*input));
    return *block;
}

auto make_tool_result_block(const ToolResultMessage& message)
    -> std::expected<json::Object, LLMError> {
    auto block = clore::net::detail::make_empty_object("failed to create tool_result block");
    if(!block.has_value()) {
        return std::unexpected(std::move(block.error()));
    }
    if(auto status =
           clore::net::detail::insert_string_field(*block,
                                                   "type",
                                                   "tool_result",
                                                   "failed to set tool_result block type");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    if(auto status = clore::net::detail::insert_string_field(*block,
                                                             "tool_use_id",
                                                             message.tool_call_id,
                                                             "failed to set tool_result id");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    auto normalized = clore::net::detail::normalize_utf8(message.content, "tool_result content");
    if(auto status = clore::net::detail::insert_string_field(*block,
                                                             "content",
                                                             std::string_view(normalized),
                                                             "failed to set tool_result content");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    return *block;
}

auto make_role_message(std::string_view role, std::string_view text)
    -> std::expected<json::Object, LLMError> {
    auto message = clore::net::detail::make_empty_object("failed to create role message");
    if(!message.has_value()) {
        return std::unexpected(std::move(message.error()));
    }
    if(auto status = clore::net::detail::insert_string_field(*message,
                                                             "role",
                                                             role,
                                                             "failed to set message role");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    auto normalized = clore::net::detail::normalize_utf8(text, "message content");
    if(auto status = clore::net::detail::insert_string_field(*message,
                                                             "content",
                                                             std::string_view(normalized),
                                                             "failed to set message content");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    return *message;
}

auto make_role_message(std::string_view role, json::Array blocks)
    -> std::expected<json::Object, LLMError> {
    auto message = clore::net::detail::make_empty_object("failed to create role message");
    if(!message.has_value()) {
        return std::unexpected(std::move(message.error()));
    }
    if(auto status = clore::net::detail::insert_string_field(*message,
                                                             "role",
                                                             role,
                                                             "failed to set message role");
       !status.has_value()) {
        return std::unexpected(std::move(status.error()));
    }
    message->insert("content", std::move(blocks));
    return *message;
}

auto parse_json_text(std::string_view raw, std::string_view context)
    -> std::expected<json::Object, LLMError> {
    return clore::net::detail::parse_json_object(raw, context);
}

auto format_schema_instruction(const ResponseFormat& format)
    -> std::expected<std::string, LLMError> {
    if(!format.schema.has_value()) {
        return std::string("Return only a JSON object. Do not include markdown fences.");
    }

    auto encoded = json::to_string(*format.schema);
    if(!encoded.has_value()) {
        return clore::net::detail::unexpected_json_error("failed to serialize response schema",
                                                         encoded.error());
    }
    return std::format(
        "Return only a JSON object matching this schema. Do not " "include markdown fences. Schema name: {}\n{}",
        format.name,
        *encoded);
}

auto validate_request(const CompletionRequest& request) -> std::expected<void, LLMError> {
    return clore::net::detail::validate_completion_request(request, false, false);
}

}  // namespace clore::net::anthropic::protocol::detail

export namespace clore::net::anthropic::protocol {

auto build_messages_url(std::string_view api_base) -> std::string;

auto build_request_json(const CompletionRequest& request) -> std::expected<std::string, LLMError>;

auto parse_response(std::string_view json) -> std::expected<CompletionResponse, LLMError>;

auto text_from_response(const CompletionResponse& response) -> std::expected<std::string, LLMError>;

auto append_tool_outputs(std::span<const Message> history,
                         const CompletionResponse& response,
                         std::span<const ToolOutput> outputs)
    -> std::expected<std::vector<Message>, LLMError>;

template <typename T>
auto parse_response_text(const CompletionResponse& response) -> std::expected<T, LLMError>;

template <typename T>
auto parse_tool_arguments(const ToolCall& call) -> std::expected<T, LLMError>;

}  // namespace clore::net::anthropic::protocol

namespace clore::net::anthropic::protocol {

auto build_messages_url(std::string_view api_base) -> std::string {
    std::string url(api_base);
    while(!url.empty() && url.back() == '/') {
        url.pop_back();
    }
    if(url.ends_with("/v1")) {
        return clore::net::detail::append_url_path(url, "messages");
    }
    return clore::net::detail::append_url_path(url, "v1/messages");
}

auto build_request_json(const CompletionRequest& request) -> std::expected<std::string, LLMError> {
    auto validation = detail::validate_request(request);
    if(!validation.has_value()) {
        return std::unexpected(std::move(validation.error()));
    }

    auto root = clore::net::detail::make_empty_object("failed to create Anthropic request root");
    if(!root.has_value()) {
        return std::unexpected(std::move(root.error()));
    }
    root->insert("model", request.model);
    root->insert("max_tokens", detail::kDefaultMaxTokens);

    auto messages = clore::net::detail::make_empty_array("failed to create Anthropic messages");
    if(!messages.has_value()) {
        return std::unexpected(std::move(messages.error()));
    }
    std::string system_text;

    for(const auto& message: request.messages) {
        auto serialized = std::visit(
            [&](const auto& current)
                -> std::expected<std::optional<kota::codec::json::Object>, LLMError> {
                using message_type = std::remove_cvref_t<decltype(current)>;
                if constexpr(std::same_as<message_type, SystemMessage>) {
                    detail::append_text_with_gap(system_text, current.content);
                    return std::optional<kota::codec::json::Object>{std::nullopt};
                } else if constexpr(std::same_as<message_type, UserMessage>) {
                    auto object = detail::make_role_message("user", current.content);
                    if(!object.has_value()) {
                        return std::unexpected(std::move(object.error()));
                    }
                    return std::optional<kota::codec::json::Object>{std::move(*object)};
                } else if constexpr(std::same_as<message_type, AssistantMessage>) {
                    auto object = detail::make_role_message("assistant", current.content);
                    if(!object.has_value()) {
                        return std::unexpected(std::move(object.error()));
                    }
                    return std::optional<kota::codec::json::Object>{std::move(*object)};
                } else if constexpr(std::same_as<message_type, AssistantToolCallMessage>) {
                    auto blocks = clore::net::detail::make_empty_array(
                        "failed to create Anthropic assistant blocks");
                    if(!blocks.has_value()) {
                        return std::unexpected(std::move(blocks.error()));
                    }
                    if(current.content.has_value()) {
                        auto text_block = detail::make_text_block(*current.content);
                        if(!text_block.has_value()) {
                            return std::unexpected(std::move(text_block.error()));
                        }
                        blocks->push_back(std::move(*text_block));
                    }
                    for(const auto& call: current.tool_calls) {
                        auto tool_block = detail::make_tool_use_block(call);
                        if(!tool_block.has_value()) {
                            return std::unexpected(std::move(tool_block.error()));
                        }
                        blocks->push_back(std::move(*tool_block));
                    }
                    auto object = detail::make_role_message("assistant", std::move(*blocks));
                    if(!object.has_value()) {
                        return std::unexpected(std::move(object.error()));
                    }
                    return std::optional<kota::codec::json::Object>{std::move(*object)};
                } else {
                    auto blocks = clore::net::detail::make_empty_array(
                        "failed to create Anthropic tool_result blocks");
                    if(!blocks.has_value()) {
                        return std::unexpected(std::move(blocks.error()));
                    }
                    auto result_block = detail::make_tool_result_block(current);
                    if(!result_block.has_value()) {
                        return std::unexpected(std::move(result_block.error()));
                    }
                    blocks->push_back(std::move(*result_block));
                    auto object = detail::make_role_message("user", std::move(*blocks));
                    if(!object.has_value()) {
                        return std::unexpected(std::move(object.error()));
                    }
                    return std::optional<kota::codec::json::Object>{std::move(*object)};
                }
            },
            message);
        if(!serialized.has_value()) {
            return std::unexpected(std::move(serialized.error()));
        }
        if(serialized->has_value()) {
            messages->push_back(std::move(**serialized));
        }
    }

    if(request.response_format.has_value()) {
        auto schema_instruction = detail::format_schema_instruction(*request.response_format);
        if(!schema_instruction.has_value()) {
            return std::unexpected(std::move(schema_instruction.error()));
        }
        detail::append_text_with_gap(system_text, *schema_instruction);
    }

    if(!system_text.empty()) {
        if(auto status = clore::net::detail::insert_string_field(*root,
                                                                 "system",
                                                                 std::string_view(system_text),
                                                                 "failed to serialize system");
           !status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
    }

    root->insert("messages", std::move(*messages));

    if(!request.tools.empty()) {
        auto tools = clore::net::detail::make_empty_array("failed to create Anthropic tools");
        if(!tools.has_value()) {
            return std::unexpected(std::move(tools.error()));
        }
        for(const auto& tool: request.tools) {
            auto object = clore::net::detail::make_empty_object("failed to create Anthropic tool");
            if(!object.has_value()) {
                return std::unexpected(std::move(object.error()));
            }
            if(auto status =
                   clore::net::detail::insert_string_field(*object,
                                                           "name",
                                                           tool.name,
                                                           "failed to serialize tool name");
               !status.has_value()) {
                return std::unexpected(std::move(status.error()));
            }
            if(auto status =
                   clore::net::detail::insert_string_field(*object,
                                                           "description",
                                                           tool.description,
                                                           "failed to serialize tool description");
               !status.has_value()) {
                return std::unexpected(std::move(status.error()));
            }
            auto input_schema =
                clore::net::detail::clone_object(tool.parameters,
                                                 "failed to clone tool input schema");
            if(!input_schema.has_value()) {
                return std::unexpected(std::move(input_schema.error()));
            }
            object->insert("input_schema", std::move(*input_schema));
            tools->push_back(std::move(*object));
        }
        root->insert("tools", std::move(*tools));
    }

    if(request.tool_choice.has_value() ||
       (request.parallel_tool_calls.has_value() && !*request.parallel_tool_calls)) {
        auto tool_choice =
            clore::net::detail::make_empty_object("failed to create Anthropic tool_choice");
        if(!tool_choice.has_value()) {
            return std::unexpected(std::move(tool_choice.error()));
        }

        if(request.tool_choice.has_value()) {
            auto status = std::visit(
                [&](const auto& current) -> std::expected<void, LLMError> {
                    using choice_type = std::remove_cvref_t<decltype(current)>;
                    if constexpr(std::same_as<choice_type, ToolChoiceAuto>) {
                        return clore::net::detail::insert_string_field(
                            *tool_choice,
                            "type",
                            "auto",
                            "failed to serialize tool_choice");
                    } else if constexpr(std::same_as<choice_type, ToolChoiceRequired>) {
                        return clore::net::detail::insert_string_field(
                            *tool_choice,
                            "type",
                            "any",
                            "failed to serialize tool_choice");
                    } else if constexpr(std::same_as<choice_type, ToolChoiceNone>) {
                        return clore::net::detail::insert_string_field(
                            *tool_choice,
                            "type",
                            "none",
                            "failed to serialize tool_choice");
                    } else {
                        auto type_status = clore::net::detail::insert_string_field(
                            *tool_choice,
                            "type",
                            "tool",
                            "failed to serialize tool_choice");
                        if(!type_status.has_value()) {
                            return std::unexpected(std::move(type_status.error()));
                        }
                        return clore::net::detail::insert_string_field(
                            *tool_choice,
                            "name",
                            current.name,
                            "failed to serialize tool_choice name");
                    }
                },
                *request.tool_choice);
            if(!status.has_value()) {
                return std::unexpected(std::move(status.error()));
            }
        } else {
            auto status =
                clore::net::detail::insert_string_field(*tool_choice,
                                                        "type",
                                                        "auto",
                                                        "failed to serialize tool_choice");
            if(!status.has_value()) {
                return std::unexpected(std::move(status.error()));
            }
        }

        if(request.parallel_tool_calls.has_value() && !*request.parallel_tool_calls) {
            tool_choice->insert("disable_parallel_tool_use", true);
        }

        root->insert("tool_choice", std::move(*tool_choice));
    }

    auto encoded = kota::codec::json::to_string(*root);
    if(!encoded.has_value()) {
        return clore::net::detail::unexpected_json_error("failed to serialize request JSON",
                                                         encoded.error());
    }
    return *encoded;
}

auto parse_response(std::string_view json_text) -> std::expected<CompletionResponse, LLMError> {
    auto parsed = detail::parse_json_text(json_text, "failed to parse Anthropic response JSON");
    if(!parsed.has_value()) {
        return std::unexpected(std::move(parsed.error()));
    }

    auto root_view = clore::net::detail::ObjectView{.value = &*parsed};
    if(auto error_value = root_view.get("error"); error_value.has_value()) {
        auto error_object = clore::net::detail::expect_object(*error_value, "error");
        if(!error_object.has_value()) {
            return std::unexpected(std::move(error_object.error()));
        }
        if(auto message_value = error_object->get("message"); message_value.has_value()) {
            auto message = clore::net::detail::expect_string(*message_value, "error.message");
            if(message.has_value()) {
                return std::unexpected(LLMError(std::format("LLM API error: {}", *message)));
            }
        }
        return std::unexpected(LLMError("LLM API returned an error payload"));
    }

    auto id_value = root_view.get("id");
    if(!id_value.has_value()) {
        return std::unexpected(LLMError("Anthropic response is missing id"));
    }
    auto id = clore::net::detail::expect_string(*id_value, "id");
    if(!id.has_value()) {
        return std::unexpected(std::move(id.error()));
    }

    auto model_value = root_view.get("model");
    if(!model_value.has_value()) {
        return std::unexpected(LLMError("Anthropic response is missing model"));
    }
    auto model = clore::net::detail::expect_string(*model_value, "model");
    if(!model.has_value()) {
        return std::unexpected(std::move(model.error()));
    }

    auto stop_reason_value = root_view.get("stop_reason");
    std::string_view stop_reason = "end_turn";
    if(stop_reason_value.has_value() && !stop_reason_value->is_null()) {
        auto parsed_stop_reason =
            clore::net::detail::expect_string(*stop_reason_value, "stop_reason");
        if(!parsed_stop_reason.has_value()) {
            return std::unexpected(std::move(parsed_stop_reason.error()));
        }
        stop_reason = *parsed_stop_reason;
    }

    if(stop_reason == "max_tokens") {
        return std::unexpected(LLMError("LLM response was truncated (stop_reason=max_tokens)"));
    }

    auto content_value = root_view.get("content");
    if(!content_value.has_value()) {
        return std::unexpected(LLMError("Anthropic response is missing content"));
    }
    auto content = clore::net::detail::expect_array(*content_value, "content");
    if(!content.has_value()) {
        return std::unexpected(std::move(content.error()));
    }

    AssistantOutput output;
    std::string text;
    std::string refusal;

    for(auto value: *content) {
        auto block = clore::net::detail::expect_object(value, "content[]");
        if(!block.has_value()) {
            return std::unexpected(std::move(block.error()));
        }

        auto type_value = block->get("type");
        if(!type_value.has_value()) {
            return std::unexpected(LLMError("content block is missing type"));
        }
        auto type = clore::net::detail::expect_string(*type_value, "content[].type");
        if(!type.has_value()) {
            return std::unexpected(std::move(type.error()));
        }

        if(*type == "text") {
            auto text_value = block->get("text");
            if(!text_value.has_value()) {
                return std::unexpected(LLMError("text block is missing text"));
            }
            auto parsed_text = clore::net::detail::expect_string(*text_value, "content[].text");
            if(!parsed_text.has_value()) {
                return std::unexpected(std::move(parsed_text.error()));
            }
            if(stop_reason == "refusal") {
                refusal += *parsed_text;
            } else {
                text += *parsed_text;
            }
            continue;
        }

        if(*type != "tool_use") {
            continue;
        }

        auto id_field = block->get("id");
        auto name_field = block->get("name");
        auto input_field = block->get("input");
        if(!id_field.has_value() || !name_field.has_value() || !input_field.has_value()) {
            return std::unexpected(LLMError("tool_use block is missing id, name, or input"));
        }

        auto call_id = clore::net::detail::expect_string(*id_field, "content[].id");
        if(!call_id.has_value()) {
            return std::unexpected(std::move(call_id.error()));
        }
        auto call_name = clore::net::detail::expect_string(*name_field, "content[].name");
        if(!call_name.has_value()) {
            return std::unexpected(std::move(call_name.error()));
        }

        auto input_object = clore::net::detail::expect_object(*input_field, "content[].input");
        if(!input_object.has_value()) {
            return std::unexpected(std::move(input_object.error()));
        }

        auto input =
            clore::net::detail::clone_object(*input_object, "failed to clone tool_use input");
        if(!input.has_value()) {
            return std::unexpected(std::move(input.error()));
        }
        auto input_json = kota::codec::json::to_string(*input);
        if(!input_json.has_value()) {
            return clore::net::detail::unexpected_json_error("failed to serialize tool input JSON",
                                                             input_json.error());
        }
        auto arguments = kota::codec::json::parse<kota::codec::json::Value>(*input_json);
        if(!arguments.has_value()) {
            return std::unexpected(LLMError(
                std::format("failed to parse tool_use input: {}", arguments.error().to_string())));
        }

        output.tool_calls.push_back(ToolCall{
            .id = std::string(*call_id),
            .name = std::string(*call_name),
            .arguments_json = std::move(*input_json),
            .arguments = std::move(*arguments),
        });
    }

    if(!text.empty()) {
        output.text = std::move(text);
    }
    if(!refusal.empty()) {
        output.refusal = std::move(refusal);
    }

    return CompletionResponse{
        .id = std::string(*id),
        .model = std::string(*model),
        .message = std::move(output),
        .raw_json = std::string(json_text),
    };
}

auto text_from_response(const CompletionResponse& response)
    -> std::expected<std::string, LLMError> {
    return clore::net::protocol::text_from_response(response);
}

auto append_tool_outputs(std::span<const Message> history,
                         const CompletionResponse& response,
                         std::span<const ToolOutput> outputs)
    -> std::expected<std::vector<Message>, LLMError> {
    return clore::net::protocol::append_tool_outputs(history, response, outputs);
}

template <typename T>
auto parse_response_text(const CompletionResponse& response) -> std::expected<T, LLMError> {
    return clore::net::protocol::parse_response_text<T>(response);
}

template <typename T>
auto parse_tool_arguments(const ToolCall& call) -> std::expected<T, LLMError> {
    return clore::net::protocol::parse_tool_arguments<T>(call);
}

}  // namespace clore::net::anthropic::protocol

// ── client wrapper ────────────────────────────────────────────────────────
namespace clore::net::anthropic::detail {

constexpr std::string_view kAnthropicBaseUrlEnv = "ANTHROPIC_BASE_URL";
constexpr std::string_view kAnthropicApiKeyEnv = "ANTHROPIC_API_KEY";
constexpr std::string_view kAnthropicVersion = "2023-06-01";

struct Protocol {
    static auto read_environment()
        -> std::expected<clore::net::detail::EnvironmentConfig, LLMError> {
        return clore::net::detail::read_credentials(clore::net::detail::CredentialEnv{
            .base_url_env = kAnthropicBaseUrlEnv,
            .api_key_env = kAnthropicApiKeyEnv,
        });
    }

    static auto build_url(const clore::net::detail::EnvironmentConfig& environment) -> std::string {
        return clore::net::anthropic::protocol::build_messages_url(environment.api_base);
    }

    static auto build_headers(const clore::net::detail::EnvironmentConfig& environment)
        -> std::vector<kota::http::header> {
        return std::vector<kota::http::header>{
            kota::http::header{
                               .name = "Content-Type",
                               .value = "application/json; charset=utf-8",
                               },
            kota::http::header{
                               .name = "x-api-key",
                               .value = environment.api_key,
                               },
            kota::http::header{
                               .name = "anthropic-version",
                               .value = std::string(kAnthropicVersion),
                               },
        };
    }

    static auto build_request_json(const CompletionRequest& request)
        -> std::expected<std::string, LLMError> {
        return clore::net::anthropic::protocol::build_request_json(request);
    }

    static auto parse_response(const clore::net::detail::RawHttpResponse& raw_response)
        -> std::expected<CompletionResponse, LLMError> {
        if(raw_response.body.empty()) {
            return std::unexpected(LLMError("empty response from Anthropic"));
        }

        auto parsed = clore::net::anthropic::protocol::parse_response(raw_response.body);
        if(!parsed.has_value()) {
            if(raw_response.http_status >= 400) {
                return std::unexpected(
                    LLMError(std::format("Anthropic request failed with HTTP {}: {}",
                                         raw_response.http_status,
                                         clore::net::detail::excerpt_for_error(raw_response.body))));
            }
            return std::unexpected(std::move(parsed.error()));
        }
        if(raw_response.http_status >= 400) {
            return std::unexpected(LLMError(
                std::format("Anthropic request failed with HTTP {}", raw_response.http_status)));
        }
        return std::move(*parsed);
    }

    static auto provider_name() -> std::string_view {
        return "Anthropic";
    }

    static auto capability_probe_key(const clore::net::detail::EnvironmentConfig& environment,
                                     const CompletionRequest& request) -> std::string {
        return clore::net::make_capability_probe_key(provider_name(),
                                                     environment.api_base,
                                                     request.model);
    }
};

}  // namespace clore::net::anthropic::detail

export namespace clore::net::anthropic {

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

}  // namespace clore::net::anthropic

export namespace clore::net::anthropic::schema {

template <typename T>
auto response_format() -> std::expected<ResponseFormat, LLMError> {
    return clore::net::schema::response_format<T>();
}

template <typename T>
auto function_tool(std::string name, std::string description)
    -> std::expected<FunctionToolDefinition, LLMError> {
    return clore::net::schema::function_tool<T>(std::move(name), std::move(description));
}

}  // namespace clore::net::anthropic::schema

namespace clore::net::anthropic {

auto call_completion_async(CompletionRequest request, kota::event_loop& loop)
    -> kota::task<CompletionResponse, LLMError> {
    co_return co_await clore::net::call_completion_async<detail::Protocol>(std::move(request),
                                                                           &loop)
        .or_fail();
}

auto call_llm_async(std::string_view model,
                    std::string_view system_prompt,
                    PromptRequest request,
                    kota::event_loop& loop) -> kota::task<std::string, LLMError> {
    co_return co_await clore::net::call_llm_async<detail::Protocol>(model,
                                                                    system_prompt,
                                                                    std::move(request),
                                                                    &loop)
        .or_fail();
}

auto call_llm_async(std::string_view model,
                    std::string_view system_prompt,
                    std::string_view prompt,
                    kota::event_loop& loop) -> kota::task<std::string, LLMError> {
    co_return co_await clore::net::call_llm_async<detail::Protocol>(model,
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
    co_return co_await clore::net::call_structured_async<detail::Protocol, T>(model,
                                                                              system_prompt,
                                                                              prompt,
                                                                              &loop)
        .or_fail();
}

}  // namespace clore::net::anthropic
