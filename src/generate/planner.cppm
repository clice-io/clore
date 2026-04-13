module;

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <functional>
#include <limits>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

export module generate:planner;

import :model;
import :path;
import config;
import extract;
import support;

export namespace clore::generate {

struct PlanError {
    std::string message;
};

auto build_page_plan_set(const config::TaskConfig& config,
                         const extract::ProjectModel& model)
    -> std::expected<PagePlanSet, PlanError>;

}  // namespace clore::generate

// ── implementation ──────────────────────────────────────────────────

namespace clore::generate {

namespace {

auto split_qualified_name(std::string_view qname) -> std::vector<std::string> {
    std::vector<std::string> parts;
    std::string current;
    current.reserve(qname.size());

    std::size_t template_depth = 0;
    for(std::size_t index = 0; index < qname.size(); ++index) {
        auto ch = qname[index];
        if(ch == '<') {
            template_depth++;
            current.push_back(ch);
            continue;
        }
        if(ch == '>') {
            if(template_depth > 0) {
                template_depth--;
            }
            current.push_back(ch);
            continue;
        }
        if(ch == ':' && template_depth == 0 && index + 1 < qname.size() &&
           qname[index + 1] == ':') {
            parts.push_back(current);
            current.clear();
            ++index;
            continue;
        }
        current.push_back(ch);
    }

    if(!current.empty()) {
        parts.push_back(std::move(current));
    }

    return parts;
}

auto join_qualified_name_parts(const std::vector<std::string>& parts,
                               std::size_t count) -> std::string {
    std::string joined;
    for(std::size_t index = 0; index < count; ++index) {
        if(index > 0) {
            joined += "::";
        }
        joined += parts[index];
    }
    return joined;
}

auto namespace_of(std::string_view qualified_name) -> std::string {
    auto parts = split_qualified_name(qualified_name);
    if(parts.size() <= 1) {
        return {};
    }
    return join_qualified_name_parts(parts, parts.size() - 1);
}

auto short_name_of(std::string_view qualified_name) -> std::string {
    auto parts = split_qualified_name(qualified_name);
    if(parts.empty()) {
        return {};
    }
    return parts.back();
}

auto has_reserved_identifier_prefix(std::string_view identifier) -> bool {
    return identifier.starts_with("_") && identifier.size() > 1 &&
           (std::isupper(static_cast<unsigned char>(identifier[1])) || identifier[1] == '_');
}

auto is_callable_kind(extract::SymbolKind kind) -> bool {
    return kind == extract::SymbolKind::Function || kind == extract::SymbolKind::Method;
}

auto namespace_of(const extract::SymbolInfo& sym) -> std::string {
    if(!sym.enclosing_namespace.empty()) {
        return sym.enclosing_namespace;
    }
    return namespace_of(sym.qualified_name);
}

auto is_renderable_namespace_name(std::string_view ns_name) -> bool {
    if(ns_name.empty() || ns_name.find("(anonymous namespace)") != std::string_view::npos) {
        return false;
    }
    if(ns_name == "std" || ns_name.starts_with("std::")) {
        return false;
    }

    auto parts = split_qualified_name(ns_name);
    if(parts.empty()) {
        return false;
    }

    for(const auto& segment : parts) {
        if(segment.empty() || segment.find('<') != std::string::npos ||
           segment.find('>') != std::string::npos ||
           has_reserved_identifier_prefix(segment)) {
            return false;
        }
    }

    return true;
}

auto is_nested_type_page_candidate(const extract::ProjectModel& model,
                                   const extract::SymbolInfo& sym) -> bool {
    if(is_type_kind(sym.lexical_parent_kind)) {
        return true;
    }

    if(sym.parent.has_value()) {
        if(auto* parent = lookup_sym(model, *sym.parent)) {
            if(is_type_kind(parent->kind)) {
                return true;
            }
        }
    }

    return false;
}

auto should_generate_type_page(const extract::ProjectModel& model,
                               const extract::SymbolInfo& sym) -> bool {
    if(!is_type_kind(sym.kind)) {
        return false;
    }
    if(sym.qualified_name.find("(anonymous namespace)") != std::string::npos) {
        return false;
    }

    auto short_name = short_name_of(sym.qualified_name);
    if(short_name.empty() || has_reserved_identifier_prefix(short_name)) {
        return false;
    }
    if(sym.lexical_parent_kind != extract::SymbolKind::Unknown &&
       sym.lexical_parent_kind != extract::SymbolKind::Namespace) {
        return false;
    }
    if(sym.parent.has_value()) {
        if(auto* parent = lookup_sym(model, *sym.parent)) {
            if(parent->kind == extract::SymbolKind::Function ||
               parent->kind == extract::SymbolKind::Method) {
                return false;
            }
        }
    }
    if(is_nested_type_page_candidate(model, sym)) {
        return false;
    }

    auto ns = namespace_of(sym);
    if(!ns.empty() && !is_renderable_namespace_name(ns)) {
        return false;
    }

    return true;
}

auto find_module_for_file(const extract::ProjectModel& model, const std::string& file_path)
    -> std::optional<std::string> {
    for(auto& [source, mod_unit] : model.modules) {
        if(source == file_path) return mod_unit.name;
    }
    return std::nullopt;
}

struct PlanBuilder {
    const config::TaskConfig& config;
    const extract::ProjectModel& model;
    std::vector<PagePlan> plans;
    std::unordered_map<std::string, std::size_t> id_to_index;
    std::vector<std::pair<std::string, std::string>> path_entries;

    auto add_plan(PagePlan plan) -> std::expected<void, PlanError> {
        auto id = plan.page_id;
        auto path = plan.relative_path;

        if(id_to_index.contains(id)) {
            return std::unexpected(PlanError{
                .message = std::format("duplicate page ID: {}", id)});
        }

        path_entries.emplace_back(path, id);
        id_to_index[id] = plans.size();
        plans.push_back(std::move(plan));
        return {};
    }

    auto get_prompt_template_path(std::string_view slot_kind) const -> std::string {
        if(slot_kind == "type_overview") return config.prompt_templates.type_overview;
        if(slot_kind == "type_usage_notes") return config.prompt_templates.type_usage_notes;
        if(slot_kind == "namespace_summary") return config.prompt_templates.namespace_summary;
        if(slot_kind == "module_summary") return config.prompt_templates.module_summary;
        if(slot_kind == "module_architecture") return config.prompt_templates.module_architecture;
        if(slot_kind == "index_overview") return config.prompt_templates.index_overview;
        if(slot_kind == "index_reading_guide") return config.prompt_templates.index_reading_guide;
        if(slot_kind == "workflow") return config.prompt_templates.workflow;
        return {};
    }

    auto make_slot(const std::string& page_id, std::string_view slot_kind) -> SlotPlan {
        return SlotPlan{
            .slot_id = page_id + "/" + std::string(slot_kind),
            .page_id = page_id,
            .slot_kind = std::string(slot_kind),
            .prompt_template_path = get_prompt_template_path(slot_kind),
            .insertion_marker = "{{slot:" + std::string(slot_kind) + "}}",
        };
    }
};

auto enumerate_type_pages(PlanBuilder& builder) -> std::expected<void, PlanError> {
    if(!builder.config.page_types.type_page) return {};

    for(auto& [id, sym] : builder.model.symbols) {
        if(!should_generate_type_page(builder.model, sym)) continue;

        auto ns = namespace_of(sym);
        auto short_name = short_name_of(sym.qualified_name);
        auto ns_parts = split_qualified_name(ns);

        PageIdentity identity{
            .page_type = PageType::Type,
            .normalized_owner_key = short_name,
            .qualified_name = sym.qualified_name,
            .namespace_segments = ns_parts,
        };

        auto path_result = compute_page_path(identity, builder.config.path_rules);
        if(!path_result.has_value()) {
            return std::unexpected(PlanError{
                .message = std::format("failed to compute path for type '{}': {}",
                                       sym.qualified_name, path_result.error().message)});
        }

        auto page_id = "type:" + sym.qualified_name;
        PagePlan plan{
            .page_id = page_id,
            .page_type = PageType::Type,
            .title = "`" + sym.qualified_name + "`",
            .relative_path = *path_result,
            .owner_keys = {sym.qualified_name},
            .deterministic_blocks = {"declaration", "template_parameters", "base_types",
                                      "derived_types", "public_members", "protected_members",
                                      "private_members", "source", "related_pages", "type_diagram"},
        };

        plan.slot_plans.push_back(builder.make_slot(page_id, "type_overview"));
        plan.slot_plans.push_back(builder.make_slot(page_id, "type_usage_notes"));

        // Dependencies: base types
        for(auto& base_id : sym.bases) {
            if(auto* base = lookup_sym(builder.model, base_id)) {
                if(should_generate_type_page(builder.model, *base)) {
                    plan.depends_on_pages.push_back("type:" + base->qualified_name);
                    plan.linked_pages.push_back("type:" + base->qualified_name);
                }
            }
        }
        // Links to derived types
        for(auto& derived_id : sym.derived) {
            if(auto* derived = lookup_sym(builder.model, derived_id)) {
                if(should_generate_type_page(builder.model, *derived)) {
                    plan.linked_pages.push_back("type:" + derived->qualified_name);
                }
            }
        }
        // Link to enclosing namespace
        if(!ns.empty() && is_renderable_namespace_name(ns)) {
            plan.linked_pages.push_back("namespace:" + ns);
        }

        if(auto r = builder.add_plan(std::move(plan)); !r) return r;
    }

    return {};
}

auto enumerate_namespace_pages(PlanBuilder& builder) -> std::expected<void, PlanError> {
    if(!builder.config.page_types.namespace_page) return {};

    for(auto& [ns_name, ns_info] : builder.model.namespaces) {
        if(ns_info.symbols.empty() && ns_info.children.empty()) continue;
        if(!is_renderable_namespace_name(ns_name)) continue;

        auto parts = split_qualified_name(ns_name);

        PageIdentity identity{
            .page_type = PageType::Namespace,
            .normalized_owner_key = ns_name,
            .qualified_name = ns_name,
            .namespace_segments = parts,
        };

        auto path_result = compute_page_path(identity, builder.config.path_rules);
        if(!path_result.has_value()) {
            return std::unexpected(PlanError{
                .message = std::format("failed to compute path for namespace '{}': {}",
                                       ns_name, path_result.error().message)});
        }

        auto page_id = "namespace:" + ns_name;
        PagePlan plan{
            .page_id = page_id,
            .page_type = PageType::Namespace,
            .title = "Namespace `" + ns_name + "`",
            .relative_path = *path_result,
            .owner_keys = {ns_name},
            .deterministic_blocks = {"subnamespaces", "types_index", "functions_index", "related_pages", "namespace_diagram"},
        };

        plan.slot_plans.push_back(builder.make_slot(page_id, "namespace_summary"));

        // Depends on type pages in this namespace
        for(auto& sym_id : ns_info.symbols) {
            if(auto* sym = lookup_sym(builder.model, sym_id)) {
                if(should_generate_type_page(builder.model, *sym)) {
                    plan.depends_on_pages.push_back("type:" + sym->qualified_name);
                    plan.linked_pages.push_back("type:" + sym->qualified_name);
                }
            }
        }

        // Link to parent namespace
        auto parent_ns = namespace_of(ns_name);
        if(!parent_ns.empty() && is_renderable_namespace_name(parent_ns)) {
            plan.linked_pages.push_back("namespace:" + parent_ns);
        }

        // Link to child namespaces
        for(auto& child_ns : ns_info.children) {
            if(!is_renderable_namespace_name(child_ns)) continue;
            plan.linked_pages.push_back("namespace:" + child_ns);
        }

        if(auto r = builder.add_plan(std::move(plan)); !r) return r;
    }

    return {};
}

auto enumerate_module_pages(PlanBuilder& builder) -> std::expected<void, PlanError> {
    if(!builder.config.page_types.module_page) return {};
    if(!builder.model.uses_modules) return {};

    // Collect interface module names
    std::unordered_set<std::string> seen;
    for(auto& [source_file, mod_unit] : builder.model.modules) {
        if(!mod_unit.is_interface) continue;
        if(!seen.insert(mod_unit.name).second) continue;

        auto parts = split_qualified_name(mod_unit.name);

        PageIdentity identity{
            .page_type = PageType::Module,
            .normalized_owner_key = mod_unit.name,
            .qualified_name = mod_unit.name,
            .module_segments = parts,
        };

        auto path_result = compute_page_path(identity, builder.config.path_rules);
        if(!path_result.has_value()) {
            return std::unexpected(PlanError{
                .message = std::format("failed to compute path for module '{}': {}",
                                       mod_unit.name, path_result.error().message)});
        }

        auto page_id = "module:" + mod_unit.name;
        PagePlan plan{
            .page_id = page_id,
            .page_type = PageType::Module,
            .title = "Module `" + mod_unit.name + "`",
            .relative_path = *path_result,
            .owner_keys = {mod_unit.name},
            .deterministic_blocks = {"imports", "types_index", "related_pages", "import_diagram"},
        };

        plan.slot_plans.push_back(builder.make_slot(page_id, "module_summary"));
        plan.slot_plans.push_back(builder.make_slot(page_id, "module_architecture"));

        // Dependencies: imported modules
        for(auto& import_name : mod_unit.imports) {
            plan.depends_on_pages.push_back("module:" + import_name);
            plan.linked_pages.push_back("module:" + import_name);
        }

        // Dependencies: contained type pages
        for(auto& sym_id : mod_unit.symbols) {
            if(auto* sym = lookup_sym(builder.model, sym_id)) {
                if(should_generate_type_page(builder.model, *sym)) {
                    plan.depends_on_pages.push_back("type:" + sym->qualified_name);
                    plan.linked_pages.push_back("type:" + sym->qualified_name);
                }
            }
        }

        if(auto r = builder.add_plan(std::move(plan)); !r) return r;
    }

    return {};
}

auto enumerate_file_pages(PlanBuilder& builder) -> std::expected<void, PlanError> {
    if(!builder.config.page_types.file_page) return {};
    namespace fs = std::filesystem;

    auto source_root = fs::path(builder.config.project_root).lexically_normal();

    for(auto& [file_path, file_info] : builder.model.files) {
        // Only files that have symbols or module declarations
        bool has_module = builder.model.modules.contains(file_path);
        if(file_info.symbols.empty() && !has_module) continue;

        auto abs = fs::path(file_path).lexically_normal();
        auto rel = abs.lexically_relative(source_root);
        if(rel.empty()) continue;
        for(const auto& part : rel) {
            if(part == "..") { rel = fs::path{}; break; }
        }
        if(rel.empty()) continue;

        auto rel_str = rel.generic_string();

        PageIdentity identity{
            .page_type = PageType::File,
            .normalized_owner_key = file_path,
            .source_relative_path = rel_str,
        };

        auto path_result = compute_page_path(identity, builder.config.path_rules);
        if(!path_result.has_value()) {
            return std::unexpected(PlanError{
                .message = std::format("failed to compute path for file '{}': {}",
                                       file_path, path_result.error().message)});
        }

        auto page_id = "file:" + file_path;
        PagePlan plan{
            .page_id = page_id,
            .page_type = PageType::File,
            .title = "File `" + rel_str + "`",
            .relative_path = *path_result,
            .owner_keys = {file_path},
            .deterministic_blocks = {"includes", "declared_symbols", "defined_symbols", "module_info"},
        };

        if(auto r = builder.add_plan(std::move(plan)); !r) return r;
    }

    return {};
}

auto enumerate_workflow_pages(PlanBuilder& builder) -> std::expected<void, PlanError> {
    if(!builder.config.page_types.workflow_page) return {};

    auto& model = builder.model;

    struct ModuleEdgeRep {
        std::string from_module;
        std::string to_module;
        std::string caller_qname;
        std::string callee_qname;
        std::size_t score = 0;
    };

    auto should_replace_edge = [](const ModuleEdgeRep& existing,
                                  const ModuleEdgeRep& candidate) -> bool {
        if(existing.caller_qname.empty()) return true;
        if(candidate.score != existing.score) return candidate.score > existing.score;
        if(candidate.caller_qname != existing.caller_qname) {
            return candidate.caller_qname < existing.caller_qname;
        }
        return candidate.callee_qname < existing.callee_qname;
    };

    auto edge_score = [](const extract::SymbolInfo& caller, const extract::SymbolInfo& callee)
        -> std::size_t {
        return caller.calls.size() + callee.called_by.size();
    };

    std::unordered_map<extract::SymbolID, std::string> symbol_to_module;
    symbol_to_module.reserve(model.symbols.size());
    for(auto& [sym_id, sym] : model.symbols) {
        if(!is_callable_kind(sym.kind)) continue;
        if(sym.qualified_name.empty()) continue;
        symbol_to_module.emplace(sym_id, sym.qualified_name);
    }
    if(symbol_to_module.size() < 2) return {};

    std::unordered_map<std::string, std::size_t> module_callable_weights;
    for(auto& [sym_id, mod_name] : symbol_to_module) {
        auto* sym = lookup_sym(model, sym_id);
        if(sym && is_callable_kind(sym->kind)) {
            module_callable_weights[mod_name]++;
        }
    }

    std::unordered_map<std::string, std::unordered_map<std::string, ModuleEdgeRep>> module_edges;
    std::unordered_set<std::string> active_modules;

    for(auto& [caller_id, caller] : model.symbols) {
        if(!is_callable_kind(caller.kind)) continue;
        auto caller_mod_it = symbol_to_module.find(caller_id);
        if(caller_mod_it == symbol_to_module.end()) continue;

        for(auto& callee_id : caller.calls) {
            auto* callee = lookup_sym(model, callee_id);
            if(!callee || !is_callable_kind(callee->kind)) continue;

            auto callee_mod_it = symbol_to_module.find(callee_id);
            if(callee_mod_it == symbol_to_module.end()) continue;
            if(caller_mod_it->second == callee_mod_it->second) continue;

            active_modules.insert(caller_mod_it->second);
            active_modules.insert(callee_mod_it->second);

            ModuleEdgeRep candidate{
                .from_module = caller_mod_it->second,
                .to_module = callee_mod_it->second,
                .caller_qname = caller.qualified_name,
                .callee_qname = callee->qualified_name,
                .score = edge_score(caller, *callee),
            };

            auto& current = module_edges[caller_mod_it->second][callee_mod_it->second];
            if(should_replace_edge(current, candidate)) {
                current.from_module = candidate.from_module;
                current.to_module = candidate.to_module;
                current.caller_qname = candidate.caller_qname;
                current.callee_qname = candidate.callee_qname;
                current.score = candidate.score;
            }
        }
    }

    if(active_modules.size() < 2) return {};

    std::vector<std::string> modules(active_modules.begin(), active_modules.end());
    std::sort(modules.begin(), modules.end());

    std::unordered_map<std::string, std::size_t> module_to_index;
    module_to_index.reserve(modules.size());
    for(std::size_t i = 0; i < modules.size(); ++i) {
        module_to_index.emplace(modules[i], i);
    }

    std::vector<std::vector<std::size_t>> graph(modules.size());
    std::unordered_map<std::size_t, std::unordered_map<std::size_t, ModuleEdgeRep>> edge_by_idx;

    for(auto& from_mod : modules) {
        auto from_it = module_edges.find(from_mod);
        if(from_it == module_edges.end()) continue;

        std::vector<std::string> targets;
        targets.reserve(from_it->second.size());
        for(auto& [to_mod, rep] : from_it->second) {
            if(module_to_index.contains(to_mod)) {
                targets.push_back(to_mod);
            }
        }
        std::sort(targets.begin(), targets.end());

        auto from_idx = module_to_index.at(from_mod);
        for(auto& to_mod : targets) {
            auto to_idx = module_to_index.at(to_mod);
            graph[from_idx].push_back(to_idx);
            edge_by_idx[from_idx][to_idx] = from_it->second.at(to_mod);
        }
    }

    std::vector<int> tarjan_index(modules.size(), -1);
    std::vector<int> tarjan_low(modules.size(), 0);
    std::vector<bool> on_stack(modules.size(), false);
    std::vector<std::size_t> stack_nodes;
    std::vector<std::size_t> comp_of(modules.size(), std::numeric_limits<std::size_t>::max());
    std::vector<std::vector<std::size_t>> components;
    int next_index = 0;

    std::function<void(std::size_t)> strong_connect = [&](std::size_t v) {
        tarjan_index[v] = next_index;
        tarjan_low[v] = next_index;
        next_index++;
        stack_nodes.push_back(v);
        on_stack[v] = true;

        for(auto to : graph[v]) {
            if(tarjan_index[to] == -1) {
                strong_connect(to);
                tarjan_low[v] = std::min(tarjan_low[v], tarjan_low[to]);
            } else if(on_stack[to]) {
                tarjan_low[v] = std::min(tarjan_low[v], tarjan_index[to]);
            }
        }

        if(tarjan_low[v] != tarjan_index[v]) return;

        components.emplace_back();
        auto comp_idx = components.size() - 1;

        while(true) {
            auto w = stack_nodes.back();
            stack_nodes.pop_back();
            on_stack[w] = false;
            comp_of[w] = comp_idx;
            components.back().push_back(w);
            if(w == v) break;
        }

        std::sort(components.back().begin(), components.back().end(),
                  [&](std::size_t lhs, std::size_t rhs) {
                      return modules[lhs] < modules[rhs];
                  });
    };

    for(std::size_t v = 0; v < modules.size(); ++v) {
        if(tarjan_index[v] == -1) {
            strong_connect(v);
        }
    }

    if(components.size() < 2) return {};

    std::vector<std::vector<std::size_t>> comp_graph(components.size());
    std::vector<std::unordered_set<std::size_t>> comp_seen(components.size());
    std::unordered_map<std::size_t, std::unordered_map<std::size_t, ModuleEdgeRep>> comp_edges;
    std::vector<std::string> comp_key(components.size());
    std::vector<std::size_t> comp_weight(components.size(), 0);

    for(std::size_t comp_idx = 0; comp_idx < components.size(); ++comp_idx) {
        comp_key[comp_idx] = modules[components[comp_idx].front()];
        for(auto mod_idx : components[comp_idx]) {
            auto mod_name = modules[mod_idx];
            auto w_it = module_callable_weights.find(mod_name);
            comp_weight[comp_idx] += w_it != module_callable_weights.end()
                ? std::max<std::size_t>(1, w_it->second)
                : 1;
        }
    }

    for(std::size_t from_idx = 0; from_idx < graph.size(); ++from_idx) {
        auto from_comp = comp_of[from_idx];
        for(auto to_idx : graph[from_idx]) {
            auto to_comp = comp_of[to_idx];
            if(from_comp == to_comp) continue;

            if(comp_seen[from_comp].insert(to_comp).second) {
                comp_graph[from_comp].push_back(to_comp);
            }

            auto rep_it = edge_by_idx.find(from_idx);
            if(rep_it == edge_by_idx.end()) continue;
            auto edge_it = rep_it->second.find(to_idx);
            if(edge_it == rep_it->second.end()) continue;
            auto& current = comp_edges[from_comp][to_comp];
            if(should_replace_edge(current, edge_it->second)) {
                current = edge_it->second;
            }
        }
    }

    for(std::size_t comp_idx = 0; comp_idx < comp_graph.size(); ++comp_idx) {
        std::sort(comp_graph[comp_idx].begin(), comp_graph[comp_idx].end(),
                  [&](std::size_t lhs, std::size_t rhs) {
                      return comp_key[lhs] < comp_key[rhs];
                  });
    }

    std::vector<std::size_t> indegree(components.size(), 0);
    for(auto& targets : comp_graph) {
        for(auto to : targets) {
            indegree[to]++;
        }
    }

    std::unordered_set<std::size_t> source_components;
    std::unordered_set<std::size_t> sink_components;
    for(std::size_t i = 0; i < components.size(); ++i) {
        if(indegree[i] == 0) source_components.insert(i);
        if(comp_graph[i].empty()) sink_components.insert(i);
    }

    if(source_components.empty() || sink_components.empty()) return {};

    std::unordered_set<std::size_t> covered_components;

    auto find_longest_path = [&]() -> std::vector<std::size_t> {
        std::vector<bool> available(components.size(), false);
        std::size_t available_count = 0;
        for(std::size_t comp_idx = 0; comp_idx < components.size(); ++comp_idx) {
            auto keep_boundary = source_components.contains(comp_idx) ||
                                 sink_components.contains(comp_idx);
            if(!covered_components.contains(comp_idx) || keep_boundary) {
                available[comp_idx] = true;
                available_count++;
            }
        }
        if(available_count < 2) return {};

        std::vector<std::size_t> in_deg(components.size(), 0);
        for(std::size_t from = 0; from < comp_graph.size(); ++from) {
            if(!available[from]) continue;
            for(auto to : comp_graph[from]) {
                if(available[to]) {
                    in_deg[to]++;
                }
            }
        }

        std::set<std::pair<std::string, std::size_t>> ready;
        for(std::size_t comp_idx = 0; comp_idx < components.size(); ++comp_idx) {
            if(available[comp_idx] && in_deg[comp_idx] == 0) {
                ready.emplace(comp_key[comp_idx], comp_idx);
            }
        }

        std::vector<std::size_t> topo;
        topo.reserve(available_count);
        while(!ready.empty()) {
            auto [key, current] = *ready.begin();
            ready.erase(ready.begin());
            topo.push_back(current);
            for(auto to : comp_graph[current]) {
                if(available[to] && --in_deg[to] == 0) {
                    ready.emplace(comp_key[to], to);
                }
            }
        }

        if(topo.size() < 2) return {};

        auto no_pred = std::numeric_limits<std::size_t>::max();
        std::vector<std::size_t> dist(components.size(), 0);
        std::vector<std::size_t> pred(components.size(), no_pred);
        for(auto comp_idx : topo) {
            dist[comp_idx] = comp_weight[comp_idx];
        }

        for(auto from : topo) {
            for(auto to : comp_graph[from]) {
                if(!available[to]) continue;
                auto candidate = dist[from] + comp_weight[to];
                if(candidate > dist[to]) {
                    dist[to] = candidate;
                    pred[to] = from;
                    continue;
                }
                if(candidate == dist[to] &&
                   (pred[to] == no_pred || comp_key[from] < comp_key[pred[to]])) {
                    pred[to] = from;
                }
            }
        }

        std::size_t best = no_pred;
        std::size_t best_dist = 0;
        for(auto comp_idx : topo) {
            if(dist[comp_idx] > best_dist) {
                best = comp_idx;
                best_dist = dist[comp_idx];
                continue;
            }
            if(dist[comp_idx] == best_dist && best != no_pred &&
               comp_key[comp_idx] < comp_key[best]) {
                best = comp_idx;
            }
        }

        if(best == no_pred) return {};

        std::vector<std::size_t> path;
        for(auto current = best; current != no_pred; current = pred[current]) {
            path.push_back(current);
        }
        std::reverse(path.begin(), path.end());
        if(path.size() < 2) return {};
        return path;
    };

    std::vector<std::vector<std::size_t>> workflow_paths;
    while(true) {
        auto path = find_longest_path();
        if(path.size() < 2) break;

        std::size_t new_coverage = 0;
        if(path.size() == 2) {
            for(auto node : path) {
                if(!covered_components.contains(node)) {
                    new_coverage++;
                }
            }
        } else {
            for(std::size_t i = 1; i + 1 < path.size(); ++i) {
                if(!covered_components.contains(path[i])) {
                    new_coverage++;
                }
            }
        }

        if(new_coverage == 0) {
            if(workflow_paths.empty()) {
                workflow_paths.push_back(std::move(path));
            }
            break;
        }

        if(path.size() == 2) {
            covered_components.insert(path[0]);
            covered_components.insert(path[1]);
        } else {
            for(std::size_t i = 1; i + 1 < path.size(); ++i) {
                covered_components.insert(path[i]);
            }
        }
        workflow_paths.push_back(std::move(path));
    }

    if(workflow_paths.empty()) return {};

    auto lookup_symbol_by_qname = [&](std::string_view qname) -> const extract::SymbolInfo* {
        for(auto& [sym_id, sym] : model.symbols) {
            if(sym.qualified_name == qname) return &sym;
        }
        return nullptr;
    };

    auto slugify = [](std::string_view text) -> std::string {
        std::string out;
        out.reserve(text.size());
        bool prev_dash = false;
        for(auto ch : text) {
            auto uc = static_cast<unsigned char>(ch);
            if(std::isalnum(uc)) {
                out.push_back(static_cast<char>(std::tolower(uc)));
                prev_dash = false;
            } else if(!prev_dash) {
                out.push_back('-');
                prev_dash = true;
            }
        }
        while(!out.empty() && out.front() == '-') out.erase(out.begin());
        while(!out.empty() && out.back() == '-') out.pop_back();
        return out;
    };

    auto& rules = builder.config.workflow_rules;

    auto overlap_ratio_percent =
        [](const std::unordered_set<std::string>& lhs,
           const std::unordered_set<std::string>& rhs) -> std::uint32_t {
        if(lhs.empty() || rhs.empty()) return 0;

        const auto* smaller = &lhs;
        const auto* larger = &rhs;
        if(lhs.size() > rhs.size()) {
            smaller = &rhs;
            larger = &lhs;
        }

        std::size_t intersection = 0;
        for(auto& key : *smaller) {
            if(larger->contains(key)) {
                ++intersection;
            }
        }

        auto base = std::min(lhs.size(), rhs.size());
        if(base == 0) return 0;
        auto ratio = (intersection * 100) / base;
        return static_cast<std::uint32_t>(ratio);
    };

    std::vector<std::unordered_set<std::string>> accepted_symbol_sets;
    std::unordered_set<std::string> accepted_symbols_union;
    std::unordered_map<std::string, std::size_t> workflow_slug_counts;

    for(auto& comp_path : workflow_paths) {
        std::vector<ModuleEdgeRep> chain_edges;
        chain_edges.reserve(comp_path.size() > 1 ? comp_path.size() - 1 : 0);
        bool valid_chain = true;
        for(std::size_t i = 0; i + 1 < comp_path.size(); ++i) {
            auto from_comp = comp_path[i];
            auto to_comp = comp_path[i + 1];
            auto from_it = comp_edges.find(from_comp);
            if(from_it == comp_edges.end()) {
                valid_chain = false;
                break;
            }
            auto edge_it = from_it->second.find(to_comp);
            if(edge_it == from_it->second.end() || edge_it->second.caller_qname.empty() ||
               edge_it->second.callee_qname.empty()) {
                valid_chain = false;
                break;
            }
            chain_edges.push_back(edge_it->second);
        }
        if(!valid_chain || chain_edges.empty()) continue;

        std::vector<std::string> owner_keys;
        owner_keys.reserve(chain_edges.size() + 1);
        owner_keys.push_back(chain_edges.front().caller_qname);
        owner_keys.push_back(chain_edges.front().callee_qname);
        for(std::size_t i = 1; i < chain_edges.size(); ++i) {
            auto& edge = chain_edges[i];
            if(owner_keys.back() != edge.caller_qname) {
                owner_keys.push_back(edge.caller_qname);
            }
            if(owner_keys.back() != edge.callee_qname) {
                owner_keys.push_back(edge.callee_qname);
            }
        }
        if(owner_keys.size() < rules.min_chain_symbols) continue;

        std::unordered_set<std::string> current_symbol_set;
        current_symbol_set.reserve(owner_keys.size());
        for(auto& key : owner_keys) {
            current_symbol_set.insert(key);
        }
        if(current_symbol_set.size() < rules.min_chain_symbols) continue;

        std::size_t new_symbol_count = 0;
        for(auto& key : current_symbol_set) {
            if(!accepted_symbols_union.contains(key)) {
                ++new_symbol_count;
            }
        }
        if(new_symbol_count < rules.min_new_symbols) continue;

        bool overlap_rejected = false;
        for(auto& accepted : accepted_symbol_sets) {
            if(overlap_ratio_percent(current_symbol_set, accepted) >
               rules.max_symbol_overlap_ratio_percent) {
                overlap_rejected = true;
                break;
            }
        }
        if(overlap_rejected) continue;

        auto first_short = short_name_of(owner_keys.front());
        auto last_short = short_name_of(owner_keys.back());
        if(first_short.empty() || last_short.empty()) continue;

        auto base_slug = slugify(first_short + "-to-" + last_short);
        if(base_slug.empty()) continue;

        auto& slug_count = workflow_slug_counts[base_slug];
        std::string slug;
        std::string page_id;
        while(true) {
            ++slug_count;
            slug = base_slug;
            if(slug_count > 1) {
                slug += std::format("-{}", slug_count);
            }
            page_id = "workflow:" + slug;
            if(!builder.id_to_index.contains(page_id)) {
                break;
            }
        }

        PageIdentity identity{
            .page_type = PageType::Workflow,
            .normalized_owner_key = slug,
        };

        auto path_result = compute_page_path(identity, builder.config.path_rules);
        if(!path_result.has_value()) {
            return std::unexpected(PlanError{
                .message = std::format("failed to compute path for workflow '{}': {}",
                                       slug, path_result.error().message)});
        }

        std::string title_chain;
        bool valid_title_chain = true;
        for(std::size_t i = 0; i < owner_keys.size(); ++i) {
            if(i > 0) title_chain += " → ";
            auto short_name = short_name_of(owner_keys[i]);
            if(short_name.empty()) {
                valid_title_chain = false;
                break;
            }
            title_chain += short_name;
        }
        if(!valid_title_chain || title_chain.empty()) continue;

        PagePlan plan{
            .page_id = page_id,
            .page_type = PageType::Workflow,
            .title = "Workflow: " + title_chain,
            .relative_path = *path_result,
            .owner_keys = std::move(owner_keys),
            .deterministic_blocks = {"call_chain", "participants", "related_pages"},
        };

        plan.slot_plans.push_back(builder.make_slot(page_id, "workflow"));

        for(auto& existing : builder.plans) {
            plan.depends_on_pages.push_back(existing.page_id);
        }

        std::unordered_set<std::string> linked_seen;
        auto add_linked_page = [&](std::string candidate_page_id) {
            if(candidate_page_id.empty()) return;
            if(linked_seen.insert(candidate_page_id).second) {
                plan.linked_pages.push_back(std::move(candidate_page_id));
            }
        };

        for(auto& qname : owner_keys) {
            auto* sym = lookup_symbol_by_qname(qname);
            if(!sym) continue;

            if(!sym->declaration_location.file.empty()) {
                add_linked_page("file:" + sym->declaration_location.file);
            }

            auto ns_name = namespace_of(*sym);
            if(is_renderable_namespace_name(ns_name)) {
                add_linked_page("namespace:" + ns_name);
            }

            if(sym->parent.has_value()) {
                if(auto* parent = lookup_sym(model, *sym->parent)) {
                    if(should_generate_type_page(model, *parent)) {
                        add_linked_page("type:" + parent->qualified_name);
                    }
                }
            }
        }

        if(auto r = builder.add_plan(std::move(plan)); !r) return r;

        accepted_symbol_sets.push_back(std::move(current_symbol_set));
        for(auto& key : accepted_symbol_sets.back()) {
            accepted_symbols_union.insert(key);
        }
        if(accepted_symbol_sets.size() >= rules.max_workflow_pages) {
            break;
        }
    }

    return {};
}

auto enumerate_index_page(PlanBuilder& builder) -> std::expected<void, PlanError> {
    if(!builder.config.page_types.index) return {};

    PageIdentity identity{
        .page_type = PageType::Index,
        .normalized_owner_key = "index",
        .qualified_name = "index",
    };

    auto path_result = compute_page_path(identity, builder.config.path_rules);
    if(!path_result.has_value()) {
        return std::unexpected(PlanError{
            .message =
                "failed to compute path for index page: " + path_result.error().message});
    }

    auto page_id = std::string("index");
    PagePlan plan{
        .page_id = page_id,
        .page_type = PageType::Index,
        .title = "API Reference",
        .relative_path = *path_result,
        .deterministic_blocks = {"all_modules", "all_namespaces", "all_types", "all_files", "all_workflows", "module_dependency_diagram"},
    };

    plan.slot_plans.push_back(builder.make_slot(page_id, "index_overview"));
    plan.slot_plans.push_back(builder.make_slot(page_id, "index_reading_guide"));

    // Index depends on all content pages so overview + reading guide can use
    // dependency summaries and links with complete context.
    for(auto& existing : builder.plans) {
        plan.depends_on_pages.push_back(existing.page_id);
        if(existing.page_type == PageType::Workflow) {
            plan.linked_pages.push_back(existing.page_id);
        }
    }

    if(auto r = builder.add_plan(std::move(plan)); !r) return r;
    return {};
}

auto topological_sort(const std::vector<PagePlan>& plans,
                      const std::unordered_map<std::string, std::size_t>& id_to_index)
    -> std::vector<std::string> {

    std::unordered_map<std::string, int> in_degree;
    std::unordered_map<std::string, std::vector<std::string>> reverse_edges;

    for(auto& plan : plans) {
        in_degree[plan.page_id];  // ensure exists
    }

    for(auto& plan : plans) {
        for(auto& dep : plan.depends_on_pages) {
            if(id_to_index.contains(dep)) {
                in_degree[plan.page_id]++;
                reverse_edges[dep].push_back(plan.page_id);
            }
        }
    }

    std::set<std::string> ready;
    for(auto& [id, degree] : in_degree) {
        if(degree == 0) ready.insert(id);
    }

    std::vector<std::string> order;
    order.reserve(plans.size());

    while(order.size() < plans.size()) {
        if(ready.empty()) {
            std::optional<std::string> injected;
            for(auto& plan : plans) {
                if(in_degree[plan.page_id] > 0 &&
                   (!injected.has_value() || plan.page_id < *injected)) {
                    injected = plan.page_id;
                }
            }
            if(!injected.has_value()) break;

            // Break cycles incrementally and let the existing topo loop
            // continue to release newly satisfiable dependents.
            in_degree[*injected] = 0;
            ready.insert(*injected);
        }

        auto current = *ready.begin();
        ready.erase(ready.begin());
        order.push_back(current);

        auto it = reverse_edges.find(current);
        if(it != reverse_edges.end()) {
            for(auto& dependent : it->second) {
                auto& degree = in_degree[dependent];
                if(degree > 0 && --degree == 0) {
                    ready.insert(dependent);
                }
            }
        }
    }

    return order;
}

}  // namespace

auto build_page_plan_set(const config::TaskConfig& config,
                         const extract::ProjectModel& model)
    -> std::expected<PagePlanSet, PlanError> {

    PlanBuilder builder{.config = config, .model = model};

    // 1. Enumerate pages in dependency order: types first, then files, namespaces, modules, index
    if(auto r = enumerate_type_pages(builder); !r) {
        return std::unexpected(PlanError{.message = r.error().message});
    }
    logging::info("planner: {} type pages", builder.plans.size());

    auto type_count = builder.plans.size();
    if(auto r = enumerate_file_pages(builder); !r) {
        return std::unexpected(PlanError{.message = r.error().message});
    }
    logging::info("planner: {} file pages", builder.plans.size() - type_count);

    auto file_count = builder.plans.size();
    if(auto r = enumerate_namespace_pages(builder); !r) {
        return std::unexpected(PlanError{.message = r.error().message});
    }
    logging::info("planner: {} namespace pages", builder.plans.size() - file_count);

    auto ns_count = builder.plans.size();
    if(auto r = enumerate_module_pages(builder); !r) {
        return std::unexpected(PlanError{.message = r.error().message});
    }
    logging::info("planner: {} module pages", builder.plans.size() - ns_count);

    auto workflow_base = builder.plans.size();
    if(auto r = enumerate_workflow_pages(builder); !r) {
        return std::unexpected(PlanError{.message = r.error().message});
    }
    logging::info("planner: {} workflow pages", builder.plans.size() - workflow_base);

    // 2. Index page (after all content pages)
    if(auto r = enumerate_index_page(builder); !r) {
        return std::unexpected(PlanError{.message = r.error().message});
    }

    logging::info("planner: {} total pages", builder.plans.size());

    if(builder.plans.empty()) {
        return std::unexpected(PlanError{.message = "no pages to generate"});
    }

    // 3. Validate path conflicts
    auto path_check = validate_no_path_conflicts(builder.path_entries);
    if(!path_check.has_value()) {
        return std::unexpected(PlanError{.message = path_check.error().message});
    }

    // 4. Topological sort
    auto order = topological_sort(builder.plans, builder.id_to_index);

    return PagePlanSet{
        .plans = std::move(builder.plans),
        .generation_order = std::move(order),
    };
}

}  // namespace clore::generate
