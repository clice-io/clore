#include "generate/prelude.h"

import config;
import extract;
import generate;

#include "generate/support.h"

using namespace clore::generate;

TEST_SUITE(render_common) {

TEST_CASE(trim_ascii_strips_spaces) {
    EXPECT_EQ(trim_ascii("  hello  "), "hello");
    EXPECT_EQ(trim_ascii("\t\nworld\r\n"), "world");
}

TEST_CASE(trim_ascii_empty_input) {
    EXPECT_EQ(trim_ascii(""), "");
    EXPECT_EQ(trim_ascii("   "), "");
}

TEST_CASE(short_name_of_returns_last_part) {
    EXPECT_EQ(short_name_of("ns::Type"), "Type");
    EXPECT_EQ(short_name_of("Global"), "Global");
    EXPECT_EQ(short_name_of(""), "");
}

TEST_CASE(namespace_of_returns_prefix) {
    EXPECT_EQ(namespace_of("ns::Type"), "ns");
    EXPECT_EQ(namespace_of("a::b::c"), "a::b");
    EXPECT_EQ(namespace_of("Global"), "");
}

TEST_CASE(strip_inline_markdown_removes_formatting) {
    EXPECT_EQ(strip_inline_markdown("`Widget`"), "Widget");
    EXPECT_EQ(strip_inline_markdown("**Bold**"), "Bold");
    EXPECT_EQ(strip_inline_markdown("[link](url)"), "link(url)");
}

};  // TEST_SUITE(render_common)
