#include <filesystem>
#include <fstream>

#include "kota/zest/zest.h"

import config;

using namespace clore::config;

namespace {

namespace fs = std::filesystem;

auto fill_required_llm_fields(TaskConfig& config) -> void {
    config.llm.provider = "openai";
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
    std::error_code ec;
    auto root = fs::temp_directory_path() / "clore_validate_project_root_not_directory";
    fs::remove_all(root, ec);
    fs::create_directories(root, ec);

    auto compile_commands = root / "compile_commands.json";
    {
        std::ofstream f(compile_commands);
        f << "[]\n";
    }

    auto temp = root / "clore_test_file.txt";
    {
        std::ofstream f(temp);
        f << "test";
    }

    TaskConfig config;
    config.compile_commands_path = compile_commands.string();
    config.project_root = temp.string();
    config.output_root = "/tmp/out";

    auto result = validate(config);
    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("project_root is not a directory"), std::string::npos);

    fs::remove_all(root, ec);
}

};  // TEST_SUITE(config_validate)

TEST_SUITE(config_validate_extra) {

TEST_CASE(compile_commands_must_be_regular_file) {
    auto temp_dir = fs::temp_directory_path() / "clore_test_dir_ccdb";
    fs::create_directories(temp_dir);

    TaskConfig config;
    config.compile_commands_path = temp_dir.string();
    config.project_root = temp_dir.string();
    config.output_root = "/tmp/out";
    fill_required_llm_fields(config);

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
    fill_required_llm_fields(config);

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
    fill_required_llm_fields(config);

    auto result = validate(config);
    EXPECT_TRUE(result.has_value());

    fs::remove_all(temp_root);
}

};  // TEST_SUITE(config_validate_extra)
