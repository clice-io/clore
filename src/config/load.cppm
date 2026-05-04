module;

#include "kota/codec/toml/toml.h"

#include <toml++/toml.hpp>

export module config:load;

import std;
import :schema;
import support;

export namespace clore::config {

struct ConfigError {
    std::string message;
};

auto load_config(std::string_view path) -> std::expected<TaskConfig, ConfigError>;

auto load_config_from_string(std::string_view toml_content)
    -> std::expected<TaskConfig, ConfigError>;

}  // namespace clore::config

namespace clore::config {

namespace {

struct RawTaskConfig {
    std::optional<FilterRule> filter;
    std::optional<LLMConfig> llm;
    std::optional<ProjectConfig> project;
};

auto to_config(RawTaskConfig&& raw) -> std::expected<TaskConfig, ConfigError> {
    TaskConfig cfg;

    if(raw.filter.has_value()) {
        cfg.filter = std::move(*raw.filter);
    }

    if(!raw.llm.has_value()) {
        return std::unexpected(ConfigError{.message = "missing required section [llm]"});
    }
    cfg.llm = std::move(*raw.llm);

    if(raw.project.has_value()) {
        cfg.project = std::move(*raw.project);
    }

    return cfg;
}

auto reject_unknown_top_level_keys(const ::toml::table& table) -> std::expected<void, ConfigError> {
    constexpr std::string_view allowed_keys[] = {
        "filter",
        "llm",
        "project",
    };

    auto contains_key = [](const auto& keys, std::string_view key) {
        for(auto candidate: keys) {
            if(candidate == key) {
                return true;
            }
        }
        return false;
    };

    for(const auto& [key, _]: table) {
        if(key.str().empty()) {
            continue;
        }

        auto key_name = std::string_view{key.str()};
        if(!contains_key(allowed_keys, key_name)) {
            return std::unexpected(
                ConfigError{.message = std::format("unknown configuration key '{}'", key_name)});
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
        return std::unexpected(
            ConfigError{.message = std::format("configuration file not found: {}", path)});
    }

    auto content = clore::support::read_utf8_text_file(config_path);
    if(!content.has_value()) {
        return std::unexpected(ConfigError{
            .message = std::format("failed to read configuration file: {}", content.error())});
    }

    auto config = load_config_from_string(*content);
    if(!config.has_value()) {
        return config;
    }

    config->workspace_root = config_path.parent_path().string();
    return config;
}

auto load_config_from_string(std::string_view toml_content)
    -> std::expected<TaskConfig, ConfigError> {
    namespace toml_codec = kota::codec::toml;

    auto normalized_toml = clore::support::strip_utf8_bom(toml_content);

    ::toml::table table;
    try {
        table = ::toml::parse(std::string{normalized_toml});
    } catch(const ::toml::parse_error& err) {
        return std::unexpected(
            ConfigError{.message = std::format("TOML parse error: {}", err.description())});
    }

    if(auto unknown = reject_unknown_top_level_keys(table); !unknown.has_value()) {
        return std::unexpected(std::move(unknown.error()));
    }

    RawTaskConfig raw{};
    auto result = toml_codec::from_toml(table, raw);
    if(!result.has_value()) {
        return std::unexpected(
            ConfigError{.message = std::format("TOML parse error: {}", result.error().message())});
    }

    return to_config(std::move(raw));
}

}  // namespace clore::config
