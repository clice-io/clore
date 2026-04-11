#include "eventide/zest/zest.h"

#include <filesystem>
#include <fstream>

import clore.config;

using namespace clore::config;

TEST_SUITE(config_schema) {
    TEST_CASE(default_construction) {
        TaskConfig config;
        EXPECT_TRUE(config.compile_commands_path.empty());
        EXPECT_TRUE(config.project_root.empty());
        EXPECT_TRUE(config.output_root.empty());
        EXPECT_TRUE(config.workspace_root.empty());
        EXPECT_TRUE(config.filter.include.empty());
        EXPECT_TRUE(config.filter.exclude.empty());
        EXPECT_TRUE(config.frontmatter.fields.empty());
        EXPECT_FALSE(config.frontmatter.template_path.has_value());
        EXPECT_TRUE(config.page_rules.empty());
        EXPECT_FALSE(config.log_level.has_value());
    }
};

TEST_SUITE(config_load) {
    TEST_CASE(load_from_string_basic) {
        auto result = load_config_from_string(R"(
log_level = "debug"
)");

        ASSERT_TRUE(result.has_value());
        auto& config = *result;
        ASSERT_TRUE(config.log_level.has_value());
        EXPECT_EQ(*config.log_level, "debug");
    }

    TEST_CASE(load_from_string_with_language) {
        auto result = load_config_from_string(R"(
[clore]
language = "简体中文"
)");

        ASSERT_TRUE(result.has_value());
        auto& config = *result;
        ASSERT_TRUE(config.language.has_value());
        EXPECT_EQ(*config.language, "简体中文");
    }

    TEST_CASE(load_from_string_with_filter) {
        auto result = load_config_from_string(R"(
[filter]
include = ["src/.*"]
exclude = ["test/.*", "build/.*"]
)");

        ASSERT_TRUE(result.has_value());
        auto& config = *result;
        ASSERT_EQ(config.filter.include.size(), 1u);
        EXPECT_EQ(config.filter.include[0], "src/.*");
        ASSERT_EQ(config.filter.exclude.size(), 2u);
        EXPECT_EQ(config.filter.exclude[0], "test/.*");
        EXPECT_EQ(config.filter.exclude[1], "build/.*");
    }

    TEST_CASE(load_from_string_with_frontmatter) {
        auto result = load_config_from_string(R"(
[frontmatter]
template_path = "/tmp/templates/frontmatter.toml"

[[frontmatter.fields]]
key = "sidebar_position"
value = "1"

[[frontmatter.fields]]
key = "sidebar_label"
value = "API Reference"
)");

        ASSERT_TRUE(result.has_value());
        auto& config = *result;
        ASSERT_TRUE(config.frontmatter.template_path.has_value());
        EXPECT_EQ(*config.frontmatter.template_path, "/tmp/templates/frontmatter.toml");
        ASSERT_EQ(config.frontmatter.fields.size(), 2u);
        EXPECT_EQ(config.frontmatter.fields[0].key, "sidebar_position");
        EXPECT_EQ(config.frontmatter.fields[0].value, "1");
    }

    TEST_CASE(load_from_string_with_page_rules) {
        auto result = load_config_from_string(R"(
[[page_rules]]
pattern = "class"
layout = "class_page"

[[page_rules]]
pattern = "function"
layout = "function_page"
)");

        ASSERT_TRUE(result.has_value());
        auto& config = *result;
        ASSERT_EQ(config.page_rules.size(), 2u);
        EXPECT_EQ(config.page_rules[0].pattern, "class");
        EXPECT_EQ(config.page_rules[0].layout, "class_page");
    }

    TEST_CASE(load_rejects_compile_commands_path) {
        auto result = load_config_from_string(R"(
compile_commands_path = "/tmp/compile_commands.json"
)");
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(load_rejects_project_root) {
        auto result = load_config_from_string(R"(
project_root = "/tmp/project"
)");
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(load_rejects_output_root) {
        auto result = load_config_from_string(R"(
output_root = "/tmp/output"
)");
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(load_rejects_max_snippet_bytes) {
        auto result = load_config_from_string(R"(
max_snippet_bytes = 8192
)");
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(load_rejects_extract_table) {
        auto result = load_config_from_string(R"(
[extract]
max_snippet_bytes = 8192
)");
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(load_rejects_llm_table) {
        auto result = load_config_from_string(R"(
[llm]
model = "gpt-5.2"
)");
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
            f << "log_level = \"info\"\n";
        }

        auto result = load_config(config_path.string());
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(fs::path(result->workspace_root).lexically_normal().generic_string(),
                  temp_dir.lexically_normal().generic_string());

        fs::remove_all(temp_dir);
    }
};
