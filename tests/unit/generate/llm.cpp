#include "eventide/zest/zest.h"

#include <cstdlib>
#include <optional>
#include <string>

import generate;

using namespace clore::generate;

namespace {

struct ScopedEnvVar {
    std::string name;
    std::optional<std::string> original_value;

    explicit ScopedEnvVar(std::string var_name) : name(std::move(var_name)) {
        if(auto* value = std::getenv(name.c_str()); value != nullptr) {
            original_value = value;
        }
    }

    ~ScopedEnvVar() { restore(); }

    void set(std::string_view value) const {
#ifdef _WIN32
        _putenv_s(name.c_str(), std::string(value).c_str());
#else
        setenv(name.c_str(), std::string(value).c_str(), 1);
#endif
    }

    void unset() const {
#ifdef _WIN32
        _putenv_s(name.c_str(), "");
#else
        unsetenv(name.c_str());
#endif
    }

    void restore() const {
        if(original_value.has_value()) {
            set(*original_value);
            return;
        }
        unset();
    }
};

}  // namespace

TEST_SUITE(llm) {
    TEST_CASE(build_request_json_uses_requested_model) {
        auto json = detail::build_request_json("gpt-5.2", "Document this function.");

        EXPECT_NE(json.find(R"("model":"gpt-5.2")"), std::string::npos);
        EXPECT_NE(json.find("Document this function."), std::string::npos);
    }

    TEST_CASE(parse_response_success) {
        auto result = detail::parse_response(
            R"({"choices":[{"message":{"content":"# Title\nGenerated docs"}}]})");

        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, "# Title\nGenerated docs");
    }

    TEST_CASE(parse_response_api_error) {
        auto result = detail::parse_response(
            R"({"error":{"message":"bad request"}})");

        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(call_llm_requires_openai_base_url_env) {
        ScopedEnvVar base_url("OPENAI_BASE_URL");
        ScopedEnvVar api_key("OPENAI_API_KEY");

        base_url.unset();
        api_key.set("test-key");

        auto result = call_llm("gpt-5.2", "ping");

        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error().message,
                  "required environment variable OPENAI_BASE_URL is not set");
    }

    TEST_CASE(call_llm_requires_openai_api_key_env) {
        ScopedEnvVar base_url("OPENAI_BASE_URL");
        ScopedEnvVar api_key("OPENAI_API_KEY");

        base_url.set("https://example.invalid/v1");
        api_key.unset();

        auto result = call_llm("gpt-5.2", "ping");

        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error().message,
                  "required environment variable OPENAI_API_KEY is not set");
    }
};
