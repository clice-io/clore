#include "generate/prelude.h"

import config;
import extract;
import generate;

#include "generate/support.h"

using namespace clore::generate;

TEST_SUITE(generate_render_symbol) {

TEST_CASE(page_supports_symbol_subpages_namespace_and_module) {
    PagePlan plan;
    plan.page_type = PageType::Namespace;
    EXPECT_TRUE(page_supports_symbol_subpages(plan));

    plan.page_type = PageType::Module;
    EXPECT_TRUE(page_supports_symbol_subpages(plan));

    plan.page_type = PageType::File;
    EXPECT_FALSE(page_supports_symbol_subpages(plan));

    plan.page_type = PageType::Index;
    EXPECT_FALSE(page_supports_symbol_subpages(plan));
}

TEST_CASE(build_page_doc_layout_empty_for_file_page) {
    extract::ProjectModel model;
    PagePlan plan;
    plan.page_type = PageType::File;
    plan.relative_path = "files/src/math.md";

    auto layout = build_page_doc_layout(plan, model);
    EXPECT_TRUE(layout.type_docs.empty());
    EXPECT_TRUE(layout.variable_docs.empty());
    EXPECT_TRUE(layout.function_docs.empty());
}

TEST_CASE(build_page_doc_layout_groups_symbols) {
    extract::ProjectModel model;
    auto file = "test.cpp";

    auto type_sym = make_type_symbol(1, "Widget", "demo::Widget", file, "A widget.");
    auto child_type_sym =
        make_type_symbol(2, "State", "demo::Widget::State", file, "Widget state.");
    auto member_fn_sym =
        make_function_symbol(3, "draw", "demo::Widget::draw", "void draw()", file, "demo");
    auto var_sym = make_type_symbol(3, "count", "demo::count", file, "");
    var_sym.kind = extract::SymbolKind::Variable;
    type_sym.children = {
        extract::SymbolID{.hash = 2},
        extract::SymbolID{.hash = 3},
        extract::SymbolID{.hash = 4},
    };
    var_sym.id = extract::SymbolID{.hash = 4};

    add_symbol(model, std::move(type_sym));
    add_symbol(model, std::move(child_type_sym));
    add_symbol(model, std::move(member_fn_sym));
    add_symbol(model, std::move(var_sym));
    add_namespace(model,
                  "demo",
                  {extract::SymbolID{.hash = 1},
                   extract::SymbolID{.hash = 2},
                   extract::SymbolID{.hash = 3},
                   extract::SymbolID{.hash = 4}});

    PagePlan plan;
    plan.page_type = PageType::Namespace;
    plan.owner_keys = {"demo"};
    plan.relative_path = "namespaces/demo/index.md";

    auto layout = build_page_doc_layout(plan, model);
    EXPECT_EQ(layout.type_docs.size(), 1u);
    EXPECT_TRUE(layout.function_docs.empty());
    EXPECT_TRUE(layout.variable_docs.empty());
}

TEST_CASE(normalize_frontmatter_title_strips_inline_markdown) {
    EXPECT_EQ(normalize_frontmatter_title("`Widget`"), "Widget");
    EXPECT_EQ(normalize_frontmatter_title("**Bold**"), "Bold");
}

};  // TEST_SUITE(generate_render_symbol)
