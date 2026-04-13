#include "eventide/zest/zest.h"

#include <cstdlib>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

import network;

using namespace clore::net::detail;

namespace {

auto environment_mutex() -> std::mutex& {
    static std::mutex mutex;
    return mutex;
}

struct ScopedEnvironmentLock {
    std::unique_lock<std::mutex> lock;

    ScopedEnvironmentLock() : lock(environment_mutex()) {}
};

struct ScopedEnvVar {
    std::string name;
    std::optional<std::string> original;

    explicit ScopedEnvVar(std::string env_name) : name(std::move(env_name)) {
        if(auto* value = std::getenv(name.c_str()); value != nullptr) {
            original = value;
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
        if(original.has_value()) {
            set(*original);
        } else {
            unset();
        }
    }
};

}  // namespace

TEST_SUITE(network_http) {
    TEST_CASE(build_chat_completions_url_normalizes_slash) {
        EXPECT_EQ(build_chat_completions_url("https://api.example/v1"),
                  "https://api.example/v1/chat/completions");
        EXPECT_EQ(build_chat_completions_url("https://api.example/v1/"),
                  "https://api.example/v1/chat/completions");
    }

    TEST_CASE(read_environment_requires_base_url) {
        ScopedEnvironmentLock env_lock;
        ScopedEnvVar base_url("OPENAI_BASE_URL");
        ScopedEnvVar api_key("OPENAI_API_KEY");

        base_url.unset();
        api_key.set("test-key");

        auto result = read_environment();
        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error().message,
                  "required environment variable OPENAI_BASE_URL is not set");
    }

    TEST_CASE(read_environment_requires_api_key) {
        ScopedEnvironmentLock env_lock;
        ScopedEnvVar base_url("OPENAI_BASE_URL");
        ScopedEnvVar api_key("OPENAI_API_KEY");

        base_url.set("https://example.invalid/v1");
        api_key.unset();

        auto result = read_environment();
        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error().message,
                  "required environment variable OPENAI_API_KEY is not set");
    }

    TEST_CASE(perform_http_request_reports_invalid_url) {
        auto result = perform_http_request("://invalid-url", "test-key", R"({"x":1})");
        EXPECT_FALSE(result.has_value());
        EXPECT_NE(result.error().message.find("curl"), std::string::npos);
    }
};
