#include <filesystem>
#include <fstream>

#include "kota/zest/zest.h"

import extract;

using namespace clore::extract;
using namespace clore::extract::cache;

namespace fs = std::filesystem;

TEST_SUITE(extract_cache) {

TEST_CASE(build_cache_key_and_split_are_inverses) {
    auto key = build_cache_key("src/math.cpp", 123456789);
    auto parts = split_cache_key(key);

    ASSERT_TRUE(parts.has_value());
    EXPECT_EQ(parts->path, "src/math.cpp");
    EXPECT_EQ(parts->compile_signature, 123456789u);
}

TEST_CASE(split_cache_key_rejects_invalid_formats) {
    auto no_delim = split_cache_key("no-delimiter");
    EXPECT_FALSE(no_delim.has_value());

    auto empty_sig = split_cache_key("path\t");
    EXPECT_FALSE(empty_sig.has_value());

    auto empty_path = split_cache_key("\t123");
    EXPECT_FALSE(empty_path.has_value());

    auto bad_sig = split_cache_key("path\tabc");
    EXPECT_FALSE(bad_sig.has_value());
}

TEST_CASE(hash_file_consistent_for_known_content) {
    auto temp_path = fs::temp_directory_path() / "clore_test_hash_file.txt";
    {
        std::ofstream f(temp_path);
        f << "hello world";
    }

    auto result = hash_file(temp_path.generic_string());
    ASSERT_TRUE(result.has_value());
    EXPECT_NE(*result, 0u);

    auto result2 = hash_file(temp_path.generic_string());
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(*result, *result2);

    std::error_code ec;
    fs::remove(temp_path, ec);
}

TEST_CASE(hash_file_fails_for_missing_file) {
    auto result = hash_file("/nonexistent/path/for/sure.txt");
    EXPECT_FALSE(result.has_value());
}

TEST_CASE(capture_dependency_snapshot_empty_is_ok) {
    auto snapshot = capture_dependency_snapshot({});
    ASSERT_TRUE(snapshot.has_value());
    EXPECT_TRUE(snapshot->files.empty());
    EXPECT_GT(snapshot->build_at, 0);
}

TEST_CASE(dependencies_changed_empty_snapshot) {
    DependencySnapshot snapshot;
    EXPECT_TRUE(dependencies_changed(snapshot));
}

TEST_CASE(dependencies_changed_mismatched_sizes) {
    DependencySnapshot snapshot;
    snapshot.build_at = 1;
    snapshot.files = {"a.cpp"};
    snapshot.hashes = {1, 2};
    snapshot.mtimes = {1};
    EXPECT_TRUE(dependencies_changed(snapshot));
}

TEST_CASE(dependencies_changed_rechecks_hash_when_mtime_is_restored) {
    auto temp_path = fs::temp_directory_path() /
                     std::format("clore_test_dependency_snapshot_{}",
                                 std::chrono::steady_clock::now().time_since_epoch().count());
    {
        std::ofstream f(temp_path, std::ios::binary);
        f << "alpha";
    }

    auto snapshot = capture_dependency_snapshot({temp_path.generic_string()});
    ASSERT_TRUE(snapshot.has_value());

    std::error_code time_error;
    auto original_time = fs::last_write_time(temp_path, time_error);
    ASSERT_FALSE(static_cast<bool>(time_error));

    {
        std::ofstream f(temp_path, std::ios::binary | std::ios::trunc);
        f << "bravo";
    }
    fs::last_write_time(temp_path, original_time, time_error);
    ASSERT_FALSE(static_cast<bool>(time_error));

    EXPECT_TRUE(dependencies_changed(*snapshot));

    std::error_code ec;
    fs::remove(temp_path, ec);
}

TEST_CASE(save_and_load_extract_cache_roundtrip) {
    auto temp_dir = fs::temp_directory_path() /
                    std::format("clore_extract_cache_{}",
                                std::chrono::steady_clock::now().time_since_epoch().count());
    fs::create_directories(temp_dir);

    std::unordered_map<std::string, CacheRecord> records;
    auto key = build_cache_key("src/demo.cpp", 42);
    records[key] = CacheRecord{
        .compile_signature = 42,
        .source_hash = 100,
        .ast_deps =
            DependencySnapshot{
                               .files = {"src/demo.cpp"},
                               .hashes = {100},
                               .mtimes = {1000},
                               .build_at = 2000,
                               },
        .scan = ScanResult{},
        .ast = ASTResult{},
    };

    auto save_result = save_extract_cache(temp_dir.generic_string(), records);
    ASSERT_TRUE(save_result.has_value());

    auto load_result = load_extract_cache(temp_dir.generic_string());
    ASSERT_TRUE(load_result.has_value());
    ASSERT_EQ(load_result->size(), 1u);

    auto& loaded = (*load_result)[key];
    EXPECT_EQ(loaded.compile_signature, 42u);
    EXPECT_EQ(loaded.source_hash, 100u);
    ASSERT_EQ(loaded.ast_deps.files.size(), 1u);
    EXPECT_EQ(loaded.ast_deps.files[0], "src/demo.cpp");

    std::error_code ec;
    fs::remove_all(temp_dir, ec);
}

TEST_CASE(save_and_load_clice_cache_roundtrip) {
    auto temp_dir = fs::temp_directory_path() /
                    std::format("clore_clice_cache_{}",
                                std::chrono::steady_clock::now().time_since_epoch().count());
    fs::create_directories(temp_dir);

    CliceCacheData data{
        .paths = {"src/demo.cpp", "build/demo.pch", "build/demo.pcm"},
        .pch = {CliceCachePCHEntry{
            .filename = "demo.pch",
            .source_file = 0,
            .hash = 11,
            .bound = 22,
            .build_at = 33,
            .deps = {CliceCacheDepEntry{.path = 0, .hash = 44}},
        }},
        .pcm = {CliceCachePCMEntry{
            .filename = "demo.pcm",
            .source_file = 0,
            .module_name = "demo.core",
            .build_at = 55,
            .deps = {CliceCacheDepEntry{.path = 0, .hash = 66}},
        }},
    };

    auto save_result = save_clice_cache(temp_dir.generic_string(), data);
    ASSERT_TRUE(save_result.has_value());

    auto load_result = load_clice_cache(temp_dir.generic_string());
    ASSERT_TRUE(load_result.has_value());
    EXPECT_EQ(load_result->paths, data.paths);
    ASSERT_EQ(load_result->pch.size(), 1u);
    EXPECT_EQ(load_result->pch[0].filename, "demo.pch");
    EXPECT_EQ(load_result->pch[0].deps[0].hash, 44u);
    ASSERT_EQ(load_result->pcm.size(), 1u);
    EXPECT_EQ(load_result->pcm[0].module_name, "demo.core");
    EXPECT_EQ(load_result->pcm[0].deps[0].hash, 66u);

    std::error_code ec;
    fs::remove_all(temp_dir, ec);
}

TEST_CASE(load_clice_cache_treats_malformed_json_as_stale) {
    auto temp_dir = fs::temp_directory_path() /
                    std::format("clore_clice_cache_stale_{}",
                                std::chrono::steady_clock::now().time_since_epoch().count());
    fs::create_directories(temp_dir / ".clice" / "cache");

    {
        std::ofstream f(temp_dir / ".clice" / "cache" / "cache.json", std::ios::binary);
        f << "{ malformed json";
    }

    auto load_result = load_clice_cache(temp_dir.generic_string());
    ASSERT_TRUE(load_result.has_value());
    EXPECT_TRUE(load_result->paths.empty());
    EXPECT_TRUE(load_result->pch.empty());
    EXPECT_TRUE(load_result->pcm.empty());

    std::error_code ec;
    fs::remove_all(temp_dir, ec);
}

TEST_CASE(load_extract_cache_rejects_missing_workspace) {
    auto result = load_extract_cache("");
    EXPECT_FALSE(result.has_value());
}

};  // TEST_SUITE(extract_cache)
