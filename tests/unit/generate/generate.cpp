#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "eventide/zest/zest.h"

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

    return config;
}

auto make_function_symbol(std::uint64_t id,
                          std::string_view name,
                          std::string_view qualified_name,
                          std::string_view signature,
                          const std::string& file,
                          std::string_view ns_name = {}) -> extract::SymbolInfo {
    extract::SymbolInfo symbol;
    symbol.id = extract::SymbolID{.hash = id};
    symbol.kind = extract::SymbolKind::Function;
    symbol.name = std::string(name);
    symbol.qualified_name = std::string(qualified_name);
    symbol.enclosing_namespace = std::string(ns_name);
    symbol.signature = std::string(signature);
    symbol.source_snippet = std::format("{} {{ return 0; }}", signature);
    symbol.declaration_location = extract::SourceLocation{
        .file = file,
        .line = 1,
        .column = 1,
    };
    symbol.definition_location = extract::SourceLocation{
        .file = file,
        .line = 3,
        .column = 1,
    };
    return symbol;
}

auto make_type_symbol(std::uint64_t id,
                      std::string_view name,
                      std::string_view qualified_name,
                      const std::string& file,
                      std::string_view doc_comment,
                      std::string_view ns_name = {}) -> extract::SymbolInfo {
    extract::SymbolInfo symbol;
    symbol.id = extract::SymbolID{.hash = id};
    symbol.kind = extract::SymbolKind::Struct;
    symbol.name = std::string(name);
    symbol.qualified_name = std::string(qualified_name);
    symbol.enclosing_namespace = std::string(ns_name);
    symbol.signature = std::format("struct {}", name);
    symbol.doc_comment = std::string(doc_comment);
    symbol.source_snippet = std::format("struct {} {{}};", name);
    symbol.declaration_location = extract::SourceLocation{
        .file = file,
        .line = 1,
        .column = 1,
    };
    return symbol;
}

auto add_symbol(extract::ProjectModel& model, extract::SymbolInfo symbol) -> void {
    auto file = symbol.declaration_location.file;
    auto symbol_id = symbol.id;
    auto qualified_name = symbol.qualified_name;

    auto& file_info = model.files[file];
    file_info.path = file;
    file_info.symbols.push_back(symbol_id);
    if(std::ranges::find(model.file_order, file) == model.file_order.end()) {
        model.file_order.push_back(file);
    }

    model.symbol_ids_by_qualified_name[qualified_name] = symbol_id;
    model.symbols.emplace(symbol_id, std::move(symbol));
}

auto add_namespace(extract::ProjectModel& model,
                   std::string name,
                   std::vector<extract::SymbolID> symbols = {},
                   std::vector<std::string> children = {}) -> void {
    auto key = name;
    model.namespaces[key] = extract::NamespaceInfo{
        .name = std::move(name),
        .symbols = std::move(symbols),
        .children = std::move(children),
    };
}

auto add_module(extract::ProjectModel& model, extract::ModuleUnit module) -> void {
    model.uses_modules = true;
    model.module_name_to_source[module.name] = module.source_file;
    model.modules.emplace(module.source_file, std::move(module));
}

auto make_basic_model(const fs::path& project_root) -> extract::ProjectModel {
    extract::ProjectModel model;
    auto file_path = (project_root / "src" / "math.cpp").generic_string();

    auto symbol =
        make_function_symbol(1, "add", "math::add", "int add(int lhs, int rhs)", file_path, "math");
    auto symbol_id = symbol.id;
    add_symbol(model, std::move(symbol));
    add_namespace(model, "math", {symbol_id});

    return model;
}

auto make_prompt_outputs(std::initializer_list<PromptRequest> requests, std::string_view text)
    -> std::unordered_map<std::string, std::string> {
    std::unordered_map<std::string, std::string> outputs;
    for(const auto& request: requests) {
        outputs[prompt_request_key(request)] = std::string(text);
    }
    return outputs;
}

}  // namespace

TEST_SUITE(generate) {

TEST_CASE(generate_dry_run_embeds_prompt_outputs) {
    ScopedTempDir temp("dry_run_prompts");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    auto model = make_basic_model(temp.path);

    auto result = generate_dry_run(config, model);

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(std::ranges::any_of(*result, [](const GeneratedPage& page) {
        return page.content.find("> Prompt (`") != std::string::npos;
    }));
}

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
    EXPECT_TRUE(std::ranges::any_of(index_plan->prompt_requests, [](const PromptRequest& request) {
        return request.kind == PromptKind::IndexOverview;
    }));
    EXPECT_TRUE(std::ranges::any_of(index_plan->prompt_requests, [](const PromptRequest& request) {
        return request.kind == PromptKind::IndexReadingGuide;
    }));
    EXPECT_TRUE(std::ranges::any_of(file_plan->prompt_requests, [](const PromptRequest& request) {
        return request.kind == PromptKind::FunctionImplementationSummary &&
               request.target_key == "math::add";
    }));
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

TEST_CASE(build_page_plan_set_uses_modules_to_emit_modules_not_files) {
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
    EXPECT_TRUE(
        std::ranges::any_of(namespace_plan->prompt_requests, [](const PromptRequest& request) {
            return request.kind == PromptKind::TypeDeclarationSummary &&
                   request.target_key == "demo::Outer";
        }));
    EXPECT_FALSE(
        std::ranges::any_of(namespace_plan->prompt_requests, [](const PromptRequest& request) {
            return request.kind == PromptKind::TypeDeclarationSummary &&
                   request.target_key == "demo::Outer::Inner";
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
    EXPECT_TRUE(
        std::ranges::any_of(namespace_plan->prompt_requests, [](const PromptRequest& request) {
            return request.kind == PromptKind::TypeDeclarationSummary &&
                   request.target_key == "demo::Client";
        }));
    EXPECT_FALSE(
        std::ranges::any_of(namespace_plan->prompt_requests, [](const PromptRequest& request) {
            return request.kind == PromptKind::TypeDeclarationSummary &&
                   request.target_key == "ActiveRunReset";
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
               });
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.alpha",
                   .is_interface = true,
                   .source_file = alpha_file,
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

TEST_CASE(build_page_plan_set_orders_cycle_participants_lexicographically) {
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
               });
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.alpha",
                   .is_interface = true,
                   .source_file = alpha_file,
                   .imports = {"demo.beta"},
               });
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.gamma",
                   .is_interface = true,
                   .source_file = gamma_file,
                   .imports = {"demo.beta"},
               });

    auto result = build_page_plan_set(config, model);

    ASSERT_TRUE(result.has_value());
    auto module_ids = result->generation_order | std::views::filter([](const std::string& id) {
                          return id.starts_with("module:");
                      });
    std::vector<std::string> ordered(module_ids.begin(), module_ids.end());
    ASSERT_EQ(ordered.size(), 3u);
    EXPECT_EQ(ordered[0], "module:demo.alpha");
    EXPECT_EQ(ordered[1], "module:demo.beta");
    EXPECT_EQ(ordered[2], "module:demo.gamma");
}

TEST_CASE(render_page_markdown_relativizes_workflow_links_in_index) {
    ScopedTempDir temp("workflow_links");
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
        .relative_path = "workflows/generate-docs-preview.md",
        .owner_keys = {"demo::main", "demo::generate_docs"},
    };

    PagePlanSet plan_set{
        .plans = {index_plan, workflow_plan}
    };
    auto links = build_link_resolver(plan_set);
    auto outputs = make_prompt_outputs({PromptRequest{.kind = PromptKind::IndexOverview},
                                        PromptRequest{.kind = PromptKind::IndexReadingGuide}},
                                       "Overview");

    auto result = render_page_markdown(index_plan, config, model, outputs, links);

    ASSERT_TRUE(result.has_value());
    EXPECT_NE(result->find("[Generate Docs](../workflows/generate-docs-preview.md)"),
              std::string::npos);
}

TEST_CASE(render_page_markdown_relativizes_file_links_in_index) {
    ScopedTempDir temp("file_links");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    auto file_path = (temp.path / "src" / "math.cpp").generic_string();
    model.files.emplace(file_path, extract::FileInfo{.path = file_path});

    PagePlan index_plan{
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
        .plans = {index_plan, file_plan}
    };
    auto links = build_link_resolver(plan_set);
    auto outputs = make_prompt_outputs({PromptRequest{.kind = PromptKind::IndexOverview},
                                        PromptRequest{.kind = PromptKind::IndexReadingGuide}},
                                       "Overview");

    auto result = render_page_markdown(index_plan, config, model, outputs, links);

    ASSERT_TRUE(result.has_value());
    EXPECT_NE(result->find("(../files/src/math.md)"), std::string::npos);
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
    add_symbol(model, std::move(widget));
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.math",
                   .is_interface = true,
                   .source_file = file,
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
                       PromptRequest{.kind = PromptKind::TypeDeclarationSummary,
                              .target_key = "demo::Widget"},
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
                              .target_key = "demo::Widget"},
                       },
    };

    auto outputs = std::unordered_map<std::string, std::string>{
        {prompt_request_key(PromptRequest{.kind = PromptKind::NamespaceSummary}),   "Summary"     },
        {prompt_request_key(PromptRequest{.kind = PromptKind::TypeDeclarationSummary,
                                          .target_key = "demo::Widget"}),
         "Declaration"                                                                            },
        {prompt_request_key(PromptRequest{.kind = PromptKind::ModuleSummary}),      "Summary"     },
        {prompt_request_key(PromptRequest{.kind = PromptKind::ModuleArchitecture}), "Architecture"},
        {prompt_request_key(PromptRequest{.kind = PromptKind::TypeImplementationSummary,
                                          .target_key = "demo::Widget"}),
         "Implementation"                                                                         },
    };
    PagePlanSet plan_set{
        .plans = {namespace_plan, module_plan}
    };
    auto links = build_link_resolver(plan_set);

    auto namespace_markdown = render_page_markdown(namespace_plan, config, model, outputs, links);
    auto module_markdown = render_page_markdown(module_plan, config, model, outputs, links);

    ASSERT_TRUE(namespace_markdown.has_value());
    ASSERT_TRUE(module_markdown.has_value());
    EXPECT_NE(namespace_markdown->find("[Module demo.math](../../modules/demo/math/index.md)"),
              std::string::npos);
    EXPECT_NE(module_markdown->find("[Namespace demo](../../../namespaces/demo/index.md)"),
              std::string::npos);
    EXPECT_EQ(namespace_markdown->find("struct Widget {}"), std::string::npos);
    EXPECT_EQ(module_markdown->find("struct Widget {}"), std::string::npos);
}

TEST_CASE(render_page_bundle_places_complex_type_members_in_nested_doc_page_without_code_blocks) {
    ScopedTempDir temp("type_member_order");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    auto file = (temp.path / "src" / "widget.cppm").generic_string();

    auto widget = make_type_symbol(500, "Widget", "demo::Widget", file, "Widget type.", "demo");
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
                       PromptRequest{.kind = PromptKind::TypeDeclarationSummary,
                              .target_key = "demo::Widget"},
                       },
    };

    auto outputs = std::unordered_map<std::string, std::string>{
        {prompt_request_key(PromptRequest{.kind = PromptKind::NamespaceSummary}),  "Summary"},
        {prompt_request_key(PromptRequest{.kind = PromptKind::TypeDeclarationSummary,
                                          .target_key = "demo::Widget"}),
         "Declaration"                                                                      },
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
    EXPECT_EQ(detail_page->content.find("struct Widget {}"), std::string::npos);
    EXPECT_EQ(detail_page->content.find("void run() { return 0; }"), std::string::npos);
    EXPECT_NE(root_page->content.find("[Overview](types/widget.md)"),
              std::string::npos);

    auto member_types = detail_page->content.find("## Member Types");
    auto member_variables = detail_page->content.find("## Member Variables");
    auto member_functions = detail_page->content.find("## Member Functions");
    ASSERT_TRUE(member_types != std::string::npos);
    ASSERT_TRUE(member_variables != std::string::npos);
    ASSERT_TRUE(member_functions != std::string::npos);
    EXPECT_LT(member_types, member_variables);
    EXPECT_LT(member_variables, member_functions);
}

TEST_CASE(namespace_summary_prompt_uses_namespace_subject) {
    ScopedTempDir temp("namespace_summary_prompt");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    extract::NamespaceInfo ns;
    ns.name = "demo::config";

    auto file = (temp.path / "src" / "config.cppm").generic_string();
    for(std::uint64_t i = 0; i < 3; ++i) {
        auto name = std::format("Type{}", i);
        auto qualified_name = std::format("demo::config::{}", name);
        auto symbol = make_type_symbol(100 + i,
                                       name,
                                       qualified_name,
                                       file,
                                       "Type documentation.",
                                       "demo::config");
        ns.symbols.push_back(symbol.id);
        add_symbol(model, std::move(symbol));
    }
    model.namespaces.emplace(ns.name, ns);

    PageSummaryCache summaries;
    auto evidence = build_evidence_for_namespace_summary(model.namespaces.at(ns.name),
                                                         model,
                                                         config.evidence_rules,
                                                         summaries,
                                                         config.project_root);
    EXPECT_EQ(evidence.subject_name, "demo::config");
    EXPECT_EQ(evidence.subject_kind, "namespace");

    auto prompt = build_prompt(PromptKind::NamespaceSummary, evidence);

    ASSERT_TRUE(prompt.has_value());
    EXPECT_NE(prompt->find("Write a summary for the namespace `demo::config`."), std::string::npos);
}

TEST_CASE(compute_page_path_sanitizes_workflow_slugs) {
    auto result = compute_page_path(PageIdentity{
        .page_type = PageType::Workflow,
        .normalized_owner_key = "  docs/generate \\ preview  ",
    });

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "workflows/docs-generate-preview.md");
}

TEST_CASE(compute_page_path_flattens_module_partition_pages) {
    auto result = compute_page_path(PageIdentity{
        .page_type = PageType::Module,
        .qualified_name = "demo.math:internal",
    });

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "modules/demo/math/internal.md");
}

TEST_CASE(render_page_markdown_strips_prompt_code_fence_markers) {
    ScopedTempDir temp("strip_prompt_code_fence_markers");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    auto file = (temp.path / "src" / "math.cppm").generic_string();
    auto sym = make_function_symbol(700, "run", "demo::run", "void run()", file, "demo");
    auto sym_id = sym.id;
    add_symbol(model, std::move(sym));
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.math",
                   .is_interface = true,
                   .source_file = file,
                   .symbols = {sym_id},
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
                       PromptRequest{.kind = PromptKind::FunctionImplementationSummary,
                              .target_key = "demo::run"},
                       },
    };

    auto outputs = std::unordered_map<std::string, std::string>{
        {prompt_request_key(PromptRequest{.kind = PromptKind::ModuleSummary}),      "Summary"     },
        {prompt_request_key(PromptRequest{.kind = PromptKind::ModuleArchitecture}), "Architecture"},
        {prompt_request_key(PromptRequest{.kind = PromptKind::FunctionImplementationSummary,
                                          .target_key = "demo::run"}),
         "Algorithm note with inline marker ```helper```.\n\n```cpp\nstep();\n```"                 },
    };

    PagePlanSet plan_set{.plans = {module_plan}};
    auto links = build_link_resolver(plan_set, model);
    auto result = render_page_markdown(module_plan, config, model, outputs, links);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->find("```"), std::string::npos);
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
    add_symbol(model, std::move(widget));
    add_symbol(model, std::move(options));
    add_symbol(model, std::move(state));
    add_symbol(model, std::move(run));
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.math",
                   .is_interface = true,
                   .source_file = file,
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
                              .target_key = "demo::Widget"},
                       },
    };

    auto outputs = std::unordered_map<std::string, std::string>{
        {prompt_request_key(PromptRequest{.kind = PromptKind::ModuleSummary}),      "Summary"     },
        {prompt_request_key(PromptRequest{.kind = PromptKind::ModuleArchitecture}), "Architecture"},
        {prompt_request_key(PromptRequest{.kind = PromptKind::TypeImplementationSummary,
                                          .target_key = "demo::Widget"}),
         "Implementation summary."                                                                },
    };

    PagePlanSet plan_set{.plans = {module_plan}};
    auto links = build_link_resolver(plan_set, model);
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
    ASSERT_TRUE(root_page != bundle->end());
    EXPECT_NE(root_page->content.find("[Overview](types/widget.md)"), std::string::npos);
}

TEST_CASE(render_page_markdown_omits_workflow_wrapper_heading) {
    auto config = make_config(fs::temp_directory_path());
    extract::ProjectModel model;

    PagePlan workflow_plan{
        .page_id = "workflow:demo-flow",
        .page_type = PageType::Workflow,
        .title = "Workflow: Demo Flow",
        .relative_path = "workflows/demo-flow.md",
        .owner_keys = {"demo::entry", "demo::finish"},
        .prompt_requests = {PromptRequest{.kind = PromptKind::Workflow}},
    };

    auto outputs = std::unordered_map<std::string, std::string>{
        {prompt_request_key(PromptRequest{.kind = PromptKind::Workflow}),
         "Title: Demo Flow\n\nThis workflow loads, validates, and emits output."},
    };

    PagePlanSet plan_set{.plans = {workflow_plan}};
    auto links = build_link_resolver(plan_set, model);
    auto result = render_page_markdown(workflow_plan, config, model, outputs, links);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->find("## Workflow\n"), std::string::npos);
    EXPECT_NE(result->find("This workflow loads, validates, and emits output."), std::string::npos);
}

TEST_CASE(validate_output_rejects_whitespace_only_content) {
    auto result = validate_output("  \r\n\t\n");

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().message, "LLM output contains only whitespace");
}

};  // TEST_SUITE(generate)
