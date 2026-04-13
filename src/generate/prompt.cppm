module;

#include <expected>
#include <string>
#include <string_view>

#include <format>

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

auto prompt_template_of(PromptKind kind) -> std::string_view {
    switch(kind) {
        case PromptKind::NamespaceSummary:
            return R"(Write a summary for the namespace `{{target_name}}`.

Output:
- Markdown fragment only
- 1 to 2 short paragraphs
- No heading
- No YAML
- No code fences or pasted source code

Describe the namespace's responsibility, notable declarations, and architectural role.
Use only the EVIDENCE section below.

{{evidence}})";
        case PromptKind::ModuleSummary:
            return R"(Write a summary for the module `{{target_name}}`.

Output:
- Markdown fragment only
- 1 to 2 short paragraphs
- No heading
- No YAML
- No code fences or pasted source code

Describe the module's responsibility and the public-facing implementation scope it owns.
Use only the EVIDENCE section below.

{{evidence}})";
        case PromptKind::ModuleArchitecture:
            return R"(Write an architecture note for the module `{{target_name}}`.

Output:
- Markdown fragment only
- 1 to 2 short paragraphs
- No heading
- No YAML
- No code fences or pasted source code

Focus on decomposition, imports, internal layering, and implementation structure.
Use only the EVIDENCE section below.

{{evidence}})";
        case PromptKind::IndexOverview:
            return R"(Write a top-level overview for this C++ project documentation set.

Output:
- Markdown fragment only
- 1 to 2 short paragraphs
- No heading
- No YAML
- No code fences or pasted source code

Summarize the main subsystems and how readers should understand the project at a high level.
Use only the EVIDENCE section below.

{{evidence}})";
        case PromptKind::IndexReadingGuide:
            return R"(Write a reading guide for this C++ project documentation set.

Output:
- Markdown fragment only
- 1 short paragraph or compact bullet list
- No heading
- No YAML
- No code fences or pasted source code

Explain how readers should navigate declarations, implementations, and workflows.
Use only the EVIDENCE section below.

{{evidence}})";
        case PromptKind::Workflow:
            return R"(Write a workflow page for `{{target_name}}`.

Output:
- Markdown fragment only
- May start with `Title: ...` on the first line if you want to refine the page title
- Use headings only from level 2 downward
- No YAML
- No code fences or pasted source code

Describe the workflow, key stages, and the responsibilities of the participating symbols.
Use only the EVIDENCE section below.

{{evidence}})";
        case PromptKind::FunctionDeclarationSummary:
            return R"(Write a declaration summary for the {{target_kind}} `{{target_name}}`.

Context: this summary appears on a declaration page. Focus on caller-facing responsibility and contract.

Output:
- Markdown fragment only
- 1 to 2 short paragraphs
- No heading
- No YAML
- No code fences or pasted source code

Do not describe implementation details.
Use only the EVIDENCE section below.

{{evidence}})";
        case PromptKind::FunctionImplementationSummary:
            return R"(Write an implementation summary for the {{target_kind}} `{{target_name}}`.

Context: this summary appears on an implementation page. Focus on algorithm, internal control flow, and dependencies.

Output:
- Markdown fragment only
- 1 to 2 short paragraphs
- No heading
- No YAML
- No code fences or pasted source code

Do not restate the public contract.
Use only the EVIDENCE section below.

{{evidence}})";
        case PromptKind::TypeDeclarationSummary:
            return R"(Write a declaration summary for the {{target_kind}} `{{target_name}}`.

Context: this summary appears on a declaration page. Focus on what the type represents and how it is used.

Output:
- Markdown fragment only
- 1 to 2 short paragraphs
- No heading
- No YAML
- No code fences or pasted source code

Do not focus on member-by-member implementation detail.
Use only the EVIDENCE section below.

{{evidence}})";
        case PromptKind::TypeImplementationSummary:
            return R"(Write an implementation summary for the {{target_kind}} `{{target_name}}`.

Context: this summary appears on an implementation page. Focus on internal structure, invariants, and important member implementations.

Output:
- Markdown fragment only
- 1 to 2 short paragraphs
- No heading
- No YAML
- No code fences or pasted source code

Do not restate the public API contract.
Use only the EVIDENCE section below.

{{evidence}})";
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
            .message = std::format("unsupported prompt kind: {}",
                                   prompt_kind_name(kind))});
    }
    return instantiate_prompt_with_evidence(
        std::string(tmpl), evidence, format_evidence_text(evidence));
}

}  // namespace clore::generate
