#pragma once

#include <cstdint>
#include <format>
#include <optional>
#include <string_view>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace clore::logging {

struct Options {
    /// No built-in default — caller must set this explicitly before logging.
    std::optional<spdlog::level::level_enum> level;
};

inline Options options;

inline void log(spdlog::level::level_enum lvl, std::string_view msg) {
    // When no level filter is set, pass every message through to the underlying
    // spdlog logger which applies its own level filter.
    if(options.level.has_value() && lvl < *options.level) {
        return;
    }
    spdlog::default_logger_raw()->log(lvl, "{}", msg);
}

inline void trace(std::string_view msg) { log(spdlog::level::trace, msg); }

template <typename... Args>
void trace(std::format_string<Args...> fmt, Args&&... args) {
    log(spdlog::level::trace, std::format(fmt, std::forward<Args>(args)...));
}

inline void debug(std::string_view msg) { log(spdlog::level::debug, msg); }

template <typename... Args>
void debug(std::format_string<Args...> fmt, Args&&... args) {
    log(spdlog::level::debug, std::format(fmt, std::forward<Args>(args)...));
}

inline void info(std::string_view msg) { log(spdlog::level::info, msg); }

template <typename... Args>
void info(std::format_string<Args...> fmt, Args&&... args) {
    log(spdlog::level::info, std::format(fmt, std::forward<Args>(args)...));
}

inline void warn(std::string_view msg) { log(spdlog::level::warn, msg); }

template <typename... Args>
void warn(std::format_string<Args...> fmt, Args&&... args) {
    log(spdlog::level::warn, std::format(fmt, std::forward<Args>(args)...));
}

inline void err(std::string_view msg) { log(spdlog::level::err, msg); }

template <typename... Args>
void err(std::format_string<Args...> fmt, Args&&... args) {
    log(spdlog::level::err, std::format(fmt, std::forward<Args>(args)...));
}

inline void stderr_logger(std::string_view name) {
    auto logger = spdlog::stderr_color_mt(std::string(name));
    spdlog::set_default_logger(logger);
    if(options.level.has_value()) {
        spdlog::set_level(*options.level);
    }
}

}  // namespace clore::logging

#define LOG_MESSAGE(name, fmt, ...)                                                                \
    do {                                                                                           \
        clore::logging::name(fmt __VA_OPT__(, ) __VA_ARGS__);                                      \
    } while(0)

#define LOG_TRACE(fmt, ...) LOG_MESSAGE(trace, fmt, __VA_ARGS__)
#define LOG_DEBUG(fmt, ...) LOG_MESSAGE(debug, fmt, __VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG_MESSAGE(info, fmt, __VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG_MESSAGE(warn, fmt, __VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG_MESSAGE(err, fmt, __VA_ARGS__)

#define LOG_MESSAGE_RET(ret, name, fmt, ...)                                                       \
    do {                                                                                           \
        LOG_MESSAGE(name, fmt, __VA_ARGS__);                                                       \
        return ret;                                                                                \
    } while(0);

#define LOG_ERROR_RET(ret, fmt, ...) LOG_MESSAGE_RET(ret, err, fmt, __VA_ARGS__)
