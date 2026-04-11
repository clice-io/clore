module;

#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>

export module generate:prompt;

import :model;
import :evidence;
import config;

export namespace clore::generate {

struct PromptError {
    std::string message;
};

auto load_prompt_template(std::string_view path) -> std::expected<std::string, PromptError>;

auto instantiate_prompt(const std::string& tmpl,
                        const EvidencePack& evidence) -> std::string;

}  // namespace clore::generate

// ── implementation ──────────────────────────────────────────────────

namespace clore::generate {

namespace {

auto target_name_of(const EvidencePack& evidence) -> std::string_view {
    if(!evidence.subject_name.empty()) {
        return evidence.subject_name;
    }
    if(!evidence.target_facts.empty()) {
        return evidence.target_facts[0].qualified_name;
    }
    return {};
}

auto target_kind_of(const EvidencePack& evidence) -> std::string_view {
    if(!evidence.subject_kind.empty()) {
        return evidence.subject_kind;
    }
    if(!evidence.target_facts.empty()) {
        return evidence.target_facts[0].kind_label;
    }
    return {};
}

auto instantiate_prompt_with_evidence(const std::string& tmpl,
                                      const EvidencePack& evidence,
                                      std::string_view evidence_text) -> std::string {
    std::string result;
    result.reserve(tmpl.size() + 4096);

    std::size_t pos = 0;
    while(pos < tmpl.size()) {
        auto marker_start = tmpl.find("{{", pos);
        if(marker_start == std::string::npos) {
            result.append(tmpl, pos, tmpl.size() - pos);
            break;
        }
        result.append(tmpl, pos, marker_start - pos);

        auto marker_end = tmpl.find("}}", marker_start);
        if(marker_end == std::string::npos) {
            result.append(tmpl, marker_start, tmpl.size() - marker_start);
            break;
        }

        auto var_name = tmpl.substr(marker_start + 2, marker_end - marker_start - 2);
        if(var_name == "evidence") {
            result.append(evidence_text);
        } else if(var_name == "target_name") {
            result.append(target_name_of(evidence));
        } else if(var_name == "target_kind") {
            result.append(target_kind_of(evidence));
        } else {
            result.append("{{");
            result.append(var_name);
            result.append("}}");
        }

        pos = marker_end + 2;
    }

    return result;
}

}  // namespace

auto load_prompt_template(std::string_view path) -> std::expected<std::string, PromptError> {
    namespace fs = std::filesystem;

    if(path.empty()) {
        return std::unexpected(PromptError{.message = "prompt template path is empty"});
    }

    auto file_path = fs::path(path);
    std::error_code ec;
    auto exists = fs::exists(file_path, ec);
    if(ec) {
        return std::unexpected(PromptError{
            .message = std::format("prompt template check failed: {}: {}",
                                   path, ec.message())});
    }
    if(!exists) {
        return std::unexpected(PromptError{
            .message = std::format("prompt template not found: {}", path)});
    }

    std::ifstream file(file_path);
    if(!file.is_open()) {
        return std::unexpected(PromptError{
            .message = std::format("failed to open prompt template: {}", path)});
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    auto content = ss.str();

    if(file.bad() || (file.fail() && !file.eof()) || ss.bad()) {
        return std::unexpected(PromptError{
            .message = std::format("failed to read prompt template: {}", path)});
    }

    if(content.empty()) {
        return std::unexpected(PromptError{
            .message = std::format("prompt template is empty: {}", path)});
    }

    return content;
}

auto instantiate_prompt(const std::string& tmpl,
                        const EvidencePack& evidence) -> std::string {
    return instantiate_prompt_with_evidence(
        tmpl, evidence, format_evidence_text(evidence));
}

}  // namespace clore::generate
