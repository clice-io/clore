module;

#include "kota/async/async.h"

export module extract;

import std;
import config;
import support;

export import :model;
export import :compiler;
export import :scan;
export import :ast;
export import :cache;
export import :filter;
export import :merge;

export namespace clore::extract {

struct ExtractError {
    std::string message;
};

auto extract_project_async(const config::TaskConfig& config, kota::event_loop& loop)
    -> kota::task<ProjectModel, ExtractError>;

}  // namespace clore::extract

namespace clore::extract {

namespace {

using Clock = std::chrono::steady_clock;
using Ms = std::chrono::milliseconds;

struct CacheEvaluation {
    std::uint64_t compile_signature = 0;
    std::optional<std::uint64_t> source_hash{};
    bool scan_valid = false;
    bool ast_valid = false;
};

struct PreparedEntryState {
    std::string normalized_file{};
    std::string cache_key{};
};

struct ParallelASTResult {
    ASTResult ast;
    cache::DependencySnapshot deps;
};

struct LoadedCaches {
    std::unordered_map<std::string, cache::CacheRecord> extract_cache_records;
    cache::CliceCacheData clice_cache;
};

auto unexpected_extract_error(std::string message) -> std::expected<void, ExtractError> {
    return std::unexpected(ExtractError{.message = std::move(message)});
}

template <typename Value>
auto unexpected_extract_value(std::string message) -> std::expected<Value, ExtractError> {
    return std::unexpected(ExtractError{.message = std::move(message)});
}

auto make_exception_error(std::string_view context, const std::exception& ex) -> ExtractError {
    return ExtractError{.message = std::format("{} threw exception: {}", context, ex.what())};
}

auto make_unknown_exception_error(std::string_view context) -> ExtractError {
    return ExtractError{.message = std::format("{} threw unknown exception", context)};
}

template <typename Value, typename Callable>
auto run_cache_io_async(Callable callable,
                        std::string operation_name,
                        kota::event_loop& loop) -> kota::task<Value, ExtractError> {
    auto queued = co_await kota::queue(std::move(callable), loop).catch_cancel();

    if(queued.is_cancelled()) {
        co_await kota::fail(ExtractError{.message = std::format("{} cancelled", operation_name)});
    }
    if(queued.has_error()) {
        co_await kota::fail(ExtractError{
            .message =
                std::format("{} worker failed: {}", operation_name, queued.error().message()),
        });
    }

    auto result = std::move(*queued);
    if(!result.has_value()) {
        co_await kota::fail(std::move(result.error()));
    }

    if constexpr(std::is_void_v<Value>) {
        co_return;
    } else {
        co_return std::move(*result);
    }
}

auto load_extract_cache_async(std::string workspace_root, kota::event_loop& loop)
    -> kota::task<std::unordered_map<std::string, cache::CacheRecord>, ExtractError> {
    return run_cache_io_async<std::unordered_map<std::string, cache::CacheRecord>>(
        [workspace_root = std::move(workspace_root)]()
            -> std::expected<std::unordered_map<std::string, cache::CacheRecord>, ExtractError> {
            try {
                auto result = cache::load_extract_cache(workspace_root);
                if(!result.has_value()) {
                    return unexpected_extract_value<
                        std::unordered_map<std::string, cache::CacheRecord>>(
                        std::format("failed to load extract cache: {}", result.error().message));
                }
                return std::move(*result);
            } catch(const std::exception& ex) {
                return unexpected_extract_value<
                    std::unordered_map<std::string, cache::CacheRecord>>(
                    make_exception_error("extract cache load", ex).message);
            } catch(...) {
                return unexpected_extract_value<
                    std::unordered_map<std::string, cache::CacheRecord>>(
                    make_unknown_exception_error("extract cache load").message);
            }
        },
        "extract cache load",
        loop);
}

auto load_clice_cache_async(std::string workspace_root, kota::event_loop& loop)
    -> kota::task<cache::CliceCacheData, ExtractError> {
    return run_cache_io_async<cache::CliceCacheData>(
        [workspace_root =
             std::move(workspace_root)]() -> std::expected<cache::CliceCacheData, ExtractError> {
            try {
                auto result = cache::load_clice_cache(workspace_root);
                if(!result.has_value()) {
                    return unexpected_extract_value<cache::CliceCacheData>(
                        std::format("failed to load clice cache: {}", result.error().message));
                }
                return std::move(*result);
            } catch(const std::exception& ex) {
                return unexpected_extract_value<cache::CliceCacheData>(
                    make_exception_error("clice cache load", ex).message);
            } catch(...) {
                return unexpected_extract_value<cache::CliceCacheData>(
                    make_unknown_exception_error("clice cache load").message);
            }
        },
        "clice cache load",
        loop);
}

auto load_caches_async(std::string workspace_root, kota::event_loop& loop)
    -> kota::task<LoadedCaches, ExtractError> {
    auto extract_task = load_extract_cache_async(workspace_root, loop);
    auto clice_task = load_clice_cache_async(std::move(workspace_root), loop);

    auto results = co_await kota::when_all(std::move(extract_task), std::move(clice_task));
    if(results.has_error()) {
        co_await kota::fail(std::move(results.error()));
    }

    auto& [extract_cache_records, clice_cache] = *results;
    co_return LoadedCaches{
        .extract_cache_records = std::move(extract_cache_records),
        .clice_cache = std::move(clice_cache),
    };
}

auto save_extract_cache_async(std::string workspace_root,
                              std::unordered_map<std::string, cache::CacheRecord> records,
                              kota::event_loop& loop) -> kota::task<void, ExtractError> {
    return run_cache_io_async<void>(
        [workspace_root = std::move(workspace_root),
         records = std::move(records)]() -> std::expected<void, ExtractError> {
            try {
                auto result = cache::save_extract_cache(workspace_root, records);
                if(!result.has_value()) {
                    return unexpected_extract_error(
                        std::format("failed to save extract cache: {}", result.error().message));
                }
                return {};
            } catch(const std::exception& ex) {
                return unexpected_extract_error(
                    make_exception_error("extract cache save", ex).message);
            } catch(...) {
                return unexpected_extract_error(
                    make_unknown_exception_error("extract cache save").message);
            }
        },
        "extract cache save",
        loop);
}

auto save_clice_cache_async(std::string workspace_root,
                            cache::CliceCacheData data,
                            kota::event_loop& loop) -> kota::task<void, ExtractError> {
    return run_cache_io_async<void>(
        [workspace_root = std::move(workspace_root),
         data = std::move(data)]() -> std::expected<void, ExtractError> {
            try {
                auto result = cache::save_clice_cache(workspace_root, data);
                if(!result.has_value()) {
                    return unexpected_extract_error(
                        std::format("failed to save clice cache: {}", result.error().message));
                }
                return {};
            } catch(const std::exception& ex) {
                return unexpected_extract_error(
                    make_exception_error("clice cache save", ex).message);
            } catch(...) {
                return unexpected_extract_error(
                    make_unknown_exception_error("clice cache save").message);
            }
        },
        "clice cache save",
        loop);
}

auto save_caches_async(std::string workspace_root,
                       std::unordered_map<std::string, cache::CacheRecord> extract_cache_records,
                       cache::CliceCacheData clice_cache,
                       kota::event_loop& loop) -> kota::task<void, ExtractError> {
    auto extract_task =
        save_extract_cache_async(workspace_root, std::move(extract_cache_records), loop);
    auto clice_task =
        save_clice_cache_async(std::move(workspace_root), std::move(clice_cache), loop);

    auto results = co_await kota::when_all(std::move(extract_task), std::move(clice_task));
    if(results.has_error()) {
        co_await kota::fail(std::move(results.error()));
    }
    co_return;
}

auto extract_ast_entry(const CompileEntry& entry)
    -> std::expected<ParallelASTResult, ExtractError> {
    try {
        auto ast_result = extract_symbols(entry);
        if(!ast_result.has_value()) {
            return std::unexpected(ExtractError{
                .message = std::format("AST extraction failed for {}: {}",
                                       entry.file,
                                       ast_result.error().message),
            });
        }
        auto deps_result = cache::capture_dependency_snapshot(ast_result->dependencies);
        if(!deps_result.has_value()) {
            return std::unexpected(ExtractError{
                .message = std::format("AST dependency snapshot failed for {}: {}",
                                       entry.file,
                                       deps_result.error().message),
            });
        }
        return ParallelASTResult{
            .ast = std::move(*ast_result),
            .deps = std::move(*deps_result),
        };
    } catch(const std::exception& ex) {
        return std::unexpected(ExtractError{
            .message = std::format("AST extraction threw for {}: {}", entry.file, ex.what()),
        });
    } catch(...) {
        return std::unexpected(ExtractError{
            .message = std::format("AST extraction threw unknown exception for {}", entry.file),
        });
    }
}

auto extract_ast_batch_async(
    const std::vector<CompileEntry>& entries,
    const std::vector<PreparedEntryState>& prepared_entries,
    const std::unordered_map<std::string, CacheEvaluation>& cache_evaluations,
    const DependencyGraph& dep_graph,
    kota::event_loop& loop)
    -> kota::task<std::vector<std::optional<ParallelASTResult>>, ExtractError> {
    std::unordered_set<std::size_t> miss_indices;
    miss_indices.reserve(entries.size());
    for(std::size_t idx = 0; idx < entries.size(); ++idx) {
        auto cache_state_it = cache_evaluations.find(prepared_entries[idx].cache_key);
        if(cache_state_it == cache_evaluations.end() || !cache_state_it->second.ast_valid) {
            miss_indices.insert(idx);
        }
    }

    std::vector<std::optional<ParallelASTResult>> results(entries.size());
    if(miss_indices.empty()) {
        co_return results;
    }

    std::unordered_set<std::string> known_files;
    known_files.reserve(dep_graph.files.size());
    for(const auto& file: dep_graph.files) {
        known_files.insert(file);
    }

    std::unordered_map<std::string, std::vector<std::size_t>> indices_by_file;
    indices_by_file.reserve(entries.size());
    for(std::size_t idx = 0; idx < entries.size(); ++idx) {
        const auto& file = prepared_entries[idx].normalized_file;
        if(!known_files.contains(file)) {
            co_await kota::fail(ExtractError{
                .message = std::format("compile entry missing from dependency graph: {}", file),
            });
        }
        indices_by_file[file].push_back(idx);
    }

    std::unordered_map<std::string, std::size_t> pending_dependencies;
    std::unordered_map<std::string, std::vector<std::string>> dependents;
    pending_dependencies.reserve(dep_graph.files.size());
    dependents.reserve(dep_graph.files.size());
    for(const auto& file: dep_graph.files) {
        pending_dependencies.emplace(file, 0);
    }
    for(const auto& edge: dep_graph.edges) {
        if(!known_files.contains(edge.from) || !known_files.contains(edge.to)) {
            co_await kota::fail(ExtractError{
                .message = std::format("dependency graph edge references unknown file: {} -> {}",
                                       edge.from,
                                       edge.to),
            });
        }
        dependents[edge.to].push_back(edge.from);
        ++pending_dependencies[edge.from];
    }

    std::size_t completed_files = 0;
    std::unordered_map<std::string, std::unique_ptr<kota::event>> ready_events;
    ready_events.reserve(dep_graph.files.size());
    for(const auto& file: dep_graph.files) {
        ready_events.try_emplace(file,
                                 std::make_unique<kota::event>(pending_dependencies.at(file) == 0));
    }

    auto process_file = [&](std::string file) -> kota::task<void, ExtractError> {
        auto ready_it = ready_events.find(file);
        if(ready_it == ready_events.end()) {
            co_await kota::fail(ExtractError{
                .message = std::format("missing readiness gate for {}", file),
            });
        }

        co_await ready_it->second->wait();

        auto indices_it = indices_by_file.find(file);
        if(indices_it == indices_by_file.end()) {
            co_await kota::fail(ExtractError{
                .message =
                    std::format("dependency graph references unknown compile entry: {}", file),
            });
        }

        std::vector<kota::task<std::expected<ParallelASTResult, ExtractError>, kota::error>> tasks;
        std::vector<std::size_t> task_indices;
        tasks.reserve(indices_it->second.size());
        task_indices.reserve(indices_it->second.size());

        for(auto idx: indices_it->second) {
            if(!miss_indices.contains(idx)) {
                continue;
            }

            logging::debug("submitting AST extraction for {}",
                           prepared_entries[idx].normalized_file);
            task_indices.push_back(idx);
            tasks.push_back(
                kota::queue([entry = entries[idx]] { return extract_ast_entry(entry); }, loop));
        }

        if(!tasks.empty()) {
            auto batch_result = co_await kota::when_all(std::move(tasks));
            if(batch_result.has_error()) {
                co_await kota::fail(ExtractError{
                    .message = std::format("AST extraction worker failed: {}",
                                           batch_result.error().message()),
                });
            }
            if(batch_result->size() != task_indices.size()) {
                co_await kota::fail(ExtractError{
                    .message =
                        std::format("AST extraction result count mismatch: expected {}, got {}",
                                    task_indices.size(),
                                    batch_result->size()),
                });
            }
            for(std::size_t result_idx = 0; result_idx < batch_result->size(); ++result_idx) {
                auto entry_idx = task_indices[result_idx];
                auto& ast_result = (*batch_result)[result_idx];
                if(!ast_result.has_value()) {
                    co_await kota::fail(std::move(ast_result.error()));
                }
                results[entry_idx] = std::move(*ast_result);
            }
        }

        ++completed_files;
        if(auto dependents_it = dependents.find(file); dependents_it != dependents.end()) {
            for(const auto& dependent: dependents_it->second) {
                auto pending_it = pending_dependencies.find(dependent);
                if(pending_it == pending_dependencies.end() || pending_it->second == 0) {
                    co_await kota::fail(ExtractError{
                        .message = std::format("invalid dependency accounting for {}", dependent),
                    });
                }
                --pending_it->second;
                if(pending_it->second == 0) {
                    auto dependent_ready_it = ready_events.find(dependent);
                    if(dependent_ready_it == ready_events.end()) {
                        co_await kota::fail(ExtractError{
                            .message = std::format("missing readiness gate for {}", dependent),
                        });
                    }
                    dependent_ready_it->second->set();
                }
            }
        }

        co_return;
    };

    kota::async_scope<ExtractError> scope;
    for(const auto& file: dep_graph.files) {
        scope.spawn(process_file(file));
    }

    auto scope_result = co_await scope;
    if(scope_result.has_error()) {
        co_await kota::fail(std::move(scope_result.error()));
    }

    if(completed_files != dep_graph.files.size()) {
        co_await kota::fail(ExtractError{
            .message = std::format(
                "dependency graph cycle or incomplete traversal: completed {} of {} files",
                completed_files,
                dep_graph.files.size()),
        });
    }

    co_return results;
}

/// Populate module information from scan cache into the project model.
auto build_module_info(ProjectModel& model, const ScanCache& scan_cache)
    -> std::expected<void, ExtractError> {
    namespace fs = std::filesystem;

    struct GroupedModuleInfo {
        std::string name{};
        bool is_interface = false;
        std::vector<std::string> imports{};
    };

    std::unordered_map<std::string, GroupedModuleInfo> grouped_modules;
    grouped_modules.reserve(scan_cache.scan_results.size());

    for(const auto& [cache_key, scan_result]: scan_cache.scan_results) {
        if(scan_result.module_name.empty()) {
            continue;
        }

        auto key_parts = cache::split_cache_key(cache_key);
        if(!key_parts.has_value()) {
            return std::unexpected(ExtractError{
                .message = std::format("invalid scan cache key '{}': {}",
                                       cache_key,
                                       key_parts.error().message),
            });
        }

        auto source_file = fs::path(key_parts->path).lexically_normal().generic_string();

        auto [group_it, inserted] =
            grouped_modules.try_emplace(source_file,
                                        GroupedModuleInfo{
                                            .name = scan_result.module_name,
                                            .is_interface = scan_result.is_interface_unit,
                                            .imports = {},
                                        });
        auto& group = group_it->second;

        if(!inserted && group.name != scan_result.module_name) {
            return std::unexpected(ExtractError{
                .message =
                    std::format("conflicting module names for source file '{}': '{}' vs '{}'",
                                source_file,
                                group.name,
                                scan_result.module_name),
            });
        }

        if(!inserted && group.is_interface != scan_result.is_interface_unit) {
            return std::unexpected(ExtractError{
                .message =
                    std::format("conflicting module interface flag for source file '{}': {} vs {}",
                                source_file,
                                group.is_interface,
                                scan_result.is_interface_unit),
            });
        }

        append_unique_range(group.imports, scan_result.module_imports);
    }

    if(grouped_modules.empty()) {
        return {};
    }

    model.uses_modules = true;

    for(auto& [source_file, group]: grouped_modules) {
        deduplicate(group.imports);

        auto& mod_unit = model.modules[source_file];
        mod_unit.name = std::move(group.name);
        mod_unit.is_interface = group.is_interface;
        mod_unit.source_file = source_file;
        mod_unit.imports = std::move(group.imports);

        auto file_it = model.files.find(source_file);
        if(file_it != model.files.end()) {
            mod_unit.symbols = file_it->second.symbols;
        }
    }

    return {};
}

}  // namespace

auto extract_project_async(const config::TaskConfig& config, kota::event_loop& loop)
    -> kota::task<ProjectModel, ExtractError> {
    auto t_total = Clock::now();

    auto t0 = Clock::now();
    auto db_result = load_compdb(config.compile_commands_path);
    if(!db_result.has_value()) {
        co_await kota::fail(
            ExtractError{.message = std::format("failed to load compilation database: {}",
                                                db_result.error().message)});
    }
    auto& db = *db_result;
    auto dt_load = std::chrono::duration_cast<Ms>(Clock::now() - t0);
    logging::info("loaded {} compile entries ({}ms)", db.entries.size(), dt_load.count());

    CompilationDatabase filtered_db;
    auto filter_root = filter_root_path(config);
    for(auto& entry: db.entries) {
        auto entry_copy = entry;
        auto abs_path = resolve_path_under_directory(entry_copy.file, entry_copy.directory);
        if(!abs_path.has_value()) {
            co_await kota::fail(ExtractError{.message = std::move(abs_path.error().message)});
        }
        entry_copy.file = abs_path->lexically_normal().generic_string();

        if(matches_filter(entry_copy.file, config.filter, filter_root)) {
            filtered_db.entries.push_back(std::move(entry_copy));
        }
    }
    auto skipped = db.entries.size() - filtered_db.entries.size();
    logging::info("filter: {} entries pass, {} skipped", filtered_db.entries.size(), skipped);

    auto loaded_caches = co_await load_caches_async(config.workspace_root, loop).or_fail();
    auto cache_records = std::move(loaded_caches.extract_cache_records);
    auto clice_cache = std::move(loaded_caches.clice_cache);
    logging::info("loaded clice cache: {} pch entries, {} pcm entries",
                  clice_cache.pch.size(),
                  clice_cache.pcm.size());

    auto fail_after_persist = [&](ExtractError error) -> kota::task<ProjectModel, ExtractError> {
        auto persist_result = co_await save_caches_async(config.workspace_root,
                                                         std::move(cache_records),
                                                         std::move(clice_cache),
                                                         loop);
        if(persist_result.has_error()) {
            co_await kota::fail(std::move(persist_result.error()));
        }
        co_await kota::fail(std::move(error));
    };
    std::unordered_map<std::string, CacheEvaluation> cache_evaluations;
    cache_evaluations.reserve(filtered_db.entries.size());
    std::vector<PreparedEntryState> prepared_entries;
    prepared_entries.reserve(filtered_db.entries.size());

    ScanCache seeded_scan_cache;
    std::size_t ast_cache_hits = 0;
    std::size_t ast_cache_misses = 0;

    for(auto& entry: filtered_db.entries) {
        if(entry.cache_key.empty()) {
            ensure_cache_key(entry);
        }

        auto canonical = entry.normalized_file;
        if(canonical.empty()) {
            canonical = normalize_entry_file(entry);
            entry.normalized_file = canonical;
        }

        auto compile_signature = entry.compile_signature;
        if(compile_signature == 0) {
            compile_signature = cache::build_compile_signature(entry);
            entry.compile_signature = compile_signature;
        }

        if(entry.cache_key.empty()) {
            entry.cache_key = cache::build_cache_key(canonical, compile_signature);
        }

        prepared_entries.push_back(PreparedEntryState{
            .normalized_file = canonical,
            .cache_key = entry.cache_key,
        });
        if(!entry.source_hash.has_value()) {
            logging::warn("extract cache disabled for {}: {}",
                          canonical,
                          "source hash unavailable");
        }

        CacheEvaluation evaluation{
            .compile_signature = compile_signature,
            .source_hash = std::nullopt,
            .scan_valid = false,
            .ast_valid = false,
        };
        if(entry.source_hash.has_value()) {
            evaluation.source_hash = *entry.source_hash;
        }

        if(auto cache_it = cache_records.find(entry.cache_key); cache_it != cache_records.end()) {
            const auto& record = cache_it->second;
            if(evaluation.source_hash.has_value() &&
               record.compile_signature == compile_signature &&
               record.source_hash == *evaluation.source_hash) {
                seeded_scan_cache.scan_results.insert_or_assign(entry.cache_key, record.scan);
                evaluation.scan_valid = true;

                if(!cache::dependencies_changed(record.ast_deps)) {
                    evaluation.ast_valid = true;
                    ++ast_cache_hits;
                } else {
                    ++ast_cache_misses;
                }
            } else {
                ++ast_cache_misses;
            }
        } else {
            ++ast_cache_misses;
        }

        cache_evaluations.insert_or_assign(entry.cache_key, evaluation);
    }

    logging::info("extract cache: {} scan hits, {} ast hits",
                  seeded_scan_cache.scan_results.size(),
                  ast_cache_hits);
    if(ast_cache_hits + ast_cache_misses > 0) {
        logging::cache_hit_rate("extract ast", ast_cache_hits, ast_cache_misses);
    }

    auto t1 = Clock::now();
    DependencyGraph dep_graph;
    auto dep_result =
        co_await build_dependency_graph_async(filtered_db, dep_graph, &seeded_scan_cache, loop);
    if(dep_result.has_error()) {
        co_await fail_after_persist(
            ExtractError{.message = std::format("failed to build dependency graph: {}",
                                                dep_result.error().message)})
            .or_fail();
    }
    auto& scan_cache = seeded_scan_cache;

    auto order_result = topological_order(dep_graph);
    if(!order_result.has_value()) {
        co_await fail_after_persist(
            ExtractError{.message = std::format("failed to compute file order: {}",
                                                order_result.error().message)})
            .or_fail();
    }
    auto dt_graph = std::chrono::duration_cast<Ms>(Clock::now() - t1);
    logging::info("dependency graph: {} files, {} edges ({}ms)",
                  dep_graph.files.size(),
                  dep_graph.edges.size(),
                  dt_graph.count());

    auto t2 = Clock::now();
    ProjectModel model;
    model.file_order = std::move(*order_result);

    auto ast_task = extract_ast_batch_async(filtered_db.entries,
                                            prepared_entries,
                                            cache_evaluations,
                                            dep_graph,
                                            loop);
    auto parallel_ast_result = co_await std::move(ast_task);
    if(parallel_ast_result.has_error()) {
        co_await fail_after_persist(std::move(parallel_ast_result.error())).or_fail();
    }
    auto& parallel_asts = *parallel_ast_result;

    std::size_t estimated_symbols = 0;
    std::unordered_map<std::string, std::filesystem::path> resolved_path_cache;

    for(std::size_t idx = 0; idx < filtered_db.entries.size(); ++idx) {
        auto cache_state_it = cache_evaluations.find(prepared_entries[idx].cache_key);
        if(cache_state_it != cache_evaluations.end() && cache_state_it->second.ast_valid) {
            auto cache_record_it = cache_records.find(prepared_entries[idx].cache_key);
            if(cache_record_it != cache_records.end()) {
                estimated_symbols += cache_record_it->second.ast.symbols.size();
            }
        } else if(idx < parallel_asts.size() && parallel_asts[idx].has_value()) {
            estimated_symbols += parallel_asts[idx]->ast.symbols.size();
        }
    }
    model.symbols.reserve(estimated_symbols);
    model.files.reserve(filtered_db.entries.size());

    const auto total_entries = filtered_db.entries.size();
    for(std::size_t idx = 0; idx < filtered_db.entries.size(); ++idx) {
        auto& entry = filtered_db.entries[idx];
        const auto& prepared_entry = prepared_entries[idx];
        const auto& normalized = prepared_entry.normalized_file;
        if(entry.cache_key.empty()) {
            co_await fail_after_persist(
                ExtractError{.message = std::format("missing cache key for {}", normalized)})
                .or_fail();
        }
        const auto& cache_key = prepared_entry.cache_key;
        auto cache_state_it = cache_evaluations.find(cache_key);
        if(cache_state_it == cache_evaluations.end()) {
            co_await fail_after_persist(
                ExtractError{
                    .message = std::format("missing extract cache evaluation for {}", normalized)})
                .or_fail();
        }

        logging::info("extracting {}/{}: {}", idx + 1, total_entries, normalized);
        auto t_file = Clock::now();

        auto t_ast = Clock::now();
        ASTResult ast_data;
        const ASTResult* ast_view = nullptr;
        cache::DependencySnapshot ast_deps_snapshot;
        bool using_cached_ast = false;

        auto cache_record_it = cache_records.find(cache_key);
        if(cache_state_it->second.ast_valid && cache_record_it != cache_records.end()) {
            ast_view = &cache_record_it->second.ast;
            using_cached_ast = true;
            auto dt_ast = std::chrono::duration_cast<Ms>(Clock::now() - t_ast);
            logging::info("  ast cache: {} symbols, {} relations ({}ms)",
                          ast_view->symbols.size(),
                          ast_view->relations.size(),
                          dt_ast.count());
        } else {
            if(!parallel_asts[idx].has_value()) {
                co_await fail_after_persist(
                    ExtractError{.message =
                                     std::format("missing parallel AST result for {}", normalized)})
                    .or_fail();
            }
            ast_data = std::move(parallel_asts[idx]->ast);
            ast_view = &ast_data;
            ast_deps_snapshot = std::move(parallel_asts[idx]->deps);

            auto dt_ast = std::chrono::duration_cast<Ms>(Clock::now() - t_ast);
            logging::info("  ast: {} symbols, {} relations ({}ms)",
                          ast_data.symbols.size(),
                          ast_data.relations.size(),
                          dt_ast.count());
        }

        auto cache_it = scan_cache.scan_results.find(cache_key);

        auto& current_file_info = model.files[normalized];
        current_file_info.path = normalized;
        std::size_t includes_kept = 0;

        auto resolve_symbol_location_path =
            [&](const std::string& path) -> std::expected<std::filesystem::path, PathResolveError> {
            namespace fs = std::filesystem;

            if(path.empty()) {
                return std::unexpected(
                    PathResolveError{.message = "compilation database entry has empty file path"});
            }

            auto make_cache_key = [&](const std::string& raw_path) -> std::string {
                auto candidate = fs::path(raw_path);
                if(candidate.is_relative()) {
                    std::string key;
                    key.reserve(entry.directory.size() + raw_path.size() + 1);
                    key.append(entry.directory);
                    key.push_back('\x1f');
                    key.append(raw_path);
                    return key;
                }
                return raw_path;
            };

            auto cache_key = make_cache_key(path);
            auto cache_it = resolved_path_cache.find(cache_key);
            if(cache_it != resolved_path_cache.end()) {
                return cache_it->second;
            }

            auto resolve = [](const fs::path& p) -> std::expected<fs::path, PathResolveError> {
                std::error_code ec;
                auto absolute = fs::absolute(p, ec);
                if(ec) {
                    return std::unexpected(PathResolveError{
                        .message = std::format("failed to resolve absolute path: {}", p.string())});
                }
                auto canonical = fs::weakly_canonical(absolute.lexically_normal(), ec);
                if(!ec) {
                    return canonical;
                }
                return absolute.lexically_normal();
            };

            auto candidate = fs::path(path);
            if(!candidate.is_relative()) {
                auto result = resolve(candidate);
                if(result.has_value()) {
                    resolved_path_cache[cache_key] = *result;
                }
                return result;
            }

            auto workspace_candidate = (filter_root / candidate).lexically_normal();
            std::error_code exists_error;
            if(fs::exists(workspace_candidate, exists_error) && !exists_error) {
                auto result = resolve(workspace_candidate);
                if(result.has_value()) {
                    resolved_path_cache[cache_key] = *result;
                }
                return result;
            }

            auto fallback = resolve_path_under_directory(path, entry.directory);
            if(!fallback.has_value()) {
                return fallback;
            }
            auto result = resolve(*fallback);
            if(result.has_value()) {
                resolved_path_cache[cache_key] = *result;
            }
            return result;
        };

        if(cache_it == scan_cache.scan_results.end()) {
            co_await fail_after_persist(
                ExtractError{
                    .message = std::format("missing dependency scan result for {}", normalized),
                })
                .or_fail();
        }

        for(auto& inc: cache_it->second.includes) {
            namespace fs = std::filesystem;
            auto inc_path = fs::path(inc.path);
            if(inc_path.is_relative()) {
                inc_path = fs::path(entry.directory) / inc_path;
            }
            inc_path = inc_path.lexically_normal();
            if(matches_filter(inc_path.string(), config.filter, filter_root)) {
                current_file_info.includes.push_back(inc_path.generic_string());
                ++includes_kept;
            }
        }

        std::size_t symbols_kept = 0;
        for(const auto& sym: ast_view->symbols) {
            auto decl_file_result = resolve_symbol_location_path(sym.declaration_location.file);
            if(!decl_file_result.has_value()) {
                co_await fail_after_persist(
                    ExtractError{.message = std::move(decl_file_result.error().message)})
                    .or_fail();
            }
            auto decl_file = decl_file_result->lexically_normal();

            if(!matches_filter(decl_file.string(), config.filter, filter_root)) {
                continue;
            }

            auto normalized_sym = sym;
            normalized_sym.declaration_location.file = decl_file.generic_string();
            if(normalized_sym.definition_location.has_value() &&
               !normalized_sym.definition_location->file.empty()) {
                auto definition_path_result =
                    resolve_symbol_location_path(normalized_sym.definition_location->file);
                if(!definition_path_result.has_value()) {
                    co_await fail_after_persist(
                        ExtractError{.message = std::move(definition_path_result.error().message)})
                        .or_fail();
                }
                normalized_sym.definition_location->file =
                    definition_path_result->lexically_normal().generic_string();
            }

            auto symbol_id = normalized_sym.id;

            auto sym_it = model.symbols.find(symbol_id);
            if(sym_it == model.symbols.end()) {
                auto [inserted_it, _] = model.symbols.emplace(symbol_id, std::move(normalized_sym));
                sym_it = inserted_it;
                ++symbols_kept;
            } else {
                merge_symbol_info(sym_it->second, std::move(normalized_sym));
            }
        }

        for(const auto& rel: ast_view->relations) {
            auto from_it = model.symbols.find(rel.from);
            if(from_it == model.symbols.end()) {
                continue;
            }

            auto to_it = model.symbols.find(rel.to);
            if(to_it == model.symbols.end()) {
                continue;
            }

            if(rel.is_inheritance) {
                to_it->second.derived.push_back(rel.from);
            } else if(rel.is_call) {
                from_it->second.calls.push_back(rel.to);
                to_it->second.called_by.push_back(rel.from);
            } else {
                from_it->second.references.push_back(rel.to);
                to_it->second.referenced_by.push_back(rel.from);
            }
        }

        if(cache_state_it->second.source_hash.has_value()) {
            if(using_cached_ast) {
                auto& record = cache_record_it->second;
                record.compile_signature = cache_state_it->second.compile_signature;
                record.source_hash = *cache_state_it->second.source_hash;
                record.scan = cache_it->second;
            } else {
                cache_records.insert_or_assign(
                    cache_key,
                    cache::CacheRecord{
                        .compile_signature = cache_state_it->second.compile_signature,
                        .source_hash = *cache_state_it->second.source_hash,
                        .ast_deps = std::move(ast_deps_snapshot),
                        .scan = cache_it->second,
                        .ast = std::move(ast_data),
                    });
            }
        } else {
            cache_records.erase(cache_key);
        }

        auto dt_file = std::chrono::duration_cast<Ms>(Clock::now() - t_file);
        logging::info("  kept: {} symbols, {} includes ({}ms)",
                      symbols_kept,
                      includes_kept,
                      dt_file.count());
    }

    rebuild_model_indexes(config, model);

    if(auto module_result = build_module_info(model, scan_cache); !module_result.has_value()) {
        co_await fail_after_persist(std::move(module_result.error())).or_fail();
    }
    rebuild_lookup_maps(model);
    if(model.uses_modules) {
        logging::info("detected {} module units", model.modules.size());
        for(auto& [source_file, mod]: model.modules) {
            logging::info("  module '{}' (interface={}) from {}",
                          mod.name,
                          mod.is_interface,
                          source_file);
        }
    }

    co_await save_caches_async(config.workspace_root,
                               std::move(cache_records),
                               std::move(clice_cache),
                               loop)
        .or_fail();

    std::size_t total_calls = 0;
    std::size_t total_refs = 0;
    for(auto& [_, sym]: model.symbols) {
        total_calls += sym.calls.size();
        total_refs += sym.references.size();
    }

    auto dt_extract = std::chrono::duration_cast<Ms>(Clock::now() - t2);
    auto dt_total = std::chrono::duration_cast<Ms>(Clock::now() - t_total);

    logging::info("project model: {} symbols, {} files, {} namespaces ({} skipped)",
                  model.symbols.size(),
                  model.files.size(),
                  model.namespaces.size(),
                  skipped);
    logging::info("relations: {} call edges, {} reference edges", total_calls, total_refs);
    logging::info("timing: load={}ms  graph={}ms  extract={}ms  total={}ms",
                  dt_load.count(),
                  dt_graph.count(),
                  dt_extract.count(),
                  dt_total.count());

    co_return model;
}

}  // namespace clore::extract
