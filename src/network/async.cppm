module;

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <string>
#include <string_view>
#include <utility>

#include "eventide/async/async.h"
#include "eventide/serde/json/error.h"

export module network:async;

export namespace clore::net {

namespace async = eventide;

struct LLMError {
    std::string message;

    LLMError() = default;

    explicit LLMError(std::string msg) : message(std::move(msg)) {}

    explicit LLMError(std::string_view msg) : message(msg) {}

    explicit LLMError(const char* msg) : message(msg) {}

    explicit LLMError(eventide::error err) : message(err.message()) {}

    explicit LLMError(eventide::serde::json::error err) : message(err.to_string()) {}

    explicit LLMError(eventide::serde::json::error_kind kind)
        : message(eventide::serde::json::error_message(kind)) {}
};

}  // namespace clore::net