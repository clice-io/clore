#include "eventide/zest/zest.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>
#include <string>

import clore.config;
import clore.extract;
import clore.generate;

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
    config.language = "English";
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

}  // namespace

TEST_SUITE(generate) {
    TEST_CASE(build_prompts_builds_prompt_for_each_page) {
        ScopedTempDir temp("build_prompts");
        fs::create_directories(temp.path / "src");

        auto config = make_config(temp.path);
        auto model = make_model(temp.path);

        auto result = build_prompts(config, model);

        ASSERT_TRUE(result.has_value());
        ASSERT_EQ(result->size(), 1u);
        auto& prompt = result->front();

        EXPECT_EQ(prompt.relative_path, "src/math.md");
        EXPECT_EQ(prompt.title, "math");
        EXPECT_NE(prompt.prompt.find("## File: `src/math.md`"), std::string::npos);
        EXPECT_NE(prompt.prompt.find("#### function: `math::add`"), std::string::npos);
        EXPECT_NE(prompt.prompt.find("int add(int lhs, int rhs)"), std::string::npos);
        EXPECT_NE(prompt.prompt.find("Adds two integers."), std::string::npos);
    }

    TEST_CASE(build_prompts_merges_header_and_source_with_same_stem) {
        ScopedTempDir temp("build_prompts_merge");
        fs::create_directories(temp.path / "src");

        auto config = make_config(temp.path);

        extract::ProjectModel model;

        auto header_path = (temp.path / "src" / "load.h").generic_string();
        auto source_path = (temp.path / "src" / "load.cpp").generic_string();

        extract::SymbolInfo declared;
        declared.id = extract::SymbolID{.hash = 1};
        declared.kind = extract::SymbolKind::Function;
        declared.name = "load";
        declared.qualified_name = "demo::load";
        declared.signature = "int load()";
        declared.source_snippet = "int load();";
        declared.declaration_location = extract::SourceLocation{.file = header_path, .line = 1, .column = 1};

        extract::SymbolInfo helper;
        helper.id = extract::SymbolID{.hash = 2};
        helper.kind = extract::SymbolKind::Function;
        helper.name = "helper";
        helper.qualified_name = "demo::helper";
        helper.signature = "static int helper()";
        helper.source_snippet = "static int helper() { return 0; }";
        helper.declaration_location = extract::SourceLocation{.file = source_path, .line = 1, .column = 1};

        model.symbols.emplace(declared.id, declared);
        model.symbols.emplace(helper.id, helper);

        model.files.emplace(header_path,
                            extract::FileInfo{.path = header_path, .symbols = {declared.id}});
        model.files.emplace(source_path,
                            extract::FileInfo{.path = source_path, .symbols = {helper.id}});

        auto result = build_prompts(config, model);

        ASSERT_TRUE(result.has_value());
        ASSERT_EQ(result->size(), 1u);
        EXPECT_EQ(result->front().relative_path, "src/load.md");
        EXPECT_NE(result->front().prompt.find("demo::load"), std::string::npos);
        EXPECT_NE(result->front().prompt.find("demo::helper"), std::string::npos);
    }

    TEST_CASE(build_prompts_trims_source_root_prefix_from_output_paths) {
        ScopedTempDir temp("build_prompts_trim_source_root");
        fs::create_directories(temp.path / "src");

        auto config = make_config(temp.path / "src");
        config.workspace_root = temp.path.generic_string();

        extract::ProjectModel model;

        auto source_path = (temp.path / "src" / "config" / "load.cpp").generic_string();
        fs::create_directories(temp.path / "src" / "config");

        extract::SymbolInfo load;
        load.id = extract::SymbolID{.hash = 3};
        load.kind = extract::SymbolKind::Function;
        load.name = "load";
        load.qualified_name = "demo::config::load";
        load.signature = "int load()";
        load.source_snippet = "int load() { return 0; }";
        load.declaration_location = extract::SourceLocation{.file = source_path, .line = 1, .column = 1};

        model.symbols.emplace(load.id, load);
        model.files.emplace(source_path,
                            extract::FileInfo{.path = source_path, .symbols = {load.id}});

        auto result = build_prompts(config, model);

        ASSERT_TRUE(result.has_value());
        ASSERT_EQ(result->size(), 1u);
        EXPECT_EQ(result->front().relative_path, "config/load.md");
        EXPECT_NE(result->front().prompt.find("## File: `config/load.md`"), std::string::npos);
    }

    TEST_CASE(write_prompts_writes_relative_output_paths) {
        ScopedTempDir temp("write_prompts");
        auto output_root = temp.path / "prompts";

        std::vector<PromptPage> prompts{
            PromptPage{
                .relative_path = "src/math.md",
                .title = "math",
                .prompt = "prompt one",
            },
            PromptPage{
                .relative_path = "src/widget.md",
                .title = "widget",
                .prompt = "prompt two\n",
            },
        };

        auto result = write_prompts(prompts, output_root.generic_string());

        ASSERT_TRUE(result.has_value());
        auto first = output_root / "src" / "math.md";
        auto second = output_root / "src" / "widget.md";
        ASSERT_TRUE(fs::exists(first));
        ASSERT_TRUE(fs::exists(second));

        EXPECT_EQ(read_text_file(first), "prompt one");
        EXPECT_EQ(read_text_file(second), "prompt two\n");
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

    TEST_CASE(build_module_graph_and_prompts_preserve_distinct_module_units) {
        ScopedTempDir temp("build_module_prompts");
        fs::create_directories(temp.path / "src");

        auto config = make_config(temp.path);

        extract::ProjectModel model;
        model.uses_modules = true;

        auto util_file = (temp.path / "src" / "util.cppm").generic_string();
        auto main_file = (temp.path / "src" / "math.cppm").generic_string();
        auto partition_file = (temp.path / "src" / "math.detail.cppm").generic_string();
        auto impl_file = (temp.path / "src" / "math_impl.cpp").generic_string();

        auto util_symbol = make_symbol(10, "helper", "demo::util::helper", "int helper()", util_file);
        auto api_symbol = make_symbol(11, "add", "demo::math::add", "int add(int lhs, int rhs)", main_file);
        auto partition_symbol = make_symbol(12, "detail", "demo::math::detail", "int detail()", partition_file);
        auto impl_symbol = make_symbol(13, "internal", "demo::math::internal", "int internal()", impl_file);

        model.symbols.emplace(util_symbol.id, util_symbol);
        model.symbols.emplace(api_symbol.id, api_symbol);
        model.symbols.emplace(partition_symbol.id, partition_symbol);
        model.symbols.emplace(impl_symbol.id, impl_symbol);

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
                .imports = {"demo.util", "demo.util"},
                .symbols = {api_symbol.id},
            });
        model.modules.emplace(
            partition_file,
            extract::ModuleUnit{
                .name = "demo.math:detail",
                .is_interface = true,
                .source_file = partition_file,
                .imports = {"demo.util"},
                .symbols = {partition_symbol.id},
            });
        model.modules.emplace(
            impl_file,
            extract::ModuleUnit{
                .name = "demo.math",
                .is_interface = false,
                .source_file = impl_file,
                .imports = {"demo.util", "demo.util"},
                .symbols = {impl_symbol.id},
            });

        auto graph = build_page_graph(config, model);

        ASSERT_EQ(graph.nodes.size(), 3u);
        ASSERT_TRUE(graph.nodes.contains("demo.util.md"));
        ASSERT_TRUE(graph.nodes.contains("demo.math.md"));
        ASSERT_TRUE(graph.nodes.contains("demo.math/detail.md"));

        auto& main_node = graph.nodes.at("demo.math.md");
        auto& partition_node = graph.nodes.at("demo.math/detail.md");

        EXPECT_EQ(std::count(main_node.depends_on.begin(), main_node.depends_on.end(), "demo.util.md"),
                  1);
        EXPECT_EQ(std::count(partition_node.depends_on.begin(), partition_node.depends_on.end(),
                             "demo.util.md"),
                  1);
        EXPECT_EQ(std::count(partition_node.depends_on.begin(), partition_node.depends_on.end(),
                             "demo.math.md"),
                  1);

        auto pos_util = std::find(graph.generation_order.begin(), graph.generation_order.end(),
                                  "demo.util.md");
        auto pos_main = std::find(graph.generation_order.begin(), graph.generation_order.end(),
                                  "demo.math.md");
        auto pos_partition = std::find(graph.generation_order.begin(), graph.generation_order.end(),
                                       "demo.math/detail.md");
        ASSERT_TRUE(pos_util != graph.generation_order.end());
        ASSERT_TRUE(pos_main != graph.generation_order.end());
        ASSERT_TRUE(pos_partition != graph.generation_order.end());
        EXPECT_LT(pos_util, pos_main);
        EXPECT_LT(pos_main, pos_partition);

        auto prompts_result = build_prompts(config, model);
        ASSERT_TRUE(prompts_result.has_value());
        ASSERT_EQ(prompts_result->size(), 3u);

        auto main_prompt_it = std::find_if(prompts_result->begin(), prompts_result->end(),
                                           [](const PromptPage& page) {
                                               return page.relative_path == "demo.math.md";
                                           });
        ASSERT_TRUE(main_prompt_it != prompts_result->end());
        EXPECT_NE(main_prompt_it->prompt.find("## Module: `demo.math`"), std::string::npos);
        EXPECT_NE(main_prompt_it->prompt.find("#### function: `demo::math::internal`"),
                  std::string::npos);

        auto partition_prompt_it = std::find_if(prompts_result->begin(), prompts_result->end(),
                                                [](const PromptPage& page) {
                                                    return page.relative_path == "demo.math/detail.md";
                                                });
        ASSERT_TRUE(partition_prompt_it != prompts_result->end());
        EXPECT_NE(partition_prompt_it->prompt.find("## Module: `demo.math:detail`"),
                  std::string::npos);
    }
};