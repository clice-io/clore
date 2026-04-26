#include "generate/prelude.h"

import config;
import extract;
import generate;

#include "generate/support.h"

TEST_SUITE(generate_page) {

TEST_CASE(render_page_markdown_relativizes_file_links_in_index) {
    ScopedTempDir temp("file_links");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    auto file_path = (temp.path / "src" / "math.cpp").generic_string();
    model.files.emplace(file_path,
                        extract::FileInfo{
                            .path = file_path,
                            .symbols = {},
                            .includes = {},
                        });

    PagePlan index_plan{
        .page_id = "index",
        .page_type = PageType::Index,
        .title = "API Reference",
        .relative_path = "index.md",
    };
    PagePlan file_plan{
        .page_id = "file:" + file_path,
        .page_type = PageType::File,
        .title = "File `src/math.cpp`",
        .relative_path = "files/src/math.md",
        .owner_keys = {file_path},
    };

    PagePlanSet plan_set{
        .plans = {index_plan, file_plan}
    };
    auto links = build_link_resolver(plan_set);
    auto outputs =
        make_prompt_outputs({PromptRequest{.kind = PromptKind::IndexOverview}}, "Overview");

    auto result = render_page_markdown(index_plan, config, model, outputs, links);

    ASSERT_TRUE(result.has_value());
    EXPECT_NE(result->find("(files/src/math.md)"), std::string::npos);
}

TEST_CASE(render_page_markdown_links_type_declarations_and_implementations_bidirectionally) {
    ScopedTempDir temp("type_decl_impl_links");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    model.uses_modules = true;

    auto file = (temp.path / "src" / "math.cppm").generic_string();
    auto widget = make_type_symbol(400, "Widget", "demo::Widget", file, "Widget type.", "demo");
    auto widget_id = widget.id;
    auto widget_key = make_symbol_target_key(widget);
    add_symbol(model, std::move(widget));
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.math",
                   .is_interface = true,
                   .source_file = file,
                   .imports = {},
                   .symbols = {widget_id},
               });
    add_namespace(model, "demo", {widget_id});

    PagePlan namespace_plan{
        .page_id = "namespace:demo",
        .page_type = PageType::Namespace,
        .title = "Namespace `demo`",
        .relative_path = "namespaces/demo/index.md",
        .owner_keys = {"demo"                                             },
        .prompt_requests =
            {
                       PromptRequest{.kind = PromptKind::NamespaceSummary},
                       PromptRequest{.kind = PromptKind::TypeDeclarationSummary, .target_key = widget_key},
                       },
    };
    PagePlan module_plan{
        .page_id = "module:demo.math",
        .page_type = PageType::Module,
        .title = "Module `demo.math`",
        .relative_path = "modules/demo/math/index.md",
        .owner_keys = {"demo.math"},
        .prompt_requests =
            {
                       PromptRequest{.kind = PromptKind::ModuleSummary},
                       PromptRequest{.kind = PromptKind::ModuleArchitecture},
                       PromptRequest{.kind = PromptKind::TypeImplementationSummary,
                              .target_key = widget_key},
                       },
    };

    auto outputs = std::unordered_map<std::string, std::string>{
        {prompt_request_key(PromptRequest{.kind = PromptKind::NamespaceSummary}),                           "Summary"     },
        {prompt_request_key(
             PromptRequest{.kind = PromptKind::TypeDeclarationSummary, .target_key = widget_key}),
         "Declaration"                                                                                                    },
        {prompt_request_key(PromptRequest{.kind = PromptKind::ModuleSummary}),                              "Summary"     },
        {prompt_request_key(PromptRequest{.kind = PromptKind::ModuleArchitecture}),                         "Architecture"},
        {prompt_request_key(PromptRequest{.kind = PromptKind::TypeImplementationSummary,
                                          .target_key = widget_key}),
         "Implementation"                                                                                                 },
    };
    PagePlanSet plan_set{
        .plans = {namespace_plan, module_plan}
    };
    auto links = build_link_resolver(plan_set);

    auto namespace_markdown = render_page_markdown(namespace_plan, config, model, outputs, links);
    auto module_markdown = render_page_markdown(module_plan, config, model, outputs, links);

    ASSERT_TRUE(namespace_markdown.has_value());
    ASSERT_TRUE(module_markdown.has_value());
    EXPECT_NE(namespace_markdown->find("[`Module demo.math`](../../modules/demo/math/index.md)"),
              std::string::npos);
    EXPECT_NE(module_markdown->find("[`Namespace demo`](../../../namespaces/demo/index.md)"),
              std::string::npos);
    EXPECT_EQ(namespace_markdown->find("struct Widget {}"), std::string::npos);
    EXPECT_EQ(module_markdown->find("struct Widget {}"), std::string::npos);
}

TEST_CASE(render_module_page_links_imports_bidirectionally) {
    ScopedTempDir temp("module_import_links");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    model.uses_modules = true;

    auto util_file = (temp.path / "src" / "util.cppm").generic_string();
    auto app_file = (temp.path / "src" / "app.cppm").generic_string();

    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.util",
                   .is_interface = true,
                   .source_file = util_file,
                   .imports = {},
                   .symbols = {},
               });
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.app",
                   .is_interface = true,
                   .source_file = app_file,
                   .imports = {"demo.util"},
                   .symbols = {},
               });

    PagePlan util_plan{
        .page_id = "module:demo.util",
        .page_type = PageType::Module,
        .title = "Module `demo.util`",
        .relative_path = "modules/demo/util/index.md",
        .owner_keys = {"demo.util"},
    };
    PagePlan app_plan{
        .page_id = "module:demo.app",
        .page_type = PageType::Module,
        .title = "Module `demo.app`",
        .relative_path = "modules/demo/app/index.md",
        .owner_keys = {"demo.app"},
    };

    PagePlanSet plan_set{
        .plans = {util_plan, app_plan}
    };
    auto links = build_link_resolver(plan_set);

    auto util_markdown = render_page_markdown(util_plan, config, model, {}, links);
    auto app_markdown = render_page_markdown(app_plan, config, model, {}, links);

    ASSERT_TRUE(util_markdown.has_value());
    ASSERT_TRUE(app_markdown.has_value());
    EXPECT_NE(app_markdown->find("[`demo.util`](../util/index.md)"), std::string::npos);
    EXPECT_NE(util_markdown->find("[`demo.app`](../app/index.md)"), std::string::npos);
}

TEST_CASE(render_page_markdown_emits_file_dependency_diagram_when_graph_is_nontrivial) {
    ScopedTempDir temp("file_dependency_diagram");
    fs::create_directories(temp.path / "src");
    fs::create_directories(temp.path / "include");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    auto file = (temp.path / "src" / "engine.cpp").generic_string();
    auto include = (temp.path / "include" / "engine.hpp").generic_string();

    auto state = make_type_symbol(450, "State", "demo::State", file, "State type.", "demo");
    auto run = make_function_symbol(451, "run", "demo::run", "void run()", file, "demo");
    add_symbol(model, std::move(state));
    add_symbol(model, std::move(run));
    model.files[file].includes = {include};

    PagePlan file_plan{
        .page_id = "file:" + file,
        .page_type = PageType::File,
        .title = "File `src/engine.cpp`",
        .relative_path = "files/src/engine.md",
        .owner_keys = {file},
    };

    PagePlanSet plan_set{.plans = {file_plan}};
    auto links = build_link_resolver(plan_set);
    auto result = render_page_markdown(file_plan, config, model, {}, links);

    ASSERT_TRUE(result.has_value());
    EXPECT_NE(result->find("## Dependency Diagram"), std::string::npos);
    EXPECT_NE(result->find("```mermaid"), std::string::npos);
    EXPECT_NE(result->find("graph LR"), std::string::npos);
}

TEST_CASE(render_page_markdown_omits_file_dependency_diagram_for_tiny_graph) {
    ScopedTempDir temp("file_dependency_diagram_tiny");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    auto file = (temp.path / "src" / "tiny.cpp").generic_string();

    auto run = make_function_symbol(452, "run", "demo::run", "void run()", file, "demo");
    add_symbol(model, std::move(run));

    PagePlan file_plan{
        .page_id = "file:" + file,
        .page_type = PageType::File,
        .title = "File `src/tiny.cpp`",
        .relative_path = "files/src/tiny.md",
        .owner_keys = {file},
    };

    PagePlanSet plan_set{.plans = {file_plan}};
    auto links = build_link_resolver(plan_set);
    auto result = render_page_markdown(file_plan, config, model, {}, links);

    ASSERT_TRUE(result.has_value());
    EXPECT_NE(result->find("# File `src/tiny.cpp`\n"), std::string::npos);
    EXPECT_NE(result->find("## Functions\n"), std::string::npos);
    EXPECT_NE(result->find("### `demo::run`\n"), std::string::npos);
    EXPECT_EQ(result->find("## Dependency Diagram"), std::string::npos);
}

TEST_CASE(render_file_page_links_includes_bidirectionally) {
    ScopedTempDir temp("file_include_links");
    fs::create_directories(temp.path / "src");
    fs::create_directories(temp.path / "include");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    auto header = (temp.path / "include" / "engine.hpp").generic_string();
    auto source = (temp.path / "src" / "engine.cpp").generic_string();

    model.files.emplace(header,
                        extract::FileInfo{
                            .path = header,
                            .symbols = {extract::SymbolID{.hash = 920}},
                            .includes = {},
                        });
    model.files.emplace(source,
                        extract::FileInfo{
                            .path = source,
                            .symbols = {extract::SymbolID{.hash = 921}},
                            .includes = {header},
                        });

    PagePlan header_plan{
        .page_id = "file:" + header,
        .page_type = PageType::File,
        .title = "File `include/engine.hpp`",
        .relative_path = "files/include/engine.hpp.md",
        .owner_keys = {header},
    };
    PagePlan source_plan{
        .page_id = "file:" + source,
        .page_type = PageType::File,
        .title = "File `src/engine.cpp`",
        .relative_path = "files/src/engine.cpp.md",
        .owner_keys = {source},
    };

    PagePlanSet plan_set{
        .plans = {header_plan, source_plan}
    };
    auto links = build_link_resolver(plan_set);

    auto header_markdown = render_page_markdown(header_plan, config, model, {}, links);
    auto source_markdown = render_page_markdown(source_plan, config, model, {}, links);

    ASSERT_TRUE(header_markdown.has_value());
    ASSERT_TRUE(source_markdown.has_value());
    EXPECT_NE(source_markdown->find("[`include/engine.hpp`](../include/engine.hpp.md)"),
              std::string::npos);
    EXPECT_NE(header_markdown->find("[`src/engine.cpp`](../src/engine.cpp.md)"), std::string::npos);
}

TEST_CASE(render_namespace_diagram_keeps_distinct_types_with_same_short_name) {
    ScopedTempDir temp("namespace_diagram_short_name_collision");
    fs::create_directories(temp.path / "src");

    extract::ProjectModel model;
    auto file = (temp.path / "src" / "diagram.cppm").generic_string();

    auto outer_a =
        make_type_symbol(460, "A", "diagram_demo::A", file, "Outer A type.", "diagram_demo");
    auto outer_a_id = outer_a.id;
    auto inner_a = make_type_symbol(461,
                                    "Inner",
                                    "diagram_demo::A::Inner",
                                    file,
                                    "Nested A type.",
                                    "diagram_demo");
    inner_a.parent = outer_a_id;
    inner_a.lexical_parent_name = "diagram_demo::A";
    inner_a.lexical_parent_kind = extract::SymbolKind::Struct;
    auto inner_a_id = inner_a.id;

    auto outer_b =
        make_type_symbol(462, "B", "diagram_demo::B", file, "Outer B type.", "diagram_demo");
    auto outer_b_id = outer_b.id;
    auto inner_b = make_type_symbol(463,
                                    "Inner",
                                    "diagram_demo::B::Inner",
                                    file,
                                    "Nested B type.",
                                    "diagram_demo");
    inner_b.parent = outer_b_id;
    inner_b.lexical_parent_name = "diagram_demo::B";
    inner_b.lexical_parent_kind = extract::SymbolKind::Struct;
    auto inner_b_id = inner_b.id;

    auto widget = make_type_symbol(464,
                                   "Widget",
                                   "diagram_demo::Widget",
                                   file,
                                   "Widget type.",
                                   "diagram_demo");
    auto widget_id = widget.id;

    add_symbol(model, std::move(outer_a));
    add_symbol(model, std::move(inner_a));
    add_symbol(model, std::move(outer_b));
    add_symbol(model, std::move(inner_b));
    add_symbol(model, std::move(widget));
    add_namespace(model,
                  "diagram_demo",
                  {outer_a_id, inner_a_id, outer_b_id, inner_b_id, widget_id});

    auto diagram = render_namespace_diagram_code(model, "diagram_demo");
    auto count_occurrences = [](std::string_view text, std::string_view needle) -> std::size_t {
        std::size_t count = 0;
        std::size_t offset = 0;
        while((offset = text.find(needle, offset)) != std::string_view::npos) {
            ++count;
            offset += needle.size();
        }
        return count;
    };

    EXPECT_FALSE(diagram.empty());
    EXPECT_EQ(count_occurrences(diagram, "[\"Inner\"]"), 2u);
    EXPECT_EQ(count_occurrences(diagram, "[\"Widget\"]"), 1u);
}

TEST_CASE(render_page_bundle_places_complex_type_members_in_nested_doc_page) {
    ScopedTempDir temp("type_member_order");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    auto file = (temp.path / "src" / "widget.cppm").generic_string();

    auto widget = make_type_symbol(500, "Widget", "demo::Widget", file, "Widget type.", "demo");
    auto widget_key = make_symbol_target_key(widget);
    auto nested =
        make_type_symbol(501, "Options", "demo::Widget::Options", file, "Nested options.", "demo");
    nested.access = "public";
    nested.parent = widget.id;
    nested.lexical_parent_name = "demo::Widget";
    nested.lexical_parent_kind = extract::SymbolKind::Struct;

    extract::SymbolInfo state;
    state.id = extract::SymbolID{.hash = 502};
    state.kind = extract::SymbolKind::Variable;
    state.name = "state";
    state.qualified_name = "demo::Widget::state";
    state.access = "private";
    state.parent = widget.id;
    state.lexical_parent_name = "demo::Widget";
    state.lexical_parent_kind = extract::SymbolKind::Struct;
    state.declaration_location = extract::SourceLocation{.file = file, .line = 4, .column = 1};

    auto run = make_function_symbol(503, "run", "demo::Widget::run", "void run()", file, "demo");
    run.kind = extract::SymbolKind::Method;
    run.access = "public";
    run.parent = widget.id;
    run.lexical_parent_name = "demo::Widget";
    run.lexical_parent_kind = extract::SymbolKind::Struct;

    widget.children = {nested.id, state.id, run.id};

    add_symbol(model, std::move(widget));
    add_symbol(model, std::move(nested));
    add_symbol(model, std::move(state));
    add_symbol(model, std::move(run));
    add_namespace(model, "demo", {extract::SymbolID{.hash = 500}, extract::SymbolID{.hash = 501}});

    PagePlan namespace_plan{
        .page_id = "namespace:demo",
        .page_type = PageType::Namespace,
        .title = "Namespace `demo`",
        .relative_path = "namespaces/demo/index.md",
        .owner_keys = {"demo"                                             },
        .prompt_requests =
            {
                       PromptRequest{.kind = PromptKind::NamespaceSummary},
                       PromptRequest{.kind = PromptKind::TypeDeclarationSummary, .target_key = widget_key},
                       },
    };

    auto outputs = std::unordered_map<std::string, std::string>{
        {prompt_request_key(PromptRequest{.kind = PromptKind::NamespaceSummary}),                           "Summary"},
        {prompt_request_key(
             PromptRequest{.kind = PromptKind::TypeDeclarationSummary, .target_key = widget_key}),
         "Declaration"                                                                                               },
    };

    PagePlanSet plan_set{.plans = {namespace_plan}};
    auto links = build_link_resolver(plan_set);
    auto bundle = render_page_bundle(namespace_plan, config, model, outputs, links);

    ASSERT_TRUE(bundle.has_value());
    auto root_page = std::ranges::find_if(*bundle, [&](const GeneratedPage& page) {
        return page.relative_path == namespace_plan.relative_path;
    });
    auto detail_page = std::ranges::find_if(*bundle, [](const GeneratedPage& page) {
        return page.relative_path == "namespaces/demo/types/widget.md";
    });

    ASSERT_TRUE(root_page != bundle->end());
    ASSERT_TRUE(detail_page != bundle->end());
    EXPECT_EQ(root_page->content.find("struct Widget {}"), std::string::npos);
    EXPECT_NE(detail_page->content.find("struct Widget {}"), std::string::npos);
    EXPECT_EQ(detail_page->content.find("void run() { return 0; }"), std::string::npos);
    EXPECT_NE(root_page->content.find("[Declaration](types/widget.md)"), std::string::npos);
    EXPECT_FALSE(std::ranges::any_of(*bundle, [](const GeneratedPage& page) {
        return page.relative_path == "namespaces/demo/types/widget-declaration.md";
    }));

    auto member_types = detail_page->content.find("## Member Types");
    auto member_variables = detail_page->content.find("## Member Variables");
    auto member_functions = detail_page->content.find("## Member Functions");
    ASSERT_TRUE(member_types != std::string::npos);
    ASSERT_TRUE(member_variables != std::string::npos);
    ASSERT_TRUE(member_functions != std::string::npos);
    EXPECT_LT(member_types, member_variables);
    EXPECT_LT(member_variables, member_functions);
    EXPECT_NE(root_page->content.find("#### Member Types"), std::string::npos);
    EXPECT_NE(root_page->content.find("##### `demo::Widget::Options`"), std::string::npos);
    EXPECT_NE(root_page->content.find("#### Member Variables"), std::string::npos);
    EXPECT_NE(root_page->content.find("##### `demo::Widget::state`"), std::string::npos);
    EXPECT_NE(root_page->content.find("#### Member Functions"), std::string::npos);
    EXPECT_NE(root_page->content.find("##### `demo::Widget::run`"), std::string::npos);
    EXPECT_NE(root_page->content.find("void run();"), std::string::npos);
    EXPECT_EQ(root_page->content.find("void run() { return 0; }"), std::string::npos);
    EXPECT_NE(detail_page->content.find("void run();"), std::string::npos);
}

TEST_CASE(render_page_markdown_preserves_prompt_code_fence_markers) {
    ScopedTempDir temp("strip_prompt_code_fence_markers");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    auto file = (temp.path / "src" / "math.cppm").generic_string();
    auto sym = make_function_symbol(700, "run", "demo::run", "void run()", file, "demo");
    auto run_key = make_symbol_target_key(sym);
    auto sym_id = sym.id;
    add_symbol(model, std::move(sym));
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.math",
                   .is_interface = true,
                   .source_file = file,
                   .imports = {},
                   .symbols = {sym_id},
               });

    PagePlan module_plan{
        .page_id = "module:demo.math",
        .page_type = PageType::Module,
        .title = "Module `demo.math`",
        .relative_path = "modules/demo/math/index.md",
        .owner_keys = {"demo.math"                                     },
        .prompt_requests =
            {
                       PromptRequest{.kind = PromptKind::ModuleSummary},
                       PromptRequest{.kind = PromptKind::ModuleArchitecture},
                       },
    };

    auto outputs = std::unordered_map<std::string, std::string>{
        {prompt_request_key(PromptRequest{.kind = PromptKind::ModuleSummary}),      "Summary"     },
        {prompt_request_key(PromptRequest{.kind = PromptKind::ModuleArchitecture}), "Architecture"},
    };

    SymbolAnalysisStore analyses;
    analyses.functions.insert_or_assign(
        run_key,
        FunctionAnalysis{
            .overview_markdown = "Overview note.",
            .details_markdown =
                "Algorithm note with inline marker " "```helper```.\n\n```cpp\nstep();\n```",
            .has_side_effects = true,
            .side_effects = {"Calls `step()`."},
            .reads_from = {},
            .writes_to = {},
            .usage_patterns = {},
        });

    PagePlanSet plan_set{.plans = {module_plan}};
    auto links = build_link_resolver(plan_set);
    auto result = render_page_markdown(module_plan, config, model, outputs, analyses, links);

    ASSERT_TRUE(result.has_value());
    EXPECT_NE(result->find("```helper```"), std::string::npos);
    EXPECT_NE(result->find("```cpp\nstep();\n```"), std::string::npos);
    EXPECT_NE(result->find("helper"), std::string::npos);
    EXPECT_NE(result->find("step();"), std::string::npos);
}

TEST_CASE(render_page_bundle_emits_nested_symbol_docs_for_complex_module_types) {
    ScopedTempDir temp("nested_symbol_docs");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    auto file = (temp.path / "src" / "widget.cppm").generic_string();

    auto widget = make_type_symbol(800, "Widget", "demo::Widget", file, "Widget type.", "demo");
    auto options =
        make_type_symbol(801, "Options", "demo::Widget::Options", file, "Nested options.", "demo");
    options.parent = widget.id;
    options.lexical_parent_name = "demo::Widget";
    options.lexical_parent_kind = extract::SymbolKind::Struct;

    extract::SymbolInfo state;
    state.id = extract::SymbolID{.hash = 802};
    state.kind = extract::SymbolKind::Variable;
    state.name = "state";
    state.qualified_name = "demo::Widget::state";
    state.parent = widget.id;
    state.lexical_parent_name = "demo::Widget";
    state.lexical_parent_kind = extract::SymbolKind::Struct;
    state.declaration_location = extract::SourceLocation{.file = file, .line = 4, .column = 1};

    auto run = make_function_symbol(803, "run", "demo::Widget::run", "void run()", file, "demo");
    run.kind = extract::SymbolKind::Method;
    run.parent = widget.id;
    run.lexical_parent_name = "demo::Widget";
    run.lexical_parent_kind = extract::SymbolKind::Struct;

    widget.children = {options.id, state.id, run.id};

    auto widget_id = widget.id;
    auto widget_key = make_symbol_target_key(widget);
    add_symbol(model, std::move(widget));
    add_symbol(model, std::move(options));
    add_symbol(model, std::move(state));
    add_symbol(model, std::move(run));
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.math",
                   .is_interface = true,
                   .source_file = file,
                   .imports = {},
                   .symbols = {widget_id},
               });

    PagePlan module_plan{
        .page_id = "module:demo.math",
        .page_type = PageType::Module,
        .title = "Module `demo.math`",
        .relative_path = "modules/demo/math/index.md",
        .owner_keys = {"demo.math"},
        .prompt_requests =
            {
                       PromptRequest{.kind = PromptKind::ModuleSummary},
                       PromptRequest{.kind = PromptKind::ModuleArchitecture},
                       PromptRequest{.kind = PromptKind::TypeImplementationSummary,
                              .target_key = widget_key},
                       },
    };

    auto outputs = std::unordered_map<std::string, std::string>{
        {prompt_request_key(PromptRequest{.kind = PromptKind::ModuleSummary}),      "Summary"     },
        {prompt_request_key(PromptRequest{.kind = PromptKind::ModuleArchitecture}), "Architecture"},
        {prompt_request_key(PromptRequest{.kind = PromptKind::TypeImplementationSummary,
                                          .target_key = widget_key}),
         "Implementation summary."                                                                },
    };

    PagePlanSet plan_set{.plans = {module_plan}};
    auto links = build_link_resolver(plan_set);
    auto bundle = render_page_bundle(module_plan, config, model, outputs, links);

    ASSERT_TRUE(bundle.has_value());
    EXPECT_TRUE(std::ranges::any_of(*bundle, [](const GeneratedPage& page) {
        return page.relative_path == "modules/demo/math/index.md";
    }));
    EXPECT_TRUE(std::ranges::any_of(*bundle, [](const GeneratedPage& page) {
        return page.relative_path == "modules/demo/math/types/widget.md";
    }));

    auto root_page = std::ranges::find_if(*bundle, [](const GeneratedPage& page) {
        return page.relative_path == "modules/demo/math/index.md";
    });
    auto detail_page = std::ranges::find_if(*bundle, [](const GeneratedPage& page) {
        return page.relative_path == "modules/demo/math/types/widget.md";
    });
    ASSERT_TRUE(root_page != bundle->end());
    ASSERT_TRUE(detail_page != bundle->end());
    EXPECT_NE(root_page->content.find("[Implementation](types/widget.md)"), std::string::npos);
    EXPECT_FALSE(std::ranges::any_of(*bundle, [](const GeneratedPage& page) {
        return page.relative_path == "modules/demo/math/types/widget-implementation.md";
    }));
    EXPECT_NE(root_page->content.find("#### Member Types"), std::string::npos);
    EXPECT_NE(root_page->content.find("##### `demo::Widget::Options`"), std::string::npos);
    EXPECT_NE(root_page->content.find("#### Member Variables"), std::string::npos);
    EXPECT_NE(root_page->content.find("##### `demo::Widget::state`"), std::string::npos);
    EXPECT_NE(root_page->content.find("#### Member Functions"), std::string::npos);
    EXPECT_NE(root_page->content.find("##### `demo::Widget::run`"), std::string::npos);
    EXPECT_NE(root_page->content.find("void run() { return 0; }"), std::string::npos);
    EXPECT_NE(detail_page->content.find("## Structure Diagram"), std::string::npos);
    EXPECT_NE(detail_page->content.find("```mermaid"), std::string::npos);
    EXPECT_NE(detail_page->content.find("void run() { return 0; }"), std::string::npos);
}

TEST_CASE(render_file_page_embeds_type_member_implementations) {
    ScopedTempDir temp("file_type_member_implementations");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    auto file = (temp.path / "src" / "widget.cpp").generic_string();

    auto widget = make_type_symbol(850, "Widget", "demo::Widget", file, "Widget type.", "demo");
    auto run = make_function_symbol(851, "run", "demo::Widget::run", "void run()", file, "demo");
    run.kind = extract::SymbolKind::Method;
    run.parent = widget.id;
    run.lexical_parent_name = "demo::Widget";
    run.lexical_parent_kind = extract::SymbolKind::Struct;
    widget.children = {run.id};

    auto widget_id = widget.id;
    auto widget_key = make_symbol_target_key(widget);
    add_symbol(model, std::move(widget));
    add_symbol(model, std::move(run));

    PagePlan file_plan{
        .page_id = "file:" + file,
        .page_type = PageType::File,
        .title = "File `src/widget.cpp`",
        .relative_path = "files/src/widget.md",
        .owner_keys = {file},
        .prompt_requests = {},
    };

    SymbolAnalysisStore analyses;
    analyses.types.insert_or_assign(widget_key,
                                    TypeAnalysis{
                                        .overview_markdown = "Widget declaration notes.",
                                        .details_markdown = "Widget implementation notes.",
                                        .invariants = {},
                                        .key_members = {},
                                        .usage_patterns = {},
                                    });

    PagePlanSet plan_set{.plans = {file_plan}};
    auto links = build_link_resolver(plan_set);
    auto markdown = render_page_markdown(file_plan, config, model, {}, analyses, links);

    ASSERT_TRUE(markdown.has_value());
    EXPECT_NE(markdown->find("### `demo::Widget`"), std::string::npos);
    EXPECT_NE(markdown->find("#### Member Functions"), std::string::npos);
    EXPECT_NE(markdown->find("##### `demo::Widget::run`"), std::string::npos);
    EXPECT_NE(markdown->find("void run() { return 0; }"), std::string::npos);
    EXPECT_NE(markdown->find("Widget implementation notes."), std::string::npos);
    EXPECT_EQ(markdown->find("Widget declaration notes."), std::string::npos);
}

TEST_CASE(render_page_markdown_wraps_analysis_list_code_elements) {
    ScopedTempDir temp("analysis_list_code");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    auto file = (temp.path / "src" / "client.cpp").generic_string();

    auto run = make_function_symbol(860, "run", "demo::run", "void run()", file, "demo");
    auto run_key = make_symbol_target_key(run);
    add_symbol(model, std::move(run));
    add_namespace(model, "demo");

    PagePlan file_plan{
        .page_id = "file:" + file,
        .page_type = PageType::File,
        .title = "File `src/client.cpp`",
        .relative_path = "files/src/client.md",
        .owner_keys = {file},
        .prompt_requests = {},
    };

    SymbolAnalysisStore analyses;
    analyses.functions.insert_or_assign(run_key,
                                        FunctionAnalysis{
                                            .overview_markdown = "Overview note.",
                                            .details_markdown = "Delegates to call_llm_async.",
                                            .has_side_effects = true,
                                            .side_effects = {"allocates std::vector<Message>"},
                                            .reads_from = {"request parameter"},
                                            .writes_to = {"returned std::expected<T, LLMError>"},
                                            .usage_patterns = {"Callers chain .or_fail()"},
                                        });

    PagePlanSet plan_set{.plans = {file_plan}};
    auto links = build_link_resolver(plan_set);
    auto markdown = render_page_markdown(file_plan, config, model, {}, analyses, links);

    ASSERT_TRUE(markdown.has_value());
    EXPECT_NE(markdown->find("Delegates to `call_llm_async`."), std::string::npos);
    EXPECT_NE(markdown->find("- allocates `std::vector<Message>`"), std::string::npos);
    EXPECT_NE(markdown->find("- returned `std::expected<T, LLMError>`"), std::string::npos);
    EXPECT_NE(markdown->find("- Callers chain `.or_fail()`"), std::string::npos);
    EXPECT_EQ(markdown->find("- allocates std::vector<Message>"), std::string::npos);
    EXPECT_EQ(markdown->find("- returned std::expected<T, LLMError>"), std::string::npos);
}

TEST_CASE(render_page_bundle_emits_function_docs_only_for_complex_functions) {
    ScopedTempDir temp("complex_function_docs");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    auto file = (temp.path / "src" / "flow.cppm").generic_string();

    auto leaf = make_function_symbol(900, "leaf", "demo::leaf", "void leaf()", file, "demo");
    auto leaf_id = leaf.id;
    auto caller =
        make_function_symbol(901, "caller", "demo::caller", "void caller()", file, "demo");
    auto caller_id = caller.id;
    auto coordinator = make_function_symbol(902,
                                            "coordinator",
                                            "demo::coordinator",
                                            "void coordinator()",
                                            file,
                                            "demo");
    auto coordinator_id = coordinator.id;
    coordinator.calls = {leaf_id};
    coordinator.called_by = {caller_id};

    add_symbol(model, std::move(leaf));
    add_symbol(model, std::move(caller));
    add_symbol(model, std::move(coordinator));
    add_namespace(model, "demo", {leaf_id, caller_id, coordinator_id});

    PagePlan namespace_plan{
        .page_id = "namespace:demo",
        .page_type = PageType::Namespace,
        .title = "Namespace `demo`",
        .relative_path = "namespaces/demo/index.md",
        .owner_keys = {"demo"},
        .prompt_requests = {PromptRequest{.kind = PromptKind::NamespaceSummary}},
    };

    auto outputs = std::unordered_map<std::string, std::string>{
        {prompt_request_key(PromptRequest{.kind = PromptKind::NamespaceSummary}), "Summary"},
    };

    PagePlanSet plan_set{.plans = {namespace_plan}};
    auto links = build_link_resolver(plan_set);
    auto bundle = render_page_bundle(namespace_plan, config, model, outputs, links);

    ASSERT_TRUE(bundle.has_value());
    EXPECT_TRUE(std::ranges::any_of(*bundle, [](const GeneratedPage& page) {
        return page.relative_path == "namespaces/demo/functions/coordinator.md";
    }));
    EXPECT_FALSE(std::ranges::any_of(*bundle, [](const GeneratedPage& page) {
        return page.relative_path == "namespaces/demo/functions/leaf.md";
    }));
    EXPECT_FALSE(std::ranges::any_of(*bundle, [](const GeneratedPage& page) {
        return page.relative_path == "namespaces/demo/functions/caller.md";
    }));
}

TEST_CASE(render_page_markdown_omits_reading_guide_section_from_index) {
    auto config = make_config(fs::temp_directory_path());
    extract::ProjectModel model;

    auto file_path = (fs::temp_directory_path() / "demo.cpp").generic_string();
    model.files.emplace(file_path,
                        extract::FileInfo{
                            .path = file_path,
                            .symbols = {},
                            .includes = {},
                        });

    PagePlan index_plan{
        .page_id = "index",
        .page_type = PageType::Index,
        .title = "API Reference",
        .relative_path = "index.md",
    };
    PagePlan file_plan{
        .page_id = "file:" + file_path,
        .page_type = PageType::File,
        .title = "File `demo.cpp`",
        .relative_path = "files/demo.md",
        .owner_keys = {file_path},
    };

    auto outputs = std::unordered_map<std::string, std::string>{
        {prompt_request_key(PromptRequest{.kind = PromptKind::IndexOverview}),
         "Overview of the generated reference."},
    };

    PagePlanSet plan_set{
        .plans = {index_plan, file_plan}
    };
    auto links = build_link_resolver(plan_set);
    auto result = render_page_markdown(index_plan, config, model, outputs, links);

    ASSERT_TRUE(result.has_value());
    EXPECT_NE(result->find("# API Reference\n"), std::string::npos);
    EXPECT_NE(result->find("Overview of the generated reference."), std::string::npos);
    EXPECT_EQ(result->find("## Reading Guide\n"), std::string::npos);
    EXPECT_NE(result->find("## Files\n"), std::string::npos);
    EXPECT_NE(result->find("(files/demo.md)"), std::string::npos);
}

};  // TEST_SUITE(generate_page)
