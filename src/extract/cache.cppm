module;

#include "kota/codec/json/json.h"

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/xxhash.h"

export module extract:cache;

import std;
import :ast;
import :compiler;
import :scan;
import support;

export namespace clore::extract::cache {

struct CacheError {
    std::string message;
};

struct CacheKeyParts {
    std::string path;
    std::uint64_t compile_signature;
};

struct DependencySnapshot {
    std::vector<std::string> files;
    std::vector<std::uint64_t> hashes;
    std::vector<std::int64_t> mtimes;
    std::int64_t build_at = 0;
};

struct CacheRecord {
    std::uint64_t compile_signature = 0;
    std::uint64_t source_hash = 0;
    DependencySnapshot ast_deps;
    ScanResult scan;
    ASTResult ast;
};

/// clice workspace cache structures: kept schema-compatible with
/// clice/src/server/workspace.cpp CacheData.
struct CliceCacheDepEntry {
    std::uint32_t path = 0;
    std::uint64_t hash = 0;
};

struct CliceCachePCHEntry {
    std::string filename;
    std::uint32_t source_file = 0;
    std::uint64_t hash = 0;
    std::uint32_t bound = 0;
    std::int64_t build_at = 0;
    std::vector<CliceCacheDepEntry> deps;
};

struct CliceCachePCMEntry {
    std::string filename;
    std::uint32_t source_file = 0;
    std::string module_name;
    std::int64_t build_at = 0;
    std::vector<CliceCacheDepEntry> deps;
};

struct CliceCacheData {
    std::vector<std::string> paths;
    std::vector<CliceCachePCHEntry> pch;
    std::vector<CliceCachePCMEntry> pcm;
};

auto build_compile_signature(const CompileEntry& entry) -> std::uint64_t;

auto build_cache_key(std::string_view normalized_path, std::uint64_t compile_signature)
    -> std::string;

auto split_cache_key(std::string_view cache_key) -> std::expected<CacheKeyParts, CacheError>;

auto hash_file(std::string_view path) -> std::expected<std::uint64_t, CacheError>;

auto capture_dependency_snapshot(const std::vector<std::string>& files)
    -> std::expected<DependencySnapshot, CacheError>;

auto dependencies_changed(const DependencySnapshot& snapshot) -> bool;

auto load_extract_cache(std::string_view workspace_root)
    -> std::expected<std::unordered_map<std::string, CacheRecord>, CacheError>;

auto save_extract_cache(std::string_view workspace_root,
                        const std::unordered_map<std::string, CacheRecord>& records)
    -> std::expected<void, CacheError>;

auto load_clice_cache(std::string_view workspace_root) -> std::expected<CliceCacheData, CacheError>;

auto save_clice_cache(std::string_view workspace_root, const CliceCacheData& data)
    -> std::expected<void, CacheError>;

}  // namespace clore::extract::cache

namespace clore::extract::cache {

namespace fs = std::filesystem;
namespace json = kota::codec::json;

namespace {

constexpr std::uint32_t kExtractCacheFormatVersion = 2;
constexpr char kCacheKeyDelimiter = '\t';

struct CachedPathHash {
    std::uint32_t path = 0;
    std::uint64_t hash = 0;
    std::int64_t mtime = 0;
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

auto cache_directory(std::string_view workspace_root) -> std::expected<fs::path, CacheError> {
    if(workspace_root.empty()) {
        return std::unexpected(CacheError{
            .message = "workspace_root must not be empty when resolving extract cache",
        });
    }

    return fs::path(workspace_root) / ".clice" / "cache" / "clore" / "extract";
}

auto cache_file_path(std::string_view workspace_root) -> std::expected<fs::path, CacheError> {
    auto root = cache_directory(workspace_root);
    if(!root.has_value()) {
        return std::unexpected(std::move(root.error()));
    }
    return *root / "cache.json";
}

auto clice_cache_file_path(std::string_view workspace_root) -> std::expected<fs::path, CacheError> {
    if(workspace_root.empty()) {
        return std::unexpected(CacheError{
            .message = "workspace_root must not be empty when resolving clice cache",
        });
    }
    return fs::path(workspace_root) / ".clice" / "cache" / "cache.json";
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
    snapshot.mtimes.reserve(serialized.deps.size());

    for(const auto& dep: serialized.deps) {
        if(dep.path >= paths.size()) {
            return std::unexpected(CacheError{
                .message = std::format("extract cache references invalid path index {}", dep.path),
            });
        }
        snapshot.files.push_back(paths[dep.path]);
        snapshot.hashes.push_back(dep.hash);
        snapshot.mtimes.push_back(dep.mtime);
    }

    return snapshot;
}

auto encode_dependency_snapshot(const DependencySnapshot& snapshot,
                                std::unordered_map<std::string, std::uint32_t>& path_ids,
                                std::vector<std::string>& paths) -> SerializedDependencySnapshot {
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
        auto hash = index < snapshot.hashes.size() ? snapshot.hashes[index] : 0;
        auto mtime = index < snapshot.mtimes.size() ? snapshot.mtimes[index] : 0;
        serialized.deps.push_back(CachedPathHash{
            .path = intern_path(snapshot.files[index]),
            .hash = hash,
            .mtime = mtime,
        });
    }

    return serialized;
}

}  // namespace

auto build_compile_signature(const CompileEntry& entry) -> std::uint64_t {
    return clore::extract::build_compile_signature(entry);
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

auto split_cache_key(std::string_view cache_key) -> std::expected<CacheKeyParts, CacheError> {
    auto delimiter_pos = cache_key.rfind(kCacheKeyDelimiter);
    if(delimiter_pos == std::string_view::npos) {
        return std::unexpected(CacheError{
            .message = std::format("invalid cache key (missing delimiter): {}", cache_key),
        });
    }

    auto path_part = cache_key.substr(0, delimiter_pos);
    auto signature_part = cache_key.substr(delimiter_pos + 1);
    if(path_part.empty() || signature_part.empty()) {
        return std::unexpected(CacheError{
            .message = std::format("invalid cache key (empty path or signature): {}", cache_key),
        });
    }

    std::uint64_t signature = 0;
    auto [ptr, ec] = std::from_chars(signature_part.data(),
                                     signature_part.data() + signature_part.size(),
                                     signature);
    if(ec != std::errc{} || ptr != signature_part.data() + signature_part.size()) {
        return std::unexpected(CacheError{
            .message = std::format("invalid cache key signature in {}", cache_key),
        });
    }

    return CacheKeyParts{
        .path = std::string(path_part),
        .compile_signature = signature,
    };
}

auto hash_file(std::string_view path) -> std::expected<std::uint64_t, CacheError> {
    auto buffer = llvm::MemoryBuffer::getFile(path);
    if(!buffer) {
        return std::unexpected(CacheError{
            .message = std::format("failed to read '{}' for hashing: {}",
                                   path,
                                   buffer.getError().message()),
        });
    }
    return llvm::xxh3_64bits((*buffer)->getBuffer());
}

auto capture_dependency_snapshot(const std::vector<std::string>& files)
    -> std::expected<DependencySnapshot, CacheError> {
    DependencySnapshot snapshot;
    snapshot.build_at = std::chrono::duration_cast<std::chrono::nanoseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    std::vector<std::string> normalized;
    normalized.reserve(files.size());
    for(const auto& file: files) {
        normalized.push_back(normalize_path_string(file));
    }
    std::ranges::sort(normalized);
    normalized.erase(std::unique(normalized.begin(), normalized.end()), normalized.end());

    struct DependencyHashTaskResult {
        std::string file;
        std::uint64_t hash = 0;
        std::int64_t mtime = 0;
        std::string error_message;
    };

    std::vector<DependencyHashTaskResult> task_results(normalized.size());
    std::mutex error_mutex;
    std::optional<CacheError> first_error;

    const auto hardware_threads = std::thread::hardware_concurrency();
    const auto num_threads = std::max(1u, hardware_threads > 0 ? hardware_threads : 1u);
    auto per_thread = (normalized.size() + num_threads - 1) / num_threads;

    auto worker = [&](std::size_t start, std::size_t end) {
        for(std::size_t i = start; i < end; ++i) {
            {
                std::lock_guard lock(error_mutex);
                if(first_error.has_value()) {
                    return;
                }
            }

            const auto& file = normalized[i];
            DependencyHashTaskResult task_result;
            task_result.file = file;

            llvm::sys::fs::file_status status;
            if(!llvm::sys::fs::status(file, status)) {
                task_result.mtime = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                        status.getLastModificationTime().time_since_epoch())
                                        .count();
            }

            auto hash = hash_file(file);
            if(!hash.has_value()) {
                // Dependency file may have been deleted (e.g. generated headers
                // cleaned between runs). Record hash=0 so the cache entry is
                // treated as changed on the next check rather than failing.
                task_result.hash = 0;
                task_results[i] = std::move(task_result);
                continue;
            }
            task_result.hash = *hash;
            task_results[i] = std::move(task_result);
        }
    };

    std::vector<std::thread> threads;
    for(std::size_t t = 0; t < num_threads; ++t) {
        auto start = t * per_thread;
        auto end = std::min(start + per_thread, normalized.size());
        if(start >= end) {
            break;
        }
        threads.emplace_back(worker, start, end);
    }

    for(auto& thread: threads) {
        thread.join();
    }

    if(first_error.has_value()) {
        return std::unexpected(std::move(*first_error));
    }

    snapshot.files.reserve(task_results.size());
    snapshot.hashes.reserve(task_results.size());
    snapshot.mtimes.reserve(task_results.size());
    for(auto& item: task_results) {
        snapshot.files.push_back(std::move(item.file));
        snapshot.hashes.push_back(item.hash);
        snapshot.mtimes.push_back(item.mtime);
    }

    return snapshot;
}

namespace {

auto check_single_dependency(const DependencySnapshot& snapshot, std::size_t index) -> bool {
    const auto& file = snapshot.files[index];
    auto stored_hash = snapshot.hashes[index];

    if(stored_hash == 0) {
        return true;
    }

    llvm::sys::fs::file_status status;
    if(llvm::sys::fs::status(file, status)) {
        return true;
    }

    auto hash = hash_file(file);
    if(!hash.has_value() || *hash != stored_hash) {
        return true;
    }

    return false;
}

}  // namespace

auto dependencies_changed(const DependencySnapshot& snapshot) -> bool {
    if(snapshot.build_at <= 0 || snapshot.files.empty()) {
        return true;
    }

    if(snapshot.files.size() != snapshot.hashes.size() ||
       snapshot.files.size() != snapshot.mtimes.size()) {
        return true;
    }

    // Sequential path for small dependency sets (avoids thread overhead)
    constexpr std::size_t kParallelThreshold = 16;
    if(snapshot.files.size() <= kParallelThreshold) {
        for(std::size_t index = 0; index < snapshot.files.size(); ++index) {
            if(check_single_dependency(snapshot, index)) {
                return true;
            }
        }
        return false;
    }

    // Parallel path for large dependency sets
    const auto hardware_threads = std::thread::hardware_concurrency();
    const auto num_threads = std::max(1u, hardware_threads > 0 ? hardware_threads : 1u);

    std::atomic<bool> changed{false};

    auto worker = [&](std::size_t start, std::size_t end) {
        for(std::size_t i = start; i < end; ++i) {
            if(changed.load(std::memory_order_relaxed)) {
                return;
            }
            if(check_single_dependency(snapshot, i)) {
                changed.store(true, std::memory_order_relaxed);
                return;
            }
        }
    };

    std::vector<std::thread> threads;
    auto per_thread = (snapshot.files.size() + num_threads - 1) / num_threads;
    for(std::size_t t = 0; t < num_threads; ++t) {
        auto start = t * per_thread;
        auto end = std::min(start + per_thread, snapshot.files.size());
        if(start >= end) {
            break;
        }
        threads.emplace_back(worker, start, end);
    }
    for(auto& thread: threads) {
        thread.join();
    }

    return changed.load(std::memory_order_relaxed);
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
                                       cache_path->generic_string(),
                                       exists_error.message()),
            });
        }
        return std::unordered_map<std::string, CacheRecord>{};
    }

    auto content = clore::support::read_utf8_text_file(*cache_path);
    if(!content.has_value()) {
        return std::unexpected(CacheError{
            .message = std::format("failed to read extract cache {}: {}",
                                   cache_path->generic_string(),
                                   content.error()),
        });
    }

    SerializedCacheData data;
    auto status = json::from_json(*content, data);
    if(!status.has_value()) {
        logging::warn("ignoring stale extract cache {}: {}",
                      cache_path->generic_string(),
                      status.error().to_string());
        return std::unordered_map<std::string, CacheRecord>{};
    }

    if(data.format_version != kExtractCacheFormatVersion) {
        logging::warn("ignoring stale extract cache {}: format version {} != {}",
                      cache_path->generic_string(),
                      data.format_version,
                      kExtractCacheFormatVersion);
        return std::unordered_map<std::string, CacheRecord>{};
    }

    std::unordered_map<std::string, CacheRecord> records;
    records.reserve(data.entries.size());

    for(const auto& entry: data.entries) {
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

        auto normalized = normalize_path_string(data.paths[entry.source_file]);
        auto cache_key = build_cache_key(normalized, entry.compile_signature);
        records.emplace(std::move(cache_key),
                        CacheRecord{
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
                                   cache_root->generic_string(),
                                   create_error.message()),
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
    for(const auto& [file, record]: records) {
        auto key_parts = split_cache_key(file);
        if(!key_parts.has_value()) {
            return std::unexpected(std::move(key_parts.error()));
        }
        if(record.compile_signature != key_parts->compile_signature) {
            return std::unexpected(CacheError{
                .message = std::format("cache key signature mismatch for {}", file),
            });
        }

        data.entries.push_back(SerializedCacheEntry{
            .source_file = intern_path(key_parts->path),
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
            .message =
                std::format("failed to serialize extract cache: {}", encoded.error().to_string()),
        });
    }

    auto cache_path = *cache_root / "cache.json";
    auto make_unique_tmp_path = [&](std::uint64_t attempt) -> fs::path {
        auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
                             std::chrono::system_clock::now().time_since_epoch())
                             .count();
        auto pid = llvm::sys::Process::getProcessId();
        auto nonce = llvm::xxh3_64bits(
            std::format("{}:{}:{}:{}", cache_path.generic_string(), pid, timestamp, attempt));

        auto candidate = cache_path;
        candidate += std::format(".tmp.{}.{}.{}", pid, timestamp, nonce);
        return candidate;
    };

    fs::path tmp_path;
    for(std::uint64_t attempt = 0; attempt < 32; ++attempt) {
        auto candidate = make_unique_tmp_path(attempt);
        std::error_code exists_error;
        auto exists = fs::exists(candidate, exists_error);
        if(exists_error) {
            return std::unexpected(CacheError{
                .message = std::format("failed to prepare extract cache temp file {}: {}",
                                       candidate.generic_string(),
                                       exists_error.message()),
            });
        }
        if(!exists) {
            tmp_path = std::move(candidate);
            break;
        }
    }
    if(tmp_path.empty()) {
        return std::unexpected(CacheError{
            .message = std::format("failed to allocate a unique temp file path near {}",
                                   cache_path.generic_string()),
        });
    }

    auto write_result = clore::support::write_utf8_text_file(tmp_path, *encoded);
    if(!write_result.has_value()) {
        return std::unexpected(CacheError{
            .message = std::format("failed to write extract cache {}: {}",
                                   tmp_path.generic_string(),
                                   write_result.error()),
        });
    }

    std::error_code rename_error;
    fs::rename(tmp_path, cache_path, rename_error);
    if(rename_error == std::errc::permission_denied || rename_error == std::errc::file_exists ||
       rename_error == std::errc::operation_not_permitted) {
        std::error_code remove_error;
        fs::remove(cache_path, remove_error);
        if(remove_error && remove_error != std::errc::no_such_file_or_directory) {
            return std::unexpected(CacheError{
                .message = std::format("failed to replace extract cache {}: {}",
                                       cache_path.generic_string(),
                                       remove_error.message()),
            });
        }

        rename_error.clear();
        fs::rename(tmp_path, cache_path, rename_error);
    }
    if(rename_error) {
        return std::unexpected(CacheError{
            .message = std::format("failed to finalize extract cache {}: {}",
                                   cache_path.generic_string(),
                                   rename_error.message()),
        });
    }

    return {};
}

auto load_clice_cache(std::string_view workspace_root)
    -> std::expected<CliceCacheData, CacheError> {
    auto cache_path = clice_cache_file_path(workspace_root);
    if(!cache_path.has_value()) {
        return std::unexpected(std::move(cache_path.error()));
    }

    std::error_code exists_error;
    if(!fs::exists(*cache_path, exists_error)) {
        if(exists_error) {
            return std::unexpected(CacheError{
                .message = std::format("failed to inspect clice cache {}: {}",
                                       cache_path->generic_string(),
                                       exists_error.message()),
            });
        }
        return CliceCacheData{};
    }

    auto content = clore::support::read_utf8_text_file(*cache_path);
    if(!content.has_value()) {
        return std::unexpected(CacheError{
            .message = std::format("failed to read clice cache {}: {}",
                                   cache_path->generic_string(),
                                   content.error()),
        });
    }

    CliceCacheData data;
    auto status = json::from_json(*content, data);
    if(!status.has_value()) {
        logging::warn("ignoring stale clice cache {}: {}",
                      cache_path->generic_string(),
                      status.error().to_string());
        return CliceCacheData{};
    }

    return data;
}

auto save_clice_cache(std::string_view workspace_root, const CliceCacheData& data)
    -> std::expected<void, CacheError> {
    auto cache_path = clice_cache_file_path(workspace_root);
    if(!cache_path.has_value()) {
        return std::unexpected(std::move(cache_path.error()));
    }

    auto parent = cache_path->parent_path();
    std::error_code create_error;
    fs::create_directories(parent, create_error);
    if(create_error) {
        return std::unexpected(CacheError{
            .message = std::format("failed to create clice cache directory {}: {}",
                                   parent.generic_string(),
                                   create_error.message()),
        });
    }

    auto encoded = json::to_json(data);
    if(!encoded.has_value()) {
        return std::unexpected(CacheError{
            .message =
                std::format("failed to serialize clice cache: {}", encoded.error().to_string()),
        });
    }

    auto make_unique_tmp_path = [&](std::uint64_t attempt) -> fs::path {
        auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
                             std::chrono::system_clock::now().time_since_epoch())
                             .count();
        auto pid = llvm::sys::Process::getProcessId();
        auto nonce = llvm::xxh3_64bits(
            std::format("{}:{}:{}:{}", cache_path->generic_string(), pid, timestamp, attempt));

        auto candidate = *cache_path;
        candidate += std::format(".tmp.{}.{}.{}", pid, timestamp, nonce);
        return candidate;
    };

    fs::path tmp_path;
    for(std::uint64_t attempt = 0; attempt < 32; ++attempt) {
        auto candidate = make_unique_tmp_path(attempt);
        std::error_code exists_error;
        auto exists = fs::exists(candidate, exists_error);
        if(exists_error) {
            return std::unexpected(CacheError{
                .message = std::format("failed to prepare clice cache temp file {}: {}",
                                       candidate.generic_string(),
                                       exists_error.message()),
            });
        }
        if(!exists) {
            tmp_path = std::move(candidate);
            break;
        }
    }
    if(tmp_path.empty()) {
        return std::unexpected(CacheError{
            .message = std::format("failed to allocate a unique temp file path near {}",
                                   cache_path->generic_string()),
        });
    }

    auto write_result = clore::support::write_utf8_text_file(tmp_path, *encoded);
    if(!write_result.has_value()) {
        return std::unexpected(CacheError{
            .message = std::format("failed to write clice cache temp file {}: {}",
                                   tmp_path.generic_string(),
                                   write_result.error()),
        });
    }

    std::error_code rename_error;
    fs::rename(tmp_path, *cache_path, rename_error);
    if(rename_error == std::errc::permission_denied || rename_error == std::errc::file_exists ||
       rename_error == std::errc::operation_not_permitted) {
        std::error_code remove_error;
        fs::remove(*cache_path, remove_error);
        if(remove_error && remove_error != std::errc::no_such_file_or_directory) {
            return std::unexpected(CacheError{
                .message = std::format("failed to replace clice cache {}: {}",
                                       cache_path->generic_string(),
                                       remove_error.message()),
            });
        }

        rename_error.clear();
        fs::rename(tmp_path, *cache_path, rename_error);
    }
    if(rename_error) {
        return std::unexpected(CacheError{
            .message = std::format("failed to finalize clice cache {}: {}",
                                   cache_path->generic_string(),
                                   rename_error.message()),
        });
    }

    return {};
}

}  // namespace clore::extract::cache
