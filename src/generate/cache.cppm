module;

#include "kota/async/async.h"
#include "kota/codec/json/json.h"

#include "llvm/Support/xxhash.h"

export module generate:cache;

import std;
import protocol;
import support;

export namespace clore::generate::cache {

struct CacheError {
    std::string message;
};

struct CacheIndex {
    std::unordered_map<std::string, std::string> entries;
};

auto make_prompt_response_cache_key(std::string_view request_key,
                                    std::string_view system_prompt,
                                    const clore::net::PromptRequest& request)
    -> std::expected<std::string, CacheError>;

auto load_cache_index(std::string_view workspace_root) -> std::expected<CacheIndex, CacheError>;

auto save_cache_entry(std::string_view workspace_root,
                      std::string_view cache_key,
                      std::string_view response) -> std::expected<void, CacheError>;

auto find_cached_response(const CacheIndex& index, std::string_view cache_key)
    -> std::optional<std::string_view>;

auto load_cache_index_async(std::string workspace_root, kota::event_loop& loop)
    -> kota::task<CacheIndex, CacheError>;

auto save_cache_entry_async(std::string workspace_root,
                            std::string cache_key,
                            std::string response,
                            kota::event_loop& loop) -> kota::task<void, CacheError>;

}  // namespace clore::generate::cache

namespace clore::generate::cache {

namespace fs = std::filesystem;

namespace {

auto cache_directory(std::string_view workspace_root) -> std::expected<fs::path, CacheError> {
    if(workspace_root.empty()) {
        return std::unexpected(CacheError{
            .message = "workspace_root must not be empty when resolving generation prompt cache"});
    }
    return fs::path(workspace_root) / ".clice" / "cache" / "clore" / "generate";
}

auto format_iso_timestamp() -> std::string {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &time);
#else
    gmtime_r(&time, &utc);
#endif
    std::array<char, 32> buf{};
    std::strftime(buf.data(), buf.size(), "%Y-%m-%dT%H-%M-%S", &utc);
    return std::string(buf.data());
}

auto current_jsonl_filename() -> fs::path {
    static std::string timestamp = format_iso_timestamp();
    return fs::path(std::format("{}.jsonl", timestamp));
}

auto all_jsonl_files(std::string_view workspace_root)
    -> std::expected<std::vector<fs::path>, CacheError> {
    auto dir = cache_directory(workspace_root);
    if(!dir.has_value()) {
        return std::unexpected(std::move(dir.error()));
    }

    std::vector<fs::path> files;
    std::error_code iter_error;
    for(const auto& entry: fs::directory_iterator(*dir, iter_error)) {
        if(iter_error) {
            continue;
        }
        if(!entry.is_regular_file()) {
            continue;
        }
        auto ext = entry.path().extension();
        if(ext == ".jsonl") {
            files.push_back(entry.path());
        }
    }
    std::ranges::sort(files);
    return files;
}

auto response_format_fingerprint(const std::optional<clore::net::ResponseFormat>& response_format)
    -> std::expected<std::string, CacheError> {
    if(!response_format.has_value()) {
        return std::string("none");
    }

    if(!response_format->schema.has_value()) {
        return std::format("{}:json_object", response_format->name);
    }

    auto schema_json = response_format->schema->to_json_string();
    if(!schema_json.has_value()) {
        return std::unexpected(CacheError{
            .message = std::format("failed to serialize response format schema '{}' (code {})",
                                   response_format->name,
                                   static_cast<int>(schema_json.error())),
        });
    }

    std::string fingerprint;
    fingerprint.reserve(response_format->name.size() + 48);
    fingerprint.append(response_format->name);
    fingerprint.push_back(':');
    fingerprint.append(response_format->strict ? "strict" : "loose");
    fingerprint.push_back(':');
    fingerprint.append(std::to_string(llvm::xxh3_64bits(*schema_json)));
    return fingerprint;
}

auto tool_choice_fingerprint(const std::optional<clore::net::ToolChoice>& tool_choice)
    -> std::string {
    if(!tool_choice.has_value()) {
        return "none";
    }
    return std::visit(
        [](const auto& current) -> std::string {
            using choice_type = std::remove_cvref_t<decltype(current)>;
            if constexpr(std::same_as<choice_type, clore::net::ToolChoiceAuto>) {
                return "auto";
            } else if constexpr(std::same_as<choice_type, clore::net::ToolChoiceRequired>) {
                return "required";
            } else if constexpr(std::same_as<choice_type, clore::net::ToolChoiceNone>) {
                return "none";
            } else {
                return std::format("tool:{}", current.name);
            }
        },
        *tool_choice);
}

auto escape_json_string(std::string_view text) -> std::string {
    std::string escaped;
    escaped.reserve(text.size());
    for(char ch: text) {
        switch(ch) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            default:
                if(static_cast<unsigned char>(ch) < 0x20) {
                    escaped += std::format("\\u{:04x}", static_cast<unsigned char>(ch));
                } else {
                    escaped += ch;
                }
        }
    }
    return escaped;
}

auto build_jsonl_line(std::string_view cache_key, std::string_view response) -> std::string {
    std::string line;
    line.reserve(cache_key.size() + response.size() + 64);
    line += "{\"key\":\"";
    line += escape_json_string(cache_key);
    line += "\",\"req\":{},\"resp\":\"";
    line += escape_json_string(response);
    line += "\"}\n";
    return line;
}

}  // namespace

auto normalize_text_for_hashing(std::string_view text) -> std::string {
    std::string result;
    result.reserve(text.size());

    // Trim leading whitespace
    std::size_t start = 0;
    while(start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) {
        ++start;
    }

    bool prev_space = false;
    for(std::size_t i = start; i < text.size(); ++i) {
        auto ch = static_cast<unsigned char>(text[i]);
        if(std::isspace(ch)) {
            prev_space = true;
        } else {
            if(prev_space && !result.empty()) {
                result.push_back(' ');
            }
            result.push_back(static_cast<char>(ch));
            prev_space = false;
        }
    }

    return result;
}

auto make_prompt_response_cache_key(std::string_view request_key,
                                    std::string_view system_prompt,
                                    const clore::net::PromptRequest& request)
    -> std::expected<std::string, CacheError> {
    auto response_format = response_format_fingerprint(request.response_format);
    if(!response_format.has_value()) {
        return std::unexpected(std::move(response_format.error()));
    }

    auto normalized_prompt = normalize_text_for_hashing(request.prompt);
    auto normalized_system_prompt = normalize_text_for_hashing(system_prompt);

    auto prompt_hash = llvm::xxh3_64bits(normalized_prompt);
    auto system_prompt_hash = llvm::xxh3_64bits(normalized_system_prompt);

    auto tool_choice = tool_choice_fingerprint(request.tool_choice);

    std::string key;
    key.reserve(request_key.size() + response_format->size() + tool_choice.size() + 128);
    key.append(request_key);
    key.push_back('\t');
    key.append(std::to_string(prompt_hash));
    key.push_back('\t');
    key.append(std::to_string(system_prompt_hash));
    key.push_back('\t');
    key.append(*response_format);
    key.push_back('\t');
    key.append(tool_choice);
    key.push_back('\t');
    key.push_back(static_cast<char>('0' + static_cast<int>(request.output_contract)));
    return key;
}

auto load_cache_index(std::string_view workspace_root) -> std::expected<CacheIndex, CacheError> {
    CacheIndex index;

    auto files = all_jsonl_files(workspace_root);
    if(!files.has_value()) {
        return std::unexpected(std::move(files.error()));
    }

    for(const auto& path: *files) {
        auto content = clore::support::read_utf8_text_file(path);
        if(!content.has_value()) {
            continue;
        }

        std::istringstream stream(*content);
        std::string line;
        while(std::getline(stream, line)) {
            if(line.empty()) {
                continue;
            }

            auto parsed = kota::codec::json::Object::parse(line);
            if(!parsed.has_value()) {
                continue;
            }

            auto key_value = parsed->get("key");
            if(!key_value.has_value()) {
                continue;
            }
            auto key = key_value->get_string();
            if(!key.has_value() || key->empty()) {
                continue;
            }

            auto resp_value = parsed->get("resp");
            if(!resp_value.has_value()) {
                continue;
            }
            auto resp = resp_value->get_string();
            if(!resp.has_value()) {
                continue;
            }

            index.entries.insert_or_assign(std::string(*key), std::string(*resp));
        }
    }

    return index;
}

auto save_cache_entry(std::string_view workspace_root,
                      std::string_view cache_key,
                      std::string_view response) -> std::expected<void, CacheError> {
    static std::mutex cache_file_mutex;
    std::lock_guard lock(cache_file_mutex);

    auto dir = cache_directory(workspace_root);
    if(!dir.has_value()) {
        return std::unexpected(std::move(dir.error()));
    }

    std::error_code create_error;
    fs::create_directories(*dir, create_error);
    if(create_error) {
        return std::unexpected(CacheError{
            .message = std::format("failed to create generation prompt cache directory {}: {}",
                                   dir->generic_string(),
                                   create_error.message()),
        });
    }

    auto jsonl_path = *dir / current_jsonl_filename();
    auto line = build_jsonl_line(cache_key, response);

    std::ofstream file(jsonl_path, std::ios::binary | std::ios::app);
    if(!file.is_open()) {
        return std::unexpected(CacheError{
            .message = std::format("failed to open prompt response cache {} for append",
                                   jsonl_path.generic_string()),
        });
    }

    file.write(line.data(), static_cast<std::streamsize>(line.size()));
    file.flush();
    if(!file) {
        return std::unexpected(CacheError{
            .message = std::format("failed to append to prompt response cache {}",
                                   jsonl_path.generic_string()),
        });
    }

    return {};
}

auto find_cached_response(const CacheIndex& index, std::string_view cache_key)
    -> std::optional<std::string_view> {
    auto it = index.entries.find(std::string(cache_key));
    if(it == index.entries.end()) {
        return std::nullopt;
    }
    return it->second;
}

auto load_cache_index_async(std::string workspace_root, kota::event_loop& loop)
    -> kota::task<CacheIndex, CacheError> {
    auto queued_result = co_await kota::queue(
                             [workspace_root = std::move(workspace_root)]() {
                                 return load_cache_index(workspace_root);
                             },
                             loop)
                             .catch_cancel();
    if(queued_result.has_error()) {
        co_await kota::fail(CacheError{
            .message = std::format("prompt response cache load task failed: {}",
                                   queued_result.error().message()),
        });
    }
    if(!queued_result->has_value()) {
        co_await kota::fail(std::move(queued_result->error()));
    }
    co_return std::move(queued_result->value());
}

auto save_cache_entry_async(std::string workspace_root,
                            std::string cache_key,
                            std::string response,
                            kota::event_loop& loop) -> kota::task<void, CacheError> {
    auto queued_result = co_await kota::queue(
                             [workspace_root = std::move(workspace_root),
                              cache_key = std::move(cache_key),
                              response = std::move(response)]() {
                                 return save_cache_entry(workspace_root, cache_key, response);
                             },
                             loop)
                             .catch_cancel();
    if(queued_result.has_error()) {
        co_await kota::fail(CacheError{
            .message = std::format("prompt response cache save task failed: {}",
                                   queued_result.error().message()),
        });
    }
    if(!queued_result->has_value()) {
        co_await kota::fail(std::move(queued_result->error()));
    }
    co_return;
}

}  // namespace clore::generate::cache
