#pragma once

#include <expected>
#include <string>

namespace clore::generate {

struct LLMError {
    std::string message;
};

namespace detail {

auto build_request_json(std::string_view model, std::string_view prompt) -> std::string;

auto parse_response(std::string_view json) -> std::expected<std::string, LLMError>;

}  // namespace detail

auto call_llm(std::string_view model, std::string_view prompt)
    -> std::expected<std::string, LLMError>;

}  // namespace clore::generate
