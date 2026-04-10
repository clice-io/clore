#pragma once

#include <expected>
#include <string>

#include "config/schema.h"

namespace clore::config {

struct ValidationError {
    std::string message;
};

auto validate(const TaskConfig& config) -> std::expected<void, ValidationError>;

}  // namespace clore::config
