module;

#include <array>
#include <cstdio>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "llvm/ADT/SmallString.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/Program.h"

export module generate:llm;

import support;

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

struct TempFileCleanup {
    std::vector<std::filesystem::path>& paths;

    ~TempFileCleanup() {
        for(auto& path : paths) {
            std::error_code ec;
            std::filesystem::remove(path, ec);
        }
    }
};

auto create_temp_file(std::string_view prefix, std::string_view suffix,
                      std::vector<std::filesystem::path>& temp_paths)
    -> std::expected<std::filesystem::path, LLMError> {
    llvm::SmallString<128> temp_path;
    if(auto ec = llvm::sys::fs::createTemporaryFile(prefix, suffix, temp_path); ec) {
        return std::unexpected(LLMError{
            .message = std::format("failed to create temp file: {}", ec.message())});
    }

    auto path = std::filesystem::path(std::string(temp_path));
    temp_paths.push_back(path);
    return path;
}

auto write_text_file(const std::filesystem::path& path, std::string_view content)
    -> std::expected<void, LLMError> {
    std::ofstream f(path, std::ios::binary);
    if(!f.is_open()) {
        return std::unexpected(LLMError{
            .message = std::format("failed to open temp file for writing: {}",
                                   path.generic_string())});
    }

    f.write(content.data(), static_cast<std::streamsize>(content.size()));
    if(!f) {
        return std::unexpected(LLMError{
            .message = std::format("failed to write temp file: {}", path.generic_string())});
    }

    return {};
}

auto read_text_file(const std::filesystem::path& path) -> std::expected<std::string, LLMError> {
    std::ifstream f(path, std::ios::binary);
    if(!f.is_open()) {
        return std::unexpected(LLMError{
            .message = std::format("failed to open temp file for reading: {}",
                                   path.generic_string())});
    }

    std::string content((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
    if(!f.good() && !f.eof()) {
        return std::unexpected(LLMError{
            .message = std::format("failed to read temp file: {}", path.generic_string())});
    }

    return content;
}

auto find_curl_executable() -> std::expected<std::string, LLMError> {
    auto curl_path = llvm::sys::findProgramByName("curl");
    if(!curl_path) {
        return std::unexpected(LLMError{
            .message = std::format("failed to locate curl executable: {}",
                                   curl_path.getError().message())});
    }

    return *curl_path;
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

    std::vector<fs::path> temp_paths;
    TempFileCleanup cleanup{temp_paths};

    auto body_path_result = create_temp_file("clore_llm_request", "json", temp_paths);
    if(!body_path_result.has_value()) {
        return std::unexpected(std::move(body_path_result.error()));
    }
    auto curl_config_result = create_temp_file("clore_llm_headers", "conf", temp_paths);
    if(!curl_config_result.has_value()) {
        return std::unexpected(std::move(curl_config_result.error()));
    }
    auto response_path_result = create_temp_file("clore_llm_response", "json", temp_paths);
    if(!response_path_result.has_value()) {
        return std::unexpected(std::move(response_path_result.error()));
    }
    auto stderr_path_result = create_temp_file("clore_llm_stderr", "log", temp_paths);
    if(!stderr_path_result.has_value()) {
        return std::unexpected(std::move(stderr_path_result.error()));
    }

    auto body_path = *body_path_result;
    auto curl_config_path = *curl_config_result;
    auto response_path = *response_path_result;
    auto stderr_path = *stderr_path_result;

    if(auto write_result = write_text_file(body_path, detail::build_request_json(model, prompt));
       !write_result.has_value()) {
        return std::unexpected(std::move(write_result.error()));
    }

    auto curl_config = std::format(
        "header = \"Content-Type: application/json\"\n"
        "header = \"Authorization: Bearer {}\"\n",
        *api_key_result);
    if(auto write_result = write_text_file(curl_config_path, curl_config);
       !write_result.has_value()) {
        return std::unexpected(std::move(write_result.error()));
    }

    auto curl_path_result = find_curl_executable();
    if(!curl_path_result.has_value()) {
        return std::unexpected(std::move(curl_path_result.error()));
    }
    auto curl_path = *curl_path_result;

    auto body_arg = std::string("@") + body_path.string();
    auto response_path_string = response_path.string();
    auto stderr_path_string = stderr_path.string();
    std::vector<std::string> arg_storage{
        curl_path,
        "--silent",
        "--show-error",
        "--request",
        "POST",
        url,
        "--config",
        curl_config_path.string(),
        "--data-binary",
        body_arg,
    };
    llvm::SmallVector<llvm::StringRef, 10> args;
    args.reserve(arg_storage.size());
    for(auto& arg : arg_storage) {
        args.push_back(arg);
    }

    std::array<std::optional<llvm::StringRef>, 3> redirects{
        std::nullopt,
        llvm::StringRef(response_path_string),
        llvm::StringRef(stderr_path_string),
    };

    logging::info("calling LLM: {} model={}", url, model);

    std::string err_msg;
    bool execution_failed = false;
    auto exit_code = llvm::sys::ExecuteAndWait(curl_path, args, std::nullopt, redirects,
                                               0, 0, &err_msg, &execution_failed);

    auto stderr_result = read_text_file(stderr_path);
    auto stderr_text = stderr_result.has_value() ? *stderr_result : std::string{};

    if(execution_failed) {
        auto message = err_msg.empty() ? stderr_text : err_msg;
        if(message.empty()) {
            message = "failed to execute curl";
        }
        return std::unexpected(LLMError{.message = std::move(message)});
    }

    if(exit_code != 0) {
        return std::unexpected(LLMError{
            .message = stderr_text.empty()
                           ? std::format("curl exited with code {}", exit_code)
                           : std::format("curl exited with code {}: {}",
                                         exit_code, stderr_text)});
    }

    auto response_result = read_text_file(response_path);
    if(!response_result.has_value()) {
        return std::unexpected(std::move(response_result.error()));
    }

    if(response_result->empty()) {
        return std::unexpected(LLMError{.message = "empty response from LLM"});
    }

    return detail::parse_response(*response_result);
}

}  // namespace clore::generate
