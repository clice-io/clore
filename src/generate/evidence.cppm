export module generate:evidence;

import std;
import :model;
import extract;

export namespace clore::generate {

struct SymbolFact {
    extract::SymbolID id;
    std::string qualified_name;
    std::string signature;
    std::string kind_label;
    std::string access;
    bool is_template = false;
    std::string template_params;
    std::string declaration_file;
    std::uint32_t declaration_line = 0;
    std::string doc_comment;
};

struct EvidencePack {
    std::string page_id;
    std::string prompt_kind;
    std::string subject_name;
    std::string subject_kind;
    std::vector<SymbolFact> target_facts;
    std::vector<SymbolFact> local_context;
    std::vector<SymbolFact> dependency_context;
    std::vector<SymbolFact> reverse_usage_context;
    std::vector<std::string> related_page_summaries;
    std::vector<std::string> source_snippets;
};

auto build_evidence_for_namespace_summary(const extract::NamespaceInfo& ns,
                                          const extract::ProjectModel& model,
                                          const SymbolAnalysisStore& analyses,
                                          std::string_view project_root) -> EvidencePack;

auto build_evidence_for_function_analysis(const extract::SymbolInfo& target,
                                          const extract::ProjectModel& model,
                                          std::string_view project_root) -> EvidencePack;

auto build_evidence_for_type_analysis(const extract::SymbolInfo& target,
                                      const extract::ProjectModel& model,
                                      std::string_view project_root) -> EvidencePack;

auto build_evidence_for_variable_analysis(const extract::SymbolInfo& target,
                                          const extract::ProjectModel& model,
                                          std::string_view project_root) -> EvidencePack;

auto build_evidence_for_module_summary(const extract::ModuleUnit& mod,
                                       const extract::ProjectModel& model,
                                       const SymbolAnalysisStore& analyses,
                                       const PageSummaryCache& page_summaries,
                                       std::string_view project_root) -> EvidencePack;

auto build_evidence_for_module_architecture(const extract::ModuleUnit& mod,
                                            const extract::ProjectModel& model,
                                            const SymbolAnalysisStore& analyses,
                                            const PageSummaryCache& page_summaries,
                                            std::string_view project_root) -> EvidencePack;

auto build_evidence_for_index_overview(const extract::ProjectModel& model,
                                       const PageSummaryCache& page_summaries) -> EvidencePack;

auto build_evidence_for_function_declaration_summary(const extract::SymbolInfo& target,
                                                     const extract::ProjectModel& model,
                                                     const SymbolAnalysisStore& analyses,
                                                     std::string_view project_root) -> EvidencePack;

auto build_evidence_for_function_implementation_summary(const extract::SymbolInfo& target,
                                                        const extract::ProjectModel& model,
                                                        std::string_view project_root)
    -> EvidencePack;

auto build_evidence_for_type_declaration_summary(const extract::SymbolInfo& target,
                                                 const extract::ProjectModel& model,
                                                 const SymbolAnalysisStore& analyses,
                                                 std::string_view project_root) -> EvidencePack;

auto build_evidence_for_type_implementation_summary(const extract::SymbolInfo& target,
                                                    const extract::ProjectModel& model,
                                                    std::string_view project_root) -> EvidencePack;

auto format_evidence_text(const EvidencePack& pack) -> std::string;

auto format_evidence_text_bounded(const EvidencePack& pack, std::size_t max_length) -> std::string;

struct PromptError {
    std::string message;
};

auto build_prompt(PromptKind kind, const EvidencePack& evidence)
    -> std::expected<std::string, PromptError>;

}  // namespace clore::generate

export namespace clore::generate::__detail {

auto to_symbol_fact(const extract::SymbolInfo& sym, const std::string& project_root) -> SymbolFact;

auto collect_facts(const extract::ProjectModel& model,
                   const std::vector<extract::SymbolID>& ids,
                   const std::string& project_root) -> std::vector<SymbolFact>;

template <typename... Groups>
auto collect_merged_facts(const extract::ProjectModel& model,
                          const std::string& project_root,
                          const Groups&... groups) -> std::vector<SymbolFact>;

auto collect_summaries(const PageSummaryCache& cache, const std::vector<std::string>& keys)
    -> std::vector<std::string>;

template <typename Range, typename SymbolIDOf>
auto collect_analysis_summaries_impl(const SymbolAnalysisStore& analyses,
                                     const extract::ProjectModel& model,
                                     const Range& items,
                                     SymbolIDOf&& symbol_id_of) -> std::vector<std::string>;

auto collect_analysis_summaries(const SymbolAnalysisStore& analyses,
                                const extract::ProjectModel& model,
                                const std::vector<extract::SymbolID>& ids)
    -> std::vector<std::string>;

auto collect_analysis_summaries(const SymbolAnalysisStore& analyses,
                                const extract::ProjectModel& model,
                                const std::vector<SymbolFact>& facts) -> std::vector<std::string>;

/// Cached namespace-level fact collection. Avoids re-traversing namespace
/// symbols for every sibling when building evidence for multiple symbols
/// in the same namespace.
auto collect_namespace_facts_cached(const extract::ProjectModel& model,
                                    const std::string& namespace_name,
                                    const std::string& project_root,
                                    extract::SymbolKind target_kind,
                                    extract::SymbolID exclude_id) -> std::vector<SymbolFact>;

}  // namespace clore::generate::__detail

// ── implementation ──────────────────────────────────────────────────

namespace clore::generate {

namespace __detail {

auto to_symbol_fact(const extract::SymbolInfo& sym, const std::string& project_root) -> SymbolFact {
    return SymbolFact{
        .id = sym.id,
        .qualified_name = sym.qualified_name,
        .signature = sym.signature,
        .kind_label = std::string(extract::symbol_kind_name(sym.kind)),
        .access = sym.access,
        .is_template = sym.is_template,
        .template_params = sym.template_params,
        .declaration_file =
            clore::generate::make_source_relative(sym.declaration_location.file, project_root),
        .declaration_line = sym.declaration_location.line,
        .doc_comment = sym.doc_comment,
    };
}

auto collect_facts(const extract::ProjectModel& model,
                   const std::vector<extract::SymbolID>& ids,
                   const std::string& project_root) -> std::vector<SymbolFact> {
    std::vector<SymbolFact> facts;
    std::unordered_set<extract::SymbolID> seen;
    for(auto& id: ids) {
        if(!seen.insert(id).second)
            continue;
        if(auto* sym = extract::lookup_symbol(model, id)) {
            facts.push_back(to_symbol_fact(*sym, project_root));
        }
    }
    return facts;
}

template <typename... Groups>
auto collect_merged_facts(const extract::ProjectModel& model,
                          const std::string& project_root,
                          const Groups&... groups) -> std::vector<SymbolFact> {
    std::vector<SymbolFact> facts;
    std::unordered_set<extract::SymbolID> seen;

    auto append_group = [&](const auto& ids) {
        for(const auto& id: ids) {
            if(!seen.insert(id).second) {
                continue;
            }
            if(auto* sym = extract::lookup_symbol(model, id)) {
                facts.push_back(to_symbol_fact(*sym, project_root));
            }
        }
    };

    (append_group(groups), ...);
    return facts;
}

auto collect_summaries(const PageSummaryCache& cache, const std::vector<std::string>& keys)
    -> std::vector<std::string> {
    std::vector<std::string> result;
    for(auto& key: keys) {
        auto it = cache.find(key);
        if(it != cache.end() && !it->second.empty()) {
            result.push_back(it->second);
        }
    }
    return result;
}

template <typename Range, typename SymbolIDOf>
auto collect_analysis_summaries_impl(const SymbolAnalysisStore& analyses,
                                     const extract::ProjectModel& model,
                                     const Range& items,
                                     SymbolIDOf&& symbol_id_of) -> std::vector<std::string> {
    std::vector<std::string> result;
    std::unordered_set<extract::SymbolID> seen;
    result.reserve(items.size());

    for(const auto& item: items) {
        auto symbol_id = std::forward<SymbolIDOf>(symbol_id_of)(item);
        if(!seen.insert(symbol_id).second) {
            continue;
        }

        auto* symbol = extract::lookup_symbol(model, symbol_id);
        if(symbol == nullptr) {
            continue;
        }
        auto* summary = analysis_overview_markdown(analyses, *symbol);
        if(summary == nullptr || summary->empty()) {
            continue;
        }
        result.push_back(*summary);
    }

    return result;
}

auto collect_analysis_summaries(const SymbolAnalysisStore& analyses,
                                const extract::ProjectModel& model,
                                const std::vector<extract::SymbolID>& ids)
    -> std::vector<std::string> {
    return collect_analysis_summaries_impl(analyses, model, ids, [](extract::SymbolID id) {
        return id;
    });
}

auto collect_analysis_summaries(const SymbolAnalysisStore& analyses,
                                const extract::ProjectModel& model,
                                const std::vector<SymbolFact>& facts) -> std::vector<std::string> {
    return collect_analysis_summaries_impl(analyses, model, facts, [](const SymbolFact& fact) {
        return fact.id;
    });
}

namespace {

struct NamespaceFactsCacheEntry {
    std::string project_root;
    std::string namespace_name;
    std::vector<SymbolFact> all_functions;
    std::vector<SymbolFact> all_types;
    std::vector<SymbolFact> all_variables;
};

auto namespace_facts_cache() -> std::unordered_map<std::string, NamespaceFactsCacheEntry>& {
    thread_local std::unordered_map<std::string, NamespaceFactsCacheEntry> cache;
    return cache;
}

}  // namespace

auto collect_namespace_facts_cached(const extract::ProjectModel& model,
                                    const std::string& namespace_name,
                                    const std::string& project_root,
                                    extract::SymbolKind target_kind,
                                    extract::SymbolID exclude_id) -> std::vector<SymbolFact> {
    if(namespace_name.empty()) {
        return {};
    }

    auto cache_key = namespace_name + '\0' + project_root;
    auto& cache = namespace_facts_cache();
    auto it = cache.find(cache_key);

    if(it == cache.end()) {
        NamespaceFactsCacheEntry entry{
            .project_root = project_root,
            .namespace_name = namespace_name,
        };

        auto ns_it = model.namespaces.find(namespace_name);
        if(ns_it != model.namespaces.end()) {
            entry.all_functions.reserve(ns_it->second.symbols.size());
            entry.all_types.reserve(ns_it->second.symbols.size());
            entry.all_variables.reserve(ns_it->second.symbols.size());

            for(auto sym_id: ns_it->second.symbols) {
                if(auto* sym = extract::lookup_symbol(model, sym_id)) {
                    auto fact = to_symbol_fact(*sym, project_root);
                    if(is_function_kind(sym->kind)) {
                        entry.all_functions.push_back(std::move(fact));
                    } else if(is_type_kind(sym->kind)) {
                        entry.all_types.push_back(std::move(fact));
                    } else if(is_variable_kind(sym->kind)) {
                        entry.all_variables.push_back(std::move(fact));
                    }
                }
            }
        }

        it = cache.emplace(std::move(cache_key), std::move(entry)).first;
    }

    const std::vector<SymbolFact>* source = nullptr;
    if(is_function_kind(target_kind)) {
        source = &it->second.all_functions;
    } else if(is_type_kind(target_kind)) {
        source = &it->second.all_types;
    } else if(is_variable_kind(target_kind)) {
        source = &it->second.all_variables;
    }

    if(!source) {
        return {};
    }

    std::vector<SymbolFact> result;
    result.reserve(source->size());
    for(const auto& fact: *source) {
        if(fact.id != exclude_id) {
            result.push_back(fact);
        }
    }
    return result;
}

}  // namespace __detail

namespace {

auto render_detailed_fact(const SymbolFact& fact) -> std::string {
    std::string text;
    text += "- " + fact.kind_label + " `" + fact.qualified_name + "`";
    if(!fact.signature.empty()) {
        text += " — `" + fact.signature + "`";
    }
    text += "\n";
    if(!fact.access.empty()) {
        text += "  Access: " + fact.access + "\n";
    }
    if(fact.is_template && !fact.template_params.empty()) {
        text += "  Template: `" + fact.template_params + "`\n";
    }
    if(!fact.declaration_file.empty() && fact.declaration_line > 0) {
        text += "  Declared at: " + fact.declaration_file + ":" +
                std::to_string(fact.declaration_line) + "\n";
    }
    if(!fact.doc_comment.empty()) {
        text += "  Doc: " + fact.doc_comment + "\n";
    }
    return text;
}

auto render_context_fact(const SymbolFact& fact) -> std::string {
    std::string text;
    text += "- " + fact.kind_label + " `" + fact.qualified_name + "`";
    if(!fact.signature.empty()) {
        text += ": `" + fact.signature + "`";
    }
    text += "\n";
    return text;
}

auto render_source_snippet(const std::string& snippet) -> std::string {
    if(snippet.empty()) {
        return {};
    }

    std::string text;
    text.reserve(snippet.size() + 64);

    bool at_line_start = true;
    for(std::size_t i = 0; i < snippet.size(); ++i) {
        if(at_line_start) {
            text += "    ";
            at_line_start = false;
        }

        if(i + 2 < snippet.size() && snippet[i] == '`' && snippet[i + 1] == '`' &&
           snippet[i + 2] == '`') {
            text += "``\\`";
            i += 2;
            continue;
        }

        char ch = snippet[i];
        text.push_back(ch);
        if(ch == '\n') {
            at_line_start = true;
        }
    }

    if(!text.ends_with('\n')) {
        text.push_back('\n');
    }
    text.push_back('\n');
    return text;
}

auto render_summary_item(const std::string& summary) -> std::string {
    return "- " + summary + "\n";
}

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

constexpr std::string_view kFunctionAnalysisPrompt =
#include "../../templates/prompts/function_analysis.txt"
    ;

constexpr std::string_view kTypeAnalysisPrompt =
#include "../../templates/prompts/type_analysis.txt"
    ;

constexpr std::string_view kVariableAnalysisPrompt =
#include "../../templates/prompts/variable_analysis.txt"
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
        case PromptKind::FunctionAnalysis: return kFunctionAnalysisPrompt;
        case PromptKind::TypeAnalysis: return kTypeAnalysisPrompt;
        case PromptKind::VariableAnalysis: return kVariableAnalysisPrompt;
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

auto append_if_fits(std::string& text, std::string_view chunk, std::size_t max_length) -> bool {
    if(text.size() + chunk.size() > max_length) {
        return false;
    }
    text.append(chunk);
    return true;
}

template <typename Items, typename RenderItem>
auto append_section_bounded(std::string& text,
                            std::string_view title,
                            const Items& items,
                            RenderItem&& render_item,
                            std::size_t max_length) -> void {
    if(items.empty()) {
        return;
    }

    const auto checkpoint = text.size();
    if(!append_if_fits(text, title, max_length)) {
        return;
    }

    bool appended_any = false;
    for(const auto& item: items) {
        auto rendered = render_item(item);
        if(!append_if_fits(text, rendered, max_length)) {
            break;
        }
        appended_any = true;
    }

    if(!appended_any) {
        text.resize(checkpoint);
        return;
    }

    (void)append_if_fits(text, "\n", max_length);
}

}  // namespace

auto format_evidence_text(const EvidencePack& pack) -> std::string {
    return format_evidence_text_bounded(pack, std::numeric_limits<std::size_t>::max());
}

auto format_evidence_text_bounded(const EvidencePack& pack, std::size_t max_length) -> std::string {
    struct SymbolSection {
        std::string_view title;
        const std::vector<SymbolFact>* items = nullptr;
        std::string (*render)(const SymbolFact&) = nullptr;
    };

    struct TextSection {
        std::string_view title;
        const std::vector<std::string>* items = nullptr;
        std::string (*render)(const std::string&) = nullptr;
    };

    std::string text;
    text.reserve(4096);

    if(!append_if_fits(text, "## EVIDENCE\n\n", max_length)) {
        return {};
    }

    const std::array<SymbolSection, 4> symbol_sections{
        SymbolSection{
                      .title = "### Target\n",
                      .items = &pack.target_facts,
                      .render = &render_detailed_fact,
                      },
        SymbolSection{
                      .title = "### Local Context\n",
                      .items = &pack.local_context,
                      .render = &render_context_fact,
                      },
        SymbolSection{
                      .title = "### Dependencies\n",
                      .items = &pack.dependency_context,
                      .render = &render_context_fact,
                      },
        SymbolSection{
                      .title = "### Used By\n",
                      .items = &pack.reverse_usage_context,
                      .render = &render_context_fact,
                      },
    };

    for(const auto& section: symbol_sections) {
        append_section_bounded(text, section.title, *section.items, section.render, max_length);
    }

    const std::array<TextSection, 2> text_sections{
        TextSection{
                    .title = "### Source Snippets\n",
                    .items = &pack.source_snippets,
                    .render = &render_source_snippet,
                    },
        TextSection{
                    .title = "### Related Page Summaries\n",
                    .items = &pack.related_page_summaries,
                    .render = &render_summary_item,
                    },
    };

    for(const auto& section: text_sections) {
        append_section_bounded(text, section.title, *section.items, section.render, max_length);
    }

    return text;
}

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
