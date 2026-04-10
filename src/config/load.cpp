#include "config/load.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "eventide/serde/toml/toml.h"
#include <toml++/toml.hpp>
#include "support/logging.h"

namespace clore::config {

namespace {

struct RawFrontmatterField {
    std::string key;
    std::string value;
};

struct RawFrontmatterConfig {
    std::vector<RawFrontmatterField> fields;
    std::optional<std::string> template_path;
};

struct RawPageRule {
    std::string pattern;
    std::string layout;
};

struct RawFilterRule {
    std::vector<std::string> include;
    std::vector<std::string> exclude;
};

struct RawCloreConfig {
    std::optional<std::string> language;
};

struct RawTaskConfig {
    std::optional<RawCloreConfig> clore;
    std::optional<RawFilterRule> filter;
    std::optional<RawFrontmatterConfig> frontmatter;
    std::optional<std::vector<RawPageRule>> page_rules;
    std::optional<std::string> log_level;
};

auto to_config(RawTaskConfig&& raw) -> TaskConfig {
    TaskConfig cfg;

    if(raw.clore.has_value()) {
        cfg.language = std::move(raw.clore->language);
    }

    if(raw.filter.has_value()) {
        cfg.filter.include = std::move(raw.filter->include);
        cfg.filter.exclude = std::move(raw.filter->exclude);
    }

    if(raw.frontmatter.has_value()) {
        cfg.frontmatter.template_path = std::move(raw.frontmatter->template_path);
        for(auto& f : raw.frontmatter->fields) {
            cfg.frontmatter.fields.push_back(
                FrontmatterField{.key = std::move(f.key), .value = std::move(f.value)});
        }
    }

    if(raw.page_rules.has_value()) {
        for(auto& r : *raw.page_rules) {
            cfg.page_rules.push_back(
                PageRule{.pattern = std::move(r.pattern), .layout = std::move(r.layout)});
        }
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
        "max_snippet_bytes",
        "extract",
        "llm",
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
    return load_config_from_string(ss.str());
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
