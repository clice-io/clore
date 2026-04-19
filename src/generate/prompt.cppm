module;

#include "kota/codec/json/json.h"

export module generate:prompt;

import std;
import :model;
import support;

namespace clore::generate {

namespace json = kota::codec::json;

}  // namespace clore::generate

export namespace clore::generate {

template <typename T>
auto parse_structured_response(std::string_view raw, std::string_view context)
    -> std::expected<T, GenerateError>;

auto normalize_markdown_fragment(std::string_view raw, std::string_view context)
    -> std::expected<std::string, GenerateError>;

auto parse_markdown_prompt_output(std::string_view raw, std::string_view context)
    -> std::expected<std::string, GenerateError>;

}  // namespace clore::generate

namespace clore::generate {

template <typename T>
auto parse_structured_response(std::string_view raw, std::string_view context)
    -> std::expected<T, GenerateError> {
    auto parsed = json::from_json<T>(raw);
    if(!parsed.has_value()) {
        return std::unexpected(GenerateError{
            .message = std::format("failed to parse structured response for {}: {}",
                                   context,
                                   parsed.error().to_string()),
        });
    }
    return *parsed;
}

namespace {

auto trim_trailing_ascii_whitespace(std::string& text) -> void {
    while(!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
        text.pop_back();
    }
}

auto contains_non_whitespace(std::string_view text) -> bool {
    return std::ranges::any_of(text, [](char ch) {
        return std::isspace(static_cast<unsigned char>(ch)) == 0;
    });
}

}  // namespace

auto normalize_markdown_fragment(std::string_view raw, std::string_view context)
    -> std::expected<std::string, GenerateError> {
    auto normalized = clore::support::ensure_utf8(raw);
    normalized = std::string(clore::support::strip_utf8_bom(normalized));
    trim_trailing_ascii_whitespace(normalized);
    if(!contains_non_whitespace(normalized)) {
        return std::unexpected(GenerateError{
            .message = std::format("empty markdown fragment for {}", context),
        });
    }
    return normalized;
}

auto parse_markdown_prompt_output(std::string_view raw, std::string_view context)
    -> std::expected<std::string, GenerateError> {
    return normalize_markdown_fragment(raw, context);
}

}  // namespace clore::generate
