module;

#include <expected>
#include <filesystem>
#include <format>
#include <string>

export module clore.config:validate;

import :schema;

export namespace clore::config {

struct ValidationError {
    std::string message;
};

auto validate(const TaskConfig& config) -> std::expected<void, ValidationError>;

}  // namespace clore::config

// ── implementation ──────────────────────────────────────────────────

namespace clore::config {

auto validate(const TaskConfig& config) -> std::expected<void, ValidationError> {
    namespace fs = std::filesystem;

    if(config.compile_commands_path.empty()) {
        return std::unexpected(ValidationError{
            .message = "compile_commands_path is required"});
    }
    if(!fs::exists(config.compile_commands_path)) {
        return std::unexpected(ValidationError{
            .message = std::format("compile_commands_path does not exist: {}",
                                   config.compile_commands_path)});
    }
    if(!fs::is_regular_file(config.compile_commands_path)) {
        return std::unexpected(ValidationError{
            .message = std::format("compile_commands_path is not a regular file: {}",
                                   config.compile_commands_path)});
    }

    if(config.project_root.empty()) {
        return std::unexpected(ValidationError{
            .message = "project_root is required"});
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
        return std::unexpected(ValidationError{
            .message = "output_root is required"});
    }
    if(fs::exists(config.output_root) && !fs::is_directory(config.output_root)) {
        return std::unexpected(ValidationError{
            .message = std::format("output_root exists but is not a directory: {}",
                                   config.output_root)});
    }

    if(!config.extract.max_snippet_bytes.has_value()) {
        return std::unexpected(ValidationError{
            .message = "extract.max_snippet_bytes is required"});
    }

    if(config.language.has_value() && config.language->empty()) {
        return std::unexpected(ValidationError{
            .message = "clore.language must not be empty"});
    }

    if(config.frontmatter.template_path.has_value()) {
        auto& tmpl = *config.frontmatter.template_path;
        if(!fs::exists(tmpl)) {
            return std::unexpected(ValidationError{
                .message = std::format("frontmatter template_path does not exist: {}", tmpl)});
        }
    }

    return {};
}

}  // namespace clore::config
