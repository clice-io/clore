#pragma once

#include <expected>
#include <string>
#include <string_view>

#include "config/schema.h"

namespace clore::config {

struct ConfigError {
    std::string message;
};

auto load_config(std::string_view path) -> std::expected<TaskConfig, ConfigError>;

auto load_config_from_string(std::string_view toml_content) -> std::expected<TaskConfig, ConfigError>;

}  // namespace clore::config
