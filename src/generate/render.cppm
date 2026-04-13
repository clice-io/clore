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

auto render_page_markdown(
    const PagePlan& plan,
    const config::TaskConfig& config,
    const extract::ProjectModel& model,
    const std::unordered_map<std::string, std::string>& prompt_outputs,
    const LinkResolver& links) -> std::expected<std::string, RenderError>;

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

auto trim_ascii(std::string_view text) -> std::string_view {
    while(!text.empty() && std::isspace(static_cast<unsigned char>(text.front())) != 0) {
        text.remove_prefix(1);
    }
    while(!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
        text.remove_suffix(1);
    }
    return text;
}

auto strip_inline_markdown(std::string_view text) -> std::string {
    std::string out;
    out.reserve(text.size());
    for(auto ch : text) {
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
        if(trimmed.starts_with("#") || trimmed.starts_with(">") ||
           trimmed.starts_with("|") || trimmed.starts_with("- ") ||
           trimmed.starts_with("* ")) {
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
    for(auto ch : text) {
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

template <typename Predicate>
auto collect_namespace_symbols(const extract::ProjectModel& model,
                               std::string_view namespace_name,
                               Predicate&& predicate)
    -> std::vector<const extract::SymbolInfo*> {
    std::vector<const extract::SymbolInfo*> symbols;
    auto ns_it = model.namespaces.find(std::string(namespace_name));
    if(ns_it == model.namespaces.end()) {
        return symbols;
    }
    for(auto sym_id : ns_it->second.symbols) {
        if(auto* sym = lookup_sym(model, sym_id);
           sym != nullptr && is_page_level_symbol(model, *sym) && predicate(*sym)) {
            symbols.push_back(sym);
        }
    }
    std::sort(symbols.begin(), symbols.end(),
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

    for(const auto& key : plan.owner_keys) {
        if(plan.page_type == PageType::Module) {
            if(auto* module = extract::find_module_by_name(model, key)) {
                for(auto sym_id : module->symbols) {
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
        for(auto sym_id : file_it->second.symbols) {
            if(!seen.insert(sym_id).second) {
                continue;
            }
            if(auto* sym = lookup_sym(model, sym_id);
               sym != nullptr && is_page_level_symbol(model, *sym) && predicate(*sym)) {
                symbols.push_back(sym);
            }
        }
    }

    std::sort(symbols.begin(), symbols.end(),
              [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  return lhs->qualified_name < rhs->qualified_name;
              });
    return symbols;
}

auto build_symbol_link_list(const std::vector<const extract::SymbolInfo*>& symbols,
                            std::string_view current_page_path,
                            const LinkResolver& links,
                            bool use_full_name = true) -> BulletList {
    BulletList list;
    for(const auto* sym : symbols) {
        ListItem item;
        item.fragments.push_back(make_text(std::string(extract::symbol_kind_name(sym->kind)) + " "));
        auto label = use_full_name ? sym->qualified_name : short_name_of(sym->qualified_name);
        if(auto* target_path = links.resolve(sym->qualified_name)) {
            item.fragments.push_back(make_link(
                label.empty() ? sym->qualified_name : label,
                make_relative_link_target(current_page_path, *target_path),
                true));
        } else {
            item.fragments.push_back(
                make_text(label.empty() ? sym->qualified_name : label));
        }
        list.items.push_back(std::move(item));
    }
    return list;
}

auto build_symbol_source_locations(const extract::SymbolInfo& sym,
                                   const config::TaskConfig& config) -> BulletList {
    BulletList list;
    if(sym.declaration_location.is_known()) {
        ListItem item;
        item.fragments.push_back(make_text("Declared at: "));
        item.fragments.push_back(make_code(
            make_source_relative(sym.declaration_location.file, config.project_root) + ":" +
            std::to_string(sym.declaration_location.line)));
        list.items.push_back(std::move(item));
    }
    if(sym.definition_location.has_value() && sym.definition_location->is_known()) {
        ListItem item;
        item.fragments.push_back(make_text("Defined at: "));
        item.fragments.push_back(make_code(
            make_source_relative(sym.definition_location->file, config.project_root) + ":" +
            std::to_string(sym.definition_location->line)));
        list.items.push_back(std::move(item));
    }
    return list;
}

template <typename Predicate>
auto build_member_list(const extract::ProjectModel& model,
                       const extract::SymbolInfo& sym,
                       Predicate&& predicate) -> BulletList {
    std::vector<const extract::SymbolInfo*> members;
    members.reserve(sym.children.size());

    BulletList list;
    for(auto child_id : sym.children) {
        auto* child = lookup_sym(model, child_id);
        if(child == nullptr) {
            continue;
        }
        if(!predicate(*child)) {
            continue;
        }
        members.push_back(child);
    }

    std::sort(members.begin(), members.end(),
              [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  auto lhs_name = lhs->qualified_name.empty() ? lhs->name : lhs->qualified_name;
                  auto rhs_name = rhs->qualified_name.empty() ? rhs->name : rhs->qualified_name;
                  return lhs_name < rhs_name;
              });

    for(const auto* child : members) {
        ListItem item;
        auto access = child->access.empty() ? std::string{"public"} : child->access;
        item.fragments.push_back(
            make_text(access + " " + std::string(extract::symbol_kind_name(child->kind)) + " "));
        auto label = child->name.empty() ? short_name_of(child->qualified_name) : child->name;
        if(label.empty()) {
            label = child->qualified_name;
        }
        item.fragments.push_back(make_code(std::move(label)));
        list.items.push_back(std::move(item));
    }
    return list;
}

auto find_implementation_pages(const extract::SymbolInfo& sym,
                               const extract::ProjectModel& model,
                               const LinkResolver& links,
                               std::string_view current_page_path,
                               const std::string& project_root)
    -> std::vector<LinkTarget> {
    std::vector<LinkTarget> results;
    std::unordered_set<std::string> seen;

    auto try_add = [&](const std::string& file_path) {
        if(file_path.empty()) {
            return;
        }
        if(auto* mod = extract::find_module_by_source(model, file_path)) {
            if(auto* target_path = links.resolve(mod->name)) {
                if(seen.insert(*target_path).second) {
                    results.push_back(make_link_target(current_page_path, "Module " + mod->name,
                                                       *target_path));
                }
            }
            return;
        }
        if(auto* target_path = links.resolve(file_path)) {
            if(seen.insert(*target_path).second) {
                results.push_back(make_link_target(
                    current_page_path,
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
                           std::string_view current_page_path)
    -> std::optional<LinkTarget> {
    if(auto* target_path = links.resolve(sym.qualified_name);
       target_path != nullptr && *target_path != current_page_path) {
        auto ns_name = !sym.enclosing_namespace.empty() ? sym.enclosing_namespace
                                                        : namespace_of(sym.qualified_name);
        auto label = ns_name.empty() ? std::string{"Declaration page"}
                                     : std::string{"Namespace "} + ns_name;
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
    for(const auto& linked : plan.linked_pages) {
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

auto build_prompt_section(std::string heading,
                          std::uint8_t level,
                          const std::string* output) -> SemanticSectionPtr {
    auto section = make_section(SemanticKind::Section, {}, std::move(heading), level);
    if(output != nullptr && !trim_ascii(*output).empty()) {
        section->children.push_back(make_raw_markdown(*output));
    }
    return section;
}

auto build_list_section(std::string heading,
                        std::uint8_t level,
                        BulletList list) -> SemanticSectionPtr {
    auto section = make_section(SemanticKind::Section, {}, std::move(heading), level);
    if(!list.items.empty()) {
        section->children.push_back(MarkdownNode{std::move(list)});
    }
    return section;
}

auto append_type_structure_sections(std::vector<MarkdownNode>& nodes,
                                    const extract::ProjectModel& model,
                                    const extract::SymbolInfo& sym,
                                    std::string_view current_page_path,
                                    const LinkResolver& links,
                                    std::uint8_t level) -> void {
    std::vector<const extract::SymbolInfo*> base_symbols;
    for(auto base_id : sym.bases) {
        if(auto* base = lookup_sym(model, base_id)) {
            base_symbols.push_back(base);
        }
    }
    std::sort(base_symbols.begin(), base_symbols.end(),
              [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  return lhs->qualified_name < rhs->qualified_name;
              });
    nodes.push_back(MarkdownNode{build_list_section(
        "Base Types", level,
        build_symbol_link_list(base_symbols, current_page_path, links, true))});

    std::vector<const extract::SymbolInfo*> derived_symbols;
    for(auto derived_id : sym.derived) {
        if(auto* derived = lookup_sym(model, derived_id)) {
            derived_symbols.push_back(derived);
        }
    }
    std::sort(derived_symbols.begin(), derived_symbols.end(),
              [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  return lhs->qualified_name < rhs->qualified_name;
              });
    nodes.push_back(MarkdownNode{build_list_section(
        "Derived Types", level,
        build_symbol_link_list(derived_symbols, current_page_path, links, true))});

    nodes.push_back(MarkdownNode{build_list_section(
        "Member Types", level,
        build_member_list(model, sym, [](const extract::SymbolInfo& child) {
            return is_type_kind(child.kind);
        }))});
    nodes.push_back(MarkdownNode{build_list_section(
        "Member Variables", level,
        build_member_list(model, sym, [](const extract::SymbolInfo& child) {
            return is_variable_kind(child.kind);
        }))});
    nodes.push_back(MarkdownNode{build_list_section(
        "Member Functions", level,
        build_member_list(model, sym, [](const extract::SymbolInfo& child) {
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

    std::string result = "graph LR\n";
    std::unordered_map<std::string, std::string> node_ids;
    std::vector<std::pair<std::string, std::string>> nodes;
    auto ensure_node_id = [&](std::string_view label) -> std::string {
        auto key = std::string(label);
        if(auto it = node_ids.find(key); it != node_ids.end()) {
            return it->second;
        }
        auto node_id = "M" + std::to_string(nodes.size());
        node_ids.emplace(key, node_id);
        nodes.emplace_back(node_id, key);
        return node_id;
    };

    auto module_id = ensure_node_id(top_module(mod_unit.name));
    std::unordered_set<std::string> seen;
    for(const auto& imp : mod_unit.imports) {
        auto label = top_module(imp);
        if(!seen.insert(label).second) {
            continue;
        }
        result += "    " + ensure_node_id(label) + " --> " + module_id + "\n";
    }
    for(const auto& [node_id, label] : nodes) {
        result += "    " + node_id + "[\"" + escape_mermaid_label(label) + "\"]\n";
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
    for(auto sym_id : ns_it->second.symbols) {
        if(auto* sym = lookup_sym(model, sym_id); sym != nullptr && is_type_kind(sym->kind)) {
            type_names.push_back(short_name_of(sym->qualified_name));
        }
    }
    bool has_child = false;
    for(const auto& child : ns_it->second.children) {
        if(!child.contains("(anonymous namespace)")) {
            has_child = true;
            break;
        }
    }
    if(type_names.empty() && !has_child) {
        return {};
    }

    std::string result = "graph TD\n";
    result += "    NS[\"" + escape_mermaid_label(short_name_of(namespace_name)) + "\"]\n";
    for(std::size_t i = 0; i < type_names.size(); ++i) {
        auto node_id = "T" + std::to_string(i);
        result += "    " + node_id + "[\"" + escape_mermaid_label(type_names[i]) + "\"]\n";
        result += "    NS --> " + node_id + "\n";
    }
    std::size_t child_index = 0;
    for(const auto& child : ns_it->second.children) {
        if(child.contains("(anonymous namespace)")) {
            continue;
        }
        auto node_id = "NSC" + std::to_string(child_index++);
        result += "    " + node_id + "[\"" + escape_mermaid_label(short_name_of(child)) + "\"]\n";
        result += "    NS --> " + node_id + "\n";
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
    for(const auto& [_, mod_unit] : model.modules) {
        if(!mod_unit.is_interface) {
            continue;
        }
        auto from = top_module(mod_unit.name);
        modules.insert(from);
        for(const auto& imp : mod_unit.imports) {
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

    std::vector<std::string> sorted(modules.begin(), modules.end());
    std::sort(sorted.begin(), sorted.end());
    std::unordered_map<std::string, std::string> node_ids;
    std::string result = "graph LR\n";
    for(std::size_t i = 0; i < sorted.size(); ++i) {
        auto node_id = std::format("M{}", i);
        node_ids.emplace(sorted[i], node_id);
        result += "    " + node_id + "[\"" + escape_mermaid_label(sorted[i]) + "\"]\n";
    }
    for(const auto& from : sorted) {
        auto dep_it = deps.find(from);
        if(dep_it == deps.end()) {
            continue;
        }
        std::vector<std::string> targets(dep_it->second.begin(), dep_it->second.end());
        std::sort(targets.begin(), targets.end());
        for(const auto& to : targets) {
            result += "    " + node_ids.at(to) + " --> " + node_ids.at(from) + "\n";
        }
    }
    return result;
}

auto extract_workflow_title(const std::string& output)
    -> std::optional<std::pair<std::string, std::string>> {
    constexpr std::string_view prefix = "Title: ";
    if(!output.starts_with(prefix)) {
        return std::nullopt;
    }
    auto nl = output.find('\n');
    if(nl == std::string::npos) {
        return std::nullopt;
    }
    auto title = output.substr(prefix.size(), nl - prefix.size());
    while(!title.empty() && (title.back() == ' ' || title.back() == '\r')) {
        title.pop_back();
    }
    if(title.empty()) {
        return std::nullopt;
    }
    auto rest_start = nl + 1;
    while(rest_start < output.size() &&
          (output[rest_start] == '\n' || output[rest_start] == '\r')) {
        ++rest_start;
    }
    if(trim_ascii(output.substr(rest_start)).empty()) {
        return std::nullopt;
    }
    return std::pair{std::move(title), output.substr(rest_start)};
}

auto build_workflow_call_chain_code(const PagePlan& plan) -> std::string {
    if(plan.owner_keys.size() < 2) {
        return {};
    }
    std::string result = "graph LR\n";
    for(std::size_t i = 0; i < plan.owner_keys.size(); ++i) {
        result += "    N" + std::to_string(i) + "[\"" +
                  escape_mermaid_label(short_name_of(plan.owner_keys[i]) + "()") + "\"]\n";
    }
    for(std::size_t i = 0; i + 1 < plan.owner_keys.size(); ++i) {
        result += "    N" + std::to_string(i) + " --> N" + std::to_string(i + 1) + "\n";
    }
    return result;
}

auto build_workflow_participants(const PagePlan& plan,
                                 const extract::ProjectModel& model,
                                 const config::TaskConfig& config) -> BulletList {
    BulletList list;
    for(const auto& qname : plan.owner_keys) {
        auto* sym = find_sym(model, qname);
        if(sym == nullptr) {
            continue;
        }
        ListItem item;
        item.fragments.push_back(make_text(std::string(extract::symbol_kind_name(sym->kind)) + " "));
        item.fragments.push_back(make_code(sym->qualified_name));
        auto rel = make_source_relative(sym->declaration_location.file, config.project_root);
        if(!rel.empty() && sym->declaration_location.line > 0) {
            item.fragments.push_back(make_text(" ("));
            item.fragments.push_back(make_code(rel + ":" + std::to_string(sym->declaration_location.line)));
            item.fragments.push_back(make_text(")"));
        }
        list.items.push_back(std::move(item));
    }
    return list;
}

auto build_namespace_page_root(const PagePlan& plan,
                               const config::TaskConfig& config,
                               const extract::ProjectModel& model,
                               const std::unordered_map<std::string, std::string>& outputs,
                               const LinkResolver& links) -> SemanticSectionPtr {
    auto root = make_section(SemanticKind::Namespace, plan.owner_keys.front(), plan.title, 1, false);
    root->children.push_back(MarkdownNode{build_prompt_section(
        "Summary", 2, prompt_output_of(outputs, PromptKind::NamespaceSummary))});

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
        std::vector<std::string> children(ns_it->second.children.begin(), ns_it->second.children.end());
        std::sort(children.begin(), children.end());
        for(const auto& child : children) {
            if(child.contains("(anonymous namespace)")) {
                continue;
            }
            if(auto* target_path = links.resolve(child)) {
                ListItem item;
                item.fragments.push_back(make_link(
                    child,
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
        for(const auto* sym : collect_namespace_symbols(model, plan.owner_keys.front(), predicate)) {
            auto entity = make_section(entity_kind, sym->qualified_name,
                                       "`" + sym->qualified_name + "`", 3);
            auto locations = build_symbol_source_locations(*sym, config);
            if(!locations.items.empty()) {
                entity->children.push_back(MarkdownNode{std::move(locations)});
            }
            push_link_paragraph(entity->children, "Implementations: ",
                                find_implementation_pages(*sym, model, links,
                                                          plan.relative_path,
                                                          config.project_root));
            if(prompt_kind.has_value()) {
                add_prompt_output(entity->children,
                                  prompt_output_of(outputs, *prompt_kind, sym->qualified_name));
            }
            if(is_type_kind(sym->kind)) {
                append_type_structure_sections(entity->children, model, *sym,
                                               plan.relative_path, links, 4);
            }
            section->children.push_back(MarkdownNode{entity});
        }
        root->children.push_back(MarkdownNode{section});
    };

    add_entity_section("Types", SemanticKind::Type,
                       [](const extract::SymbolInfo& sym) { return is_type_kind(sym.kind); },
                       PromptKind::TypeDeclarationSummary);
    add_entity_section("Variables", SemanticKind::Variable,
                       [](const extract::SymbolInfo& sym) { return is_variable_kind(sym.kind); },
                       std::nullopt);
    add_entity_section("Functions", SemanticKind::Function,
                       [](const extract::SymbolInfo& sym) { return is_function_kind(sym.kind); },
                       PromptKind::FunctionDeclarationSummary);

    root->children.push_back(MarkdownNode{build_list_section("Related Pages", 2, [&]() {
        BulletList list;
        for(const auto& target : build_related_page_targets(plan, links, plan.relative_path)) {
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
                            const LinkResolver& links) -> SemanticSectionPtr {
    auto root = make_section(SemanticKind::Module, plan.owner_keys.front(), plan.title, 1, false);
    root->children.push_back(MarkdownNode{build_prompt_section(
        "Summary", 2, prompt_output_of(outputs, PromptKind::ModuleSummary))});

    if(auto* module = extract::find_module_by_name(model, plan.owner_keys.front())) {
        root->children.push_back(MarkdownNode{build_list_section("Imports", 2, [&]() {
            BulletList list;
            for(const auto& imported : module->imports) {
                ListItem item;
                if(auto* target_path = links.resolve(imported)) {
                    item.fragments.push_back(make_link(
                        imported,
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
        for(const auto* sym : collect_implementation_symbols(plan, model, predicate)) {
            auto entity = make_section(entity_kind, sym->qualified_name,
                                       "`" + sym->qualified_name + "`", 3);
            auto locations = build_symbol_source_locations(*sym, config);
            if(!locations.items.empty()) {
                entity->children.push_back(MarkdownNode{std::move(locations)});
            }
            push_optional_link_paragraph(entity->children, "Declaration: ",
                                         find_declaration_page(*sym, links, plan.relative_path));
            if(prompt_kind.has_value()) {
                add_prompt_output(entity->children,
                                  prompt_output_of(outputs, *prompt_kind, sym->qualified_name));
            }
            if(is_type_kind(sym->kind)) {
                append_type_structure_sections(entity->children, model, *sym,
                                               plan.relative_path, links, 4);
            }
            section->children.push_back(MarkdownNode{entity});
        }
        root->children.push_back(MarkdownNode{section});
    };

    add_impl_section("Types", SemanticKind::Type,
                     [](const extract::SymbolInfo& sym) { return is_type_kind(sym.kind); },
                     PromptKind::TypeImplementationSummary);
    add_impl_section("Variables", SemanticKind::Variable,
                     [](const extract::SymbolInfo& sym) { return is_variable_kind(sym.kind); },
                     std::nullopt);
    add_impl_section("Functions", SemanticKind::Function,
                     [](const extract::SymbolInfo& sym) { return is_function_kind(sym.kind); },
                     PromptKind::FunctionImplementationSummary);

    root->children.push_back(MarkdownNode{build_prompt_section(
        "Internal Structure", 2,
        prompt_output_of(outputs, PromptKind::ModuleArchitecture))});

    root->children.push_back(MarkdownNode{build_list_section("Related Pages", 2, [&]() {
        BulletList list;
        for(const auto& target : build_related_page_targets(plan, links, plan.relative_path)) {
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
            for(const auto& include : file_it->second.includes) {
                ListItem item;
                item.fragments.push_back(make_code(make_source_relative(include, config.project_root)));
                list.items.push_back(std::move(item));
            }
            return list;
        }())});
    }

    auto add_impl_section = [&](std::string heading,
                                SemanticKind entity_kind,
                                auto&& predicate,
                                std::optional<PromptKind> prompt_kind) {
        auto section = make_section(SemanticKind::Section, {}, std::move(heading), 2);
        for(const auto* sym : collect_implementation_symbols(plan, model, predicate)) {
            auto entity = make_section(entity_kind, sym->qualified_name,
                                       "`" + sym->qualified_name + "`", 3);
            auto locations = build_symbol_source_locations(*sym, config);
            if(!locations.items.empty()) {
                entity->children.push_back(MarkdownNode{std::move(locations)});
            }
            push_optional_link_paragraph(entity->children, "Declaration: ",
                                         find_declaration_page(*sym, links, plan.relative_path));
            if(prompt_kind.has_value()) {
                add_prompt_output(entity->children,
                                  prompt_output_of(outputs, *prompt_kind, sym->qualified_name));
            }
            if(is_type_kind(sym->kind)) {
                append_type_structure_sections(entity->children, model, *sym,
                                               plan.relative_path, links, 4);
            }
            section->children.push_back(MarkdownNode{entity});
        }
        root->children.push_back(MarkdownNode{section});
    };

    add_impl_section("Types", SemanticKind::Type,
                     [](const extract::SymbolInfo& sym) { return is_type_kind(sym.kind); },
                     PromptKind::TypeImplementationSummary);
    add_impl_section("Variables", SemanticKind::Variable,
                     [](const extract::SymbolInfo& sym) { return is_variable_kind(sym.kind); },
                     std::nullopt);
    add_impl_section("Functions", SemanticKind::Function,
                     [](const extract::SymbolInfo& sym) { return is_function_kind(sym.kind); },
                     PromptKind::FunctionImplementationSummary);

    if(auto module_name = find_module_for_file(model, plan.owner_keys.front())) {
        auto section = make_section(SemanticKind::Section, {}, "Module Information", 2);
        Paragraph paragraph;
        paragraph.fragments.push_back(make_text("Module: "));
        if(auto* target_path = links.resolve(*module_name)) {
            paragraph.fragments.push_back(make_link(
                *module_name,
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
        for(const auto& target : build_related_page_targets(plan, links, plan.relative_path)) {
            ListItem item;
            item.fragments.push_back(make_link(target.label, target.target));
            list.items.push_back(std::move(item));
        }
        return list;
    }())});

    return root;
}

auto build_workflow_page_root(const PagePlan& plan,
                              const config::TaskConfig& config,
                              const extract::ProjectModel& model,
                              const std::unordered_map<std::string, std::string>& outputs,
                              std::string page_title,
                              const LinkResolver& links) -> SemanticSectionPtr {
    auto root = make_section(SemanticKind::Workflow, plan.page_id, std::move(page_title), 1, false);
    root->children.push_back(MarkdownNode{build_prompt_section(
        "Workflow", 2, prompt_output_of(outputs, PromptKind::Workflow))});

    auto chain = build_workflow_call_chain_code(plan);
    if(!chain.empty()) {
        auto section = make_section(SemanticKind::Section, {}, "Call Chain", 2);
        section->children.push_back(make_mermaid(std::move(chain)));
        root->children.push_back(MarkdownNode{section});
    }

    root->children.push_back(MarkdownNode{build_list_section(
        "Participants", 2, build_workflow_participants(plan, model, config))});
    root->children.push_back(MarkdownNode{build_list_section("Related Pages", 2, [&]() {
        BulletList list;
        for(const auto& target : build_related_page_targets(plan, links, plan.relative_path)) {
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
    root->children.push_back(MarkdownNode{build_prompt_section(
        "Overview", 2, prompt_output_of(outputs, PromptKind::IndexOverview))});
    root->children.push_back(MarkdownNode{build_prompt_section(
        "Reading Guide", 2, prompt_output_of(outputs, PromptKind::IndexReadingGuide))});

    if(model.uses_modules) {
        root->children.push_back(MarkdownNode{build_list_section("Modules", 2, [&]() {
            BulletList list;
            std::unordered_set<std::string> seen;
            std::vector<std::string> names;
            for(const auto& [_, mod_unit] : model.modules) {
                if(mod_unit.is_interface && seen.insert(mod_unit.name).second &&
                   links.resolve(mod_unit.name) != nullptr) {
                    names.push_back(mod_unit.name);
                }
            }
            std::sort(names.begin(), names.end());
            for(const auto& name : names) {
                ListItem item;
                item.fragments.push_back(make_link(
                    name,
                    make_relative_link_target(plan.relative_path, *links.resolve(name)),
                    true));
                list.items.push_back(std::move(item));
            }
            return list;
        }())});
    } else {
        root->children.push_back(MarkdownNode{build_list_section("Files", 2, [&]() {
            BulletList list;
            std::vector<std::pair<std::string, std::string>> files;
            for(const auto& [path, _] : model.files) {
                if(links.resolve(path) != nullptr) {
                    files.emplace_back(make_source_relative(path, config.project_root), path);
                }
            }
            std::sort(files.begin(), files.end());
            for(const auto& [label, key] : files) {
                ListItem item;
                item.fragments.push_back(make_link(
                    label,
                    make_relative_link_target(plan.relative_path, *links.resolve(key)),
                    true));
                list.items.push_back(std::move(item));
            }
            return list;
        }())});
    }

    root->children.push_back(MarkdownNode{build_list_section("Namespaces", 2, [&]() {
        BulletList list;
        std::vector<std::string> namespaces;
        for(const auto& [name, _] : model.namespaces) {
            if(!name.contains("(anonymous namespace)") && links.resolve(name) != nullptr) {
                namespaces.push_back(name);
            }
        }
        std::sort(namespaces.begin(), namespaces.end());
        for(const auto& name : namespaces) {
            ListItem item;
            item.fragments.push_back(make_link(
                name,
                make_relative_link_target(plan.relative_path, *links.resolve(name)),
                true));
            list.items.push_back(std::move(item));
        }
        return list;
    }())});

    root->children.push_back(MarkdownNode{build_list_section("Types", 2, [&]() {
        std::vector<const extract::SymbolInfo*> symbols;
        for(const auto& [_, sym] : model.symbols) {
            if(is_type_kind(sym.kind) && !sym.qualified_name.contains("(anonymous namespace)") &&
               links.resolve(sym.qualified_name) != nullptr) {
                symbols.push_back(&sym);
            }
        }
        std::sort(symbols.begin(), symbols.end(),
                  [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                      return lhs->qualified_name < rhs->qualified_name;
                  });
        return build_symbol_link_list(symbols, plan.relative_path, links, true);
    }())});

    root->children.push_back(MarkdownNode{build_list_section("Workflows", 2, [&]() {
        BulletList list;
        std::vector<std::pair<std::string, std::string>> workflows;
        for(const auto& linked : plan.linked_pages) {
            if(!linked.starts_with("workflow:")) {
                continue;
            }
            auto slug = linked.substr(9);
            auto* path = links.resolve(slug);
            if(path == nullptr) {
                continue;
            }
            auto* title = links.resolve_page_title(linked);
            auto label = title != nullptr && !title->empty() ? strip_inline_markdown(*title)
                                                             : slug;
            workflows.emplace_back(label, *path);
        }
        std::sort(workflows.begin(), workflows.end());
        for(const auto& [label, path] : workflows) {
            ListItem item;
            item.fragments.push_back(make_link(
                label,
                make_relative_link_target(plan.relative_path, path),
                false));
            list.items.push_back(std::move(item));
        }
        return list;
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
                     std::string title) -> SemanticSectionPtr {
    switch(plan.page_type) {
        case PageType::Index:
            return build_index_page_root(plan, config, model, outputs, links);
        case PageType::Namespace:
            return build_namespace_page_root(plan, config, model, outputs, links);
        case PageType::Module:
            return build_module_page_root(plan, config, model, outputs, links);
        case PageType::File:
            return build_file_page_root(plan, config, model, outputs, links);
        case PageType::Workflow:
            return build_workflow_page_root(plan, config, model, outputs, std::move(title), links);
    }
    return make_section(SemanticKind::Section, {}, std::move(title), 1, false);
}

auto select_primary_description_source(
    const PagePlan& plan,
    const std::unordered_map<std::string, std::string>& outputs) -> std::string {
    auto from_prompt = [&](PromptKind kind) -> std::string {
        auto* output = prompt_output_of(outputs, kind);
        return output != nullptr ? extract_first_plain_paragraph(*output) : std::string{};
    };

    switch(plan.page_type) {
        case PageType::Namespace:
            if(auto text = from_prompt(PromptKind::NamespaceSummary); !text.empty()) return text;
            break;
        case PageType::Module:
            if(auto text = from_prompt(PromptKind::ModuleSummary); !text.empty()) return text;
            if(auto text = from_prompt(PromptKind::ModuleArchitecture); !text.empty()) return text;
            break;
        case PageType::Workflow:
            if(auto text = from_prompt(PromptKind::Workflow); !text.empty()) return text;
            break;
        case PageType::Index:
            if(auto text = from_prompt(PromptKind::IndexOverview); !text.empty()) return text;
            if(auto text = from_prompt(PromptKind::IndexReadingGuide); !text.empty()) return text;
            break;
        case PageType::File:
            break;
    }

    for(const auto& [_, value] : outputs) {
        if(auto text = extract_first_plain_paragraph(value); !text.empty()) {
            return text;
        }
    }
    return {};
}

auto build_frontmatter(const PagePlan& plan,
                       std::string_view page_title,
                       std::string_view body,
                       const std::unordered_map<std::string, std::string>& outputs)
    -> Frontmatter {
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

}  // namespace

auto build_link_resolver(const PagePlanSet& plan_set) -> LinkResolver {
    LinkResolver resolver;
    for(const auto& plan : plan_set.plans) {
        resolver.page_id_to_title[plan.page_id] = plan.title;
        for(const auto& key : plan.owner_keys) {
            resolver.name_to_path[key] = plan.relative_path;
        }
        auto colon = plan.page_id.find(':');
        if(colon != std::string::npos) {
            resolver.name_to_path[plan.page_id.substr(colon + 1)] = plan.relative_path;
        }
        for(const auto& request : plan.prompt_requests) {
            if(request.target_key.empty()) {
                continue;
            }

            switch(request.kind) {
                case PromptKind::TypeDeclarationSummary:
                case PromptKind::FunctionDeclarationSummary:
                    resolver.name_to_path[request.target_key] = plan.relative_path;
                    break;
                case PromptKind::TypeImplementationSummary:
                case PromptKind::FunctionImplementationSummary:
                    resolver.name_to_path.emplace(request.target_key, plan.relative_path);
                    break;
                default:
                    break;
            }
        }
    }
    return resolver;
}

auto render_page_markdown(
    const PagePlan& plan,
    const config::TaskConfig& config,
    const extract::ProjectModel& model,
    const std::unordered_map<std::string, std::string>& prompt_outputs,
    const LinkResolver& links) -> std::expected<std::string, RenderError> {
    auto page_title = plan.title;
    if(plan.page_type == PageType::Workflow) {
        if(auto* output = prompt_output_of(prompt_outputs, PromptKind::Workflow)) {
            if(auto extracted = extract_workflow_title(*output)) {
                page_title = extracted->first;
            }
        }
    }

    MarkdownDocument document;
    document.children.push_back(MarkdownNode{build_page_root(
        plan, config, model, prompt_outputs, links, page_title)});
    auto body = render_markdown(document);
    if(body.empty()) {
        return std::unexpected(RenderError{
            .message = std::format("rendered markdown is empty for '{}'", plan.page_id),
        });
    }

    document.frontmatter = build_frontmatter(plan, page_title, body, prompt_outputs);
    return render_markdown(document);
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
    for(const auto& part : rel) {
        if(part == "." || part == "..") {
            return std::unexpected(RenderError{
                .message = std::format(
                    "page output path must not contain '.' or '..': {}",
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
                                       parent.generic_string(), ec.message()),
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