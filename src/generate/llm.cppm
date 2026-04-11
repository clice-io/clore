module;

#include <cstdio>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

#include "llvm/Support/JSON.h"

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

export module clore.generate:llm;

import clore.support;

export namespace clore::generate {

struct LLMError {
    std::string message;
};

namespace detail {

auto build_request_json(std::string_view model, std::string_view prompt) -> std::string;

auto parse_response(std::string_view json) -> std::expected<std::string, LLMError>;

}  // namespace detail

auto call_llm(std::string_view model, std::string_view prompt)
    -> std::expected<std::string, LLMError>;

}  // namespace clore::generate

// ── implementation ──────────────────────────────────────────────────

namespace clore::generate {

namespace {

constexpr std::string_view kOpenAIBaseUrlEnv = "OPENAI_BASE_URL";
constexpr std::string_view kOpenAIApiKeyEnv = "OPENAI_API_KEY";

auto read_required_env(std::string_view name) -> std::expected<std::string, LLMError> {
    auto* value = std::getenv(std::string{name}.c_str());
    if(value == nullptr || value[0] == '\0') {
        return std::unexpected(LLMError{
            .message = std::format("required environment variable {} is not set", name)});
    }
    return std::string(value);
}

auto build_chat_completions_url(std::string_view api_base) -> std::string {
    std::string url(api_base);
    if(!url.empty() && url.back() != '/') {
        url += '/';
    }
    url += "chat/completions";
    return url;
}

auto escape_json_string(std::string_view s) -> std::string {
    std::string out;
    out.reserve(s.size() + 16);
    for(char c : s) {
        switch(c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if(static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned>(c));
                    out += buf;
                } else {
                    out += c;
                }
                break;
        }
    }
    return out;
}

auto read_pipe(FILE* pipe) -> std::string {
    std::string result;
    char buf[4096];
    while(auto n = std::fread(buf, 1, sizeof(buf), pipe)) {
        result.append(buf, n);
    }
    return result;
}

}  // namespace

namespace detail {

auto build_request_json(std::string_view model, std::string_view prompt) -> std::string {
    std::string system_msg =
        "You are a C++ documentation writer. Generate clear, well-structured "
        "Markdown documentation for C++ code elements. Focus on explaining purpose, "
        "parameters, return values, and relationships. Use proper Markdown formatting.";

    return std::format(
        R"({{"model":"{}","messages":[{{"role":"system","content":"{}"}},{{"role":"user","content":"{}"}}]}})",
        escape_json_string(model),
        escape_json_string(system_msg),
        escape_json_string(prompt));
}

auto parse_response(std::string_view json) -> std::expected<std::string, LLMError> {
    auto parsed = llvm::json::parse(json);
    if(!parsed) {
        return std::unexpected(LLMError{
            .message = std::format("failed to parse LLM response JSON: {}",
                                   llvm::toString(parsed.takeError()))});
    }

    auto* root = parsed->getAsObject();
    if(!root) {
        return std::unexpected(LLMError{.message = "LLM response is not a JSON object"});
    }

    if(auto* error = root->getObject("error")) {
        auto msg = error->getString("message").value_or("unknown error");
        return std::unexpected(LLMError{
            .message = std::format("LLM API error: {}", std::string_view(msg))});
    }

    auto* choices = root->getArray("choices");
    if(!choices || choices->empty()) {
        return std::unexpected(LLMError{.message = "LLM response has no choices"});
    }

    auto* first = (*choices)[0].getAsObject();
    if(!first) {
        return std::unexpected(LLMError{.message = "LLM response choice is not an object"});
    }

    auto* message = first->getObject("message");
    if(!message) {
        return std::unexpected(LLMError{.message = "LLM response choice has no message"});
    }

    auto content = message->getString("content");
    if(!content) {
        return std::unexpected(LLMError{.message = "LLM response message has no content"});
    }

    return std::string(*content);
}

}  // namespace detail

auto call_llm(std::string_view model, std::string_view prompt)
    -> std::expected<std::string, LLMError> {
    namespace fs = std::filesystem;

    if(model.empty()) {
        return std::unexpected(LLMError{.message = "llm model must not be empty"});
    }

    auto api_base_result = read_required_env(kOpenAIBaseUrlEnv);
    if(!api_base_result.has_value()) {
        return std::unexpected(std::move(api_base_result.error()));
    }

    auto api_key_result = read_required_env(kOpenAIApiKeyEnv);
    if(!api_key_result.has_value()) {
        return std::unexpected(std::move(api_key_result.error()));
    }

    auto url = build_chat_completions_url(*api_base_result);

    auto body_path = fs::temp_directory_path() / "clore_llm_request.json";
    {
        std::ofstream f(body_path);
        if(!f.is_open()) {
            return std::unexpected(LLMError{
                .message = "failed to create temp file for LLM request"});
        }
        auto body = detail::build_request_json(model, prompt);
        f << body;
    }

    auto cmd = std::format(
        "curl -s -X POST \"{}\" "
        "-H \"Content-Type: application/json\" "
        "-H \"Authorization: Bearer {}\" "
        "-d \"@{}\"",
        url, *api_key_result, body_path.generic_string());

    logging::info("calling LLM: {} model={}", url, model);

    auto* pipe = popen(cmd.c_str(), "r");
    if(!pipe) {
        fs::remove(body_path);
        return std::unexpected(LLMError{.message = "failed to execute curl"});
    }

    auto response = read_pipe(pipe);
    auto exit_code = pclose(pipe);
    fs::remove(body_path);

    if(exit_code != 0) {
        return std::unexpected(LLMError{
            .message = std::format("curl exited with code {}", exit_code)});
    }

    if(response.empty()) {
        return std::unexpected(LLMError{.message = "empty response from LLM"});
    }

    return detail::parse_response(response);
}

}  // namespace clore::generate
