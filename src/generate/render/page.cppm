export module generate:page;

import std;
import :markdown;
import :model;
import :common;
import :symbol;
import config;
import extract;
import support;

namespace clore::generate {
namespace {

template <typename CollectSymbols, typename AppendLinks, typename AppendDocLinks>
auto append_standard_symbol_sections(std::vector<MarkdownNode>& root_children,
                                     const config::TaskConfig& config,
                                     const extract::ProjectModel& model,
                                     const SymbolAnalysisStore& analyses,
                                     const PagePlan& plan,
                                     const LinkResolver& links,
                                     const PageDocLayout& layout,
                                     CollectSymbols&& collect_symbols,
                                     AppendLinks&& append_links,
                                     AppendDocLinks&& append_doc_links) -> void {
    auto append_symbol_section =
        [&](std::string heading, SemanticKind entity_kind, auto&& predicate) -> void {
        auto section = make_section(SemanticKind::Section, {}, std::move(heading), 2);
        for(const auto* sym: collect_symbols(predicate)) {
            auto entity =
                make_section(entity_kind, sym->qualified_name, "`" + sym->qualified_name + "`", 3);
            auto locations = build_symbol_source_locations(*sym, config);
            for(auto& node: locations) {
                entity->children.push_back(std::move(node));
            }
            append_links(entity->children, *sym);
            append_doc_links(entity->children, *sym);
            add_symbol_analysis_sections(entity->children, analyses, plan, *sym, 4);
            append_type_member_sections(entity->children,
                                        *sym,
                                        config,
                                        model,
                                        analyses,
                                        plan,
                                        links,
                                        layout,
                                        plan.relative_path,
                                        4);
            section->children.push_back(MarkdownNode{std::move(entity)});
        }
        root_children.push_back(MarkdownNode{std::move(section)});
    };

    append_symbol_section("Types", SemanticKind::Type, [](const extract::SymbolInfo& sym) {
        return is_type_kind(sym.kind);
    });
    append_symbol_section("Variables", SemanticKind::Variable, [](const extract::SymbolInfo& sym) {
        return is_variable_kind(sym.kind);
    });
    append_symbol_section("Functions", SemanticKind::Function, [](const extract::SymbolInfo& sym) {
        return is_function_kind(sym.kind);
    });
}

auto append_module_item(BulletList& list,
                        std::string_view current_page_path,
                        const LinkResolver& links,
                        const std::string& module_name) -> void {
    ListItem item;
    if(auto* target_path = links.resolve_module(module_name)) {
        item.fragments.push_back(
            make_link(module_name,
                      make_relative_link_target(current_page_path, *target_path),
                      true));
    } else {
        item.fragments.push_back(make_code(module_name));
    }
    list.items.push_back(std::move(item));
}

auto append_file_item(BulletList& list,
                      std::string_view current_page_path,
                      const LinkResolver& links,
                      std::string label,
                      const std::string& file_path) -> void {
    ListItem item;
    if(auto* target_path = links.resolve(file_path)) {
        item.fragments.push_back(
            make_link(std::move(label),
                      make_relative_link_target(current_page_path, *target_path),
                      true));
    } else {
        item.fragments.push_back(make_code(std::move(label)));
    }
    list.items.push_back(std::move(item));
}

auto prompt_output_of_local_page(const std::unordered_map<std::string, std::string>& outputs,
                                 PromptKind kind) -> const std::string* {
    PromptRequest request{.kind = kind};
    auto it = outputs.find(prompt_request_key(request));
    return it != outputs.end() ? &it->second : nullptr;
}

auto select_primary_description_source_page(
    const PagePlan& plan,
    const std::unordered_map<std::string, std::string>& outputs) -> std::string {
    auto from_prompt = [&](PromptKind kind) -> std::string {
        auto* output = prompt_output_of_local_page(outputs, kind);
        return output != nullptr ? clore::support::extract_first_plain_paragraph(*output)
                                 : std::string{};
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
        if(auto text = clore::support::extract_first_plain_paragraph(value); !text.empty()) {
            return text;
        }
    }
    return {};
}

auto build_frontmatter_page(const PagePlan& plan,
                            std::string_view page_title,
                            std::string_view body,
                            const std::unordered_map<std::string, std::string>& outputs)
    -> Frontmatter {
    auto title = normalize_frontmatter_title(page_title);
    auto description = select_primary_description_source_page(plan, outputs);
    if(description.empty()) {
        description = clore::support::extract_first_plain_paragraph(body);
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
}  // namespace clore::generate

export namespace clore::generate {

auto build_namespace_page_root(const PagePlan& plan,
                               const config::TaskConfig& config,
                               const extract::ProjectModel& model,
                               const std::unordered_map<std::string, std::string>& outputs,
                               const SymbolAnalysisStore& analyses,
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
        root->children.push_back(MarkdownNode{std::move(section)});
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
    root->children.push_back(MarkdownNode{std::move(subnamespaces)});

    append_standard_symbol_sections(
        root->children,
        config,
        model,
        analyses,
        plan,
        links,
        layout,
        [&](auto&& predicate) {
            return collect_namespace_symbols(model,
                                             plan.owner_keys.front(),
                                             std::forward<decltype(predicate)>(predicate));
        },
        [&](std::vector<MarkdownNode>& nodes, const extract::SymbolInfo& sym) {
            push_link_paragraph(nodes,
                                "Implementations: ",
                                find_implementation_pages(sym,
                                                          model,
                                                          links,
                                                          plan.relative_path,
                                                          config.project_root));
        },
        [&](std::vector<MarkdownNode>& nodes, const extract::SymbolInfo& sym) {
            add_symbol_doc_links(nodes,
                                 plan.relative_path,
                                 layout,
                                 sym,
                                 symbol_doc_view_for(plan, sym));
        });

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
                            const SymbolAnalysisStore& analyses,
                            const LinkResolver& links,
                            const PageDocLayout& layout) -> SemanticSectionPtr {
    auto root = make_section(SemanticKind::Module, plan.owner_keys.front(), plan.title, 1, false);
    root->children.push_back(MarkdownNode{
        build_prompt_section("Summary", 2, prompt_output_of(outputs, PromptKind::ModuleSummary))});

    if(auto* module = extract::find_module_by_name(model, plan.owner_keys.front())) {
        root->children.push_back(MarkdownNode{build_list_section("Imports", 2, [&]() {
            BulletList list;
            for(const auto& imported: module->imports) {
                append_module_item(list, plan.relative_path, links, imported);
            }
            return list;
        }())});
        root->children.push_back(MarkdownNode{build_list_section("Imported By", 2, [&]() {
            BulletList list;
            std::vector<std::string> module_names;
            for(const auto& [_, candidate]: model.modules) {
                if(candidate.name == module->name) {
                    continue;
                }
                if(std::ranges::find(candidate.imports, module->name) != candidate.imports.end()) {
                    module_names.push_back(candidate.name);
                }
            }
            std::sort(module_names.begin(), module_names.end());
            module_names.erase(std::unique(module_names.begin(), module_names.end()),
                               module_names.end());
            for(const auto& module_name: module_names) {
                append_module_item(list, plan.relative_path, links, module_name);
            }
            return list;
        }())});
        auto import_diagram = render_import_diagram_code(*module);
        if(!import_diagram.empty()) {
            auto section = make_section(SemanticKind::Section, {}, "Dependency Diagram", 2);
            section->children.push_back(make_mermaid(std::move(import_diagram)));
            root->children.push_back(MarkdownNode{std::move(section)});
        }
    }

    append_standard_symbol_sections(
        root->children,
        config,
        model,
        analyses,
        plan,
        links,
        layout,
        [&](auto&& predicate) {
            return collect_implementation_symbols(plan,
                                                  model,
                                                  std::forward<decltype(predicate)>(predicate));
        },
        [&](std::vector<MarkdownNode>& nodes, const extract::SymbolInfo& sym) {
            push_optional_link_paragraph(nodes,
                                         "Declaration: ",
                                         find_declaration_page(sym, links, plan.relative_path));
        },
        [&](std::vector<MarkdownNode>& nodes, const extract::SymbolInfo& sym) {
            add_symbol_doc_links(nodes,
                                 plan.relative_path,
                                 layout,
                                 sym,
                                 symbol_doc_view_for(plan, sym));
        });

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
                          const std::unordered_map<std::string, std::string>&,
                          const SymbolAnalysisStore& analyses,
                          const LinkResolver& links) -> SemanticSectionPtr {
    auto root = make_section(SemanticKind::File, plan.owner_keys.front(), plan.title, 1, false);

    if(auto file_it = model.files.find(plan.owner_keys.front()); file_it != model.files.end()) {
        root->children.push_back(MarkdownNode{build_list_section("Includes", 2, [&]() {
            BulletList list;
            for(const auto& include: file_it->second.includes) {
                append_file_item(list,
                                 plan.relative_path,
                                 links,
                                 make_source_relative(include, config.project_root),
                                 include);
            }
            return list;
        }())});
        root->children.push_back(MarkdownNode{build_list_section("Included By", 2, [&]() {
            BulletList list;
            std::vector<std::string> file_paths;
            for(const auto& [candidate_path, candidate]: model.files) {
                if(candidate_path == file_it->first) {
                    continue;
                }
                if(std::ranges::find(candidate.includes, file_it->first) !=
                   candidate.includes.end()) {
                    file_paths.push_back(candidate_path);
                }
            }
            std::sort(file_paths.begin(), file_paths.end(), [&](const auto& lhs, const auto& rhs) {
                return make_source_relative(lhs, config.project_root) <
                       make_source_relative(rhs, config.project_root);
            });
            for(const auto& file_path: file_paths) {
                append_file_item(list,
                                 plan.relative_path,
                                 links,
                                 make_source_relative(file_path, config.project_root),
                                 file_path);
            }
            return list;
        }())});
    }
    if(auto dependency_diagram = render_file_dependency_diagram_code(plan, config, model);
       !dependency_diagram.empty()) {
        auto section = make_section(SemanticKind::Section, {}, "Dependency Diagram", 2);
        section->children.push_back(make_mermaid(std::move(dependency_diagram)));
        root->children.push_back(MarkdownNode{std::move(section)});
    }

    append_standard_symbol_sections(
        root->children,
        config,
        model,
        analyses,
        plan,
        links,
        PageDocLayout{},
        [&](auto&& predicate) {
            return collect_implementation_symbols(plan,
                                                  model,
                                                  std::forward<decltype(predicate)>(predicate));
        },
        [&](std::vector<MarkdownNode>& nodes, const extract::SymbolInfo& sym) {
            push_optional_link_paragraph(nodes,
                                         "Declaration: ",
                                         find_declaration_page(sym, links, plan.relative_path));
        },
        [](std::vector<MarkdownNode>&, const extract::SymbolInfo&) {});

    if(auto module_name = find_module_for_file(model, plan.owner_keys.front())) {
        auto section = make_section(SemanticKind::Section, {}, "Module Information", 2);
        Paragraph paragraph;
        paragraph.fragments.push_back(make_text("Module: "));
        if(auto* target_path = links.resolve_module(*module_name)) {
            paragraph.fragments.push_back(
                make_link(*module_name,
                          make_relative_link_target(plan.relative_path, *target_path),
                          true));
        } else {
            paragraph.fragments.push_back(make_code(*module_name));
        }
        section->children.push_back(MarkdownNode{std::move(paragraph)});
        root->children.push_back(MarkdownNode{std::move(section)});
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
                   links.resolve_module(mod_unit.name) != nullptr) {
                    names.push_back(mod_unit.name);
                }
            }
            std::sort(names.begin(), names.end());
            for(const auto& name: names) {
                ListItem item;
                item.fragments.push_back(make_link(
                    name,
                    make_relative_link_target(plan.relative_path, *links.resolve_module(name)),
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
        root->children.push_back(MarkdownNode{std::move(section)});
    }

    return root;
}

auto build_page_root(const PagePlan& plan,
                     const config::TaskConfig& config,
                     const extract::ProjectModel& model,
                     const std::unordered_map<std::string, std::string>& outputs,
                     const SymbolAnalysisStore& analyses,
                     const LinkResolver& links,
                     const PageDocLayout& layout) -> SemanticSectionPtr {
    switch(plan.page_type) {
        case PageType::Index: return build_index_page_root(plan, config, model, outputs, links);
        case PageType::Namespace:
            return build_namespace_page_root(plan, config, model, outputs, analyses, links, layout);
        case PageType::Module:
            return build_module_page_root(plan, config, model, outputs, analyses, links, layout);
        case PageType::File:
            return build_file_page_root(plan, config, model, outputs, analyses, links);
    }
    return make_section(SemanticKind::Section, {}, plan.title, 1, false);
}

auto render_page_bundle(const PagePlan& plan,
                        const config::TaskConfig& config,
                        const extract::ProjectModel& model,
                        const std::unordered_map<std::string, std::string>& prompt_outputs,
                        const SymbolAnalysisStore& analyses,
                        const LinkResolver& links)
    -> std::expected<std::vector<GeneratedPage>, RenderError>;

auto render_page_bundle(const PagePlan& plan,
                        const config::TaskConfig& config,
                        const extract::ProjectModel& model,
                        const std::unordered_map<std::string, std::string>& prompt_outputs,
                        const LinkResolver& links)
    -> std::expected<std::vector<GeneratedPage>, RenderError> {
    return render_page_bundle(plan, config, model, prompt_outputs, SymbolAnalysisStore{}, links);
}

auto render_page_markdown(const PagePlan& plan,
                          const config::TaskConfig& config,
                          const extract::ProjectModel& model,
                          const std::unordered_map<std::string, std::string>& prompt_outputs,
                          const SymbolAnalysisStore& analyses,
                          const LinkResolver& links) -> std::expected<std::string, RenderError> {
    auto bundle = render_page_bundle(plan, config, model, prompt_outputs, analyses, links);
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

auto render_page_markdown(const PagePlan& plan,
                          const config::TaskConfig& config,
                          const extract::ProjectModel& model,
                          const std::unordered_map<std::string, std::string>& prompt_outputs,
                          const LinkResolver& links) -> std::expected<std::string, RenderError> {
    return render_page_markdown(plan, config, model, prompt_outputs, SymbolAnalysisStore{}, links);
}

auto render_page_bundle(const PagePlan& plan,
                        const config::TaskConfig& config,
                        const extract::ProjectModel& model,
                        const std::unordered_map<std::string, std::string>& prompt_outputs,
                        const SymbolAnalysisStore& analyses,
                        const LinkResolver& links)
    -> std::expected<std::vector<GeneratedPage>, RenderError> {
    auto layout = build_page_doc_layout(plan, model);

    MarkdownDocument document;
    document.children.push_back(MarkdownNode{
        build_page_root(plan, config, model, prompt_outputs, analyses, links, layout)});
    auto body = render_markdown(document);
    if(body.empty()) {
        return std::unexpected(RenderError{
            .message = std::format("rendered markdown is empty for '{}'", plan.page_id),
        });
    }

    auto frontmatter = build_frontmatter_page(plan, plan.title, body, prompt_outputs);
    auto page_title = frontmatter.title;
    document.frontmatter = frontmatter;
    std::vector<GeneratedPage> pages;
    pages.push_back(GeneratedPage{
        .title = std::move(page_title),
        .relative_path = plan.relative_path,
        .content = render_markdown(document),
    });

    if(page_supports_symbol_subpages(plan)) {
        std::optional<RenderError> append_error;
        for_each_symbol_doc_group(layout, [&](const std::vector<SymbolDocPlan>& group) {
            if(append_error.has_value()) {
                return;
            }
            if(auto result = append_symbol_doc_pages(pages,
                                                     group,
                                                     plan,
                                                     config,
                                                     model,
                                                     prompt_outputs,
                                                     analyses,
                                                     links,
                                                     layout);
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
    if(!parent.empty()) {
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
