#include "eventide/zest/zest.h"

#include <filesystem>
#include <fstream>

#include "extract/compdb_test_utils.h"

import clore.extract;

using namespace clore::extract;

TEST_SUITE(compdb) {
    TEST_CASE(load_nonexistent) {
        auto result = load_compdb("/nonexistent/compile_commands.json");
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(load_valid_compdb) {
        namespace fs = std::filesystem;

        auto temp_dir = fs::temp_directory_path() / "clore_test_compdb";
        fs::create_directories(temp_dir);
        auto compdb_path = temp_dir / "compile_commands.json";

        clore::testing::write_compile_commands(compdb_path,
                                               {{
                                                   .directory = temp_dir,
                                                   .file = temp_dir / "test.cpp",
                                                   .arguments = {
                                                       "clang++",
                                                       "-std=c++23",
                                                       "-c",
                                                       "test.cpp",
                                                       "-o",
                                                       "test.o",
                                                   },
                                               }});

        // Also create the source file so clang can find it
        {
            std::ofstream f(temp_dir / "test.cpp");
            f << "int main() { return 0; }\n";
        }

        auto result = load_compdb(compdb_path.string());
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result->entries.size(), 1u);
        EXPECT_EQ(result->entries[0].directory, temp_dir.string());

        fs::remove_all(temp_dir);
    }

    TEST_CASE(load_invalid_json) {
        namespace fs = std::filesystem;

        auto temp_dir = fs::temp_directory_path() / "clore_test_compdb_invalid";
        fs::create_directories(temp_dir);
        auto compdb_path = temp_dir / "compile_commands.json";

        {
            std::ofstream f(compdb_path);
            f << "this is not valid json";
        }

        auto result = load_compdb(compdb_path.string());
        EXPECT_FALSE(result.has_value());

        fs::remove_all(temp_dir);
    }

    TEST_CASE(lookup_by_file) {
        CompilationDatabase db;
        db.entries.push_back(CompileEntry{
            .file = "/project/src/main.cpp",
            .directory = "/project",
            .arguments = {"clang++", "-c", "src/main.cpp"},
        });
        db.entries.push_back(CompileEntry{
            .file = "/project/src/util.cpp",
            .directory = "/project",
            .arguments = {"clang++", "-c", "src/util.cpp"},
        });

        auto results = lookup(db, "/project/src/main.cpp");
        EXPECT_EQ(results.size(), 1u);

        auto no_results = lookup(db, "/project/src/nonexistent.cpp");
        EXPECT_EQ(no_results.size(), 0u);
    }
};
