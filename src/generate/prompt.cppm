module;

#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

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
                        const EvidencePack& evidence,
                        std::string_view failure_marker) -> std::string;

auto instantiate_prompt_bounded(const std::string& tmpl,
                                const EvidencePack& evidence,
                                std::string_view failure_marker,
                                std::uint32_t max_length)
    -> std::expected<std::string, PromptError>;

auto validate_prompt(const std::string& prompt, std::uint32_t max_length)
    -> std::expected<void, PromptError>;

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

auto count_evidence_markers(const std::string& tmpl) -> std::size_t {
    std::size_t count = 0;
    std::size_t pos = 0;
    while(pos < tmpl.size()) {
        auto marker_start = tmpl.find("{{", pos);
        if(marker_start == std::string::npos) {
            break;
        }

        auto marker_end = tmpl.find("}}", marker_start);
        if(marker_end == std::string::npos) {
            break;
        }

        auto var_name = tmpl.substr(marker_start + 2, marker_end - marker_start - 2);
        if(var_name == "evidence") {
            ++count;
        }
        pos = marker_end + 2;
    }
    return count;
}

auto instantiate_prompt_with_evidence(const std::string& tmpl,
                                      const EvidencePack& evidence,
                                      std::string_view failure_marker,
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
        if(var_name == "failure_marker") {
            result.append(failure_marker);
        } else if(var_name == "evidence") {
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
    if(!fs::exists(file_path)) {
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

    if(content.empty()) {
        return std::unexpected(PromptError{
            .message = std::format("prompt template is empty: {}", path)});
    }

    return content;
}

auto instantiate_prompt(const std::string& tmpl,
                        const EvidencePack& evidence,
                        std::string_view failure_marker) -> std::string {
    return instantiate_prompt_with_evidence(
        tmpl, evidence, failure_marker, format_evidence_text(evidence));
}

auto instantiate_prompt_bounded(const std::string& tmpl,
                                const EvidencePack& evidence,
                                std::string_view failure_marker,
                                std::uint32_t max_length)
    -> std::expected<std::string, PromptError> {
    auto prompt_without_evidence = instantiate_prompt_with_evidence(
        tmpl, evidence, failure_marker, "");

    const auto evidence_marker_count = count_evidence_markers(tmpl);
    if(prompt_without_evidence.size() > max_length) {
        return std::unexpected(PromptError{
            .message = std::format(
                "prompt template content exceeds max length before evidence: {} > {}",
                prompt_without_evidence.size(), max_length)});
    }

    std::string evidence_text;
    if(evidence_marker_count > 0) {
        const auto evidence_budget =
            (static_cast<std::size_t>(max_length) - prompt_without_evidence.size()) /
            evidence_marker_count;
        evidence_text = format_evidence_text_bounded(evidence, evidence_budget);
        if(evidence_text.empty()) {
            return std::unexpected(PromptError{
                .message = std::format(
                    "prompt template leaves no room for evidence within max length {}",
                    max_length)});
        }
    }

    auto prompt = instantiate_prompt_with_evidence(
        tmpl, evidence, failure_marker, evidence_text);
    auto validation = validate_prompt(prompt, max_length);
    if(!validation.has_value()) {
        return std::unexpected(std::move(validation.error()));
    }
    return prompt;
}

auto validate_prompt(const std::string& prompt, std::uint32_t max_length)
    -> std::expected<void, PromptError> {
    if(prompt.empty()) {
        return std::unexpected(PromptError{.message = "rendered prompt is empty"});
    }
    if(prompt.size() > max_length) {
        return std::unexpected(PromptError{
            .message = std::format("rendered prompt exceeds max length: {} > {}",
                                   prompt.size(), max_length)});
    }
    return {};
}

}  // namespace clore::generate
