#pragma once

#include <expected>
#include <string>

#include "config/schema.h"

namespace clore::config {

struct NormalizeError {
    std::string message;
};

auto normalize(TaskConfig& config) -> std::expected<void, NormalizeError>;

}  // namespace clore::config
