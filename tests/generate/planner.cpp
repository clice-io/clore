#include "generate/prelude.h"

import config;
import extract;
import generate;

#include "generate/support.h"

TEST_SUITE(generate_planner) {

TEST_CASE(build_page_plan_set_creates_index_and_file_pages_by_default) {
    ScopedTempDir temp("default_page_plan");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    auto model = make_basic_model(temp.path);

    auto result = build_page_plan_set(config, model);

    ASSERT_TRUE(result.has_value());

    auto index_plan = std::ranges::find_if(result->plans, [](const PagePlan& plan) {
        return plan.page_type == PageType::Index;
    });
    auto file_plan = std::ranges::find_if(result->plans, [](const PagePlan& plan) {
        return plan.page_type == PageType::File;
    });

    ASSERT_TRUE(index_plan != result->plans.end());
    ASSERT_TRUE(file_plan != result->plans.end());
    EXPECT_EQ(index_plan->relative_path, "index.md");
    EXPECT_TRUE(std::ranges::any_of(index_plan->prompt_requests, [](const PromptRequest& request) {
        return request.kind == PromptKind::IndexOverview;
    }));
    EXPECT_TRUE(file_plan->prompt_requests.empty());
}

TEST_CASE(build_page_plan_set_uses_modules_to_emit_modules_only) {
    ScopedTempDir temp("module_plans");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);

    extract::ProjectModel model;
    auto util_file = (temp.path / "src" / "util.cppm").generic_string();
    auto main_file = (temp.path / "src" / "math.cppm").generic_string();

    auto util_symbol = make_function_symbol(10,
                                            "helper",
                                            "demo::util::helper",
                                            "int helper()",
                                            util_file,
                                            "demo::util");
    auto api_symbol = make_function_symbol(11,
                                           "add",
                                           "demo::math::add",
                                           "int add(int lhs, int rhs)",
                                           main_file,
                                           "demo::math");
    auto util_id = util_symbol.id;
    auto api_id = api_symbol.id;
    add_symbol(model, std::move(util_symbol));
    add_symbol(model, std::move(api_symbol));

    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.util",
                   .is_interface = true,
                   .source_file = util_file,
                   .imports = {},
                   .symbols = {util_id},
               });
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.math",
                   .is_interface = true,
                   .source_file = main_file,
                   .imports = {"demo.util"},
                   .symbols = {api_id},
               });

    auto result = build_page_plan_set(config, model);

    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(std::ranges::any_of(result->plans, [](const PagePlan& plan) {
        return plan.page_type == PageType::File;
    }));
    EXPECT_TRUE(std::ranges::any_of(result->plans, [](const PagePlan& plan) {
        return plan.page_id == "module:demo.util";
    }));
    EXPECT_TRUE(std::ranges::any_of(result->plans, [](const PagePlan& plan) {
        return plan.page_id == "module:demo.math";
    }));

    auto pos_util = std::ranges::find(result->generation_order, "module:demo.util");
    auto pos_math = std::ranges::find(result->generation_order, "module:demo.math");
    ASSERT_TRUE(pos_util != result->generation_order.end());
    ASSERT_TRUE(pos_math != result->generation_order.end());
    EXPECT_LT(pos_util, pos_math);
}

TEST_CASE(build_page_plan_set_keeps_parent_only_namespace_pages) {
    ScopedTempDir temp("parent_only_namespace_pages");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);

    extract::ProjectModel model;
    auto file = (temp.path / "src" / "config.cppm").generic_string();
    auto type_symbol = make_type_symbol(200,
                                        "Options",
                                        "demo::config::Options",
                                        file,
                                        "Configuration options.",
                                        "demo::config");
    auto type_id = type_symbol.id;
    add_symbol(model, std::move(type_symbol));

    add_namespace(model, "demo", {}, {"demo::config"});
    add_namespace(model, "demo::config", {type_id});

    auto result = build_page_plan_set(config, model);

    ASSERT_TRUE(result.has_value());
    auto root_plan = std::ranges::find_if(result->plans, [](const PagePlan& plan) {
        return plan.page_id == "namespace:demo";
    });
    ASSERT_TRUE(root_plan != result->plans.end());
    EXPECT_TRUE(std::ranges::find(root_plan->linked_pages, "namespace:demo::config") !=
                root_plan->linked_pages.end());
}

TEST_CASE(build_page_plan_set_keeps_nested_types_on_their_owner_page) {
    ScopedTempDir temp("nested_type_members");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);

    extract::ProjectModel model;
    auto file = (temp.path / "src" / "widget.cppm").generic_string();

    auto outer = make_type_symbol(300, "Outer", "demo::Outer", file, "Outer type.", "demo");
    auto inner =
        make_type_symbol(301, "Inner", "demo::Outer::Inner", file, "Nested helper.", "demo");
    inner.lexical_parent_name = "demo::Outer";
    inner.lexical_parent_kind = extract::SymbolKind::Struct;

    add_symbol(model, std::move(outer));
    add_symbol(model, std::move(inner));
    add_namespace(model, "demo", {extract::SymbolID{.hash = 300}, extract::SymbolID{.hash = 301}});

    auto result = build_page_plan_set(config, model);

    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(std::ranges::any_of(result->plans, [](const PagePlan& plan) {
        return plan.page_id.starts_with("type:");
    }));

    auto namespace_plan = std::ranges::find_if(result->plans, [](const PagePlan& plan) {
        return plan.page_id == "namespace:demo";
    });
    ASSERT_TRUE(namespace_plan != result->plans.end());
    EXPECT_EQ(namespace_plan->prompt_requests.size(), 1u);
    EXPECT_EQ(namespace_plan->prompt_requests[0].kind, PromptKind::NamespaceSummary);
    EXPECT_FALSE(
        std::ranges::any_of(namespace_plan->prompt_requests, [](const PromptRequest& request) {
            return request.kind == PromptKind::TypeDeclarationSummary &&
                   parse_symbol_target_key(request.target_key).qualified_name ==
                       "demo::Outer::Inner";
        }));
}

TEST_CASE(build_page_plan_set_skips_function_local_types_from_page_entries) {
    ScopedTempDir temp("skip_function_local_type_entries");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);

    extract::ProjectModel model;
    auto file = (temp.path / "src" / "client.cppm").generic_string();

    auto exported = make_type_symbol(310, "Client", "demo::Client", file, "Client type.", "demo");
    auto local =
        make_type_symbol(311, "ActiveRunReset", "ActiveRunReset", file, "Local helper.", "demo");
    local.lexical_parent_name = "demo::run";
    local.lexical_parent_kind = extract::SymbolKind::Function;

    add_symbol(model, std::move(exported));
    add_symbol(model, std::move(local));
    add_namespace(model, "demo", {extract::SymbolID{.hash = 310}, extract::SymbolID{.hash = 311}});

    auto result = build_page_plan_set(config, model);

    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(std::ranges::any_of(result->plans, [](const PagePlan& plan) {
        return plan.page_id.starts_with("type:");
    }));

    auto namespace_plan = std::ranges::find_if(result->plans, [](const PagePlan& plan) {
        return plan.page_id == "namespace:demo";
    });
    ASSERT_TRUE(namespace_plan != result->plans.end());
    EXPECT_EQ(namespace_plan->prompt_requests.size(), 1u);
    EXPECT_EQ(namespace_plan->prompt_requests[0].kind, PromptKind::NamespaceSummary);
    EXPECT_FALSE(
        std::ranges::any_of(namespace_plan->prompt_requests, [](const PromptRequest& request) {
            return request.kind == PromptKind::TypeDeclarationSummary &&
                   parse_symbol_target_key(request.target_key).qualified_name == "ActiveRunReset";
        }));
}

TEST_CASE(build_page_plan_set_skips_std_namespace_pages) {
    ScopedTempDir temp("skip_std_namespace_pages");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);

    extract::ProjectModel model;
    auto file = (temp.path / "src" / "demo.cppm").generic_string();

    auto sym = make_type_symbol(500, "Foo", "demo::Foo", file, "A type.", "demo");
    auto sym_id = sym.id;
    add_symbol(model, std::move(sym));

    add_namespace(model, "demo", {sym_id});
    add_namespace(model, "std");

    auto result = build_page_plan_set(config, model);

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(std::ranges::any_of(result->plans, [](const PagePlan& plan) {
        return plan.page_id == "namespace:demo";
    }));
    EXPECT_FALSE(std::ranges::any_of(result->plans, [](const PagePlan& plan) {
        return plan.page_id == "namespace:std";
    }));
}

TEST_CASE(build_page_plan_set_orders_independent_modules_lexicographically) {
    ScopedTempDir temp("module_generation_order");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);

    extract::ProjectModel model;
    auto beta_file = (temp.path / "src" / "beta.cppm").generic_string();
    auto alpha_file = (temp.path / "src" / "alpha.cppm").generic_string();

    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.beta",
                   .is_interface = true,
                   .source_file = beta_file,
                   .imports = {},
                   .symbols = {},
               });
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.alpha",
                   .is_interface = true,
                   .source_file = alpha_file,
                   .imports = {},
                   .symbols = {},
               });

    auto result = build_page_plan_set(config, model);

    ASSERT_TRUE(result.has_value());
    auto module_ids = result->generation_order | std::views::filter([](const std::string& id) {
                          return id.starts_with("module:");
                      });
    std::vector<std::string> ordered(module_ids.begin(), module_ids.end());
    ASSERT_EQ(ordered.size(), 2u);
    EXPECT_EQ(ordered[0], "module:demo.alpha");
    EXPECT_EQ(ordered[1], "module:demo.beta");
}

TEST_CASE(build_page_plan_set_rejects_dependency_cycles) {
    ScopedTempDir temp("module_cycle_generation_order");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);

    extract::ProjectModel model;
    auto beta_file = (temp.path / "src" / "beta.cppm").generic_string();
    auto alpha_file = (temp.path / "src" / "alpha.cppm").generic_string();
    auto gamma_file = (temp.path / "src" / "gamma.cppm").generic_string();

    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.beta",
                   .is_interface = true,
                   .source_file = beta_file,
                   .imports = {"demo.alpha"},
                   .symbols = {},
               });
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.alpha",
                   .is_interface = true,
                   .source_file = alpha_file,
                   .imports = {"demo.beta"},
                   .symbols = {},
               });
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.gamma",
                   .is_interface = true,
                   .source_file = gamma_file,
                   .imports = {"demo.beta"},
                   .symbols = {},
               });

    auto result = build_page_plan_set(config, model);

    EXPECT_FALSE(result.has_value());
    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("page dependency cycle detected"), std::string::npos);
}

TEST_CASE(compute_page_path_places_index_at_root) {
    auto result = compute_page_path(PageIdentity{
        .page_type = PageType::Index,
        .normalized_owner_key = "index",
    });

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "index.md");
}

TEST_CASE(compute_page_path_flattens_module_partition_pages) {
    auto result = compute_page_path(PageIdentity{
        .page_type = PageType::Module,
        .qualified_name = "demo.math:internal",
    });

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "modules/demo/math/internal.md");
}

};  // TEST_SUITE(generate_planner)
