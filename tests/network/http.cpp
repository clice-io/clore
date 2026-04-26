#include "kota/http/http.h"

#include <string>
#include <vector>

#include "kota/async/async.h"
#include "kota/zest/zest.h"

import http;

using namespace clore::net::detail;

TEST_SUITE(network_http) {

TEST_CASE(perform_http_request_reports_invalid_url) {
    std::vector<kota::http::header> headers{
        kota::http::header{
                           .name = "Content-Type",
                           .value = "application/json; charset=utf-8",
                           },
        kota::http::header{
                           .name = "Authorization",
                           .value = "Bearer test-key",
                           },
    };

    auto result = perform_http_request("://invalid-url", headers, R"({"x":1})");

    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("request failed"), std::string::npos);
}

TEST_CASE(perform_http_request_async_reports_invalid_url) {
    kota::event_loop loop;
    std::vector<kota::http::header> headers{
        kota::http::header{
                           .name = "Content-Type",
                           .value = "application/json; charset=utf-8",
                           },
        kota::http::header{
                           .name = "Authorization",
                           .value = "Bearer test-key",
                           },
    };
    auto operation =
        perform_http_request_async("://invalid-url", std::move(headers), R"({"x":1})", loop);

    loop.schedule(operation);
    loop.run();

    auto result = operation.result();
    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("request failed"), std::string::npos);
}

};  // TEST_SUITE(network_http)
