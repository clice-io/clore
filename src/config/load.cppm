module;

#include <algorithm>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <optional>
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

struct RawEvidenceRulesConfig {
    std::uint32_t max_callers = 0;
    std::uint32_t max_callees = 0;
    std::uint32_t max_siblings = 0;
    std::uint32_t max_source_bytes = 0;
    std::uint32_t max_related_summaries = 0;
};

struct RawLLMConfig {
    std::string system_prompt;
    std::uint32_t retry_count = 0;
    std::uint32_t retry_initial_backoff_ms = 0;
};

struct RawExtractConfig {
    std::optional<std::uint32_t> max_snippet_bytes;
};

struct RawTaskConfig {
    std::optional<RawFilterRule> filter;
    std::optional<RawEvidenceRulesConfig> evidence_rules;
    std::optional<RawLLMConfig> llm;
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
    cfg.llm.retry_count = raw.llm->retry_count;
    cfg.llm.retry_initial_backoff_ms = raw.llm->retry_initial_backoff_ms;

    cfg.log_level = std::move(raw.log_level);
    return cfg;
}

auto reject_forbidden_keys(const ::toml::table& table) -> std::expected<void, ConfigError> {
    constexpr std::string_view cli_backed_keys[] = {
        "compile_commands_path",
        "project_root",
        "output_root",
    };
    constexpr std::string_view removed_sections[] = {
        "validation",
        "navigation",
        "builtin",
        "workflow_rules",
        "page_types",
        "path_rules",
        "prompt_templates",
        "page_templates",
        "section_order",
    };

    auto contains_key = [](const auto& keys, std::string_view key) {
        for(auto candidate : keys) {
            if(candidate == key) {
                return true;
            }
        }
        return false;
    };

    for(const auto& [key, _] : table) {
        if(key.str().empty()) {
            continue;
        }

        auto key_name = std::string_view{key.str()};
        if(contains_key(cli_backed_keys, key_name)) {
            return std::unexpected(ConfigError{
                .message = std::format("configuration key '{}' is not supported; use CLI arguments instead",
                                       key_name)});
        }
        if(contains_key(removed_sections, key_name)) {
            return std::unexpected(ConfigError{
                .message = std::format("configuration key '{}' is no longer supported", key_name)});
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

    auto content = clore::support::read_utf8_text_file(config_path);
    if(!content.has_value()) {
        return std::unexpected(ConfigError{.message = std::format(
                                   "failed to read configuration file: {}",
                                   content.error())});
    }

    auto config = load_config_from_string(*content);
    if(!config.has_value()) {
        return config;
    }

    config->workspace_root = config_path.parent_path().string();
    return config;
}

auto load_config_from_string(std::string_view toml_content) -> std::expected<TaskConfig, ConfigError> {
    namespace serde_toml = eventide::serde::toml;

    auto normalized_toml = clore::support::strip_utf8_bom(toml_content);

    auto parsed = ::toml::parse(std::string{normalized_toml});
    if(!parsed) {
        return std::unexpected(ConfigError{
            .message = std::format("TOML parse error: {}", parsed.error().description())});
    }

    auto& table = parsed.table();

    if(auto forbidden = reject_forbidden_keys(table); !forbidden.has_value()) {
        return std::unexpected(std::move(forbidden.error()));
    }

    RawTaskConfig raw{};
    auto result = serde_toml::from_toml(table, raw);
    if(!result.has_value()) {
        return std::unexpected(ConfigError{
            .message = std::format("TOML parse error: {}", result.error().message())});
    }

    return to_config(std::move(raw));
}

}  // namespace clore::config
