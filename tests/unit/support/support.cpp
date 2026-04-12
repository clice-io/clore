#include "eventide/zest/zest.h"

#include <string>

import support;

using namespace clore::support;

TEST_SUITE(support_utf8) {
    TEST_CASE(truncate_utf8_preserves_code_point_boundaries) {
        const std::string text =
            "A\xE4\xB8\xAD\xE6\x96\x87\xF0\x9F\x99\x82" "B";

        EXPECT_EQ(truncate_utf8(text, 1), "A");
        EXPECT_EQ(truncate_utf8(text, 4), std::string("A\xE4\xB8\xAD"));
        EXPECT_EQ(truncate_utf8(text, 10), std::string("A\xE4\xB8\xAD\xE6\x96\x87"));
        EXPECT_EQ(truncate_utf8(text, 11),
                  std::string("A\xE4\xB8\xAD\xE6\x96\x87\xF0\x9F\x99\x82"));
    }

    TEST_CASE(truncate_utf8_repairs_invalid_bytes_before_truncating) {
        std::string text = "bad";
        text.push_back(static_cast<char>(0xFF));
        text += "prompt";

        auto truncated = truncate_utf8(text, 6);

        EXPECT_EQ(truncated, std::string("bad\xEF\xBF\xBD"));
    }
};
