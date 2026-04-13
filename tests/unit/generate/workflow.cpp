#include "eventide/zest/zest.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <format>
#include <ranges>
#include <string>

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
                 const std::string& file) -> extract::SymbolInfo {
    extract::SymbolInfo symbol;
    symbol.id = extract::SymbolID{.hash = id};
    symbol.kind = extract::SymbolKind::Function;
    symbol.name = std::string(name);
    symbol.qualified_name = std::string(qualified_name);
    symbol.signature = std::format("void {}()", name);
    symbol.declaration_location = extract::SourceLocation{
        .file = file,
        .line = 1,
        .column = 1,
    };
    return symbol;
}

auto make_base_config(const fs::path& root) -> config::TaskConfig {
    config::TaskConfig cfg;
    cfg.project_root = root.generic_string();
    cfg.output_root = (root / "docs").generic_string();
    cfg.workspace_root = root.generic_string();
    cfg.extract.max_snippet_bytes = 512;

    cfg.path_rules.index_path = "index.md";
    cfg.path_rules.module_prefix = "modules";
    cfg.path_rules.namespace_prefix = "namespaces";
    cfg.path_rules.type_prefix = "types";
    cfg.path_rules.file_prefix = "files";
    cfg.path_rules.workflow_prefix = "workflows";
    cfg.path_rules.name_normalize = "lowercase";

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
        config.page_types.index = true;
        config.page_types.module_page = true;
        config.page_types.workflow_page = true;

        extract::ProjectModel model;
        model.uses_modules = true;

        auto ui_file = (temp.path / "src" / "ui.cppm").generic_string();
        auto service_file = (temp.path / "src" / "service.cppm").generic_string();
        auto repo_file = (temp.path / "src" / "repo.cppm").generic_string();
        auto admin_ui_file = (temp.path / "src" / "admin_ui.cppm").generic_string();
        auto admin_logic_file = (temp.path / "src" / "admin_logic.cppm").generic_string();

        auto ui_run = make_symbol(1, "run", "app::ui::run", ui_file);
        auto service_handle = make_symbol(2, "handle", "app::service::handle", service_file);
        auto repo_save = make_symbol(3, "save", "app::repo::save", repo_file);
        auto admin_entry = make_symbol(4, "entry", "app::admin::entry", admin_ui_file);
        auto admin_run = make_symbol(5, "run", "app::admin::run", admin_logic_file);

        ui_run.calls = {service_handle.id};
        service_handle.calls = {repo_save.id};
        admin_entry.calls = {admin_run.id};

        model.symbols.emplace(ui_run.id, ui_run);
        model.symbols.emplace(service_handle.id, service_handle);
        model.symbols.emplace(repo_save.id, repo_save);
        model.symbols.emplace(admin_entry.id, admin_entry);
        model.symbols.emplace(admin_run.id, admin_run);

        model.modules.emplace(ui_file, extract::ModuleUnit{
            .name = "app.ui",
            .is_interface = true,
            .source_file = ui_file,
            .symbols = {ui_run.id},
        });
        model.modules.emplace(service_file, extract::ModuleUnit{
            .name = "app.service",
            .is_interface = true,
            .source_file = service_file,
            .symbols = {service_handle.id},
        });
        model.modules.emplace(repo_file, extract::ModuleUnit{
            .name = "app.repo",
            .is_interface = true,
            .source_file = repo_file,
            .symbols = {repo_save.id},
        });
        model.modules.emplace(admin_ui_file, extract::ModuleUnit{
            .name = "app.admin_ui",
            .is_interface = true,
            .source_file = admin_ui_file,
            .symbols = {admin_entry.id},
        });
        model.modules.emplace(admin_logic_file, extract::ModuleUnit{
            .name = "app.admin_logic",
            .is_interface = true,
            .source_file = admin_logic_file,
            .symbols = {admin_run.id},
        });

        auto result = build_page_plan_set(config, model);
        ASSERT_TRUE(result.has_value());

        auto feature_count = std::ranges::count_if(result->plans, [](const PagePlan& plan) {
            return plan.page_type == PageType::Workflow;
        });
        EXPECT_EQ(feature_count, 2u);

        auto has_main_chain = std::ranges::any_of(result->plans, [](const PagePlan& plan) {
            if(plan.page_type != PageType::Workflow) return false;
            return std::ranges::find(plan.owner_keys, "app::ui::run") != plan.owner_keys.end() &&
                   std::ranges::find(plan.owner_keys, "app::service::handle") != plan.owner_keys.end() &&
                   std::ranges::find(plan.owner_keys, "app::repo::save") != plan.owner_keys.end();
        });
        EXPECT_TRUE(has_main_chain);

        auto has_admin_chain = std::ranges::any_of(result->plans, [](const PagePlan& plan) {
            if(plan.page_type != PageType::Workflow) return false;
            return std::ranges::find(plan.owner_keys, "app::admin::entry") != plan.owner_keys.end() &&
                   std::ranges::find(plan.owner_keys, "app::admin::run") != plan.owner_keys.end();
        });
        EXPECT_TRUE(has_admin_chain);
    }

    TEST_CASE(build_page_plan_set_generates_workflow_pages_without_modules) {
        ScopedTempDir temp("call_chain_no_modules");
        fs::create_directories(temp.path / "include");

        auto config = make_base_config(temp.path);
        config.page_types.index = true;
        config.page_types.module_page = false;
        config.page_types.workflow_page = true;

        extract::ProjectModel model;
        model.uses_modules = false;

        auto header_file = (temp.path / "include" / "workflow.hpp").generic_string();

        auto entry = make_symbol(101, "entry", "demo::workflow::entry", header_file);
        auto execute = make_symbol(102, "execute", "demo::workflow::execute", header_file);
        auto persist = make_symbol(103, "persist", "demo::workflow::persist", header_file);

        entry.calls = {execute.id};
        execute.calls = {persist.id};

        model.symbols.emplace(entry.id, entry);
        model.symbols.emplace(execute.id, execute);
        model.symbols.emplace(persist.id, persist);

        auto result = build_page_plan_set(config, model);
        ASSERT_TRUE(result.has_value());

        auto feature_count = std::ranges::count_if(result->plans, [](const PagePlan& plan) {
            return plan.page_type == PageType::Workflow;
        });
        EXPECT_EQ(feature_count, 1u);

        auto has_chain = std::ranges::any_of(result->plans, [](const PagePlan& plan) {
            if(plan.page_type != PageType::Workflow) return false;
            return std::ranges::find(plan.owner_keys, "demo::workflow::entry") !=
                       plan.owner_keys.end() &&
                   std::ranges::find(plan.owner_keys, "demo::workflow::execute") !=
                       plan.owner_keys.end() &&
                   std::ranges::find(plan.owner_keys, "demo::workflow::persist") !=
                       plan.owner_keys.end();
        });
        EXPECT_TRUE(has_chain);
    }

    TEST_CASE(render_all_workflows_index_sorts_workflow_links) {
        config::TaskConfig cfg;
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
            .relative_path = "workflows/alpha-flow/index.md",
            .owner_keys = {"alpha::entry", "alpha::exit"},
        };
        PagePlan beta_plan{
            .page_id = "workflow:beta-flow",
            .page_type = PageType::Workflow,
            .title = "Beta",
            .relative_path = "workflows/beta-flow/index.md",
            .owner_keys = {"beta::entry", "beta::exit"},
        };

        auto links = build_link_resolver(PagePlanSet{
            .plans = {index_plan, alpha_plan, beta_plan},
        });
        auto block = render_deterministic_block("all_workflows", index_plan, model, cfg, links);
        auto alpha_pos = block.find("Alpha Flow");
        auto beta_pos = block.find("Beta Flow");

        ASSERT_TRUE(alpha_pos != std::string::npos);
        ASSERT_TRUE(beta_pos != std::string::npos);
        EXPECT_LT(alpha_pos, beta_pos);
    }

    TEST_CASE(generate_dry_run_prepends_vitepress_frontmatter) {
        ScopedTempDir temp("vitepress_frontmatter");
        fs::create_directories(temp.path / "src");
        fs::create_directories(temp.path / "templates");

        auto file_template = temp.path / "templates" / "file.md";
        {
            std::ofstream out(file_template);
            out << "# {{title}}\n\n{{block:declared_symbols}}\n";
        }

        auto config = make_base_config(temp.path);
        config.page_types.file_page = true;
        config.page_templates.file_page = file_template.generic_string();
        config.builtin.vitepress = true;

        extract::ProjectModel model;
        auto source_file = (temp.path / "src" / "demo.cpp").generic_string();
        auto symbol = make_symbol(11, "demo", "app::demo", source_file);
        model.symbols.emplace(symbol.id, symbol);
        model.files.emplace(source_file, extract::FileInfo{
            .path = source_file,
            .symbols = {symbol.id},
        });

        auto generated = generate_dry_run(config, model);
        ASSERT_TRUE(generated.has_value());
        ASSERT_FALSE(generated->empty());
        auto& page = generated->front().content;
        EXPECT_TRUE(page.starts_with("---\nlayout: doc\n"));
        EXPECT_NE(page.find("title:"), std::string::npos);
        EXPECT_NE(page.find("titleTemplate: false"), std::string::npos);
        EXPECT_NE(page.find("description:"), std::string::npos);
        EXPECT_NE(page.find("navbar: true"), std::string::npos);
        EXPECT_NE(page.find("sidebar: true"), std::string::npos);
        EXPECT_NE(page.find("aside: false"), std::string::npos);
        EXPECT_NE(page.find("outline: false"), std::string::npos);
        EXPECT_NE(page.find("footer: true"), std::string::npos);
        EXPECT_NE(page.find("pageClass:"), std::string::npos);
    }
};
