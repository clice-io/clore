#include <filesystem>
#include <fstream>

#include "extract/compdb.h"
#include "kota/zest/zest.h"

import extract;

using namespace clore::extract;

TEST_SUITE(compiler) {

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
                                           {
                                               {
                                                .directory = temp_dir,
                                                .file = temp_dir / "test.cpp",
                                                .arguments =
                                                       {
                                                           "clang++",
                                                           "-std=c++23",
                                                           "-c",
                                                           "test.cpp",
                                                           "-o",
                                                           "test.o",
                                                       }, }
    });

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
        .cache_key = {},
    });
    db.entries.push_back(CompileEntry{
        .file = "/project/src/util.cpp",
        .directory = "/project",
        .arguments = {"clang++", "-c", "src/util.cpp"},
        .cache_key = {},
    });

    auto results = lookup(db, "/project/src/main.cpp");
    EXPECT_EQ(results.size(), 1u);

    auto no_results = lookup(db, "/project/src/nonexistent.cpp");
    EXPECT_EQ(no_results.size(), 0u);
}

TEST_CASE(lookup_resolves_relative_file_against_entry_directory) {
    namespace fs = std::filesystem;

    auto project_dir =
        (fs::temp_directory_path() / "clore_test_compdb_relative").lexically_normal();
    auto source_path = (project_dir / "src" / "main.cpp").lexically_normal();

    CompilationDatabase db;
    db.entries.push_back(CompileEntry{
        .file = "src/main.cpp",
        .directory = project_dir.generic_string(),
        .arguments = {"clang++", "-c", "src/main.cpp"},
        .cache_key = {},
    });

    auto absolute_results = lookup(db, source_path.generic_string());
    ASSERT_EQ(absolute_results.size(), 1u);
    EXPECT_EQ(absolute_results.front()->file, "src/main.cpp");
    EXPECT_EQ(absolute_results.front()->directory, project_dir.generic_string());

    auto relative_results = lookup(db, "src/main.cpp");
    ASSERT_EQ(relative_results.size(), 1u);
    EXPECT_EQ(relative_results.front()->file, "src/main.cpp");
    EXPECT_EQ(relative_results.front()->directory, project_dir.generic_string());
}

TEST_CASE(toolchain_cache_records_sanitized_arguments) {
    CompilationDatabase db;
    CompileEntry entry{
        .file = "/project/src/main.cpp",
        .directory = "/project",
        .arguments = {"clang++", "-std=c++23", "-c", "src/main.cpp", "-o", "main.o"},
        .cache_key = {},
    };
    ensure_cache_key(entry);

    EXPECT_FALSE(db.has_cached_toolchain());
    auto first = query_toolchain_cached(db, entry);
    EXPECT_TRUE(db.has_cached_toolchain());
    EXPECT_FALSE(first.empty());
    EXPECT_EQ(first.front(), "-std=c++23");
}

TEST_CASE(toolchain_cache_hit_returns_same_result) {
    CompilationDatabase db;
    CompileEntry entry{
        .file = "/project/src/main.cpp",
        .directory = "/project",
        .arguments = {"clang++", "-std=c++23", "-c", "src/main.cpp", "-o", "main.o"},
        .cache_key = {},
    };
    ensure_cache_key(entry);

    auto first = query_toolchain_cached(db, entry);
    auto second = query_toolchain_cached(db, entry);
    EXPECT_EQ(first, second);
    EXPECT_EQ(db.toolchain_cache.size(), 1u);
}

};  // TEST_SUITE(compiler)
