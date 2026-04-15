module;

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

export module generate:render;

import :markdown;
import :model;
import config;
import extract;
import support;

export namespace clore::generate {

struct RenderError {
    std::string message;
};

auto build_link_resolver(const PagePlanSet& plan_set) -> LinkResolver;
auto build_link_resolver(const PagePlanSet& plan_set, const extract::ProjectModel& model)
    -> LinkResolver;

auto normalize_prompt_output(std::string_view content) -> std::string;

auto render_page_markdown(const PagePlan& plan,
                          const config::TaskConfig& config,
                          const extract::ProjectModel& model,
                          const std::unordered_map<std::string, std::string>& prompt_outputs,
                          const LinkResolver& links) -> std::expected<std::string, RenderError>;

auto render_page_bundle(const PagePlan& plan,
                        const config::TaskConfig& config,
                        const extract::ProjectModel& model,
                        const std::unordered_map<std::string, std::string>& prompt_outputs,
                        const LinkResolver& links)
    -> std::expected<std::vector<GeneratedPage>, RenderError>;

auto validate_output(const std::string& content) -> std::expected<void, RenderError>;

auto write_page(const GeneratedPage& page, std::string_view output_root)
    -> std::expected<void, RenderError>;

}  // namespace clore::generate

namespace clore::generate {

namespace {

struct LinkTarget {
    std::string label;
    std::string target;
};

enum class SymbolDocView : std::uint8_t {
    Declaration,
    Implementation,
    Details,
};

struct SymbolDocPlan {
    const extract::SymbolInfo* symbol = nullptr;
    std::string index_path;
    std::string detail_path;
    std::vector<SymbolDocPlan> children;
};

struct PageDocLayout {
    std::vector<SymbolDocPlan> type_docs;
    std::vector<SymbolDocPlan> variable_docs;
    std::vector<SymbolDocPlan> function_docs;
    std::unordered_map<std::string, std::string> index_paths;
    std::unordered_map<std::string, std::string> detail_paths;
};

template <typename Visitor>
auto for_each_symbol_doc_group(const PageDocLayout& layout, Visitor&& visitor) -> void {
    visitor(layout.type_docs);
    visitor(layout.variable_docs);
    visitor(layout.function_docs);
}

auto trim_ascii(std::string_view text) -> std::string_view {
    while(!text.empty() && std::isspace(static_cast<unsigned char>(text.front())) != 0) {
        text.remove_prefix(1);
    }
    while(!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
        text.remove_suffix(1);
    }
    return text;
}

auto page_supports_symbol_subpages(const PagePlan& plan) -> bool {
    return plan.page_type == PageType::Namespace || plan.page_type == PageType::Module;
}

auto page_directory_of(std::string_view relative_path) -> std::string {
    namespace fs = std::filesystem;
    auto path = fs::path(relative_path).lexically_normal();
    if(path.filename() == "index.md") {
        return path.parent_path().generic_string();
    }
    return path.parent_path().generic_string();
}

auto join_relative_paths(std::string_view base,
                         std::string_view first,
                         std::string_view second = {}) -> std::string {
    namespace fs = std::filesystem;
    auto result = fs::path(base);
    if(!first.empty()) {
        result /= fs::path(first);
    }
    if(!second.empty()) {
        result /= fs::path(second);
    }
    return result.lexically_normal().generic_string();
}

auto sanitize_doc_slug(std::string_view raw) -> std::string {
    std::string slug;
    slug.reserve(raw.size());
    bool prev_dash = false;
    for(auto ch: raw) {
        auto uc = static_cast<unsigned char>(ch);
        if(std::isalnum(uc) != 0) {
            slug.push_back(static_cast<char>(std::tolower(uc)));
            prev_dash = false;
            continue;
        }
        if(!prev_dash) {
            slug.push_back('-');
            prev_dash = true;
        }
    }
    while(!slug.empty() && slug.front() == '-')
        slug.erase(slug.begin());
    while(!slug.empty() && slug.back() == '-')
        slug.pop_back();
    if(slug.empty()) {
        slug = "unnamed";
    }
    return slug;
}

auto strip_inline_markdown(std::string_view text) -> std::string {
    std::string out;
    out.reserve(text.size());
    for(auto ch: text) {
        if(ch == '`' || ch == '*' || ch == '_' || ch == '[' || ch == ']' || ch == '#') {
            continue;
        }
        out.push_back(ch);
    }
    return std::string(trim_ascii(out));
}

auto extract_first_plain_paragraph(std::string_view markdown) -> std::string {
    std::istringstream stream{std::string(markdown)};
    std::string line;
    bool in_code_block = false;
    std::string paragraph;

    while(std::getline(stream, line)) {
        auto trimmed = trim_ascii(line);
        if(trimmed.starts_with("```")) {
            in_code_block = !in_code_block;
            continue;
        }
        if(in_code_block) {
            continue;
        }
        if(trimmed.empty()) {
            if(!paragraph.empty()) {
                break;
            }
            continue;
        }
        if(trimmed.starts_with("#") || trimmed.starts_with(">") || trimmed.starts_with("|") ||
           trimmed.starts_with("- ") || trimmed.starts_with("* ")) {
            if(!paragraph.empty()) {
                break;
            }
            continue;
        }

        if(!paragraph.empty()) {
            paragraph.push_back(' ');
        }
        paragraph += trimmed;
    }

    return strip_inline_markdown(paragraph);
}

auto normalize_frontmatter_title(std::string_view page_title) -> std::string {
    auto plain = strip_inline_markdown(page_title);
    if(!plain.empty()) {
        return plain;
    }
    return std::string(trim_ascii(page_title));
}

auto short_name_of(std::string_view qualified_name) -> std::string {
    auto parts = extract::split_top_level_qualified_name(qualified_name);
    if(parts.empty()) {
        return {};
    }
    return parts.back();
}

auto namespace_of(std::string_view qualified_name) -> std::string {
    return extract::namespace_prefix_from_qualified_name(qualified_name);
}

auto make_relative_link_target(std::string_view current_page_path,
                               std::string_view target_page_path) -> std::string {
    namespace fs = std::filesystem;

    auto current = fs::path(current_page_path).lexically_normal();
    auto target = fs::path(target_page_path).lexically_normal();
    auto base = current.has_parent_path() ? current.parent_path() : fs::path{"."};
    auto rel = target.lexically_relative(base);
    if(rel.empty()) {
        return target.generic_string();
    }
    return rel.generic_string();
}

auto escape_mermaid_label(std::string_view text) -> std::string {
    std::string escaped;
    escaped.reserve(text.size());
    for(auto ch: text) {
        switch(ch) {
            case '\\': escaped += "\\\\"; break;
            case '"': escaped += "\\\""; break;
            case '\n':
            case '\r': escaped += ' '; break;
            default: escaped.push_back(ch); break;
        }
    }
    return escaped;
}

auto prompt_output_of(const std::unordered_map<std::string, std::string>& outputs,
                      PromptKind kind,
                      std::string_view target_key = {}) -> const std::string* {
    PromptRequest request{
        .kind = kind,
        .target_key = std::string(target_key),
    };
    auto it = outputs.find(prompt_request_key(request));
    return it != outputs.end() ? &it->second : nullptr;
}

auto normalize_prompt_outputs(const std::unordered_map<std::string, std::string>& outputs)
    -> std::unordered_map<std::string, std::string> {
    std::unordered_map<std::string, std::string> normalized;
    normalized.reserve(outputs.size());
    for(const auto& [key, value]: outputs) {
        normalized.emplace(key, normalize_prompt_output(value));
    }
    return normalized;
}

auto make_link_target(std::string_view current_page_path,
                      std::string label,
                      std::string_view target_page_path) -> LinkTarget {
    return LinkTarget{
        .label = std::move(label),
        .target = make_relative_link_target(current_page_path, target_page_path),
    };
}

auto push_link_paragraph(std::vector<MarkdownNode>& nodes,
                         std::string label,
                         const std::vector<LinkTarget>& targets) -> void {
    if(targets.empty()) {
        return;
    }
    Paragraph paragraph;
    paragraph.fragments.push_back(make_text(std::move(label)));
    for(std::size_t i = 0; i < targets.size(); ++i) {
        if(i > 0) {
            paragraph.fragments.push_back(make_text(" | "));
        }
        paragraph.fragments.push_back(make_link(targets[i].label, targets[i].target));
    }
    nodes.push_back(MarkdownNode{std::move(paragraph)});
}

auto push_optional_link_paragraph(std::vector<MarkdownNode>& nodes,
                                  std::string label,
                                  const std::optional<LinkTarget>& target) -> void {
    if(!target.has_value()) {
        return;
    }
    Paragraph paragraph;
    paragraph.fragments.push_back(make_text(std::move(label)));
    paragraph.fragments.push_back(make_link(target->label, target->target));
    nodes.push_back(MarkdownNode{std::move(paragraph)});
}

auto is_variable_kind(extract::SymbolKind kind) -> bool {
    return kind == extract::SymbolKind::Variable || kind == extract::SymbolKind::EnumMember;
}

auto symbol_doc_group(const extract::SymbolInfo& sym) -> std::string_view {
    if(is_type_kind(sym.kind)) {
        return "types";
    }
    if(is_function_kind(sym.kind)) {
        return "functions";
    }
    return "variables";
}

auto detail_view_for(const PagePlan& plan, const extract::SymbolInfo& sym) -> SymbolDocView {
    switch(plan.page_type) {
        case PageType::Namespace: return SymbolDocView::Declaration;
        case PageType::Module:
            return is_variable_kind(sym.kind) ? SymbolDocView::Details
                                              : SymbolDocView::Implementation;
        default: return SymbolDocView::Details;
    }
}

auto collect_documentable_children(const extract::ProjectModel& model,
                                   const extract::SymbolInfo& sym)
    -> std::vector<const extract::SymbolInfo*> {
    std::vector<const extract::SymbolInfo*> children;
    children.reserve(sym.children.size());
    for(auto child_id: sym.children) {
        auto* child = lookup_sym(model, child_id);
        if(child == nullptr) {
            continue;
        }
        if(!is_type_kind(child->kind) && !is_function_kind(child->kind) &&
           !is_variable_kind(child->kind)) {
            continue;
        }
        children.push_back(child);
    }

    std::sort(children.begin(),
              children.end(),
              [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  if(lhs->qualified_name != rhs->qualified_name) {
                      return lhs->qualified_name < rhs->qualified_name;
                  }
                  return lhs->name < rhs->name;
              });
    return children;
}

auto should_render_symbol_subpage(const extract::ProjectModel& model,
                                  const extract::SymbolInfo& sym) -> bool {
    if(!is_type_kind(sym.kind)) {
        return false;
    }

    auto children = collect_documentable_children(model, sym);
    auto has_member_type = std::ranges::any_of(children, [](const extract::SymbolInfo* child) {
        return child != nullptr && is_type_kind(child->kind);
    });
    auto has_member_variable = std::ranges::any_of(children, [](const extract::SymbolInfo* child) {
        return child != nullptr && is_variable_kind(child->kind);
    });
    auto has_member_function = std::ranges::any_of(children, [](const extract::SymbolInfo* child) {
        return child != nullptr && is_function_kind(child->kind);
    });

    auto has_type_axis = has_member_type || sym.is_template;
    return has_type_axis && has_member_variable && has_member_function;
}

auto default_symbol_slug(const extract::SymbolInfo& sym) -> std::string {
    auto base = !sym.name.empty() ? sym.name : short_name_of(sym.qualified_name);
    if(base.empty()) {
        base = std::string(extract::symbol_kind_name(sym.kind));
    }
    return sanitize_doc_slug(base);
}

template <typename Predicate>
auto collect_namespace_symbols(const extract::ProjectModel& model,
                               std::string_view namespace_name,
                               Predicate&& predicate) -> std::vector<const extract::SymbolInfo*> {
    std::vector<const extract::SymbolInfo*> symbols;
    auto ns_it = model.namespaces.find(std::string(namespace_name));
    if(ns_it == model.namespaces.end()) {
        return symbols;
    }
    for(auto sym_id: ns_it->second.symbols) {
        if(auto* sym = lookup_sym(model, sym_id);
           sym != nullptr && is_page_level_symbol(model, *sym) && predicate(*sym)) {
            symbols.push_back(sym);
        }
    }
    std::sort(symbols.begin(),
              symbols.end(),
              [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  return lhs->qualified_name < rhs->qualified_name;
              });
    return symbols;
}

template <typename Predicate>
auto collect_implementation_symbols(const PagePlan& plan,
                                    const extract::ProjectModel& model,
                                    Predicate&& predicate)
    -> std::vector<const extract::SymbolInfo*> {
    std::vector<const extract::SymbolInfo*> symbols;
    std::unordered_set<extract::SymbolID> seen;

    for(const auto& key: plan.owner_keys) {
        if(plan.page_type == PageType::Module) {
            for(auto* module : extract::find_modules_by_name(model, key)) {
                for(auto sym_id: module->symbols) {
                    if(!seen.insert(sym_id).second) {
                        continue;
                    }
                    if(auto* sym = lookup_sym(model, sym_id);
                       sym != nullptr && is_page_level_symbol(model, *sym) && predicate(*sym)) {
                        symbols.push_back(sym);
                    }
                }
            }
            continue;
        }

        auto file_it = model.files.find(key);
        if(file_it == model.files.end()) {
            continue;
        }
        for(auto sym_id: file_it->second.symbols) {
            if(!seen.insert(sym_id).second) {
                continue;
            }
            if(auto* sym = lookup_sym(model, sym_id);
               sym != nullptr && is_page_level_symbol(model, *sym) && predicate(*sym)) {
                symbols.push_back(sym);
            }
        }
    }

    std::sort(symbols.begin(),
              symbols.end(),
              [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  return lhs->qualified_name < rhs->qualified_name;
              });
    return symbols;
}

auto prompt_kind_for_symbol(const PagePlan& plan, const extract::SymbolInfo& sym)
    -> std::optional<PromptKind> {
    switch(plan.page_type) {
        case PageType::Namespace:
            if(is_type_kind(sym.kind)) {
                return PromptKind::TypeDeclarationSummary;
            }
            if(is_function_kind(sym.kind)) {
                return PromptKind::FunctionDeclarationSummary;
            }
            break;
        case PageType::Module:
        case PageType::File:
            if(is_type_kind(sym.kind)) {
                return PromptKind::TypeImplementationSummary;
            }
            if(is_function_kind(sym.kind)) {
                return PromptKind::FunctionImplementationSummary;
            }
            break;
        case PageType::Index: break;
    }
    return std::nullopt;
}

auto build_symbol_doc_plans(const PagePlan& plan,
                            const extract::ProjectModel& model,
                            const std::vector<const extract::SymbolInfo*>& symbols,
                            std::string_view base_dir) -> std::vector<SymbolDocPlan> {
    std::unordered_map<std::string, std::size_t> slug_counts;
    std::vector<SymbolDocPlan> plans;
    plans.reserve(symbols.size());

    for(const auto* sym: symbols) {
        if(sym == nullptr || !should_render_symbol_subpage(model, *sym)) {
            continue;
        }

        auto base_slug = default_symbol_slug(*sym);
        auto slug_key = std::string(symbol_doc_group(*sym)) + "/" + base_slug;
        auto& count = slug_counts[slug_key];
        ++count;

        auto slug = count == 1 ? base_slug : std::format("{}-{}", base_slug, count);
        auto group_dir = join_relative_paths(base_dir, symbol_doc_group(*sym));

        SymbolDocPlan doc{
            .symbol = sym,
            .index_path = join_relative_paths(group_dir, slug + ".md"),
            .detail_path = {},
            .children = {},
        };
        plans.push_back(std::move(doc));
    }

    return plans;
}

auto register_symbol_doc_plan(PageDocLayout& layout, const SymbolDocPlan& plan) -> void {
    if(plan.symbol == nullptr || plan.symbol->qualified_name.empty()) {
        return;
    }

    layout.index_paths[plan.symbol->qualified_name] = plan.index_path;
    if(!plan.detail_path.empty()) {
        layout.detail_paths[plan.symbol->qualified_name] = plan.detail_path;
    }
    for(const auto& child: plan.children) {
        register_symbol_doc_plan(layout, child);
    }
}

auto build_page_doc_layout(const PagePlan& plan, const extract::ProjectModel& model)
    -> PageDocLayout {
    PageDocLayout layout;
    if(!page_supports_symbol_subpages(plan)) {
        return layout;
    }

    auto base_dir = page_directory_of(plan.relative_path);
    if(base_dir.empty()) {
        return layout;
    }

    auto collect_page_symbols = [&](auto&& predicate) -> std::vector<const extract::SymbolInfo*> {
        if(plan.page_type == PageType::Namespace) {
            return collect_namespace_symbols(model, plan.owner_keys.front(), predicate);
        }
        return collect_implementation_symbols(plan, model, predicate);
    };

    auto assign_group = [&](auto& group, auto&& predicate) {
        group = build_symbol_doc_plans(plan, model, collect_page_symbols(predicate), base_dir);
    };

    assign_group(layout.type_docs, [](const extract::SymbolInfo& sym) {
        return is_type_kind(sym.kind);
    });
    assign_group(layout.variable_docs, [](const extract::SymbolInfo& sym) {
        return is_variable_kind(sym.kind);
    });
    assign_group(layout.function_docs, [](const extract::SymbolInfo& sym) {
        return is_function_kind(sym.kind);
    });

    for_each_symbol_doc_group(layout, [&](const std::vector<SymbolDocPlan>& group) {
        for(const auto& plan_entry: group) {
            register_symbol_doc_plan(layout, plan_entry);
        }
    });

    return layout;
}

auto build_symbol_link_list(const std::vector<const extract::SymbolInfo*>& symbols,
                            std::string_view current_page_path,
                            const LinkResolver& links,
                            bool use_full_name = true) -> BulletList {
    BulletList list;
    for(const auto* sym: symbols) {
        ListItem item;
        item.fragments.push_back(
            make_text(std::string(extract::symbol_kind_name(sym->kind)) + " "));
        auto label = use_full_name ? sym->qualified_name : short_name_of(sym->qualified_name);
        if(auto* target_path = links.resolve(sym->qualified_name)) {
            item.fragments.push_back(
                make_link(label.empty() ? sym->qualified_name : label,
                          make_relative_link_target(current_page_path, *target_path),
                          true));
        } else {
            item.fragments.push_back(make_text(label.empty() ? sym->qualified_name : label));
        }
        list.items.push_back(std::move(item));
    }
    return list;
}

auto build_symbol_source_locations(const extract::SymbolInfo& sym, const config::TaskConfig& config)
    -> BulletList {
    BulletList list;
    if(sym.declaration_location.is_known()) {
        ListItem item;
        item.fragments.push_back(make_text("Declared at: "));
        item.fragments.push_back(
            make_code(make_source_relative(sym.declaration_location.file, config.project_root) +
                      ":" + std::to_string(sym.declaration_location.line)));
        list.items.push_back(std::move(item));
    }
    if(sym.definition_location.has_value() && sym.definition_location->is_known()) {
        ListItem item;
        item.fragments.push_back(make_text("Defined at: "));
        item.fragments.push_back(
            make_code(make_source_relative(sym.definition_location->file, config.project_root) +
                      ":" + std::to_string(sym.definition_location->line)));
        list.items.push_back(std::move(item));
    }
    return list;
}

template <typename Predicate>
auto build_member_list(const extract::ProjectModel& model,
                       const extract::SymbolInfo& sym,
                       std::string_view current_page_path,
                       const LinkResolver& links,
                       Predicate&& predicate) -> BulletList {
    std::vector<const extract::SymbolInfo*> members;
    members.reserve(sym.children.size());

    BulletList list;
    for(auto child_id: sym.children) {
        auto* child = lookup_sym(model, child_id);
        if(child == nullptr) {
            continue;
        }
        if(!predicate(*child)) {
            continue;
        }
        members.push_back(child);
    }

    std::sort(members.begin(),
              members.end(),
              [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  auto lhs_name = lhs->qualified_name.empty() ? lhs->name : lhs->qualified_name;
                  auto rhs_name = rhs->qualified_name.empty() ? rhs->name : rhs->qualified_name;
                  return lhs_name < rhs_name;
              });

    for(const auto* child: members) {
        ListItem item;
        auto access = child->access.empty() ? std::string{"public"} : child->access;
        item.fragments.push_back(
            make_text(access + " " + std::string(extract::symbol_kind_name(child->kind)) + " "));
        auto label = child->name.empty() ? short_name_of(child->qualified_name) : child->name;
        if(label.empty()) {
            label = child->qualified_name;
        }
        if(auto* target_path = links.resolve(child->qualified_name);
           target_path != nullptr && *target_path != current_page_path) {
            item.fragments.push_back(
                make_link(label, make_relative_link_target(current_page_path, *target_path), true));
        } else {
            item.fragments.push_back(make_code(std::move(label)));
        }
        list.items.push_back(std::move(item));
    }
    return list;
}

auto find_implementation_pages(const extract::SymbolInfo& sym,
                               const extract::ProjectModel& model,
                               const LinkResolver& links,
                               std::string_view current_page_path,
                               const std::string& project_root) -> std::vector<LinkTarget> {
    std::vector<LinkTarget> results;
    std::unordered_set<std::string> seen;

    auto try_add = [&](const std::string& file_path) {
        if(file_path.empty()) {
            return;
        }
        if(auto* mod = extract::find_module_by_source(model, file_path)) {
            if(auto* target_path = links.resolve(mod->name)) {
                if(seen.insert(*target_path).second) {
                    results.push_back(
                        make_link_target(current_page_path, "Module " + mod->name, *target_path));
                }
            }
            return;
        }
        if(auto* target_path = links.resolve(file_path)) {
            if(seen.insert(*target_path).second) {
                results.push_back(make_link_target(current_page_path,
                                                   make_source_relative(file_path, project_root),
                                                   *target_path));
            }
        }
    };

    if(sym.definition_location.has_value()) {
        try_add(sym.definition_location->file);
    }
    try_add(sym.declaration_location.file);
    return results;
}

auto find_declaration_page(const extract::SymbolInfo& sym,
                           const LinkResolver& links,
                           std::string_view current_page_path) -> std::optional<LinkTarget> {
    if(auto* target_path = links.resolve(sym.qualified_name);
       target_path != nullptr && *target_path != current_page_path) {
        auto ns_name = !sym.enclosing_namespace.empty() ? sym.enclosing_namespace
                                                        : namespace_of(sym.qualified_name);
        auto label =
            ns_name.empty() ? std::string{"Declaration page"} : std::string{"Namespace "} + ns_name;
        return make_link_target(current_page_path, std::move(label), *target_path);
    }

    auto ns_name = !sym.enclosing_namespace.empty() ? sym.enclosing_namespace
                                                    : namespace_of(sym.qualified_name);
    if(ns_name.empty()) {
        return std::nullopt;
    }
    if(auto* target_path = links.resolve(ns_name)) {
        return make_link_target(current_page_path, "Namespace " + ns_name, *target_path);
    }
    return std::nullopt;
}

auto find_module_for_file(const extract::ProjectModel& model, std::string_view file_path)
    -> std::optional<std::string> {
    if(auto* mod = extract::find_module_by_source(model, file_path)) {
        return mod->name;
    }
    return std::nullopt;
}

auto build_related_page_targets(const PagePlan& plan,
                                const LinkResolver& links,
                                std::string_view current_page_path) -> std::vector<LinkTarget> {
    std::vector<LinkTarget> targets;
    std::unordered_set<std::string> seen;
    for(const auto& linked: plan.linked_pages) {
        auto colon = linked.find(':');
        auto entity_name = colon == std::string::npos ? linked : linked.substr(colon + 1);
        auto* target_path = links.resolve(entity_name);
        if(target_path == nullptr || !seen.insert(*target_path).second) {
            continue;
        }
        auto* title = links.resolve_page_title(linked);
        auto label = title != nullptr && !title->empty() ? strip_inline_markdown(*title)
                                                         : strip_inline_markdown(entity_name);
        if(label.empty()) {
            label = entity_name;
        }
        targets.push_back(make_link_target(current_page_path, std::move(label), *target_path));
    }
    return targets;
}

auto build_prompt_section(std::string heading, std::uint8_t level, const std::string* output)
    -> SemanticSectionPtr {
    auto section = make_section(SemanticKind::Section, {}, std::move(heading), level);
    if(output != nullptr && !trim_ascii(*output).empty()) {
        section->children.push_back(make_raw_markdown(*output));
    }
    return section;
}

auto build_list_section(std::string heading, std::uint8_t level, BulletList list)
    -> SemanticSectionPtr {
    auto section = make_section(SemanticKind::Section, {}, std::move(heading), level);
    if(!list.items.empty()) {
        section->children.push_back(MarkdownNode{std::move(list)});
    }
    return section;
}

constexpr std::size_t kMermaidMinNodes = 4;
constexpr std::size_t kMermaidMinEdges = 3;

auto should_emit_mermaid(std::size_t node_count, std::size_t edge_count) -> bool {
    return node_count >= kMermaidMinNodes || edge_count >= kMermaidMinEdges;
}

auto append_type_structure_sections(std::vector<MarkdownNode>& nodes,
                                    const extract::ProjectModel& model,
                                    const extract::SymbolInfo& sym,
                                    std::string_view current_page_path,
                                    const LinkResolver& links,
                                    std::uint8_t level) -> void {
    std::vector<const extract::SymbolInfo*> base_symbols;
    for(auto base_id: sym.bases) {
        if(auto* base = lookup_sym(model, base_id)) {
            base_symbols.push_back(base);
        }
    }
    std::sort(base_symbols.begin(),
              base_symbols.end(),
              [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  return lhs->qualified_name < rhs->qualified_name;
              });
    nodes.push_back(MarkdownNode{
        build_list_section("Base Types",
                           level,
                           build_symbol_link_list(base_symbols, current_page_path, links, true))});

    std::vector<const extract::SymbolInfo*> derived_symbols;
    for(auto derived_id: sym.derived) {
        if(auto* derived = lookup_sym(model, derived_id)) {
            derived_symbols.push_back(derived);
        }
    }
    std::sort(derived_symbols.begin(),
              derived_symbols.end(),
              [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  return lhs->qualified_name < rhs->qualified_name;
              });

    auto collect_member_symbols = [&](auto&& predicate) -> std::vector<const extract::SymbolInfo*> {
        std::vector<const extract::SymbolInfo*> members;
        members.reserve(sym.children.size());
        for(auto child_id: sym.children) {
            if(auto* child = lookup_sym(model, child_id); child != nullptr && predicate(*child)) {
                members.push_back(child);
            }
        }
        std::sort(members.begin(),
                  members.end(),
                  [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                      return lhs->qualified_name < rhs->qualified_name;
                  });
        return members;
    };

    auto member_type_symbols = collect_member_symbols(
        [](const extract::SymbolInfo& child) { return is_type_kind(child.kind); });
    auto member_variable_symbols = collect_member_symbols(
        [](const extract::SymbolInfo& child) { return is_variable_kind(child.kind); });
    auto member_function_symbols = collect_member_symbols(
        [](const extract::SymbolInfo& child) { return is_function_kind(child.kind); });

    auto edge_count = base_symbols.size() + derived_symbols.size() + member_type_symbols.size() +
                      member_variable_symbols.size() + member_function_symbols.size();
    auto node_count = 1 + edge_count;
    if(should_emit_mermaid(node_count, edge_count)) {
        auto type_label = short_name_of(sym.qualified_name);
        if(type_label.empty()) {
            type_label = sym.name.empty() ? sym.qualified_name : sym.name;
        }

        std::string diagram = "graph TD\n";
        diagram += "    SELF[\"" + escape_mermaid_label(type_label) + "\"]\n";

        auto append_nodes = [&](std::string_view prefix,
                                const std::vector<const extract::SymbolInfo*>& symbols,
                                std::string_view edge_op,
                                std::string_view marker,
                                bool call_suffix) {
            for(std::size_t i = 0; i < symbols.size(); ++i) {
                auto id = std::string(prefix) + std::to_string(i);
                auto label = short_name_of(symbols[i]->qualified_name);
                if(label.empty()) {
                    label = symbols[i]->name.empty() ? symbols[i]->qualified_name : symbols[i]->name;
                }
                auto rendered = std::string(marker) + label + (call_suffix ? "()" : "");
                diagram += "    " + id + "[\"" + escape_mermaid_label(rendered) + "\"]\n";
                if(edge_op == "-->self") {
                    diagram += "    " + id + " --> SELF\n";
                } else if(edge_op == "self-->") {
                    diagram += "    SELF --> " + id + "\n";
                }
            }
        };

        append_nodes("B", base_symbols, "-->self", "base: ", false);
        append_nodes("D", derived_symbols, "self-->", "derived: ", false);
        append_nodes("MT", member_type_symbols, "self-->", "type: ", false);
        append_nodes("MV", member_variable_symbols, "self-->", "var: ", false);
        append_nodes("MF", member_function_symbols, "self-->", "fn: ", true);

        auto section = make_section(SemanticKind::Section, {}, "Structure Diagram", level);
        section->children.push_back(make_mermaid(std::move(diagram)));
        nodes.push_back(MarkdownNode{section});
    }

    nodes.push_back(MarkdownNode{build_list_section(
        "Derived Types",
        level,
        build_symbol_link_list(derived_symbols, current_page_path, links, true))});

    nodes.push_back(
        MarkdownNode{build_list_section("Member Types",
                                        level,
                                        build_member_list(model,
                                                          sym,
                                                          current_page_path,
                                                          links,
                                                          [](const extract::SymbolInfo& child) {
                                                              return is_type_kind(child.kind);
                                                          }))});
    nodes.push_back(
        MarkdownNode{build_list_section("Member Variables",
                                        level,
                                        build_member_list(model,
                                                          sym,
                                                          current_page_path,
                                                          links,
                                                          [](const extract::SymbolInfo& child) {
                                                              return is_variable_kind(child.kind);
                                                          }))});
    nodes.push_back(
        MarkdownNode{build_list_section("Member Functions",
                                        level,
                                        build_member_list(model,
                                                          sym,
                                                          current_page_path,
                                                          links,
                                                          [](const extract::SymbolInfo& child) {
                                                              return is_function_kind(child.kind);
                                                          }))});
}

auto add_prompt_output(std::vector<MarkdownNode>& nodes, const std::string* output) -> void {
    if(output != nullptr && !trim_ascii(*output).empty()) {
        nodes.push_back(make_raw_markdown(*output));
    }
}

auto render_import_diagram_code(const extract::ModuleUnit& mod_unit) -> std::string {
    if(mod_unit.imports.empty()) {
        return {};
    }
    auto top_module = [](std::string_view name) -> std::string {
        auto colon = name.find(':');
        return std::string(colon == std::string_view::npos ? name : name.substr(0, colon));
    };

    auto module_label = top_module(mod_unit.name);
    std::unordered_set<std::string> seen;
    std::vector<std::string> imports;
    for(const auto& imp: mod_unit.imports) {
        auto label = top_module(imp);
        if(label == module_label || !seen.insert(label).second) {
            continue;
        }
        imports.push_back(label);
    }

    auto edge_count = imports.size();
    auto node_count = 1 + imports.size();
    if(!should_emit_mermaid(node_count, edge_count)) {
        return {};
    }

    std::sort(imports.begin(), imports.end());

    std::string result = "graph LR\n";
    result += "    M0[\"" + escape_mermaid_label(module_label) + "\"]\n";
    for(std::size_t i = 0; i < imports.size(); ++i) {
        auto node_id = "I" + std::to_string(i);
        result += "    " + node_id + "[\"" + escape_mermaid_label(imports[i]) + "\"]\n";
        result += "    " + node_id + " --> M0\n";
    }
    return result;
}

auto render_namespace_diagram_code(const extract::ProjectModel& model,
                                   std::string_view namespace_name) -> std::string {
    auto ns_it = model.namespaces.find(std::string(namespace_name));
    if(ns_it == model.namespaces.end()) {
        return {};
    }
    std::vector<std::string> type_names;
    for(auto sym_id: ns_it->second.symbols) {
        if(auto* sym = lookup_sym(model, sym_id); sym != nullptr && is_type_kind(sym->kind)) {
            type_names.push_back(short_name_of(sym->qualified_name));
        }
    }
    std::sort(type_names.begin(), type_names.end());
    type_names.erase(std::unique(type_names.begin(), type_names.end()), type_names.end());

    std::vector<std::string> children;
    for(const auto& child: ns_it->second.children) {
        if(!child.contains("(anonymous namespace)")) {
            children.push_back(short_name_of(child));
        }
    }
    std::sort(children.begin(), children.end());
    children.erase(std::unique(children.begin(), children.end()), children.end());

    auto edge_count = type_names.size() + children.size();
    auto node_count = 1 + edge_count;
    if(!should_emit_mermaid(node_count, edge_count)) {
        return {};
    }

    std::string result = "graph TD\n";
    result += "    NS[\"" + escape_mermaid_label(short_name_of(namespace_name)) + "\"]\n";
    for(std::size_t i = 0; i < type_names.size(); ++i) {
        auto node_id = "T" + std::to_string(i);
        result += "    " + node_id + "[\"" + escape_mermaid_label(type_names[i]) + "\"]\n";
        result += "    NS --> " + node_id + "\n";
    }
    for(std::size_t child_index = 0; child_index < children.size(); ++child_index) {
        auto node_id = "NSC" + std::to_string(child_index);
        result += "    " + node_id + "[\"" + escape_mermaid_label(children[child_index]) + "\"]\n";
        result += "    NS --> " + node_id + "\n";
    }
    return result;
}

auto render_file_dependency_diagram_code(const PagePlan& plan,
                                         const config::TaskConfig& config,
                                         const extract::ProjectModel& model) -> std::string {
    if(plan.owner_keys.empty()) {
        return {};
    }
    auto file_it = model.files.find(plan.owner_keys.front());
    if(file_it == model.files.end()) {
        return {};
    }

    std::vector<std::string> include_labels;
    include_labels.reserve(file_it->second.includes.size());
    for(const auto& include: file_it->second.includes) {
        auto label = make_source_relative(include, config.project_root);
        if(label.empty()) {
            label = include;
        }
        include_labels.push_back(std::move(label));
    }
    std::sort(include_labels.begin(), include_labels.end());
    include_labels.erase(std::unique(include_labels.begin(), include_labels.end()),
                         include_labels.end());

    auto symbols = collect_implementation_symbols(plan, model, [](const extract::SymbolInfo& sym) {
        return is_type_kind(sym.kind) || is_variable_kind(sym.kind) || is_function_kind(sym.kind);
    });

    auto edge_count = include_labels.size() + symbols.size();
    auto node_count = 1 + edge_count;
    if(!should_emit_mermaid(node_count, edge_count)) {
        return {};
    }

    auto file_label = make_source_relative(plan.owner_keys.front(), config.project_root);
    if(file_label.empty()) {
        file_label = plan.owner_keys.front();
    }

    std::string result = "graph LR\n";
    result += "    F[\"" + escape_mermaid_label(file_label) + "\"]\n";
    for(std::size_t i = 0; i < include_labels.size(); ++i) {
        auto node_id = "I" + std::to_string(i);
        result += "    " + node_id + "[\"" + escape_mermaid_label(include_labels[i]) + "\"]\n";
        result += "    " + node_id + " --> F\n";
    }
    for(std::size_t i = 0; i < symbols.size(); ++i) {
        auto node_id = "S" + std::to_string(i);
        auto symbol_label = short_name_of(symbols[i]->qualified_name);
        if(symbol_label.empty()) {
            symbol_label =
                symbols[i]->name.empty() ? symbols[i]->qualified_name : symbols[i]->name;
        }
        result += "    " + node_id + "[\"" + escape_mermaid_label(symbol_label) + "\"]\n";
        result += "    F --> " + node_id + "\n";
    }

    return result;
}

auto render_module_dependency_diagram_code(const extract::ProjectModel& model) -> std::string {
    auto top_module = [](std::string_view name) -> std::string {
        auto colon = name.find(':');
        return std::string(colon == std::string_view::npos ? name : name.substr(0, colon));
    };

    std::unordered_map<std::string, std::unordered_set<std::string>> deps;
    std::unordered_set<std::string> modules;
    for(const auto& [_, mod_unit]: model.modules) {
        if(!mod_unit.is_interface) {
            continue;
        }
        auto from = top_module(mod_unit.name);
        modules.insert(from);
        for(const auto& imp: mod_unit.imports) {
            auto to = top_module(imp);
            if(to != from) {
                deps[from].insert(to);
                modules.insert(to);
            }
        }
    }
    if(modules.size() < 2) {
        return {};
    }

    std::size_t edge_count = 0;
    for(const auto& [_, to_set]: deps) {
        edge_count += to_set.size();
    }
    if(!should_emit_mermaid(modules.size(), edge_count)) {
        return {};
    }

    std::vector<std::string> sorted(modules.begin(), modules.end());
    std::sort(sorted.begin(), sorted.end());
    std::unordered_map<std::string, std::string> node_ids;
    std::string result = "graph LR\n";
    for(std::size_t i = 0; i < sorted.size(); ++i) {
        auto node_id = std::format("M{}", i);
        node_ids.emplace(sorted[i], node_id);
        result += "    " + node_id + "[\"" + escape_mermaid_label(sorted[i]) + "\"]\n";
    }
    for(const auto& from: sorted) {
        auto dep_it = deps.find(from);
        if(dep_it == deps.end()) {
            continue;
        }
        std::vector<std::string> targets(dep_it->second.begin(), dep_it->second.end());
        std::sort(targets.begin(), targets.end());
        for(const auto& to: targets) {
            result += "    " + node_ids.at(to) + " --> " + node_ids.at(from) + "\n";
        }
    }
    return result;
}

auto symbol_semantic_kind(const extract::SymbolInfo& sym) -> SemanticKind {
    if(is_type_kind(sym.kind)) {
        return SemanticKind::Type;
    }
    if(is_function_kind(sym.kind)) {
        return SemanticKind::Function;
    }
    return SemanticKind::Variable;
}

auto doc_label(SymbolDocView view) -> std::string_view {
    switch(view) {
        case SymbolDocView::Declaration: return "Declaration";
        case SymbolDocView::Implementation: return "Implementation";
        case SymbolDocView::Details: return "Details";
    }
    return "Details";
}

auto find_doc_index_path(const PageDocLayout& layout, std::string_view qualified_name)
    -> const std::string* {
    auto it = layout.index_paths.find(std::string(qualified_name));
    return it != layout.index_paths.end() ? &it->second : nullptr;
}

auto find_doc_detail_path(const PageDocLayout& layout, std::string_view qualified_name)
    -> const std::string* {
    auto it = layout.detail_paths.find(std::string(qualified_name));
    return it != layout.detail_paths.end() ? &it->second : nullptr;
}

auto build_child_doc_list(const std::vector<SymbolDocPlan>& children,
                          std::string_view current_page_path) -> BulletList {
    BulletList list;
    for(const auto& child: children) {
        if(child.symbol == nullptr) {
            continue;
        }
        ListItem item;
        item.fragments.push_back(
            make_text(std::string(extract::symbol_kind_name(child.symbol->kind)) + " "));
        item.fragments.push_back(
            make_link(child.symbol->qualified_name,
                      make_relative_link_target(current_page_path, child.index_path),
                      true));
        list.items.push_back(std::move(item));
    }
    return list;
}

auto add_symbol_summary_content(std::vector<MarkdownNode>& nodes,
                                const extract::SymbolInfo& sym,
                                const std::string* output) -> void {
    if(output != nullptr && !trim_ascii(*output).empty()) {
        nodes.push_back(make_raw_markdown(*output));
        return;
    }

    if(!trim_ascii(sym.doc_comment).empty()) {
        nodes.push_back(make_paragraph(std::string(trim_ascii(sym.doc_comment))));
        return;
    }

    if(!sym.signature.empty()) {
        Paragraph paragraph;
        paragraph.fragments.push_back(make_text("Signature: "));
        paragraph.fragments.push_back(make_code(sym.signature));
        nodes.push_back(MarkdownNode{std::move(paragraph)});
    }
}

auto add_symbol_doc_links(std::vector<MarkdownNode>& nodes,
                          std::string_view current_page_path,
                          const PageDocLayout& layout,
                          const extract::SymbolInfo& sym,
                          SymbolDocView view) -> void {
    std::vector<LinkTarget> targets;
    if(auto* index_path = find_doc_index_path(layout, sym.qualified_name);
       index_path != nullptr && *index_path != current_page_path) {
        targets.push_back(make_link_target(current_page_path, "Overview", *index_path));
    }
    if(auto* detail_path = find_doc_detail_path(layout, sym.qualified_name);
       detail_path != nullptr && *detail_path != current_page_path) {
        targets.push_back(
            make_link_target(current_page_path, std::string(doc_label(view)), *detail_path));
    }
    push_link_paragraph(nodes, "Docs: ", targets);
}

auto push_owner_link(std::vector<MarkdownNode>& nodes,
                     std::string_view current_page_path,
                     const PagePlan& owner_plan) -> void {
    Paragraph paragraph;
    paragraph.fragments.push_back(make_text("Owner: "));
    paragraph.fragments.push_back(
        make_link(strip_inline_markdown(owner_plan.title),
                  make_relative_link_target(current_page_path, owner_plan.relative_path)));
    nodes.push_back(MarkdownNode{std::move(paragraph)});
}

auto collect_related_symbols(const extract::ProjectModel& model,
                             const std::vector<extract::SymbolID>& ids)
    -> std::vector<const extract::SymbolInfo*> {
    std::vector<const extract::SymbolInfo*> symbols;
    std::unordered_set<extract::SymbolID> seen;
    symbols.reserve(ids.size());
    for(auto id: ids) {
        if(!seen.insert(id).second) {
            continue;
        }
        if(auto* sym = lookup_sym(model, id)) {
            symbols.push_back(sym);
        }
    }
    std::sort(symbols.begin(),
              symbols.end(),
              [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  return lhs->qualified_name < rhs->qualified_name;
              });
    return symbols;
}

auto append_relation_section(std::vector<MarkdownNode>& nodes,
                             std::string heading,
                             std::uint8_t level,
                             const std::vector<extract::SymbolID>& ids,
                             const extract::ProjectModel& model,
                             std::string_view current_page_path,
                             const LinkResolver& links) -> void {
    auto symbols = collect_related_symbols(model, ids);
    nodes.push_back(MarkdownNode{
        build_list_section(std::move(heading),
                           level,
                           build_symbol_link_list(symbols, current_page_path, links, true))});
}

auto render_document_page(std::string relative_path,
                          Frontmatter frontmatter,
                          SemanticSectionPtr root) -> std::expected<GeneratedPage, RenderError> {
    MarkdownDocument document;
    document.frontmatter = std::move(frontmatter);
    document.children.push_back(MarkdownNode{std::move(root)});

    auto content = render_markdown(document);
    if(content.empty()) {
        return std::unexpected(RenderError{
            .message = std::format("rendered markdown is empty for '{}'", relative_path)});
    }

    return GeneratedPage{
        .relative_path = std::move(relative_path),
        .content = std::move(content),
    };
}

auto build_symbol_frontmatter(const extract::SymbolInfo& sym,
                              std::string_view title,
                              const std::string* output,
                              std::string_view fallback_description) -> Frontmatter {
    auto description = output != nullptr ? extract_first_plain_paragraph(*output) : std::string{};
    if(description.empty() && !trim_ascii(sym.doc_comment).empty()) {
        description = extract_first_plain_paragraph(sym.doc_comment);
    }
    if(description.empty() && !fallback_description.empty()) {
        description = std::string(fallback_description);
    }
    if(description.empty()) {
        description = normalize_frontmatter_title(title);
    }

    return Frontmatter{
        .title = normalize_frontmatter_title(title),
        .description = std::move(description),
    };
}

auto render_symbol_overview_page(const SymbolDocPlan& doc_plan,
                                 const PagePlan& owner_plan,
                                 const config::TaskConfig& config,
                                 const extract::ProjectModel& model,
                                 const std::unordered_map<std::string, std::string>& outputs,
                                 const LinkResolver& links,
                                 const PageDocLayout& layout)
    -> std::expected<GeneratedPage, RenderError> {
    if(doc_plan.symbol == nullptr) {
        return std::unexpected(RenderError{.message = "symbol overview page missing symbol"});
    }

    auto view = detail_view_for(owner_plan, *doc_plan.symbol);
    const std::string* output = nullptr;
    if(auto kind = prompt_kind_for_symbol(owner_plan, *doc_plan.symbol); kind.has_value()) {
        output = prompt_output_of(outputs, *kind, make_symbol_target_key(*doc_plan.symbol));
    }

    auto heading = "`" + doc_plan.symbol->qualified_name + "`";
    auto root = make_section(symbol_semantic_kind(*doc_plan.symbol),
                             doc_plan.symbol->qualified_name,
                             heading,
                             1,
                             false);
    push_owner_link(root->children, doc_plan.index_path, owner_plan);
    add_symbol_doc_links(root->children, doc_plan.index_path, layout, *doc_plan.symbol, view);

    auto locations = build_symbol_source_locations(*doc_plan.symbol, config);
    if(!locations.items.empty()) {
        root->children.push_back(MarkdownNode{std::move(locations)});
    }

    if(owner_plan.page_type != PageType::Namespace) {
        push_optional_link_paragraph(
            root->children,
            "Declaration: ",
            find_declaration_page(*doc_plan.symbol, links, doc_plan.index_path));
    }
    if(owner_plan.page_type == PageType::Namespace) {
        push_link_paragraph(root->children,
                            "Implementations: ",
                            find_implementation_pages(*doc_plan.symbol,
                                                      model,
                                                      links,
                                                      doc_plan.index_path,
                                                      config.project_root));
    }

    add_symbol_summary_content(root->children, *doc_plan.symbol, output);

    if(is_type_kind(doc_plan.symbol->kind)) {
        append_type_structure_sections(root->children,
                                       model,
                                       *doc_plan.symbol,
                                       doc_plan.index_path,
                                       links,
                                       2);
    }

    if(!doc_plan.children.empty()) {
        root->children.push_back(MarkdownNode{
            build_list_section("Nested Pages",
                               2,
                               build_child_doc_list(doc_plan.children, doc_plan.index_path))});
    }

    return render_document_page(
        doc_plan.index_path,
        build_symbol_frontmatter(*doc_plan.symbol,
                                 doc_plan.symbol->qualified_name,
                                 output,
                                 std::string(doc_label(view)) + " overview"),
        std::move(root));
}

auto render_symbol_detail_page(const SymbolDocPlan& doc_plan,
                               const PagePlan& owner_plan,
                               const config::TaskConfig& config,
                               const extract::ProjectModel& model,
                               const std::unordered_map<std::string, std::string>& outputs,
                               const LinkResolver& links,
                               const PageDocLayout& layout)
    -> std::expected<GeneratedPage, RenderError> {
    if(doc_plan.symbol == nullptr) {
        return std::unexpected(RenderError{.message = "symbol detail page missing symbol"});
    }

    auto view = detail_view_for(owner_plan, *doc_plan.symbol);
    const std::string* output = nullptr;
    if(auto kind = prompt_kind_for_symbol(owner_plan, *doc_plan.symbol); kind.has_value()) {
        output = prompt_output_of(outputs, *kind, make_symbol_target_key(*doc_plan.symbol));
    }

    auto heading = std::format("`{}` {}", doc_plan.symbol->qualified_name, doc_label(view));
    auto root = make_section(symbol_semantic_kind(*doc_plan.symbol),
                             doc_plan.symbol->qualified_name,
                             heading,
                             1,
                             false);
    push_owner_link(root->children, doc_plan.detail_path, owner_plan);
    add_symbol_doc_links(root->children, doc_plan.detail_path, layout, *doc_plan.symbol, view);

    auto locations = build_symbol_source_locations(*doc_plan.symbol, config);
    if(!locations.items.empty()) {
        root->children.push_back(MarkdownNode{std::move(locations)});
    }

    if(owner_plan.page_type != PageType::Namespace) {
        push_optional_link_paragraph(
            root->children,
            "Declaration: ",
            find_declaration_page(*doc_plan.symbol, links, doc_plan.detail_path));
    }
    if(owner_plan.page_type == PageType::Namespace) {
        push_link_paragraph(root->children,
                            "Implementations: ",
                            find_implementation_pages(*doc_plan.symbol,
                                                      model,
                                                      links,
                                                      doc_plan.detail_path,
                                                      config.project_root));
    }

    if(output != nullptr && !trim_ascii(*output).empty()) {
        root->children.push_back(
            MarkdownNode{build_prompt_section(std::string(doc_label(view)) + " Notes", 2, output)});
    } else {
        add_symbol_summary_content(root->children, *doc_plan.symbol, output);
    }

    if(is_type_kind(doc_plan.symbol->kind)) {
        append_type_structure_sections(root->children,
                                       model,
                                       *doc_plan.symbol,
                                       doc_plan.detail_path,
                                       links,
                                       2);
    } else if(is_function_kind(doc_plan.symbol->kind)) {
        append_relation_section(root->children,
                                "Calls",
                                2,
                                doc_plan.symbol->calls,
                                model,
                                doc_plan.detail_path,
                                links);
        append_relation_section(root->children,
                                "Called By",
                                2,
                                doc_plan.symbol->called_by,
                                model,
                                doc_plan.detail_path,
                                links);
    } else {
        append_relation_section(root->children,
                                "References",
                                2,
                                doc_plan.symbol->references,
                                model,
                                doc_plan.detail_path,
                                links);
        append_relation_section(root->children,
                                "Referenced By",
                                2,
                                doc_plan.symbol->referenced_by,
                                model,
                                doc_plan.detail_path,
                                links);
    }

    if(!doc_plan.children.empty()) {
        root->children.push_back(MarkdownNode{
            build_list_section("Nested Pages",
                               2,
                               build_child_doc_list(doc_plan.children, doc_plan.detail_path))});
    }

    return render_document_page(
        doc_plan.detail_path,
        build_symbol_frontmatter(
            *doc_plan.symbol,
            std::format("{} {}", doc_plan.symbol->qualified_name, doc_label(view)),
            output,
            std::string(doc_label(view)) + " details"),
        std::move(root));
}

auto append_symbol_doc_pages(std::vector<GeneratedPage>& pages,
                             const std::vector<SymbolDocPlan>& doc_plans,
                             const PagePlan& owner_plan,
                             const config::TaskConfig& config,
                             const extract::ProjectModel& model,
                             const std::unordered_map<std::string, std::string>& outputs,
                             const LinkResolver& links,
                             const PageDocLayout& layout) -> std::expected<void, RenderError> {
    for(const auto& doc_plan: doc_plans) {
        auto overview =
            render_symbol_overview_page(doc_plan, owner_plan, config, model, outputs, links, layout);
        if(!overview.has_value()) {
            return std::unexpected(std::move(overview.error()));
        }
        pages.push_back(std::move(*overview));

        if(!doc_plan.detail_path.empty()) {
            auto detail = render_symbol_detail_page(
                doc_plan, owner_plan, config, model, outputs, links, layout);
            if(!detail.has_value()) {
                return std::unexpected(std::move(detail.error()));
            }
            pages.push_back(std::move(*detail));
        }

        if(auto nested = append_symbol_doc_pages(pages,
                                                 doc_plan.children,
                                                 owner_plan,
                                                 config,
                                                 model,
                                                 outputs,
                                                 links,
                                                 layout);
           !nested.has_value()) {
            return nested;
        }
    }
    return {};
}

auto build_namespace_page_root(const PagePlan& plan,
                               const config::TaskConfig& config,
                               const extract::ProjectModel& model,
                               const std::unordered_map<std::string, std::string>& outputs,
                               const LinkResolver& links,
                               const PageDocLayout& layout) -> SemanticSectionPtr {
    auto root =
        make_section(SemanticKind::Namespace, plan.owner_keys.front(), plan.title, 1, false);
    root->children.push_back(MarkdownNode{
        build_prompt_section("Summary",
                             2,
                             prompt_output_of(outputs, PromptKind::NamespaceSummary))});

    auto namespace_diagram = render_namespace_diagram_code(model, plan.owner_keys.front());
    if(!namespace_diagram.empty()) {
        auto section = make_section(SemanticKind::Section, {}, "Diagram", 2);
        section->children.push_back(make_mermaid(std::move(namespace_diagram)));
        root->children.push_back(MarkdownNode{section});
    }

    auto subnamespaces = build_list_section("Subnamespaces", 2, [&]() {
        BulletList list;
        auto ns_it = model.namespaces.find(plan.owner_keys.front());
        if(ns_it == model.namespaces.end()) {
            return list;
        }
        std::vector<std::string> children(ns_it->second.children.begin(),
                                          ns_it->second.children.end());
        std::sort(children.begin(), children.end());
        for(const auto& child: children) {
            if(child.contains("(anonymous namespace)")) {
                continue;
            }
            if(auto* target_path = links.resolve(child)) {
                ListItem item;
                item.fragments.push_back(
                    make_link(child,
                              make_relative_link_target(plan.relative_path, *target_path),
                              true));
                list.items.push_back(std::move(item));
            }
        }
        return list;
    }());
    root->children.push_back(MarkdownNode{subnamespaces});

    auto add_entity_section = [&](std::string heading,
                                  SemanticKind entity_kind,
                                  auto&& predicate,
                                  std::optional<PromptKind> prompt_kind) {
        auto section = make_section(SemanticKind::Section, {}, std::move(heading), 2);
        for(const auto* sym: collect_namespace_symbols(model, plan.owner_keys.front(), predicate)) {
            auto entity =
                make_section(entity_kind, sym->qualified_name, "`" + sym->qualified_name + "`", 3);
            auto has_doc_page = find_doc_index_path(layout, sym->qualified_name) != nullptr;
            auto locations = build_symbol_source_locations(*sym, config);
            if(!locations.items.empty()) {
                entity->children.push_back(MarkdownNode{std::move(locations)});
            }
            push_link_paragraph(entity->children,
                                "Implementations: ",
                                find_implementation_pages(*sym,
                                                          model,
                                                          links,
                                                          plan.relative_path,
                                                          config.project_root));
            add_symbol_doc_links(entity->children,
                                 plan.relative_path,
                                 layout,
                                 *sym,
                                 detail_view_for(plan, *sym));
            if(prompt_kind.has_value()) {
                add_prompt_output(entity->children,
                                  prompt_output_of(outputs, *prompt_kind, make_symbol_target_key(*sym)));
            }
            if(is_type_kind(sym->kind) && !has_doc_page) {
                append_type_structure_sections(entity->children,
                                               model,
                                               *sym,
                                               plan.relative_path,
                                               links,
                                               4);
            }
            section->children.push_back(MarkdownNode{entity});
        }
        root->children.push_back(MarkdownNode{section});
    };

    add_entity_section(
        "Types",
        SemanticKind::Type,
        [](const extract::SymbolInfo& sym) { return is_type_kind(sym.kind); },
        PromptKind::TypeDeclarationSummary);
    add_entity_section(
        "Variables",
        SemanticKind::Variable,
        [](const extract::SymbolInfo& sym) { return is_variable_kind(sym.kind); },
        std::nullopt);
    add_entity_section(
        "Functions",
        SemanticKind::Function,
        [](const extract::SymbolInfo& sym) { return is_function_kind(sym.kind); },
        PromptKind::FunctionDeclarationSummary);

    root->children.push_back(MarkdownNode{build_list_section("Related Pages", 2, [&]() {
        BulletList list;
        for(const auto& target: build_related_page_targets(plan, links, plan.relative_path)) {
            ListItem item;
            item.fragments.push_back(make_link(target.label, target.target));
            list.items.push_back(std::move(item));
        }
        return list;
    }())});

    return root;
}

auto build_module_page_root(const PagePlan& plan,
                            const config::TaskConfig& config,
                            const extract::ProjectModel& model,
                            const std::unordered_map<std::string, std::string>& outputs,
                            const LinkResolver& links,
                            const PageDocLayout& layout) -> SemanticSectionPtr {
    auto root = make_section(SemanticKind::Module, plan.owner_keys.front(), plan.title, 1, false);
    root->children.push_back(MarkdownNode{
        build_prompt_section("Summary", 2, prompt_output_of(outputs, PromptKind::ModuleSummary))});

    if(auto* module = extract::find_module_by_name(model, plan.owner_keys.front())) {
        root->children.push_back(MarkdownNode{build_list_section("Imports", 2, [&]() {
            BulletList list;
            for(const auto& imported: module->imports) {
                ListItem item;
                if(auto* target_path = links.resolve(imported)) {
                    item.fragments.push_back(
                        make_link(imported,
                                  make_relative_link_target(plan.relative_path, *target_path),
                                  true));
                } else {
                    item.fragments.push_back(make_code(imported));
                }
                list.items.push_back(std::move(item));
            }
            return list;
        }())});
        auto import_diagram = render_import_diagram_code(*module);
        if(!import_diagram.empty()) {
            auto section = make_section(SemanticKind::Section, {}, "Dependency Diagram", 2);
            section->children.push_back(make_mermaid(std::move(import_diagram)));
            root->children.push_back(MarkdownNode{section});
        }
    }

    auto add_impl_section = [&](std::string heading,
                                SemanticKind entity_kind,
                                auto&& predicate,
                                std::optional<PromptKind> prompt_kind) {
        auto section = make_section(SemanticKind::Section, {}, std::move(heading), 2);
        for(const auto* sym: collect_implementation_symbols(plan, model, predicate)) {
            auto entity =
                make_section(entity_kind, sym->qualified_name, "`" + sym->qualified_name + "`", 3);
            auto has_doc_page = find_doc_index_path(layout, sym->qualified_name) != nullptr;
            auto locations = build_symbol_source_locations(*sym, config);
            if(!locations.items.empty()) {
                entity->children.push_back(MarkdownNode{std::move(locations)});
            }
            push_optional_link_paragraph(entity->children,
                                         "Declaration: ",
                                         find_declaration_page(*sym, links, plan.relative_path));
            add_symbol_doc_links(entity->children,
                                 plan.relative_path,
                                 layout,
                                 *sym,
                                 detail_view_for(plan, *sym));
            if(prompt_kind.has_value()) {
                add_prompt_output(entity->children,
                                  prompt_output_of(outputs, *prompt_kind, make_symbol_target_key(*sym)));
            }
            if(is_type_kind(sym->kind) && !has_doc_page) {
                append_type_structure_sections(entity->children,
                                               model,
                                               *sym,
                                               plan.relative_path,
                                               links,
                                               4);
            }
            section->children.push_back(MarkdownNode{entity});
        }
        root->children.push_back(MarkdownNode{section});
    };

    add_impl_section(
        "Types",
        SemanticKind::Type,
        [](const extract::SymbolInfo& sym) { return is_type_kind(sym.kind); },
        PromptKind::TypeImplementationSummary);
    add_impl_section(
        "Variables",
        SemanticKind::Variable,
        [](const extract::SymbolInfo& sym) { return is_variable_kind(sym.kind); },
        std::nullopt);
    add_impl_section(
        "Functions",
        SemanticKind::Function,
        [](const extract::SymbolInfo& sym) { return is_function_kind(sym.kind); },
        PromptKind::FunctionImplementationSummary);

    root->children.push_back(MarkdownNode{
        build_prompt_section("Internal Structure",
                             2,
                             prompt_output_of(outputs, PromptKind::ModuleArchitecture))});

    root->children.push_back(MarkdownNode{build_list_section("Related Pages", 2, [&]() {
        BulletList list;
        for(const auto& target: build_related_page_targets(plan, links, plan.relative_path)) {
            ListItem item;
            item.fragments.push_back(make_link(target.label, target.target));
            list.items.push_back(std::move(item));
        }
        return list;
    }())});

    return root;
}

auto build_file_page_root(const PagePlan& plan,
                          const config::TaskConfig& config,
                          const extract::ProjectModel& model,
                          const std::unordered_map<std::string, std::string>& outputs,
                          const LinkResolver& links) -> SemanticSectionPtr {
    auto root = make_section(SemanticKind::File, plan.owner_keys.front(), plan.title, 1, false);

    if(auto file_it = model.files.find(plan.owner_keys.front()); file_it != model.files.end()) {
        root->children.push_back(MarkdownNode{build_list_section("Includes", 2, [&]() {
            BulletList list;
            for(const auto& include: file_it->second.includes) {
                ListItem item;
                item.fragments.push_back(
                    make_code(make_source_relative(include, config.project_root)));
                list.items.push_back(std::move(item));
            }
            return list;
        }())});
    }
    if(auto dependency_diagram = render_file_dependency_diagram_code(plan, config, model);
       !dependency_diagram.empty()) {
        auto section = make_section(SemanticKind::Section, {}, "Dependency Diagram", 2);
        section->children.push_back(make_mermaid(std::move(dependency_diagram)));
        root->children.push_back(MarkdownNode{section});
    }

    auto add_impl_section = [&](std::string heading,
                                SemanticKind entity_kind,
                                auto&& predicate,
                                std::optional<PromptKind> prompt_kind) {
        auto section = make_section(SemanticKind::Section, {}, std::move(heading), 2);
        for(const auto* sym: collect_implementation_symbols(plan, model, predicate)) {
            auto entity =
                make_section(entity_kind, sym->qualified_name, "`" + sym->qualified_name + "`", 3);
            auto locations = build_symbol_source_locations(*sym, config);
            if(!locations.items.empty()) {
                entity->children.push_back(MarkdownNode{std::move(locations)});
            }
            push_optional_link_paragraph(entity->children,
                                         "Declaration: ",
                                         find_declaration_page(*sym, links, plan.relative_path));
            if(prompt_kind.has_value()) {
                add_prompt_output(entity->children,
                                  prompt_output_of(outputs, *prompt_kind, make_symbol_target_key(*sym)));
            }
            if(is_type_kind(sym->kind)) {
                append_type_structure_sections(entity->children,
                                               model,
                                               *sym,
                                               plan.relative_path,
                                               links,
                                               4);
            }
            section->children.push_back(MarkdownNode{entity});
        }
        root->children.push_back(MarkdownNode{section});
    };

    add_impl_section(
        "Types",
        SemanticKind::Type,
        [](const extract::SymbolInfo& sym) { return is_type_kind(sym.kind); },
        PromptKind::TypeImplementationSummary);
    add_impl_section(
        "Variables",
        SemanticKind::Variable,
        [](const extract::SymbolInfo& sym) { return is_variable_kind(sym.kind); },
        std::nullopt);
    add_impl_section(
        "Functions",
        SemanticKind::Function,
        [](const extract::SymbolInfo& sym) { return is_function_kind(sym.kind); },
        PromptKind::FunctionImplementationSummary);

    if(auto module_name = find_module_for_file(model, plan.owner_keys.front())) {
        auto section = make_section(SemanticKind::Section, {}, "Module Information", 2);
        Paragraph paragraph;
        paragraph.fragments.push_back(make_text("Module: "));
        if(auto* target_path = links.resolve(*module_name)) {
            paragraph.fragments.push_back(
                make_link(*module_name,
                          make_relative_link_target(plan.relative_path, *target_path),
                          true));
        } else {
            paragraph.fragments.push_back(make_code(*module_name));
        }
        section->children.push_back(MarkdownNode{std::move(paragraph)});
        root->children.push_back(MarkdownNode{section});
    }

    root->children.push_back(MarkdownNode{build_list_section("Related Pages", 2, [&]() {
        BulletList list;
        for(const auto& target: build_related_page_targets(plan, links, plan.relative_path)) {
            ListItem item;
            item.fragments.push_back(make_link(target.label, target.target));
            list.items.push_back(std::move(item));
        }
        return list;
    }())});

    return root;
}

auto build_index_page_root(const PagePlan& plan,
                           const config::TaskConfig& config,
                           const extract::ProjectModel& model,
                           const std::unordered_map<std::string, std::string>& outputs,
                           const LinkResolver& links) -> SemanticSectionPtr {
    auto root = make_section(SemanticKind::Index, {}, plan.title, 1, false);
    root->children.push_back(MarkdownNode{
        build_prompt_section("Overview", 2, prompt_output_of(outputs, PromptKind::IndexOverview))});

    if(model.uses_modules) {
        root->children.push_back(MarkdownNode{build_list_section("Modules", 2, [&]() {
            BulletList list;
            std::unordered_set<std::string> seen;
            std::vector<std::string> names;
            for(const auto& [_, mod_unit]: model.modules) {
                if(mod_unit.is_interface && seen.insert(mod_unit.name).second &&
                   links.resolve(mod_unit.name) != nullptr) {
                    names.push_back(mod_unit.name);
                }
            }
            std::sort(names.begin(), names.end());
            for(const auto& name: names) {
                ListItem item;
                item.fragments.push_back(
                    make_link(name,
                              make_relative_link_target(plan.relative_path, *links.resolve(name)),
                              true));
                list.items.push_back(std::move(item));
            }
            return list;
        }())});
    }

    root->children.push_back(MarkdownNode{build_list_section("Files", 2, [&]() {
        BulletList list;
        std::vector<std::pair<std::string, std::string>> files;
        for(const auto& [path, _]: model.files) {
            if(links.resolve(path) != nullptr) {
                files.emplace_back(make_source_relative(path, config.project_root), path);
            }
        }
        std::sort(files.begin(), files.end());
        for(const auto& [label, key]: files) {
            ListItem item;
            item.fragments.push_back(
                make_link(label,
                          make_relative_link_target(plan.relative_path, *links.resolve(key)),
                          true));
            list.items.push_back(std::move(item));
        }
        return list;
    }())});

    root->children.push_back(MarkdownNode{build_list_section("Namespaces", 2, [&]() {
        BulletList list;
        std::vector<std::string> namespaces;
        for(const auto& [name, _]: model.namespaces) {
            if(!name.contains("(anonymous namespace)") && links.resolve(name) != nullptr) {
                namespaces.push_back(name);
            }
        }
        std::sort(namespaces.begin(), namespaces.end());
        for(const auto& name: namespaces) {
            ListItem item;
            item.fragments.push_back(
                make_link(name,
                          make_relative_link_target(plan.relative_path, *links.resolve(name)),
                          true));
            list.items.push_back(std::move(item));
        }
        return list;
    }())});

    root->children.push_back(MarkdownNode{build_list_section("Types", 2, [&]() {
        std::vector<const extract::SymbolInfo*> symbols;
        for(const auto& [_, sym]: model.symbols) {
            if(is_type_kind(sym.kind) && !sym.qualified_name.contains("(anonymous namespace)") &&
               links.resolve(sym.qualified_name) != nullptr) {
                symbols.push_back(&sym);
            }
        }
        std::sort(symbols.begin(),
                  symbols.end(),
                  [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                      return lhs->qualified_name < rhs->qualified_name;
                  });
        return build_symbol_link_list(symbols, plan.relative_path, links, true);
    }())});

    auto module_diagram = render_module_dependency_diagram_code(model);
    if(!module_diagram.empty()) {
        auto section = make_section(SemanticKind::Section, {}, "Module Dependency Diagram", 2);
        section->children.push_back(make_mermaid(std::move(module_diagram)));
        root->children.push_back(MarkdownNode{section});
    }

    return root;
}

auto build_page_root(const PagePlan& plan,
                     const config::TaskConfig& config,
                     const extract::ProjectModel& model,
                     const std::unordered_map<std::string, std::string>& outputs,
                     const LinkResolver& links,
                     const PageDocLayout& layout) -> SemanticSectionPtr {
    switch(plan.page_type) {
        case PageType::Index: return build_index_page_root(plan, config, model, outputs, links);
        case PageType::Namespace:
            return build_namespace_page_root(plan, config, model, outputs, links, layout);
        case PageType::Module:
            return build_module_page_root(plan, config, model, outputs, links, layout);
        case PageType::File: return build_file_page_root(plan, config, model, outputs, links);
    }
    return make_section(SemanticKind::Section, {}, plan.title, 1, false);
}

auto select_primary_description_source(const PagePlan& plan,
                                       const std::unordered_map<std::string, std::string>& outputs)
    -> std::string {
    auto from_prompt = [&](PromptKind kind) -> std::string {
        auto* output = prompt_output_of(outputs, kind);
        return output != nullptr ? extract_first_plain_paragraph(*output) : std::string{};
    };

    switch(plan.page_type) {
        case PageType::Namespace:
            if(auto text = from_prompt(PromptKind::NamespaceSummary); !text.empty())
                return text;
            break;
        case PageType::Module:
            if(auto text = from_prompt(PromptKind::ModuleSummary); !text.empty())
                return text;
            if(auto text = from_prompt(PromptKind::ModuleArchitecture); !text.empty())
                return text;
            break;
        case PageType::Index:
            if(auto text = from_prompt(PromptKind::IndexOverview); !text.empty())
                return text;
        case PageType::File: break;
    }

    for(const auto& [_, value]: outputs) {
        if(auto text = extract_first_plain_paragraph(value); !text.empty()) {
            return text;
        }
    }
    return {};
}

auto build_frontmatter(const PagePlan& plan,
                       std::string_view page_title,
                       std::string_view body,
                       const std::unordered_map<std::string, std::string>& outputs) -> Frontmatter {
    auto title = normalize_frontmatter_title(page_title);
    auto description = select_primary_description_source(plan, outputs);
    if(description.empty()) {
        description = extract_first_plain_paragraph(body);
    }
    if(description.empty()) {
        description = title;
    }

    return Frontmatter{
        .title = std::move(title),
        .description = std::move(description),
    };
}

auto build_link_resolver_impl(PagePlanSet plan_set, const extract::ProjectModel* model)
    -> LinkResolver {
    LinkResolver resolver;
    auto register_preferred_path =
        [&](std::string_view name, std::string_view path, bool overwrite) {
            if(overwrite) {
                resolver.name_to_path[std::string(name)] = std::string(path);
                return;
            }
            resolver.name_to_path.emplace(std::string(name), std::string(path));
        };

    for(const auto& plan: plan_set.plans) {
        resolver.page_id_to_title[plan.page_id] = plan.title;
        for(const auto& key: plan.owner_keys) {
            resolver.name_to_path[key] = plan.relative_path;
        }
        auto colon = plan.page_id.find(':');
        if(colon != std::string::npos) {
            resolver.name_to_path[plan.page_id.substr(colon + 1)] = plan.relative_path;
        }

        if(model != nullptr) {
            auto layout = build_page_doc_layout(plan, *model);
            if(plan.page_type == PageType::Namespace) {
                for(const auto& [qualified_name, path]: layout.index_paths) {
                    register_preferred_path(qualified_name, path, true);
                }
            } else if(plan.page_type == PageType::Module) {
                for(const auto& [qualified_name, path]: layout.index_paths) {
                    register_preferred_path(qualified_name, path, false);
                }
            }
        }

        for(const auto& request: plan.prompt_requests) {
            if(request.target_key.empty()) {
                continue;
            }

            switch(request.kind) {
                case PromptKind::TypeDeclarationSummary:
                case PromptKind::FunctionDeclarationSummary:
                    register_preferred_path(request.target_key, plan.relative_path, true);
                    register_preferred_path(parse_symbol_target_key(request.target_key).qualified_name,
                                            plan.relative_path,
                                            true);
                    break;
                case PromptKind::TypeImplementationSummary:
                case PromptKind::FunctionImplementationSummary:
                    register_preferred_path(request.target_key, plan.relative_path, false);
                    register_preferred_path(parse_symbol_target_key(request.target_key).qualified_name,
                                            plan.relative_path,
                                            false);
                    break;
                default: break;
            }
        }
    }

    return resolver;
}

}  // namespace

auto build_link_resolver(const PagePlanSet& plan_set) -> LinkResolver {
    return build_link_resolver_impl(plan_set, nullptr);
}

auto build_link_resolver(const PagePlanSet& plan_set, const extract::ProjectModel& model)
    -> LinkResolver {
    return build_link_resolver_impl(plan_set, &model);
}

auto normalize_prompt_output(std::string_view content) -> std::string {
    return std::string(content);
}

auto render_page_markdown(const PagePlan& plan,
                          const config::TaskConfig& config,
                          const extract::ProjectModel& model,
                          const std::unordered_map<std::string, std::string>& prompt_outputs,
                          const LinkResolver& links) -> std::expected<std::string, RenderError> {
    auto bundle = render_page_bundle(plan, config, model, prompt_outputs, links);
    if(!bundle.has_value()) {
        return std::unexpected(std::move(bundle.error()));
    }
    auto it = std::ranges::find_if(*bundle, [&](const GeneratedPage& page) {
        return page.relative_path == plan.relative_path;
    });
    if(it == bundle->end()) {
        return std::unexpected(RenderError{
            .message = std::format("rendered page bundle is missing '{}'", plan.relative_path)});
    }
    return it->content;
}

auto render_page_bundle(const PagePlan& plan,
                        const config::TaskConfig& config,
                        const extract::ProjectModel& model,
                        const std::unordered_map<std::string, std::string>& prompt_outputs,
                        const LinkResolver& links)
    -> std::expected<std::vector<GeneratedPage>, RenderError> {
    auto normalized_outputs = normalize_prompt_outputs(prompt_outputs);

    auto layout = build_page_doc_layout(plan, model);

    MarkdownDocument document;
    document.children.push_back(MarkdownNode{
        build_page_root(plan, config, model, normalized_outputs, links, layout)});
    auto body = render_markdown(document);
    if(body.empty()) {
        return std::unexpected(RenderError{
            .message = std::format("rendered markdown is empty for '{}'", plan.page_id),
        });
    }

    document.frontmatter = build_frontmatter(plan, plan.title, body, normalized_outputs);
    std::vector<GeneratedPage> pages;
    pages.push_back(GeneratedPage{
        .relative_path = plan.relative_path,
        .content = render_markdown(document),
    });

    if(page_supports_symbol_subpages(plan)) {
        std::optional<RenderError> append_error;
        for_each_symbol_doc_group(layout, [&](const std::vector<SymbolDocPlan>& group) {
            if(append_error.has_value()) {
                return;
            }
            if(auto result = append_symbol_doc_pages(
                   pages, group, plan, config, model, normalized_outputs, links, layout);
               !result.has_value()) {
                append_error = std::move(result.error());
            }
        });
        if(append_error.has_value()) {
            return std::unexpected(std::move(*append_error));
        }
    }

    return pages;
}

auto validate_output(const std::string& content) -> std::expected<void, RenderError> {
    if(content.empty()) {
        return std::unexpected(RenderError{.message = "LLM output is empty"});
    }
    if(std::ranges::all_of(content, [](char ch) {
           return std::isspace(static_cast<unsigned char>(ch)) != 0;
       })) {
        return std::unexpected(RenderError{.message = "LLM output contains only whitespace"});
    }
    if(content.starts_with("# ") || content.find("\n# ") != std::string::npos) {
        return std::unexpected(RenderError{.message = "LLM output contains H1 heading '# '"});
    }
    if(content.find("```") != std::string::npos) {
        return std::unexpected(RenderError{.message = "LLM output contains code fence"});
    }
    return {};
}

auto write_page(const GeneratedPage& page, std::string_view output_root)
    -> std::expected<void, RenderError> {
    namespace fs = std::filesystem;

    auto root = fs::path(output_root);
    auto rel = fs::path(page.relative_path);
    if(rel.is_absolute()) {
        return std::unexpected(RenderError{
            .message = std::format("page output path must be relative: {}", page.relative_path),
        });
    }
    for(const auto& part: rel) {
        if(part == "." || part == "..") {
            return std::unexpected(RenderError{
                .message = std::format("page output path must not contain '.' or '..': {}",
                                       page.relative_path),
            });
        }
    }

    auto target = (root / rel).lexically_normal();
    auto parent = target.parent_path();
    if(!fs::exists(parent)) {
        std::error_code ec;
        fs::create_directories(parent, ec);
        if(ec) {
            return std::unexpected(RenderError{
                .message = std::format("failed to create directory {}: {}",
                                       parent.generic_string(),
                                       ec.message()),
            });
        }
    }

    auto write_result = clore::support::write_utf8_text_file(target, page.content);
    if(!write_result.has_value()) {
        return std::unexpected(RenderError{.message = std::move(write_result.error())});
    }
    return {};
}

}  // namespace clore::generate
