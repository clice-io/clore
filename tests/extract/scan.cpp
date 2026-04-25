#include <filesystem>
#include <fstream>
#include <string>

#include "kota/async/async.h"
#include "kota/zest/zest.h"

import extract;

using namespace clore::extract;

namespace {

auto canonical_test_path(const std::filesystem::path& path) -> std::string {
    namespace fs = std::filesystem;

    std::error_code ec;
    auto absolute = fs::absolute(path, ec);
    if(ec) {
        return path.lexically_normal().generic_string();
    }

    auto normalized = absolute.lexically_normal();
    auto canonical = fs::weakly_canonical(normalized, ec);
    if(ec) {
        return normalized.generic_string();
    }

    return canonical.generic_string();
}

auto run_build_dependency_graph_async(CompilationDatabase db,
                                      DependencyGraph& graph,
                                      ScanCache& cache) -> std::expected<void, ScanError> {
    kota::event_loop loop;
    auto task = build_dependency_graph_async(std::move(db), graph, &cache, loop);
    loop.schedule(task);
    loop.run();

    auto result = task.result();
    if(result.has_error()) {
        return std::unexpected(std::move(result.error()));
    }
    return {};
}

}  // namespace

TEST_SUITE(scan) {

TEST_CASE(topological_order_simple) {
    DependencyGraph graph;
    graph.files = {"a.h", "b.h", "c.cpp"};
    graph.edges = {
        {.from = "c.cpp", .to = "b.h"},
        {.from = "b.h",   .to = "a.h"},
    };

    auto order_result = topological_order(graph);
    ASSERT_TRUE(order_result.has_value());
    auto& order = *order_result;
    ASSERT_EQ(order.size(), 3u);

    // a.h should come before b.h, b.h should come before c.cpp
    std::size_t pos_a = 0, pos_b = 0, pos_c = 0;
    for(std::size_t i = 0; i < order.size(); ++i) {
        if(order[i] == "a.h")
            pos_a = i;
        if(order[i] == "b.h")
            pos_b = i;
        if(order[i] == "c.cpp")
            pos_c = i;
    }
    EXPECT_LT(pos_a, pos_b);
    EXPECT_LT(pos_b, pos_c);
}

TEST_CASE(topological_order_no_edges) {
    DependencyGraph graph;
    graph.files = {"a.cpp", "b.cpp", "c.cpp"};

    auto order_result = topological_order(graph);
    ASSERT_TRUE(order_result.has_value());
    EXPECT_EQ(order_result->size(), 3u);
}

TEST_CASE(topological_order_with_cycle) {
    DependencyGraph graph;
    graph.files = {"a.h", "b.h"};
    graph.edges = {
        {.from = "a.h", .to = "b.h"},
        {.from = "b.h", .to = "a.h"},
    };

    // A cycle must be reported as an error — not silently swallowed.
    auto order_result = topological_order(graph);
    EXPECT_FALSE(order_result.has_value());
    EXPECT_NE(order_result.error().message.find("cycle"), std::string::npos);
}

TEST_CASE(topological_order_diamond) {
    DependencyGraph graph;
    graph.files = {"base.h", "left.h", "right.h", "top.cpp"};
    graph.edges = {
        {.from = "left.h",  .to = "base.h" },
        {.from = "right.h", .to = "base.h" },
        {.from = "top.cpp", .to = "left.h" },
        {.from = "top.cpp", .to = "right.h"},
    };

    auto order_result = topological_order(graph);
    ASSERT_TRUE(order_result.has_value());
    auto& order = *order_result;
    ASSERT_EQ(order.size(), 4u);

    // base.h should come before left.h and right.h
    // left.h and right.h should come before top.cpp
    std::size_t pos_base = 0, pos_left = 0, pos_right = 0, pos_top = 0;
    for(std::size_t i = 0; i < order.size(); ++i) {
        if(order[i] == "base.h")
            pos_base = i;
        if(order[i] == "left.h")
            pos_left = i;
        if(order[i] == "right.h")
            pos_right = i;
        if(order[i] == "top.cpp")
            pos_top = i;
    }
    EXPECT_LT(pos_base, pos_left);
    EXPECT_LT(pos_base, pos_right);
    EXPECT_LT(pos_left, pos_top);
    EXPECT_LT(pos_right, pos_top);
}

TEST_CASE(build_dependency_graph_mints_missing_cache_keys) {
    namespace fs = std::filesystem;

    auto root = (fs::temp_directory_path() / "clore_scan_missing_cache_key").lexically_normal();
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root / "src");

    {
        std::ofstream source_file(root / "src" / "main.cpp");
        source_file << "int main() { return 0; }\n";
    }

    auto source = canonical_test_path(root / "src" / "main.cpp");

    CompilationDatabase db{
        .entries =
            {
                      CompileEntry{
                    .file = source,
                    .directory = root.generic_string(),
                    .arguments = {"clang++",
                                  "-std=c++23",
                                  "-c",
                                  source,
                                  "-o",
                                  (root / "src" / "main.o").generic_string()},
                    .cache_key = {},
                }, },
        .toolchain_cache = {},
    };

    auto source_entry = db.entries[0];
    ensure_cache_key(source_entry);

    ScanCache initial_cache;
    initial_cache.scan_results.emplace(source_entry.cache_key,
                                       ScanResult{
                                           .module_name = "seeded.module",
                                           .is_interface_unit = false,
                                           .includes = {},
                                           .module_imports = {},
                                       });
    DependencyGraph graph;

    auto result = run_build_dependency_graph_async(std::move(db), graph, initial_cache);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(graph.files.size(), 1u);
    EXPECT_TRUE(std::ranges::find(graph.files, source) != graph.files.end());
    EXPECT_EQ(initial_cache.scan_results.size(), 1u);
    EXPECT_TRUE(initial_cache.scan_results.contains(source_entry.cache_key));
    EXPECT_EQ(initial_cache.scan_results.at(source_entry.cache_key).module_name, "seeded.module");

    fs::remove_all(root, ec);
}

TEST_CASE(build_dependency_graph_with_initial_cache_mints_missing_cache_keys) {
    namespace fs = std::filesystem;

    auto root =
        (fs::temp_directory_path() / "clore_scan_missing_cache_key_seed").lexically_normal();
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root / "src");
    fs::create_directories(root / "include");

    {
        std::ofstream source_file(root / "src" / "main.cpp");
        source_file << "#include \"../include/header.hpp\"\nint main() { return 0; }\n";
    }
    {
        std::ofstream header_file(root / "include" / "header.hpp");
        header_file << "#pragma once\n";
    }

    auto source = canonical_test_path(root / "src" / "main.cpp");
    auto header = canonical_test_path(root / "include" / "header.hpp");

    CompilationDatabase db{
        .entries =
            {
                      CompileEntry{
                    .file = "src/main.cpp",
                    .directory = root.generic_string(),
                    .arguments = {},
                    .cache_key = {},
                },        CompileEntry{
                    .file = "include/header.hpp",
                    .directory = root.generic_string(),
                    .arguments = {},
                    .cache_key = {},
                }, },
        .toolchain_cache = {},
    };

    auto source_entry = db.entries[0];
    auto header_entry = db.entries[1];
    ensure_cache_key(source_entry);
    ensure_cache_key(header_entry);

    ScanCache initial_cache;
    initial_cache.scan_results.emplace(source_entry.cache_key,
                                       ScanResult{
                                           .module_name = {},
                                           .is_interface_unit = false,
                                           .includes = {IncludeInfo{.path = header}},
                                           .module_imports = {},
                                       });
    initial_cache.scan_results.emplace(header_entry.cache_key,
                                       ScanResult{
                                           .module_name = {},
                                           .is_interface_unit = false,
                                           .includes = {},
                                           .module_imports = {},
                                       });
    DependencyGraph graph;

    auto result = run_build_dependency_graph_async(std::move(db), graph, initial_cache);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(graph.files.size(), 2u);
    EXPECT_TRUE(std::ranges::find(graph.files, source) != graph.files.end());
    EXPECT_TRUE(std::ranges::find(graph.files, header) != graph.files.end());
    EXPECT_EQ(initial_cache.scan_results.size(), 2u);

    fs::remove_all(root, ec);
}

TEST_CASE(build_dependency_graph_clears_preexisting_graph_state) {
    namespace fs = std::filesystem;

    auto root = (fs::temp_directory_path() / "clore_scan_graph_reset").lexically_normal();
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root / "src");

    {
        std::ofstream source_file(root / "src" / "main.cpp");
        source_file << "int main() { return 0; }\n";
    }

    auto source = canonical_test_path(root / "src" / "main.cpp");

    CompilationDatabase db{
        .entries =
            {
                      CompileEntry{
                    .file = source,
                    .directory = root.generic_string(),
                    .arguments = {},
                    .cache_key = source + "\t1",
                }, },
        .toolchain_cache = {},
    };

    ScanCache initial_cache;
    initial_cache.scan_results.emplace(source + "\t1",
                                       ScanResult{
                                           .module_name = {},
                                           .is_interface_unit = false,
                                           .includes = {},
                                           .module_imports = {},
                                       });

    DependencyGraph graph;
    graph.files = {"stale.cpp"};
    graph.edges = {
        {.from = "stale.cpp", .to = "stale.hpp"}
    };

    auto result = run_build_dependency_graph_async(std::move(db), graph, initial_cache);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(graph.files.size(), 1u);
    EXPECT_EQ(graph.files[0], source);
    EXPECT_TRUE(graph.edges.empty());

    fs::remove_all(root, ec);
}

TEST_CASE(build_dependency_graph_scans_uncached_entries_and_populates_cache) {
    namespace fs = std::filesystem;

    auto root = (fs::temp_directory_path() / "clore_scan_uncached_entries").lexically_normal();
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root / "src");

    {
        std::ofstream source_file(root / "src" / "main.cpp");
        source_file << "#include \"lib.cpp\"\nint main() { return answer(); }\n";
    }
    {
        std::ofstream source_file(root / "src" / "lib.cpp");
        source_file << "int answer() { return 42; }\n";
    }

    auto main_source = canonical_test_path(root / "src" / "main.cpp");
    auto lib_source = canonical_test_path(root / "src" / "lib.cpp");

    CompilationDatabase db{
        .entries =
            {
                      CompileEntry{
                    .file = main_source,
                    .directory = root.generic_string(),
                    .arguments = {"clang++",
                                  "-std=c++23",
                                  "-c",
                                  main_source,
                                  "-o",
                                  (root / "src" / "main.o").generic_string()},
                    .cache_key = {},
                },        CompileEntry{
                    .file = lib_source,
                    .directory = root.generic_string(),
                    .arguments = {"clang++",
                                  "-std=c++23",
                                  "-c",
                                  lib_source,
                                  "-o",
                                  (root / "src" / "lib.o").generic_string()},
                    .cache_key = {},
                }, },
        .toolchain_cache = {},
    };

    DependencyGraph graph;
    ScanCache cache;

    auto result = run_build_dependency_graph_async(std::move(db), graph, cache);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(graph.files.size(), 2u);
    ASSERT_EQ(graph.edges.size(), 1u);
    EXPECT_EQ(graph.edges[0].from, main_source);
    EXPECT_EQ(graph.edges[0].to, lib_source);
    EXPECT_EQ(cache.scan_results.size(), 2u);
    EXPECT_TRUE(std::ranges::all_of(cache.scan_results, [](const auto& item) {
        return item.second.module_name.empty();
    }));

    fs::remove_all(root, ec);
}

TEST_CASE(build_dependency_graph_deduplicates_duplicate_sources_and_cache_writes) {
    namespace fs = std::filesystem;

    auto root = (fs::temp_directory_path() / "clore_scan_duplicate_sources").lexically_normal();
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root / "src");

    {
        std::ofstream source_file(root / "src" / "main.cpp");
        source_file << "#include \"lib.cpp\"\nint main() { return answer(); }\n";
    }
    {
        std::ofstream source_file(root / "src" / "lib.cpp");
        source_file << "int answer() { return 42; }\n";
    }

    auto main_source = canonical_test_path(root / "src" / "main.cpp");
    auto lib_source = canonical_test_path(root / "src" / "lib.cpp");

    CompilationDatabase db{
        .entries =
            {
                      CompileEntry{
                    .file = main_source,
                    .directory = root.generic_string(),
                    .arguments = {"clang++",
                                  "-std=c++23",
                                  "-c",
                                  main_source,
                                  "-o",
                                  (root / "src" / "main.o").generic_string()},
                    .cache_key = main_source + "\t1",
                }, CompileEntry{
                    .file = main_source,
                    .directory = root.generic_string(),
                    .arguments = {"clang++",
                                  "-std=c++23",
                                  "-c",
                                  main_source,
                                  "-o",
                                  (root / "src" / "main-dup.o").generic_string()},
                    .cache_key = main_source + "\t2",
                }, CompileEntry{
                    .file = lib_source,
                    .directory = root.generic_string(),
                    .arguments = {"clang++",
                                  "-std=c++23",
                                  "-c",
                                  lib_source,
                                  "-o",
                                  (root / "src" / "lib.o").generic_string()},
                    .cache_key = lib_source + "\t3",
                }, },
        .toolchain_cache = {},
    };

    DependencyGraph graph;
    ScanCache cache;

    auto result = run_build_dependency_graph_async(std::move(db), graph, cache);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(graph.files.size(), 2u);
    ASSERT_EQ(graph.edges.size(), 1u);
    EXPECT_EQ(graph.edges[0].from, main_source);
    EXPECT_EQ(graph.edges[0].to, lib_source);
    EXPECT_EQ(cache.scan_results.size(), 2u);
    EXPECT_TRUE(cache.scan_results.contains(main_source + "\t1"));
    EXPECT_FALSE(cache.scan_results.contains(main_source + "\t2"));
    EXPECT_TRUE(cache.scan_results.contains(lib_source + "\t3"));

    fs::remove_all(root, ec);
}

TEST_CASE(scan_module_decl_normalizes_partition_imports_and_deduplicates) {
    ScanResult result;

    scan_module_decl(R"(
export module extract:scan;
import :compiler;
import :compiler;
import support;
)",
                     result);

    EXPECT_EQ(result.module_name, "extract:scan");
    EXPECT_TRUE(result.is_interface_unit);
    ASSERT_EQ(result.module_imports.size(), 2u);
    EXPECT_EQ(result.module_imports[0], "extract:compiler");
    EXPECT_EQ(result.module_imports[1], "support");
}

TEST_CASE(scan_module_decl_ignores_module_fragments) {
    ScanResult global_fragment;
    scan_module_decl(R"(
module;
export module demo.core;
)",
                     global_fragment);

    EXPECT_EQ(global_fragment.module_name, "demo.core");
    EXPECT_TRUE(global_fragment.is_interface_unit);

    ScanResult private_fragment;
    scan_module_decl(R"(
module :private;
import :detail;
)",
                     private_fragment);

    EXPECT_TRUE(private_fragment.module_name.empty());
    EXPECT_FALSE(private_fragment.is_interface_unit);
    ASSERT_EQ(private_fragment.module_imports.size(), 1u);
    EXPECT_EQ(private_fragment.module_imports[0], ":detail");
}

TEST_CASE(build_dependency_graph_normalizes_relative_includes_like_entries) {
    namespace fs = std::filesystem;

    auto root = (fs::temp_directory_path() / "clore_scan_normalization").lexically_normal();
    auto source = canonical_test_path(root / "src" / "main.cpp");
    auto header = canonical_test_path(root / "header.hpp");

    CompilationDatabase db{
        .entries =
            {
                      CompileEntry{
                    .file = "./src/main.cpp",
                    .directory = root.generic_string(),
                    .arguments = {},
                    .cache_key = source + "\t1",
                },        CompileEntry{
                    .file = "include/../header.hpp",
                    .directory = root.generic_string(),
                    .arguments = {},
                    .cache_key = header + "\t2",
                }, },
        .toolchain_cache = {},
    };

    ScanCache initial_cache;
    initial_cache.scan_results.emplace(source + "\t1",
                                       ScanResult{
                                           .module_name = {},
                                           .is_interface_unit = false,
                                           .includes = {IncludeInfo{.path = "./header.hpp"}},
                                           .module_imports = {},
                                       });
    initial_cache.scan_results.emplace(header + "\t2",
                                       ScanResult{
                                           .module_name = {},
                                           .is_interface_unit = false,
                                           .includes = {},
                                           .module_imports = {},
                                       });
    DependencyGraph graph;

    auto result = run_build_dependency_graph_async(std::move(db), graph, initial_cache);

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(graph.edges.size(), 1u);
    EXPECT_EQ(graph.edges[0].from, source);
    EXPECT_EQ(graph.edges[0].to, header);
}

TEST_CASE(build_dependency_graph_canonicalizes_symlinked_entries_and_includes) {
    namespace fs = std::filesystem;

    auto root = (fs::temp_directory_path() / "clore_scan_symlink_normalization").lexically_normal();
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root / "src");
    fs::create_directories(root / "real");

    {
        std::ofstream source_file(root / "src" / "main.cpp");
        source_file << "#include \"header.hpp\"\n";
    }
    {
        std::ofstream header_file(root / "real" / "header.hpp");
        header_file << "#pragma once\n";
    }

    fs::create_directory_symlink(root / "real", root / "alias", ec);
    ASSERT_TRUE(!ec);

    auto source = canonical_test_path(root / "src" / "main.cpp");
    auto header_real = canonical_test_path(root / "real" / "header.hpp");
    auto header_alias = (root / "alias" / "header.hpp").lexically_normal().generic_string();

    CompilationDatabase db{
        .entries =
            {
                      CompileEntry{
                    .file = "src/main.cpp",
                    .directory = root.generic_string(),
                    .arguments = {},
                    .cache_key = source + "\t1",
                },        CompileEntry{
                    .file = "alias/header.hpp",
                    .directory = root.generic_string(),
                    .arguments = {},
                    .cache_key = header_alias + "\t2",
                }, },
        .toolchain_cache = {},
    };

    ScanCache initial_cache;
    initial_cache.scan_results.emplace(source + "\t1",
                                       ScanResult{
                                           .module_name = {},
                                           .is_interface_unit = false,
                                           .includes = {IncludeInfo{.path = header_real}},
                                           .module_imports = {},
                                       });
    initial_cache.scan_results.emplace(header_alias + "\t2",
                                       ScanResult{
                                           .module_name = {},
                                           .is_interface_unit = false,
                                           .includes = {},
                                           .module_imports = {},
                                       });
    DependencyGraph graph;

    auto result = run_build_dependency_graph_async(std::move(db), graph, initial_cache);

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(graph.edges.size(), 1u);
    EXPECT_EQ(graph.edges[0].from, source);
    EXPECT_EQ(graph.edges[0].to, header_real);
    EXPECT_TRUE(std::ranges::find(graph.files, header_real) != graph.files.end());

    fs::remove_all(root, ec);
}

};  // TEST_SUITE(scan)
