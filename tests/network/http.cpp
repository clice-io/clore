#include <string>

#include "kota/async/async.h"
#include "kota/zest/zest.h"

import http;

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
    EXPECT_NE(result.error().message.find("request failed"), std::string::npos);
}

TEST_CASE(perform_http_request_async_reports_invalid_url) {
    kota::event_loop loop;
    auto operation = perform_http_request_async("://invalid-url", "test-key", R"({"x":1})", loop);

    loop.schedule(operation);
    loop.run();

    auto result = operation.result();
    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("request failed"), std::string::npos);
}

};  // TEST_SUITE(network_http)
