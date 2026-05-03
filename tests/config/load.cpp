#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

#include "kota/zest/zest.h"

import config;

using namespace clore::config;

namespace {

constexpr auto kMinimalValidConfig = R"(
[llm]
system_prompt = "You are a documentation writer."
retry_limit = 3
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

TEST_SUITE(config_load) {

TEST_CASE(load_minimal_valid) {
    auto result = load_config_from_string(kMinimalValidConfig);
    ASSERT_TRUE(result.has_value());
    auto& config = *result;
    EXPECT_EQ(config.llm.retry_limit, 3u);
}

TEST_CASE(load_requires_required_sections) {
    auto result = load_config_from_string(R"(
[filter]
include = ["src/"]
)");
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
)");
    EXPECT_FALSE(result.has_value());
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().message, "unknown configuration key 'compile_commands_path'");
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

TEST_CASE(load_rejects_removed_page_types_section) {
    auto result = load_config_from_string(R"(
[page_types]
index = true
)");
    EXPECT_FALSE(result.has_value());
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().message, "unknown configuration key 'page_types'");
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
)");
    EXPECT_FALSE(result.has_value());
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().message, "unknown configuration key 'validation'");
}

TEST_CASE(load_rejects_removed_navigation_section) {
    auto result = load_config_from_string(std::string(kMinimalValidConfig) + R"(
[navigation]
consume_dependency_summaries = true
)");
    EXPECT_FALSE(result.has_value());
}

TEST_CASE(load_rejects_removed_builtin_section) {
    auto result = load_config_from_string(std::string(kMinimalValidConfig) + R"(
[builtin]
vitepress = true
)");
    EXPECT_FALSE(result.has_value());
}

TEST_CASE(load_rejects_removed_workflow_rules_section) {
    auto result = load_config_from_string(std::string(kMinimalValidConfig) + R"(
[workflow_rules]
min_chain_symbols = 2
)");
    EXPECT_FALSE(result.has_value());
}

TEST_CASE(load_filter_symbols_section) {
    auto toml = std::string(kMinimalValidConfig) + R"(
[filter]
include = []
exclude = []

[filter.symbols]
hide_private = true
hide_protected = false
exclude_name_prefixes = ["_impl_", "_detail_"]
exclude_namespace_segments = ["detail", "internal"]
)";
    auto result = load_config_from_string(toml);
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->filter.symbols.has_value());
    auto& sym = *result->filter.symbols;
    EXPECT_TRUE(sym.hide_private);
    EXPECT_FALSE(sym.hide_protected);
    ASSERT_EQ(sym.exclude_name_prefixes.size(), 2u);
    EXPECT_EQ(sym.exclude_name_prefixes[0], "_impl_");
    EXPECT_EQ(sym.exclude_name_prefixes[1], "_detail_");
    ASSERT_EQ(sym.exclude_namespace_segments.size(), 2u);
    EXPECT_EQ(sym.exclude_namespace_segments[0], "detail");
    EXPECT_EQ(sym.exclude_namespace_segments[1], "internal");
}

TEST_CASE(validate_rejects_empty_prefix) {
    auto toml = std::string(kMinimalValidConfig) + R"(
[filter]
include = []
exclude = []

[filter.symbols]
exclude_name_prefixes = [""]
)";
    auto result = load_config_from_string(toml);
    EXPECT_FALSE(result.has_value());
}

};  // TEST_SUITE(config_load)
