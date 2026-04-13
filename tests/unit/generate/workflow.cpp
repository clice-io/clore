#include "eventide/zest/zest.h"

#include <chrono>
#include <filesystem>
#include <format>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

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
        path = fs::temp_directory_path() / std::format("clore_feature_{}_{}", name, ticks);
        fs::create_directories(path);
    }

    ~ScopedTempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
};

auto make_symbol(std::uint64_t id,
                 std::string_view name,
                 std::string_view qualified_name,
                 const std::string& file,
                 std::string_view ns_name = {}) -> extract::SymbolInfo {
    extract::SymbolInfo symbol;
    symbol.id = extract::SymbolID{.hash = id};
    symbol.kind = extract::SymbolKind::Function;
    symbol.name = std::string(name);
    symbol.qualified_name = std::string(qualified_name);
    symbol.enclosing_namespace = std::string(ns_name);
    symbol.signature = std::format("void {}()", name);
    symbol.source_snippet = std::format("void {}() {{}}", name);
    symbol.declaration_location = extract::SourceLocation{
        .file = file,
        .line = 1,
        .column = 1,
    };
    symbol.definition_location = extract::SourceLocation{
        .file = file,
        .line = 2,
        .column = 1,
    };
    return symbol;
}

auto add_symbol(extract::ProjectModel& model, extract::SymbolInfo symbol) -> void {
    auto file = symbol.declaration_location.file;
    auto id = symbol.id;
    auto qname = symbol.qualified_name;

    auto& file_info = model.files[file];
    file_info.path = file;
    file_info.symbols.push_back(id);
    if(std::ranges::find(model.file_order, file) == model.file_order.end()) {
        model.file_order.push_back(file);
    }

    model.symbol_ids_by_qualified_name[qname] = id;
    model.symbols.emplace(id, std::move(symbol));
}

auto add_module(extract::ProjectModel& model, extract::ModuleUnit module) -> void {
    model.uses_modules = true;
    model.module_name_to_source[module.name] = module.source_file;
    model.modules.emplace(module.source_file, std::move(module));
}

auto make_base_config(const fs::path& root) -> config::TaskConfig {
    config::TaskConfig cfg;
    cfg.project_root = root.generic_string();
    cfg.output_root = (root / "docs").generic_string();
    cfg.workspace_root = root.generic_string();
    cfg.extract.max_snippet_bytes = 512;

    cfg.evidence_rules.max_callers = 3;
    cfg.evidence_rules.max_callees = 3;
    cfg.evidence_rules.max_siblings = 3;
    cfg.evidence_rules.max_source_bytes = 256;
    cfg.evidence_rules.max_related_summaries = 2;

    cfg.workflow_rules.min_chain_symbols = 2;
    cfg.workflow_rules.min_new_symbols = 1;
    cfg.workflow_rules.max_symbol_overlap_ratio_percent = 50;
    cfg.workflow_rules.max_workflow_pages = 8;
    cfg.workflow_rules.llm_review_top_k = 6;
    cfg.workflow_rules.llm_selected_count = 4;

    cfg.llm.system_prompt = "system";
    cfg.llm.retry_count = 1;
    cfg.llm.retry_initial_backoff_ms = 1;

    return cfg;
}

}  // namespace

TEST_SUITE(workflow) {
    TEST_CASE(build_page_plan_set_extracts_independent_call_chain_features) {
        ScopedTempDir temp("call_chain_planner");
        fs::create_directories(temp.path / "src");

        auto config = make_base_config(temp.path);

        extract::ProjectModel model;
        auto ui_file = (temp.path / "src" / "ui.cppm").generic_string();
        auto service_file = (temp.path / "src" / "service.cppm").generic_string();
        auto repo_file = (temp.path / "src" / "repo.cppm").generic_string();
        auto admin_ui_file = (temp.path / "src" / "admin_ui.cppm").generic_string();
        auto admin_logic_file = (temp.path / "src" / "admin_logic.cppm").generic_string();

        auto ui_run = make_symbol(1, "run", "app::ui::run", ui_file, "app::ui");
        auto service_handle = make_symbol(2, "handle", "app::service::handle", service_file,
                                          "app::service");
        auto repo_save = make_symbol(3, "save", "app::repo::save", repo_file, "app::repo");
        auto admin_entry = make_symbol(4, "entry", "app::admin::entry", admin_ui_file,
                                       "app::admin");
        auto admin_run = make_symbol(5, "run", "app::admin::run", admin_logic_file,
                                     "app::admin");

        ui_run.calls = {service_handle.id};
        service_handle.calls = {repo_save.id};
        admin_entry.calls = {admin_run.id};

        auto ui_id = ui_run.id;
        auto service_id = service_handle.id;
        auto repo_id = repo_save.id;
        auto admin_entry_id = admin_entry.id;
        auto admin_run_id = admin_run.id;
        add_symbol(model, std::move(ui_run));
        add_symbol(model, std::move(service_handle));
        add_symbol(model, std::move(repo_save));
        add_symbol(model, std::move(admin_entry));
        add_symbol(model, std::move(admin_run));

        add_module(model, extract::ModuleUnit{
            .name = "app.ui",
            .is_interface = true,
            .source_file = ui_file,
            .symbols = {ui_id},
        });
        add_module(model, extract::ModuleUnit{
            .name = "app.service",
            .is_interface = true,
            .source_file = service_file,
            .symbols = {service_id},
        });
        add_module(model, extract::ModuleUnit{
            .name = "app.repo",
            .is_interface = true,
            .source_file = repo_file,
            .symbols = {repo_id},
        });
        add_module(model, extract::ModuleUnit{
            .name = "app.admin_ui",
            .is_interface = true,
            .source_file = admin_ui_file,
            .symbols = {admin_entry_id},
        });
        add_module(model, extract::ModuleUnit{
            .name = "app.admin_logic",
            .is_interface = true,
            .source_file = admin_logic_file,
            .symbols = {admin_run_id},
        });

        auto result = build_page_plan_set(config, model);
        ASSERT_TRUE(result.has_value());

        auto workflow_count = std::ranges::count_if(result->plans, [](const PagePlan& plan) {
            return plan.page_type == PageType::Workflow;
        });
        EXPECT_EQ(workflow_count, 2u);

        auto has_main_chain = std::ranges::any_of(result->plans, [](const PagePlan& plan) {
            if(plan.page_type != PageType::Workflow) {
                return false;
            }
            return std::ranges::find(plan.owner_keys, "app::ui::run") != plan.owner_keys.end() &&
                   std::ranges::find(plan.owner_keys, "app::service::handle") != plan.owner_keys.end() &&
                   std::ranges::find(plan.owner_keys, "app::repo::save") != plan.owner_keys.end();
        });
        EXPECT_TRUE(has_main_chain);

        auto has_admin_chain = std::ranges::any_of(result->plans, [](const PagePlan& plan) {
            if(plan.page_type != PageType::Workflow) {
                return false;
            }
            return std::ranges::find(plan.owner_keys, "app::admin::entry") != plan.owner_keys.end() &&
                   std::ranges::find(plan.owner_keys, "app::admin::run") != plan.owner_keys.end();
        });
        EXPECT_TRUE(has_admin_chain);
    }

    TEST_CASE(build_page_plan_set_generates_workflow_pages_without_modules) {
        ScopedTempDir temp("call_chain_no_modules");
        fs::create_directories(temp.path / "include");

        auto config = make_base_config(temp.path);

        extract::ProjectModel model;
        auto header_file = (temp.path / "include" / "workflow.hpp").generic_string();

        auto entry = make_symbol(101, "entry", "demo::workflow::entry", header_file,
                                 "demo::workflow");
        auto execute = make_symbol(102, "execute", "demo::workflow::execute", header_file,
                                   "demo::workflow");
        auto persist = make_symbol(103, "persist", "demo::workflow::persist", header_file,
                                   "demo::workflow");

        entry.calls = {execute.id};
        execute.calls = {persist.id};

        add_symbol(model, std::move(entry));
        add_symbol(model, std::move(execute));
        add_symbol(model, std::move(persist));

        auto result = build_page_plan_set(config, model);
        ASSERT_TRUE(result.has_value());

        auto workflow_count = std::ranges::count_if(result->plans, [](const PagePlan& plan) {
            return plan.page_type == PageType::Workflow;
        });
        EXPECT_EQ(workflow_count, 1u);

        auto workflow_plan = std::ranges::find_if(result->plans, [](const PagePlan& plan) {
            return plan.page_type == PageType::Workflow;
        });
        ASSERT_TRUE(workflow_plan != result->plans.end());
        EXPECT_EQ(workflow_plan->page_id, "workflow:entry-to-persist");
        EXPECT_EQ(workflow_plan->relative_path, "workflows/entry-to-persist.md");
    }

    TEST_CASE(build_page_plan_set_disambiguates_duplicate_workflow_slugs_without_hash_suffix) {
        ScopedTempDir temp("workflow_slug_disambiguation");
        fs::create_directories(temp.path / "src");

        auto config = make_base_config(temp.path);

        extract::ProjectModel model;
        auto src_file = (temp.path / "src" / "workflow.cpp").generic_string();

        auto a_start = make_symbol(201, "start", "demo::a::start", src_file, "demo::a");
        auto a_mid = make_symbol(202, "process", "demo::a::process", src_file, "demo::a");
        auto a_end = make_symbol(203, "end", "demo::a::end", src_file, "demo::a");
        auto b_start = make_symbol(204, "start", "demo::b::start", src_file, "demo::b");
        auto b_mid = make_symbol(205, "process", "demo::b::process", src_file, "demo::b");
        auto b_end = make_symbol(206, "end", "demo::b::end", src_file, "demo::b");

        a_start.calls = {a_mid.id};
        a_mid.calls = {a_end.id};
        b_start.calls = {b_mid.id};
        b_mid.calls = {b_end.id};

        add_symbol(model, std::move(a_start));
        add_symbol(model, std::move(a_mid));
        add_symbol(model, std::move(a_end));
        add_symbol(model, std::move(b_start));
        add_symbol(model, std::move(b_mid));
        add_symbol(model, std::move(b_end));

        auto result = build_page_plan_set(config, model);
        ASSERT_TRUE(result.has_value());

        EXPECT_TRUE(std::ranges::any_of(result->plans, [](const PagePlan& plan) {
            return plan.page_type == PageType::Workflow &&
                   plan.page_id == "workflow:start-to-end" &&
                   plan.relative_path == "workflows/start-to-end.md";
        }));
        EXPECT_TRUE(std::ranges::any_of(result->plans, [](const PagePlan& plan) {
            return plan.page_type == PageType::Workflow &&
                   plan.page_id == "workflow:start-to-end-2" &&
                   plan.relative_path == "workflows/start-to-end-2.md";
        }));
    }

    TEST_CASE(render_page_markdown_sorts_workflow_links_in_index) {
        auto cfg = make_base_config(fs::temp_directory_path());
        extract::ProjectModel model;

        PagePlan index_plan{
            .page_id = "index",
            .page_type = PageType::Index,
            .title = "Index",
            .relative_path = "index.md",
            .linked_pages = {"workflow:beta-flow", "workflow:alpha-flow"},
        };
        PagePlan alpha_plan{
            .page_id = "workflow:alpha-flow",
            .page_type = PageType::Workflow,
            .title = "Alpha",
            .relative_path = "workflows/alpha-flow.md",
            .owner_keys = {"alpha::entry", "alpha::exit"},
        };
        PagePlan beta_plan{
            .page_id = "workflow:beta-flow",
            .page_type = PageType::Workflow,
            .title = "Beta",
            .relative_path = "workflows/beta-flow.md",
            .owner_keys = {"beta::entry", "beta::exit"},
        };

        auto links = build_link_resolver(PagePlanSet{.plans = {index_plan, alpha_plan, beta_plan}});
        auto outputs = std::unordered_map<std::string, std::string>{
            {prompt_request_key(PromptRequest{.kind = PromptKind::IndexOverview}), "Overview"},
            {prompt_request_key(PromptRequest{.kind = PromptKind::IndexReadingGuide}), "Guide"},
        };

        auto markdown = render_page_markdown(index_plan, cfg, model, outputs, links);

        ASSERT_TRUE(markdown.has_value());
        auto alpha_pos = markdown->find("Alpha");
        auto beta_pos = markdown->find("Beta");
        ASSERT_TRUE(alpha_pos != std::string::npos);
        ASSERT_TRUE(beta_pos != std::string::npos);
        EXPECT_LT(alpha_pos, beta_pos);
    }

    TEST_CASE(build_page_plan_set_workflows_depend_only_on_referenced_pages) {
        ScopedTempDir temp("workflow_dependencies");
        fs::create_directories(temp.path / "src");

        auto config = make_base_config(temp.path);

        extract::ProjectModel model;
        auto flow_file = (temp.path / "src" / "flow.cpp").generic_string();
        auto extra_file = (temp.path / "src" / "extra.cpp").generic_string();

        auto entry = make_symbol(301, "entry", "demo::workflow::entry", flow_file,
                                 "demo::workflow");
        auto step = make_symbol(302, "step", "demo::workflow::step", flow_file,
                                "demo::workflow");
        entry.calls = {step.id};

        extract::SymbolInfo unrelated_type;
        unrelated_type.id = extract::SymbolID{.hash = 303};
        unrelated_type.kind = extract::SymbolKind::Struct;
        unrelated_type.name = "Unrelated";
        unrelated_type.qualified_name = "demo::Unrelated";
        unrelated_type.signature = "struct Unrelated";
        unrelated_type.enclosing_namespace = "demo";
        unrelated_type.declaration_location = extract::SourceLocation{
            .file = extra_file,
            .line = 1,
            .column = 1,
        };

        add_symbol(model, std::move(entry));
        add_symbol(model, std::move(step));
        add_symbol(model, std::move(unrelated_type));

        auto result = build_page_plan_set(config, model);
        ASSERT_TRUE(result.has_value());

        auto workflow_plan = std::ranges::find_if(result->plans, [](const PagePlan& plan) {
            return plan.page_type == PageType::Workflow;
        });
        ASSERT_TRUE(workflow_plan != result->plans.end());

        auto unrelated_file_page = std::string("file:") + extra_file;
        auto workflow_file_page = std::string("file:") + flow_file;

        EXPECT_TRUE(std::ranges::none_of(workflow_plan->depends_on_pages,
                                         [](const std::string& page_id) {
                                             return page_id.starts_with("type:");
                                         }));
        EXPECT_EQ(std::ranges::find(workflow_plan->depends_on_pages, unrelated_file_page),
                  workflow_plan->depends_on_pages.end());
        EXPECT_NE(std::ranges::find(workflow_plan->depends_on_pages, workflow_file_page),
                  workflow_plan->depends_on_pages.end());
    }

    TEST_CASE(generate_dry_run_emits_minimal_frontmatter) {
        ScopedTempDir temp("minimal_frontmatter");
        fs::create_directories(temp.path / "src");

        auto config = make_base_config(temp.path);

        extract::ProjectModel model;
        auto source_file = (temp.path / "src" / "demo.cpp").generic_string();
        auto symbol = make_symbol(11, "demo", "app::demo", source_file, "app");
        add_symbol(model, std::move(symbol));

        auto generated = generate_dry_run(config, model);
        ASSERT_TRUE(generated.has_value());
        EXPECT_TRUE(std::ranges::any_of(*generated, [](const GeneratedPage& page) {
            return page.content.starts_with("---\ntitle:") &&
                   page.content.find("title:") != std::string::npos &&
                   page.content.find("description:") != std::string::npos &&
                   page.content.find("layout: doc") != std::string::npos &&
                   page.content.find("template: doc") != std::string::npos &&
                   page.content.find("pageClass:") == std::string::npos &&
                   page.content.find("outline:") == std::string::npos;
        }));
    }
};