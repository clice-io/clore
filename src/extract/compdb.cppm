module;

#include <algorithm>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>
#include <vector>

#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/JSONCompilationDatabase.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/xxhash.h"

export module extract:compdb;

import support;

export namespace clore::extract {

struct CompileEntry {
    std::string file;
    std::string directory;
    std::vector<std::string> arguments;
    std::string cache_key;
};

struct CompilationDatabase {
    std::vector<CompileEntry> entries;
};

struct CompDbError {
    std::string message;
};

auto load_compdb(std::string_view path) -> std::expected<CompilationDatabase, CompDbError>;

auto lookup(const CompilationDatabase& db, std::string_view file)
    -> std::vector<const CompileEntry*>;

// ── argument utilities ──────────────────────────────────────────────

auto strip_compiler_path(const std::vector<std::string>& args) -> std::vector<std::string>;

auto normalize_argument_path(std::string_view path, std::string_view directory)
    -> std::filesystem::path;

auto sanitize_driver_arguments(const CompileEntry& entry) -> std::vector<std::string>;

auto sanitize_tool_arguments(const CompileEntry& entry) -> std::vector<std::string>;

}  // namespace clore::extract

// ── implementation ──────────────────────────────────────────────────

namespace clore::extract {

namespace {

auto normalize_path_string(std::string_view path) -> std::string {
    return std::filesystem::path(path).lexically_normal().generic_string();
}

auto normalize_entry_file(const CompileEntry& entry) -> std::string {
    namespace fs = std::filesystem;
    auto path = fs::path(entry.file);
    if(path.is_relative()) {
        path = fs::path(entry.directory) / path;
    }
    return path.lexically_normal().generic_string();
}

auto build_compile_signature(const CompileEntry& entry, std::string_view normalized_file)
    -> std::uint64_t {
    std::string payload;
    payload.reserve(entry.directory.size() + normalized_file.size() + entry.arguments.size() * 16);
    payload.append(normalize_path_string(entry.directory));
    payload.push_back('\0');
    payload.append(normalized_file);
    payload.push_back('\0');
    for(const auto& argument : entry.arguments) {
        payload.append(argument);
        payload.push_back('\0');
    }
    return llvm::xxh3_64bits(payload);
}

auto ensure_cache_key(CompileEntry& entry) -> void {
    auto normalized_file = normalize_entry_file(entry);
    auto signature = build_compile_signature(entry, normalized_file);
    entry.cache_key = normalized_file + "\t" + std::to_string(signature);
}

}  // namespace

auto load_compdb(std::string_view path) -> std::expected<CompilationDatabase, CompDbError> {
    namespace fs = std::filesystem;

    auto compdb_path = fs::path(path);
    if(!fs::exists(compdb_path)) {
        return std::unexpected(CompDbError{
            .message = std::format("compile_commands.json not found: {}", path)});
    }

    std::string error_message;
    auto json_db = clang::tooling::JSONCompilationDatabase::loadFromFile(
        std::string(path), error_message, clang::tooling::JSONCommandLineSyntax::AutoDetect);

    if(!json_db) {
        return std::unexpected(CompDbError{
            .message = std::format("failed to load compile_commands.json: {}", error_message)});
    }

    CompilationDatabase db;
    for(auto& cmd : json_db->getAllCompileCommands()) {
        CompileEntry entry;
        entry.file = cmd.Filename;
        entry.directory = cmd.Directory;
        entry.arguments.reserve(cmd.CommandLine.size());
        for(auto& arg : cmd.CommandLine) {
            entry.arguments.push_back(arg);
        }
        ensure_cache_key(entry);
        db.entries.push_back(std::move(entry));
    }

    logging::info("loaded {} compile commands from {}", db.entries.size(), path);
    return db;
}

auto lookup(const CompilationDatabase& db, std::string_view file)
    -> std::vector<const CompileEntry*> {
    std::vector<const CompileEntry*> results;

    namespace fs = std::filesystem;
    auto target = fs::path(file).lexically_normal();

    for(auto& entry : db.entries) {
        auto entry_path = (fs::path(entry.directory) / entry.file).lexically_normal();
        if(entry_path == target) {
            results.push_back(&entry);
        }
    }
    return results;
}

auto strip_compiler_path(const std::vector<std::string>& args) -> std::vector<std::string> {
    if(args.size() <= 1) {
        return {};
    }
    return std::vector<std::string>(args.begin() + 1, args.end());
}

auto normalize_argument_path(std::string_view path, std::string_view directory)
    -> std::filesystem::path {
    auto normalized = std::filesystem::path(path);
    if(normalized.is_relative()) {
        normalized = std::filesystem::path(directory) / normalized;
    }
    return normalized.lexically_normal();
}

auto sanitize_driver_arguments(const CompileEntry& entry) -> std::vector<std::string> {
    auto adjusted = entry.arguments;
    auto source_path = normalize_argument_path(entry.file, entry.directory);

    std::erase_if(adjusted, [&](const std::string& arg) {
        if(arg.empty() || arg.starts_with('-')) {
            return false;
        }
        return normalize_argument_path(arg, entry.directory) == source_path;
    });

    return adjusted;
}

auto sanitize_tool_arguments(const CompileEntry& entry) -> std::vector<std::string> {
    return strip_compiler_path(sanitize_driver_arguments(entry));
}

}  // namespace clore::extract
