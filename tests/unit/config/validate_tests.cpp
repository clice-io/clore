#include "eventide/zest/zest.h"

#include <filesystem>
#include <fstream>

#include "config/config.h"

using namespace clore::config;

TEST_SUITE(config_validate) {
    TEST_CASE(missing_compile_commands_path) {
        TaskConfig config;
        config.project_root = ".";
        config.output_root = "/tmp/out";

        auto result = validate(config);
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(missing_project_root) {
        TaskConfig config;
        config.compile_commands_path = "compile_commands.json";
        config.output_root = "/tmp/out";

        auto result = validate(config);
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(missing_output_root) {
        TaskConfig config;
        config.compile_commands_path = "compile_commands.json";
        config.project_root = ".";

        auto result = validate(config);
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(nonexistent_compile_commands) {
        TaskConfig config;
        config.compile_commands_path = "/nonexistent/compile_commands.json";
        config.project_root = ".";
        config.output_root = "/tmp/out";

        auto result = validate(config);
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(project_root_not_directory) {
        namespace fs = std::filesystem;

        // Create a temp file to test against
        auto temp = fs::temp_directory_path() / "clore_test_file.txt";
        {
            std::ofstream f(temp);
            f << "test";
        }

        TaskConfig config;
        config.compile_commands_path = temp.string();
        config.project_root = temp.string();
        config.output_root = "/tmp/out";

        auto result = validate(config);
        EXPECT_FALSE(result.has_value());

        fs::remove(temp);
    }
};

TEST_SUITE(config_normalize) {
    TEST_CASE(normalize_paths) {
        TaskConfig config;
        config.compile_commands_path = "build/compile_commands.json";
        config.project_root = ".";
        config.output_root = "docs/output";

        auto result = normalize(config);
        ASSERT_TRUE(result.has_value());

        namespace fs = std::filesystem;
        // After normalization, paths should be absolute
        EXPECT_TRUE(fs::path(config.compile_commands_path).is_absolute());
        EXPECT_TRUE(fs::path(config.project_root).is_absolute());
        EXPECT_TRUE(fs::path(config.output_root).is_absolute());
    }

    TEST_CASE(normalize_backslashes) {
        TaskConfig config;
        config.compile_commands_path = "C:\\build\\compile_commands.json";
        config.project_root = "C:\\project";
        config.output_root = "C:\\output";

        auto result = normalize(config);
        ASSERT_TRUE(result.has_value());

        // Backslashes should be converted to forward slashes
        EXPECT_EQ(config.compile_commands_path.find('\\'), std::string::npos);
        EXPECT_EQ(config.project_root.find('\\'), std::string::npos);
        EXPECT_EQ(config.output_root.find('\\'), std::string::npos);
    }

    TEST_CASE(normalize_filter_patterns_backslashes) {
        TaskConfig config;
        config.compile_commands_path = "C:\\build\\compile_commands.json";
        config.project_root = "C:\\project";
        config.output_root = "C:\\output";
        config.filter.include = {"src\\", "include\\api\\"};
        config.filter.exclude = {"build\\Debug\\_deps\\"};

        auto result = normalize(config);
        ASSERT_TRUE(result.has_value());

        EXPECT_EQ(config.filter.include[0].find('\\'), std::string::npos);
        EXPECT_EQ(config.filter.include[1].find('\\'), std::string::npos);
        EXPECT_EQ(config.filter.exclude[0].find('\\'), std::string::npos);
    }

    // CRITICAL: empty required paths must be rejected before fs::absolute,
    // which would silently resolve "" to cwd and bypass required-field validation.
    TEST_CASE(normalize_rejects_empty_compile_commands) {
        TaskConfig config;
        config.compile_commands_path = "";  // empty
        config.project_root = "/some/path";
        config.output_root = "/some/out";

        auto result = normalize(config);
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(normalize_rejects_empty_project_root) {
        TaskConfig config;
        config.compile_commands_path = "/some/compile_commands.json";
        config.project_root = "";  // empty
        config.output_root = "/some/out";

        auto result = normalize(config);
        EXPECT_FALSE(result.has_value());
    }

    TEST_CASE(normalize_rejects_empty_output_root) {
        TaskConfig config;
        config.compile_commands_path = "/some/compile_commands.json";
        config.project_root = "/some/path";
        config.output_root = "";  // empty

        auto result = normalize(config);
        EXPECT_FALSE(result.has_value());
    }
};

TEST_SUITE(config_validate_extra) {
    TEST_CASE(compile_commands_must_be_regular_file) {
        namespace fs = std::filesystem;

        // A directory is not a regular file
        auto temp_dir = fs::temp_directory_path() / "clore_test_dir_ccdb";
        fs::create_directories(temp_dir);

        TaskConfig config;
        config.compile_commands_path = temp_dir.string();
        config.project_root = temp_dir.string();
        config.output_root = "/tmp/out";
        config.extract.max_snippet_bytes = 2048;

        auto result = validate(config);
        EXPECT_FALSE(result.has_value());

        fs::remove_all(temp_dir);
    }

    TEST_CASE(output_root_must_be_directory_if_exists) {
        namespace fs = std::filesystem;

        auto temp = fs::temp_directory_path() / "clore_test_output_file";
        {
            std::ofstream f(temp);
            f << "not a directory";
        }

        TaskConfig config;
        config.compile_commands_path = temp.string();  // doesn't matter for this check
        config.project_root = "/some/path";
        config.output_root = temp.string();  // exists but is a file
        config.extract.max_snippet_bytes = 2048;

        auto result = validate(config);
        EXPECT_FALSE(result.has_value());

        fs::remove(temp);
    }

    TEST_CASE(max_snippet_bytes_required) {
        namespace fs = std::filesystem;

        auto temp_file = fs::temp_directory_path() / "clore_cc.json";
        {
            std::ofstream f(temp_file);
            f << "[]";
        }
        auto temp_dir = fs::temp_directory_path() / "clore_proj";
        fs::create_directories(temp_dir);

        TaskConfig config;
        config.compile_commands_path = temp_file.string();
        config.project_root = temp_dir.string();
        config.output_root = temp_dir.string();
        // max_snippet_bytes intentionally not set

        auto result = validate(config);
        EXPECT_FALSE(result.has_value());

        fs::remove(temp_file);
        fs::remove_all(temp_dir);
    }
};
