module;

#include "kota/codec/json/json.h"

export module generate:analysis;

import std;
import :evidence;
import :model;
import config;
import extract;
import support;

export namespace clore::generate {

template <typename T>
auto parse_structured_response(std::string_view raw, std::string_view context)
    -> std::expected<T, GenerateError>;

auto normalize_markdown_fragment(std::string_view raw, std::string_view context)
    -> std::expected<std::string, GenerateError>;

auto parse_markdown_prompt_output(std::string_view raw, std::string_view context)
    -> std::expected<std::string, GenerateError>;

auto analysis_prompt_kind_for_symbol(const extract::SymbolInfo& sym) -> std::optional<PromptKind>;

auto symbol_prompt_kinds_for_symbol(const extract::SymbolInfo& sym) -> std::vector<PromptKind>;

auto is_base_symbol_analysis_prompt(PromptKind kind) -> bool;

auto is_declaration_summary_prompt(PromptKind kind) -> bool;

auto store_fallback_analysis(SymbolAnalysisStore& analyses,
                             const extract::SymbolInfo& sym,
                             const extract::ProjectModel& model) -> void;

auto apply_symbol_analysis_response(SymbolAnalysisStore& analyses,
                                    const extract::SymbolInfo& sym,
                                    const extract::ProjectModel& model,
                                    PromptKind kind,
                                    std::string_view raw_response)
    -> std::expected<void, GenerateError>;

auto build_symbol_analysis_prompt(const extract::SymbolInfo& sym,
                                  PromptKind kind,
                                  const extract::ProjectModel& model,
                                  const config::TaskConfig& config,
                                  const SymbolAnalysisStore& analyses)
    -> std::expected<std::string, GenerateError>;

}  // namespace clore::generate

namespace clore::generate {

namespace json = kota::codec::json;

namespace {

auto trim_trailing_ascii_whitespace(std::string& text) -> void {
    while(!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
        text.pop_back();
    }
}

auto contains_non_whitespace(std::string_view text) -> bool {
    return std::ranges::any_of(text, [](char ch) {
        return std::isspace(static_cast<unsigned char>(ch)) == 0;
    });
}

auto fallback_overview_markdown(const extract::SymbolInfo& sym) -> std::string {
    if(auto doc = clore::support::extract_first_plain_paragraph(sym.doc_comment); !doc.empty()) {
        return doc;
    }
    return std::format("`{}` is a {}.", sym.qualified_name, extract::symbol_kind_name(sym.kind));
}

auto fallback_function_analysis(const extract::SymbolInfo& sym) -> FunctionAnalysis {
    FunctionAnalysis analysis;
    analysis.overview_markdown = fallback_overview_markdown(sym);
    analysis.details_markdown =
        !sym.signature.empty()
            ? std::format("Implements `{}`.", sym.signature)
            : std::format(
                  "Implementation details for `{}` are available in the " "extracted source.",
                  sym.qualified_name);
    if(!sym.called_by.empty() || !sym.referenced_by.empty()) {
        analysis.usage_patterns.push_back("Referenced by related project symbols.");
    }
    return analysis;
}

auto fallback_type_analysis(const extract::SymbolInfo& sym, const extract::ProjectModel& model)
    -> TypeAnalysis {
    TypeAnalysis analysis;
    analysis.overview_markdown = fallback_overview_markdown(sym);
    analysis.details_markdown =
        std::format("`{}` groups related members and implementation details.", sym.qualified_name);

    for(auto child_id: sym.children) {
        if(auto* child = extract::lookup_symbol(model, child_id); child != nullptr) {
            if(analysis.key_members.size() >= 6) {
                break;
            }
            auto label = child->name.empty() ? child->qualified_name : child->name;
            analysis.key_members.push_back(
                std::format("{} {}", extract::symbol_kind_name(child->kind), label));
        }
    }
    if(!sym.derived.empty()) {
        analysis.usage_patterns.push_back("Extended by derived types in the project.");
    }
    return analysis;
}

auto fallback_variable_analysis(const extract::SymbolInfo& sym) -> VariableAnalysis {
    VariableAnalysis analysis;
    analysis.overview_markdown = fallback_overview_markdown(sym);
    analysis.details_markdown =
        std::format("`{}` participates in surrounding logic where it is referenced.",
                    sym.qualified_name);
    if(!sym.referenced_by.empty()) {
        analysis.usage_patterns.push_back("Read or referenced by related symbols.");
    }
    return analysis;
}

auto merge_function_analysis(FunctionAnalysis& target,
                             const FunctionAnalysis& fallback,
                             FunctionAnalysis parsed) -> void {
    if(target.overview_markdown.empty() || target.overview_markdown == fallback.overview_markdown) {
        target.overview_markdown = std::move(parsed.overview_markdown);
    }
    if(target.details_markdown.empty() || target.details_markdown == fallback.details_markdown) {
        target.details_markdown = std::move(parsed.details_markdown);
    }
    target.has_side_effects = parsed.has_side_effects;
    target.side_effects = std::move(parsed.side_effects);
    target.reads_from = std::move(parsed.reads_from);
    target.writes_to = std::move(parsed.writes_to);
    target.usage_patterns = std::move(parsed.usage_patterns);
}

auto merge_type_analysis(TypeAnalysis& target, const TypeAnalysis& fallback, TypeAnalysis parsed)
    -> void {
    if(target.overview_markdown.empty() || target.overview_markdown == fallback.overview_markdown) {
        target.overview_markdown = std::move(parsed.overview_markdown);
    }
    if(target.details_markdown.empty() || target.details_markdown == fallback.details_markdown) {
        target.details_markdown = std::move(parsed.details_markdown);
    }
    target.invariants = std::move(parsed.invariants);
    target.key_members = std::move(parsed.key_members);
    target.usage_patterns = std::move(parsed.usage_patterns);
}

auto parse_function_analysis_lenient(std::string_view raw, std::string_view context)
    -> std::expected<FunctionAnalysis, GenerateError> {
    auto parsed = parse_structured_response<FunctionAnalysis>(raw, context);
    if(parsed.has_value()) {
        return parsed;
    }

    auto normalized = normalize_markdown_fragment(raw, context);
    if(!normalized.has_value()) {
        return std::unexpected(std::move(parsed.error()));
    }

    FunctionAnalysis fallback{};
    fallback.overview_markdown = std::move(*normalized);
    fallback.details_markdown = fallback.overview_markdown;
    return fallback;
}

auto parse_type_analysis_lenient(std::string_view raw, std::string_view context)
    -> std::expected<TypeAnalysis, GenerateError> {
    auto parsed = parse_structured_response<TypeAnalysis>(raw, context);
    if(parsed.has_value()) {
        return parsed;
    }

    auto normalized = normalize_markdown_fragment(raw, context);
    if(!normalized.has_value()) {
        return std::unexpected(std::move(parsed.error()));
    }

    TypeAnalysis fallback{};
    fallback.overview_markdown = std::move(*normalized);
    fallback.details_markdown = fallback.overview_markdown;
    return fallback;
}

auto parse_variable_analysis_lenient(std::string_view raw, std::string_view context)
    -> std::expected<VariableAnalysis, GenerateError> {
    auto parsed = parse_structured_response<VariableAnalysis>(raw, context);
    if(parsed.has_value()) {
        return parsed;
    }

    auto normalized = normalize_markdown_fragment(raw, context);
    if(!normalized.has_value()) {
        return std::unexpected(std::move(parsed.error()));
    }

    VariableAnalysis fallback{};
    fallback.overview_markdown = std::move(*normalized);
    fallback.details_markdown = fallback.overview_markdown;
    return fallback;
}

}  // namespace

template <typename T>
auto parse_structured_response(std::string_view raw, std::string_view context)
    -> std::expected<T, GenerateError> {
    auto parsed = json::from_json<T>(raw);
    if(!parsed.has_value()) {
        return std::unexpected(GenerateError{
            .message = std::format("failed to parse structured response for {}: {}",
                                   context,
                                   parsed.error().to_string()),
        });
    }
    return *parsed;
}

auto normalize_markdown_fragment(std::string_view raw, std::string_view context)
    -> std::expected<std::string, GenerateError> {
    auto normalized = clore::support::ensure_utf8(raw);
    normalized = std::string(clore::support::strip_utf8_bom(normalized));
    trim_trailing_ascii_whitespace(normalized);
    if(!contains_non_whitespace(normalized)) {
        return std::unexpected(GenerateError{
            .message = std::format("empty markdown fragment for {}", context),
        });
    }
    return normalized;
}

auto parse_markdown_prompt_output(std::string_view raw, std::string_view context)
    -> std::expected<std::string, GenerateError> {
    return normalize_markdown_fragment(raw, context);
}

auto analysis_prompt_kind_for_symbol(const extract::SymbolInfo& sym) -> std::optional<PromptKind> {
    if(is_function_kind(sym.kind)) {
        return PromptKind::FunctionAnalysis;
    }
    if(is_type_kind(sym.kind)) {
        return PromptKind::TypeAnalysis;
    }
    if(is_variable_kind(sym.kind)) {
        return PromptKind::VariableAnalysis;
    }
    return std::nullopt;
}

auto symbol_prompt_kinds_for_symbol(const extract::SymbolInfo& sym) -> std::vector<PromptKind> {
    auto base_kind = analysis_prompt_kind_for_symbol(sym);
    if(!base_kind.has_value()) {
        return {};
    }

    if(*base_kind == PromptKind::FunctionAnalysis) {
        return {
            *base_kind,
            PromptKind::FunctionDeclarationSummary,
            PromptKind::FunctionImplementationSummary,
        };
    }
    if(*base_kind == PromptKind::TypeAnalysis) {
        return {
            *base_kind,
            PromptKind::TypeDeclarationSummary,
            PromptKind::TypeImplementationSummary,
        };
    }
    if(*base_kind == PromptKind::VariableAnalysis) {
        return {*base_kind};
    }
    return {};
}

auto is_base_symbol_analysis_prompt(PromptKind kind) -> bool {
    return kind == PromptKind::FunctionAnalysis || kind == PromptKind::TypeAnalysis ||
           kind == PromptKind::VariableAnalysis;
}

auto is_declaration_summary_prompt(PromptKind kind) -> bool {
    return kind == PromptKind::FunctionDeclarationSummary ||
           kind == PromptKind::TypeDeclarationSummary;
}

auto store_fallback_analysis(SymbolAnalysisStore& analyses,
                             const extract::SymbolInfo& sym,
                             const extract::ProjectModel& model) -> void {
    auto target_key = make_symbol_target_key(sym);
    if(is_function_kind(sym.kind)) {
        analyses.functions[target_key] = fallback_function_analysis(sym);
    } else if(is_type_kind(sym.kind)) {
        analyses.types[target_key] = fallback_type_analysis(sym, model);
    } else if(is_variable_kind(sym.kind)) {
        analyses.variables[target_key] = fallback_variable_analysis(sym);
    }
}

auto apply_symbol_analysis_response(SymbolAnalysisStore& analyses,
                                    const extract::SymbolInfo& sym,
                                    const extract::ProjectModel& model,
                                    PromptKind kind,
                                    std::string_view raw_response)
    -> std::expected<void, GenerateError> {
    auto target_key = make_symbol_target_key(sym);

    switch(kind) {
        case PromptKind::FunctionAnalysis: {
            auto parsed = parse_function_analysis_lenient(
                raw_response,
                prompt_request_key(PromptRequest{.kind = kind, .target_key = target_key}));
            if(!parsed.has_value()) {
                return std::unexpected(std::move(parsed.error()));
            }
            auto fallback = fallback_function_analysis(sym);
            merge_function_analysis(analyses.functions[target_key], fallback, std::move(*parsed));
            return {};
        }
        case PromptKind::FunctionDeclarationSummary:
        case PromptKind::FunctionImplementationSummary: {
            auto parsed = parse_markdown_prompt_output(
                raw_response,
                prompt_request_key(PromptRequest{.kind = kind, .target_key = target_key}));
            if(!parsed.has_value()) {
                return std::unexpected(std::move(parsed.error()));
            }
            auto& analysis = analyses.functions[target_key];
            if(kind == PromptKind::FunctionDeclarationSummary) {
                analysis.overview_markdown = std::move(*parsed);
            } else {
                analysis.details_markdown = std::move(*parsed);
            }
            return {};
        }
        case PromptKind::TypeAnalysis: {
            auto parsed = parse_type_analysis_lenient(
                raw_response,
                prompt_request_key(PromptRequest{.kind = kind, .target_key = target_key}));
            if(!parsed.has_value()) {
                return std::unexpected(std::move(parsed.error()));
            }
            auto fallback = fallback_type_analysis(sym, model);
            merge_type_analysis(analyses.types[target_key], fallback, std::move(*parsed));
            return {};
        }
        case PromptKind::TypeDeclarationSummary:
        case PromptKind::TypeImplementationSummary: {
            auto parsed = parse_markdown_prompt_output(
                raw_response,
                prompt_request_key(PromptRequest{.kind = kind, .target_key = target_key}));
            if(!parsed.has_value()) {
                return std::unexpected(std::move(parsed.error()));
            }
            auto& analysis = analyses.types[target_key];
            if(kind == PromptKind::TypeDeclarationSummary) {
                analysis.overview_markdown = std::move(*parsed);
            } else {
                analysis.details_markdown = std::move(*parsed);
            }
            return {};
        }
        case PromptKind::VariableAnalysis: {
            auto parsed = parse_variable_analysis_lenient(
                raw_response,
                prompt_request_key(PromptRequest{.kind = kind, .target_key = target_key}));
            if(!parsed.has_value()) {
                return std::unexpected(std::move(parsed.error()));
            }
            analyses.variables[target_key] = std::move(*parsed);
            return {};
        }
        default:
            return std::unexpected(GenerateError{
                .message =
                    std::format("unsupported analysis prompt kind '{}'", prompt_kind_name(kind)),
            });
    }
}

auto build_symbol_analysis_prompt(const extract::SymbolInfo& sym,
                                  PromptKind kind,
                                  const extract::ProjectModel& model,
                                  const config::TaskConfig& config,
                                  const SymbolAnalysisStore& analyses)
    -> std::expected<std::string, GenerateError> {
    EvidencePack evidence;
    switch(kind) {
        case PromptKind::FunctionAnalysis:
            evidence = build_evidence_for_function_analysis(sym, model, config.project_root);
            break;
        case PromptKind::FunctionDeclarationSummary:
            evidence = build_evidence_for_function_declaration_summary(sym,
                                                                       model,
                                                                       analyses,
                                                                       config.project_root);
            break;
        case PromptKind::FunctionImplementationSummary:
            evidence =
                build_evidence_for_function_implementation_summary(sym, model, config.project_root);
            break;
        case PromptKind::TypeAnalysis:
            evidence = build_evidence_for_type_analysis(sym, model, config.project_root);
            break;
        case PromptKind::TypeDeclarationSummary:
            evidence = build_evidence_for_type_declaration_summary(sym,
                                                                   model,
                                                                   analyses,
                                                                   config.project_root);
            break;
        case PromptKind::TypeImplementationSummary:
            evidence =
                build_evidence_for_type_implementation_summary(sym, model, config.project_root);
            break;
        case PromptKind::VariableAnalysis:
            evidence = build_evidence_for_variable_analysis(sym, model, config.project_root);
            break;
        default:
            return std::unexpected(GenerateError{
                .message =
                    std::format("unsupported analysis prompt kind '{}'", prompt_kind_name(kind)),
            });
    }

    evidence.page_id = "symbol_analysis_phase";
    evidence.prompt_kind = std::string(prompt_kind_name(kind));
    evidence.subject_name = sym.qualified_name;

    auto prompt = build_prompt(kind, evidence);
    if(!prompt.has_value()) {
        return std::unexpected(GenerateError{
            .message = std::format("failed to build analysis prompt for '{}': {}",
                                   make_symbol_target_key(sym),
                                   prompt.error().message),
        });
    }

    return std::move(*prompt);
}

}  // namespace clore::generate
