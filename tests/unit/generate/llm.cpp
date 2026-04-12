#include "eventide/zest/zest.h"

#include <cstdint>
#include <cstdlib>
#include <expected>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "llvm/Support/JSON.h"

import generate;
import support;

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
        auto json = detail::build_request_json("gpt-5.2", "You are a writer.", "Document this function.");

        EXPECT_NE(json.find(R"("model":"gpt-5.2")"), std::string::npos);
        EXPECT_NE(json.find("Document this function."), std::string::npos);
    }

    TEST_CASE(build_request_json_preserves_utf8_content) {
        auto json = detail::build_request_json(
            "deepseek-chat",
            "你是一名中文文档作者。",
            "请说明 `LLMClient` 的职责。"
        );

        auto parsed = llvm::json::parse(json);
        ASSERT_TRUE(static_cast<bool>(parsed));

        auto* root = parsed->getAsObject();
        ASSERT_TRUE(root != nullptr);
        EXPECT_EQ(root->getString("model").value_or(""), "deepseek-chat");

        auto* messages = root->getArray("messages");
        ASSERT_TRUE(messages != nullptr);
        ASSERT_EQ(messages->size(), 2u);

        auto* system_message = (*messages)[0].getAsObject();
        auto* user_message = (*messages)[1].getAsObject();
        ASSERT_TRUE(system_message != nullptr);
        ASSERT_TRUE(user_message != nullptr);
        EXPECT_EQ(system_message->getString("content").value_or(""), "你是一名中文文档作者。");
        EXPECT_EQ(user_message->getString("content").value_or(""), "请说明 `LLMClient` 的职责。");
    }

    TEST_CASE(build_request_json_repairs_invalid_utf8_content) {
        std::string invalid_prompt = "bad";
        invalid_prompt.push_back(static_cast<char>(0xFF));
        invalid_prompt += "prompt";

        auto json = detail::build_request_json("deepseek-chat", "系统提示", invalid_prompt);

        auto parsed = llvm::json::parse(json);
        ASSERT_TRUE(static_cast<bool>(parsed));

        auto* root = parsed->getAsObject();
        ASSERT_TRUE(root != nullptr);
        auto* messages = root->getArray("messages");
        ASSERT_TRUE(messages != nullptr);
        auto* user_message = (*messages)[1].getAsObject();
        ASSERT_TRUE(user_message != nullptr);
        EXPECT_EQ(user_message->getString("content").value_or(""),
                  clore::support::ensure_utf8(invalid_prompt));
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

    TEST_CASE(parse_response_supports_utf8_content) {
        auto result = detail::parse_response(
            R"({"choices":[{"message":{"content":"中文摘要：负责生成文档。"}}]})");

        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, "中文摘要：负责生成文档。");
    }

    TEST_CASE(parse_response_supports_content_parts) {
        auto result = detail::parse_response(
            R"({"choices":[{"message":{"content":[{"type":"text","text":"第一段。"},{"type":"output_text","text":"第二段。"}]}}]})");

        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, "第一段。第二段。");
    }

    TEST_CASE(call_llm_requires_openai_base_url_env) {
        ScopedEnvVar base_url("OPENAI_BASE_URL");
        ScopedEnvVar api_key("OPENAI_API_KEY");

        base_url.unset();
        api_key.set("test-key");

        auto result = call_llm("gpt-5.2", "system", "ping");

        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error().message,
                  "required environment variable OPENAI_BASE_URL is not set");
    }

    TEST_CASE(call_llm_requires_openai_api_key_env) {
        ScopedEnvVar base_url("OPENAI_BASE_URL");
        ScopedEnvVar api_key("OPENAI_API_KEY");

        base_url.set("https://example.invalid/v1");
        api_key.unset();

        auto result = call_llm("gpt-5.2", "system", "ping");

        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error().message,
                  "required environment variable OPENAI_API_KEY is not set");
    }

    TEST_CASE(llm_client_requires_openai_base_url_env) {
        ScopedEnvVar base_url("OPENAI_BASE_URL");
        ScopedEnvVar api_key("OPENAI_API_KEY");

        base_url.unset();
        api_key.set("test-key");

        LLMClient client("gpt-5.2", "system", 4, 3, 250);
        (void)client.submit(0, "ping");

        auto result = client.run([](std::uint64_t, auto) {});

        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error().message,
                  "required environment variable OPENAI_BASE_URL is not set");
    }

    TEST_CASE(llm_client_requires_openai_api_key_env) {
        ScopedEnvVar base_url("OPENAI_BASE_URL");
        ScopedEnvVar api_key("OPENAI_API_KEY");

        base_url.set("https://example.invalid/v1");
        api_key.unset();

        LLMClient client("gpt-5.2", "system", 4, 3, 250);
        (void)client.submit(0, "ping");

        auto result = client.run([](std::uint64_t, auto) {});

        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error().message,
                  "required environment variable OPENAI_API_KEY is not set");
    }

    TEST_CASE(llm_client_run_with_no_work_succeeds) {
        LLMClient client("gpt-5.2", "system", 4, 3, 250);

        // run() with nothing submitted should return immediately
        auto result = client.run([](std::uint64_t, auto) {});

        // No pending work → no env required, early exit
        EXPECT_TRUE(result.has_value());
    }

    TEST_CASE(llm_client_rejects_zero_max_concurrent) {
        LLMClient client("gpt-5.2", "system", 0, 3, 250);

        auto submit_result = client.submit(0, "ping");
        EXPECT_FALSE(submit_result.has_value());
        EXPECT_EQ(submit_result.error().message,
                  "max_concurrent must be greater than 0");

        auto run_result = client.run([](std::uint64_t, auto) {});

        EXPECT_FALSE(run_result.has_value());
        EXPECT_EQ(run_result.error().message,
                  "max_concurrent must be greater than 0");
    }
};
