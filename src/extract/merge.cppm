export module extract:merge;

import std;
import config;
import :model;
import :filter;
import support;

export namespace clore::extract {

template <typename T>
void append_unique(std::vector<T>& values, const T& value) {
    if(std::find(values.begin(), values.end(), value) == values.end()) {
        values.push_back(value);
    }
}

template <typename T>
void append_unique_range(std::vector<T>& values, const std::vector<T>& incoming) {
    if(incoming.empty()) {
        return;
    }
    // When incoming is the smaller collection, build the set from it
    // instead of from values. This avoids O(|values|) space overhead
    // which becomes expensive as values grows across repeated merges.
    if(incoming.size() <= values.size()) {
        std::unordered_set<T> remaining(incoming.begin(), incoming.end());
        for(const auto& value: values) {
            remaining.erase(value);
        }
        for(const auto& value: incoming) {
            if(remaining.contains(value)) {
                values.push_back(value);
                remaining.erase(value);
            }
        }
        return;
    }
    std::unordered_set<T> seen(values.begin(), values.end());
    seen.reserve(values.size() + incoming.size());
    for(const auto& value: incoming) {
        if(seen.insert(value).second) {
            values.push_back(value);
        }
    }
}

template <typename T>
void deduplicate(std::vector<T>& values) {
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
}

auto merge_symbol_info(SymbolInfo& current, SymbolInfo&& incoming) -> void;
auto merge_symbol_info(SymbolInfo& current, const SymbolInfo& incoming) -> void;

auto rebuild_model_indexes(const config::TaskConfig& config, ProjectModel& model) -> void;

auto rebuild_lookup_maps(ProjectModel& model) -> void;

}  // namespace clore::extract

namespace clore::extract {

namespace {

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
    for(const auto& part: parts) {
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

auto find_enclosing_namespace(const ProjectModel& model,
                              const SymbolInfo& sym,
                              const std::unordered_set<std::string>& known_namespace_names)
    -> std::string {
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

    auto ns = namespace_prefix_from_qualified_name(sym.qualified_name);
    if(!ns.empty() && known_namespace_names.contains(ns)) {
        return ns;
    }
    return {};
}

template <typename T>
auto merge_symbol_info_impl(SymbolInfo& current, T&& incoming) -> void {
    const auto& incoming_ref = incoming;

    const bool prefer_incoming_definition =
        incoming_ref.definition_location.has_value() && !current.definition_location.has_value();
    const bool prefer_incoming_snippet =
        prefer_incoming_definition ||
        (current.source_snippet_length < incoming_ref.source_snippet_length &&
         incoming_ref.source_snippet_length > 0);

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
        current.source_snippet_offset = incoming_ref.source_snippet_offset;
        current.source_snippet_length = incoming_ref.source_snippet_length;
    } else if(current.source_snippet_length == 0 && incoming_ref.source_snippet_length > 0) {
        current.source_snippet = incoming_ref.source_snippet;
        current.source_snippet_offset = incoming_ref.source_snippet_offset;
        current.source_snippet_length = incoming_ref.source_snippet_length;
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
}

}  // namespace

auto merge_symbol_info(SymbolInfo& current, SymbolInfo&& incoming) -> void {
    merge_symbol_info_impl(current, std::move(incoming));
}

auto merge_symbol_info(SymbolInfo& current, const SymbolInfo& incoming) -> void {
    merge_symbol_info_impl(current, incoming);
}

auto rebuild_model_indexes(const config::TaskConfig& config, ProjectModel& model) -> void {
    namespace fs = std::filesystem;

    auto run_parallel_chunks = [&](std::size_t total, auto&& worker) {
        if(total == 0) {
            return;
        }

        const auto hardware_threads = std::thread::hardware_concurrency();
        const auto num_threads =
            std::max<std::size_t>(1, hardware_threads == 0 ? 1 : hardware_threads);
        const auto per_thread = (total + num_threads - 1) / num_threads;

        std::vector<std::thread> threads;
        threads.reserve(num_threads);
        for(std::size_t thread_index = 0; thread_index < num_threads; ++thread_index) {
            auto begin = thread_index * per_thread;
            auto end = std::min(begin + per_thread, total);
            if(begin >= end) {
                break;
            }
            threads.emplace_back(worker, thread_index, begin, end);
        }
        for(auto& thread: threads) {
            thread.join();
        }
    };

    for(auto& [_, file_info]: model.files) {
        file_info.symbols.clear();
        deduplicate(file_info.includes);
    }

    model.namespaces.clear();

    for(auto& [_, sym]: model.symbols) {
        deduplicate(sym.calls);
        deduplicate(sym.references);
        sym.children.clear();
        // derived is built incrementally during relation merging (inheritance
        // relations), so it only needs deduplication here.
        deduplicate(sym.derived);
        // called_by / referenced_by are built incrementally during relation
        // merging, so they only need deduplication here.
        deduplicate(sym.called_by);
        deduplicate(sym.referenced_by);
    }

    auto filter_root = filter_root_path(config);
    std::vector<SymbolID> symbol_ids;
    symbol_ids.reserve(model.symbols.size());
    for(const auto& [symbol_id, _]: model.symbols) {
        symbol_ids.push_back(symbol_id);
    }

    // Collect all known namespace qualified names for strict validation
    // when inferring namespace from qualified name prefixes.
    std::unordered_set<std::string> known_namespace_names;
    known_namespace_names.reserve(model.symbols.size() / 8);
    for(const auto& [_, sym]: model.symbols) {
        if(sym.kind == SymbolKind::Namespace) {
            known_namespace_names.insert(sym.qualified_name);
        }
    }

    std::unordered_map<std::string,
                       std::vector<SymbolID>,
                       clore::support::TransparentStringHash,
                       clore::support::TransparentStringEqual>
        file_symbols;
    std::unordered_map<std::string,
                       std::vector<SymbolID>,
                       clore::support::TransparentStringHash,
                       clore::support::TransparentStringEqual>
        namespace_symbols;
    std::vector<std::pair<SymbolID, SymbolID>> parent_children;
    std::vector<std::string> explicit_namespaces;

    struct LocalIndexData {
        std::vector<std::pair<std::string, SymbolID>> file_symbols;
        std::vector<std::pair<std::string, SymbolID>> namespace_symbols;
        std::vector<std::pair<SymbolID, SymbolID>> parent_children;
        std::vector<std::string> explicit_namespaces;
    };

    const auto hardware_threads = std::thread::hardware_concurrency();
    const auto num_threads = std::max<std::size_t>(1, hardware_threads == 0 ? 1 : hardware_threads);
    std::vector<LocalIndexData> local_data(num_threads);

    run_parallel_chunks(
        symbol_ids.size(),
        [&](std::size_t thread_index, std::size_t begin, std::size_t end) {
            auto& local = local_data[thread_index];
            local.file_symbols.reserve(end - begin);
            local.namespace_symbols.reserve(end - begin);
            local.parent_children.reserve(end - begin);

            for(std::size_t i = begin; i < end; ++i) {
                auto symbol_id = symbol_ids[i];
                auto sym_it = model.symbols.find(symbol_id);
                if(sym_it == model.symbols.end()) {
                    continue;
                }
                const auto& sym = sym_it->second;

                auto owner_path = fs::path(sym.declaration_location.file);
                if(owner_path.is_relative()) {
                    owner_path = filter_root / owner_path;
                }
                owner_path = owner_path.lexically_normal();

                if(matches_filter(owner_path.string(), config.filter, filter_root)) {
                    local.file_symbols.emplace_back(owner_path.generic_string(), symbol_id);
                }

                if(sym.kind == SymbolKind::Namespace) {
                    local.explicit_namespaces.push_back(sym.qualified_name);
                } else {
                    auto ns_name = find_enclosing_namespace(model, sym, known_namespace_names);
                    if(!ns_name.empty()) {
                        local.namespace_symbols.emplace_back(std::move(ns_name), symbol_id);
                    }
                }

                if(sym.parent.has_value()) {
                    local.parent_children.emplace_back(*sym.parent, symbol_id);
                }
            }
        });

    for(auto& local: local_data) {
        for(auto& [owner_key, symbol_id]: local.file_symbols) {
            file_symbols[owner_key].push_back(symbol_id);
        }
        for(auto& [namespace_name, symbol_id]: local.namespace_symbols) {
            namespace_symbols[namespace_name].push_back(symbol_id);
        }
        parent_children.insert(parent_children.end(),
                               local.parent_children.begin(),
                               local.parent_children.end());
        explicit_namespaces.insert(explicit_namespaces.end(),
                                   local.explicit_namespaces.begin(),
                                   local.explicit_namespaces.end());
    }

    for(auto& [owner_key, ids]: file_symbols) {
        auto& owner_file_info = model.files[owner_key];
        owner_file_info.path = owner_key;
        owner_file_info.symbols = std::move(ids);
    }

    for(auto& namespace_name: explicit_namespaces) {
        (void)ensure_namespace_hierarchy(model, namespace_name);
    }
    for(auto& [namespace_name, ids]: namespace_symbols) {
        auto canonical_ns_name = ensure_namespace_hierarchy(model, namespace_name);
        if(canonical_ns_name.empty()) {
            continue;
        }
        auto& ns_info = model.namespaces[canonical_ns_name];
        ns_info.name = canonical_ns_name;
        auto& target = ns_info.symbols;
        target.insert(target.end(), ids.begin(), ids.end());
    }

    for(const auto& [parent_id, child_id]: parent_children) {
        auto parent_it = model.symbols.find(parent_id);
        if(parent_it != model.symbols.end()) {
            parent_it->second.children.push_back(child_id);
        }
    }

    std::vector<FileInfo*> file_infos;
    file_infos.reserve(model.files.size());
    for(auto& [_, file_info]: model.files) {
        file_infos.push_back(&file_info);
    }
    run_parallel_chunks(file_infos.size(), [&](std::size_t, std::size_t begin, std::size_t end) {
        for(std::size_t i = begin; i < end; ++i) {
            deduplicate(file_infos[i]->symbols);
        }
    });

    std::vector<NamespaceInfo*> namespace_infos;
    namespace_infos.reserve(model.namespaces.size());
    for(auto& [_, ns_info]: model.namespaces) {
        namespace_infos.push_back(&ns_info);
    }
    run_parallel_chunks(namespace_infos.size(),
                        [&](std::size_t, std::size_t begin, std::size_t end) {
                            for(std::size_t i = begin; i < end; ++i) {
                                deduplicate(namespace_infos[i]->symbols);
                                deduplicate(namespace_infos[i]->children);
                            }
                        });

    std::vector<SymbolInfo*> symbols;
    symbols.reserve(model.symbols.size());
    for(auto& [_, sym]: model.symbols) {
        symbols.push_back(&sym);
    }
    run_parallel_chunks(symbols.size(), [&](std::size_t, std::size_t begin, std::size_t end) {
        for(std::size_t i = begin; i < end; ++i) {
            deduplicate(symbols[i]->children);
            deduplicate(symbols[i]->derived);
        }
    });
}

auto rebuild_lookup_maps(ProjectModel& model) -> void {
    model.symbol_ids_by_qualified_name.clear();
    model.module_name_to_sources.clear();

    for(auto& [symbol_id, sym]: model.symbols) {
        if(!sym.qualified_name.empty()) {
            model.symbol_ids_by_qualified_name[sym.qualified_name].push_back(symbol_id);
        }
    }

    for(auto& [qualified_name, symbol_ids]: model.symbol_ids_by_qualified_name) {
        std::sort(symbol_ids.begin(), symbol_ids.end(), [&](SymbolID lhs, SymbolID rhs) {
            auto lhs_it = model.symbols.find(lhs);
            auto rhs_it = model.symbols.find(rhs);
            if(lhs_it == model.symbols.end()) {
                return rhs_it != model.symbols.end();
            }
            if(rhs_it == model.symbols.end()) {
                return false;
            }
            const auto& lhs_sym = lhs_it->second;
            const auto& rhs_sym = rhs_it->second;
            if(lhs_sym.signature != rhs_sym.signature) {
                return lhs_sym.signature < rhs_sym.signature;
            }
            if(lhs_sym.declaration_location.file != rhs_sym.declaration_location.file) {
                return lhs_sym.declaration_location.file < rhs_sym.declaration_location.file;
            }
            if(lhs_sym.declaration_location.line != rhs_sym.declaration_location.line) {
                return lhs_sym.declaration_location.line < rhs_sym.declaration_location.line;
            }
            return lhs < rhs;
        });
        symbol_ids.erase(std::unique(symbol_ids.begin(), symbol_ids.end()), symbol_ids.end());
        if(symbol_ids.size() > 1) {
            logging::info("qualified name '{}' has {} overload candidates",
                          qualified_name,
                          symbol_ids.size());
        }
    }

    for(auto& [source_file, mod_unit]: model.modules) {
        if(!mod_unit.name.empty()) {
            auto& sources = model.module_name_to_sources[mod_unit.name];
            sources.push_back(source_file);
        }
    }

    for(auto& [module_name, sources]: model.module_name_to_sources) {
        std::sort(sources.begin(), sources.end());
        sources.erase(std::unique(sources.begin(), sources.end()), sources.end());

        std::size_t interface_count = 0;
        std::string first_interface;
        std::string second_interface;
        for(const auto& source: sources) {
            auto it = model.modules.find(source);
            if(it != model.modules.end() && it->second.is_interface) {
                ++interface_count;
                if(interface_count == 1) {
                    first_interface = source;
                } else if(interface_count == 2) {
                    second_interface = source;
                }
            }
        }
        if(interface_count > 1) {
            logging::warn("duplicate module interface '{}' from '{}' and '{}'",
                          module_name,
                          first_interface,
                          second_interface);
        }
    }
}

}  // namespace clore::extract
