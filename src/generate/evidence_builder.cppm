module generate:evidence_builder;

import std;
import :evidence;
import :model;
import extract;

namespace clore::generate {

namespace {

// Resolve the source snippet on demand.  The const_cast is safe here because
// source_snippet is a lazily-populated cache field; mutating it does not change
// the observable semantics of the SymbolInfo.
auto maybe_resolve_snippet(const extract::SymbolInfo& sym) -> bool {
    if(!sym.source_snippet.empty()) {
        return true;
    }
    static std::mutex resolve_mutex;
    std::lock_guard lock(resolve_mutex);
    if(!sym.source_snippet.empty()) {
        return true;
    }
    return extract::resolve_source_snippet(const_cast<extract::SymbolInfo&>(sym));
}

}  // namespace

auto build_evidence_for_namespace_summary(const extract::NamespaceInfo& ns,
                                          const extract::ProjectModel& model,
                                          const SymbolAnalysisStore& analyses,
                                          std::string_view project_root) -> EvidencePack {
    auto root = std::string(project_root);

    EvidencePack pack;
    pack.prompt_kind = "namespace_summary";
    pack.subject_name = ns.name;
    pack.subject_kind = "namespace";

    SymbolFact namespace_fact;
    namespace_fact.qualified_name = ns.name;
    namespace_fact.kind_label = "namespace";
    pack.target_facts.push_back(std::move(namespace_fact));

    std::vector<extract::SymbolID> symbol_ids;
    symbol_ids.reserve(ns.symbols.size());
    for(auto sym_id: ns.symbols) {
        if(auto* sym = extract::lookup_symbol(model, sym_id);
           sym != nullptr && (is_type_kind(sym->kind) || is_function_kind(sym->kind) ||
                              is_variable_kind(sym->kind))) {
            symbol_ids.push_back(sym_id);
        }
    }

    pack.local_context = __detail::collect_facts(model, symbol_ids, root);
    pack.related_page_summaries = __detail::collect_analysis_summaries(analyses, model, symbol_ids);

    return pack;
}

auto build_evidence_for_function_analysis(const extract::SymbolInfo& target,
                                          const extract::ProjectModel& model,
                                          std::string_view project_root) -> EvidencePack {
    auto root = std::string(project_root);
    EvidencePack pack;
    pack.prompt_kind = "function_analysis";
    pack.subject_name = target.qualified_name;
    pack.subject_kind = std::string(extract::symbol_kind_name(target.kind));
    pack.target_facts.push_back(__detail::to_symbol_fact(target, root));

    if(maybe_resolve_snippet(target)) {
        pack.source_snippets.push_back(target.source_snippet);
    }

    pack.reverse_usage_context =
        __detail::collect_merged_facts(model, root, target.called_by, target.referenced_by);
    pack.dependency_context = __detail::collect_facts(model, target.calls, root);

    if(!target.enclosing_namespace.empty()) {
        pack.local_context = __detail::collect_namespace_facts_cached(model,
                                                                      target.enclosing_namespace,
                                                                      root,
                                                                      target.kind,
                                                                      target.id);
    }

    return pack;
}

auto build_evidence_for_type_analysis(const extract::SymbolInfo& target,
                                      const extract::ProjectModel& model,
                                      std::string_view project_root) -> EvidencePack {
    auto root = std::string(project_root);
    EvidencePack pack;
    pack.prompt_kind = "type_analysis";
    pack.subject_name = target.qualified_name;
    pack.subject_kind = std::string(extract::symbol_kind_name(target.kind));
    pack.target_facts.push_back(__detail::to_symbol_fact(target, root));

    if(maybe_resolve_snippet(target)) {
        pack.source_snippets.push_back(target.source_snippet);
    }

    pack.reverse_usage_context = __detail::collect_merged_facts(model,
                                                                root,
                                                                target.called_by,
                                                                target.referenced_by,
                                                                target.derived);
    pack.dependency_context = __detail::collect_facts(model, target.bases, root);

    std::vector<extract::SymbolID> member_ids;
    member_ids.reserve(target.children.size());
    for(auto child_id: target.children) {
        member_ids.push_back(child_id);
    }
    pack.local_context = __detail::collect_facts(model, member_ids, root);

    return pack;
}

auto build_evidence_for_variable_analysis(const extract::SymbolInfo& target,
                                          const extract::ProjectModel& model,
                                          std::string_view project_root) -> EvidencePack {
    auto root = std::string(project_root);
    EvidencePack pack;
    pack.prompt_kind = "variable_analysis";
    pack.subject_name = target.qualified_name;
    pack.subject_kind = std::string(extract::symbol_kind_name(target.kind));
    pack.target_facts.push_back(__detail::to_symbol_fact(target, root));

    if(maybe_resolve_snippet(target)) {
        pack.source_snippets.push_back(target.source_snippet);
    }

    pack.reverse_usage_context =
        __detail::collect_merged_facts(model, root, target.referenced_by, target.called_by);
    pack.dependency_context = __detail::collect_facts(model, target.references, root);

    if(!target.enclosing_namespace.empty()) {
        pack.local_context = __detail::collect_namespace_facts_cached(model,
                                                                      target.enclosing_namespace,
                                                                      root,
                                                                      target.kind,
                                                                      target.id);
    }

    return pack;
}

auto build_evidence_for_module_summary(const extract::ModuleUnit& mod,
                                       const extract::ProjectModel& model,
                                       const SymbolAnalysisStore& analyses,
                                       const PageSummaryCache& page_summaries,
                                       std::string_view project_root) -> EvidencePack {
    auto root = std::string(project_root);
    EvidencePack pack;
    pack.prompt_kind = "module_summary";
    pack.subject_name = mod.name;
    pack.subject_kind = "module";

    pack.target_facts = __detail::collect_facts(model, mod.symbols, root);

    for(auto& import_name: mod.imports) {
        SymbolFact import_fact;
        import_fact.qualified_name = import_name;
        import_fact.kind_label = "module_import";
        pack.dependency_context.push_back(std::move(import_fact));
    }

    pack.related_page_summaries =
        __detail::collect_analysis_summaries(analyses, model, mod.symbols);

    auto imported_summaries = __detail::collect_summaries(page_summaries, mod.imports);
    for(auto& summary: imported_summaries) {
        pack.related_page_summaries.push_back(std::move(summary));
    }

    return pack;
}

auto build_evidence_for_module_architecture(const extract::ModuleUnit& mod,
                                            const extract::ProjectModel& model,
                                            const SymbolAnalysisStore& analyses,
                                            const PageSummaryCache& page_summaries,
                                            std::string_view project_root) -> EvidencePack {
    auto root = std::string(project_root);
    EvidencePack pack;
    pack.prompt_kind = "module_architecture";
    pack.subject_name = mod.name;
    pack.subject_kind = "module";

    pack.target_facts = __detail::collect_facts(model, mod.symbols, root);

    for(auto& import_name: mod.imports) {
        SymbolFact import_fact;
        import_fact.qualified_name = import_name;
        import_fact.kind_label = "module_import";
        pack.dependency_context.push_back(std::move(import_fact));
    }

    pack.related_page_summaries =
        __detail::collect_analysis_summaries(analyses, model, mod.symbols);

    auto imported_summaries = __detail::collect_summaries(page_summaries, mod.imports);
    for(auto& summary: imported_summaries) {
        pack.related_page_summaries.push_back(std::move(summary));
    }

    return pack;
}

auto build_evidence_for_index_overview(const extract::ProjectModel& model,
                                       const PageSummaryCache& page_summaries) -> EvidencePack {
    EvidencePack pack;
    pack.prompt_kind = "index_overview";
    pack.subject_name = "index";
    pack.subject_kind = "index";

    for(auto& [source_file, mod_unit]: model.modules) {
        if(mod_unit.is_interface && mod_unit.name.find(':') == std::string::npos) {
            SymbolFact fact;
            fact.qualified_name = mod_unit.name;
            fact.kind_label = "module";
            pack.target_facts.push_back(std::move(fact));
        }
    }

    for(auto& [ns_name, ns_info]: model.namespaces) {
        if(ns_name.find("::") == std::string::npos) {
            SymbolFact fact;
            fact.qualified_name = ns_name;
            fact.kind_label = "namespace";
            pack.local_context.push_back(std::move(fact));
        }
    }

    std::vector<std::string> summary_keys;
    for(auto& fact: pack.target_facts) {
        summary_keys.push_back(fact.qualified_name);
    }
    pack.related_page_summaries = __detail::collect_summaries(page_summaries, summary_keys);

    return pack;
}

auto build_evidence_for_function_declaration_summary(const extract::SymbolInfo& target,
                                                     const extract::ProjectModel& model,
                                                     const SymbolAnalysisStore& analyses,
                                                     std::string_view project_root)
    -> EvidencePack {
    auto root = std::string(project_root);
    EvidencePack pack;
    pack.prompt_kind = "function_declaration_summary";
    pack.subject_name = target.qualified_name;
    pack.subject_kind = std::string(extract::symbol_kind_name(target.kind));
    pack.target_facts.push_back(__detail::to_symbol_fact(target, root));

    pack.reverse_usage_context =
        __detail::collect_merged_facts(model, root, target.called_by, target.referenced_by);
    pack.dependency_context = __detail::collect_facts(model, target.calls, root);

    if(!target.enclosing_namespace.empty()) {
        pack.local_context = __detail::collect_namespace_facts_cached(model,
                                                                      target.enclosing_namespace,
                                                                      root,
                                                                      target.kind,
                                                                      target.id);
    }

    pack.related_page_summaries =
        __detail::collect_analysis_summaries(analyses, model, pack.reverse_usage_context);

    return pack;
}

auto build_evidence_for_function_implementation_summary(const extract::SymbolInfo& target,
                                                        const extract::ProjectModel& model,
                                                        std::string_view project_root)
    -> EvidencePack {
    auto root = std::string(project_root);
    EvidencePack pack;
    pack.prompt_kind = "function_implementation_summary";
    pack.subject_name = target.qualified_name;
    pack.subject_kind = std::string(extract::symbol_kind_name(target.kind));
    pack.target_facts.push_back(__detail::to_symbol_fact(target, root));

    if(maybe_resolve_snippet(target)) {
        pack.source_snippets.push_back(target.source_snippet);
    }

    pack.dependency_context = __detail::collect_facts(model, target.calls, root);

    if(target.definition_location.has_value() && !target.definition_location->file.empty()) {
        auto file_it = model.files.find(target.definition_location->file);
        if(file_it != model.files.end()) {
            std::vector<extract::SymbolID> local_ids;
            for(auto& sym_id: file_it->second.symbols) {
                if(sym_id == target.id) {
                    continue;
                }
                local_ids.push_back(sym_id);
            }
            pack.local_context = __detail::collect_facts(model, local_ids, root);
        }
    }

    return pack;
}

auto build_evidence_for_type_declaration_summary(const extract::SymbolInfo& target,
                                                 const extract::ProjectModel& model,
                                                 const SymbolAnalysisStore& analyses,
                                                 std::string_view project_root) -> EvidencePack {
    auto root = std::string(project_root);
    EvidencePack pack;
    pack.prompt_kind = "type_declaration_summary";
    pack.subject_name = target.qualified_name;
    pack.subject_kind = std::string(extract::symbol_kind_name(target.kind));
    pack.target_facts.push_back(__detail::to_symbol_fact(target, root));

    pack.reverse_usage_context = __detail::collect_merged_facts(model,
                                                                root,
                                                                target.called_by,
                                                                target.referenced_by,
                                                                target.derived);
    pack.dependency_context = __detail::collect_facts(model, target.bases, root);

    if(!target.enclosing_namespace.empty()) {
        pack.local_context = __detail::collect_namespace_facts_cached(model,
                                                                      target.enclosing_namespace,
                                                                      root,
                                                                      target.kind,
                                                                      target.id);
    }

    pack.related_page_summaries =
        __detail::collect_analysis_summaries(analyses, model, pack.dependency_context);

    return pack;
}

auto build_evidence_for_type_implementation_summary(const extract::SymbolInfo& target,
                                                    const extract::ProjectModel& model,
                                                    std::string_view project_root) -> EvidencePack {
    auto root = std::string(project_root);
    EvidencePack pack;
    pack.prompt_kind = "type_implementation_summary";
    pack.subject_name = target.qualified_name;
    pack.subject_kind = std::string(extract::symbol_kind_name(target.kind));
    pack.target_facts.push_back(__detail::to_symbol_fact(target, root));

    if(maybe_resolve_snippet(target)) {
        pack.source_snippets.push_back(target.source_snippet);
    }

    std::vector<extract::SymbolID> member_ids;
    for(auto& child_id: target.children) {
        member_ids.push_back(child_id);
    }
    pack.local_context = __detail::collect_facts(model, member_ids, root);

    std::vector<extract::SymbolID> dependency_ids = target.bases;
    for(auto& child_id: target.children) {
        if(auto* child = extract::lookup_symbol(model, child_id);
           child != nullptr && is_function_kind(child->kind)) {
            dependency_ids.insert(dependency_ids.end(), child->calls.begin(), child->calls.end());
        }
    }
    pack.dependency_context = __detail::collect_facts(model, dependency_ids, root);

    return pack;
}

}  // namespace clore::generate
