module;

#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "eventide/serde/toml/toml.h"
#include <toml++/toml.hpp>

export module config:load;

import :schema;
import support;

export namespace clore::config {

struct ConfigError {
    std::string message;
};

auto load_config(std::string_view path) -> std::expected<TaskConfig, ConfigError>;

auto load_config_from_string(std::string_view toml_content) -> std::expected<TaskConfig, ConfigError>;

}  // namespace clore::config

// ── implementation ──────────────────────────────────────────────────

namespace clore::config {

namespace {

struct RawFilterRule {
    std::vector<std::string> include;
    std::vector<std::string> exclude;
};

struct RawPageTypesConfig {
    bool repository = false;
    bool index = false;
    bool module_page = false;
    bool namespace_page = false;
    bool type_page = false;
    bool file_page = false;
};

struct RawPathRulesConfig {
    std::string repository_path;
    std::string index_path;
    std::string module_prefix;
    std::string namespace_prefix;
    std::string type_prefix;
    std::string file_prefix;
    std::string name_normalize;
};

struct RawPromptTemplatesConfig {
    std::string type_overview;
    std::string type_usage_notes;
    std::string namespace_summary;
    std::string module_summary;
    std::string module_architecture;
    std::string repository_overview;
    std::string reading_guide;
};

struct RawPageTemplatesConfig {
    std::string repository;
    std::string index;
    std::string module_page;
    std::string namespace_page;
    std::string type_page;
    std::string file_page;
};

struct RawEvidenceRulesConfig {
    std::uint32_t max_callers = 0;
    std::uint32_t max_callees = 0;
    std::uint32_t max_siblings = 0;
    std::uint32_t max_source_bytes = 0;
    std::uint32_t max_related_summaries = 0;
};

struct RawLLMConfig {
    std::string system_prompt;
    std::string failure_marker;
    std::uint32_t max_output_length = 0;
    std::uint32_t max_prompt_length = 0;
};

struct RawValidationConfig {
    bool fail_on_empty_section = false;
    bool fail_on_h1_in_output = false;
};

struct RawNavigationConfig {
    bool consume_dependency_summaries = false;
};

struct RawSectionOrderConfig {
    std::vector<std::string> type_page;
    std::vector<std::string> namespace_page;
    std::vector<std::string> module_page;
    std::vector<std::string> repository_page;
    std::vector<std::string> file_page;
};

struct RawExtractConfig {
    std::optional<std::uint32_t> max_snippet_bytes;
};

struct RawTaskConfig {
    std::optional<RawFilterRule> filter;
    std::optional<RawPageTypesConfig> page_types;
    std::optional<RawPathRulesConfig> path_rules;
    std::optional<RawPromptTemplatesConfig> prompt_templates;
    std::optional<RawPageTemplatesConfig> page_templates;
    std::optional<RawEvidenceRulesConfig> evidence_rules;
    std::optional<RawLLMConfig> llm;
    std::optional<RawValidationConfig> validation;
    std::optional<RawNavigationConfig> navigation;
    std::optional<RawSectionOrderConfig> section_order;
    std::optional<RawExtractConfig> extract;
    std::optional<std::string> log_level;
};

auto to_config(RawTaskConfig&& raw) -> std::expected<TaskConfig, ConfigError> {
    TaskConfig cfg;

    if(raw.filter.has_value()) {
        cfg.filter.include = std::move(raw.filter->include);
        cfg.filter.exclude = std::move(raw.filter->exclude);
    }

    if(raw.extract.has_value()) {
        cfg.extract.max_snippet_bytes = raw.extract->max_snippet_bytes;
    }

    if(!raw.page_types.has_value()) {
        return std::unexpected(ConfigError{.message = "missing required section [page_types]"}); 
    }
    cfg.page_types.repository = raw.page_types->repository;
    cfg.page_types.index = raw.page_types->index;
    cfg.page_types.module_page = raw.page_types->module_page;
    cfg.page_types.namespace_page = raw.page_types->namespace_page;
    cfg.page_types.type_page = raw.page_types->type_page;
    cfg.page_types.file_page = raw.page_types->file_page;

    if(!raw.path_rules.has_value()) {
        return std::unexpected(ConfigError{.message = "missing required section [path_rules]"});
    }
    cfg.path_rules.repository_path = std::move(raw.path_rules->repository_path);
    cfg.path_rules.index_path = std::move(raw.path_rules->index_path);
    cfg.path_rules.module_prefix = std::move(raw.path_rules->module_prefix);
    cfg.path_rules.namespace_prefix = std::move(raw.path_rules->namespace_prefix);
    cfg.path_rules.type_prefix = std::move(raw.path_rules->type_prefix);
    cfg.path_rules.file_prefix = std::move(raw.path_rules->file_prefix);
    cfg.path_rules.name_normalize = std::move(raw.path_rules->name_normalize);

    if(!raw.prompt_templates.has_value()) {
        return std::unexpected(ConfigError{.message = "missing required section [prompt_templates]"});
    }
    cfg.prompt_templates.type_overview = std::move(raw.prompt_templates->type_overview);
    cfg.prompt_templates.type_usage_notes = std::move(raw.prompt_templates->type_usage_notes);
    cfg.prompt_templates.namespace_summary = std::move(raw.prompt_templates->namespace_summary);
    cfg.prompt_templates.module_summary = std::move(raw.prompt_templates->module_summary);
    cfg.prompt_templates.module_architecture = std::move(raw.prompt_templates->module_architecture);
    cfg.prompt_templates.repository_overview = std::move(raw.prompt_templates->repository_overview);
    cfg.prompt_templates.reading_guide = std::move(raw.prompt_templates->reading_guide);

    if(!raw.page_templates.has_value()) {
        return std::unexpected(ConfigError{.message = "missing required section [page_templates]"});
    }
    cfg.page_templates.repository = std::move(raw.page_templates->repository);
    cfg.page_templates.index = std::move(raw.page_templates->index);
    cfg.page_templates.module_page = std::move(raw.page_templates->module_page);
    cfg.page_templates.namespace_page = std::move(raw.page_templates->namespace_page);
    cfg.page_templates.type_page = std::move(raw.page_templates->type_page);
    cfg.page_templates.file_page = std::move(raw.page_templates->file_page);

    if(!raw.evidence_rules.has_value()) {
        return std::unexpected(ConfigError{.message = "missing required section [evidence_rules]"});
    }
    cfg.evidence_rules.max_callers = raw.evidence_rules->max_callers;
    cfg.evidence_rules.max_callees = raw.evidence_rules->max_callees;
    cfg.evidence_rules.max_siblings = raw.evidence_rules->max_siblings;
    cfg.evidence_rules.max_source_bytes = raw.evidence_rules->max_source_bytes;
    cfg.evidence_rules.max_related_summaries = raw.evidence_rules->max_related_summaries;

    if(!raw.llm.has_value()) {
        return std::unexpected(ConfigError{.message = "missing required section [llm]"});
    }
    cfg.llm.system_prompt = std::move(raw.llm->system_prompt);
    cfg.llm.failure_marker = std::move(raw.llm->failure_marker);
    cfg.llm.max_output_length = raw.llm->max_output_length;
    cfg.llm.max_prompt_length = raw.llm->max_prompt_length;

    if(!raw.validation.has_value()) {
        return std::unexpected(ConfigError{.message = "missing required section [validation]"}); 
    }
    cfg.validation.fail_on_empty_section = raw.validation->fail_on_empty_section;
    cfg.validation.fail_on_h1_in_output = raw.validation->fail_on_h1_in_output;

    if(!raw.navigation.has_value()) {
        return std::unexpected(ConfigError{.message = "missing required section [navigation]"});
    }
    cfg.navigation.consume_dependency_summaries = raw.navigation->consume_dependency_summaries;

    if(raw.section_order.has_value()) {
        cfg.section_order.type_page = std::move(raw.section_order->type_page);
        cfg.section_order.namespace_page = std::move(raw.section_order->namespace_page);
        cfg.section_order.module_page = std::move(raw.section_order->module_page);
        cfg.section_order.repository_page = std::move(raw.section_order->repository_page);
        cfg.section_order.file_page = std::move(raw.section_order->file_page);
    }

    cfg.log_level = std::move(raw.log_level);
    return cfg;
}

auto reject_forbidden_keys(std::string_view toml_content) -> std::expected<void, ConfigError> {
    auto parsed = toml::parse(std::string{toml_content});
    if(!parsed) {
        return std::unexpected(ConfigError{
            .message = std::format("TOML parse error: {}", parsed.error().description())});
    }

    auto& table = parsed.table();

    constexpr std::string_view forbidden_top_level_keys[] = {
        "compile_commands_path",
        "project_root",
        "output_root",
    };

    for(auto key : forbidden_top_level_keys) {
        if(table.contains(key)) {
            return std::unexpected(ConfigError{
                .message = std::format("configuration key '{}' is not supported; use CLI arguments instead",
                                       key)});
        }
    }

    return {};
}

}  // namespace

auto load_config(std::string_view path) -> std::expected<TaskConfig, ConfigError> {
    namespace fs = std::filesystem;

    auto config_path = fs::path(path);
    if(config_path.is_relative()) {
        config_path = fs::absolute(config_path);
    }
    config_path = config_path.lexically_normal();

    if(!fs::exists(config_path)) {
        return std::unexpected(ConfigError{
            .message = std::format("configuration file not found: {}", path)});
    }

    std::ifstream file(config_path);
    if(!file.is_open()) {
        return std::unexpected(ConfigError{
            .message = std::format("failed to open configuration file: {}", path)});
    }

    std::ostringstream ss;
    ss << file.rdbuf();

    auto config = load_config_from_string(ss.str());
    if(!config.has_value()) {
        return config;
    }

    config->workspace_root = config_path.parent_path().string();
    return config;
}

auto load_config_from_string(std::string_view toml_content) -> std::expected<TaskConfig, ConfigError> {
    namespace toml = eventide::serde::toml;

    if(auto forbidden = reject_forbidden_keys(toml_content); !forbidden.has_value()) {
        return std::unexpected(std::move(forbidden.error()));
    }

    RawTaskConfig raw{};
    auto result = toml::parse(toml_content, raw);
    if(!result.has_value()) {
        return std::unexpected(ConfigError{
            .message = std::format("TOML parse error: {}", result.error().message())});
    }

    return to_config(std::move(raw));
}

}  // namespace clore::config
