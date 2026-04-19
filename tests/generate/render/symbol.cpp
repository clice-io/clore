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
    auto type_id = type_sym.id;

    auto child_type_sym =
        make_type_symbol(2, "State", "demo::Widget::State", file, "Widget state.");
    child_type_sym.parent = type_id;
    child_type_sym.lexical_parent_kind = extract::SymbolKind::Struct;
    child_type_sym.lexical_parent_name = "demo::Widget";
    auto child_type_id = child_type_sym.id;

    auto member_var_sym = make_type_symbol(3, "value", "demo::Widget::value", file, "");
    member_var_sym.kind = extract::SymbolKind::Variable;
    member_var_sym.parent = type_id;
    member_var_sym.lexical_parent_kind = extract::SymbolKind::Struct;
    member_var_sym.lexical_parent_name = "demo::Widget";
    auto member_var_id = member_var_sym.id;

    auto member_fn_sym =
        make_function_symbol(4, "draw", "demo::Widget::draw", "void draw()", file, "demo");
    member_fn_sym.parent = type_id;
    member_fn_sym.lexical_parent_kind = extract::SymbolKind::Struct;
    member_fn_sym.lexical_parent_name = "demo::Widget";
    auto member_fn_id = member_fn_sym.id;

    auto var_sym = make_type_symbol(5, "count", "demo::count", file, "");
    var_sym.kind = extract::SymbolKind::Variable;
    auto var_id = var_sym.id;

    auto fn_sym = make_function_symbol(6, "render", "demo::render", "void render()", file, "demo");
    auto fn_id = fn_sym.id;

    type_sym.children = {
        child_type_id,
        member_var_id,
        member_fn_id,
    };

    add_symbol(model, std::move(type_sym));
    add_symbol(model, std::move(child_type_sym));
    add_symbol(model, std::move(member_var_sym));
    add_symbol(model, std::move(member_fn_sym));
    add_symbol(model, std::move(var_sym));
    add_symbol(model, std::move(fn_sym));
    add_namespace(model, "demo", {type_id, var_id, fn_id});

    PagePlan plan;
    plan.page_type = PageType::Namespace;
    plan.owner_keys = {"demo"};
    plan.relative_path = "namespaces/demo/index.md";

    auto layout = build_page_doc_layout(plan, model);
    EXPECT_EQ(layout.type_docs.size(), 1u);
    EXPECT_EQ(layout.variable_docs.size(), 1u);
    EXPECT_EQ(layout.function_docs.size(), 1u);
    EXPECT_FALSE(layout.type_docs.front().detail_path.empty());
    EXPECT_FALSE(layout.variable_docs.front().detail_path.empty());
    EXPECT_FALSE(layout.function_docs.front().detail_path.empty());
    EXPECT_TRUE(layout.detail_paths.contains("demo::Widget"));
    EXPECT_TRUE(layout.detail_paths.contains("demo::count"));
    EXPECT_TRUE(layout.detail_paths.contains("demo::render"));
}

TEST_CASE(normalize_frontmatter_title_strips_inline_markdown) {
    EXPECT_EQ(normalize_frontmatter_title("`Widget`"), "Widget");
    EXPECT_EQ(normalize_frontmatter_title("**Bold**"), "Bold");
}

};  // TEST_SUITE(generate_render_symbol)
