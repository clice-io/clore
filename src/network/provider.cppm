module;

#include "kota/codec/json/json.h"

export module provider;

import std;
import http;
import protocol;
import schema;

export namespace clore::net::detail {

struct CredentialEnv {
    std::string_view base_url_env;
    std::string_view api_key_env;
};

auto read_credentials(CredentialEnv env) -> std::expected<EnvironmentConfig, LLMError>;

auto append_url_path(std::string_view base_url, std::string_view path) -> std::string;

auto validate_completion_request(const CompletionRequest& request,
                                 bool validate_response_format_schema,
                                 bool validate_tool_schemas) -> std::expected<void, LLMError>;

auto parse_json_object(std::string_view raw, std::string_view context)
    -> std::expected<kota::codec::json::Object, LLMError>;

auto serialize_tool_arguments(kota::codec::json::Value arguments, std::string_view context)
    -> std::expected<std::pair<std::string, kota::codec::json::Value>, LLMError>;

}  // namespace clore::net::detail

namespace clore::net::detail {

namespace json = kota::codec::json;

auto read_credentials(CredentialEnv env) -> std::expected<EnvironmentConfig, LLMError> {
    return read_environment(env.base_url_env, env.api_key_env);
}

auto append_url_path(std::string_view base_url, std::string_view path) -> std::string {
    std::string url(base_url);
    while(!url.empty() && url.back() == '/') {
        url.pop_back();
    }

    std::string suffix(path);
    while(!suffix.empty() && suffix.front() == '/') {
        suffix.erase(suffix.begin());
    }

    if(!suffix.empty()) {
        url += '/';
        url += suffix;
    }
    return url;
}

auto validate_completion_request(const CompletionRequest& request,
                                 bool validate_response_format_schema,
                                 bool validate_tool_schemas) -> std::expected<void, LLMError> {
    if(request.model.empty()) {
        return std::unexpected(LLMError("request model must not be empty"));
    }
    if(request.messages.empty()) {
        return std::unexpected(LLMError("request messages must not be empty"));
    }

    if(validate_response_format_schema && request.response_format.has_value()) {
        auto status = validate_response_format(*request.response_format);
        if(!status.has_value()) {
            return std::unexpected(std::move(status.error()));
        }
    }

    if(validate_tool_schemas) {
        for(const auto& tool: request.tools) {
            auto status = validate_tool_definition(tool);
            if(!status.has_value()) {
                return std::unexpected(std::move(status.error()));
            }
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
            for(const auto& tool: request.tools) {
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

    for(const auto& message: request.messages) {
        auto status = std::visit(
            [](const auto& current) -> std::expected<void, LLMError> {
                using message_type = std::remove_cvref_t<decltype(current)>;
                if constexpr(std::same_as<message_type, AssistantToolCallMessage>) {
                    if(!current.content.has_value() && current.tool_calls.empty()) {
                        return std::unexpected(LLMError(
                            "assistant tool-call message must contain content " "or tool_calls"));
                    }
                    std::unordered_set<std::string> ids;
                    for(const auto& call: current.tool_calls) {
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

auto parse_json_object(std::string_view raw, std::string_view context)
    -> std::expected<json::Object, LLMError> {
    auto parsed = json::parse<json::Object>(raw);
    if(!parsed.has_value()) {
        return std::unexpected(
            LLMError(std::format("{}: {}", context, parsed.error().to_string())));
    }
    return *parsed;
}

auto serialize_tool_arguments(json::Value arguments, std::string_view context)
    -> std::expected<std::pair<std::string, json::Value>, LLMError> {
    auto encoded = json::to_string(arguments);
    if(!encoded.has_value()) {
        return unexpected_json_error(context, encoded.error());
    }

    auto parsed = json::parse<json::Value>(*encoded);
    if(!parsed.has_value()) {
        return std::unexpected(
            LLMError(std::format("{}: {}", context, parsed.error().to_string())));
    }

    return std::pair<std::string, json::Value>{std::move(*encoded), std::move(*parsed)};
}

}  // namespace clore::net::detail
