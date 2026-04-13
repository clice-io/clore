module;

#include <expected>
#include <filesystem>
#include <format>
#include <string>

export module config:validate;

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

auto validate_file_exists(const std::string& path, std::string_view field)
    -> std::expected<void, ValidationError> {
    namespace fs = std::filesystem;
    if(path.empty()) {
        return std::unexpected(ValidationError{
            .message = std::format("{} is required and must not be empty", field)});
    }
    if(!fs::exists(path)) {
        return std::unexpected(ValidationError{
            .message = std::format("{} does not exist: {}", field, path)});
    }
    if(!fs::is_regular_file(path)) {
        return std::unexpected(ValidationError{
            .message = std::format("{} is not a regular file: {}", field, path)});
    }
    return {};
}

auto validate_nonempty(const std::string& value, std::string_view field)
    -> std::expected<void, ValidationError> {
    if(value.empty()) {
        return std::unexpected(ValidationError{
            .message = std::format("{} is required and must not be empty", field)});
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
        return std::unexpected(ValidationError{
            .message = std::format("output_root exists but is not a directory: {}",
                                   config.output_root)});
    }

    if(!config.extract.max_snippet_bytes.has_value()) {
        return std::unexpected(ValidationError{
            .message = "extract.max_snippet_bytes is required"});
    }
    if(*config.extract.max_snippet_bytes == 0) {
        return std::unexpected(ValidationError{
            .message = "extract.max_snippet_bytes must be greater than 0"});
    }

    // Validate path_rules
    if(auto r = validate_nonempty(config.path_rules.index_path, "path_rules.index_path"); !r) return r;
    if(auto r = validate_nonempty(config.path_rules.module_prefix, "path_rules.module_prefix"); !r) return r;
    if(auto r = validate_nonempty(config.path_rules.namespace_prefix, "path_rules.namespace_prefix"); !r) return r;
    if(auto r = validate_nonempty(config.path_rules.type_prefix, "path_rules.type_prefix"); !r) return r;
    if(auto r = validate_nonempty(config.path_rules.file_prefix, "path_rules.file_prefix"); !r) return r;
    if(auto r = validate_nonempty(config.path_rules.workflow_prefix, "path_rules.workflow_prefix"); !r) return r;
    if(auto r = validate_nonempty(config.path_rules.name_normalize, "path_rules.name_normalize"); !r) return r;

    // Validate prompt templates exist when their page types are enabled
    if(config.page_types.type_page) {
        if(auto r = validate_file_exists(config.prompt_templates.type_overview, "prompt_templates.type_overview"); !r) return r;
        if(auto r = validate_file_exists(config.prompt_templates.type_usage_notes, "prompt_templates.type_usage_notes"); !r) return r;
    }
    if(config.page_types.namespace_page) {
        if(auto r = validate_file_exists(config.prompt_templates.namespace_summary, "prompt_templates.namespace_summary"); !r) return r;
    }
    if(config.page_types.module_page) {
        if(auto r = validate_file_exists(config.prompt_templates.module_summary, "prompt_templates.module_summary"); !r) return r;
        if(auto r = validate_file_exists(config.prompt_templates.module_architecture, "prompt_templates.module_architecture"); !r) return r;
    }
    if(config.page_types.index) {
        if(auto r = validate_file_exists(config.prompt_templates.index_overview, "prompt_templates.index_overview"); !r) return r;
        if(auto r = validate_file_exists(config.prompt_templates.index_reading_guide, "prompt_templates.index_reading_guide"); !r) return r;
    }
    if(config.page_types.workflow_page) {
        if(auto r = validate_file_exists(config.prompt_templates.workflow, "prompt_templates.workflow"); !r) return r;
    }

    // Validate page templates exist when their page types are enabled
    if(config.page_types.index) {
        if(auto r = validate_file_exists(config.page_templates.index, "page_templates.index"); !r) return r;
    }
    if(config.page_types.module_page) {
        if(auto r = validate_file_exists(config.page_templates.module_page, "page_templates.module_page"); !r) return r;
    }
    if(config.page_types.namespace_page) {
        if(auto r = validate_file_exists(config.page_templates.namespace_page, "page_templates.namespace_page"); !r) return r;
    }
    if(config.page_types.type_page) {
        if(auto r = validate_file_exists(config.page_templates.type_page, "page_templates.type_page"); !r) return r;
    }
    if(config.page_types.file_page) {
        if(auto r = validate_file_exists(config.page_templates.file_page, "page_templates.file_page"); !r) return r;
    }
    if(config.page_types.workflow_page) {
        if(auto r = validate_file_exists(config.page_templates.workflow_page, "page_templates.workflow_page"); !r) return r;
    }

    // Validate evidence rules
    if(auto r = validate_nonzero(config.evidence_rules.max_callers, "evidence_rules.max_callers"); !r) return r;
    if(auto r = validate_nonzero(config.evidence_rules.max_callees, "evidence_rules.max_callees"); !r) return r;
    if(auto r = validate_nonzero(config.evidence_rules.max_siblings, "evidence_rules.max_siblings"); !r) return r;
    if(auto r = validate_nonzero(config.evidence_rules.max_source_bytes, "evidence_rules.max_source_bytes"); !r) return r;
    if(auto r = validate_nonzero(config.evidence_rules.max_related_summaries, "evidence_rules.max_related_summaries"); !r) return r;

    // Validate workflow rules
    if(auto r = validate_nonzero(config.workflow_rules.min_chain_symbols,
                                 "workflow_rules.min_chain_symbols");
       !r) return r;
    if(config.workflow_rules.min_chain_symbols < 2) {
        return std::unexpected(ValidationError{
            .message =
                "workflow_rules.min_chain_symbols must be greater than or equal to 2"});
    }
    if(auto r = validate_nonzero(config.workflow_rules.min_new_symbols,
                                 "workflow_rules.min_new_symbols");
       !r) return r;
    if(auto r = validate_nonzero(config.workflow_rules.max_workflow_pages,
                                 "workflow_rules.max_workflow_pages");
       !r) return r;
    if(auto r = validate_nonzero(
            config.workflow_rules.llm_review_top_k,
            "workflow_rules.llm_review_top_k");
       !r) return r;
    if(auto r = validate_nonzero(
            config.workflow_rules.llm_selected_count,
            "workflow_rules.llm_selected_count");
       !r) return r;
    if(config.workflow_rules.max_symbol_overlap_ratio_percent > 100) {
        return std::unexpected(ValidationError{
            .message =
                "workflow_rules.max_symbol_overlap_ratio_percent must be less than or equal to 100"});
    }
    if(config.workflow_rules.llm_selected_count >
       config.workflow_rules.llm_review_top_k) {
        return std::unexpected(ValidationError{
            .message =
                "workflow_rules.llm_selected_count must be less than or equal to workflow_rules.llm_review_top_k"});
    }

    // Validate LLM config
    if(auto r = validate_nonempty(config.llm.system_prompt, "llm.system_prompt"); !r) return r;
    if(auto r = validate_nonzero(config.llm.retry_count, "llm.retry_count"); !r) return r;
    if(auto r = validate_nonzero(config.llm.retry_initial_backoff_ms, "llm.retry_initial_backoff_ms"); !r) return r;

    // Validate name_normalize is a known strategy
    auto& norm = config.path_rules.name_normalize;
    if(norm != "lowercase" && norm != "identity") {
        return std::unexpected(ValidationError{
            .message = std::format("path_rules.name_normalize must be 'lowercase' or 'identity', got '{}'", norm)});
    }

    return {};
}

}  // namespace clore::config
