#include "generate/prelude.h"

import config;
import extract;
import generate;

#include "generate/support.h"

using namespace clore::generate;

TEST_SUITE(generate_prompt) {

TEST_CASE(normalize_markdown_fragment_trims_trailing_newlines) {
    auto result = normalize_markdown_fragment("Hello world.\n\n", "test");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "Hello world.");
}

TEST_CASE(normalize_markdown_fragment_rejects_empty) {
    auto result = normalize_markdown_fragment("\n\n", "test");
    EXPECT_FALSE(result.has_value());
}

TEST_CASE(normalize_markdown_fragment_rejects_whitespace_only) {
    auto result = normalize_markdown_fragment("   \n\t\n  ", "test");
    EXPECT_FALSE(result.has_value());
}

TEST_CASE(parse_markdown_prompt_output_normalizes_input) {
    auto result = parse_markdown_prompt_output("  Summary text.  ", "test");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "  Summary text.");
}

};  // TEST_SUITE(generate_prompt)
