#include <coroutine>
#include <cstdlib>

#include "kota/async/async.h"
#include "kota/zest/zest.h"

import network;

using namespace clore::net;

TEST_SUITE(network_dispatch) {

TEST_CASE(validate_llm_provider_environment_reports_missing_env) {
    auto* anthropic_base_url = std::getenv("ANTHROPIC_BASE_URL");
    auto* anthropic_api_key = std::getenv("ANTHROPIC_API_KEY");
    auto* openai_base_url = std::getenv("OPENAI_BASE_URL");
    auto* openai_api_key = std::getenv("OPENAI_API_KEY");
    if((anthropic_base_url != nullptr && anthropic_base_url[0] != '\0' &&
        anthropic_api_key != nullptr && anthropic_api_key[0] != '\0') ||
       (openai_base_url != nullptr && openai_base_url[0] != '\0' && openai_api_key != nullptr &&
        openai_api_key[0] != '\0')) {
        return;
    }

    auto result = validate_llm_provider_environment();

    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("ANTHROPIC_BASE_URL"), std::string::npos);
    EXPECT_NE(result.error().message.find("OPENAI_BASE_URL"), std::string::npos);
}

};  // TEST_SUITE(network_dispatch)
