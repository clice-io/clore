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
#include <vector>

export module extract;

export import :symbol;
export import :model;
export import :compdb;
export import :scan;
export import :tooling;
export import :ast;
export import :cache;
export import :qualified_name;

import config;
import support;

export namespace clore::extract {

struct ExtractError {
    std::string message;
};

auto extract_project(const config::TaskConfig& config)
    -> std::expected<ProjectModel, ExtractError>;

}  // namespace clore::extract

// ── implementation ──────────────────────────────────────────────────

namespace clore::extract {

namespace {

bool path_prefix_matches(std::string_view relative, std::string_view pattern) {
    if(pattern.empty()) return false;

    if(pattern.find('/') != std::string_view::npos) {
        return relative.starts_with(pattern);
    }

    if(relative == pattern) return true;
    if(relative.size() < pattern.size() + 1) return false;
    if(!relative.starts_with(pattern)) return false;
    return relative[pattern.size()] == '/';
}

auto project_relative_path(const std::filesystem::path& file,
                           const std::filesystem::path& root_path)
    -> std::optional<std::filesystem::path> {
    auto rel = file.lexically_relative(root_path);
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

bool matches_filter(const std::string& file, const config::FilterRule& filter,
                    const std::filesystem::path& filter_root) {
    namespace fs = std::filesystem;

    auto file_path = fs::path(file).lexically_normal();
    auto root_path = filter_root.lexically_normal();

    auto rel_opt = project_relative_path(file_path, root_path);
    if(!rel_opt.has_value()) return false;

    auto relative_str = rel_opt->generic_string();

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

    for(auto& pattern : filter.exclude) {
        if(path_prefix_matches(relative_str, pattern)) {
            return false;
        }
    }

    return true;
}

auto filter_root_path(const config::TaskConfig& config) -> std::filesystem::path {
    namespace fs = std::filesystem;

    if(!config.workspace_root.empty()) {
        return fs::path(config.workspace_root).lexically_normal();
    }

    return fs::path(config.project_root).lexically_normal();
}

using Clock = std::chrono::steady_clock;
using Ms    = std::chrono::milliseconds;

struct CacheEvaluation {
    std::uint64_t compile_signature = 0;
    std::optional<std::uint64_t> source_hash;
    bool scan_valid = false;
    bool ast_valid = false;
};

template <typename T>
void append_unique(std::vector<T>& values, const T& value) {
    if(std::find(values.begin(), values.end(), value) == values.end()) {
        values.push_back(value);
    }
}

template <typename T>
void append_unique_range(std::vector<T>& values, const std::vector<T>& incoming) {
    for(const auto& value : incoming) {
        append_unique(values, value);
    }
}

template <typename T>
void deduplicate(std::vector<T>& values) {
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
}

auto rebuild_lookup_maps(ProjectModel& model) -> void {
    model.symbol_ids_by_qualified_name.clear();
    model.module_name_to_sources.clear();

    for(auto& [symbol_id, sym] : model.symbols) {
        if(!sym.qualified_name.empty()) {
            model.symbol_ids_by_qualified_name[sym.qualified_name].push_back(symbol_id);
        }
    }

    for(auto& [qualified_name, symbol_ids] : model.symbol_ids_by_qualified_name) {
        std::sort(symbol_ids.begin(), symbol_ids.end());
        symbol_ids.erase(std::unique(symbol_ids.begin(), symbol_ids.end()), symbol_ids.end());
        if(symbol_ids.size() > 1) {
            logging::info("qualified name '{}' has {} overload candidates",
                          qualified_name,
                          symbol_ids.size());
        }
    }

    for(auto& [source_file, mod_unit] : model.modules) {
        if(!mod_unit.name.empty()) {
            auto& sources = model.module_name_to_sources[mod_unit.name];
            sources.push_back(source_file);
        }
    }

    for(auto& [module_name, sources] : model.module_name_to_sources) {
        std::sort(sources.begin(), sources.end());
        sources.erase(std::unique(sources.begin(), sources.end()), sources.end());
        if(sources.size() > 1) {
            logging::warn("duplicate module name '{}' from '{}' and '{}'",
                          module_name,
                          sources.front(),
                          sources[1]);
        }
    }
}

auto ensure_namespace_hierarchy(ProjectModel& model, std::string_view namespace_name)
    -> std::string {
    if(namespace_name.empty() ||
       namespace_name.find("(anonymous namespace)") != std::string_view::npos) {
        return {};
    }

    auto parts = split_top_level_qualified_name(namespace_name);
    if(parts.empty()) {
        return {};
    }

    std::string parent_name;
    std::string current_name;
    for(const auto& part : parts) {
        if(part.empty()) {
            return {};
        }

        if(!current_name.empty()) {
            current_name += "::";
        }
        current_name += part;

        auto& current_info = model.namespaces[current_name];
        current_info.name = current_name;

        if(!parent_name.empty()) {
            auto& parent_info = model.namespaces[parent_name];
            parent_info.name = parent_name;
            append_unique(parent_info.children, current_name);
        }

        parent_name = current_name;
    }

    return current_name;
}

auto find_enclosing_namespace(const ProjectModel& model, const SymbolInfo& sym) -> std::string {
    if(!sym.enclosing_namespace.empty()) {
        return sym.enclosing_namespace;
    }

    auto parent_id = sym.parent;
    while(parent_id.has_value()) {
        auto parent_it = model.symbols.find(*parent_id);
        if(parent_it == model.symbols.end()) {
            break;
        }
        if(parent_it->second.kind == SymbolKind::Namespace) {
            return parent_it->second.qualified_name;
        }
        parent_id = parent_it->second.parent;
    }

    return namespace_prefix_from_qualified_name(sym.qualified_name);
}

template <typename T>
auto merge_symbol_info_impl(SymbolInfo& current, T&& incoming) -> void {
    const auto& incoming_ref = incoming;

    const bool prefer_incoming_definition =
        incoming_ref.definition_location.has_value() && !current.definition_location.has_value();
    const bool prefer_incoming_snippet =
        prefer_incoming_definition ||
        (current.source_snippet.size() < incoming_ref.source_snippet.size() &&
         !incoming_ref.source_snippet.empty());

    if(current.name.empty() && !incoming_ref.name.empty()) {
        current.name = incoming_ref.name;
    }
    if(current.qualified_name.empty() && !incoming_ref.qualified_name.empty()) {
        current.qualified_name = incoming_ref.qualified_name;
    }
    if(current.enclosing_namespace.empty() && !incoming_ref.enclosing_namespace.empty()) {
        current.enclosing_namespace = incoming_ref.enclosing_namespace;
    }
    if(!current.declaration_location.is_known() && incoming_ref.declaration_location.is_known()) {
        current.declaration_location = incoming_ref.declaration_location;
    }
    if((!current.definition_location.has_value() && incoming_ref.definition_location.has_value()) ||
       (prefer_incoming_definition && incoming_ref.definition_location.has_value())) {
        current.definition_location = incoming_ref.definition_location;
    }
    if((current.signature.empty() && !incoming_ref.signature.empty()) ||
       (prefer_incoming_definition && !incoming_ref.signature.empty())) {
        current.signature = incoming_ref.signature;
    }
    if(current.doc_comment.empty() && !incoming_ref.doc_comment.empty()) {
        current.doc_comment = incoming_ref.doc_comment;
    }
    if(prefer_incoming_snippet) {
        current.source_snippet = incoming_ref.source_snippet;
    } else if(current.source_snippet.empty() && !incoming_ref.source_snippet.empty()) {
        current.source_snippet = incoming_ref.source_snippet;
    }
    if(!current.parent.has_value() && incoming_ref.parent.has_value()) {
        current.parent = incoming_ref.parent;
    }
    if(current.lexical_parent_name.empty() && !incoming_ref.lexical_parent_name.empty()) {
        current.lexical_parent_name = incoming_ref.lexical_parent_name;
    }
    if(current.lexical_parent_kind == SymbolKind::Unknown &&
       incoming_ref.lexical_parent_kind != SymbolKind::Unknown) {
        current.lexical_parent_kind = incoming_ref.lexical_parent_kind;
    }
    if(current.access.empty() && !incoming_ref.access.empty()) {
        current.access = incoming_ref.access;
    }
    if(!current.is_template && incoming_ref.is_template) {
        current.is_template = true;
        current.template_params = incoming_ref.template_params;
    } else if(current.template_params.empty() && !incoming_ref.template_params.empty()) {
        current.template_params = incoming_ref.template_params;
    }

    append_unique_range(current.children, incoming_ref.children);
    append_unique_range(current.bases, incoming_ref.bases);
    append_unique_range(current.derived, incoming_ref.derived);
    append_unique_range(current.calls, incoming_ref.calls);
    append_unique_range(current.called_by, incoming_ref.called_by);
    append_unique_range(current.references, incoming_ref.references);
    append_unique_range(current.referenced_by, incoming_ref.referenced_by);

    deduplicate(current.children);
    deduplicate(current.bases);
    deduplicate(current.derived);
    deduplicate(current.calls);
    deduplicate(current.called_by);
    deduplicate(current.references);
    deduplicate(current.referenced_by);
}

auto merge_symbol_info(SymbolInfo& current, SymbolInfo&& incoming) -> void {
    merge_symbol_info_impl(current, std::move(incoming));
}

auto merge_symbol_info(SymbolInfo& current, const SymbolInfo& incoming) -> void {
    merge_symbol_info_impl(current, incoming);
}

auto rebuild_model_indexes(const config::TaskConfig& config, ProjectModel& model) -> void {
    namespace fs = std::filesystem;

    for(auto& [_, file_info] : model.files) {
        file_info.symbols.clear();
        deduplicate(file_info.includes);
    }

    model.namespaces.clear();

    for(auto& [_, sym] : model.symbols) {
        deduplicate(sym.calls);
        deduplicate(sym.references);
        sym.children.clear();
        sym.derived.clear();
        sym.called_by.clear();
        sym.referenced_by.clear();
    }

    auto filter_root = filter_root_path(config);

    for(auto& [symbol_id, sym] : model.symbols) {
        auto owner_path = fs::path(sym.declaration_location.file);
        if(owner_path.is_relative()) {
            owner_path = filter_root / owner_path;
        }
        owner_path = owner_path.lexically_normal();

        if(matches_filter(owner_path.string(), config.filter, filter_root)) {
            auto owner_key = owner_path.generic_string();
            auto& owner_file_info = model.files[owner_key];
            owner_file_info.path = owner_key;
            append_unique(owner_file_info.symbols, symbol_id);
        }

        if(sym.kind == SymbolKind::Namespace) {
            (void)ensure_namespace_hierarchy(model, sym.qualified_name);
        } else {
            auto ns_name = find_enclosing_namespace(model, sym);
            auto canonical_ns_name = ensure_namespace_hierarchy(model, ns_name);
            if(!canonical_ns_name.empty()) {
                auto& ns_info = model.namespaces[canonical_ns_name];
                ns_info.name = canonical_ns_name;
                append_unique(ns_info.symbols, symbol_id);
            }
        }

        if(sym.parent.has_value()) {
            auto parent_it = model.symbols.find(*sym.parent);
            if(parent_it != model.symbols.end()) {
                append_unique(parent_it->second.children, symbol_id);
            }
        }

        for(const auto& base_id : sym.bases) {
            auto base_it = model.symbols.find(base_id);
            if(base_it != model.symbols.end()) {
                append_unique(base_it->second.derived, symbol_id);
            }
        }
    }

    for(auto& [_, file_info] : model.files) {
        deduplicate(file_info.symbols);
    }
    for(auto& [_, ns_info] : model.namespaces) {
        deduplicate(ns_info.symbols);
        deduplicate(ns_info.children);
    }
    for(auto& [_, sym] : model.symbols) {
        deduplicate(sym.children);
        deduplicate(sym.derived);
    }
}

/// Populate module information from scan cache into the project model.
auto build_module_info(ProjectModel& model, const ScanCache& scan_cache)
    -> std::expected<void, ExtractError> {
    namespace fs = std::filesystem;

    for(auto& [cache_key, scan_result] : scan_cache) {
        if(scan_result.module_name.empty()) continue;

        auto key_parts = cache::split_cache_key(cache_key);
        if(!key_parts.has_value()) {
            return std::unexpected(ExtractError{
                .message = std::format("invalid scan cache key '{}': {}",
                                       cache_key,
                                       key_parts.error().message),
            });
        }

        model.uses_modules = true;

        auto source_file = fs::path(key_parts->path).lexically_normal().generic_string();
        auto& mod_unit = model.modules[source_file];
        mod_unit.name = scan_result.module_name;
        mod_unit.is_interface = scan_result.is_interface_unit;
        mod_unit.source_file = source_file;
        mod_unit.imports = scan_result.module_imports;

        // Associate symbols from that file to this module unit
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

    // 1b. Pre-filter entries
    CompilationDatabase filtered_db;
    auto filter_root = filter_root_path(config);
    for(auto& entry : db.entries) {
        auto entry_copy = entry;
        auto abs_path = resolve_path_under_directory(entry_copy.file, entry_copy.directory);
        if(!abs_path.has_value()) {
            return std::unexpected(std::move(abs_path.error()));
        }
        entry_copy.file = abs_path->lexically_normal().generic_string();

        if(matches_filter(entry_copy.file, config.filter, filter_root)) {
            filtered_db.entries.push_back(std::move(entry_copy));
        }
    }
    auto skipped = db.entries.size() - filtered_db.entries.size();
    logging::info("filter: {} entries pass, {} skipped",
                  filtered_db.entries.size(), skipped);

    auto cache_result = cache::load_extract_cache(config.workspace_root);
    std::unordered_map<std::string, cache::CacheRecord> cache_records;
    if(!cache_result.has_value()) {
        logging::warn("failed to load extract cache; proceeding without cache: {}",
                      cache_result.error().message);
    } else {
        cache_records = std::move(*cache_result);
    }
    std::unordered_map<std::string, CacheEvaluation> cache_evaluations;
    cache_evaluations.reserve(filtered_db.entries.size());

    ScanCache seeded_scan_cache;
    std::size_t ast_cache_hits = 0;

    for(auto& entry : filtered_db.entries) {
        auto normalized = std::filesystem::path(entry.file).lexically_normal().generic_string();
        auto compile_signature = cache::build_compile_signature(entry);
        entry.cache_key = cache::build_cache_key(normalized, compile_signature);
        auto source_hash = cache::hash_file(normalized);
        if(!source_hash.has_value()) {
            logging::warn("extract cache disabled for {}: {}",
                          normalized, source_hash.error().message);
        }

        CacheEvaluation evaluation{
            .compile_signature = compile_signature,
        };
        if(source_hash.has_value()) {
            evaluation.source_hash = *source_hash;
        }

        if(auto cache_it = cache_records.find(entry.cache_key); cache_it != cache_records.end()) {
            const auto& record = cache_it->second;
            if(evaluation.source_hash.has_value() &&
               record.compile_signature == compile_signature &&
               record.source_hash == *evaluation.source_hash) {
                seeded_scan_cache.insert_or_assign(entry.cache_key, record.scan);
                evaluation.scan_valid = true;

                if(!cache::dependencies_changed(record.ast_deps)) {
                    evaluation.ast_valid = true;
                    ++ast_cache_hits;
                }
            }
        }

        cache_evaluations.insert_or_assign(entry.cache_key, evaluation);
    }

    logging::info("extract cache: {} scan hits, {} ast hits",
                  seeded_scan_cache.size(), ast_cache_hits);

    // 2. Build dependency graph + ScanCache
    auto t1 = Clock::now();
    auto dep_result = build_dependency_graph(filtered_db, seeded_scan_cache);
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

    // 3. Extract symbols for each file
    auto t2 = Clock::now();
    ProjectModel model;
    model.file_order = std::move(*order_result);

    const auto total_entries = filtered_db.entries.size();
    for(std::size_t idx = 0; idx < filtered_db.entries.size(); ++idx) {
        auto& entry = filtered_db.entries[idx];
        namespace fs = std::filesystem;
        auto normalized = fs::path(entry.file).lexically_normal().generic_string();
        if(entry.cache_key.empty()) {
            return std::unexpected(ExtractError{
                .message = std::format("missing cache key for {}", normalized)});
        }
        auto cache_key = entry.cache_key;
        auto cache_state_it = cache_evaluations.find(cache_key);
        if(cache_state_it == cache_evaluations.end()) {
            return std::unexpected(ExtractError{
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
                          ast_view->symbols.size(), ast_view->relations.size(), dt_ast.count());
        } else {
            auto ast_result = extract_symbols(entry, *config.extract.max_snippet_bytes);
            if(!ast_result.has_value()) {
                return std::unexpected(ExtractError{
                    .message = std::format("failed to extract symbols from {}: {}",
                                           entry.file, ast_result.error().message)});
            }
            ast_data = std::move(*ast_result);
            ast_view = &ast_data;
            auto ast_deps_result = cache::capture_dependency_snapshot(ast_data.dependencies);
            if(!ast_deps_result.has_value()) {
                logging::warn("failed to capture AST dependency snapshot for {}: {}",
                              normalized, ast_deps_result.error().message);
            } else {
                ast_deps_snapshot = std::move(*ast_deps_result);
            }

            auto dt_ast = std::chrono::duration_cast<Ms>(Clock::now() - t_ast);
            logging::info("  ast: {} symbols, {} relations ({}ms)",
                          ast_data.symbols.size(), ast_data.relations.size(), dt_ast.count());
        }

        auto cache_it = scan_cache.find(cache_key);

        auto& current_file_info = model.files[normalized];
        current_file_info.path = normalized;
        std::size_t includes_kept = 0;

        if(cache_it == scan_cache.end()) {
            logging::warn("scan cache miss for {}, re-scanning", entry.file);
            auto scan_result = scan_file(entry);
            if(!scan_result.has_value()) {
                return std::unexpected(ExtractError{
                    .message = std::format("failed to scan includes for {}: {}",
                                           entry.file, scan_result.error().message)});
            }

            auto [rescanned_it, _] = scan_cache.insert_or_assign(cache_key, std::move(*scan_result));
            cache_it = rescanned_it;
        }

        for(auto& inc : cache_it->second.includes) {
            namespace fs = std::filesystem;
            auto inc_path = fs::path(inc.path);
            if(inc_path.is_relative()) {
                inc_path = fs::path(entry.directory) / inc_path;
            }
            inc_path = inc_path.lexically_normal();
            if(matches_filter(inc_path.string(), config.filter, filter_root)) {
                append_unique(current_file_info.includes, inc_path.generic_string());
                ++includes_kept;
            }
        }

        std::size_t symbols_kept = 0;
        for(const auto& sym : ast_view->symbols) {
            namespace fs = std::filesystem;
            auto decl_file = fs::path(sym.declaration_location.file);
            if(decl_file.is_relative()) {
                decl_file = fs::path(entry.directory) / decl_file;
            }
            decl_file = decl_file.lexically_normal();

            if(!matches_filter(decl_file.string(), config.filter, filter_root)) {
                continue;
            }

            auto owner_path_result = resolve_path_under_directory(sym.declaration_location.file,
                                                                  entry.directory);
            if(!owner_path_result.has_value()) {
                return std::unexpected(std::move(owner_path_result.error()));
            }

            auto owner_path = owner_path_result->lexically_normal();
            if(!matches_filter(owner_path.string(), config.filter, filter_root)) {
                continue;
            }

            auto normalized_sym = sym;
            normalized_sym.declaration_location.file = owner_path.generic_string();
            if(normalized_sym.definition_location.has_value() &&
               !normalized_sym.definition_location->file.empty()) {
                auto definition_path_result = resolve_path_under_directory(
                    normalized_sym.definition_location->file,
                    entry.directory);
                if(!definition_path_result.has_value()) {
                    return std::unexpected(std::move(definition_path_result.error()));
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

        for(const auto& rel : ast_view->relations) {
            auto from_it = model.symbols.find(rel.from);
            if(from_it == model.symbols.end()) continue;

            if(rel.is_call) {
                append_unique(from_it->second.calls, rel.to);
            } else {
                append_unique(from_it->second.references, rel.to);
            }
        }

        if(cache_state_it->second.source_hash.has_value()) {
            if(using_cached_ast) {
                auto& record = cache_record_it->second;
                record.compile_signature = cache_state_it->second.compile_signature;
                record.source_hash = *cache_state_it->second.source_hash;
                record.scan = cache_it->second;
            } else {
                cache_records.insert_or_assign(cache_key, cache::CacheRecord{
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
                      symbols_kept, includes_kept, dt_file.count());
    }

    rebuild_model_indexes(config, model);

    // 4. Build reverse edges
    auto t3 = Clock::now();
    logging::info("building reverse edges for {} symbols...", model.symbols.size());
    for(auto& [id, sym] : model.symbols) {
        for(auto& callee_id : sym.calls) {
            auto callee_it = model.symbols.find(callee_id);
            if(callee_it != model.symbols.end()) {
                append_unique(callee_it->second.called_by, id);
            }
        }
        for(auto& ref_id : sym.references) {
            auto ref_it = model.symbols.find(ref_id);
            if(ref_it != model.symbols.end()) {
                append_unique(ref_it->second.referenced_by, id);
            }
        }
    }
    auto dt_reverse = std::chrono::duration_cast<Ms>(Clock::now() - t3);
    logging::info("reverse edges done ({}ms)", dt_reverse.count());

    // 5. Build module information from scan results
    if(auto module_result = build_module_info(model, scan_cache); !module_result.has_value()) {
        return std::unexpected(std::move(module_result.error()));
    }
    rebuild_lookup_maps(model);
    if(model.uses_modules) {
        logging::info("detected {} module units", model.modules.size());
        for(auto& [source_file, mod] : model.modules) {
            logging::info("  module '{}' (interface={}) from {}",
                          mod.name, mod.is_interface, source_file);
        }
    }

    auto save_cache_result = cache::save_extract_cache(config.workspace_root, cache_records);
    if(!save_cache_result.has_value()) {
        logging::warn("failed to save extract cache; continuing without persisted cache: {}",
                      save_cache_result.error().message);
    }

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
