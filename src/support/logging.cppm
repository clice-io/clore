module;

#ifdef _WIN32
#include <Windows.h>
#endif

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "llvm/Support/xxhash.h"

export module support;

import std;

export namespace clore::support {

struct TransparentStringHash {
    using is_transparent = void;

    [[nodiscard]] auto operator()(std::string_view value) const noexcept -> std::size_t {
        return std::hash<std::string_view>{}(value);
    }

    [[nodiscard]] auto operator()(const std::string& value) const noexcept -> std::size_t {
        return (*this)(std::string_view{value});
    }

    [[nodiscard]] auto operator()(const char* value) const noexcept -> std::size_t {
        return (*this)(std::string_view{value});
    }
};

struct TransparentStringEqual {
    using is_transparent = void;

    [[nodiscard]] auto operator()(std::string_view lhs, std::string_view rhs) const noexcept
        -> bool {
        return lhs == rhs;
    }

    [[nodiscard]] auto operator()(const std::string& lhs, std::string_view rhs) const noexcept
        -> bool {
        return std::string_view{lhs} == rhs;
    }

    [[nodiscard]] auto operator()(std::string_view lhs, const std::string& rhs) const noexcept
        -> bool {
        return lhs == std::string_view{rhs};
    }

    [[nodiscard]] auto operator()(const std::string& lhs, const std::string& rhs) const noexcept
        -> bool {
        return lhs == rhs;
    }
};

struct CacheKeyParts {
    std::string path;
    std::uint64_t compile_signature = 0;
};

auto extract_first_plain_paragraph(std::string_view markdown) -> std::string;

auto normalize_path_string(std::string_view path) -> std::string;

auto build_compile_signature(std::string_view directory,
                             std::string_view normalized_file,
                             const std::vector<std::string>& arguments) -> std::uint64_t;

auto build_cache_key(std::string_view normalized_path, std::uint64_t compile_signature)
    -> std::string;

auto split_cache_key(std::string_view cache_key) -> std::expected<CacheKeyParts, std::string>;

auto ensure_utf8(std::string_view text) -> std::string;

auto canonical_log_level_name(std::string_view value) -> std::optional<std::string>;

auto normalize_line_endings(std::string_view text) -> std::string;

auto truncate_utf8(std::string_view text, std::size_t max_bytes) -> std::string;

auto strip_utf8_bom(std::string_view text) -> std::string_view;

auto read_utf8_text_file(const std::filesystem::path& path)
    -> std::expected<std::string, std::string>;

auto write_utf8_text_file(const std::filesystem::path& path, std::string_view content)
    -> std::expected<void, std::string>;

auto enable_utf8_console() -> void;

auto topological_order(
    const std::vector<std::string>& nodes,
    const std::unordered_map<std::string, std::vector<std::string>>& reverse_edges,
    std::unordered_map<std::string, int> in_degree) -> std::optional<std::vector<std::string>>;

}  // namespace clore::support

export namespace clore::logging {

inline std::optional<spdlog::level::level_enum> g_log_level;

inline void log(spdlog::level::level_enum lvl, std::string_view msg) {
    if(g_log_level.has_value() && lvl < *g_log_level) {
        return;
    }
    spdlog::default_logger_raw()->log(lvl, "{}", msg);
}

template <spdlog::level::level_enum Level>
struct LogProxy {
    void operator()(std::string_view msg) const {
        log(Level, msg);
    }

    template <typename... Args>
        requires (sizeof...(Args) > 0)
    void operator()(std::format_string<Args...> fmt, Args&&... args) const {
        log(Level, std::format(fmt, std::forward<Args>(args)...));
    }
};

constexpr inline LogProxy<spdlog::level::trace> trace;
constexpr inline LogProxy<spdlog::level::debug> debug;
constexpr inline LogProxy<spdlog::level::info> info;
constexpr inline LogProxy<spdlog::level::warn> warn;
constexpr inline LogProxy<spdlog::level::err> err;

inline void stderr_logger(std::string_view name) {
    auto logger = spdlog::stderr_color_mt(std::string(name));
    spdlog::set_default_logger(logger);
    if(g_log_level.has_value()) {
        spdlog::set_level(*g_log_level);
    }
}

inline void cache_hit_rate(std::string_view name, std::size_t hits, std::size_t misses) {
    const auto total = hits + misses;
    if(total == 0) {
        throw std::logic_error(std::format("cache hit rate '{}' requires non-zero total", name));
    }

    const auto rate = (static_cast<double>(hits) * 100.0) / static_cast<double>(total);
    info("{} cache: {} hits, {} misses ({:.1f}%)", name, hits, misses, rate);
}

}  // namespace clore::logging

namespace clore::support {

namespace {

constexpr unsigned char kUtf8Bom[] = {0xEF, 0xBB, 0xBF};
constexpr char kUtf8Replacement[] = "\xEF\xBF\xBD";
constexpr char kCacheKeyDelimiter = '\t';

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
        if(remaining >= 2 && is_continuation_byte(static_cast<unsigned char>(text[offset + 1]))) {
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
        if(remaining >= 3 && is_continuation_byte(static_cast<unsigned char>(text[offset + 1])) &&
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
            if(second >= 0x90U && second <= 0xBFU && is_continuation_byte(third) &&
               is_continuation_byte(fourth)) {
                return 4;
            }
        }
        return 0;
    }

    if(lead >= 0xF1U && lead <= 0xF3U) {
        if(remaining >= 4 && is_continuation_byte(static_cast<unsigned char>(text[offset + 1])) &&
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
            if(second >= 0x80U && second <= 0x8FU && is_continuation_byte(third) &&
               is_continuation_byte(fourth)) {
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

auto strip_inline_markdown_text(std::string_view text) -> std::string {
    std::string plain;
    plain.reserve(text.size());

    bool in_code = false;
    for(std::size_t i = 0; i < text.size(); ++i) {
        auto ch = text[i];

        if(ch == '`') {
            in_code = !in_code;
            continue;
        }

        if(!in_code && (ch == '*' || ch == '_' || ch == '~')) {
            continue;
        }

        if(!in_code && ch == '[') {
            auto close = text.find(']', i + 1);
            auto open_paren = close != std::string_view::npos ? text.find('(', close + 1)
                                                              : std::string_view::npos;
            auto close_paren = open_paren != std::string_view::npos ? text.find(')', open_paren + 1)
                                                                    : std::string_view::npos;
            if(close != std::string_view::npos && open_paren == close + 1 &&
               close_paren != std::string_view::npos) {
                plain.append(text.substr(i + 1, close - (i + 1)));
                i = close_paren;
                continue;
            }
        }

        plain.push_back(ch);
    }

    while(!plain.empty() && std::isspace(static_cast<unsigned char>(plain.front())) != 0) {
        plain.erase(plain.begin());
    }
    while(!plain.empty() && std::isspace(static_cast<unsigned char>(plain.back())) != 0) {
        plain.pop_back();
    }

    return plain;
}

}  // namespace

auto extract_first_plain_paragraph(std::string_view markdown) -> std::string {
    std::istringstream stream{std::string(markdown)};
    std::string line;
    bool in_code_block = false;
    std::string paragraph;

    while(std::getline(stream, line)) {
        std::string_view trimmed = line;
        while(!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.front())) != 0) {
            trimmed.remove_prefix(1);
        }
        while(!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.back())) != 0) {
            trimmed.remove_suffix(1);
        }

        if(trimmed.starts_with("```")) {
            in_code_block = !in_code_block;
            continue;
        }
        if(in_code_block) {
            continue;
        }
        if(trimmed.empty()) {
            if(!paragraph.empty()) {
                break;
            }
            continue;
        }
        if(trimmed.starts_with("#") || trimmed.starts_with(">") || trimmed.starts_with("|") ||
           trimmed.starts_with("- ") || trimmed.starts_with("* ")) {
            if(!paragraph.empty()) {
                break;
            }
            continue;
        }

        if(!paragraph.empty()) {
            paragraph.push_back(' ');
        }
        paragraph.append(trimmed);
    }

    return strip_inline_markdown_text(paragraph);
}

auto normalize_path_string(std::string_view path) -> std::string {
    return std::filesystem::path(path).lexically_normal().generic_string();
}

auto build_compile_signature(std::string_view directory,
                             std::string_view normalized_file,
                             const std::vector<std::string>& arguments) -> std::uint64_t {
    std::string payload;
    payload.reserve(directory.size() + normalized_file.size() + arguments.size() * 16);
    payload.append(normalize_path_string(directory));
    payload.push_back('\0');
    payload.append(normalized_file);
    payload.push_back('\0');
    for(const auto& argument: arguments) {
        payload.append(argument);
        payload.push_back('\0');
    }
    return llvm::xxh3_64bits(payload);
}

auto build_cache_key(std::string_view normalized_path, std::uint64_t compile_signature)
    -> std::string {
    std::string key;
    key.reserve(normalized_path.size() + 1 + 20);
    key.append(normalized_path);
    key.push_back(kCacheKeyDelimiter);
    key.append(std::to_string(compile_signature));
    return key;
}

auto split_cache_key(std::string_view cache_key) -> std::expected<CacheKeyParts, std::string> {
    auto delimiter_pos = cache_key.rfind(kCacheKeyDelimiter);
    if(delimiter_pos == std::string_view::npos) {
        return std::unexpected(std::format("invalid cache key (missing delimiter): {}", cache_key));
    }

    auto path_part = cache_key.substr(0, delimiter_pos);
    auto signature_part = cache_key.substr(delimiter_pos + 1);
    if(path_part.empty() || signature_part.empty()) {
        return std::unexpected(
            std::format("invalid cache key (empty path or signature): {}", cache_key));
    }

    std::uint64_t signature = 0;
    auto [ptr, ec] = std::from_chars(signature_part.data(),
                                     signature_part.data() + signature_part.size(),
                                     signature);
    if(ec != std::errc{} || ptr != signature_part.data() + signature_part.size()) {
        return std::unexpected(std::format("invalid cache key signature in {}", cache_key));
    }

    return CacheKeyParts{
        .path = std::string(path_part),
        .compile_signature = signature,
    };
}

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

auto canonical_log_level_name(std::string_view value) -> std::optional<std::string> {
    if(value.empty()) {
        return std::nullopt;
    }

    std::string normalized(value);
    for(auto& ch: normalized) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }

    auto level = spdlog::level::from_str(normalized);
    if(level == spdlog::level::off && normalized != "off") {
        return std::nullopt;
    }

    return normalized;
}

auto normalize_line_endings(std::string_view text) -> std::string {
    std::string normalized;
    normalized.reserve(text.size());

    for(std::size_t index = 0; index < text.size(); ++index) {
        if(text[index] == '\r') {
            normalized.push_back('\n');
            if(index + 1 < text.size() && text[index + 1] == '\n') {
                ++index;
            }
            continue;
        }
        normalized.push_back(text[index]);
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
    if(text.size() >= std::size(kUtf8Bom) && static_cast<unsigned char>(text[0]) == kUtf8Bom[0] &&
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
        return std::unexpected(std::format("failed to open text file: {}", path.generic_string()));
    }

    std::string content;
    std::error_code size_error;
    auto size = std::filesystem::file_size(path, size_error);
    if(!size_error) {
        content.reserve(static_cast<std::size_t>(size));
    }

    std::array<char, 8192> chunk{};
    while(file) {
        file.read(chunk.data(), static_cast<std::streamsize>(chunk.size()));
        auto bytes_read = file.gcount();
        if(bytes_read > 0) {
            content.append(chunk.data(), static_cast<std::size_t>(bytes_read));
        }
    }

    if(file.bad() || (file.fail() && !file.eof())) {
        return std::unexpected(std::format("failed to read text file: {}", path.generic_string()));
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
        return std::unexpected(std::format("failed to write text file: {}", path.generic_string()));
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

auto topological_order(
    const std::vector<std::string>& nodes,
    const std::unordered_map<std::string, std::vector<std::string>>& reverse_edges,
    std::unordered_map<std::string, int> in_degree) -> std::optional<std::vector<std::string>> {
    std::set<std::string> ready;
    for(auto& node: nodes) {
        auto it = in_degree.find(node);
        if(it == in_degree.end() || it->second == 0) {
            ready.insert(node);
        }
    }

    std::vector<std::string> order;
    order.reserve(nodes.size());

    while(!ready.empty()) {
        auto current = *ready.begin();
        ready.erase(ready.begin());
        order.push_back(current);

        auto it = reverse_edges.find(current);
        if(it != reverse_edges.end()) {
            for(auto& dependent: it->second) {
                auto deg_it = in_degree.find(dependent);
                if(deg_it != in_degree.end() && deg_it->second > 0 && --deg_it->second == 0) {
                    ready.insert(dependent);
                }
            }
        }
    }

    if(order.size() < nodes.size()) {
        return std::nullopt;
    }
    return order;
}

}  // namespace clore::support
