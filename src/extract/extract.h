#pragma once

#include <expected>
#include <string>

#include "config/schema.h"
#include "extract/model.h"

namespace clore::extract {

struct ExtractError {
    std::string message;
};

auto extract_project(const config::TaskConfig& config)
    -> std::expected<ProjectModel, ExtractError>;

}  // namespace clore::extract
