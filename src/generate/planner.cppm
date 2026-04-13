module;

#include <algorithm>
#include <cctype>
#include <expected>
#include <filesystem>
#include <format>
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

auto is_type_kind(extract::SymbolKind kind) -> bool {
    switch(kind) {
        case extract::SymbolKind::Class:
        case extract::SymbolKind::Struct:
        case extract::SymbolKind::Enum:
        case extract::SymbolKind::Union:
        case extract::SymbolKind::Concept:
        case extract::SymbolKind::Template:
        case extract::SymbolKind::TypeAlias:
            return true;
        default:
            return false;
    }
}

auto has_reserved_identifier_prefix(std::string_view identifier) -> bool {
    return identifier.starts_with("_") && identifier.size() > 1 &&
           (std::isupper(static_cast<unsigned char>(identifier[1])) || identifier[1] == '_');
}

auto lookup_sym(const extract::ProjectModel& model, extract::SymbolID id)
    -> const extract::SymbolInfo* {
    auto it = model.symbols.find(id);
    return it != model.symbols.end() ? &it->second : nullptr;
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
        if(slot_kind == "repository_overview") return config.prompt_templates.repository_overview;
        if(slot_kind == "reading_guide") return config.prompt_templates.reading_guide;
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
                                      "private_members", "source", "related_pages"},
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
            .deterministic_blocks = {"subnamespaces", "types_index", "functions_index", "related_pages"},
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
            .deterministic_blocks = {"imports", "types_index", "related_pages"},
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
        .deterministic_blocks = {"all_modules", "all_namespaces", "all_types", "all_files"},
    };

    plan.slot_plans.push_back(builder.make_slot(page_id, "repository_overview"));
    plan.slot_plans.push_back(builder.make_slot(page_id, "reading_guide"));

    // Index depends on all content pages so overview + reading guide can use
    // dependency summaries and links with complete context.
    for(auto& existing : builder.plans) {
        plan.depends_on_pages.push_back(existing.page_id);
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
