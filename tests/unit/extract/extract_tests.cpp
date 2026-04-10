#include "eventide/zest/zest.h"

#include <filesystem>
#include <format>
#include <fstream>

#include "config/config.h"
#include "extract/compdb.h"
#include "extract/extract.h"
#include "extract/scan.h"
#include "extract/compdb_test_utils.h"

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
        clore::testing::write_compile_commands(
            temp_dir / "compile_commands.json",
            {{
                .directory = src_dir,
                .file = src_dir / "math.cpp",
                .arguments = {
                    "clang++",
                    "-std=c++23",
                    std::format("-I{}", src_dir.string()),
                    "-c",
                    "math.cpp",
                    "-o",
                    "math.o",
                },
            },
             {
                 .directory = src_dir,
                 .file = src_dir / "widget.cpp",
                 .arguments = {
                     "clang++",
                     "-std=c++23",
                     std::format("-I{}", src_dir.string()),
                     "-c",
                     "widget.cpp",
                     "-o",
                     "widget.o",
                 },
             }});

        config::TaskConfig task_config;
        task_config.compile_commands_path = (temp_dir / "compile_commands.json").string();
        task_config.project_root = temp_dir.string();
        task_config.output_root = (temp_dir / "docs").string();
        task_config.extract.max_snippet_bytes = 2048;

        auto result = extract::extract_project(task_config);
        ASSERT_TRUE(result.has_value());

        auto& model = *result;
        EXPECT_GT(model.symbols.size(), 0u);
        EXPECT_GT(model.files.size(), 0u);

        // Verify we found the expected symbols
        bool found_add = false;
        bool found_multiply = false;
        bool found_widget = false;

        for(auto& [id, sym] : model.symbols) {
            if(sym.name == "add") found_add = true;
            if(sym.name == "multiply") found_multiply = true;
            if(sym.name == "Widget") found_widget = true;
        }

        EXPECT_TRUE(found_add);
        EXPECT_TRUE(found_multiply);
        EXPECT_TRUE(found_widget);

        fs::remove_all(temp_dir);
    }
};

TEST_SUITE(extract_filter_security) {
    // HIGH: matches_filter must reject paths that escape the project root via "..".
    // If fs::relative produces "..", a file outside the root must not be processed.
    TEST_CASE(rejects_path_outside_project_root) {
        // We build a compile_commands.json whose "file" field points outside the
        // project_root.  extract_project should skip (filter rejects) that file.
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
                                               {{
                                                   .directory = outside,
                                                   .file = outside / "secret.cpp",
                                                   .arguments = {
                                                       "clang++",
                                                       "-c",
                                                       "secret.cpp",
                                                   },
                                               }});

        config::TaskConfig cfg;
        cfg.compile_commands_path = (root / "compile_commands.json").string();
        cfg.project_root = root.string();
        cfg.output_root = (root / "out").string();
        cfg.extract.max_snippet_bytes = 512;

        auto result = extract::extract_project(cfg);
        // The outside file is filtered; since no files remain in the project, the
        // model should be empty (no error — filtering is not an error).
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result->files.size(), 0u);

        fs::remove_all(root);
        fs::remove_all(outside);
    }
};

TEST_SUITE(extract_filter_semantics) {
    // "src/" should match only project-root-relative "src/..." paths, not any
    // random ".../src/..." segment in dependency trees such as build/_deps.
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

        clore::testing::write_compile_commands(
            root / "compile_commands.json",
            {{
                 .directory = project_src,
                 .file = project_src / "keep.cpp",
                 .arguments = {
                     "clang++",
                     "-std=c++23",
                     "-c",
                     "keep.cpp",
                     "-o",
                     "keep.o",
                 },
             },
             {
                 .directory = deps_src,
                 .file = deps_src / "drop.cpp",
                 .arguments = {
                     "clang++",
                     "-std=c++23",
                     "-c",
                     "drop.cpp",
                     "-o",
                     "drop.o",
                 },
             }});

        config::TaskConfig cfg;
        cfg.compile_commands_path = (root / "compile_commands.json").string();
        cfg.project_root = root.string();
        cfg.output_root = (root / "out").string();
        cfg.extract.max_snippet_bytes = 512;
        cfg.filter.include = {"src/"};

        auto result = extract::extract_project(cfg);
        ASSERT_TRUE(result.has_value());

        // The dep file should be excluded.
        for(auto& [path, _] : result->files) {
            EXPECT_EQ(path.find("_deps"), std::string::npos);
        }

        bool found_keep = false;
        bool found_drop = false;
        for(auto& [_, sym] : result->symbols) {
            if(sym.name == "keep") found_keep = true;
            if(sym.name == "drop") found_drop = true;
        }
        EXPECT_TRUE(found_keep);
        EXPECT_FALSE(found_drop);

        fs::remove_all(root);
    }
};

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
};
