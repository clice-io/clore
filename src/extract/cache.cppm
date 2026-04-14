module;

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "eventide/serde/json/deserializer.h"
#include "eventide/serde/json/serializer.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/xxhash.h"

export module extract:cache;

import :ast;
import :compdb;
import :scan;
import support;

export namespace clore::extract::cache {

struct CacheError {
    std::string message;
};

struct DependencySnapshot {
    std::vector<std::string> files;
    std::vector<std::uint64_t> hashes;
    std::int64_t build_at = 0;
};

struct CacheRecord {
    std::uint64_t compile_signature = 0;
    std::uint64_t source_hash = 0;
    DependencySnapshot ast_deps;
    ScanResult scan;
    ASTResult ast;
};

auto build_compile_signature(const CompileEntry& entry) -> std::uint64_t;

auto hash_file(std::string_view path) -> std::expected<std::uint64_t, CacheError>;

auto capture_dependency_snapshot(const std::vector<std::string>& files)
    -> std::expected<DependencySnapshot, CacheError>;

auto dependencies_changed(const DependencySnapshot& snapshot) -> bool;

auto load_extract_cache(std::string_view workspace_root)
    -> std::expected<std::unordered_map<std::string, CacheRecord>, CacheError>;

auto save_extract_cache(std::string_view workspace_root,
                        const std::unordered_map<std::string, CacheRecord>& records)
    -> std::expected<void, CacheError>;

}  // namespace clore::extract::cache

namespace clore::extract::cache {

namespace fs = std::filesystem;
namespace json = eventide::serde::json;

namespace {

constexpr std::uint32_t kExtractCacheFormatVersion = 1;

struct CachedPathHash {
    std::uint32_t path = 0;
    std::uint64_t hash = 0;
};

struct SerializedDependencySnapshot {
    std::int64_t build_at = 0;
    std::vector<CachedPathHash> deps;
};

struct SerializedCacheEntry {
    std::uint32_t source_file = 0;
    std::uint64_t compile_signature = 0;
    std::uint64_t source_hash = 0;
    SerializedDependencySnapshot ast_deps;
    ScanResult scan;
    ASTResult ast;
};

struct SerializedCacheData {
    std::uint32_t format_version = kExtractCacheFormatVersion;
    std::vector<std::string> paths;
    std::vector<SerializedCacheEntry> entries;
};

auto cache_directory(std::string_view workspace_root)
    -> std::expected<fs::path, CacheError> {
    if(workspace_root.empty()) {
        return std::unexpected(CacheError{
            .message = "workspace_root must not be empty when resolving extract cache",
        });
    }

    return fs::path(workspace_root) / ".clice" / "cache" / "clore" / "extract";
}

auto cache_file_path(std::string_view workspace_root)
    -> std::expected<fs::path, CacheError> {
    auto root = cache_directory(workspace_root);
    if(!root.has_value()) {
        return std::unexpected(std::move(root.error()));
    }
    return *root / "cache.json";
}

auto normalize_path_string(std::string_view path) -> std::string {
    return fs::path(path).lexically_normal().generic_string();
}

auto decode_dependency_snapshot(const SerializedDependencySnapshot& serialized,
                                const std::vector<std::string>& paths)
    -> std::expected<DependencySnapshot, CacheError> {
    DependencySnapshot snapshot;
    snapshot.build_at = serialized.build_at;
    snapshot.files.reserve(serialized.deps.size());
    snapshot.hashes.reserve(serialized.deps.size());

    for(const auto& dep : serialized.deps) {
        if(dep.path >= paths.size()) {
            return std::unexpected(CacheError{
                .message = std::format("extract cache references invalid path index {}", dep.path),
            });
        }
        snapshot.files.push_back(paths[dep.path]);
        snapshot.hashes.push_back(dep.hash);
    }

    return snapshot;
}

auto encode_dependency_snapshot(const DependencySnapshot& snapshot,
                                std::unordered_map<std::string, std::uint32_t>& path_ids,
                                std::vector<std::string>& paths)
    -> SerializedDependencySnapshot {
    auto intern_path = [&](std::string_view value) -> std::uint32_t {
        auto normalized = normalize_path_string(value);
        auto [it, inserted] =
            path_ids.try_emplace(normalized, static_cast<std::uint32_t>(paths.size()));
        if(inserted) {
            paths.push_back(std::move(normalized));
        }
        return it->second;
    };

    SerializedDependencySnapshot serialized;
    serialized.build_at = snapshot.build_at;
    serialized.deps.reserve(snapshot.files.size());

    for(std::size_t index = 0; index < snapshot.files.size(); ++index) {
        serialized.deps.push_back(CachedPathHash{
            .path = intern_path(snapshot.files[index]),
            .hash = snapshot.hashes[index],
        });
    }

    return serialized;
}

}  // namespace

auto build_compile_signature(const CompileEntry& entry) -> std::uint64_t {
    std::string payload;
    payload.reserve(entry.directory.size() + entry.file.size() + entry.arguments.size() * 16);
    payload.append(normalize_path_string(entry.directory));
    payload.push_back('\0');
    payload.append(normalize_path_string(entry.file));
    payload.push_back('\0');
    for(const auto& argument : entry.arguments) {
        payload.append(argument);
        payload.push_back('\0');
    }
    return llvm::xxh3_64bits(payload);
}

auto hash_file(std::string_view path) -> std::expected<std::uint64_t, CacheError> {
    auto buffer = llvm::MemoryBuffer::getFile(path);
    if(!buffer) {
        return std::unexpected(CacheError{
            .message = std::format("failed to read '{}' for hashing: {}",
                                   path, buffer.getError().message()),
        });
    }
    return llvm::xxh3_64bits((*buffer)->getBuffer());
}

auto capture_dependency_snapshot(const std::vector<std::string>& files)
    -> std::expected<DependencySnapshot, CacheError> {
    DependencySnapshot snapshot;
    snapshot.build_at = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    std::vector<std::string> normalized;
    normalized.reserve(files.size());
    for(const auto& file : files) {
        normalized.push_back(normalize_path_string(file));
    }
    std::ranges::sort(normalized);
    normalized.erase(std::unique(normalized.begin(), normalized.end()), normalized.end());

    snapshot.files.reserve(normalized.size());
    snapshot.hashes.reserve(normalized.size());
    for(const auto& file : normalized) {
        auto hash = hash_file(file);
        if(!hash.has_value()) {
            return std::unexpected(CacheError{
                .message = std::format("failed to hash dependency '{}': {}",
                                       file, hash.error().message),
            });
        }
        snapshot.files.push_back(file);
        snapshot.hashes.push_back(*hash);
    }

    return snapshot;
}

auto dependencies_changed(const DependencySnapshot& snapshot) -> bool {
    if(snapshot.build_at <= 0 || snapshot.files.empty()) {
        return true;
    }

    if(snapshot.files.size() != snapshot.hashes.size()) {
        return true;
    }

    for(std::size_t index = 0; index < snapshot.files.size(); ++index) {
        const auto& file = snapshot.files[index];
        llvm::sys::fs::file_status status;
        if(auto ec = llvm::sys::fs::status(file, status)) {
            if(snapshot.hashes[index] != 0) {
                return true;
            }
            continue;
        }

        auto current_mtime = std::chrono::duration_cast<std::chrono::nanoseconds>(
            status.getLastModificationTime().time_since_epoch()).count();
        if(current_mtime <= snapshot.build_at) {
            continue;
        }

        auto hash = hash_file(file);
        if(!hash.has_value() || *hash != snapshot.hashes[index]) {
            return true;
        }
    }

    return false;
}

auto load_extract_cache(std::string_view workspace_root)
    -> std::expected<std::unordered_map<std::string, CacheRecord>, CacheError> {
    auto cache_path = cache_file_path(workspace_root);
    if(!cache_path.has_value()) {
        return std::unexpected(std::move(cache_path.error()));
    }

    std::error_code exists_error;
    if(!fs::exists(*cache_path, exists_error)) {
        if(exists_error) {
            return std::unexpected(CacheError{
                .message = std::format("failed to inspect extract cache {}: {}",
                                       cache_path->generic_string(), exists_error.message()),
            });
        }
        return std::unordered_map<std::string, CacheRecord>{};
    }

    auto content = clore::support::read_utf8_text_file(*cache_path);
    if(!content.has_value()) {
        return std::unexpected(CacheError{
            .message = std::format("failed to read extract cache {}: {}",
                                   cache_path->generic_string(), content.error()),
        });
    }

    SerializedCacheData data;
    auto status = json::from_json(*content, data);
    if(!status.has_value()) {
        return std::unexpected(CacheError{
            .message = std::format("failed to parse extract cache {}: {}",
                                   cache_path->generic_string(), status.error().to_string()),
        });
    }

    if(data.format_version != kExtractCacheFormatVersion) {
        return std::unexpected(CacheError{
            .message = std::format(
                "unsupported extract cache format version {} in {}",
                data.format_version,
                cache_path->generic_string()),
        });
    }

    std::unordered_map<std::string, CacheRecord> records;
    records.reserve(data.entries.size());

    for(const auto& entry : data.entries) {
        if(entry.source_file >= data.paths.size()) {
            return std::unexpected(CacheError{
                .message = std::format("extract cache entry references invalid source index {}",
                                       entry.source_file),
            });
        }

        auto deps = decode_dependency_snapshot(entry.ast_deps, data.paths);
        if(!deps.has_value()) {
            return std::unexpected(std::move(deps.error()));
        }

        records.emplace(normalize_path_string(data.paths[entry.source_file]), CacheRecord{
            .compile_signature = entry.compile_signature,
            .source_hash = entry.source_hash,
            .ast_deps = std::move(*deps),
            .scan = entry.scan,
            .ast = entry.ast,
        });
    }

    return records;
}

auto save_extract_cache(std::string_view workspace_root,
                        const std::unordered_map<std::string, CacheRecord>& records)
    -> std::expected<void, CacheError> {
    auto cache_root = cache_directory(workspace_root);
    if(!cache_root.has_value()) {
        return std::unexpected(std::move(cache_root.error()));
    }

    std::error_code create_error;
    fs::create_directories(*cache_root, create_error);
    if(create_error) {
        return std::unexpected(CacheError{
            .message = std::format("failed to create extract cache directory {}: {}",
                                   cache_root->generic_string(), create_error.message()),
        });
    }

    SerializedCacheData data;
    std::unordered_map<std::string, std::uint32_t> path_ids;
    path_ids.reserve(records.size());

    auto intern_path = [&](std::string_view value) -> std::uint32_t {
        auto normalized = normalize_path_string(value);
        auto [it, inserted] =
            path_ids.try_emplace(normalized, static_cast<std::uint32_t>(data.paths.size()));
        if(inserted) {
            data.paths.push_back(std::move(normalized));
        }
        return it->second;
    };

    data.entries.reserve(records.size());
    for(const auto& [file, record] : records) {
        data.entries.push_back(SerializedCacheEntry{
            .source_file = intern_path(file),
            .compile_signature = record.compile_signature,
            .source_hash = record.source_hash,
            .ast_deps = encode_dependency_snapshot(record.ast_deps, path_ids, data.paths),
            .scan = record.scan,
            .ast = record.ast,
        });
    }

    auto encoded = json::to_json(data);
    if(!encoded.has_value()) {
        return std::unexpected(CacheError{
            .message = std::format("failed to serialize extract cache: {}",
                                   encoded.error().to_string()),
        });
    }

    auto cache_path = *cache_root / "cache.json";
    auto tmp_path = cache_path;
    tmp_path += ".tmp";

    auto write_result = clore::support::write_utf8_text_file(tmp_path, *encoded);
    if(!write_result.has_value()) {
        return std::unexpected(CacheError{
            .message = std::format("failed to write extract cache {}: {}",
                                   tmp_path.generic_string(), write_result.error()),
        });
    }

    std::error_code rename_error;
    fs::rename(tmp_path, cache_path, rename_error);
    if(rename_error) {
        auto can_retry_replace = [&](const std::error_code& error) {
            return error == std::errc::permission_denied ||
                   error == std::errc::file_exists ||
                   error == std::errc::operation_not_permitted;
        };
        if(!can_retry_replace(rename_error)) {
            return std::unexpected(CacheError{
                .message = std::format("failed to finalize extract cache {}: {}",
                                       cache_path.generic_string(), rename_error.message()),
            });
        }

        std::error_code remove_error;
        fs::remove(cache_path, remove_error);
        if(remove_error && remove_error != std::errc::no_such_file_or_directory) {
            return std::unexpected(CacheError{
                .message = std::format("failed to replace extract cache {}: {}",
                                       cache_path.generic_string(), remove_error.message()),
            });
        }

        rename_error.clear();
        fs::rename(tmp_path, cache_path, rename_error);
        if(rename_error) {
            return std::unexpected(CacheError{
                .message = std::format("failed to finalize extract cache {}: {}",
                                       cache_path.generic_string(), rename_error.message()),
            });
        }
    }

    return {};
}

}  // namespace clore::extract::cache
