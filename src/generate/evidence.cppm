module;

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

export module generate:evidence;

import :model;
import config;
import extract;
import support;

export namespace clore::generate {

auto build_evidence_for_type_overview(
    const extract::SymbolInfo& target,
    const extract::ProjectModel& model,
    const config::EvidenceRulesConfig& rules,
    const PageSummaryCache& summaries,
    std::string_view project_root) -> EvidencePack;

auto build_evidence_for_type_usage_notes(
    const extract::SymbolInfo& target,
    const extract::ProjectModel& model,
    const config::EvidenceRulesConfig& rules,
    std::string_view project_root) -> EvidencePack;

auto build_evidence_for_namespace_summary(
    const extract::NamespaceInfo& ns,
    const extract::ProjectModel& model,
    const config::EvidenceRulesConfig& rules,
    const PageSummaryCache& summaries,
    std::string_view project_root) -> EvidencePack;

auto build_evidence_for_module_summary(
    const extract::ModuleUnit& mod,
    const extract::ProjectModel& model,
    const config::EvidenceRulesConfig& rules,
    const PageSummaryCache& summaries,
    std::string_view project_root) -> EvidencePack;

auto build_evidence_for_module_architecture(
    const extract::ModuleUnit& mod,
    const extract::ProjectModel& model,
    const config::EvidenceRulesConfig& rules,
    const PageSummaryCache& summaries,
    std::string_view project_root) -> EvidencePack;

auto build_evidence_for_index_overview(
    const extract::ProjectModel& model,
    const config::EvidenceRulesConfig& rules,
    const PageSummaryCache& summaries) -> EvidencePack;

auto build_evidence_for_workflow(
    const std::vector<std::string>& chain_keys,
    const extract::ProjectModel& model,
    const config::EvidenceRulesConfig& rules,
    const PageSummaryCache& summaries,
    std::string_view project_root) -> EvidencePack;

auto format_evidence_text(const EvidencePack& pack) -> std::string;

auto format_evidence_text_bounded(const EvidencePack& pack, std::size_t max_length)
    -> std::string;

}  // namespace clore::generate

// ── implementation ──────────────────────────────────────────────────

namespace clore::generate {

namespace {

auto to_symbol_fact(const extract::SymbolInfo& sym, const std::string& project_root) -> SymbolFact {
    return SymbolFact{
        .id = sym.id,
        .qualified_name = sym.qualified_name,
        .signature = sym.signature,
        .kind_label = std::string(extract::symbol_kind_name(sym.kind)),
        .access = sym.access,
        .is_template = sym.is_template,
        .template_params = sym.template_params,
        .declaration_file = clore::generate::make_source_relative(sym.declaration_location.file, project_root),
        .declaration_line = sym.declaration_location.line,
        .doc_comment = sym.doc_comment,
    };
}

auto lookup(const extract::ProjectModel& model, extract::SymbolID id)
    -> const extract::SymbolInfo* {
    return clore::generate::lookup_sym(model, id);
}

auto collect_facts(const extract::ProjectModel& model,
                   const std::vector<extract::SymbolID>& ids,
                   std::uint32_t max_count,
                   const std::string& project_root) -> std::vector<SymbolFact> {
    std::vector<SymbolFact> facts;
    std::unordered_set<extract::SymbolID> seen;
    for(auto& id : ids) {
        if(facts.size() >= max_count) break;
        if(!seen.insert(id).second) continue;
        if(auto* sym = lookup(model, id)) {
            facts.push_back(to_symbol_fact(*sym, project_root));
        }
    }
    return facts;
}

auto collect_summaries(const PageSummaryCache& cache,
                       const std::vector<std::string>& keys,
                       std::uint32_t max_count) -> std::vector<std::string> {
    std::vector<std::string> result;
    for(auto& key : keys) {
        if(result.size() >= max_count) break;
        auto it = cache.find(key);
        if(it != cache.end() && !it->second.empty()) {
            result.push_back(it->second);
        }
    }
    return result;
}

auto truncate_snippet(const std::string& snippet, std::uint32_t max_bytes) -> std::string {
    auto normalized = clore::support::ensure_utf8(snippet);
    if(normalized.size() <= max_bytes) return normalized;
    return clore::support::truncate_utf8(normalized, max_bytes) + "\n// ... truncated";
}

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
    return "```cpp\n" + snippet + "\n```\n";
}

auto render_summary_item(const std::string& summary) -> std::string {
    return "- " + summary + "\n";
}

auto append_if_fits(std::string& text, std::string_view chunk, std::size_t max_length) -> bool {
    if(text.size() + chunk.size() > max_length) {
        return false;
    }
    text.append(chunk);
    return true;
}

template <typename Items, typename RenderItem>
auto append_section_bounded(std::string& text, std::string_view title,
                            const Items& items, RenderItem&& render_item,
                            std::size_t max_length) -> void {
    if(items.empty()) {
        return;
    }

    const auto checkpoint = text.size();
    if(!append_if_fits(text, title, max_length)) {
        return;
    }

    bool appended_any = false;
    for(const auto& item : items) {
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

auto build_evidence_for_type_overview(
    const extract::SymbolInfo& target,
    const extract::ProjectModel& model,
    const config::EvidenceRulesConfig& rules,
    const PageSummaryCache& summaries,
    std::string_view project_root) -> EvidencePack {

    auto root = std::string(project_root);
    EvidencePack pack;
    pack.slot_kind = "type_overview";
    pack.subject_name = target.qualified_name;
    pack.subject_kind = std::string(extract::symbol_kind_name(target.kind));
    pack.target_facts.push_back(to_symbol_fact(target, root));

    // Base types
    pack.dependency_context = collect_facts(model, target.bases, rules.max_callees, root);

    // Derived types
    pack.reverse_usage_context = collect_facts(model, target.derived, rules.max_callers, root);

    // Public members as local context
    std::vector<extract::SymbolID> member_ids;
    for(auto& child_id : target.children) {
        if(auto* child = lookup(model, child_id)) {
            if(child->access.empty() || child->access == "public") {
                member_ids.push_back(child_id);
            }
        }
    }
    pack.local_context = collect_facts(model, member_ids, rules.max_siblings, root);

    // Direct callers + callees
    auto callers = collect_facts(model, target.called_by, rules.max_callers, root);
    auto callees = collect_facts(model, target.calls, rules.max_callees, root);
    for(auto& f : callers) pack.reverse_usage_context.push_back(std::move(f));
    for(auto& f : callees) pack.dependency_context.push_back(std::move(f));

    // Parent namespace
    if(target.parent.has_value()) {
        if(auto* parent = lookup(model, *target.parent)) {
            pack.local_context.push_back(to_symbol_fact(*parent, root));
        }
    }

    // Source snippet
    if(!target.source_snippet.empty()) {
        pack.source_snippets.push_back(truncate_snippet(target.source_snippet, rules.max_source_bytes));
    }

    // Doc comment (already in target_facts)

    // Related page summaries
    std::vector<std::string> summary_keys;
    for(auto& base_id : target.bases) {
        if(auto* base = lookup(model, base_id)) {
            summary_keys.push_back(base->qualified_name);
        }
    }
    pack.related_page_summaries = collect_summaries(summaries, summary_keys, rules.max_related_summaries);

    return pack;
}

auto build_evidence_for_type_usage_notes(
    const extract::SymbolInfo& target,
    const extract::ProjectModel& model,
    const config::EvidenceRulesConfig& rules,
    std::string_view project_root) -> EvidencePack {

    auto root = std::string(project_root);
    EvidencePack pack;
    pack.slot_kind = "type_usage_notes";
    pack.subject_name = target.qualified_name;
    pack.subject_kind = std::string(extract::symbol_kind_name(target.kind));
    pack.target_facts.push_back(to_symbol_fact(target, root));

    pack.reverse_usage_context = collect_facts(model, target.called_by, rules.max_callers, root);
    auto refs = collect_facts(model, target.referenced_by, rules.max_callers, root);
    for(auto& f : refs) pack.reverse_usage_context.push_back(std::move(f));

    // Selected methods
    std::vector<extract::SymbolID> method_ids;
    for(auto& child_id : target.children) {
        if(auto* child = lookup(model, child_id)) {
            if(child->kind == extract::SymbolKind::Method ||
               child->kind == extract::SymbolKind::Function) {
                method_ids.push_back(child_id);
            }
        }
    }
    pack.local_context = collect_facts(model, method_ids, rules.max_siblings, root);

    if(!target.source_snippet.empty()) {
        pack.source_snippets.push_back(truncate_snippet(target.source_snippet, rules.max_source_bytes));
    }

    return pack;
}

auto build_evidence_for_namespace_summary(
    const extract::NamespaceInfo& ns,
    const extract::ProjectModel& model,
    const config::EvidenceRulesConfig& rules,
    const PageSummaryCache& summaries,
    std::string_view project_root) -> EvidencePack {

    auto root = std::string(project_root);

    EvidencePack pack;
    pack.slot_kind = "namespace_summary";
    pack.subject_name = ns.name;
    pack.subject_kind = "namespace";

    SymbolFact namespace_fact;
    namespace_fact.qualified_name = ns.name;
    namespace_fact.kind_label = "namespace";
    pack.target_facts.push_back(std::move(namespace_fact));

    // Types in namespace
    std::vector<extract::SymbolID> type_ids;
    std::vector<extract::SymbolID> func_ids;
    for(auto& sym_id : ns.symbols) {
        if(auto* sym = lookup(model, sym_id)) {
            switch(sym->kind) {
                case extract::SymbolKind::Class:
                case extract::SymbolKind::Struct:
                case extract::SymbolKind::Enum:
                case extract::SymbolKind::Union:
                case extract::SymbolKind::Concept:
                case extract::SymbolKind::Template:
                case extract::SymbolKind::TypeAlias:
                    type_ids.push_back(sym_id);
                    break;
                case extract::SymbolKind::Function:
                    func_ids.push_back(sym_id);
                    break;
                default:
                    break;
            }
        }
    }

    auto type_facts = collect_facts(model, type_ids, rules.max_siblings, root);
    auto func_facts = collect_facts(model, func_ids, rules.max_siblings, root);
    for(auto& fact : type_facts) {
        pack.local_context.push_back(std::move(fact));
    }
    for(auto& fact : func_facts) {
        pack.local_context.push_back(std::move(fact));
    }

    // Related page summaries from type pages
    std::vector<std::string> summary_keys;
    for(auto& fact : pack.local_context) {
        summary_keys.push_back(fact.qualified_name);
    }
    pack.related_page_summaries = collect_summaries(summaries, summary_keys, rules.max_related_summaries);

    return pack;
}

auto build_evidence_for_module_summary(
    const extract::ModuleUnit& mod,
    const extract::ProjectModel& model,
    const config::EvidenceRulesConfig& rules,
    const PageSummaryCache& summaries,
    std::string_view project_root) -> EvidencePack {

    auto root = std::string(project_root);
    EvidencePack pack;
    pack.slot_kind = "module_summary";
    pack.subject_name = mod.name;
    pack.subject_kind = "module";

    pack.target_facts = collect_facts(model, mod.symbols, rules.max_siblings, root);

    // Imports as dependency context
    for(auto& import_name : mod.imports) {
        SymbolFact import_fact;
        import_fact.qualified_name = import_name;
        import_fact.kind_label = "module_import";
        pack.dependency_context.push_back(std::move(import_fact));
    }

    // Related page summaries from imported modules
    pack.related_page_summaries = collect_summaries(summaries, mod.imports, rules.max_related_summaries);

    return pack;
}

auto build_evidence_for_module_architecture(
    const extract::ModuleUnit& mod,
    const extract::ProjectModel& model,
    const config::EvidenceRulesConfig& rules,
    const PageSummaryCache& summaries,
    std::string_view project_root) -> EvidencePack {

    auto root = std::string(project_root);
    EvidencePack pack;
    pack.slot_kind = "module_architecture";
    pack.subject_name = mod.name;
    pack.subject_kind = "module";

    // Exported symbols
    pack.target_facts = collect_facts(model, mod.symbols, rules.max_siblings, root);

    // Imports
    for(auto& import_name : mod.imports) {
        SymbolFact import_fact;
        import_fact.qualified_name = import_name;
        import_fact.kind_label = "module_import";
        pack.dependency_context.push_back(std::move(import_fact));
    }

    // Summaries from child type pages + imported module pages
    std::vector<std::string> summary_keys;
    for(auto& fact : pack.target_facts) {
        summary_keys.push_back(fact.qualified_name);
    }
    for(auto& import_name : mod.imports) {
        summary_keys.push_back(import_name);
    }
    pack.related_page_summaries = collect_summaries(summaries, summary_keys, rules.max_related_summaries);

    return pack;
}

auto build_evidence_for_index_overview(
    const extract::ProjectModel& model,
    const config::EvidenceRulesConfig& rules,
    const PageSummaryCache& summaries) -> EvidencePack {

    EvidencePack pack;
    pack.slot_kind = "index_overview";
    pack.subject_name = "index";
    pack.subject_kind = "index";

    // Top-level modules
    for(auto& [source_file, mod_unit] : model.modules) {
        if(mod_unit.is_interface && mod_unit.name.find(':') == std::string::npos) {
            SymbolFact fact;
            fact.qualified_name = mod_unit.name;
            fact.kind_label = "module";
            pack.target_facts.push_back(std::move(fact));
        }
    }

    // Top namespaces
    for(auto& [ns_name, ns_info] : model.namespaces) {
        if(ns_name.find("::") == std::string::npos) {
            SymbolFact fact;
            fact.qualified_name = ns_name;
            fact.kind_label = "namespace";
            pack.local_context.push_back(std::move(fact));
        }
    }

    // Summaries from module pages
    std::vector<std::string> summary_keys;
    for(auto& fact : pack.target_facts) {
        summary_keys.push_back(fact.qualified_name);
    }
    pack.related_page_summaries = collect_summaries(summaries, summary_keys, rules.max_related_summaries);

    return pack;
}

auto build_evidence_for_workflow(
    const std::vector<std::string>& chain_keys,
    const extract::ProjectModel& model,
    const config::EvidenceRulesConfig& rules,
    const PageSummaryCache& summaries,
    std::string_view project_root) -> EvidencePack {

    auto root = std::string(project_root);
    EvidencePack pack;
    pack.slot_kind = "workflow";
    if(!chain_keys.empty()) {
        pack.subject_name = chain_keys[0];
    }
    pack.subject_kind = "workflow";

    std::unordered_map<std::string, const extract::SymbolInfo*> symbol_by_qname;
    symbol_by_qname.reserve(model.symbols.size());
    for(auto& [id, sym] : model.symbols) {
        if(!sym.qualified_name.empty()) {
            symbol_by_qname.emplace(sym.qualified_name, &sym);
        }
    }

    std::unordered_set<extract::SymbolID> chain_symbol_ids;
    std::vector<const extract::SymbolInfo*> chain_symbols;
    chain_symbols.reserve(chain_keys.size());

    // Target facts: symbols in the selected calling chain
    for(auto& qname : chain_keys) {
        auto it = symbol_by_qname.find(qname);
        if(it == symbol_by_qname.end()) {
            continue;
        }

        auto* sym = it->second;
        if(!chain_symbol_ids.insert(sym->id).second) {
            continue;
        }

        chain_symbols.push_back(sym);
        pack.target_facts.push_back(to_symbol_fact(*sym, root));
        if(!sym->source_snippet.empty()) {
            pack.source_snippets.push_back(
                truncate_snippet(sym->source_snippet, rules.max_source_bytes));
        }
    }

    // Local context: explicit chain call edges (structured semantics for LLM)
    for(std::size_t i = 0; i + 1 < chain_keys.size(); ++i) {
        SymbolFact edge_fact;
        edge_fact.qualified_name = chain_keys[i] + " -> " + chain_keys[i + 1];
        edge_fact.kind_label = "call_edge";
        pack.local_context.push_back(std::move(edge_fact));
    }

    // Boundary contexts: direct callees/callers outside the selected chain
    std::unordered_set<extract::SymbolID> dep_seen;
    std::unordered_set<extract::SymbolID> rev_seen;
    for(auto* sym : chain_symbols) {
        for(auto& callee_id : sym->calls) {
            if(pack.dependency_context.size() >= rules.max_callees) break;
            if(chain_symbol_ids.contains(callee_id) || !dep_seen.insert(callee_id).second) {
                continue;
            }
            if(auto* callee = lookup(model, callee_id)) {
                pack.dependency_context.push_back(to_symbol_fact(*callee, root));
            }
        }

        for(auto& caller_id : sym->called_by) {
            if(pack.reverse_usage_context.size() >= rules.max_callers) break;
            if(chain_symbol_ids.contains(caller_id) || !rev_seen.insert(caller_id).second) {
                continue;
            }
            if(auto* caller = lookup(model, caller_id)) {
                pack.reverse_usage_context.push_back(to_symbol_fact(*caller, root));
            }
        }
    }

    // Related page summaries from chain participants and nearby boundary symbols
    std::vector<std::string> summary_keys;
    for(auto& fact : pack.target_facts) {
        summary_keys.push_back(fact.qualified_name);
    }
    for(auto& fact : pack.dependency_context) {
        summary_keys.push_back(fact.qualified_name);
    }
    for(auto& fact : pack.reverse_usage_context) {
        summary_keys.push_back(fact.qualified_name);
    }
    pack.related_page_summaries = collect_summaries(summaries, summary_keys, rules.max_related_summaries);

    return pack;
}

auto format_evidence_text(const EvidencePack& pack) -> std::string {
    return format_evidence_text_bounded(pack, std::numeric_limits<std::size_t>::max());
}

auto format_evidence_text_bounded(const EvidencePack& pack, std::size_t max_length)
    -> std::string {
    std::string text;
    text.reserve(4096);

    if(!append_if_fits(text, "## EVIDENCE\n\n", max_length)) {
        return {};
    }

    append_section_bounded(text, "### Target\n", pack.target_facts,
                           render_detailed_fact, max_length);
    append_section_bounded(text, "### Local Context\n", pack.local_context,
                           render_context_fact, max_length);
    append_section_bounded(text, "### Dependencies\n", pack.dependency_context,
                           render_context_fact, max_length);
    append_section_bounded(text, "### Used By\n", pack.reverse_usage_context,
                           render_context_fact, max_length);
    append_section_bounded(text, "### Source Snippets\n", pack.source_snippets,
                           render_source_snippet, max_length);
    append_section_bounded(text, "### Related Page Summaries\n",
                           pack.related_page_summaries,
                           render_summary_item, max_length);

    return text;
}

}  // namespace clore::generate
