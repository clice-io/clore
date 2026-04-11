#include "eventide/zest/zest.h"

#include <filesystem>
#include <fstream>

import config;

using namespace clore::config;

namespace {

// Minimal valid config with all required sections for tests that need a valid parse.
constexpr auto kMinimalValidConfig = R"(
[page_types]
index = true
module_page = true
namespace_page = true
type_page = true
file_page = true

[path_rules]
index_path = "index.md"
module_prefix = "modules"
namespace_prefix = "namespaces"
type_prefix = "types"
file_prefix = "files"
name_normalize = "lowercase"

[prompt_templates]
type_overview = "prompts/type_overview.txt"
type_usage_notes = "prompts/type_usage_notes.txt"
namespace_summary = "prompts/namespace_summary.txt"
module_summary = "prompts/module_summary.txt"
module_architecture = "prompts/module_architecture.txt"
repository_overview = "prompts/repository_overview.txt"
reading_guide = "prompts/reading_guide.txt"

[page_templates]
index = "pages/index.md"
module_page = "pages/module.md"
namespace_page = "pages/namespace.md"
type_page = "pages/type.md"
file_page = "pages/file.md"

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

[validation]
fail_on_empty_section = true
fail_on_h1_in_output = true

[navigation]
consume_dependency_summaries = true
)";

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
        EXPECT_TRUE(config.page_types.index);
        EXPECT_EQ(config.path_rules.name_normalize, "lowercase");
        EXPECT_EQ(config.llm.retry_count, 3u);
        EXPECT_EQ(config.llm.retry_initial_backoff_ms, 250u);
        EXPECT_EQ(config.evidence_rules.max_callers, 5u);
    }

    TEST_CASE(load_requires_page_types_section) {
        auto result = load_config_from_string(R"(
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
};
