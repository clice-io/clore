#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>

#include "extract/compdb.h"
#include "kota/zest/zest.h"

import config;
import extract;
import generate;

using namespace clore;

TEST_SUITE(extract_integration) {

TEST_CASE(full_pipeline) {
    namespace fs = std::filesystem;

    auto temp_dir = fs::temp_directory_path() / "clore_test_integration";
    fs::create_directories(temp_dir / "src");

    // Create source files
    {
        std::ofstream f(temp_dir / "src" / "math.h");
        f << R"(
#pragma once

namespace mylib::math {

/// Add two integers.
int add(int a, int b);

/// Multiply two integers.
int multiply(int a, int b);

}  // namespace mylib::math
)";
    }

    {
        std::ofstream f(temp_dir / "src" / "math.cpp");
        f << R"(
#include "math.h"

namespace mylib::math {

int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }

}  // namespace mylib::math
)";
    }

    {
        std::ofstream f(temp_dir / "src" / "widget.h");
        f << R"(
#pragma once

#include <string>

namespace mylib {

/// A simple widget with a name and value.
class Widget {
public:
    Widget(std::string name, int value);
    const std::string& name() const;
    int value() const;
    void set_value(int v);

private:
    std::string name_;
    int value_;
};

}  // namespace mylib
)";
    }

    {
        std::ofstream f(temp_dir / "src" / "widget.cpp");
        f << R"(
#include "widget.h"

namespace mylib {

Widget::Widget(std::string name, int value) : name_(std::move(name)), value_(value) {}
const std::string& Widget::name() const { return name_; }
int Widget::value() const { return value_; }
void Widget::set_value(int v) { value_ = v; }

}  // namespace mylib
)";
    }

    // Create compile_commands.json
    auto src_dir = temp_dir / "src";
    clore::testing::write_compile_commands(temp_dir / "compile_commands.json",
                                           {
                                               {
                                                .directory = src_dir,
                                                .file = src_dir / "math.cpp",
                                                .arguments =
                                                       {
                                                           "clang++",
                                                           "-std=c++23",
                                                           std::format("-I{}", src_dir.string()),
                                                           "-c",
                                                           "math.cpp",
                                                           "-o",
                                                           "math.o",
                                                       }, },
                                               {
                                                .directory = src_dir,
                                                .file = src_dir / "widget.cpp",
                                                .arguments =
                                                       {
                                                           "clang++",
                                                           "-std=c++23",
                                                           std::format("-I{}", src_dir.string()),
                                                           "-c",
                                                           "widget.cpp",
                                                           "-o",
                                                           "widget.o",
                                                       }, }
    });

    config::TaskConfig task_config;
    task_config.compile_commands_path = (temp_dir / "compile_commands.json").string();
    task_config.project_root = temp_dir.string();
    task_config.output_root = (temp_dir / "docs").string();
    task_config.workspace_root = temp_dir.string();

    auto result = extract::extract_project(task_config);
    ASSERT_TRUE(result.has_value());

    auto& model = *result;
    EXPECT_GT(model.symbols.size(), 0u);
    EXPECT_GT(model.files.size(), 0u);

    // Verify we found the expected symbols
    bool found_add = false;
    bool found_multiply = false;
    bool found_widget = false;

    for(auto& [id, sym]: model.symbols) {
        if(sym.name == "add")
            found_add = true;
        if(sym.name == "multiply")
            found_multiply = true;
        if(sym.name == "Widget")
            found_widget = true;
    }

    EXPECT_TRUE(found_add);
    EXPECT_TRUE(found_multiply);
    EXPECT_TRUE(found_widget);

    auto math_h = extract::canonical_graph_path(temp_dir / "src" / "math.h");
    auto math_cpp = extract::canonical_graph_path(temp_dir / "src" / "math.cpp");
    auto add_it = std::ranges::find_if(model.symbols, [](const auto& item) {
        return item.second.qualified_name == "mylib::math::add";
    });
    ASSERT_TRUE(add_it != model.symbols.end());
    EXPECT_EQ(add_it->second.declaration_location.file, math_h);
    ASSERT_TRUE(add_it->second.definition_location.has_value());
    EXPECT_EQ(add_it->second.definition_location->file, math_cpp);
    EXPECT_TRUE(model.files.contains(add_it->second.declaration_location.file));
    EXPECT_TRUE(model.files.contains(add_it->second.definition_location->file));

    fs::remove_all(temp_dir);
}

TEST_CASE(writes_extract_cache_under_clice_cache_root) {
    namespace fs = std::filesystem;

    auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    auto root = fs::temp_directory_path() / std::format("clore_extract_cache_test_{}", ticks);
    fs::create_directories(root / "src");

    {
        std::ofstream f(root / "src" / "math.cpp");
        f << R"(
int add(int lhs, int rhs) {
    return lhs + rhs;
}
)";
    }

    clore::testing::write_compile_commands(root / "compile_commands.json",
                                           {
                                               {
                                                .directory = root / "src",
                                                .file = root / "src" / "math.cpp",
                                                .arguments =
                                                       {
                                                           "clang++",
                                                           "-std=c++23",
                                                           "-c",
                                                           "math.cpp",
                                                           "-o",
                                                           "math.o",
                                                       }, }
    });

    config::TaskConfig cfg;
    cfg.compile_commands_path = (root / "compile_commands.json").string();
    cfg.project_root = root.string();
    cfg.output_root = (root / "out").string();
    cfg.workspace_root = root.string();

    auto first = extract::extract_project(cfg);
    ASSERT_TRUE(first.has_value());
    auto first_symbols = first->symbols.size();
    auto first_files = first->files.size();

    auto cache_path = root / ".clice" / "cache" / "clore" / "extract" / "cache.json";
    EXPECT_TRUE(fs::exists(cache_path));
    auto clice_cache_path = root / ".clice" / "cache" / "cache.json";
    EXPECT_TRUE(fs::exists(clice_cache_path));

    auto second = extract::extract_project(cfg);
    ASSERT_TRUE(second.has_value());

    EXPECT_EQ(second->symbols.size(), first_symbols);
    EXPECT_EQ(second->files.size(), first_files);

    auto first_add = std::ranges::find_if(first->symbols, [](const auto& item) {
        return item.second.name == "add";
    });
    auto second_add = std::ranges::find_if(second->symbols, [](const auto& item) {
        return item.second.name == "add";
    });
    ASSERT_TRUE(first_add != first->symbols.end());
    ASSERT_TRUE(second_add != second->symbols.end());
    EXPECT_EQ(second_add->second.signature, first_add->second.signature);
    EXPECT_EQ(second_add->second.source_snippet, first_add->second.source_snippet);
    EXPECT_EQ(second_add->second.declaration_location.file,
              first_add->second.declaration_location.file);
    EXPECT_EQ(second_add->second.declaration_location.line,
              first_add->second.declaration_location.line);
    ASSERT_TRUE(first_add->second.definition_location.has_value());
    ASSERT_TRUE(second_add->second.definition_location.has_value());
    EXPECT_EQ(second_add->second.definition_location->file,
              first_add->second.definition_location->file);
    EXPECT_EQ(second->file_order, first->file_order);

    fs::remove_all(root);
}

TEST_CASE(detects_distinct_module_units_in_project_model) {
    namespace fs = std::filesystem;

    auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    auto root = fs::temp_directory_path() / std::format("clore_module_extract_test_{}", ticks);
    fs::create_directories(root / "src");

    {
        std::ofstream f(root / "src" / "math.cppm");
        f << R"(
export module demo.math;

export int add(int lhs, int rhs) {
    return lhs + rhs;
}
)";
    }

    {
        std::ofstream f(root / "src" / "math.detail.cppm");
        f << R"(
export module demo.math:detail;

export int detail() {
    return 7;
}
)";
    }

    clore::testing::write_compile_commands(root / "compile_commands.json",
                                           {
                                               {
                                                .directory = root / "src",
                                                .file = root / "src" / "math.cppm",
                                                .arguments =
                                                       {
                                                           "clang++",
                                                           "-std=c++23",
                                                           "-c",
                                                           "math.cppm",
                                                           "-o",
                                                           "math.pcm",
                                                       }, },
                                               {
                                                .directory = root / "src",
                                                .file = root / "src" / "math.detail.cppm",
                                                .arguments =
                                                       {
                                                           "clang++",
                                                           "-std=c++23",
                                                           "-c",
                                                           "math.detail.cppm",
                                                           "-o",
                                                           "math.detail.pcm",
                                                       }, }
    });

    config::TaskConfig cfg;
    cfg.compile_commands_path = (root / "compile_commands.json").string();
    cfg.project_root = root.string();
    cfg.output_root = (root / "out").string();
    cfg.workspace_root = root.string();

    auto result = extract::extract_project(cfg);
    ASSERT_TRUE(result.has_value());

    EXPECT_TRUE(result->uses_modules);
    ASSERT_EQ(result->modules.size(), 2u);

    auto math_path = fs::weakly_canonical(root / "src" / "math.cppm").generic_string();
    auto detail_path = fs::weakly_canonical(root / "src" / "math.detail.cppm").generic_string();
    ASSERT_TRUE(result->modules.contains(math_path));
    ASSERT_TRUE(result->modules.contains(detail_path));
    EXPECT_EQ(result->modules.at(math_path).name, "demo.math");
    EXPECT_EQ(result->modules.at(detail_path).name, "demo.math:detail");
    EXPECT_TRUE(result->modules.at(math_path).is_interface);
    EXPECT_TRUE(result->modules.at(detail_path).is_interface);

    fs::remove_all(root);
}

TEST_CASE(resolves_relative_line_directives_against_workspace_root) {
    namespace fs = std::filesystem;

    auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    auto root =
        fs::temp_directory_path() / std::format("clore_relative_line_directive_test_{}", ticks);
    fs::create_directories(root / "src");
    fs::create_directories(root / "build");

    auto source_file = root / "src" / "math.cpp";
    {
        std::ofstream f(source_file);
        f << R"(
#line 42 "src/math.cpp"
int add(int lhs, int rhs) {
    return lhs + rhs;
}
)";
    }

    clore::testing::write_compile_commands(root / "compile_commands.json",
                                           {
                                               {
                                                .directory = root / "build",
                                                .file = source_file,
                                                .arguments =
                                                       {
                                                           "clang++",
                                                           "-std=c++23",
                                                           "-c",
                                                           source_file.string(),
                                                           "-o",
                                                           "math.o",
                                                       }, }
    });

    config::TaskConfig cfg;
    cfg.compile_commands_path = (root / "compile_commands.json").string();
    cfg.project_root = root.string();
    cfg.output_root = (root / "out").string();
    cfg.workspace_root = root.string();
    cfg.filter.include = {"src"};

    auto result = extract::extract_project(cfg);
    ASSERT_TRUE(result.has_value());

    auto add_it = std::ranges::find_if(result->symbols,
                                       [](const auto& item) { return item.second.name == "add"; });
    ASSERT_TRUE(add_it != result->symbols.end());
    auto expected_source = extract::canonical_graph_path(source_file);
    EXPECT_EQ(add_it->second.declaration_location.file, expected_source);
    EXPECT_TRUE(result->files.contains(expected_source));

    fs::remove_all(root);
}

TEST_CASE(merges_cached_module_imports_for_duplicate_compile_entries) {
    namespace fs = std::filesystem;

    auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    auto root = fs::temp_directory_path() / std::format("clore_module_import_merge_test_{}", ticks);
    fs::create_directories(root / "src");

    auto source_file = root / "src" / "service.cppm";
    {
        std::ofstream f(source_file);
        f << R"(
export module demo.service;

export int run() {
    return 0;
}
)";
    }

    clore::testing::write_compile_commands(root / "compile_commands.json",
                                           {
                                               {
                                                .directory = root / "src",
                                                .file = source_file,
                                                .arguments =
                                                       {
                                                           "clang++",
                                                           "-std=c++23",
                                                           "-DPROFILE_CORE=1",
                                                           "-c",
                                                           "service.cppm",
                                                           "-o",
                                                           "service.core.pcm",
                                                       }, },
                                               {
                                                .directory = root / "src",
                                                .file = source_file,
                                                .arguments =
                                                       {
                                                           "clang++",
                                                           "-std=c++23",
                                                           "-DPROFILE_UTIL=1",
                                                           "-c",
                                                           "service.cppm",
                                                           "-o",
                                                           "service.util.pcm",
                                                       }, }
    });

    auto source_hash = extract::cache::hash_file(source_file.generic_string());
    ASSERT_TRUE(source_hash.has_value());

    auto source = fs::weakly_canonical(source_file).generic_string();

    extract::CompileEntry core_entry{
        .file = source,
        .directory = (root / "src").generic_string(),
        .arguments =
            {
                        "clang++", "-std=c++23",
                        "-DPROFILE_CORE=1", "-c",
                        "service.cppm", "-o",
                        "service.core.pcm", },
    };
    auto core_signature = extract::cache::build_compile_signature(core_entry);

    extract::CompileEntry util_entry{
        .file = source,
        .directory = (root / "src").generic_string(),
        .arguments =
            {
                        "clang++", "-std=c++23",
                        "-DPROFILE_UTIL=1", "-c",
                        "service.cppm", "-o",
                        "service.util.pcm", },
    };
    auto util_signature = extract::cache::build_compile_signature(util_entry);

    std::unordered_map<std::string, extract::cache::CacheRecord> cache_records;
    cache_records.emplace(extract::cache::build_cache_key(source, core_signature),
                          extract::cache::CacheRecord{
                              .compile_signature = core_signature,
                              .source_hash = *source_hash,
                              .ast_deps = {},
                              .scan =
                                  extract::ScanResult{
                                           .module_name = "demo.service",
                                           .is_interface_unit = true,
                                           .module_imports = {"demo.core"},
                                           },
                              .ast = {},
    });
    cache_records.emplace(extract::cache::build_cache_key(source, util_signature),
                          extract::cache::CacheRecord{
                              .compile_signature = util_signature,
                              .source_hash = *source_hash,
                              .ast_deps = {},
                              .scan =
                                  extract::ScanResult{
                                           .module_name = "demo.service",
                                           .is_interface_unit = true,
                                           .module_imports = {"demo.util"},
                                           },
                              .ast = {},
    });

    auto save_cache = extract::cache::save_extract_cache(root.string(), cache_records);
    ASSERT_TRUE(save_cache.has_value());

    config::TaskConfig cfg;
    cfg.compile_commands_path = (root / "compile_commands.json").string();
    cfg.project_root = root.string();
    cfg.output_root = (root / "out").string();
    cfg.workspace_root = root.string();

    auto result = extract::extract_project(cfg);
    ASSERT_TRUE(result.has_value());

    ASSERT_TRUE(result->modules.contains(source));
    const auto& imports = result->modules.at(source).imports;
    ASSERT_EQ(imports.size(), 2u);
    EXPECT_TRUE(std::ranges::find(imports, "demo.core") != imports.end());
    EXPECT_TRUE(std::ranges::find(imports, "demo.util") != imports.end());

    fs::remove_all(root);
}

TEST_CASE(rejects_conflicting_cached_module_metadata_for_same_source_file) {
    namespace fs = std::filesystem;

    auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    auto root = fs::temp_directory_path() / std::format("clore_module_conflict_test_{}", ticks);
    fs::create_directories(root / "src");

    auto source_file = root / "src" / "service.cppm";
    {
        std::ofstream f(source_file);
        f << R"(
export module demo.alpha;

export int run() {
    return 0;
}
)";
    }

    clore::testing::write_compile_commands(root / "compile_commands.json",
                                           {
                                               {
                                                .directory = root / "src",
                                                .file = source_file,
                                                .arguments =
                                                       {
                                                           "clang++",
                                                           "-std=c++23",
                                                           "-DPROFILE_ALPHA=1",
                                                           "-c",
                                                           "service.cppm",
                                                           "-o",
                                                           "service.alpha.pcm",
                                                       }, },
                                               {
                                                .directory = root / "src",
                                                .file = source_file,
                                                .arguments =
                                                       {
                                                           "clang++",
                                                           "-std=c++23",
                                                           "-DPROFILE_BETA=1",
                                                           "-c",
                                                           "service.cppm",
                                                           "-o",
                                                           "service.beta.pcm",
                                                       }, }
    });

    auto source_hash = extract::cache::hash_file(source_file.generic_string());
    ASSERT_TRUE(source_hash.has_value());

    auto source = fs::weakly_canonical(source_file).generic_string();

    extract::CompileEntry alpha_entry{
        .file = source,
        .directory = (root / "src").generic_string(),
        .arguments =
            {
                        "clang++", "-std=c++23",
                        "-DPROFILE_ALPHA=1", "-c",
                        "service.cppm", "-o",
                        "service.alpha.pcm", },
    };
    auto alpha_signature = extract::cache::build_compile_signature(alpha_entry);

    extract::CompileEntry beta_entry{
        .file = source,
        .directory = (root / "src").generic_string(),
        .arguments =
            {
                        "clang++", "-std=c++23",
                        "-DPROFILE_BETA=1", "-c",
                        "service.cppm", "-o",
                        "service.beta.pcm", },
    };
    auto beta_signature = extract::cache::build_compile_signature(beta_entry);

    std::unordered_map<std::string, extract::cache::CacheRecord> cache_records;
    cache_records.emplace(extract::cache::build_cache_key(source, alpha_signature),
                          extract::cache::CacheRecord{
                              .compile_signature = alpha_signature,
                              .source_hash = *source_hash,
                              .ast_deps = {                          },
                              .scan =
                                  extract::ScanResult{
                                           .module_name = "demo.alpha",
                                           .is_interface_unit = true,
                                           },
                              .ast = {                          },
    });
    cache_records.emplace(extract::cache::build_cache_key(source, beta_signature),
                          extract::cache::CacheRecord{
                              .compile_signature = beta_signature,
                              .source_hash = *source_hash,
                              .ast_deps = {                         },
                              .scan =
                                  extract::ScanResult{
                                           .module_name = "demo.beta",
                                           .is_interface_unit = true,
                                           },
                              .ast = {                         },
    });

    auto save_cache = extract::cache::save_extract_cache(root.string(), cache_records);
    ASSERT_TRUE(save_cache.has_value());

    config::TaskConfig cfg;
    cfg.compile_commands_path = (root / "compile_commands.json").string();
    cfg.project_root = root.string();
    cfg.output_root = (root / "out").string();
    cfg.workspace_root = root.string();

    auto result = extract::extract_project(cfg);
    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("conflicting module names for source file"),
              std::string::npos);

    fs::remove_all(root);
}

TEST_CASE(populates_parent_only_namespace_entries_for_nested_namespaces) {
    namespace fs = std::filesystem;

    auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    auto root =
        fs::temp_directory_path() / std::format("clore_nested_namespace_extract_test_{}", ticks);
    fs::create_directories(root / "src");

    {
        std::ofstream f(root / "src" / "config.cpp");
        f << R"(
namespace demo::config {

struct Options {};

}  // namespace demo::config
)";
    }

    clore::testing::write_compile_commands(root / "compile_commands.json",
                                           {
                                               {
                                                .directory = root / "src",
                                                .file = root / "src" / "config.cpp",
                                                .arguments =
                                                       {
                                                           "clang++",
                                                           "-std=c++23",
                                                           "-c",
                                                           "config.cpp",
                                                           "-o",
                                                           "config.o",
                                                       }, }
    });

    config::TaskConfig cfg;
    cfg.compile_commands_path = (root / "compile_commands.json").string();
    cfg.project_root = root.string();
    cfg.output_root = (root / "out").string();
    cfg.workspace_root = root.string();

    auto result = extract::extract_project(cfg);
    ASSERT_TRUE(result.has_value());

    ASSERT_TRUE(result->namespaces.contains("demo"));
    ASSERT_TRUE(result->namespaces.contains("demo::config"));
    EXPECT_TRUE(result->namespaces.at("demo").symbols.empty());
    EXPECT_EQ(result->namespaces.at("demo").children.size(), 1u);
    EXPECT_EQ(result->namespaces.at("demo").children[0], "demo::config");

    fs::remove_all(root);
}

};  // TEST_SUITE(extract_integration)

TEST_SUITE(extract_filter_security) {

// HIGH: matches_filter must reject paths that escape the project root via
// "..".
// If fs::relative produces "..", a file outside the root must not be
// processed.
TEST_CASE(rejects_path_outside_project_root) {
    // We build a compile_commands.json whose "file" field points outside
    // the
    // project_root.  extract_project should skip (filter rejects) that
    // file.
    namespace fs = std::filesystem;

    auto root = fs::temp_directory_path() / "clore_filter_test_root";
    auto outside = fs::temp_directory_path() / "clore_filter_test_outside";
    fs::create_directories(root);
    fs::create_directories(outside);

    // Source file outside root
    {
        std::ofstream f(outside / "secret.cpp");
        f << "int secret() { return 42; }\n";
    }

    // compile_commands.json inside root, pointing at file outside root
    clore::testing::write_compile_commands(root / "compile_commands.json",
                                           {
                                               {
                                                .directory = outside,
                                                .file = outside / "secret.cpp",
                                                .arguments =
                                                       {
                                                           "clang++",
                                                           "-c",
                                                           "secret.cpp",
                                                       }, }
    });

    config::TaskConfig cfg;
    cfg.compile_commands_path = (root / "compile_commands.json").string();
    cfg.project_root = root.string();
    cfg.output_root = (root / "out").string();
    cfg.workspace_root = root.string();

    auto result = extract::extract_project(cfg);
    // The outside file is filtered; since no files remain in the project, the
    // model should be empty (no error — filtering is not an error).
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->files.size(), 0u);

    fs::remove_all(root);
    fs::remove_all(outside);
}

};  // TEST_SUITE(extract_filter_security)

TEST_SUITE(extract_filter_semantics) {

// "src/" should be interpreted relative to the workspace root even when
// the source root itself is nested under that workspace.
TEST_CASE(include_src_does_not_match_deps_src) {
    namespace fs = std::filesystem;

    auto root = fs::temp_directory_path() / "clore_filter_prefix_test";
    auto project_src = root / "src";
    auto deps_src = root / "build" / "Debug" / "_deps" / "dep" / "src";
    fs::create_directories(project_src);
    fs::create_directories(deps_src);

    // Project file under src/
    {
        std::ofstream f(project_src / "keep.cpp");
        f << "int keep() { return 1; }\n";
    }

    // Dependency-like file under build/.../_deps/.../src/
    {
        std::ofstream f(deps_src / "drop.cpp");
        f << "int drop() { return 2; }\n";
    }

    clore::testing::write_compile_commands(root / "compile_commands.json",
                                           {
                                               {
                                                .directory = project_src,
                                                .file = project_src / "keep.cpp",
                                                .arguments =
                                                       {
                                                           "clang++",
                                                           "-std=c++23",
                                                           "-c",
                                                           "keep.cpp",
                                                           "-o",
                                                           "keep.o",
                                                       }, },
                                               {
                                                .directory = deps_src,
                                                .file = deps_src / "drop.cpp",
                                                .arguments =
                                                       {
                                                           "clang++",
                                                           "-std=c++23",
                                                           "-c",
                                                           "drop.cpp",
                                                           "-o",
                                                           "drop.o",
                                                       }, }
    });

    config::TaskConfig cfg;
    cfg.compile_commands_path = (root / "compile_commands.json").string();
    cfg.project_root = project_src.string();
    cfg.output_root = (root / "out").string();
    cfg.workspace_root = root.string();
    cfg.filter.include = {"src"};

    auto result = extract::extract_project(cfg);
    ASSERT_TRUE(result.has_value());

    // The dep file should be excluded.
    for(auto& [path, _]: result->files) {
        EXPECT_EQ(path.find("_deps"), std::string::npos);
    }

    bool found_keep = false;
    bool found_drop = false;
    for(auto& [_, sym]: result->symbols) {
        if(sym.name == "keep")
            found_keep = true;
        if(sym.name == "drop")
            found_drop = true;
    }
    EXPECT_TRUE(found_keep);
    EXPECT_FALSE(found_drop);

    fs::remove_all(root);
}

};  // TEST_SUITE(extract_filter_semantics)

TEST_SUITE(extract_symbol_ownership) {

TEST_CASE(shared_header_symbols_are_not_duplicated_in_including_sources) {
    namespace fs = std::filesystem;

    auto root = fs::temp_directory_path() / "clore_symbol_ownership_test";
    fs::remove_all(root);
    fs::create_directories(root / "src");

    {
        std::ofstream f(root / "src" / "shared.h");
        f << R"(
#pragma once

namespace demo::config {

struct Shared {
    int value;
};

}  // namespace demo::config
)";
    }

    {
        std::ofstream f(root / "src" / "load.cpp");
        f << R"(
#include "shared.h"

int load_value() {
    demo::config::Shared shared{42};
    return shared.value;
}
)";
    }

    {
        std::ofstream f(root / "src" / "other.cpp");
        f << R"(
#include "shared.h"

int other_value() {
    demo::config::Shared shared{7};
    return shared.value;
}
)";
    }

    clore::testing::write_compile_commands(
        root / "compile_commands.json",
        {
            {
             .directory = root / "src",
             .file = root / "src" / "load.cpp",
             .arguments =
                    {
                        "clang++",
                        "-std=c++23",
                        std::format("-I{}", (root / "src").string()),
                        "-c",
                        "load.cpp",
                        "-o",
                        "load.o",
                    }, },
            {
             .directory = root / "src",
             .file = root / "src" / "other.cpp",
             .arguments =
                    {
                        "clang++",
                        "-std=c++23",
                        std::format("-I{}", (root / "src").string()),
                        "-c",
                        "other.cpp",
                        "-o",
                        "other.o",
                    }, }
    });

    config::TaskConfig cfg;
    cfg.compile_commands_path = (root / "compile_commands.json").string();
    cfg.project_root = root.string();
    cfg.output_root = (root / "out").string();
    cfg.workspace_root = root.string();

    auto model_result = extract::extract_project(cfg);
    ASSERT_TRUE(model_result.has_value());

    auto& model = *model_result;
    auto shared_header = extract::canonical_graph_path(root / "src" / "shared.h");
    auto load_cpp = extract::canonical_graph_path(root / "src" / "load.cpp");

    ASSERT_TRUE(model.files.contains(shared_header));
    ASSERT_TRUE(model.files.contains(load_cpp));

    auto shared_it = std::ranges::find_if(model.symbols, [](const auto& item) {
        return item.second.qualified_name == "demo::config::Shared";
    });
    ASSERT_TRUE(shared_it != model.symbols.end());
    auto namespace_it = std::ranges::find_if(model.symbols, [](const auto& item) {
        return item.second.kind == extract::SymbolKind::Namespace &&
               item.second.qualified_name == "demo::config";
    });
    ASSERT_TRUE(namespace_it != model.symbols.end());

    auto shared_id = shared_it->first;
    auto namespace_id = namespace_it->first;

    EXPECT_EQ(std::count(model.files.at(shared_header).symbols.begin(),
                         model.files.at(shared_header).symbols.end(),
                         shared_id),
              1);
    EXPECT_EQ(std::count(model.files.at(shared_header).symbols.begin(),
                         model.files.at(shared_header).symbols.end(),
                         namespace_id),
              1);
    EXPECT_EQ(std::count(model.files.at(load_cpp).symbols.begin(),
                         model.files.at(load_cpp).symbols.end(),
                         shared_id),
              0);
    EXPECT_EQ(std::count(model.symbols.at(namespace_id).children.begin(),
                         model.symbols.at(namespace_id).children.end(),
                         shared_id),
              1);

    auto parent_namespace_it = std::ranges::find_if(model.symbols, [](const auto& item) {
        return item.second.kind == extract::SymbolKind::Namespace &&
               item.second.qualified_name == "demo";
    });
    ASSERT_TRUE(parent_namespace_it != model.symbols.end());
    EXPECT_EQ(std::count(parent_namespace_it->second.children.begin(),
                         parent_namespace_it->second.children.end(),
                         namespace_id),
              1);

    fs::remove_all(root);
}

};  // TEST_SUITE(extract_symbol_ownership)

TEST_SUITE(scan_error_propagation) {

// HIGH: scan_file must return an error for an empty argument list.
TEST_CASE(empty_args_returns_error) {
    extract::CompileEntry entry;
    entry.file = "test.cpp";
    entry.directory = "/tmp";
    entry.arguments = {};  // empty

    auto result = extract::scan_file(entry);
    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("empty"), std::string::npos);
}

};  // TEST_SUITE(scan_error_propagation)
