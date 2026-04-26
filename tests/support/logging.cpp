#include <string>

#include "kota/zest/zest.h"

import support;

using namespace clore::support;

TEST_SUITE(logging_support) {

TEST_CASE(truncate_utf8_preserves_code_point_boundaries) {
    const std::string text = "A\xE4\xB8\xAD\xE6\x96\x87\xF0\x9F\x99\x82" "B";

    EXPECT_EQ(truncate_utf8(text, 1), "A");
    EXPECT_EQ(truncate_utf8(text, 4), std::string("A\xE4\xB8\xAD"));
    EXPECT_EQ(truncate_utf8(text, 10), std::string("A\xE4\xB8\xAD\xE6\x96\x87"));
    EXPECT_EQ(truncate_utf8(text, 11), std::string("A\xE4\xB8\xAD\xE6\x96\x87\xF0\x9F\x99\x82"));
}

TEST_CASE(truncate_utf8_repairs_invalid_bytes_before_truncating) {
    std::string text = "bad";
    text.push_back(static_cast<char>(0xFF));
    text += "prompt";

    auto truncated = truncate_utf8(text, 6);

    EXPECT_EQ(truncated, std::string("bad\xEF\xBF\xBD"));
}

TEST_CASE(normalize_line_endings_rewrites_crlf_and_cr) {
    auto normalized = normalize_line_endings("line1\r\nline2\rline3\n");

    EXPECT_EQ(normalized, "line1\nline2\nline3\n");
}

TEST_CASE(canonical_log_level_name_accepts_case_insensitive_names) {
    auto off = canonical_log_level_name("OFF");
    auto mixed = canonical_log_level_name("Off");
    auto warn = canonical_log_level_name("WaRn");
    auto invalid = canonical_log_level_name("verbose");

    ASSERT_TRUE(off.has_value());
    ASSERT_TRUE(mixed.has_value());
    ASSERT_TRUE(warn.has_value());
    EXPECT_EQ(*off, "off");
    EXPECT_EQ(*mixed, "off");
    EXPECT_EQ(*warn, "warn");
    EXPECT_FALSE(invalid.has_value());
}

};  // TEST_SUITE(logging_support)
