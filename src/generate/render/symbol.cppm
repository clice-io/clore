export module generate:symbol;

import std;
import :markdown;
import :model;
import :common;
import :diagram;
import config;
import extract;

export namespace clore::generate {

struct SymbolDocPlan {
    const extract::SymbolInfo* symbol = nullptr;
    std::string index_path;
    std::vector<SymbolDocPlan> children;
};

struct PageDocLayout {
    std::vector<SymbolDocPlan> type_docs;
    std::vector<SymbolDocPlan> variable_docs;
    std::vector<SymbolDocPlan> function_docs;
    std::unordered_map<std::string, std::string> index_paths;
};

template <typename Visitor>
auto for_each_symbol_doc_group(const PageDocLayout& layout, Visitor&& visitor) -> void {
    visitor(layout.type_docs);
    visitor(layout.variable_docs);
    visitor(layout.function_docs);
}

auto normalize_frontmatter_title(std::string_view page_title) -> std::string;

auto page_supports_symbol_subpages(const PagePlan& plan) -> bool;

auto build_page_doc_layout(const PagePlan& plan, const extract::ProjectModel& model)
    -> PageDocLayout;

auto find_doc_index_path(const PageDocLayout& layout, std::string_view qualified_name)
    -> const std::string*;

auto add_symbol_doc_links(std::vector<MarkdownNode>& nodes,
                          std::string_view current_page_path,
                          const PageDocLayout& layout,
                          const extract::SymbolInfo& sym,
                          SymbolDocView view) -> void;

auto append_type_member_sections(std::vector<MarkdownNode>& nodes,
                                 const extract::SymbolInfo& sym,
                                 const config::TaskConfig& config,
                                 const extract::ProjectModel& model,
                                 const SymbolAnalysisStore& analyses,
                                 const PagePlan& plan,
                                 const LinkResolver& links,
                                 const PageDocLayout& layout,
                                 std::string_view current_page_path,
                                 std::uint8_t level) -> void;

auto append_symbol_doc_pages(std::vector<GeneratedPage>& pages,
                             const std::vector<SymbolDocPlan>& doc_plans,
                             const PagePlan& owner_plan,
                             const config::TaskConfig& config,
                             const extract::ProjectModel& model,
                             const std::unordered_map<std::string, std::string>& outputs,
                             const SymbolAnalysisStore& analyses,
                             const LinkResolver& links,
                             const PageDocLayout& layout) -> std::expected<void, RenderError>;

}  // namespace clore::generate

namespace clore::generate {

namespace {

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
    while(!slug.empty() && slug.front() == '-') {
        slug.erase(slug.begin());
    }
    while(!slug.empty() && slug.back() == '-') {
        slug.pop_back();
    }
    if(slug.empty()) {
        slug = "unnamed";
    }
    return slug;
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

auto collect_documentable_children(const extract::ProjectModel& model,
                                   const extract::SymbolInfo& sym)
    -> std::vector<const extract::SymbolInfo*> {
    std::vector<const extract::SymbolInfo*> children;
    children.reserve(sym.children.size());
    for(auto child_id: sym.children) {
        auto* child = extract::lookup_symbol(model, child_id);
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
    if(is_variable_kind(sym.kind)) {
        return false;
    }
    if(is_function_kind(sym.kind)) {
        auto has_dependency_axis = !sym.calls.empty() || !sym.references.empty();
        auto has_usage_axis = !sym.called_by.empty() || !sym.referenced_by.empty();
        return has_dependency_axis && has_usage_axis;
    }
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
    for(const auto& child: plan.children) {
        register_symbol_doc_plan(layout, child);
    }
}

template <typename Predicate>
auto collect_member_symbols(const extract::ProjectModel& model,
                            const extract::SymbolInfo& sym,
                            Predicate&& predicate) -> std::vector<const extract::SymbolInfo*> {
    std::vector<const extract::SymbolInfo*> members;
    members.reserve(sym.children.size());

    for(auto child_id: sym.children) {
        auto* child = extract::lookup_symbol(model, child_id);
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

    return members;
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

auto add_symbol_fallback_content(std::vector<MarkdownNode>& nodes,
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

auto resolved_snippet(const extract::SymbolInfo& sym) -> const std::string* {
    if(!trim_ascii(sym.source_snippet).empty()) {
        return &sym.source_snippet;
    }

    static std::mutex mutex;
    std::lock_guard lock(mutex);
    if(!trim_ascii(sym.source_snippet).empty()) {
        return &sym.source_snippet;
    }
    if(extract::resolve_source_snippet(const_cast<extract::SymbolInfo&>(sym)) &&
       !trim_ascii(sym.source_snippet).empty()) {
        return &sym.source_snippet;
    }
    return nullptr;
}

auto declaration_snippet(const extract::SymbolInfo& sym) -> std::optional<std::string> {
    if(is_function_kind(sym.kind) && !trim_ascii(sym.signature).empty()) {
        auto declaration = std::string(trim_ascii(sym.signature));
        if(!declaration.ends_with(';')) {
            declaration.push_back(';');
        }
        // Skip meaningless signatures with no parameters: "ClassName();" or "void();"
        auto lparen = declaration.find('(');
        auto rparen = declaration.find(')', lparen);
        if(lparen != std::string::npos && rparen != std::string::npos &&
           rparen + 1 == declaration.size() - 1 && declaration.back() == ';') {
            auto between = std::string_view(declaration).substr(lparen + 1, rparen - lparen - 1);
            if(std::ranges::all_of(between, [](char c) { return std::isspace(static_cast<unsigned char>(c)); })) {
                return std::nullopt;
            }
        }
        return declaration;
    }

    if(auto* snippet = resolved_snippet(sym)) {
        return *snippet;
    }
    return std::nullopt;
}

auto implementation_snippet(const extract::SymbolInfo& sym) -> const std::string* {
    return resolved_snippet(sym);
}

auto append_symbol_snippet(std::vector<MarkdownNode>& nodes,
                           const extract::SymbolInfo& sym,
                           const PagePlan& plan,
                           std::uint8_t level) -> void {
    if(plan.page_type == PageType::Namespace) {
        auto snippet = declaration_snippet(sym);
        if(!snippet.has_value()) {
            return;
        }
        auto section = make_section(SemanticKind::Section, {}, "Declaration", level);
        section->children.push_back(make_code_fence("cpp", std::move(*snippet)));
        nodes.push_back(MarkdownNode{std::move(section)});
        return;
    }

    auto* snippet = implementation_snippet(sym);
    if(snippet == nullptr) {
        return;
    }
    auto section = make_section(SemanticKind::Section, {}, "Implementation", level);
    section->children.push_back(make_code_fence("cpp", *snippet));
    nodes.push_back(MarkdownNode{std::move(section)});
}

auto append_type_structure_sections(std::vector<MarkdownNode>& nodes,
                                    const SymbolDocPlan& doc_plan,
                                    const config::TaskConfig& config,
                                    const extract::ProjectModel& model,
                                    const SymbolAnalysisStore& analyses,
                                    const PagePlan& owner_plan,
                                    const LinkResolver& links,
                                    const PageDocLayout& layout,
                                    std::string_view current_page_path) -> void {
    if(doc_plan.symbol == nullptr || !is_type_kind(doc_plan.symbol->kind)) {
        return;
    }

    std::vector<const extract::SymbolInfo*> base_symbols;
    for(auto base_id: doc_plan.symbol->bases) {
        if(auto* base = extract::lookup_symbol(model, base_id)) {
            base_symbols.push_back(base);
        }
    }
    std::sort(base_symbols.begin(),
              base_symbols.end(),
              [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  return lhs->qualified_name < rhs->qualified_name;
              });

    std::vector<const extract::SymbolInfo*> derived_symbols;
    for(auto derived_id: doc_plan.symbol->derived) {
        if(auto* derived = extract::lookup_symbol(model, derived_id)) {
            derived_symbols.push_back(derived);
        }
    }
    std::sort(derived_symbols.begin(),
              derived_symbols.end(),
              [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  return lhs->qualified_name < rhs->qualified_name;
              });

    auto member_type_symbols =
        collect_member_symbols(model, *doc_plan.symbol, [](const extract::SymbolInfo& child) {
            return is_type_kind(child.kind);
        });
    auto member_variable_symbols =
        collect_member_symbols(model, *doc_plan.symbol, [](const extract::SymbolInfo& child) {
            return is_variable_kind(child.kind);
        });
    auto member_function_symbols =
        collect_member_symbols(model, *doc_plan.symbol, [](const extract::SymbolInfo& child) {
            return is_function_kind(child.kind);
        });

    auto edge_count = base_symbols.size() + derived_symbols.size() + member_type_symbols.size() +
                      member_variable_symbols.size() + member_function_symbols.size();
    auto node_count = 1 + edge_count;
    if(should_emit_mermaid(node_count, edge_count)) {
        auto type_label = short_name_of(doc_plan.symbol->qualified_name);
        if(type_label.empty()) {
            type_label = doc_plan.symbol->name.empty() ? doc_plan.symbol->qualified_name
                                                       : doc_plan.symbol->name;
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
                    label =
                        symbols[i]->name.empty() ? symbols[i]->qualified_name : symbols[i]->name;
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

        auto section = make_section(SemanticKind::Section, {}, "Structure Diagram", 2);
        section->children.push_back(make_mermaid(std::move(diagram)));
        nodes.push_back(MarkdownNode{std::move(section)});
    }

    nodes.push_back(MarkdownNode{
        build_list_section("Base Types",
                           2,
                           build_symbol_link_list(base_symbols, current_page_path, links, true))});
    nodes.push_back(MarkdownNode{build_list_section(
        "Derived Types",
        2,
        build_symbol_link_list(derived_symbols, current_page_path, links, true))});
    append_type_member_sections(nodes,
                                *doc_plan.symbol,
                                config,
                                model,
                                analyses,
                                owner_plan,
                                links,
                                layout,
                                current_page_path,
                                2);
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
        if(auto* sym = extract::lookup_symbol(model, id)) {
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

auto append_symbol_context_links(std::vector<MarkdownNode>& nodes,
                                 const extract::SymbolInfo& sym,
                                 const config::TaskConfig& config,
                                 const extract::ProjectModel& model,
                                 const PagePlan& plan,
                                 const LinkResolver& links,
                                 const PageDocLayout& layout,
                                 std::string_view current_page_path) -> void {
    if(plan.page_type == PageType::Namespace) {
        push_link_paragraph(
            nodes,
            "Implementations: ",
            find_implementation_pages(sym, model, links, current_page_path, config.project_root));
    } else {
        push_optional_link_paragraph(nodes,
                                     "Declaration: ",
                                     find_declaration_page(sym, links, current_page_path));
    }
    add_symbol_doc_links(nodes, current_page_path, layout, sym, symbol_doc_view_for(plan, sym));
}

auto append_embedded_symbol_content(std::vector<MarkdownNode>& nodes,
                                    const extract::SymbolInfo& sym,
                                    const config::TaskConfig& config,
                                    const extract::ProjectModel& model,
                                    const SymbolAnalysisStore& analyses,
                                    const PagePlan& plan,
                                    const LinkResolver& links,
                                    const PageDocLayout& layout,
                                    std::string_view current_page_path,
                                    std::uint8_t level) -> void {
    auto entity = make_section(symbol_semantic_kind(sym),
                               sym.qualified_name,
                               "`" + sym.qualified_name + "`",
                               level,
                               false);
    auto locations = build_symbol_source_locations(sym, config);
    for(auto& node: locations) {
        entity->children.push_back(std::move(node));
    }

    append_symbol_context_links(entity->children,
                                sym,
                                config,
                                model,
                                plan,
                                links,
                                layout,
                                current_page_path);
    append_symbol_snippet(entity->children, sym, plan, level + 1);
    add_symbol_analysis_sections(entity->children, analyses, plan, sym, level + 1);

    if(is_type_kind(sym.kind)) {
        append_type_member_sections(entity->children,
                                    sym,
                                    config,
                                    model,
                                    analyses,
                                    plan,
                                    links,
                                    layout,
                                    current_page_path,
                                    level + 1);
    }

    nodes.push_back(MarkdownNode{std::move(entity)});
}

auto append_member_section(std::vector<MarkdownNode>& nodes,
                           std::string heading,
                           const std::vector<const extract::SymbolInfo*>& members,
                           const config::TaskConfig& config,
                           const extract::ProjectModel& model,
                           const SymbolAnalysisStore& analyses,
                           const PagePlan& plan,
                           const LinkResolver& links,
                           const PageDocLayout& layout,
                           std::string_view current_page_path,
                           std::uint8_t level) -> void {
    auto section = make_section(SemanticKind::Section, {}, std::move(heading), level);
    for(const auto* member: members) {
        if(member == nullptr) {
            continue;
        }
        append_embedded_symbol_content(section->children,
                                       *member,
                                       config,
                                       model,
                                       analyses,
                                       plan,
                                       links,
                                       layout,
                                       current_page_path,
                                       level + 1);
    }
    nodes.push_back(MarkdownNode{std::move(section)});
}

auto render_document_page(std::string relative_path,
                          Frontmatter frontmatter,
                          SemanticSectionPtr root) -> std::expected<GeneratedPage, RenderError> {
    MarkdownDocument document;
    auto page_title = frontmatter.title;
    document.frontmatter = std::move(frontmatter);
    document.children.push_back(MarkdownNode{std::move(root)});

    auto content = render_markdown(document);
    if(content.empty()) {
        return std::unexpected(RenderError{
            .message = std::format("rendered markdown is empty for '{}'", relative_path)});
    }

    return GeneratedPage{
        .title = std::move(page_title),
        .relative_path = std::move(relative_path),
        .content = std::move(content),
    };
}

auto build_symbol_frontmatter(const extract::SymbolInfo& sym,
                              std::string_view title,
                              const std::string* output,
                              std::string_view fallback_description) -> Frontmatter {
    auto description =
        output != nullptr ? clore::support::extract_first_plain_paragraph(*output) : std::string{};
    if(description.empty() && !trim_ascii(sym.doc_comment).empty()) {
        description = clore::support::extract_first_plain_paragraph(sym.doc_comment);
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

auto render_symbol_page(const SymbolDocPlan& doc_plan,
                        const PagePlan& owner_plan,
                        const config::TaskConfig& config,
                        const extract::ProjectModel& model,
                        const std::unordered_map<std::string, std::string>&,
                        const SymbolAnalysisStore& analyses,
                        const LinkResolver& links,
                        const PageDocLayout& layout) -> std::expected<GeneratedPage, RenderError> {
    if(doc_plan.symbol == nullptr) {
        return std::unexpected(RenderError{.message = "symbol page missing symbol"});
    }

    auto view = symbol_doc_view_for(owner_plan, *doc_plan.symbol);
    auto* output = owner_plan.page_type == PageType::Namespace
                       ? analysis_overview_markdown(analyses, *doc_plan.symbol)
                       : analysis_details_markdown(analyses, *doc_plan.symbol);

    auto heading = "`" + doc_plan.symbol->qualified_name + "`";
    auto root = make_section(symbol_semantic_kind(*doc_plan.symbol),
                             doc_plan.symbol->qualified_name,
                             heading,
                             1,
                             false);
    push_owner_link(root->children, doc_plan.index_path, owner_plan);
    add_symbol_doc_links(root->children, doc_plan.index_path, layout, *doc_plan.symbol, view);

    auto locations = build_symbol_source_locations(*doc_plan.symbol, config);
    for(auto& node: locations) {
        root->children.push_back(std::move(node));
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

    append_symbol_snippet(root->children, *doc_plan.symbol, owner_plan, 2);
    add_symbol_fallback_content(root->children, *doc_plan.symbol, output);
    add_symbol_analysis_sections(root->children, analyses, owner_plan, *doc_plan.symbol, 2);

    if(is_type_kind(doc_plan.symbol->kind)) {
        append_type_structure_sections(root->children,
                                       doc_plan,
                                       config,
                                       model,
                                       analyses,
                                       owner_plan,
                                       links,
                                       layout,
                                       doc_plan.index_path);
    } else if(is_function_kind(doc_plan.symbol->kind)) {
        append_relation_section(root->children,
                                "Calls",
                                2,
                                doc_plan.symbol->calls,
                                model,
                                doc_plan.index_path,
                                links);
        append_relation_section(root->children,
                                "Called By",
                                2,
                                doc_plan.symbol->called_by,
                                model,
                                doc_plan.index_path,
                                links);
    } else {
        append_relation_section(root->children,
                                "References",
                                2,
                                doc_plan.symbol->references,
                                model,
                                doc_plan.index_path,
                                links);
        append_relation_section(root->children,
                                "Referenced By",
                                2,
                                doc_plan.symbol->referenced_by,
                                model,
                                doc_plan.index_path,
                                links);
    }

    if(!doc_plan.children.empty()) {
        root->children.push_back(MarkdownNode{
            build_list_section("Nested Pages",
                               2,
                               build_child_doc_list(doc_plan.children, doc_plan.index_path))});
    }

    return render_document_page(doc_plan.index_path,
                                build_symbol_frontmatter(*doc_plan.symbol,
                                                         doc_plan.symbol->qualified_name,
                                                         output,
                                                         std::string(doc_label(view)) + " page"),
                                std::move(root));
}

}  // namespace

auto find_doc_index_path(const PageDocLayout& layout, std::string_view qualified_name)
    -> const std::string* {
    auto it = layout.index_paths.find(std::string(qualified_name));
    return it != layout.index_paths.end() ? &it->second : nullptr;
}

auto add_symbol_doc_links(std::vector<MarkdownNode>& nodes,
                          std::string_view current_page_path,
                          const PageDocLayout& layout,
                          const extract::SymbolInfo& sym,
                          SymbolDocView view) -> void {
    std::vector<LinkTarget> targets;
    if(auto* index_path = find_doc_index_path(layout, sym.qualified_name);
       index_path != nullptr && *index_path != current_page_path) {
        targets.push_back(
            make_link_target(current_page_path, std::string(doc_label(view)), *index_path));
    }
    push_link_paragraph(nodes, "Docs: ", targets);
}

auto append_type_member_sections(std::vector<MarkdownNode>& nodes,
                                 const extract::SymbolInfo& sym,
                                 const config::TaskConfig& config,
                                 const extract::ProjectModel& model,
                                 const SymbolAnalysisStore& analyses,
                                 const PagePlan& plan,
                                 const LinkResolver& links,
                                 const PageDocLayout& layout,
                                 std::string_view current_page_path,
                                 std::uint8_t level) -> void {
    if(!is_type_kind(sym.kind)) {
        return;
    }

    append_member_section(nodes,
                          "Member Types",
                          collect_member_symbols(model,
                                                 sym,
                                                 [](const extract::SymbolInfo& child) {
                                                     return is_type_kind(child.kind);
                                                 }),
                          config,
                          model,
                          analyses,
                          plan,
                          links,
                          layout,
                          current_page_path,
                          level);
    append_member_section(nodes,
                          "Member Variables",
                          collect_member_symbols(model,
                                                 sym,
                                                 [](const extract::SymbolInfo& child) {
                                                     return is_variable_kind(child.kind);
                                                 }),
                          config,
                          model,
                          analyses,
                          plan,
                          links,
                          layout,
                          current_page_path,
                          level);
    append_member_section(nodes,
                          "Member Functions",
                          collect_member_symbols(model,
                                                 sym,
                                                 [](const extract::SymbolInfo& child) {
                                                     return is_function_kind(child.kind);
                                                 }),
                          config,
                          model,
                          analyses,
                          plan,
                          links,
                          layout,
                          current_page_path,
                          level);
}

auto normalize_frontmatter_title(std::string_view page_title) -> std::string {
    auto plain = strip_inline_markdown(page_title);
    if(!plain.empty()) {
        return plain;
    }
    return std::string(trim_ascii(page_title));
}

auto page_supports_symbol_subpages(const PagePlan& plan) -> bool {
    return plan.page_type == PageType::Namespace || plan.page_type == PageType::Module;
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

    auto page_symbols = [&]() -> std::vector<const extract::SymbolInfo*> {
        if(plan.page_type == PageType::Namespace) {
            return collect_namespace_symbols(model,
                                             plan.owner_keys.front(),
                                             [](const extract::SymbolInfo&) { return true; });
        }
        return collect_implementation_symbols(plan, model, [](const extract::SymbolInfo&) {
            return true;
        });
    }();

    std::vector<const extract::SymbolInfo*> type_symbols;
    std::vector<const extract::SymbolInfo*> variable_symbols;
    std::vector<const extract::SymbolInfo*> function_symbols;
    type_symbols.reserve(page_symbols.size());
    variable_symbols.reserve(page_symbols.size());
    function_symbols.reserve(page_symbols.size());

    for(const auto* sym: page_symbols) {
        if(sym == nullptr) {
            continue;
        }
        if(is_type_kind(sym->kind)) {
            type_symbols.push_back(sym);
            continue;
        }
        if(is_variable_kind(sym->kind)) {
            variable_symbols.push_back(sym);
            continue;
        }
        if(is_function_kind(sym->kind)) {
            function_symbols.push_back(sym);
        }
    }

    layout.type_docs = build_symbol_doc_plans(plan, model, type_symbols, base_dir);
    layout.variable_docs = build_symbol_doc_plans(plan, model, variable_symbols, base_dir);
    layout.function_docs = build_symbol_doc_plans(plan, model, function_symbols, base_dir);

    for_each_symbol_doc_group(layout, [&](const std::vector<SymbolDocPlan>& group) {
        for(const auto& plan_entry: group) {
            register_symbol_doc_plan(layout, plan_entry);
        }
    });

    return layout;
}

auto append_symbol_doc_pages(std::vector<GeneratedPage>& pages,
                             const std::vector<SymbolDocPlan>& doc_plans,
                             const PagePlan& owner_plan,
                             const config::TaskConfig& config,
                             const extract::ProjectModel& model,
                             const std::unordered_map<std::string, std::string>& outputs,
                             const SymbolAnalysisStore& analyses,
                             const LinkResolver& links,
                             const PageDocLayout& layout) -> std::expected<void, RenderError> {
    for(const auto& doc_plan: doc_plans) {
        auto page = render_symbol_page(doc_plan,
                                       owner_plan,
                                       config,
                                       model,
                                       outputs,
                                       analyses,
                                       links,
                                       layout);
        if(!page.has_value()) {
            return std::unexpected(std::move(page.error()));
        }
        pages.push_back(std::move(*page));

        if(auto nested = append_symbol_doc_pages(pages,
                                                 doc_plan.children,
                                                 owner_plan,
                                                 config,
                                                 model,
                                                 outputs,
                                                 analyses,
                                                 links,
                                                 layout);
           !nested.has_value()) {
            return nested;
        }
    }
    return {};
}

}  // namespace clore::generate
