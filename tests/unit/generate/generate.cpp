#include "eventide/zest/zest.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <ranges>
#include <sstream>
#include <string>
#include <unordered_map>

import config;
import extract;
import generate;

using namespace clore;
using namespace clore::generate;

namespace {

namespace fs = std::filesystem;

struct ScopedTempDir {
    fs::path path;

    explicit ScopedTempDir(std::string_view name) {
        auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
        path = fs::temp_directory_path() / std::format("clore_{}_{}", name, ticks);
        fs::create_directories(path);
    }

    ~ScopedTempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
};

auto read_text_file(const fs::path& path) -> std::string {
    std::ifstream f(path);
    std::stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}

auto make_config(const fs::path& project_root) -> config::TaskConfig {
    config::TaskConfig config;
    config.project_root = project_root.generic_string();
    config.output_root = (project_root / "docs").generic_string();
    config.workspace_root = project_root.generic_string();
    config.extract.max_snippet_bytes = 1024;

    config.page_types.index = false;
    config.page_types.module_page = true;
    config.page_types.namespace_page = true;
    config.page_types.type_page = true;
    config.page_types.file_page = true;
    config.page_types.workflow_page = false;

    config.path_rules.index_path = "index.md";
    config.path_rules.module_prefix = "modules";
    config.path_rules.namespace_prefix = "namespaces";
    config.path_rules.type_prefix = "types";
    config.path_rules.file_prefix = "files";
    config.path_rules.workflow_prefix = "workflows";
    config.path_rules.name_normalize = "lowercase";

    config.evidence_rules.max_callers = 5;
    config.evidence_rules.max_callees = 5;
    config.evidence_rules.max_siblings = 8;
    config.evidence_rules.max_source_bytes = 4096;
    config.evidence_rules.max_related_summaries = 3;

    config.workflow_rules.min_chain_symbols = 2;
    config.workflow_rules.min_new_symbols = 1;
    config.workflow_rules.max_symbol_overlap_ratio_percent = 50;
    config.workflow_rules.max_workflow_pages = 8;
    config.workflow_rules.llm_review_top_k = 6;
    config.workflow_rules.llm_selected_count = 4;

    config.llm.system_prompt = "You are a writer.";
    config.llm.retry_count = 3;
    config.llm.retry_initial_backoff_ms = 250;

    config.validation.fail_on_empty_section = false;
    config.validation.fail_on_h1_in_output = true;

    config.navigation.consume_dependency_summaries = true;

    return config;
}

auto make_model(const fs::path& project_root) -> extract::ProjectModel {
    auto file_path = (project_root / "src" / "math.cpp").generic_string();

    extract::SymbolInfo symbol;
    symbol.id = extract::SymbolID{.hash = 1};
    symbol.kind = extract::SymbolKind::Function;
    symbol.name = "add";
    symbol.qualified_name = "math::add";
    symbol.signature = "int add(int lhs, int rhs)";
    symbol.doc_comment = "Adds two integers.";
    symbol.source_snippet = "int add(int lhs, int rhs) { return lhs + rhs; }";
    symbol.declaration_location = extract::SourceLocation{
        .file = file_path,
        .line = 1,
        .column = 1,
    };

    extract::FileInfo file_info;
    file_info.path = file_path;
    file_info.symbols.push_back(symbol.id);

    extract::ProjectModel model;
    model.symbols.emplace(symbol.id, std::move(symbol));
    model.files.emplace(file_path, std::move(file_info));
    model.file_order.push_back(file_path);

    return model;
}

auto make_symbol(std::uint64_t id, std::string_view name, std::string_view qualified_name,
                 std::string_view signature, const std::string& file)
    -> extract::SymbolInfo {
    extract::SymbolInfo symbol;
    symbol.id = extract::SymbolID{.hash = id};
    symbol.kind = extract::SymbolKind::Function;
    symbol.name = std::string(name);
    symbol.qualified_name = std::string(qualified_name);
    symbol.signature = std::string(signature);
    symbol.source_snippet = std::format("{} {{}}", signature);
    symbol.declaration_location = extract::SourceLocation{
        .file = file,
        .line = 1,
        .column = 1,
    };
    return symbol;
}

auto make_type_symbol(std::uint64_t id, std::string_view name,
                      std::string_view qualified_name, const std::string& file,
                      std::string_view doc_comment) -> extract::SymbolInfo {
    extract::SymbolInfo symbol;
    symbol.id = extract::SymbolID{.hash = id};
    symbol.kind = extract::SymbolKind::Struct;
    symbol.name = std::string(name);
    symbol.qualified_name = std::string(qualified_name);
    symbol.signature = std::format("struct {}", name);
    symbol.doc_comment = std::string(doc_comment);
    symbol.declaration_location = extract::SourceLocation{
        .file = file,
        .line = 1,
        .column = 1,
    };
    return symbol;
}

}  // namespace

TEST_SUITE(generate) {
    TEST_CASE(build_page_plan_set_creates_plans_for_files) {
        ScopedTempDir temp("build_page_plan");
        fs::create_directories(temp.path / "src");

        auto config = make_config(temp.path);
        auto model = make_model(temp.path);

        auto result = build_page_plan_set(config, model);

        ASSERT_TRUE(result.has_value());
        EXPECT_GE(result->plans.size(), 1u);

        auto file_plan = std::ranges::find_if(result->plans, [](const PagePlan& p) {
            return p.page_type == PageType::File;
        });
        ASSERT_TRUE(file_plan != result->plans.end());
        EXPECT_FALSE(file_plan->relative_path.empty());
    }

    TEST_CASE(build_page_plan_set_generates_single_index_page) {
        ScopedTempDir temp("single_index_page");
        fs::create_directories(temp.path / "src");

        auto config = make_config(temp.path);
        config.page_types.index = true;
        auto model = make_model(temp.path);

        auto result = build_page_plan_set(config, model);

        ASSERT_TRUE(result.has_value());

        auto index_plan = std::ranges::find_if(result->plans, [](const PagePlan& p) {
            return p.page_type == PageType::Index;
        });
        ASSERT_TRUE(index_plan != result->plans.end());
        EXPECT_TRUE(std::ranges::find(index_plan->deterministic_blocks, "all_modules")
                    != index_plan->deterministic_blocks.end());
        EXPECT_TRUE(std::ranges::find(index_plan->deterministic_blocks, "all_namespaces")
                    != index_plan->deterministic_blocks.end());
        EXPECT_TRUE(std::ranges::find(index_plan->deterministic_blocks, "all_types")
                    != index_plan->deterministic_blocks.end());
        EXPECT_TRUE(std::ranges::find(index_plan->deterministic_blocks, "all_files")
                    != index_plan->deterministic_blocks.end());
    }

    TEST_CASE(write_pages_writes_relative_output_paths) {
        ScopedTempDir temp("write_pages");
        auto output_root = temp.path / "docs";

        std::vector<GeneratedPage> pages{
            GeneratedPage{
                .relative_path = "src/math.md",
                .content = "# Math\n",
            },
        };

        auto result = write_pages(pages, output_root.generic_string());

        ASSERT_TRUE(result.has_value());

        auto target = output_root / "src" / "math.md";
        ASSERT_TRUE(fs::exists(target));
        EXPECT_EQ(read_text_file(target), "# Math\n");
    }

    TEST_CASE(build_page_plan_set_creates_module_plans) {
        ScopedTempDir temp("build_module_plans");
        fs::create_directories(temp.path / "src");

        auto config = make_config(temp.path);

        extract::ProjectModel model;
        model.uses_modules = true;

        auto util_file = (temp.path / "src" / "util.cppm").generic_string();
        auto main_file = (temp.path / "src" / "math.cppm").generic_string();

        auto util_symbol = make_symbol(10, "helper", "demo::util::helper", "int helper()", util_file);
        auto api_symbol = make_symbol(11, "add", "demo::math::add", "int add(int lhs, int rhs)", main_file);

        model.symbols.emplace(util_symbol.id, util_symbol);
        model.symbols.emplace(api_symbol.id, api_symbol);

        model.modules.emplace(
            util_file,
            extract::ModuleUnit{
                .name = "demo.util",
                .is_interface = true,
                .source_file = util_file,
                .imports = {},
                .symbols = {util_symbol.id},
            });
        model.modules.emplace(
            main_file,
            extract::ModuleUnit{
                .name = "demo.math",
                .is_interface = true,
                .source_file = main_file,
                .imports = {"demo.util"},
                .symbols = {api_symbol.id},
            });

        auto result = build_page_plan_set(config, model);

        ASSERT_TRUE(result.has_value());

        auto has_util = std::ranges::any_of(result->plans, [](const PagePlan& p) {
            return p.page_type == PageType::Module && p.page_id.find("demo.util") != std::string::npos;
        });
        auto has_math = std::ranges::any_of(result->plans, [](const PagePlan& p) {
            return p.page_type == PageType::Module && p.page_id.find("demo.math") != std::string::npos;
        });
        EXPECT_TRUE(has_util);
        EXPECT_TRUE(has_math);

        // Verify generation order: util before math
        auto pos_util = std::ranges::find_if(result->generation_order, [](const std::string& id) {
            return id.find("demo.util") != std::string::npos;
        });
        auto pos_math = std::ranges::find_if(result->generation_order, [](const std::string& id) {
            return id.find("demo.math") != std::string::npos;
        });
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
        auto type_symbol = make_type_symbol(200, "Options", "demo::config::Options",
                                            file, "Configuration options.");
        model.symbols.emplace(type_symbol.id, type_symbol);

        extract::NamespaceInfo root_ns;
        root_ns.name = "demo";
        root_ns.children.push_back("demo::config");

        extract::NamespaceInfo child_ns;
        child_ns.name = "demo::config";
        child_ns.symbols.push_back(type_symbol.id);

        model.namespaces.emplace(root_ns.name, root_ns);
        model.namespaces.emplace(child_ns.name, child_ns);

        auto result = build_page_plan_set(config, model);

        ASSERT_TRUE(result.has_value());

        auto root_plan = std::ranges::find_if(result->plans, [](const PagePlan& p) {
            return p.page_id == "namespace:demo";
        });
        ASSERT_TRUE(root_plan != result->plans.end());
        EXPECT_TRUE(std::ranges::find(root_plan->linked_pages, "namespace:demo::config") !=
                    root_plan->linked_pages.end());
    }

    TEST_CASE(build_page_plan_set_skips_lexically_nested_type_pages) {
        ScopedTempDir temp("skip_nested_type_pages");
        fs::create_directories(temp.path / "src");

        auto config = make_config(temp.path);

        extract::ProjectModel model;
        auto file = (temp.path / "src" / "widget.cppm").generic_string();

        auto outer = make_type_symbol(300, "Outer", "demo::Outer", file, "Outer type.");
        auto helper = make_type_symbol(301, "Inner", "demo::Outer::Inner", file, "Nested helper.");
        helper.lexical_parent_name = "demo::Outer";
        helper.lexical_parent_kind = extract::SymbolKind::Struct;
        helper.enclosing_namespace = "demo";

        model.symbols.emplace(outer.id, outer);
        model.symbols.emplace(helper.id, helper);

        extract::NamespaceInfo ns;
        ns.name = "demo";
        ns.symbols = {outer.id, helper.id};
        model.namespaces.emplace(ns.name, ns);

        auto result = build_page_plan_set(config, model);

        ASSERT_TRUE(result.has_value());
        EXPECT_TRUE(std::ranges::any_of(result->plans, [](const PagePlan& p) {
            return p.page_id == "type:demo::Outer";
        }));
        EXPECT_FALSE(std::ranges::any_of(result->plans, [](const PagePlan& p) {
            return p.page_id == "type:demo::Outer::Inner";
        }));
    }

    TEST_CASE(build_page_plan_set_skips_function_local_type_pages) {
        ScopedTempDir temp("skip_function_local_type_pages");
        fs::create_directories(temp.path / "src");

        auto config = make_config(temp.path);

        extract::ProjectModel model;
        auto file = (temp.path / "src" / "client.cppm").generic_string();

        auto exported = make_type_symbol(310, "Client", "demo::Client", file, "Client type.");
        auto local = make_type_symbol(311, "ActiveRunReset", "ActiveRunReset", file, "Local helper.");
        local.enclosing_namespace = "demo";
        local.lexical_parent_name = "demo::run";
        local.lexical_parent_kind = extract::SymbolKind::Function;

        model.symbols.emplace(exported.id, exported);
        model.symbols.emplace(local.id, local);

        extract::NamespaceInfo ns;
        ns.name = "demo";
        ns.symbols = {exported.id, local.id};
        model.namespaces.emplace(ns.name, ns);

        auto result = build_page_plan_set(config, model);

        ASSERT_TRUE(result.has_value());
        EXPECT_TRUE(std::ranges::any_of(result->plans, [](const PagePlan& p) {
            return p.page_id == "type:demo::Client";
        }));
        EXPECT_FALSE(std::ranges::any_of(result->plans, [](const PagePlan& p) {
            return p.page_id == "type:ActiveRunReset";
        }));
    }

    TEST_CASE(build_page_plan_set_skips_std_namespace_pages) {
        ScopedTempDir temp("skip_std_namespace_pages");
        fs::create_directories(temp.path / "src");

        auto config = make_config(temp.path);

        extract::ProjectModel model;
        auto file = (temp.path / "src" / "demo.cppm").generic_string();

        auto sym = make_type_symbol(500, "Foo", "demo::Foo", file, "A type.");
        sym.enclosing_namespace = "demo";
        model.symbols.emplace(sym.id, sym);

        extract::NamespaceInfo project_ns;
        project_ns.name = "demo";
        project_ns.symbols = {sym.id};

        extract::NamespaceInfo std_ns;
        std_ns.name = "std";

        model.namespaces.emplace(project_ns.name, project_ns);
        model.namespaces.emplace(std_ns.name, std_ns);

        auto result = build_page_plan_set(config, model);

        ASSERT_TRUE(result.has_value());
        EXPECT_TRUE(std::ranges::any_of(result->plans, [](const PagePlan& p) {
            return p.page_id == "namespace:demo";
        }));
        EXPECT_FALSE(std::ranges::any_of(result->plans, [](const PagePlan& p) {
            return p.page_id == "namespace:std";
        }));
    }

    TEST_CASE(build_page_plan_set_orders_independent_modules_lexicographically) {
        ScopedTempDir temp("module_generation_order");
        fs::create_directories(temp.path / "src");

        auto config = make_config(temp.path);
        config.page_types.index = false;
        config.page_types.file_page = false;
        config.page_types.namespace_page = false;
        config.page_types.type_page = false;
        config.page_types.module_page = true;

        extract::ProjectModel model;
        model.uses_modules = true;

        auto beta_file = (temp.path / "src" / "beta.cppm").generic_string();
        auto alpha_file = (temp.path / "src" / "alpha.cppm").generic_string();

        model.modules.emplace(
            beta_file,
            extract::ModuleUnit{
                .name = "demo.beta",
                .is_interface = true,
                .source_file = beta_file,
            });
        model.modules.emplace(
            alpha_file,
            extract::ModuleUnit{
                .name = "demo.alpha",
                .is_interface = true,
                .source_file = alpha_file,
            });

        auto result = build_page_plan_set(config, model);

        ASSERT_TRUE(result.has_value());
        ASSERT_EQ(result->generation_order.size(), 2u);
        EXPECT_EQ(result->generation_order[0], "module:demo.alpha");
        EXPECT_EQ(result->generation_order[1], "module:demo.beta");
    }

    TEST_CASE(build_page_plan_set_orders_cycle_participants_lexicographically) {
        ScopedTempDir temp("module_cycle_generation_order");
        fs::create_directories(temp.path / "src");

        auto config = make_config(temp.path);
        config.page_types.index = false;
        config.page_types.file_page = false;
        config.page_types.namespace_page = false;
        config.page_types.type_page = false;
        config.page_types.module_page = true;

        extract::ProjectModel model;
        model.uses_modules = true;

        auto beta_file = (temp.path / "src" / "beta.cppm").generic_string();
        auto alpha_file = (temp.path / "src" / "alpha.cppm").generic_string();
        auto gamma_file = (temp.path / "src" / "gamma.cppm").generic_string();

        model.modules.emplace(
            beta_file,
            extract::ModuleUnit{
                .name = "demo.beta",
                .is_interface = true,
                .source_file = beta_file,
                .imports = {"demo.alpha"},
            });
        model.modules.emplace(
            alpha_file,
            extract::ModuleUnit{
                .name = "demo.alpha",
                .is_interface = true,
                .source_file = alpha_file,
                .imports = {"demo.beta"},
            });
        model.modules.emplace(
            gamma_file,
            extract::ModuleUnit{
                .name = "demo.gamma",
                .is_interface = true,
                .source_file = gamma_file,
                .imports = {"demo.beta"},
            });

        auto result = build_page_plan_set(config, model);

        ASSERT_TRUE(result.has_value());
        ASSERT_EQ(result->generation_order.size(), 3u);
        EXPECT_EQ(result->generation_order[0], "module:demo.alpha");
        EXPECT_EQ(result->generation_order[1], "module:demo.beta");
        EXPECT_EQ(result->generation_order[2], "module:demo.gamma");
    }

    TEST_CASE(render_deterministic_block_relativizes_related_page_links) {
        ScopedTempDir temp("relative_links");

        auto config = make_config(temp.path);
        extract::ProjectModel model;

        PagePlan current_plan{
            .page_id = "namespace:demo",
            .page_type = PageType::Namespace,
            .title = "Namespace `demo`",
            .relative_path = "namespaces/demo/index.md",
            .linked_pages = {"type:demo::Widget"},
        };
        PagePlan target_plan{
            .page_id = "type:demo::Widget",
            .page_type = PageType::Type,
            .title = "`demo::Widget`",
            .relative_path = "types/demo/widget.md",
            .owner_keys = {"demo::Widget"},
        };

        PagePlanSet plan_set{
            .plans = {current_plan, target_plan},
        };

        auto links = build_link_resolver(plan_set);
        auto block = render_deterministic_block("related_pages", current_plan, model, config, links);

        EXPECT_NE(block.find("(../../types/demo/widget.md)"), std::string::npos);
    }

    TEST_CASE(render_deterministic_block_relativizes_all_files_links) {
        ScopedTempDir temp("relative_file_links");
        fs::create_directories(temp.path / "src");

        auto config = make_config(temp.path);
        extract::ProjectModel model;

        auto file_path = (temp.path / "src" / "math.cpp").generic_string();
        model.files.emplace(file_path, extract::FileInfo{.path = file_path});

        PagePlan current_plan{
            .page_id = "index",
            .page_type = PageType::Index,
            .title = "API Reference",
            .relative_path = "guides/index.md",
        };
        PagePlan file_plan{
            .page_id = "file:" + file_path,
            .page_type = PageType::File,
            .title = "File `src/math.cpp`",
            .relative_path = "files/src/math.md",
            .owner_keys = {file_path},
        };

        PagePlanSet plan_set{
            .plans = {current_plan, file_plan},
        };

        auto links = build_link_resolver(plan_set);
        auto block = render_deterministic_block("all_files", current_plan, model, config, links);

        EXPECT_NE(block.find("(../files/src/math.md)"), std::string::npos);
    }

    TEST_CASE(render_deterministic_block_renders_module_type_indexes) {
        ScopedTempDir temp("module_type_index");
        fs::create_directories(temp.path / "src");

        auto config = make_config(temp.path);

        extract::ProjectModel model;
        model.uses_modules = true;

        auto file = (temp.path / "src" / "math.cppm").generic_string();
        auto widget = make_type_symbol(400, "Widget", "demo::Widget", file, "Widget type.");
        model.symbols.emplace(widget.id, widget);
        model.modules.emplace(
            file,
            extract::ModuleUnit{
                .name = "demo.math",
                .is_interface = true,
                .source_file = file,
                .symbols = {widget.id},
            });

        PagePlan module_plan{
            .page_id = "module:demo.math",
            .page_type = PageType::Module,
            .title = "Module `demo.math`",
            .relative_path = "modules/demo/math.md",
            .owner_keys = {"demo.math"},
        };
        PagePlan type_plan{
            .page_id = "type:demo::Widget",
            .page_type = PageType::Type,
            .title = "`demo::Widget`",
            .relative_path = "types/demo/widget.md",
            .owner_keys = {"demo::Widget"},
        };

        PagePlanSet plan_set{
            .plans = {module_plan, type_plan},
        };

        auto links = build_link_resolver(plan_set);
        auto block = render_deterministic_block("types_index", module_plan, model, config, links);

        EXPECT_NE(block.find("demo::Widget"), std::string::npos);
        EXPECT_NE(block.find("../../types/demo/widget.md"), std::string::npos);
    }

    TEST_CASE(render_deterministic_block_skips_unplanned_namespaces_in_index) {
        ScopedTempDir temp("filtered_index_namespaces");

        auto config = make_config(temp.path);
        extract::ProjectModel model;
        model.namespaces.emplace("demo", extract::NamespaceInfo{.name = "demo"});
        model.namespaces.emplace("std::hash<demo::Widget>",
                                 extract::NamespaceInfo{.name = "std::hash<demo::Widget>"});

        PagePlan index_plan{
            .page_id = "index",
            .page_type = PageType::Index,
            .title = "API Reference",
            .relative_path = "index.md",
        };
        PagePlan namespace_plan{
            .page_id = "namespace:demo",
            .page_type = PageType::Namespace,
            .title = "Namespace `demo`",
            .relative_path = "namespaces/demo/index.md",
            .owner_keys = {"demo"},
        };

        PagePlanSet plan_set{
            .plans = {index_plan, namespace_plan},
        };

        auto links = build_link_resolver(plan_set);
        auto block = render_deterministic_block("all_namespaces", index_plan, model, config, links);

        EXPECT_NE(block.find("demo"), std::string::npos);
        EXPECT_EQ(block.find("std::hash<demo::Widget>"), std::string::npos);
    }

    TEST_CASE(render_deterministic_block_uses_curated_workflow_titles) {
        ScopedTempDir temp("workflow_titles");

        auto config = make_config(temp.path);
        extract::ProjectModel model;

        PagePlan index_plan{
            .page_id = "index",
            .page_type = PageType::Index,
            .title = "API Reference",
            .relative_path = "guides/index.md",
            .linked_pages = {"workflow:generate-docs-preview"},
        };
        PagePlan workflow_plan{
            .page_id = "workflow:generate-docs-preview",
            .page_type = PageType::Workflow,
            .title = "Generate Docs",
            .relative_path = "workflows/generate-docs-preview/index.md",
            .owner_keys = {"demo::main", "demo::generate_docs"},
        };

        auto links = build_link_resolver(PagePlanSet{
            .plans = {index_plan, workflow_plan},
        });
        auto block = render_deterministic_block("all_workflows", index_plan, model, config, links);

        EXPECT_NE(block.find("[Generate Docs]"), std::string::npos);
        EXPECT_EQ(block.find("Generate Docs Preview"), std::string::npos);
    }

    TEST_CASE(namespace_summary_prompt_uses_namespace_subject) {
        ScopedTempDir temp("namespace_summary_prompt");
        fs::create_directories(temp.path / "src");

        auto config = make_config(temp.path);

        extract::ProjectModel model;
        extract::NamespaceInfo ns;
        ns.name = "demo::config";

        auto file = (temp.path / "src" / "config.cppm").generic_string();
        for(std::uint64_t i = 0; i < 8; ++i) {
            auto name = std::format("Type{}", i);
            auto qualified_name = std::format("demo::config::{}", name);
            auto doc_comment = std::string(240, static_cast<char>('a' + static_cast<int>(i)));
            auto symbol = make_type_symbol(100 + i, name, qualified_name, file, doc_comment);
            ns.symbols.push_back(symbol.id);
            model.symbols.emplace(symbol.id, std::move(symbol));
        }
        model.namespaces.emplace(ns.name, ns);

        PageSummaryCache summaries;
        summaries.emplace("demo::config::Type0", std::string(180, 'x'));
        summaries.emplace("demo::config::Type1", std::string(180, 'y'));
        summaries.emplace("demo::config::Type2", std::string(180, 'z'));

        auto evidence = build_evidence_for_namespace_summary(
            model.namespaces.at(ns.name), model, config.evidence_rules, summaries, config.project_root);
        EXPECT_EQ(evidence.subject_name, "demo::config");
        EXPECT_EQ(evidence.subject_kind, "namespace");

        auto prompt = instantiate_prompt(
            "Write a Summary for the namespace `{{target_name}}`.\n\n{{evidence}}",
            evidence);

        ASSERT_FALSE(prompt.empty());
        EXPECT_NE(prompt.find("namespace `demo::config`"), std::string::npos);
        EXPECT_EQ(prompt.find("namespace `demo::config::Type0`"), std::string::npos);
    }

    TEST_CASE(assemble_page_strips_empty_sections_from_crlf_templates) {
        std::unordered_map<std::string, std::string> blocks{
            {"declaration", "```cpp\nstruct Example {};\n```\n"},
        };
        std::unordered_map<std::string, std::string> slots;

        auto result = assemble_page(
            "# {{title}}\r\n\r\n## Summary\r\n\r\n{{slot:type_overview}}\r\n\r\n## Declaration\r\n\r\n{{block:declaration}}",
            "`Example`", blocks, slots, false);

        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result->find("## Summary"), std::string::npos);
        EXPECT_NE(result->find("## Declaration"), std::string::npos);
    }

    TEST_CASE(assemble_page_keeps_indented_headings_as_content) {
        std::unordered_map<std::string, std::string> blocks;
        std::unordered_map<std::string, std::string> slots;

        auto result = assemble_page(
            "# {{title}}\n\n    ## Literal heading\n\n## Details\n\nBody\n",
            "`Example`", blocks, slots, false);

        ASSERT_TRUE(result.has_value());
        EXPECT_NE(result->find("    ## Literal heading"), std::string::npos);
        EXPECT_NE(result->find("## Details"), std::string::npos);
    }

    TEST_CASE(compute_page_path_sanitizes_workflow_slugs) {
        auto config = make_config(fs::temp_directory_path());

        auto result = compute_page_path(
            PageIdentity{
                .page_type = PageType::Workflow,
                .normalized_owner_key = "  docs/generate \\ preview  ",
            },
            config.path_rules);

        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, "workflows/docs-generate-preview/index.md");
    }

    TEST_CASE(validate_output_rejects_whitespace_only_content) {
        auto validation = config::ValidationConfig{};
        auto result = validate_output("  \r\n\t\n", validation);

        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error().message, "LLM output contains only whitespace");
    }
};
