#include <filesystem>

#include "kota/zest/zest.h"

import extract;
import config;

using namespace clore::extract;
using namespace clore::config;

namespace fs = std::filesystem;

TEST_SUITE(extract_filter) {

TEST_CASE(path_prefix_matches_exact) {
    EXPECT_TRUE(path_prefix_matches("src", "src"));
    EXPECT_TRUE(path_prefix_matches("src/math.cpp", "src"));
    EXPECT_TRUE(path_prefix_matches("src/detail/math.cpp", "src/detail"));
}

TEST_CASE(path_prefix_matches_rejects_non_prefix) {
    EXPECT_FALSE(path_prefix_matches("src/math.cpp", "math"));
    EXPECT_FALSE(path_prefix_matches("src/math.cpp", "src2"));
    EXPECT_FALSE(path_prefix_matches("srcmath.cpp", "src"));
}

TEST_CASE(path_prefix_matches_empty_pattern) {
    EXPECT_FALSE(path_prefix_matches("src", ""));
    EXPECT_FALSE(path_prefix_matches("src", "/"));
}

TEST_CASE(path_prefix_matches_trailing_slash_stripped) {
    EXPECT_TRUE(path_prefix_matches("src/math.cpp", "src/"));
    EXPECT_TRUE(path_prefix_matches("src/math.cpp", "src//"));
}

TEST_CASE(project_relative_path_rejects_escape) {
    auto base = fs::path("/project").lexically_normal();
    auto result = project_relative_path(fs::path("/project/../secret.cpp"), base);
    EXPECT_FALSE(result.has_value());
}

TEST_CASE(project_relative_path_accepts_child) {
    auto base = fs::path("/project").lexically_normal();
    auto result = project_relative_path(fs::path("/project/src/main.cpp"), base);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->generic_string(), "src/main.cpp");
}

TEST_CASE(resolve_path_under_directory_empty_path) {
    auto result = resolve_path_under_directory("", "/project");
    EXPECT_FALSE(result.has_value());
}

TEST_CASE(resolve_path_under_directory_relative_needs_directory) {
    auto result = resolve_path_under_directory("src/main.cpp", "");
    EXPECT_FALSE(result.has_value());
}

TEST_CASE(resolve_path_under_directory_resolves_relative) {
    auto result = resolve_path_under_directory("src/main.cpp", "/project");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->lexically_normal().generic_string(), "/project/src/main.cpp");
}

TEST_CASE(resolve_path_under_directory_preserves_absolute) {
    auto result = resolve_path_under_directory("/abs/main.cpp", "/project");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->lexically_normal().generic_string(), "/abs/main.cpp");
}

TEST_CASE(matches_filter_include_only) {
    FilterRule filter;
    filter.include = {"src"};

    EXPECT_TRUE(matches_filter("/project/src/main.cpp", filter, fs::path("/project")));
    EXPECT_FALSE(matches_filter("/project/tests/main.cpp", filter, fs::path("/project")));
}

TEST_CASE(matches_filter_exclude_wins) {
    FilterRule filter;
    filter.include = {"src"};
    filter.exclude = {"src/detail"};

    EXPECT_TRUE(matches_filter("/project/src/main.cpp", filter, fs::path("/project")));
    EXPECT_FALSE(matches_filter("/project/src/detail/internal.cpp", filter, fs::path("/project")));
}

TEST_CASE(matches_filter_no_include_matches_all_except_exclude) {
    FilterRule filter;
    filter.exclude = {"tests"};

    EXPECT_TRUE(matches_filter("/project/src/main.cpp", filter, fs::path("/project")));
    EXPECT_FALSE(matches_filter("/project/tests/main.cpp", filter, fs::path("/project")));
}

TEST_CASE(filter_root_path_prefers_workspace) {
    TaskConfig config;
    config.workspace_root = "/ws";
    config.project_root = "/proj";

    EXPECT_EQ(filter_root_path(config), fs::path("/ws").lexically_normal());
}

TEST_CASE(filter_root_path_falls_back_to_project) {
    TaskConfig config;
    config.project_root = "/proj";

    EXPECT_EQ(filter_root_path(config), fs::path("/proj").lexically_normal());
}

TEST_CASE(symbol_filter_allows_public_by_default) {
    SymbolInfo sym;
    sym.id = SymbolID{.hash = 1};
    sym.name = "Widget";
    sym.access = "public";
    sym.enclosing_namespace = "demo";

    SymbolFilterConfig rule;
    EXPECT_TRUE(matches_symbol_filter(sym, rule));
}

TEST_CASE(symbol_filter_hide_private) {
    SymbolInfo sym;
    sym.id = SymbolID{.hash = 2};
    sym.name = "impl_";
    sym.access = "private";
    sym.enclosing_namespace = "demo";

    SymbolFilterConfig rule;
    rule.hide_private = true;
    EXPECT_FALSE(matches_symbol_filter(sym, rule));

    // Public symbols not affected
    sym.access = "public";
    EXPECT_TRUE(matches_symbol_filter(sym, rule));
}

TEST_CASE(symbol_filter_hide_protected) {
    SymbolInfo sym;
    sym.id = SymbolID{.hash = 3};
    sym.name = "helper";
    sym.access = "protected";
    sym.enclosing_namespace = "base";

    SymbolFilterConfig rule;
    rule.hide_protected = true;
    EXPECT_FALSE(matches_symbol_filter(sym, rule));

    // Private not affected by hide_protected alone
    sym.access = "private";
    EXPECT_TRUE(matches_symbol_filter(sym, rule));
}

TEST_CASE(symbol_filter_exclude_name_prefix) {
    SymbolInfo sym;
    sym.id = SymbolID{.hash = 4};
    sym.name = "_impl_helper";
    sym.access = "public";
    sym.enclosing_namespace = "detail";

    SymbolFilterConfig rule;
    rule.exclude_name_prefixes = {"_impl_"};
    EXPECT_FALSE(matches_symbol_filter(sym, rule));

    // Non-matching prefix not filtered
    sym.name = "Widget";
    EXPECT_TRUE(matches_symbol_filter(sym, rule));
}

TEST_CASE(symbol_filter_exclude_namespace_segment) {
    SymbolInfo sym;
    sym.id = SymbolID{.hash = 5};
    sym.name = "Impl";
    sym.access = "public";
    sym.enclosing_namespace = "clore::detail";

    SymbolFilterConfig rule;
    rule.exclude_namespace_segments = {"detail"};
    EXPECT_FALSE(matches_symbol_filter(sym, rule));

    // Different namespace not filtered
    sym.enclosing_namespace = "clore::extract";
    EXPECT_TRUE(matches_symbol_filter(sym, rule));
}

};  // TEST_SUITE(extract_filter)
