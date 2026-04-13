#include "eventide/zest/zest.h"

#include <string>

import network;

using namespace clore::net::detail;

TEST_SUITE(network_http) {
    TEST_CASE(build_chat_completions_url_normalizes_slash) {
        EXPECT_EQ(build_chat_completions_url("https://api.example/v1"),
                  "https://api.example/v1/chat/completions");
        EXPECT_EQ(build_chat_completions_url("https://api.example/v1/"),
                  "https://api.example/v1/chat/completions");
    }

    TEST_CASE(perform_http_request_reports_invalid_url) {
        auto result = perform_http_request("://invalid-url", "test-key", R"({"x":1})");
        EXPECT_FALSE(result.has_value());
        EXPECT_NE(result.error().message.find("curl"), std::string::npos);
    }
};
