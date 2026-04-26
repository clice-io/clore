export module config:validate;

import std;
import :schema;

export namespace clore::config {

struct ValidationError {
    std::string message;
};

auto validate(const TaskConfig& config) -> std::expected<void, ValidationError>;

}  // namespace clore::config

// ── implementation ──────────────────────────────────────────────────

namespace clore::config {

namespace {

auto validate_nonempty(const std::string& value, std::string_view field)
    -> std::expected<void, ValidationError> {
    if(value.empty()) {
        return std::unexpected(
            ValidationError{.message = std::format("{} is required and must not be empty", field)});
    }
    return {};
}

auto validate_nonzero(std::uint32_t value, std::string_view field)
    -> std::expected<void, ValidationError> {
    if(value == 0) {
        return std::unexpected(ValidationError{
            .message = std::format("{} is required and must be greater than 0", field)});
    }
    return {};
}

}  // namespace

auto validate(const TaskConfig& config) -> std::expected<void, ValidationError> {
    namespace fs = std::filesystem;

    if(config.compile_commands_path.empty()) {
        return std::unexpected(ValidationError{.message = "compile_commands_path is required"});
    }
    if(!fs::exists(config.compile_commands_path)) {
        return std::unexpected(
            ValidationError{.message = std::format("compile_commands_path does not exist: {}",
                                                   config.compile_commands_path)});
    }
    if(!fs::is_regular_file(config.compile_commands_path)) {
        return std::unexpected(ValidationError{
            .message = std::format("compile_commands_path is not a regular file: {}",
                                   config.compile_commands_path)});
    }

    if(config.project_root.empty()) {
        return std::unexpected(ValidationError{.message = "project_root is required"});
    }
    if(!fs::exists(config.project_root)) {
        return std::unexpected(ValidationError{
            .message = std::format("project_root does not exist: {}", config.project_root)});
    }
    if(!fs::is_directory(config.project_root)) {
        return std::unexpected(ValidationError{
            .message = std::format("project_root is not a directory: {}", config.project_root)});
    }

    if(config.output_root.empty()) {
        return std::unexpected(ValidationError{.message = "output_root is required"});
    }
    if(fs::exists(config.output_root) && !fs::is_directory(config.output_root)) {
        return std::unexpected(
            ValidationError{.message = std::format("output_root exists but is not a directory: {}",
                                                   config.output_root)});
    }

    // Validate LLM config
    if(auto r = validate_nonempty(config.llm.system_prompt, "llm.system_prompt"); !r)
        return r;
    if(auto r = validate_nonzero(config.llm.retry_limit, "llm.retry_limit"); !r)
        return r;

    return {};
}

}  // namespace clore::config
