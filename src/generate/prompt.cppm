module;

#include <expected>
#include <format>
#include <string>
#include <string_view>

export module generate:prompt;

import :model;
import :evidence;

export namespace clore::generate {

struct PromptError {
    std::string message;
};

auto build_prompt(PromptKind kind, const EvidencePack& evidence)
    -> std::expected<std::string, PromptError>;

}  // namespace clore::generate

// ── implementation ──────────────────────────────────────────────────

namespace clore::generate {

namespace {

constexpr std::string_view kNamespaceSummaryPrompt =
#include "../../templates/prompts/namespace_summary.txt"
    ;

constexpr std::string_view kModuleSummaryPrompt =
#include "../../templates/prompts/module_summary.txt"
    ;

constexpr std::string_view kModuleArchitecturePrompt =
#include "../../templates/prompts/module_architecture.txt"
    ;

constexpr std::string_view kIndexOverviewPrompt =
#include "../../templates/prompts/index_overview.txt"
    ;

constexpr std::string_view kFunctionDeclarationSummaryPrompt =
#include "../../templates/prompts/function_declaration_summary.txt"
    ;

constexpr std::string_view kFunctionImplementationSummaryPrompt =
#include "../../templates/prompts/function_implementation_summary.txt"
    ;

constexpr std::string_view kTypeDeclarationSummaryPrompt =
#include "../../templates/prompts/type_declaration_summary.txt"
    ;

constexpr std::string_view kTypeImplementationSummaryPrompt =
#include "../../templates/prompts/type_implementation_summary.txt"
    ;

auto prompt_template_of(PromptKind kind) -> std::string_view {
    switch(kind) {
        case PromptKind::NamespaceSummary: return kNamespaceSummaryPrompt;
        case PromptKind::ModuleSummary: return kModuleSummaryPrompt;
        case PromptKind::ModuleArchitecture: return kModuleArchitecturePrompt;
        case PromptKind::IndexOverview: return kIndexOverviewPrompt;
        case PromptKind::FunctionDeclarationSummary: return kFunctionDeclarationSummaryPrompt;
        case PromptKind::FunctionImplementationSummary: return kFunctionImplementationSummaryPrompt;
        case PromptKind::TypeDeclarationSummary: return kTypeDeclarationSummaryPrompt;
        case PromptKind::TypeImplementationSummary: return kTypeImplementationSummaryPrompt;
    }
    return {};
}

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

auto build_prompt(PromptKind kind, const EvidencePack& evidence)
    -> std::expected<std::string, PromptError> {
    auto tmpl = prompt_template_of(kind);
    if(tmpl.empty()) {
        return std::unexpected(PromptError{
            .message = std::format("unsupported prompt kind: {}", prompt_kind_name(kind))});
    }
    return instantiate_prompt_with_evidence(std::string(tmpl),
                                            evidence,
                                            format_evidence_text(evidence));
}

}  // namespace clore::generate
