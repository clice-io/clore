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

auto namespace_of(std::string_view qualified_name) -> std::string {
    return extract::namespace_prefix_from_qualified_name(qualified_name);
}

auto short_name_of(std::string_view qualified_name) -> std::string {
    auto parts = extract::split_top_level_qualified_name(qualified_name);
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

    auto parts = extract::split_top_level_qualified_name(ns_name);
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

auto find_module_for_file(const extract::ProjectModel& model, const std::string& file_path)
    -> std::optional<std::string> {
    if(auto* mod = extract::find_module_by_source(model, file_path)) {
        return mod->name;
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

    auto make_page_prompt(PromptKind kind) const -> PromptRequest {
        return PromptRequest{.kind = kind};
    }

    auto make_symbol_prompt(PromptKind kind, const std::string& symbol_key) const
        -> PromptRequest {
        return PromptRequest{
            .kind = kind,
            .target_key = symbol_key,
        };
    }
};

auto enumerate_namespace_pages(PlanBuilder& builder) -> std::expected<void, PlanError> {
    for(auto& [ns_name, ns_info] : builder.model.namespaces) {
        if(ns_info.symbols.empty() && ns_info.children.empty()) continue;
        if(!is_renderable_namespace_name(ns_name)) continue;

        auto parts = extract::split_top_level_qualified_name(ns_name);

        PageIdentity identity{
            .page_type = PageType::Namespace,
            .normalized_owner_key = ns_name,
            .qualified_name = ns_name,
        };

        auto path_result = compute_page_path(identity);
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
        };

        plan.prompt_requests.push_back(builder.make_page_prompt(PromptKind::NamespaceSummary));

        // Per-symbol declaration prompts for types and functions
        for(auto& sym_id : ns_info.symbols) {
            if(auto* sym = lookup_sym(builder.model, sym_id)) {
                if(!is_page_level_symbol(builder.model, *sym)) {
                    continue;
                }
                if(is_type_kind(sym->kind)) {
                    plan.prompt_requests.push_back(
                        builder.make_symbol_prompt(PromptKind::TypeDeclarationSummary,
                                                   make_symbol_target_key(*sym)));
                } else if(is_function_kind(sym->kind)) {
                    plan.prompt_requests.push_back(
                        builder.make_symbol_prompt(PromptKind::FunctionDeclarationSummary,
                                                   make_symbol_target_key(*sym)));
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
    if(!builder.model.uses_modules) return {};

    // Collect interface module names
    std::unordered_set<std::string> seen;
    for(auto& [source_file, mod_unit] : builder.model.modules) {
        if(!mod_unit.is_interface) continue;
        if(!seen.insert(mod_unit.name).second) continue;

        auto parts = extract::split_top_level_qualified_name(mod_unit.name);

        PageIdentity identity{
            .page_type = PageType::Module,
            .normalized_owner_key = mod_unit.name,
            .qualified_name = mod_unit.name,
        };

        auto path_result = compute_page_path(identity);
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
        };

        plan.prompt_requests.push_back(builder.make_page_prompt(PromptKind::ModuleSummary));
        plan.prompt_requests.push_back(builder.make_page_prompt(PromptKind::ModuleArchitecture));

        // Per-symbol implementation prompts for types and functions in this module
        for(auto& sym_id : mod_unit.symbols) {
            if(auto* sym = lookup_sym(builder.model, sym_id)) {
                if(!is_page_level_symbol(builder.model, *sym)) {
                    continue;
                }
                if(is_type_kind(sym->kind)) {
                    plan.prompt_requests.push_back(
                        builder.make_symbol_prompt(PromptKind::TypeImplementationSummary,
                                                   make_symbol_target_key(*sym)));
                } else if(is_function_kind(sym->kind)) {
                    plan.prompt_requests.push_back(
                        builder.make_symbol_prompt(PromptKind::FunctionImplementationSummary,
                                                   make_symbol_target_key(*sym)));
                }
            }
        }

        // Dependencies: imported modules
        for(auto& import_name : mod_unit.imports) {
            plan.depends_on_pages.push_back("module:" + import_name);
            plan.linked_pages.push_back("module:" + import_name);
        }
        if(auto r = builder.add_plan(std::move(plan)); !r) return r;
    }

    return {};
}

auto enumerate_file_pages(PlanBuilder& builder) -> std::expected<void, PlanError> {
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

        auto path_result = compute_page_path(identity);
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
        };

        // Per-symbol implementation prompts
        for(auto& sym_id : file_info.symbols) {
            if(auto* sym = lookup_sym(builder.model, sym_id)) {
                if(!is_page_level_symbol(builder.model, *sym)) {
                    continue;
                }
                if(is_type_kind(sym->kind)) {
                    plan.prompt_requests.push_back(
                        builder.make_symbol_prompt(PromptKind::TypeImplementationSummary,
                                                   make_symbol_target_key(*sym)));
                } else if(is_function_kind(sym->kind)) {
                    plan.prompt_requests.push_back(
                        builder.make_symbol_prompt(PromptKind::FunctionImplementationSummary,
                                                   make_symbol_target_key(*sym)));
                }
            }
        }

        if(auto r = builder.add_plan(std::move(plan)); !r) return r;
    }

    return {};
}

auto enumerate_index_page(PlanBuilder& builder) -> std::expected<void, PlanError> {
    PageIdentity identity{
        .page_type = PageType::Index,
        .normalized_owner_key = "index",
        .qualified_name = "index",
    };

    auto path_result = compute_page_path(identity);
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
    };

    plan.prompt_requests.push_back(builder.make_page_prompt(PromptKind::IndexOverview));

    // Index depends on all content pages so the overview can use dependency
    // summaries and links with complete context.
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

    // 1. Enumerate declaration and implementation pages before derived views.
    //
    // Mutually exclusive output:
    // - Header-based projects emit per-file implementation pages.
    // - Module-based projects emit per-module implementation pages.
    const bool emit_modules = builder.model.uses_modules;
    if(emit_modules) {
        if(auto r = enumerate_module_pages(builder); !r) {
            return std::unexpected(PlanError{.message = r.error().message});
        }
        logging::info("planner: {} module pages", builder.plans.size());
    } else {
        if(auto r = enumerate_file_pages(builder); !r) {
            return std::unexpected(PlanError{.message = r.error().message});
        }
        logging::info("planner: {} file pages", builder.plans.size());
    }

    auto impl_count = builder.plans.size();
    if(auto r = enumerate_namespace_pages(builder); !r) {
        return std::unexpected(PlanError{.message = r.error().message});
    }
    logging::info("planner: {} namespace pages", builder.plans.size() - impl_count);

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
