#include "extract/extract.h"

#include <chrono>
#include <expected>
#include <filesystem>
#include <format>
#include <optional>
#include <string_view>

#include "extract/ast.h"
#include "extract/compdb.h"
#include "extract/scan.h"
#include "support/logging.h"

namespace clore::extract {

namespace {

/// Returns true if `relative` matches the configured pattern.
///
/// Patterns are interpreted as *project-relative path prefixes* (with forward
/// slashes), not as arbitrary substrings. This prevents accidental matches like
/// "src/" matching "build/_deps/.../src/...".
bool path_prefix_matches(std::string_view relative, std::string_view pattern) {
    if(pattern.empty()) return false;

    // If the pattern contains a path separator, treat it as an explicit prefix.
    // Otherwise treat it as a top-level directory name.
    if(pattern.find('/') != std::string_view::npos) {
        return relative.starts_with(pattern);
    }

    if(relative == pattern) return true;
    if(relative.size() < pattern.size() + 1) return false;
    if(!relative.starts_with(pattern)) return false;
    return relative[pattern.size()] == '/';
}

auto project_relative_path(const std::filesystem::path& file,
                           const std::filesystem::path& project_root)
    -> std::optional<std::filesystem::path> {
    auto rel = file.lexically_relative(project_root);
    if(rel.empty()) return std::nullopt;
    for(const auto& part : rel) {
        if(part == "..") return std::nullopt;
    }
    return rel;
}

auto resolve_path_under_directory(const std::string& path,
                                  const std::string& directory)
    -> std::expected<std::filesystem::path, ExtractError> {
    namespace fs = std::filesystem;

    if(path.empty()) {
        return std::unexpected(ExtractError{.message = "compilation database entry has empty file path"});
    }

    auto p = fs::path(path);
    if(p.is_relative()) {
        if(directory.empty()) {
            return std::unexpected(ExtractError{
                .message = std::format("relative file path '{}' requires a non-empty 'directory' in compile_commands.json",
                                       path),
            });
        }
        p = fs::path(directory) / p;
    }

    return p.lexically_normal();
}

/// Returns true if `file` should be processed according to `filter`.
///
/// Security: rejects any path whose fs::relative result contains a ".."
/// component, which would mean `file` escapes `project_root` and could be
/// used to bypass the expected-path boundary.
bool matches_filter(const std::string& file, const config::FilterRule& filter,
                    const std::string& project_root) {
    namespace fs = std::filesystem;

    auto file_path = fs::path(file).lexically_normal();
    auto root_path = fs::path(project_root).lexically_normal();

    auto rel_opt = project_relative_path(file_path, root_path);
    if(!rel_opt.has_value()) return false;

    auto relative_str = rel_opt->generic_string();  // forward slashes

    // File must match at least one include pattern (if any are specified).
    if(!filter.include.empty()) {
        bool matched = false;
        for(auto& pattern : filter.include) {
            if(path_prefix_matches(relative_str, pattern)) {
                matched = true;
                break;
            }
        }
        if(!matched) return false;
    }

    // File must not match any exclude pattern.
    for(auto& pattern : filter.exclude) {
        if(path_prefix_matches(relative_str, pattern)) {
            return false;
        }
    }

    return true;
}

using Clock = std::chrono::steady_clock;
using Ms    = std::chrono::milliseconds;

}  // namespace

auto extract_project(const config::TaskConfig& config)
    -> std::expected<ProjectModel, ExtractError> {
    auto t_total = Clock::now();

    // 1. Load compilation database
    auto t0 = Clock::now();
    auto db_result = load_compdb(config.compile_commands_path);
    if(!db_result.has_value()) {
        return std::unexpected(ExtractError{
            .message = std::format("failed to load compilation database: {}",
                                   db_result.error().message)});
    }
    auto& db = *db_result;
    auto dt_load = std::chrono::duration_cast<Ms>(Clock::now() - t0);
    logging::info("loaded {} compile entries ({}ms)", db.entries.size(), dt_load.count());

    // 1b. Pre-filter entries so the dependency graph and symbol extraction
    //     only process files that pass the user's include/exclude rules.
    //     This avoids expensive preprocessor and AST work on irrelevant files.
    CompilationDatabase filtered_db;
    for(auto& entry : db.entries) {
        auto entry_copy = entry;
        auto abs_path = resolve_path_under_directory(entry_copy.file, entry_copy.directory);
        if(!abs_path.has_value()) {
            return std::unexpected(std::move(abs_path.error()));
        }
        entry_copy.file = abs_path->string();  // keep OS-native separators for clang

        if(matches_filter(entry_copy.file, config.filter, config.project_root)) {
            filtered_db.entries.push_back(std::move(entry_copy));
        }
    }
    auto skipped = db.entries.size() - filtered_db.entries.size();
    logging::info("filter: {} entries pass, {} skipped",
                  filtered_db.entries.size(), skipped);

    // 2. Build dependency graph and get topological order.
    //    build_dependency_graph now also populates a ScanCache so we can reuse
    //    the per-file scan results later without re-running the preprocessor.
    auto t1 = Clock::now();
    auto dep_result = build_dependency_graph(filtered_db);
    if(!dep_result.has_value()) {
        return std::unexpected(ExtractError{
            .message = std::format("failed to build dependency graph: {}",
                                   dep_result.error().message)});
    }
    auto& dep_graph = dep_result->graph;
    auto& scan_cache = dep_result->cache;

    auto order_result = topological_order(dep_graph);
    if(!order_result.has_value()) {
        return std::unexpected(ExtractError{
            .message = std::format("failed to compute file order: {}",
                                   order_result.error().message)});
    }
    auto dt_graph = std::chrono::duration_cast<Ms>(Clock::now() - t1);
    logging::info("dependency graph: {} files, {} edges ({}ms)",
                  dep_graph.files.size(), dep_graph.edges.size(), dt_graph.count());

    // 3. Extract symbols for each file, reusing cached scan results.
    auto t2 = Clock::now();
    ProjectModel model;
    model.file_order = std::move(*order_result);

    const auto total_entries = filtered_db.entries.size();
    for(std::size_t idx = 0; idx < filtered_db.entries.size(); ++idx) {
        auto& entry = filtered_db.entries[idx];
        namespace fs = std::filesystem;
        auto normalized = fs::path(entry.file).lexically_normal().generic_string();

        logging::info("extracting {}/{}: {}", idx + 1, total_entries, normalized);
        auto t_file = Clock::now();

        // Extract symbols and relations -- fail fast on any error
        auto t_ast = Clock::now();
        auto ast_result = extract_symbols(entry, *config.extract.max_snippet_bytes);
        if(!ast_result.has_value()) {
            return std::unexpected(ExtractError{
                .message = std::format("failed to extract symbols from {}: {}",
                                       entry.file, ast_result.error().message)});
        }
        auto dt_ast = std::chrono::duration_cast<Ms>(Clock::now() - t_ast);
        logging::info("  ast: {} symbols, {} relations ({}ms)",
                      ast_result->symbols.size(), ast_result->relations.size(), dt_ast.count());

        // Look up includes from the scan cache instead of re-running the
        // preprocessor.  The key must use the OS-native normalized form that
        // build_dependency_graph stored (lexically_normal().string()).
        auto cache_key = fs::path(entry.file).lexically_normal().string();
        auto cache_it = scan_cache.find(cache_key);

        // Build file info
        FileInfo file_info;
        file_info.path = normalized;
        std::size_t includes_kept = 0;

        if(cache_it != scan_cache.end()) {
            for(auto& inc : cache_it->second.includes) {
                // Keep include edges only within the configured project filter;
                // external headers (deps, system headers) should not affect the
                // project page graph.
                namespace fs = std::filesystem;
                auto inc_path = fs::path(inc.path);
                if(inc_path.is_relative()) {
                    inc_path = fs::path(entry.directory) / inc_path;
                }
                inc_path = inc_path.lexically_normal();
                if(matches_filter(inc_path.string(), config.filter, config.project_root)) {
                    file_info.includes.push_back(inc_path.string());
                    ++includes_kept;
                }
            }
        } else {
            // Fallback: scan if the file somehow wasn't in the cache (should
            // not happen for entries from the compilation database).
            logging::warn("scan cache miss for {}, re-scanning", entry.file);
            auto scan_result = scan_file(entry);
            if(!scan_result.has_value()) {
                return std::unexpected(ExtractError{
                    .message = std::format("failed to scan includes for {}: {}",
                                           entry.file, scan_result.error().message)});
            }
            for(auto& inc : scan_result->includes) {
                namespace fs = std::filesystem;
                auto inc_path = fs::path(inc.path);
                if(inc_path.is_relative()) {
                    inc_path = fs::path(entry.directory) / inc_path;
                }
                inc_path = inc_path.lexically_normal();
                if(matches_filter(inc_path.string(), config.filter, config.project_root)) {
                    file_info.includes.push_back(inc_path.string());
                    ++includes_kept;
                }
            }
        }

        // Process extracted symbols
        std::size_t symbols_kept = 0;
        for(auto& sym : ast_result->symbols) {
            // Filter symbols by their declared source file so dependency code
            // (e.g. FetchContent sources under build/_deps) does not end up in
            // the project model or generated docs.
            namespace fs = std::filesystem;
            auto decl_file = fs::path(sym.declaration_location.file);
            if(decl_file.is_relative()) {
                decl_file = fs::path(entry.directory) / decl_file;
            }
            decl_file = decl_file.lexically_normal();

            if(!matches_filter(decl_file.string(), config.filter, config.project_root)) {
                continue;
            }

            file_info.symbols.push_back(sym.id);
            ++symbols_kept;

            // Register in namespace
            auto ns_end = sym.qualified_name.rfind("::");
            if(ns_end != std::string::npos) {
                auto ns_name = sym.qualified_name.substr(0, ns_end);
                model.namespaces[ns_name].name = ns_name;
                model.namespaces[ns_name].symbols.push_back(sym.id);
            }

            // Register children with parent
            if(sym.parent.has_value()) {
                auto parent_it = model.symbols.find(*sym.parent);
                if(parent_it != model.symbols.end()) {
                    parent_it->second.children.push_back(sym.id);
                }
            }

            // Register derived with base
            for(auto& base_id : sym.bases) {
                auto base_it = model.symbols.find(base_id);
                if(base_it != model.symbols.end()) {
                    base_it->second.derived.push_back(sym.id);
                }
            }

            model.symbols.emplace(sym.id, std::move(sym));
        }

        // Wire forward call/reference edges onto SymbolInfo.
        for(auto& rel : ast_result->relations) {
            auto from_it = model.symbols.find(rel.from);
            if(from_it == model.symbols.end()) continue;

            if(rel.is_call) {
                from_it->second.calls.push_back(rel.to);
            } else {
                from_it->second.references.push_back(rel.to);
            }
        }

        model.files.emplace(normalized, std::move(file_info));
        auto dt_file = std::chrono::duration_cast<Ms>(Clock::now() - t_file);
        logging::info("  kept: {} symbols, {} includes ({}ms)",
                      symbols_kept, includes_kept, dt_file.count());
    }

    // 4. Build reverse edges (called_by / referenced_by).
    //    Forward edges (calls / references) were written above; now iterate
    //    all symbols and populate the reverse direction.
    auto t3 = Clock::now();
    logging::info("building reverse edges for {} symbols...", model.symbols.size());
    for(auto& [id, sym] : model.symbols) {
        for(auto& callee_id : sym.calls) {
            auto callee_it = model.symbols.find(callee_id);
            if(callee_it != model.symbols.end()) {
                callee_it->second.called_by.push_back(id);
            }
        }
        for(auto& ref_id : sym.references) {
            auto ref_it = model.symbols.find(ref_id);
            if(ref_it != model.symbols.end()) {
                ref_it->second.referenced_by.push_back(id);
            }
        }
    }
    auto dt_reverse = std::chrono::duration_cast<Ms>(Clock::now() - t3);
    logging::info("reverse edges done ({}ms)", dt_reverse.count());

    // Count total relation edges for logging.
    std::size_t total_calls = 0, total_refs = 0;
    for(auto& [id, sym] : model.symbols) {
        total_calls += sym.calls.size();
        total_refs += sym.references.size();
    }

    auto dt_extract = std::chrono::duration_cast<Ms>(Clock::now() - t2);
    auto dt_total   = std::chrono::duration_cast<Ms>(Clock::now() - t_total);

    logging::info("project model: {} symbols, {} files, {} namespaces ({} skipped)",
                  model.symbols.size(), model.files.size(), model.namespaces.size(), skipped);
    logging::info("relations: {} call edges, {} reference edges", total_calls, total_refs);
    logging::info("timing: load={}ms  graph={}ms  extract={}ms  total={}ms",
                  dt_load.count(), dt_graph.count(), dt_extract.count(), dt_total.count());

    return model;
}

}  // namespace clore::extract
