#include <filesystem>

#include "kota/zest/zest.h"

import config;

using namespace clore::config;

namespace {

namespace fs = std::filesystem;

}  // namespace

TEST_SUITE(config_normalize) {

TEST_CASE(normalize_paths) {
    TaskConfig config;
    config.compile_commands_path = "build/compile_commands.json";
    config.project_root = ".";
    config.output_root = "docs/output";

    auto result = normalize(config);
    ASSERT_TRUE(result.has_value());

    EXPECT_TRUE(fs::path(config.compile_commands_path).is_absolute());
    EXPECT_TRUE(fs::path(config.project_root).is_absolute());
    EXPECT_TRUE(fs::path(config.output_root).is_absolute());
    EXPECT_TRUE(fs::path(config.workspace_root).is_absolute());
}

TEST_CASE(normalize_backslashes) {
    TaskConfig config;
    config.compile_commands_path = "workspace\\build\\compile_commands.json";
    config.project_root = "workspace\\project";
    config.output_root = "workspace\\output";

    auto result = normalize(config);
    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(config.compile_commands_path.find('\\'), std::string::npos);
    EXPECT_EQ(config.project_root.find('\\'), std::string::npos);
    EXPECT_EQ(config.output_root.find('\\'), std::string::npos);
    EXPECT_EQ(config.workspace_root.find('\\'), std::string::npos);
}

TEST_CASE(normalize_filter_patterns_backslashes) {
    TaskConfig config;
    config.compile_commands_path = "workspace\\build\\compile_commands.json";
    config.project_root = "workspace\\project";
    config.output_root = "workspace\\output";
    config.filter.include = {"src\\", "include\\api\\"};
    config.filter.exclude = {"build\\Debug\\_deps\\"};

    auto result = normalize(config);
    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(config.filter.include[0].find('\\'), std::string::npos);
    EXPECT_EQ(config.filter.include[1].find('\\'), std::string::npos);
    EXPECT_EQ(config.filter.exclude[0].find('\\'), std::string::npos);
}

TEST_CASE(normalize_rejects_empty_compile_commands) {
    TaskConfig config;
    config.compile_commands_path = "";
    config.project_root = "/some/path";
    config.output_root = "/some/out";

    auto result = normalize(config);
    EXPECT_FALSE(result.has_value());
}

TEST_CASE(normalize_rejects_empty_project_root) {
    TaskConfig config;
    config.compile_commands_path = "/some/compile_commands.json";
    config.project_root = "";
    config.output_root = "/some/out";

    auto result = normalize(config);
    EXPECT_FALSE(result.has_value());
}

TEST_CASE(normalize_rejects_empty_output_root) {
    TaskConfig config;
    config.compile_commands_path = "/some/compile_commands.json";
    config.project_root = "/some/path";
    config.output_root = "";

    auto result = normalize(config);
    EXPECT_FALSE(result.has_value());
}

};  // TEST_SUITE(config_normalize)
