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

auto extract_project(const config::TaskConfig& config) -> std::expected<ProjectModel, ExtractError>;

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

auto parallel_extract_ast_batch(
    const std::vector<CompileEntry>& entries,
    const std::vector<PreparedEntryState>& prepared_entries,
    const std::unordered_map<std::string, CacheEvaluation>& cache_evaluations)
    -> std::expected<std::vector<std::optional<ParallelASTResult>>, ExtractError> {
    std::vector<std::size_t> miss_indices;
    for(std::size_t idx = 0; idx < entries.size(); ++idx) {
        auto cache_state_it = cache_evaluations.find(prepared_entries[idx].cache_key);
        if(cache_state_it == cache_evaluations.end() || !cache_state_it->second.ast_valid) {
            miss_indices.push_back(idx);
        }
    }

    std::vector<std::optional<ParallelASTResult>> results(entries.size());
    if(miss_indices.empty()) {
        return results;
    }

    kota::event_loop loop;

    auto run = [&]() -> kota::task<std::optional<ExtractError>, kota::error> {
        std::vector<kota::task<std::optional<ExtractError>, kota::error>> tasks;
        tasks.reserve(miss_indices.size());

        for(auto idx: miss_indices) {
            tasks.push_back(kota::queue(
                [&entries, &results, idx]() -> std::optional<ExtractError> {
                    auto ast_result = extract_symbols(entries[idx]);
                    if(!ast_result.has_value()) {
                        return ExtractError{.message = ast_result.error().message};
                    }
                    auto deps_result = cache::capture_dependency_snapshot(ast_result->dependencies);
                    if(!deps_result.has_value()) {
                        return ExtractError{.message = deps_result.error().message};
                    }
                    results[idx] = ParallelASTResult{
                        .ast = std::move(*ast_result),
                        .deps = std::move(*deps_result),
                    };
                    return std::nullopt;
                },
                loop));
        }

        auto all_result = co_await kota::when_all(std::move(tasks));
        if(all_result.has_error()) {
            co_return ExtractError{.message =
                                       std::format("parallel AST extraction failed: error {}",
                                                   all_result.error().value())};
        }

        for(auto& task_result: *all_result) {
            if(task_result.has_value()) {
                co_return std::move(*task_result);
            }
        }
        co_return std::nullopt;
    };

    auto task = run();
    loop.schedule(task);
    loop.run();

    auto result = task.result();
    if(result.has_error()) {
        return std::unexpected(ExtractError{
            .message = std::format("parallel AST extraction failed: {}", result.error().message()),
        });
    }
    if(result->has_value()) {
        return std::unexpected(std::move(**result));
    }

    return results;
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
                .message = std::format(
                    "conflicting module interface flag for source " "file '{}': {} vs {}",
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

auto extract_project(const config::TaskConfig& config)
    -> std::expected<ProjectModel, ExtractError> {
    auto t_total = Clock::now();

    auto t0 = Clock::now();
    auto db_result = load_compdb(config.compile_commands_path);
    if(!db_result.has_value()) {
        return std::unexpected(
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
            return std::unexpected(ExtractError{.message = std::move(abs_path.error().message)});
        }
        entry_copy.file = abs_path->lexically_normal().generic_string();

        if(matches_filter(entry_copy.file, config.filter, filter_root)) {
            filtered_db.entries.push_back(std::move(entry_copy));
        }
    }
    auto skipped = db.entries.size() - filtered_db.entries.size();
    logging::info("filter: {} entries pass, {} skipped", filtered_db.entries.size(), skipped);

    using ExtractCacheResult =
        std::expected<std::unordered_map<std::string, cache::CacheRecord>, cache::CacheError>;
    using CliceCacheResult = std::expected<cache::CliceCacheData, cache::CacheError>;

    ExtractCacheResult extract_cache_result;
    CliceCacheResult clice_cache_result_holder;

    {
        std::jthread t1([&extract_cache_result, workspace_root = config.workspace_root] {
            extract_cache_result = cache::load_extract_cache(workspace_root);
        });
        std::jthread t2([&clice_cache_result_holder, workspace_root = config.workspace_root] {
            clice_cache_result_holder = cache::load_clice_cache(workspace_root);
        });
    }

    if(!extract_cache_result.has_value()) {
        return std::unexpected(ExtractError{
            .message = std::format("failed to load extract cache: {}",
                                   extract_cache_result.error().message),
        });
    }
    auto cache_records = std::move(*extract_cache_result);

    if(!clice_cache_result_holder.has_value()) {
        return std::unexpected(ExtractError{
            .message = std::format("failed to load clice cache: {}",
                                   clice_cache_result_holder.error().message),
        });
    }
    auto clice_cache = std::move(*clice_cache_result_holder);
    logging::info("loaded clice cache: {} pch entries, {} pcm entries",
                  clice_cache.pch.size(),
                  clice_cache.pcm.size());

    auto persist_caches = [&]() -> std::expected<void, ExtractError> {
        auto extract_cache_copy = cache_records;
        auto clice_cache_copy = clice_cache;

        std::expected<void, cache::CacheError> save_extract_result;
        std::expected<void, cache::CacheError> save_clice_result;

        {
            std::jthread t1([&save_extract_result,
                             workspace_root = config.workspace_root,
                             cache_data = std::move(extract_cache_copy)]() {
                save_extract_result = cache::save_extract_cache(workspace_root, cache_data);
            });
            std::jthread t2([&save_clice_result,
                             workspace_root = config.workspace_root,
                             cache_data = std::move(clice_cache_copy)]() {
                save_clice_result = cache::save_clice_cache(workspace_root, cache_data);
            });
        }

        if(!save_extract_result.has_value()) {
            return std::unexpected(ExtractError{
                .message = std::format("failed to save extract cache: {}",
                                       save_extract_result.error().message),
            });
        }

        if(!save_clice_result.has_value()) {
            return std::unexpected(ExtractError{
                .message = std::format("failed to save clice cache: {}",
                                       save_clice_result.error().message),
            });
        }
        return {};
    };

    auto fail_after_persist =
        [persist_caches](ExtractError error) mutable -> std::expected<ProjectModel, ExtractError> {
        if(auto persist_result = persist_caches(); !persist_result.has_value()) {
            return std::unexpected(std::move(persist_result.error()));
        }
        return std::unexpected(std::move(error));
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
    auto dep_result = build_dependency_graph(filtered_db, dep_graph, &seeded_scan_cache);
    if(!dep_result.has_value()) {
        return fail_after_persist(
            ExtractError{.message = std::format("failed to build dependency graph: {}",
                                                dep_result.error().message)});
    }
    auto& scan_cache = seeded_scan_cache;

    auto order_result = topological_order(dep_graph);
    if(!order_result.has_value()) {
        return fail_after_persist(
            ExtractError{.message = std::format("failed to compute file order: {}",
                                                order_result.error().message)});
    }
    auto dt_graph = std::chrono::duration_cast<Ms>(Clock::now() - t1);
    logging::info("dependency graph: {} files, {} edges ({}ms)",
                  dep_graph.files.size(),
                  dep_graph.edges.size(),
                  dt_graph.count());

    auto t2 = Clock::now();
    ProjectModel model;
    model.file_order = std::move(*order_result);

    auto parallel_ast_result =
        parallel_extract_ast_batch(filtered_db.entries, prepared_entries, cache_evaluations);
    if(!parallel_ast_result.has_value()) {
        return fail_after_persist(std::move(parallel_ast_result.error()));
    }
    auto& parallel_asts = *parallel_ast_result;

    // Pre-allocate model capacity to avoid repeated rehashing during the
    // serial merge loop.
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
            return fail_after_persist(
                ExtractError{.message = std::format("missing cache key for {}", normalized)});
        }
        const auto& cache_key = prepared_entry.cache_key;
        auto cache_state_it = cache_evaluations.find(cache_key);
        if(cache_state_it == cache_evaluations.end()) {
            return fail_after_persist(ExtractError{
                .message = std::format("missing extract cache evaluation for {}", normalized)});
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
                return fail_after_persist(ExtractError{
                    .message = std::format("missing parallel AST result for {}", normalized)});
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
            logging::warn("scan cache miss for {}, re-scanning", entry.file);
            auto scan_result = scan_file(entry);
            if(!scan_result.has_value()) {
                return fail_after_persist(
                    ExtractError{.message = std::format("failed to scan includes for {}: {}",
                                                        entry.file,
                                                        scan_result.error().message)});
            }

            auto [rescanned_it, _] =
                scan_cache.scan_results.insert_or_assign(cache_key, std::move(*scan_result));
            cache_it = rescanned_it;
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
                return fail_after_persist(
                    ExtractError{.message = std::move(decl_file_result.error().message)});
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
                    return fail_after_persist(
                        ExtractError{.message = std::move(definition_path_result.error().message)});
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
        return fail_after_persist(std::move(module_result.error()));
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

    if(auto persist_result = persist_caches(); !persist_result.has_value()) {
        return std::unexpected(std::move(persist_result.error()));
    }

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

    return model;
}

}  // namespace clore::extract
