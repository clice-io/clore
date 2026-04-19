module;

#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/Support/xxhash.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Driver/CreateInvocationFromArgs.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/Utils.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/JSONCompilationDatabase.h"

export module extract:compiler;

import std;
import support;

export namespace clore::extract {

struct CompileEntry {
    std::string file{};
    std::string directory{};
    std::vector<std::string> arguments{};
    std::string normalized_file{};
    std::uint64_t compile_signature = 0;
    std::optional<std::uint64_t> source_hash{};
    std::string cache_key{};
};

struct CompilationDatabase {
    std::vector<CompileEntry> entries{};
    std::unordered_map<std::string, std::vector<std::string>> toolchain_cache{};

    auto has_cached_toolchain() const -> bool;
};

struct CompDbError {
    std::string message;
};

auto load_compdb(std::string_view path) -> std::expected<CompilationDatabase, CompDbError>;

auto lookup(const CompilationDatabase& db, std::string_view file)
    -> std::vector<const CompileEntry*>;

auto strip_compiler_path(const std::vector<std::string>& args) -> std::vector<std::string>;

auto normalize_argument_path(std::string_view path, std::string_view directory)
    -> std::filesystem::path;

auto sanitize_driver_arguments(const CompileEntry& entry) -> std::vector<std::string>;

auto sanitize_tool_arguments(const CompileEntry& entry) -> std::vector<std::string>;

auto normalize_entry_file(const CompileEntry& entry) -> std::string;

auto build_compile_signature(const CompileEntry& entry) -> std::uint64_t;

auto ensure_cache_key(CompileEntry& entry) -> void;

auto query_toolchain_cached(CompilationDatabase& db, const CompileEntry& entry)
    -> std::vector<std::string>;

auto create_compiler_instance(const CompileEntry& entry)
    -> std::unique_ptr<clang::CompilerInstance>;

}  // namespace clore::extract

namespace clore::extract {

namespace {

auto build_compile_signature_impl(const CompileEntry& entry, std::string_view normalized_file)
    -> std::uint64_t {
    return clore::support::build_compile_signature(entry.directory,
                                                   normalized_file,
                                                   entry.arguments);
}

auto try_hash_source_file(std::string_view normalized_file) -> std::optional<std::uint64_t> {
    auto buffer = llvm::MemoryBuffer::getFile(normalized_file);
    if(!buffer) {
        return std::nullopt;
    }
    return llvm::xxh3_64bits((*buffer)->getBuffer());
}

}  // namespace

auto normalize_entry_file(const CompileEntry& entry) -> std::string {
    namespace fs = std::filesystem;
    auto path = fs::path(entry.file);
    if(path.is_relative()) {
        path = fs::path(entry.directory) / path;
    }
    std::error_code ec;
    auto absolute = fs::absolute(path, ec);
    if(!ec) {
        path = std::move(absolute);
    }
    path = path.lexically_normal();
    auto canonical = fs::weakly_canonical(path, ec);
    if(!ec) {
        return canonical.generic_string();
    }
    return path.generic_string();
}

auto build_compile_signature(const CompileEntry& entry) -> std::uint64_t {
    if(!entry.normalized_file.empty() && entry.compile_signature != 0) {
        return entry.compile_signature;
    }
    auto normalized_file =
        entry.normalized_file.empty() ? normalize_entry_file(entry) : entry.normalized_file;
    return build_compile_signature_impl(entry, normalized_file);
}

auto ensure_cache_key_impl(CompileEntry& entry) -> void {
    entry.normalized_file = normalize_entry_file(entry);
    entry.compile_signature = build_compile_signature_impl(entry, entry.normalized_file);
    entry.source_hash = try_hash_source_file(entry.normalized_file);
    entry.cache_key =
        clore::support::build_cache_key(entry.normalized_file, entry.compile_signature);
}

auto load_compdb(std::string_view path) -> std::expected<CompilationDatabase, CompDbError> {
    namespace fs = std::filesystem;

    auto compdb_path = fs::path(path);
    if(!fs::exists(compdb_path)) {
        return std::unexpected(
            CompDbError{.message = std::format("compile_commands.json not found: {}", path)});
    }

    std::string error_message;
    auto json_db = clang::tooling::JSONCompilationDatabase::loadFromFile(
        std::string(path),
        error_message,
        clang::tooling::JSONCommandLineSyntax::AutoDetect);

    if(!json_db) {
        return std::unexpected(CompDbError{
            .message = std::format("failed to load compile_commands.json: {}", error_message)});
    }

    CompilationDatabase db;
    for(auto& cmd: json_db->getAllCompileCommands()) {
        CompileEntry entry;
        entry.file = cmd.Filename;
        entry.directory = cmd.Directory;
        entry.arguments.reserve(cmd.CommandLine.size());
        for(auto& arg: cmd.CommandLine) {
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

    for(auto& entry: db.entries) {
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
    std::error_code ec;
    auto absolute = std::filesystem::absolute(normalized, ec);
    if(!ec) {
        normalized = std::move(absolute);
    }
    normalized = normalized.lexically_normal();
    auto canonical = std::filesystem::weakly_canonical(normalized, ec);
    if(!ec) {
        return canonical;
    }
    return normalized;
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

auto ensure_cache_key(CompileEntry& entry) -> void {
    ensure_cache_key_impl(entry);
}

auto CompilationDatabase::has_cached_toolchain() const -> bool {
    return !toolchain_cache.empty();
}

auto query_toolchain_cached(CompilationDatabase& db, const CompileEntry& entry)
    -> std::vector<std::string> {
    if(entry.arguments.empty()) {
        return {};
    }

    auto key = entry.cache_key;
    if(key.empty()) {
        auto copy = entry;
        ensure_cache_key(copy);
        key = std::move(copy.cache_key);
    }

    if(auto it = db.toolchain_cache.find(key); it != db.toolchain_cache.end()) {
        return it->second;
    }

    auto sanitized = sanitize_tool_arguments(entry);
    db.toolchain_cache.insert_or_assign(key, sanitized);
    return sanitized;
}

namespace {

// Thread-local cache of parsed CompilerInvocation keyed by compile signature.
// This avoids re-parsing command-line arguments for files that share the same
// compile configuration (same directory, include paths, defines, etc.).
auto invocation_cache()
    -> std::unordered_map<std::uint64_t, std::shared_ptr<clang::CompilerInvocation>>& {
    thread_local std::unordered_map<std::uint64_t, std::shared_ptr<clang::CompilerInvocation>>
        cache;
    return cache;
}

}  // namespace

auto create_compiler_instance(const CompileEntry& entry)
    -> std::unique_ptr<clang::CompilerInstance> {
    auto driver_args = sanitize_driver_arguments(entry);
    if(driver_args.empty()) {
        return nullptr;
    }

    llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> vfs = llvm::vfs::createPhysicalFileSystem();

    clang::DiagnosticOptions diag_opts;
    auto diagnostics = clang::CompilerInstance::createDiagnostics(*vfs,
                                                                  diag_opts,
                                                                  new clang::IgnoringDiagConsumer(),
                                                                  true);
    if(!diagnostics) {
        return nullptr;
    }

    std::shared_ptr<clang::CompilerInvocation> invocation;

    // Try to reuse a cached invocation for the same compile signature.
    auto sig = entry.compile_signature;
    if(sig != 0) {
        auto& cache = invocation_cache();
        auto cached_it = cache.find(sig);
        if(cached_it != cache.end() && cached_it->second) {
            // Clone the cached invocation; all options are identical for the
            // same compile signature (same directory, flags, etc.).
            invocation = std::make_shared<clang::CompilerInvocation>(*cached_it->second);
        }
    }

    if(!invocation) {
        // Parse invocation from command-line arguments.
        llvm::SmallVector<const char*> argv;
        argv.reserve(driver_args.size() + 1);
        for(auto& arg: driver_args) {
            argv.push_back(arg.c_str());
        }
        // Append a placeholder input; the real input is set later.
        argv.push_back(entry.file.c_str());

        const bool is_cc1 = argv.size() >= 2 && llvm::StringRef(argv[1]) == "-cc1";
        if(is_cc1) {
            invocation = std::make_shared<clang::CompilerInvocation>();
            if(!clang::CompilerInvocation::CreateFromArgs(*invocation,
                                                          llvm::ArrayRef(argv).drop_front(2),
                                                          *diagnostics,
                                                          argv[0])) {
                return nullptr;
            }
        } else {
            clang::CreateInvocationOptions options{
                .Diags = diagnostics,
                .VFS = vfs,
                .ProbePrecompiled = false,
            };
            auto unique_invocation = clang::createInvocation(argv, options);
            if(!unique_invocation) {
                return nullptr;
            }
            invocation = std::shared_ptr<clang::CompilerInvocation>(std::move(unique_invocation));
            if(!invocation) {
                return nullptr;
            }
        }

        invocation->getFrontendOpts().DisableFree = false;
        invocation->getFileSystemOpts().WorkingDir = entry.directory;

        if(sig != 0) {
            invocation_cache()[sig] = invocation;
        }
    }

    // Reset input to the specific file for this entry.
    invocation->getFrontendOpts().Inputs.clear();
    invocation->getFrontendOpts().Inputs.push_back(
        clang::FrontendInputFile(entry.file, clang::InputKind(clang::Language::CXX)));

    auto instance = std::make_unique<clang::CompilerInstance>(invocation);
    instance->setVirtualFileSystem(vfs);
    instance->createDiagnostics(new clang::IgnoringDiagConsumer(), true);
    instance->createFileManager();

    if(!instance->createTarget()) {
        return nullptr;
    }

    return instance;
}

}  // namespace clore::extract
