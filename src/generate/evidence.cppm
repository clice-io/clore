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

auto build_evidence_for_namespace_summary(
    const extract::NamespaceInfo &ns, const extract::ProjectModel &model,
    const config::EvidenceRulesConfig &rules, const PageSummaryCache &summaries,
    std::string_view project_root) -> EvidencePack;

auto build_evidence_for_module_summary(const extract::ModuleUnit &mod,
                                       const extract::ProjectModel &model,
                                       const config::EvidenceRulesConfig &rules,
                                       const PageSummaryCache &summaries,
                                       std::string_view project_root)
    -> EvidencePack;

auto build_evidence_for_module_architecture(
    const extract::ModuleUnit &mod, const extract::ProjectModel &model,
    const config::EvidenceRulesConfig &rules, const PageSummaryCache &summaries,
    std::string_view project_root) -> EvidencePack;

auto build_evidence_for_index_overview(const extract::ProjectModel &model,
                                       const config::EvidenceRulesConfig &rules,
                                       const PageSummaryCache &summaries)
    -> EvidencePack;

auto build_evidence_for_function_declaration_summary(
    const extract::SymbolInfo &target, const extract::ProjectModel &model,
    const config::EvidenceRulesConfig &rules, const PageSummaryCache &summaries,
    std::string_view project_root) -> EvidencePack;

auto build_evidence_for_function_implementation_summary(
    const extract::SymbolInfo &target, const extract::ProjectModel &model,
    const config::EvidenceRulesConfig &rules, std::string_view project_root)
    -> EvidencePack;

auto build_evidence_for_type_declaration_summary(
    const extract::SymbolInfo &target, const extract::ProjectModel &model,
    const config::EvidenceRulesConfig &rules, const PageSummaryCache &summaries,
    std::string_view project_root) -> EvidencePack;

auto build_evidence_for_type_implementation_summary(
    const extract::SymbolInfo &target, const extract::ProjectModel &model,
    const config::EvidenceRulesConfig &rules, std::string_view project_root)
    -> EvidencePack;

auto format_evidence_text(const EvidencePack &pack) -> std::string;

auto format_evidence_text_bounded(const EvidencePack &pack,
                                  std::size_t max_length) -> std::string;

} // namespace clore::generate

// ── implementation ──────────────────────────────────────────────────

namespace clore::generate {

namespace {

auto to_symbol_fact(const extract::SymbolInfo &sym,
                    const std::string &project_root) -> SymbolFact {
  return SymbolFact{
      .id = sym.id,
      .qualified_name = sym.qualified_name,
      .signature = sym.signature,
      .kind_label = std::string(extract::symbol_kind_name(sym.kind)),
      .access = sym.access,
      .is_template = sym.is_template,
      .template_params = sym.template_params,
      .declaration_file = clore::generate::make_source_relative(
          sym.declaration_location.file, project_root),
      .declaration_line = sym.declaration_location.line,
      .doc_comment = sym.doc_comment,
  };
}

auto lookup(const extract::ProjectModel &model, extract::SymbolID id)
    -> const extract::SymbolInfo * {
  return clore::generate::lookup_sym(model, id);
}

auto collect_facts(const extract::ProjectModel &model,
                   const std::vector<extract::SymbolID> &ids,
                   std::uint32_t max_count, const std::string &project_root)
    -> std::vector<SymbolFact> {
  std::vector<SymbolFact> facts;
  std::unordered_set<extract::SymbolID> seen;
  for (auto &id : ids) {
    if (facts.size() >= max_count)
      break;
    if (!seen.insert(id).second)
      continue;
    if (auto *sym = lookup(model, id)) {
      facts.push_back(to_symbol_fact(*sym, project_root));
    }
  }
  return facts;
}

template <typename... Groups>
auto collect_merged_facts(const extract::ProjectModel &model,
                          std::uint32_t max_count,
                          const std::string &project_root,
                          const Groups &...groups) -> std::vector<SymbolFact> {
  std::vector<SymbolFact> facts;
  std::unordered_set<extract::SymbolID> seen;

  auto append_group = [&](const auto &ids) {
    for (const auto &id : ids) {
      if (facts.size() >= max_count) {
        return;
      }
      if (!seen.insert(id).second) {
        continue;
      }
      if (auto *sym = lookup(model, id)) {
        facts.push_back(to_symbol_fact(*sym, project_root));
      }
    }
  };

  (append_group(groups), ...);
  return facts;
}

auto collect_summaries(const PageSummaryCache &cache,
                       const std::vector<std::string> &keys,
                       std::uint32_t max_count) -> std::vector<std::string> {
  std::vector<std::string> result;
  for (auto &key : keys) {
    if (result.size() >= max_count)
      break;
    auto it = cache.find(key);
    if (it != cache.end() && !it->second.empty()) {
      result.push_back(it->second);
    }
  }
  return result;
}

auto summary_cache_key_for_fact(const SymbolFact &fact) -> std::string {
  if (fact.signature.empty()) {
    return fact.qualified_name;
  }

  std::string key;
  key.reserve(fact.qualified_name.size() + fact.signature.size() + 1);
  key += fact.qualified_name;
  key += '\t';
  key += fact.signature;
  return key;
}

auto truncate_snippet(const std::string &snippet, std::uint32_t max_bytes)
    -> std::string {
  auto normalized = clore::support::ensure_utf8(snippet);
  constexpr std::string_view suffix = "\n// ... truncated";
  if (normalized.size() <= max_bytes) {
    return normalized;
  }
  if (max_bytes <= suffix.size()) {
    return clore::support::truncate_utf8(std::string(suffix), max_bytes);
  }
  auto truncated_max =
      (max_bytes > suffix.size()) ? (max_bytes - suffix.size()) : 0;
  return clore::support::truncate_utf8(normalized, truncated_max) +
         std::string(suffix);
}

auto render_detailed_fact(const SymbolFact &fact) -> std::string {
  std::string text;
  text += "- " + fact.kind_label + " `" + fact.qualified_name + "`";
  if (!fact.signature.empty()) {
    text += " — `" + fact.signature + "`";
  }
  text += "\n";
  if (!fact.access.empty()) {
    text += "  Access: " + fact.access + "\n";
  }
  if (fact.is_template && !fact.template_params.empty()) {
    text += "  Template: `" + fact.template_params + "`\n";
  }
  if (!fact.declaration_file.empty() && fact.declaration_line > 0) {
    text += "  Declared at: " + fact.declaration_file + ":" +
            std::to_string(fact.declaration_line) + "\n";
  }
  if (!fact.doc_comment.empty()) {
    text += "  Doc: " + fact.doc_comment + "\n";
  }
  return text;
}

auto render_context_fact(const SymbolFact &fact) -> std::string {
  std::string text;
  text += "- " + fact.kind_label + " `" + fact.qualified_name + "`";
  if (!fact.signature.empty()) {
    text += ": `" + fact.signature + "`";
  }
  text += "\n";
  return text;
}

auto render_source_snippet(const std::string &snippet) -> std::string {
  return "```cpp\n" + snippet + "\n```\n";
}

auto render_summary_item(const std::string &summary) -> std::string {
  return "- " + summary + "\n";
}

auto append_if_fits(std::string &text, std::string_view chunk,
                    std::size_t max_length) -> bool {
  if (text.size() + chunk.size() > max_length) {
    return false;
  }
  text.append(chunk);
  return true;
}

template <typename Items, typename RenderItem>
auto append_section_bounded(std::string &text, std::string_view title,
                            const Items &items, RenderItem &&render_item,
                            std::size_t max_length) -> void {
  if (items.empty()) {
    return;
  }

  const auto checkpoint = text.size();
  if (!append_if_fits(text, title, max_length)) {
    return;
  }

  bool appended_any = false;
  for (const auto &item : items) {
    auto rendered = render_item(item);
    if (!append_if_fits(text, rendered, max_length)) {
      break;
    }
    appended_any = true;
  }

  if (!appended_any) {
    text.resize(checkpoint);
    return;
  }

  (void)append_if_fits(text, "\n", max_length);
}

} // namespace

auto build_evidence_for_namespace_summary(
    const extract::NamespaceInfo &ns, const extract::ProjectModel &model,
    const config::EvidenceRulesConfig &rules, const PageSummaryCache &summaries,
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

  // Types in namespace
  std::vector<extract::SymbolID> type_ids;
  std::vector<extract::SymbolID> func_ids;
  for (auto &sym_id : ns.symbols) {
    if (auto *sym = lookup(model, sym_id)) {
      switch (sym->kind) {
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
  for (auto &fact : type_facts) {
    pack.local_context.push_back(std::move(fact));
  }
  for (auto &fact : func_facts) {
    pack.local_context.push_back(std::move(fact));
  }

  // Related page summaries from type pages
  std::vector<std::string> summary_keys;
  for (auto &fact : pack.local_context) {
    summary_keys.push_back(summary_cache_key_for_fact(fact));
  }
  pack.related_page_summaries =
      collect_summaries(summaries, summary_keys, rules.max_related_summaries);

  return pack;
}

auto build_evidence_for_module_summary(const extract::ModuleUnit &mod,
                                       const extract::ProjectModel &model,
                                       const config::EvidenceRulesConfig &rules,
                                       const PageSummaryCache &summaries,
                                       std::string_view project_root)
    -> EvidencePack {

  auto root = std::string(project_root);
  EvidencePack pack;
  pack.prompt_kind = "module_summary";
  pack.subject_name = mod.name;
  pack.subject_kind = "module";

  pack.target_facts =
      collect_facts(model, mod.symbols, rules.max_siblings, root);

  // Imports as dependency context
  for (auto &import_name : mod.imports) {
    SymbolFact import_fact;
    import_fact.qualified_name = import_name;
    import_fact.kind_label = "module_import";
    pack.dependency_context.push_back(std::move(import_fact));
  }

  // Related page summaries from imported modules
  pack.related_page_summaries =
      collect_summaries(summaries, mod.imports, rules.max_related_summaries);

  return pack;
}

auto build_evidence_for_module_architecture(
    const extract::ModuleUnit &mod, const extract::ProjectModel &model,
    const config::EvidenceRulesConfig &rules, const PageSummaryCache &summaries,
    std::string_view project_root) -> EvidencePack {

  auto root = std::string(project_root);
  EvidencePack pack;
  pack.prompt_kind = "module_architecture";
  pack.subject_name = mod.name;
  pack.subject_kind = "module";

  // Exported symbols
  pack.target_facts =
      collect_facts(model, mod.symbols, rules.max_siblings, root);

  // Imports
  for (auto &import_name : mod.imports) {
    SymbolFact import_fact;
    import_fact.qualified_name = import_name;
    import_fact.kind_label = "module_import";
    pack.dependency_context.push_back(std::move(import_fact));
  }

  // Summaries from child type pages + imported module pages
  std::vector<std::string> summary_keys;
  for (auto &fact : pack.target_facts) {
    summary_keys.push_back(summary_cache_key_for_fact(fact));
  }
  for (auto &import_name : mod.imports) {
    summary_keys.push_back(import_name);
  }
  pack.related_page_summaries =
      collect_summaries(summaries, summary_keys, rules.max_related_summaries);

  return pack;
}

auto build_evidence_for_index_overview(const extract::ProjectModel &model,
                                       const config::EvidenceRulesConfig &rules,
                                       const PageSummaryCache &summaries)
    -> EvidencePack {

  EvidencePack pack;
  pack.prompt_kind = "index_overview";
  pack.subject_name = "index";
  pack.subject_kind = "index";

  // Top-level modules (respect configured limit)
  for (auto &[source_file, mod_unit] : model.modules) {
    if (pack.target_facts.size() >= rules.max_top_modules) {
      break;
    }
    if (mod_unit.is_interface && mod_unit.name.find(':') == std::string::npos) {
      SymbolFact fact;
      fact.qualified_name = mod_unit.name;
      fact.kind_label = "module";
      pack.target_facts.push_back(std::move(fact));
    }
  }

  // Top namespaces (respect configured limit)
  for (auto &[ns_name, ns_info] : model.namespaces) {
    if (pack.local_context.size() >= rules.max_top_namespaces) {
      break;
    }
    if (ns_name.find("::") == std::string::npos) {
      SymbolFact fact;
      fact.qualified_name = ns_name;
      fact.kind_label = "namespace";
      pack.local_context.push_back(std::move(fact));
    }
  }

  // Summaries from module pages
  std::vector<std::string> summary_keys;
  for (auto &fact : pack.target_facts) {
    summary_keys.push_back(summary_cache_key_for_fact(fact));
  }
  pack.related_page_summaries =
      collect_summaries(summaries, summary_keys, rules.max_related_summaries);

  return pack;
}

auto build_evidence_for_function_declaration_summary(
    const extract::SymbolInfo &target, const extract::ProjectModel &model,
    const config::EvidenceRulesConfig &rules, const PageSummaryCache &summaries,
    std::string_view project_root) -> EvidencePack {

  auto root = std::string(project_root);
  EvidencePack pack;
  pack.prompt_kind = "function_declaration_summary";
  pack.subject_name = target.qualified_name;
  pack.subject_kind = std::string(extract::symbol_kind_name(target.kind));
  pack.target_facts.push_back(to_symbol_fact(target, root));

  // Callers — how this function is used
  pack.reverse_usage_context = collect_merged_facts(
      model, rules.max_callers, root, target.called_by, target.referenced_by);

  // Callees — what it delegates to (helps understand contract)
  pack.dependency_context =
      collect_facts(model, target.calls, rules.max_callees, root);

  // Sibling functions in the same namespace as local context
  if (!target.enclosing_namespace.empty()) {
    auto ns_it = model.namespaces.find(target.enclosing_namespace);
    if (ns_it != model.namespaces.end()) {
      std::vector<extract::SymbolID> sibling_ids;
      for (auto &sym_id : ns_it->second.symbols) {
        if (sym_id == target.id)
          continue;
        if (auto *sym = lookup(model, sym_id)) {
          if (is_function_kind(sym->kind)) {
            sibling_ids.push_back(sym_id);
          }
        }
      }
      pack.local_context =
          collect_facts(model, sibling_ids, rules.max_siblings, root);
    }
  }

  // Related summaries from callers
  std::vector<std::string> summary_keys;
  for (auto &f : pack.reverse_usage_context) {
    summary_keys.push_back(summary_cache_key_for_fact(f));
  }
  pack.related_page_summaries =
      collect_summaries(summaries, summary_keys, rules.max_related_summaries);

  return pack;
}

auto build_evidence_for_function_implementation_summary(
    const extract::SymbolInfo &target, const extract::ProjectModel &model,
    const config::EvidenceRulesConfig &rules, std::string_view project_root)
    -> EvidencePack {

  auto root = std::string(project_root);
  EvidencePack pack;
  pack.prompt_kind = "function_implementation_summary";
  pack.subject_name = target.qualified_name;
  pack.subject_kind = std::string(extract::symbol_kind_name(target.kind));
  pack.target_facts.push_back(to_symbol_fact(target, root));

  // Source snippet — the primary evidence for implementation description
  if (!target.source_snippet.empty()) {
    pack.source_snippets.push_back(
        truncate_snippet(target.source_snippet, rules.max_source_bytes));
  }

  // Callees — what this function delegates to internally
  pack.dependency_context =
      collect_facts(model, target.calls, rules.max_callees, root);

  // Local context: other symbols defined in the same file
  if (target.definition_location.has_value() &&
      !target.definition_location->file.empty()) {
    auto file_it = model.files.find(target.definition_location->file);
    if (file_it != model.files.end()) {
      std::vector<extract::SymbolID> local_ids;
      for (auto &sym_id : file_it->second.symbols) {
        if (sym_id == target.id)
          continue;
        local_ids.push_back(sym_id);
      }
      pack.local_context =
          collect_facts(model, local_ids, rules.max_siblings, root);
    }
  }

  return pack;
}

auto build_evidence_for_type_declaration_summary(
    const extract::SymbolInfo &target, const extract::ProjectModel &model,
    const config::EvidenceRulesConfig &rules, const PageSummaryCache &summaries,
    std::string_view project_root) -> EvidencePack {

  auto root = std::string(project_root);
  EvidencePack pack;
  pack.prompt_kind = "type_declaration_summary";
  pack.subject_name = target.qualified_name;
  pack.subject_kind = std::string(extract::symbol_kind_name(target.kind));
  pack.target_facts.push_back(to_symbol_fact(target, root));

  // Callers / references — how this type is used
  pack.reverse_usage_context =
      collect_merged_facts(model, rules.max_callers, root, target.called_by,
                           target.referenced_by, target.derived);

  // Base types and derived types
  pack.dependency_context =
      collect_facts(model, target.bases, rules.max_callees, root);

  // Sibling types as local context
  if (!target.enclosing_namespace.empty()) {
    auto ns_it = model.namespaces.find(target.enclosing_namespace);
    if (ns_it != model.namespaces.end()) {
      std::vector<extract::SymbolID> sibling_ids;
      for (auto &sym_id : ns_it->second.symbols) {
        if (sym_id == target.id)
          continue;
        if (auto *sym = lookup(model, sym_id)) {
          if (is_type_kind(sym->kind)) {
            sibling_ids.push_back(sym_id);
          }
        }
      }
      pack.local_context =
          collect_facts(model, sibling_ids, rules.max_siblings, root);
    }
  }

  // Related summaries
  std::vector<std::string> summary_keys;
  for (auto &f : pack.dependency_context) {
    summary_keys.push_back(summary_cache_key_for_fact(f));
  }
  pack.related_page_summaries =
      collect_summaries(summaries, summary_keys, rules.max_related_summaries);

  return pack;
}

auto build_evidence_for_type_implementation_summary(
    const extract::SymbolInfo &target, const extract::ProjectModel &model,
    const config::EvidenceRulesConfig &rules, std::string_view project_root)
    -> EvidencePack {

  auto root = std::string(project_root);
  EvidencePack pack;
  pack.prompt_kind = "type_implementation_summary";
  pack.subject_name = target.qualified_name;
  pack.subject_kind = std::string(extract::symbol_kind_name(target.kind));
  pack.target_facts.push_back(to_symbol_fact(target, root));

  // Source snippet — primary evidence for implementation
  if (!target.source_snippet.empty()) {
    pack.source_snippets.push_back(
        truncate_snippet(target.source_snippet, rules.max_source_bytes));
  }

  // Members as local context
  std::vector<extract::SymbolID> member_ids;
  for (auto &child_id : target.children) {
    member_ids.push_back(child_id);
  }
  pack.local_context =
      collect_facts(model, member_ids, rules.max_siblings, root);

  // Base types as dependencies
  pack.dependency_context =
      collect_facts(model, target.bases, rules.max_callees, root);

  // Methods' callees as further dependency context
  for (auto &child_id : target.children) {
    if (pack.dependency_context.size() >= rules.max_callees) {
      break;
    }
    if (auto *child = lookup(model, child_id)) {
      if (is_function_kind(child->kind)) {
        auto remaining = rules.max_callees - pack.dependency_context.size();
        auto child_callees =
            collect_facts(model, child->calls, remaining, root);
        for (auto &f : child_callees) {
          pack.dependency_context.push_back(std::move(f));
          if (pack.dependency_context.size() >= rules.max_callees) {
            break;
          }
        }
      }
    }
  }

  return pack;
}

auto format_evidence_text(const EvidencePack &pack) -> std::string {
  return format_evidence_text_bounded(pack,
                                      std::numeric_limits<std::size_t>::max());
}

auto format_evidence_text_bounded(const EvidencePack &pack,
                                  std::size_t max_length) -> std::string {
  std::string text;
  text.reserve(4096);

  if (!append_if_fits(text, "## EVIDENCE\n\n", max_length)) {
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
                         pack.related_page_summaries, render_summary_item,
                         max_length);

  return text;
}

} // namespace clore::generate
