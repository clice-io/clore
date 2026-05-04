export module generate:common;

import std;
import :markdown;
import :model;
import config;
import extract;

export namespace clore::generate {

struct LinkTarget {
    std::string label;
    std::string target;
    bool code_style = false;
};

enum class SymbolDocView : std::uint8_t {
    Declaration,
    Implementation,
    Details,
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
    for(auto ch: text) {
        if(ch == '`' || ch == '*' || ch == '_' || ch == '[' || ch == ']' || ch == '#') {
            continue;
        }
        out.push_back(ch);
    }
    return std::string(trim_ascii(out));
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

auto prompt_output_of(const std::unordered_map<std::string, std::string>& outputs,
                      PromptKind kind,
                      std::string_view target_key = {}) -> const std::string* {
    auto it = outputs.find(prompt_request_key(PromptRequest{
        .kind = kind,
        .target_key = std::string(target_key),
    }));
    return it != outputs.end() ? &it->second : nullptr;
}

auto make_link_target(std::string_view current_page_path,
                      std::string label,
                      std::string_view target_page_path,
                      bool code_style = false) -> LinkTarget {
    return LinkTarget{
        .label = std::move(label),
        .target = make_relative_link_target(current_page_path, target_page_path),
        .code_style = code_style,
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
        paragraph.fragments.push_back(
            make_link(targets[i].label, targets[i].target, targets[i].code_style));
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
    paragraph.fragments.push_back(make_link(target->label, target->target, target->code_style));
    nodes.push_back(MarkdownNode{std::move(paragraph)});
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

auto add_prompt_output(std::vector<MarkdownNode>& nodes, const std::string* output) -> void {
    if(output != nullptr && !trim_ascii(*output).empty()) {
        nodes.push_back(make_raw_markdown(*output));
    }
}

auto build_string_list(const std::vector<std::string>& items) -> BulletList {
    BulletList list;
    for(const auto& item_text: items) {
        if(trim_ascii(item_text).empty()) {
            continue;
        }
        ListItem item;
        item.fragments = code_spanned_fragments(item_text);
        list.items.push_back(std::move(item));
    }
    return list;
}

auto symbol_analysis_markdown_for(const SymbolAnalysisStore& analyses,
                                  const PagePlan& plan,
                                  const extract::SymbolInfo& sym) -> const std::string* {
    if(plan.page_type == PageType::Namespace) {
        return analysis_overview_markdown(analyses, sym);
    }
    return analysis_details_markdown(analyses, sym);
}

auto add_symbol_analysis_detail_sections(std::vector<MarkdownNode>& nodes,
                                         const SymbolAnalysisStore& analyses,
                                         const PagePlan& plan,
                                         const extract::SymbolInfo& sym,
                                         std::uint8_t level) -> void;

auto add_symbol_analysis_sections(std::vector<MarkdownNode>& nodes,
                                  const SymbolAnalysisStore& analyses,
                                  const PagePlan& plan,
                                  const extract::SymbolInfo& sym,
                                  std::uint8_t level) -> void {
    add_prompt_output(nodes, symbol_analysis_markdown_for(analyses, plan, sym));
    add_symbol_analysis_detail_sections(nodes, analyses, plan, sym, level);
}

auto add_symbol_analysis_detail_sections(std::vector<MarkdownNode>& nodes,
                                         const SymbolAnalysisStore& analyses,
                                         const PagePlan& plan,
                                         const extract::SymbolInfo& sym,
                                         std::uint8_t level) -> void {

    auto make_list_node =
        [&](std::string heading,
            const std::vector<std::string>& items) -> std::optional<MarkdownNode> {
        auto list = build_string_list(items);
        if(list.items.empty()) {
            return std::nullopt;
        }
        return MarkdownNode{build_list_section(std::move(heading), level, std::move(list))};
    };

    auto target_key = make_symbol_target_key(sym);
    if(auto* analysis = find_function_analysis(analyses, target_key)) {
        if(plan.page_type != PageType::Namespace) {
            if(analysis->has_side_effects) {
                if(auto side_effects = make_list_node("Side Effects", analysis->side_effects);
                   side_effects.has_value()) {
                    nodes.push_back(std::move(*side_effects));
                }
            } else {
                auto section = make_section(SemanticKind::Section, {}, "Side Effects", level);
                section->children.push_back(make_paragraph(
                    "No observable side effects are evident from the extracted code."));
                nodes.push_back(MarkdownNode{std::move(section)});
            }

            if(auto reads = make_list_node("Reads From", analysis->reads_from); reads.has_value()) {
                nodes.push_back(std::move(*reads));
            }
            if(auto writes = make_list_node("Writes To", analysis->writes_to); writes.has_value()) {
                nodes.push_back(std::move(*writes));
            }
        }

        if(auto usage = make_list_node("Usage Patterns", analysis->usage_patterns);
           usage.has_value()) {
            nodes.push_back(std::move(*usage));
        }
        return;
    }

    if(auto* analysis = find_type_analysis(analyses, target_key)) {
        if(auto invariants = make_list_node("Invariants", analysis->invariants);
           invariants.has_value()) {
            nodes.push_back(std::move(*invariants));
        }
        if(auto members = make_list_node("Key Members", analysis->key_members);
           members.has_value()) {
            nodes.push_back(std::move(*members));
        }
        if(auto usage = make_list_node("Usage Patterns", analysis->usage_patterns);
           usage.has_value()) {
            nodes.push_back(std::move(*usage));
        }
        return;
    }

    if(auto* analysis = find_variable_analysis(analyses, target_key)) {
        if(plan.page_type != PageType::Namespace) {
            if(analysis->is_mutated) {
                if(auto mutations = make_list_node("Mutation Sources", analysis->mutation_sources);
                   mutations.has_value()) {
                    nodes.push_back(std::move(*mutations));
                }
            } else {
                auto section = make_section(SemanticKind::Section, {}, "Mutation", level);
                section->children.push_back(
                    make_paragraph("No mutation is evident from the extracted code."));
                nodes.push_back(MarkdownNode{std::move(section)});
            }
        }

        if(auto usage = make_list_node("Usage Patterns", analysis->usage_patterns);
           usage.has_value()) {
            nodes.push_back(std::move(*usage));
        }
    }
}

auto symbol_doc_view_for(const PagePlan& plan, const extract::SymbolInfo& sym) -> SymbolDocView {
    switch(plan.page_type) {
        case PageType::Namespace: return SymbolDocView::Declaration;
        case PageType::Module:
            return is_variable_kind(sym.kind) ? SymbolDocView::Details
                                              : SymbolDocView::Implementation;
        default: return SymbolDocView::Details;
    }
}

auto doc_label(SymbolDocView view) -> std::string_view {
    switch(view) {
        case SymbolDocView::Declaration: return "Declaration";
        case SymbolDocView::Implementation: return "Implementation";
        case SymbolDocView::Details: return "Details";
    }
    return "Details";
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
        if(auto* sym = extract::lookup_symbol(model, sym_id);
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
            for(auto* module: extract::find_modules_by_name(model, key)) {
                for(auto sym_id: module->symbols) {
                    if(!seen.insert(sym_id).second) {
                        continue;
                    }
                    if(auto* sym = extract::lookup_symbol(model, sym_id);
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
            if(auto* sym = extract::lookup_symbol(model, sym_id);
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
            item.fragments.push_back(make_code(label.empty() ? sym->qualified_name : label));
        }
        list.items.push_back(std::move(item));
    }
    return list;
}

auto make_external_source_link(const config::TaskConfig& config,
                               const std::string& file_path,
                               std::uint32_t line = 0) -> std::optional<std::string> {
    if(!config.project.source_base.has_value()) {
        return std::nullopt;
    }

    auto base = config.project.source_base->empty()
                    ? std::string_view{}
                    : std::string_view(*config.project.source_base);
    auto trimmed = trim_ascii(base);
    if(trimmed.empty()) {
        return std::nullopt;
    }

    while(!trimmed.empty() && trimmed.back() == '/') {
        trimmed.remove_suffix(1);
    }

    if(trimmed.empty()) {
        return std::nullopt;
    }

    std::string url;
    url.reserve(trimmed.size() + file_path.size() + 32);

    url.append(trimmed);
    url.push_back('/');
    url.append(make_source_relative(file_path, config.project_root));

    if(line > 0) {
        url.append("#L");
        url.append(std::to_string(line));
    }

    return url;
}

auto make_source_link_target(const extract::SourceLocation& location,
                             const config::TaskConfig& config,
                             const LinkResolver& links,
                             std::string_view current_page_path) -> LinkTarget {
    auto label = make_source_relative(location.file, config.project_root) + ":" +
                 std::to_string(location.line);

    if(auto external_url = make_external_source_link(config, location.file, location.line);
       external_url.has_value()) {
        return LinkTarget{
            .label = std::move(label),
            .target = std::move(*external_url),
            .code_style = false,
        };
    }

    if(auto* target_path = links.resolve(location.file)) {
        return make_link_target(current_page_path, std::move(label), *target_path, true);
    }
    return LinkTarget{
        .label = std::move(label),
        .target = {},
        .code_style = true,
    };
}

auto push_location_paragraph(std::vector<MarkdownNode>& nodes, std::string label, LinkTarget target)
    -> void {
    Paragraph paragraph;
    paragraph.fragments.push_back(make_text(std::move(label)));
    if(target.target.empty()) {
        paragraph.fragments.push_back(make_code(std::move(target.label)));
    } else {
        paragraph.fragments.push_back(
            make_link(std::move(target.label), std::move(target.target), target.code_style));
    }
    nodes.push_back(MarkdownNode{std::move(paragraph)});
}

auto build_symbol_source_locations(const extract::SymbolInfo& sym,
                                   const config::TaskConfig& config,
                                   const LinkResolver& links,
                                   std::string_view current_page_path)
    -> std::vector<MarkdownNode> {
    std::vector<MarkdownNode> nodes;
    if(sym.declaration_location.is_known()) {
        push_location_paragraph(
            nodes,
            "Declaration: ",
            make_source_link_target(sym.declaration_location, config, links, current_page_path));
    }
    if(sym.definition_location.has_value() && sym.definition_location->is_known()) {
        push_location_paragraph(
            nodes,
            "Definition: ",
            make_source_link_target(*sym.definition_location, config, links, current_page_path));
    }
    return nodes;
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
            if(auto* target_path = links.resolve_module(mod->name)) {
                if(seen.insert(*target_path).second) {
                    results.push_back(make_link_target(current_page_path,
                                                       "Module " + mod->name,
                                                       *target_path,
                                                       true));
                }
            }
            return;
        }
        if(auto* target_path = links.resolve(file_path)) {
            if(seen.insert(*target_path).second) {
                results.push_back(make_link_target(current_page_path,
                                                   make_source_relative(file_path, project_root),
                                                   *target_path,
                                                   true));
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
            ns_name.empty() ? std::string{"Declaration"} : std::string{"Namespace "} + ns_name;
        return make_link_target(current_page_path, std::move(label), *target_path, true);
    }

    auto ns_name = !sym.enclosing_namespace.empty() ? sym.enclosing_namespace
                                                    : namespace_of(sym.qualified_name);
    if(ns_name.empty()) {
        return std::nullopt;
    }
    if(auto* target_path = links.resolve_namespace(ns_name)) {
        return make_link_target(current_page_path, "Namespace " + ns_name, *target_path, true);
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

        const std::string* target_path = nullptr;
        if(colon != std::string::npos) {
            auto prefix = linked.substr(0, colon);
            if(prefix == "module") {
                target_path = links.resolve_module(entity_name);
            } else if(prefix == "namespace") {
                target_path = links.resolve_namespace(entity_name);
            }
        }
        if(target_path == nullptr) {
            target_path = links.resolve(entity_name);
        }

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

}  // namespace clore::generate
