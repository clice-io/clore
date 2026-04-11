module;

#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

export module clore.support;

export namespace clore::logging {

struct Options {
    /// No built-in default — caller must set this explicitly before logging.
    std::optional<spdlog::level::level_enum> level;
};

inline Options options;

inline void log(spdlog::level::level_enum lvl, std::string_view msg) {
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
