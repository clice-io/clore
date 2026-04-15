#include "eventide/zest/zest.h"

import extract;

using namespace clore::extract;

TEST_SUITE(symbol) {
    TEST_CASE(symbol_kind_names) {
        EXPECT_EQ(symbol_kind_name(SymbolKind::Namespace), "namespace");
        EXPECT_EQ(symbol_kind_name(SymbolKind::Class), "class");
        EXPECT_EQ(symbol_kind_name(SymbolKind::Struct), "struct");
        EXPECT_EQ(symbol_kind_name(SymbolKind::Union), "union");
        EXPECT_EQ(symbol_kind_name(SymbolKind::Enum), "enum");
        EXPECT_EQ(symbol_kind_name(SymbolKind::EnumMember), "enum_member");
        EXPECT_EQ(symbol_kind_name(SymbolKind::Function), "function");
        EXPECT_EQ(symbol_kind_name(SymbolKind::Method), "method");
        EXPECT_EQ(symbol_kind_name(SymbolKind::Variable), "variable");
        EXPECT_EQ(symbol_kind_name(SymbolKind::Field), "field");
        EXPECT_EQ(symbol_kind_name(SymbolKind::TypeAlias), "type_alias");
        EXPECT_EQ(symbol_kind_name(SymbolKind::Macro), "macro");
        EXPECT_EQ(symbol_kind_name(SymbolKind::Template), "template");
        EXPECT_EQ(symbol_kind_name(SymbolKind::Concept), "concept");
        EXPECT_EQ(symbol_kind_name(SymbolKind::Unknown), "unknown");
    }

    TEST_CASE(symbol_id_equality) {
        SymbolID a{.hash = 42};
        SymbolID b{.hash = 42};
        SymbolID c{.hash = 99};

        EXPECT_EQ(a, b);
        EXPECT_NE(a, c);
    }

    TEST_CASE(symbol_id_hash) {
        SymbolID id{.hash = 123456};
        auto h = std::hash<SymbolID>{}(id);
        EXPECT_EQ(h, std::hash<std::uint64_t>{}(123456));
    }

    TEST_CASE(symbol_id_ordering) {
        SymbolID a{.hash = 10};
        SymbolID b{.hash = 20};

        EXPECT_LT(a, b);
        EXPECT_GT(b, a);
    }

    TEST_CASE(split_top_level_qualified_name_ignores_leading_global_qualifier) {
        auto parts = split_top_level_qualified_name("::demo::Widget");

        ASSERT_EQ(parts.size(), 2u);
        EXPECT_EQ(parts[0], "demo");
        EXPECT_EQ(parts[1], "Widget");
        EXPECT_EQ(namespace_prefix_from_qualified_name("::demo::Widget"), "demo");
    }
};
