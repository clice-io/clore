#include <vector>

#include "kota/zest/zest.h"

import extract;
import config;

using namespace clore::extract;
using namespace clore::config;

TEST_SUITE(extract_merge) {

TEST_CASE(append_unique_adds_once) {
    std::vector<int> v;
    append_unique(v, 1);
    append_unique(v, 1);
    append_unique(v, 2);

    ASSERT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
}

TEST_CASE(append_unique_range_prefers_smaller_incoming) {
    std::vector<int> base = {1, 2, 3, 4, 5};
    std::vector<int> incoming = {3, 4, 6};
    append_unique_range(base, incoming);

    ASSERT_EQ(base.size(), 6u);
    EXPECT_EQ(base.back(), 6);
}

TEST_CASE(append_unique_range_prefers_smaller_base) {
    std::vector<int> base = {1, 2};
    std::vector<int> incoming = {2, 3, 4, 5, 6};
    append_unique_range(base, incoming);

    ASSERT_EQ(base.size(), 6u);
}

TEST_CASE(deduplicate_sorts_and_removes_duplicates) {
    std::vector<int> v = {3, 1, 2, 1, 3};
    deduplicate(v);

    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
}

TEST_CASE(merge_symbol_info_prefers_incoming_definition) {
    SymbolInfo current;
    current.id = SymbolID{.hash = 1};
    current.name = "foo";
    current.signature = "void foo()";
    current.declaration_location = SourceLocation{.file = "a.cpp", .line = 1, .column = 1};

    SymbolInfo incoming;
    incoming.id = SymbolID{.hash = 1};
    incoming.name = "foo";
    incoming.signature = "void foo()";
    incoming.declaration_location = SourceLocation{.file = "a.cpp", .line = 1, .column = 1};
    incoming.definition_location = SourceLocation{.file = "a.cpp", .line = 5, .column = 1};
    incoming.source_snippet = "void foo() {}";
    incoming.source_snippet_length = 13;

    merge_symbol_info(current, std::move(incoming));

    ASSERT_TRUE(current.definition_location.has_value());
    EXPECT_EQ(current.definition_location->line, 5u);
    EXPECT_EQ(current.source_snippet, "void foo() {}");
    EXPECT_EQ(current.source_snippet_length, 13u);
}

TEST_CASE(merge_symbol_info_keeps_current_if_incoming_empty) {
    SymbolInfo current;
    current.id = SymbolID{.hash = 1};
    current.name = "foo";
    current.doc_comment = "existing docs";

    SymbolInfo incoming;
    incoming.id = SymbolID{.hash = 1};
    incoming.name = "foo";

    merge_symbol_info(current, std::move(incoming));

    EXPECT_EQ(current.doc_comment, "existing docs");
}

TEST_CASE(merge_symbol_info_merges_relations) {
    SymbolInfo current;
    current.id = SymbolID{.hash = 1};
    current.calls = {SymbolID{.hash = 10}};

    SymbolInfo incoming;
    incoming.id = SymbolID{.hash = 1};
    incoming.calls = {SymbolID{.hash = 10}, SymbolID{.hash = 20}};

    merge_symbol_info(current, std::move(incoming));

    ASSERT_EQ(current.calls.size(), 2u);
}

TEST_CASE(rebuild_lookup_maps_groups_overloads) {
    ProjectModel model;
    auto file = "test.cpp";

    SymbolInfo a;
    a.id = SymbolID{.hash = 1};
    a.kind = SymbolKind::Function;
    a.qualified_name = "demo::foo";
    a.signature = "int foo(int)";
    a.declaration_location = SourceLocation{.file = file, .line = 1, .column = 1};
    model.symbols[a.id] = std::move(a);

    SymbolInfo b;
    b.id = SymbolID{.hash = 2};
    b.kind = SymbolKind::Function;
    b.qualified_name = "demo::foo";
    b.signature = "double foo(double)";
    b.declaration_location = SourceLocation{.file = file, .line = 2, .column = 1};
    model.symbols[b.id] = std::move(b);

    rebuild_lookup_maps(model);

    ASSERT_EQ(model.symbol_ids_by_qualified_name["demo::foo"].size(), 2u);
}

TEST_CASE(rebuild_lookup_maps_builds_module_name_to_sources) {
    ProjectModel model;
    model.modules["a.cpp"] = ModuleUnit{
        .name = "math",
        .is_interface = true,
        .source_file = "a.cpp",
    };
    model.modules["b.cpp"] = ModuleUnit{
        .name = "math",
        .is_interface = false,
        .source_file = "b.cpp",
    };

    rebuild_lookup_maps(model);

    ASSERT_EQ(model.module_name_to_sources["math"].size(), 2u);
}

};  // TEST_SUITE(extract_merge)
