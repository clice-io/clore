module;

#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <format>
#include <ios>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

export module support;

export namespace clore::support {

auto ensure_utf8(std::string_view text) -> std::string;

auto truncate_utf8(std::string_view text, std::size_t max_bytes) -> std::string;

auto strip_utf8_bom(std::string_view text) -> std::string_view;

auto read_utf8_text_file(const std::filesystem::path& path)
    -> std::expected<std::string, std::string>;

auto write_utf8_text_file(const std::filesystem::path& path, std::string_view content)
    -> std::expected<void, std::string>;

auto enable_utf8_console() -> void;

}  // namespace clore::support

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

namespace clore::support {

namespace {

constexpr unsigned char kUtf8Bom[] = {0xEF, 0xBB, 0xBF};
constexpr char kUtf8Replacement[] = "\xEF\xBF\xBD";

auto is_continuation_byte(unsigned char byte) -> bool {
    return (byte & 0xC0U) == 0x80U;
}

auto valid_utf8_sequence_length(std::string_view text, std::size_t offset) -> std::size_t {
    auto lead = static_cast<unsigned char>(text[offset]);
    auto remaining = text.size() - offset;

    if(lead <= 0x7FU) {
        return 1;
    }

    if(lead >= 0xC2U && lead <= 0xDFU) {
        if(remaining >= 2 &&
           is_continuation_byte(static_cast<unsigned char>(text[offset + 1]))) {
            return 2;
        }
        return 0;
    }

    if(lead == 0xE0U) {
        if(remaining >= 3) {
            auto second = static_cast<unsigned char>(text[offset + 1]);
            auto third = static_cast<unsigned char>(text[offset + 2]);
            if(second >= 0xA0U && second <= 0xBFU && is_continuation_byte(third)) {
                return 3;
            }
        }
        return 0;
    }

    if((lead >= 0xE1U && lead <= 0xECU) || (lead >= 0xEEU && lead <= 0xEFU)) {
        if(remaining >= 3 &&
           is_continuation_byte(static_cast<unsigned char>(text[offset + 1])) &&
           is_continuation_byte(static_cast<unsigned char>(text[offset + 2]))) {
            return 3;
        }
        return 0;
    }

    if(lead == 0xEDU) {
        if(remaining >= 3) {
            auto second = static_cast<unsigned char>(text[offset + 1]);
            auto third = static_cast<unsigned char>(text[offset + 2]);
            if(second >= 0x80U && second <= 0x9FU && is_continuation_byte(third)) {
                return 3;
            }
        }
        return 0;
    }

    if(lead == 0xF0U) {
        if(remaining >= 4) {
            auto second = static_cast<unsigned char>(text[offset + 1]);
            auto third = static_cast<unsigned char>(text[offset + 2]);
            auto fourth = static_cast<unsigned char>(text[offset + 3]);
            if(second >= 0x90U && second <= 0xBFU &&
               is_continuation_byte(third) && is_continuation_byte(fourth)) {
                return 4;
            }
        }
        return 0;
    }

    if(lead >= 0xF1U && lead <= 0xF3U) {
        if(remaining >= 4 &&
           is_continuation_byte(static_cast<unsigned char>(text[offset + 1])) &&
           is_continuation_byte(static_cast<unsigned char>(text[offset + 2])) &&
           is_continuation_byte(static_cast<unsigned char>(text[offset + 3]))) {
            return 4;
        }
        return 0;
    }

    if(lead == 0xF4U) {
        if(remaining >= 4) {
            auto second = static_cast<unsigned char>(text[offset + 1]);
            auto third = static_cast<unsigned char>(text[offset + 2]);
            auto fourth = static_cast<unsigned char>(text[offset + 3]);
            if(second >= 0x80U && second <= 0x8FU &&
               is_continuation_byte(third) && is_continuation_byte(fourth)) {
                return 4;
            }
        }
        return 0;
    }

    return 0;
}

auto utf8_prefix_length(std::string_view text, std::size_t max_bytes) -> std::size_t {
    std::size_t offset = 0;
    while(offset < text.size()) {
        auto sequence_length = valid_utf8_sequence_length(text, offset);
        if(sequence_length == 0 || offset + sequence_length > max_bytes) {
            break;
        }
        offset += sequence_length;
    }
    return offset;
}

}  // namespace

auto ensure_utf8(std::string_view text) -> std::string {
    std::string normalized;
    normalized.reserve(text.size());

    for(std::size_t offset = 0; offset < text.size();) {
        auto sequence_length = valid_utf8_sequence_length(text, offset);
        if(sequence_length == 0) {
            normalized += kUtf8Replacement;
            ++offset;
            continue;
        }

        normalized.append(text.substr(offset, sequence_length));
        offset += sequence_length;
    }

    return normalized;
}

auto truncate_utf8(std::string_view text, std::size_t max_bytes) -> std::string {
    auto normalized = ensure_utf8(text);
    if(normalized.size() <= max_bytes) {
        return normalized;
    }

    normalized.resize(utf8_prefix_length(normalized, max_bytes));
    return normalized;
}

auto strip_utf8_bom(std::string_view text) -> std::string_view {
    if(text.size() >= std::size(kUtf8Bom) &&
       static_cast<unsigned char>(text[0]) == kUtf8Bom[0] &&
       static_cast<unsigned char>(text[1]) == kUtf8Bom[1] &&
       static_cast<unsigned char>(text[2]) == kUtf8Bom[2]) {
        return text.substr(std::size(kUtf8Bom));
    }

    return text;
}

auto read_utf8_text_file(const std::filesystem::path& path)
    -> std::expected<std::string, std::string> {
    std::ifstream file(path, std::ios::binary);
    if(!file.is_open()) {
        return std::unexpected(
            std::format("failed to open text file: {}", path.generic_string()));
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    auto content = buffer.str();

    if(file.bad() || (file.fail() && !file.eof()) || buffer.bad()) {
        return std::unexpected(
            std::format("failed to read text file: {}", path.generic_string()));
    }

    auto normalized = strip_utf8_bom(content);
    if(normalized.size() == content.size()) {
        return content;
    }

    return std::string(normalized);
}

auto write_utf8_text_file(const std::filesystem::path& path, std::string_view content)
    -> std::expected<void, std::string> {
    auto normalized = ensure_utf8(content);

    std::ofstream file(path, std::ios::binary);
    if(!file.is_open()) {
        return std::unexpected(
            std::format("failed to open text file for writing: {}", path.generic_string()));
    }

    file.write(normalized.data(), static_cast<std::streamsize>(normalized.size()));
    file.flush();
    if(!file) {
        return std::unexpected(
            std::format("failed to write text file: {}", path.generic_string()));
    }

    return {};
}

auto enable_utf8_console() -> void {
#ifdef _WIN32
    if(SetConsoleCP(CP_UTF8) == 0) {
        clore::logging::warn("SetConsoleCP(CP_UTF8) failed with error {}",
                             static_cast<std::uint32_t>(GetLastError()));
    }
    if(SetConsoleOutputCP(CP_UTF8) == 0) {
        clore::logging::warn("SetConsoleOutputCP(CP_UTF8) failed with error {}",
                             static_cast<std::uint32_t>(GetLastError()));
    }
#endif
}

}  // namespace clore::support
