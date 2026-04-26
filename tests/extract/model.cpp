#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>

#include "kota/zest/zest.h"

import extract;

using namespace clore::extract;

namespace {

constexpr std::uint64_t kSourceSnippetHashOffsetBasis = 14695981039346656037ULL;
constexpr std::uint64_t kSourceSnippetHashPrime = 1099511628211ULL;

auto hash_source_snippet_bytes(std::string_view bytes) -> std::uint64_t {
    auto hash = kSourceSnippetHashOffsetBasis;
    for(auto ch: bytes) {
        hash ^= static_cast<std::uint64_t>(static_cast<unsigned char>(ch));
        hash *= kSourceSnippetHashPrime;
    }
    return hash;
}

}  // namespace

TEST_SUITE(model) {

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

TEST_CASE(resolve_source_snippet_returns_false_for_missing_files) {
    SymbolInfo symbol;
    symbol.declaration_location.file = "/path/that/does/not/exist.cpp";
    symbol.source_snippet_offset = 0;
    symbol.source_snippet_length = 32;

    EXPECT_FALSE(resolve_source_snippet(symbol));
}

TEST_CASE(resolve_source_snippet_rejects_stale_same_size_content) {
    namespace fs = std::filesystem;

    auto temp_path = fs::temp_directory_path() /
                     std::format("clore_model_snippet_{}.cpp",
                                 std::chrono::steady_clock::now().time_since_epoch().count());
    constexpr std::string_view kSnippet = "function()";
    const std::string original = "prefix function() suffix\n";
    const std::string rewritten = "prefix variable() suffix\n";
    const auto offset = original.find(kSnippet);
    ASSERT_NE(offset, std::string::npos);

    {
        std::ofstream f(temp_path, std::ios::binary);
        f << original;
    }

    SymbolInfo symbol;
    symbol.declaration_location.file = temp_path.generic_string();
    symbol.source_snippet_offset = static_cast<std::uint32_t>(offset);
    symbol.source_snippet_length = static_cast<std::uint32_t>(kSnippet.size());
    symbol.source_snippet_file_size = original.size();
    symbol.source_snippet_hash = hash_source_snippet_bytes(kSnippet);

    ASSERT_TRUE(resolve_source_snippet(symbol));
    EXPECT_EQ(symbol.source_snippet, kSnippet);

    {
        std::ofstream f(temp_path, std::ios::binary | std::ios::trunc);
        f << rewritten;
    }

    SymbolInfo stale_symbol;
    stale_symbol.declaration_location.file = temp_path.generic_string();
    stale_symbol.source_snippet_offset = static_cast<std::uint32_t>(offset);
    stale_symbol.source_snippet_length = static_cast<std::uint32_t>(kSnippet.size());
    stale_symbol.source_snippet_file_size = original.size();
    stale_symbol.source_snippet_hash = hash_source_snippet_bytes(kSnippet);

    EXPECT_FALSE(resolve_source_snippet(stale_symbol));

    std::error_code ec;
    fs::remove(temp_path, ec);
}

};  // TEST_SUITE(model)
