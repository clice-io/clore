module;

#include "kota/async/async.h"

#include "llvm/Support/Error.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/DependencyDirectivesScanner.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"

export module extract:scan;

import std;
import :compiler;
import support;

export namespace clore::extract {

struct ScanError {
    std::string message;
};

struct IncludeInfo {
    std::string path{};
    bool is_angled = false;
};

struct ScanResult {
    std::string module_name{};
    bool is_interface_unit = false;
    std::vector<IncludeInfo> includes{};
    std::vector<std::string> module_imports{};
};

/// Persistent scan cache shared across successive dependency scans.
///
/// Invalidation: callers should clear/discard this cache when compilation DB
/// or file system state changes.
struct ScanCache {
    std::unordered_map<std::string, ScanResult> scan_results{};
};

auto scan_file(const CompileEntry& entry) -> std::expected<ScanResult, ScanError>;

/// Fast module declaration scan using Clang's dependency directives scanner.
/// Populates module_name, is_interface_unit, and module_imports in ScanResult
/// without running the full preprocessor.
auto scan_module_decl(std::string_view file_content, ScanResult& result) -> void;

struct DependencyEdge {
    std::string from;
    std::string to;
};

struct DependencyGraph {
    std::vector<std::string> files{};
    std::vector<DependencyEdge> edges{};
};

auto build_dependency_graph_async(const CompilationDatabase& db,
                                  DependencyGraph& graph,
                                  ScanCache* cache,
                                  kota::event_loop& loop) -> kota::task<void, ScanError>;

auto topological_order(const DependencyGraph& graph)
    -> std::expected<std::vector<std::string>, ScanError>;

}  // namespace clore::extract

// ── implementation ──────────────────────────────────────────────────

namespace clore::extract {

namespace {

auto normalize_partition_import(std::string_view current_module_name, std::string import_name)
    -> std::string {
    if(import_name.starts_with(':') && !current_module_name.empty()) {
        auto main_name = current_module_name;
        if(auto colon_pos = current_module_name.find(':'); colon_pos != std::string::npos) {
            main_name = current_module_name.substr(0, colon_pos);
        }

        std::string normalized;
        normalized.reserve(main_name.size() + import_name.size());
        normalized += main_name;
        normalized += import_name;
        return normalized;
    }

    return import_name;
}

class ScanPPCallbacks : public clang::PPCallbacks {
public:
    ScanResult& result;

    explicit ScanPPCallbacks(ScanResult& result) : result(result) {}

    void InclusionDirective(clang::SourceLocation /*hash_loc*/,
                            const clang::Token& /*include_tok*/,
                            llvm::StringRef file_name,
                            bool is_angled,
                            clang::CharSourceRange /*filename_range*/,
                            clang::OptionalFileEntryRef file,
                            llvm::StringRef /*search_path*/,
                            llvm::StringRef /*relative_path*/,
                            const clang::Module* /*suggested_module*/,
                            bool /*module_imported*/,
                            clang::SrcMgr::CharacteristicKind /*file_type*/) override {
        IncludeInfo info;
        if(file) {
            info.path = file->getFileEntry().tryGetRealPathName().str();
            if(info.path.empty()) {
                info.path = file->getName().str();
            }
        } else {
            info.path = file_name.str();
        }
        info.is_angled = is_angled;
        result.includes.push_back(std::move(info));
    }
};

class ScanAction : public clang::PreprocessOnlyAction {
public:
    ScanResult& result;

    explicit ScanAction(ScanResult& result) : result(result) {}

    void ExecuteAction() override {
        auto& pp = getCompilerInstance().getPreprocessor();
        pp.addPPCallbacks(std::make_unique<ScanPPCallbacks>(result));
        clang::PreprocessOnlyAction::ExecuteAction();
    }
};

}  // namespace

auto scan_module_decl(std::string_view file_content, ScanResult& result) -> void {
    // Use Clang's dependency directives scanner for fast module detection.
    llvm::SmallVector<clang::dependency_directives_scan::Token, 64> tokens;
    llvm::SmallVector<clang::dependency_directives_scan::Directive, 16> directives;

    if(clang::scanSourceForDependencyDirectives(file_content, tokens, directives)) {
        // Scanner failed; fall back to no module detection.
        return;
    }

    namespace dds = clang::dependency_directives_scan;
    auto is_whitespace_only = [](std::string_view text) -> bool {
        return !text.empty() && std::ranges::all_of(text, [](char ch) {
            return std::isspace(static_cast<unsigned char>(ch)) != 0;
        });
    };
    auto is_punctuation_only = [](std::string_view text) -> bool {
        return !text.empty() && std::ranges::all_of(text, [](char ch) {
            return std::ispunct(static_cast<unsigned char>(ch)) != 0;
        });
    };

    for(auto& dir: directives) {
        if(dir.Kind == dds::cxx_export_module_decl || dir.Kind == dds::cxx_module_decl) {
            // Collect module name from tokens: identifiers + '.' + ':'
            std::string module_name;

            // Skip 'export' and 'module' keywords
            bool past_module_keyword = false;
            bool saw_first_name_token = false;
            bool is_fragment = false;
            for(auto& tok: dir.Tokens) {
                auto tok_text = file_content.substr(tok.Offset, tok.Length);
                if(is_whitespace_only(tok_text)) {
                    continue;
                }

                if(!past_module_keyword) {
                    if(tok_text == "module") {
                        past_module_keyword = true;
                    }
                    continue;
                }

                if(!saw_first_name_token) {
                    saw_first_name_token = true;
                    if(tok_text == ";" || tok_text == ":" || tok_text.starts_with(':') ||
                       is_punctuation_only(tok_text)) {
                        is_fragment = true;
                        break;
                    }
                }

                // Stop at semicolon or end
                if(tok_text == ";")
                    break;

                module_name += tok_text;
            }

            if(!is_fragment && !module_name.empty()) {
                result.module_name = std::move(module_name);
                result.is_interface_unit = (dir.Kind == dds::cxx_export_module_decl);
            }
        } else if(dir.Kind == dds::cxx_import_decl) {
            // Collect import name
            std::string import_name;

            bool past_import_keyword = false;
            for(auto& tok: dir.Tokens) {
                auto tok_text = file_content.substr(tok.Offset, tok.Length);

                if(!past_import_keyword) {
                    if(tok_text == "import") {
                        past_import_keyword = true;
                    }
                    continue;
                }

                if(tok_text == ";")
                    break;

                import_name += tok_text;
            }

            if(!import_name.empty()) {
                auto normalized_import =
                    normalize_partition_import(result.module_name, std::move(import_name));
                if(std::ranges::find(result.module_imports, normalized_import) ==
                   result.module_imports.end()) {
                    result.module_imports.push_back(std::move(normalized_import));
                }
            }
        }
    }
}

auto scan_file(const CompileEntry& entry) -> std::expected<ScanResult, ScanError> {
    if(entry.arguments.empty()) {
        return std::unexpected(
            ScanError{.message = std::format("empty argument list for file: {}", entry.file)});
    }

    ScanResult result;

    // Fast module declaration scan from file content
    {
        namespace fs = std::filesystem;
        auto normalized =
            entry.normalized_file.empty() ? normalize_entry_file(entry) : entry.normalized_file;
        auto file_path = fs::path(normalized);
        std::ifstream ifs(file_path);
        if(ifs.is_open()) {
            std::string content((std::istreambuf_iterator<char>(ifs)),
                                std::istreambuf_iterator<char>());
            if(ifs.bad() || (!ifs.eof() && ifs.fail())) {
                return std::unexpected(
                    ScanError{.message = std::format("failed to read source file: {}",
                                                     file_path.generic_string())});
            }
            scan_module_decl(content, result);
        }
    }

    auto instance = create_compiler_instance(entry);
    if(!instance) {
        return std::unexpected(ScanError{
            .message = std::format("failed to create compiler instance for file: {}", entry.file)});
    }
    auto& frontend_opts = instance->getInvocation().getFrontendOpts();
    frontend_opts.ProgramAction = clang::frontend::RunPreprocessorOnly;
    frontend_opts.OutputFile.clear();
    frontend_opts.ModuleOutputPath.clear();

    auto action = std::make_unique<ScanAction>(result);
    if(!action->BeginSourceFile(*instance, instance->getFrontendOpts().Inputs[0])) {
        return std::unexpected(ScanError{
            .message = std::format("failed to begin preprocessing for file: {}", entry.file)});
    }

    if(auto error = action->Execute()) {
        llvm::consumeError(std::move(error));
        action->EndSourceFile();
        return std::unexpected(
            ScanError{.message = std::format("preprocessor scan failed for file: {}", entry.file)});
    }

    action->EndSourceFile();
    return result;
}

namespace {

struct PreparedScanEntry {
    std::string normalized_file{};
    std::string cache_key{};
};

auto prepare_scan_entry(const CompileEntry& entry) -> PreparedScanEntry {
    auto normalized_file =
        entry.normalized_file.empty() ? normalize_entry_file(entry) : entry.normalized_file;
    auto cache_key = entry.cache_key;
    if(cache_key.empty()) {
        auto compile_signature = entry.compile_signature != 0
                                     ? entry.compile_signature
                                     : clore::support::build_compile_signature(entry.directory,
                                                                               normalized_file,
                                                                               entry.arguments);
        cache_key = clore::support::build_cache_key(normalized_file, compile_signature);
    }

    return PreparedScanEntry{
        .normalized_file = std::move(normalized_file),
        .cache_key = std::move(cache_key),
    };
}

struct MissingScanTask {
    const CompileEntry* entry = nullptr;
};

auto run_scan_task(MissingScanTask task, kota::event_loop& loop)
    -> kota::task<ScanResult, ScanError> {
    if(task.entry == nullptr) {
        co_await kota::fail(ScanError{.message = "missing dependency scan task entry"});
    }

    auto file = task.entry->file;
    auto scanned =
        co_await kota::queue(
            [entry = *task.entry, file]() -> std::expected<ScanResult, ScanError> {
                try {
                    return scan_file(entry);
                } catch(const std::exception& ex) {
                    return std::unexpected(ScanError{
                        .message =
                            std::format("dependency scan threw for file '{}': {}", file, ex.what()),
                    });
                } catch(...) {
                    return std::unexpected(ScanError{
                        .message =
                            std::format("dependency scan threw unknown exception for file '{}'",
                                        file),
                    });
                }
            },
            loop)
            .catch_cancel();
    if(scanned.is_cancelled()) {
        co_await kota::fail(ScanError{
            .message = std::format("dependency scan cancelled for file: {}", file),
        });
    }
    if(scanned.has_error()) {
        co_await kota::fail(ScanError{
            .message = std::format("dependency scan task failed for file '{}': {}",
                                   file,
                                   scanned.error().message()),
        });
    }
    if(!scanned->has_value()) {
        co_await kota::fail(std::move(scanned->error()));
    }

    co_return std::move(**scanned);
}

}  // namespace

auto build_dependency_graph_async(const CompilationDatabase& db,
                                  DependencyGraph& graph,
                                  ScanCache* cache,
                                  kota::event_loop& loop) -> kota::task<void, ScanError> {
    graph.files.clear();
    graph.edges.clear();

    std::unordered_set<std::string> entry_files;
    std::unordered_set<std::string> file_set;
    std::vector<PreparedScanEntry> prepared_entries;
    prepared_entries.reserve(db.entries.size());

    for(const auto& entry: db.entries) {
        auto prepared = prepare_scan_entry(entry);
        entry_files.insert(prepared.normalized_file);
        if(file_set.insert(prepared.normalized_file).second)
            graph.files.push_back(prepared.normalized_file);
        prepared_entries.push_back(std::move(prepared));
    }

    auto* scan_results = cache ? &cache->scan_results : nullptr;
    std::vector<std::optional<ScanResult>> cached_results(db.entries.size());
    std::vector<std::size_t> missing_task_indices(db.entries.size(),
                                                  std::numeric_limits<std::size_t>::max());
    std::unordered_map<std::string, std::size_t> missing_task_by_cache_key;
    missing_task_by_cache_key.reserve(db.entries.size());
    std::vector<MissingScanTask> missing_tasks;
    missing_tasks.reserve(db.entries.size());

    for(std::size_t idx = 0; idx < db.entries.size(); ++idx) {
        const auto& entry = db.entries[idx];
        const auto& cache_key = prepared_entries[idx].cache_key;

        if(scan_results) {
            auto cached_it = scan_results->find(cache_key);
            if(cached_it != scan_results->end()) {
                cached_results[idx] = cached_it->second;
                continue;
            }
        }

        auto [task_it, inserted] =
            missing_task_by_cache_key.try_emplace(cache_key, missing_tasks.size());
        if(inserted) {
            missing_tasks.push_back(MissingScanTask{
                .entry = &entry,
            });
        }
        missing_task_indices[idx] = task_it->second;
    }

    std::vector<ScanResult> scanned_results(missing_tasks.size());
    if(!missing_tasks.empty()) {
        std::vector<kota::task<ScanResult, ScanError>> tasks;
        tasks.reserve(missing_tasks.size());
        for(auto task: missing_tasks) {
            tasks.push_back(run_scan_task(task, loop));
        }

        auto all_result = co_await kota::when_all(std::move(tasks));
        if(all_result.has_error()) {
            co_await kota::fail(std::move(all_result.error()));
        }
        if(all_result->size() != missing_tasks.size()) {
            co_await kota::fail(ScanError{
                .message = std::format("dependency scan result count mismatch: expected {}, got {}",
                                       missing_tasks.size(),
                                       all_result->size()),
            });
        }
        for(std::size_t i = 0; i < all_result->size(); ++i) {
            scanned_results[i] = std::move((*all_result)[i]);
        }
    }

    std::unordered_set<std::string> seen_files;
    seen_files.reserve(db.entries.size());
    std::unordered_set<std::string> emitted_edges;
    emitted_edges.reserve(db.entries.size());

    for(std::size_t idx = 0; idx < db.entries.size(); ++idx) {
        const auto& entry = db.entries[idx];
        const auto& normalized = prepared_entries[idx].normalized_file;
        const auto& cache_key = prepared_entries[idx].cache_key;

        const ScanResult* scan_result = nullptr;
        if(cached_results[idx].has_value()) {
            scan_result = &*cached_results[idx];
        } else {
            auto task_index = missing_task_indices[idx];
            if(task_index >= scanned_results.size()) {
                co_await kota::fail(ScanError{
                    .message =
                        std::format("missing dependency scan result for file: {}", normalized),
                });
            }
            scan_result = &scanned_results[task_index];
        }

        if(seen_files.insert(normalized).second) {
            for(auto& inc: scan_result->includes) {
                auto inc_normalized =
                    normalize_argument_path(inc.path, entry.directory).generic_string();
                if(entry_files.contains(inc_normalized)) {
                    auto edge_key = normalized;
                    edge_key.push_back('\0');
                    edge_key += inc_normalized;
                    if(emitted_edges.insert(std::move(edge_key)).second) {
                        graph.edges.push_back(DependencyEdge{
                            .from = normalized,
                            .to = inc_normalized,
                        });
                    }
                }
            }
        }

        if(scan_results) {
            scan_results->insert_or_assign(cache_key, *scan_result);
        }
    }

    co_return;
}

auto topological_order(const DependencyGraph& graph)
    -> std::expected<std::vector<std::string>, ScanError> {
    std::unordered_map<std::string, std::vector<std::string>> adj;
    std::unordered_map<std::string, int> in_degree;

    for(auto& f: graph.files) {
        in_degree[f] = 0;
    }

    for(auto& edge: graph.edges) {
        adj[edge.to].push_back(edge.from);
        in_degree[edge.from]++;
    }

    auto order = clore::support::topological_order(graph.files, adj, in_degree);
    if(!order.has_value()) {
        return std::unexpected(
            ScanError{.message = "dependency cycle detected in project include graph"});
    }

    return *order;
}

}  // namespace clore::extract
