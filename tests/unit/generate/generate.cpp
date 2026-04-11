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

    config.page_types.repository = false;
    config.page_types.index = false;
    config.page_types.module_page = true;
    config.page_types.namespace_page = true;
    config.page_types.type_page = true;
    config.page_types.file_page = true;

    config.path_rules.repository_path = "repository.md";
    config.path_rules.index_path = "index.md";
    config.path_rules.module_prefix = "modules";
    config.path_rules.namespace_prefix = "namespaces";
    config.path_rules.type_prefix = "types";
    config.path_rules.file_prefix = "files";
    config.path_rules.name_normalize = "lowercase";

    config.evidence_rules.max_callers = 5;
    config.evidence_rules.max_callees = 5;
    config.evidence_rules.max_siblings = 8;
    config.evidence_rules.max_source_bytes = 4096;
    config.evidence_rules.max_related_summaries = 3;

    config.llm.system_prompt = "You are a writer.";
    config.llm.failure_marker = "[FAILED]";
    config.llm.max_output_length = 4096;
    config.llm.max_prompt_length = 16384;

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

    TEST_CASE(namespace_summary_prompt_uses_namespace_subject_and_fits_limit) {
        ScopedTempDir temp("namespace_summary_prompt");
        fs::create_directories(temp.path / "src");

        auto config = make_config(temp.path);
        config.llm.max_prompt_length = 1024;

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

        auto prompt_result = instantiate_prompt_bounded(
            "Write a Summary for the namespace `{{target_name}}`.\n\n{{evidence}}",
            evidence,
            config.llm.failure_marker,
            config.llm.max_prompt_length);

        ASSERT_TRUE(prompt_result.has_value());
        EXPECT_LE(prompt_result->size(), static_cast<std::size_t>(config.llm.max_prompt_length));
        EXPECT_NE(prompt_result->find("namespace `demo::config`"), std::string::npos);
        EXPECT_EQ(prompt_result->find("namespace `demo::config::Type0`"), std::string::npos);
    }
};
