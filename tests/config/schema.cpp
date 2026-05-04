#include "kota/zest/zest.h"

import config;

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
    EXPECT_FALSE(config.project.source_base.has_value());
    // log_level is CLI-only, not part of TaskConfig schema
}

};  // TEST_SUITE(config_schema)
