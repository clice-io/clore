#include "generate/prelude.h"

import config;
import extract;
import generate;

#include "generate/support.h"

using namespace clore::generate;

TEST_SUITE(generate_analysis) {

TEST_CASE(analysis_prompt_kind_for_symbol_function) {
    auto sym = make_function_symbol(1, "foo", "ns::foo", "void foo()", "file.cpp", "ns");
    auto kind = analysis_prompt_kind_for_symbol(sym);
    ASSERT_TRUE(kind.has_value());
    EXPECT_EQ(*kind, PromptKind::FunctionAnalysis);
}

TEST_CASE(analysis_prompt_kind_for_symbol_type) {
    auto sym = make_type_symbol(1, "Widget", "ns::Widget", "file.cpp", "doc", "ns");
    auto kind = analysis_prompt_kind_for_symbol(sym);
    ASSERT_TRUE(kind.has_value());
    EXPECT_EQ(*kind, PromptKind::TypeAnalysis);
}

TEST_CASE(analysis_prompt_kind_for_symbol_variable) {
    auto sym = make_type_symbol(1, "count", "ns::count", "file.cpp", "doc", "ns");
    sym.kind = extract::SymbolKind::Variable;
    auto kind = analysis_prompt_kind_for_symbol(sym);
    ASSERT_TRUE(kind.has_value());
    EXPECT_EQ(*kind, PromptKind::VariableAnalysis);
}

TEST_CASE(symbol_prompt_kinds_for_symbol_function_returns_three) {
    auto sym = make_function_symbol(1, "foo", "ns::foo", "void foo()", "file.cpp", "ns");
    auto kinds = symbol_prompt_kinds_for_symbol(sym);
    EXPECT_EQ(kinds.size(), 3u);
}

TEST_CASE(is_base_symbol_analysis_prompt) {
    EXPECT_TRUE(is_base_symbol_analysis_prompt(PromptKind::FunctionAnalysis));
    EXPECT_TRUE(is_base_symbol_analysis_prompt(PromptKind::TypeAnalysis));
    EXPECT_TRUE(is_base_symbol_analysis_prompt(PromptKind::VariableAnalysis));
    EXPECT_FALSE(is_base_symbol_analysis_prompt(PromptKind::FunctionDeclarationSummary));
}

TEST_CASE(is_declaration_summary_prompt) {
    EXPECT_TRUE(is_declaration_summary_prompt(PromptKind::FunctionDeclarationSummary));
    EXPECT_TRUE(is_declaration_summary_prompt(PromptKind::TypeDeclarationSummary));
    EXPECT_FALSE(is_declaration_summary_prompt(PromptKind::FunctionImplementationSummary));
}

TEST_CASE(store_fallback_analysis_populates_store) {
    auto sym = make_function_symbol(1, "foo", "ns::foo", "void foo()", "file.cpp", "ns");
    SymbolAnalysisStore store;
    extract::ProjectModel model;
    store_fallback_analysis(store, sym, model);
    auto key = make_symbol_target_key(sym);
    EXPECT_TRUE(find_function_analysis(store, key) != nullptr);
}

};  // TEST_SUITE(generate_analysis)
