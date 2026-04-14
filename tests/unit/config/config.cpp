#include "eventide/zest/zest.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

import config;

using namespace clore::config;

namespace {

constexpr auto kMinimalValidConfig = R"(
[extract]
max_snippet_bytes = 1024

[evidence_rules]
max_callers = 5
max_callees = 5
max_siblings = 8
max_source_bytes = 4096
max_related_summaries = 3

[llm]
system_prompt = "You are a documentation writer."
retry_count = 3
retry_initial_backoff_ms = 250
)";

auto make_valid_config(std::string_view system_prompt) -> std::string {
    auto config = std::string(kMinimalValidConfig);
    auto key = std::string_view{"system_prompt = \""};
    auto value_start = config.find(key);
    if(value_start != std::string::npos) {
        value_start += key.size();
        auto value_end = config.find('"', value_start);
        if(value_end != std::string::npos) {
            config.replace(value_start, value_end - value_start, system_prompt);
        }
    }
    return config;
}

}  // namespace

TEST_SUITE(config_schema) {
    TEST_CASE(default_construction) {
        TaskConfig config;
        EXPECT_TRUE(config.compile_commands_path.empty());
        EXPECT_TRUE(config.project_root.empty());
        EXPECT_TRUE(config.output_root.empty());
        EXPECT_TRUE(config.workspace_root.empty());
        EXPECT_TRUE(config.filter.include.empty());
        EXPECT_TRUE(config.filter.exclude.empty());
        EXPECT_FALSE(config.log_level.has_value());
    }
};

TEST_SUITE(config_load) {
    TEST_CASE(load_minimal_valid) {
        auto result = load_config_from_string(kMinimalValidConfig);
        ASSERT_TRUE(result.has_value());
        auto& config = *result;
        ASSERT_TRUE(config.extract.max_snippet_bytes.has_value());
        EXPECT_EQ(*config.extract.max_snippet_bytes, 1024u);
        EXPECT_EQ(config.llm.retry_count, 3u);
        EXPECT_EQ(config.llm.retry_initial_backoff_ms, 250u);
        EXPECT_EQ(config.evidence_rules.max_callers, 5u);
        EXPECT_EQ(config.evidence_rules.max_related_summaries, 3u);
    }

    TEST_CASE(load_requires_required_sections) {
        auto result = load_config_from_string(R"(
[filter]
include = ["src/"]
)" );
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(load_from_string_with_filter) {
        auto toml = std::string(kMinimalValidConfig) + R"(
[filter]
include = ["src/.*"]
exclude = ["test/.*", "build/.*"]
)";
        auto result = load_config_from_string(toml);
        ASSERT_TRUE(result.has_value());
        auto& config = *result;
        ASSERT_EQ(config.filter.include.size(), 1u);
        EXPECT_EQ(config.filter.include[0], "src/.*");
        ASSERT_EQ(config.filter.exclude.size(), 2u);
        EXPECT_EQ(config.filter.exclude[0], "test/.*");
        EXPECT_EQ(config.filter.exclude[1], "build/.*");
    }

    TEST_CASE(load_rejects_compile_commands_path) {
        auto result = load_config_from_string(R"(
compile_commands_path = "/tmp/compile_commands.json"
)" );
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(load_rejects_project_root) {
        auto result = load_config_from_string(R"(
project_root = "/tmp/project"
)" );
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(load_rejects_output_root) {
        auto result = load_config_from_string(R"(
output_root = "/tmp/output"
)" );
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(load_rejects_removed_page_types_section) {
        auto result = load_config_from_string(R"(
[page_types]
index = true
)" );
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(load_from_invalid_toml) {
        auto result = load_config_from_string("this is not valid toml {{{}}}");
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(load_from_nonexistent_file) {
        auto result = load_config("/nonexistent/path/config.toml");
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(load_config_sets_workspace_root_from_file_location) {
        namespace fs = std::filesystem;

        auto temp_dir = fs::temp_directory_path() / "clore_config_workspace_root";
        fs::remove_all(temp_dir);
        fs::create_directories(temp_dir);

        auto config_path = temp_dir / "clore.toml";
        {
            std::ofstream f(config_path);
            f << kMinimalValidConfig;
        }

        auto result = load_config(config_path.string());
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(fs::path(result->workspace_root).lexically_normal().generic_string(),
                  temp_dir.lexically_normal().generic_string());

        fs::remove_all(temp_dir);
    }

    TEST_CASE(load_from_string_supports_utf8_bom_and_unicode_system_prompt) {
        auto toml = std::string("\xEF\xBB\xBF") + make_valid_config("你是一名中文文档作者。");

        auto result = load_config_from_string(toml);

        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result->llm.system_prompt, "你是一名中文文档作者。");
    }

    TEST_CASE(load_rejects_removed_validation_section) {
        auto result = load_config_from_string(std::string(kMinimalValidConfig) + R"(
[validation]
fail_on_empty_section = true
fail_on_h1_in_output = true
)" );
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(load_rejects_removed_navigation_section) {
        auto result = load_config_from_string(std::string(kMinimalValidConfig) + R"(
[navigation]
consume_dependency_summaries = true
)" );
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(load_rejects_removed_builtin_section) {
        auto result = load_config_from_string(std::string(kMinimalValidConfig) + R"(
[builtin]
vitepress = true
)" );
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(load_rejects_removed_workflow_rules_section) {
        auto result = load_config_from_string(std::string(kMinimalValidConfig) + R"(
[workflow_rules]
min_chain_symbols = 2
)");
        EXPECT_FALSE(result.has_value());
    }
};