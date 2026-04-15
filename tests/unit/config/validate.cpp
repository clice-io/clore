#include "eventide/zest/zest.h"

#include <filesystem>
#include <fstream>

import config;

using namespace clore::config;

namespace {

namespace fs = std::filesystem;

auto fill_required_generation_fields(TaskConfig& config) -> void {
    config.extract.max_snippet_bytes = 512;

    config.evidence_rules.max_callers = 2;
    config.evidence_rules.max_callees = 2;
    config.evidence_rules.max_siblings = 2;
    config.evidence_rules.max_source_bytes = 1024;
    config.evidence_rules.max_related_summaries = 2;
    config.evidence_rules.max_top_modules = 2;
    config.evidence_rules.max_top_namespaces = 2;

    config.llm.system_prompt = "system";
    config.llm.retry_count = 1;
    config.llm.retry_initial_backoff_ms = 1;
}

}  // namespace

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
};

TEST_SUITE(config_validate_extra) {
    TEST_CASE(compile_commands_must_be_regular_file) {
        auto temp_dir = fs::temp_directory_path() / "clore_test_dir_ccdb";
        fs::create_directories(temp_dir);

        TaskConfig config;
        config.compile_commands_path = temp_dir.string();
        config.project_root = temp_dir.string();
        config.output_root = "/tmp/out";
        fill_required_generation_fields(config);

        auto result = validate(config);
        EXPECT_FALSE(result.has_value());

        fs::remove_all(temp_dir);
    }

    TEST_CASE(output_root_must_be_directory_if_exists) {
        auto temp_root = fs::temp_directory_path() / "clore_validate_output_root";
        fs::remove_all(temp_root);
        fs::create_directories(temp_root);

        auto compile_commands = temp_root / "compile_commands.json";
        {
            std::ofstream f(compile_commands);
            f << "[]";
        }

        auto output_file = temp_root / "output.txt";
        {
            std::ofstream f(output_file);
            f << "not a directory";
        }

        TaskConfig config;
        config.compile_commands_path = compile_commands.string();
        config.project_root = temp_root.string();
        config.output_root = output_file.string();
        fill_required_generation_fields(config);

        auto result = validate(config);
        EXPECT_FALSE(result.has_value());

        fs::remove_all(temp_root);
    }

    TEST_CASE(max_snippet_bytes_required) {
        auto temp_root = fs::temp_directory_path() / "clore_validate_missing_snippet_bytes";
        fs::remove_all(temp_root);
        fs::create_directories(temp_root);

        auto compile_commands = temp_root / "compile_commands.json";
        {
            std::ofstream f(compile_commands);
            f << "[]";
        }

        TaskConfig config;
        config.compile_commands_path = compile_commands.string();
        config.project_root = temp_root.string();
        config.output_root = temp_root.string();
        fill_required_generation_fields(config);
        config.extract.max_snippet_bytes.reset();

        auto result = validate(config);
        EXPECT_FALSE(result.has_value());

        fs::remove_all(temp_root);
    }

    TEST_CASE(max_snippet_bytes_must_be_positive) {
        auto temp_root = fs::temp_directory_path() / "clore_validate_positive_snippet_bytes";
        fs::remove_all(temp_root);
        fs::create_directories(temp_root);

        auto compile_commands = temp_root / "compile_commands.json";
        {
            std::ofstream f(compile_commands);
            f << "[]";
        }

        TaskConfig config;
        config.compile_commands_path = compile_commands.string();
        config.project_root = temp_root.string();
        config.output_root = temp_root.string();
        fill_required_generation_fields(config);
        config.extract.max_snippet_bytes = 0;

        auto result = validate(config);
        EXPECT_FALSE(result.has_value());

        fs::remove_all(temp_root);
    }

    TEST_CASE(validate_accepts_builtin_generation_config_without_templates) {
        auto temp_root = fs::temp_directory_path() / "clore_validate_builtin_config";
        fs::remove_all(temp_root);
        fs::create_directories(temp_root);

        auto compile_commands = temp_root / "compile_commands.json";
        {
            std::ofstream f(compile_commands);
            f << "[]";
        }

        TaskConfig config;
        config.compile_commands_path = compile_commands.string();
        config.project_root = temp_root.string();
        config.output_root = temp_root.string();
        fill_required_generation_fields(config);

        auto result = validate(config);
        EXPECT_TRUE(result.has_value());

        fs::remove_all(temp_root);
    }

};
