export module generate:diagram;

import std;
import :model;
import config;
import extract;
import support;

export namespace clore::generate {

auto should_emit_mermaid(std::size_t node_count, std::size_t edge_count) -> bool;

auto escape_mermaid_label(std::string_view text) -> std::string;

auto render_import_diagram_code(const extract::ModuleUnit& mod_unit) -> std::string;

auto render_namespace_diagram_code(const extract::ProjectModel& model,
                                   std::string_view namespace_name) -> std::string;

auto render_file_dependency_diagram_code(const PagePlan& plan,
                                         const config::TaskConfig& config,
                                         const extract::ProjectModel& model) -> std::string;

auto render_module_dependency_diagram_code(const extract::ProjectModel& model) -> std::string;

}  // namespace clore::generate

namespace clore::generate {

namespace {

constexpr std::size_t kMermaidMinNodes = 4;
constexpr std::size_t kMermaidMinEdges = 3;

template <typename RenderFn>
auto render_cached_diagram(RenderFn&& render_fn) -> std::string {
    return render_fn();
}

auto is_std_name(std::string_view name) -> bool {
    return name == "std" || name.starts_with("std::");
}

auto short_name_of_local(std::string_view qualified_name) -> std::string {
    auto parts = extract::split_top_level_qualified_name(qualified_name);
    if(parts.empty()) {
        return {};
    }
    return parts.back();
}

auto is_variable_kind_local(extract::SymbolKind kind) -> bool {
    return kind == extract::SymbolKind::Variable || kind == extract::SymbolKind::EnumMember;
}

template <typename Predicate>
auto collect_implementation_symbols_for_diagram(const PagePlan& plan,
                                                const extract::ProjectModel& model,
                                                Predicate&& predicate)
    -> std::vector<const extract::SymbolInfo*> {
    std::vector<const extract::SymbolInfo*> symbols;
    std::unordered_set<extract::SymbolID> seen;

    for(const auto& key: plan.owner_keys) {
        if(plan.page_type == PageType::Module) {
            for(auto* module: extract::find_modules_by_name(model, key)) {
                for(auto sym_id: module->symbols) {
                    if(!seen.insert(sym_id).second) {
                        continue;
                    }
                    if(auto* sym = extract::lookup_symbol(model, sym_id);
                       sym != nullptr && is_page_level_symbol(model, *sym) && predicate(*sym)) {
                        symbols.push_back(sym);
                    }
                }
            }
            continue;
        }

        auto file_it = model.files.find(key);
        if(file_it == model.files.end()) {
            continue;
        }
        for(auto sym_id: file_it->second.symbols) {
            if(!seen.insert(sym_id).second) {
                continue;
            }
            if(auto* sym = extract::lookup_symbol(model, sym_id);
               sym != nullptr && is_page_level_symbol(model, *sym) && predicate(*sym)) {
                symbols.push_back(sym);
            }
        }
    }

    std::sort(symbols.begin(),
              symbols.end(),
              [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  return lhs->qualified_name < rhs->qualified_name;
              });
    return symbols;
}

}  // namespace

auto should_emit_mermaid(std::size_t node_count, std::size_t edge_count) -> bool {
    return node_count >= kMermaidMinNodes || edge_count >= kMermaidMinEdges;
}

auto escape_mermaid_label(std::string_view text) -> std::string {
    std::string escaped;
    escaped.reserve(text.size());
    for(auto ch: text) {
        switch(ch) {
            case '\\': escaped += "\\\\"; break;
            case '"': escaped += "\\\""; break;
            case '\n':
            case '\r': escaped += ' '; break;
            default: escaped.push_back(ch); break;
        }
    }
    return escaped;
}

auto render_import_diagram_code(const extract::ModuleUnit& mod_unit) -> std::string {
    return render_cached_diagram([&]() {
        if(mod_unit.imports.empty()) {
            return std::string{};
        }
        auto top_module = [](std::string_view name) -> std::string {
            auto colon = name.find(':');
            return std::string(colon == std::string_view::npos ? name : name.substr(0, colon));
        };

        auto module_label = top_module(mod_unit.name);
        if(is_std_name(module_label)) {
            return std::string{};
        }

        std::unordered_set<std::string> seen;
        std::vector<std::string> imports;
        for(const auto& imp: mod_unit.imports) {
            auto label = top_module(imp);
            if(label == module_label || is_std_name(label) || !seen.insert(label).second) {
                continue;
            }
            imports.push_back(label);
        }

        auto edge_count = imports.size();
        auto node_count = 1 + imports.size();
        if(!should_emit_mermaid(node_count, edge_count)) {
            return std::string{};
        }

        std::sort(imports.begin(), imports.end());

        std::string result = "graph LR\n";
        result += "    M0[\"" + escape_mermaid_label(module_label) + "\"]\n";
        for(std::size_t i = 0; i < imports.size(); ++i) {
            auto node_id = "I" + std::to_string(i);
            result += "    " + node_id + "[\"" + escape_mermaid_label(imports[i]) + "\"]\n";
            result += "    " + node_id + " --> M0\n";
        }
        return result;
    });
}

auto render_namespace_diagram_code(const extract::ProjectModel& model,
                                   std::string_view namespace_name) -> std::string {
    return render_cached_diagram([&]() {
        auto ns_it = model.namespaces.find(std::string(namespace_name));
        if(ns_it == model.namespaces.end()) {
            return std::string{};
        }
        std::vector<const extract::SymbolInfo*> types;
        std::unordered_set<extract::SymbolID> seen_types;
        for(auto sym_id: ns_it->second.symbols) {
            if(auto* sym = extract::lookup_symbol(model, sym_id);
               sym != nullptr && is_type_kind(sym->kind) && seen_types.insert(sym->id).second) {
                types.push_back(sym);
            }
        }
        std::sort(types.begin(),
                  types.end(),
                  [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                      return lhs->qualified_name < rhs->qualified_name;
                  });

        std::vector<std::string> children;
        for(const auto& child: ns_it->second.children) {
            if(!child.contains("(anonymous namespace)") && !is_std_name(child)) {
                children.push_back(short_name_of_local(child));
            }
        }
        std::sort(children.begin(), children.end());
        children.erase(std::unique(children.begin(), children.end()), children.end());

        auto edge_count = types.size() + children.size();
        auto node_count = 1 + edge_count;
        if(!should_emit_mermaid(node_count, edge_count)) {
            return std::string{};
        }

        std::string result = "graph TD\n";
        result += "    NS[\"" + escape_mermaid_label(short_name_of_local(namespace_name)) + "\"]\n";
        for(std::size_t i = 0; i < types.size(); ++i) {
            auto node_id = "T" + std::to_string(i);
            auto label = short_name_of_local(types[i]->qualified_name);
            result += "    " + node_id + "[\"" + escape_mermaid_label(label) + "\"]\n";
            result += "    NS --> " + node_id + "\n";
        }
        for(std::size_t child_index = 0; child_index < children.size(); ++child_index) {
            auto node_id = "NSC" + std::to_string(child_index);
            result +=
                "    " + node_id + "[\"" + escape_mermaid_label(children[child_index]) + "\"]\n";
            result += "    NS --> " + node_id + "\n";
        }
        return result;
    });
}

auto render_file_dependency_diagram_code(const PagePlan& plan,
                                         const config::TaskConfig& config,
                                         const extract::ProjectModel& model) -> std::string {
    if(plan.owner_keys.empty()) {
        return {};
    }

    return render_cached_diagram([&]() {
        auto file_it = model.files.find(plan.owner_keys.front());
        if(file_it == model.files.end()) {
            return std::string{};
        }

        std::vector<std::string> include_labels;
        include_labels.reserve(file_it->second.includes.size());
        for(const auto& include: file_it->second.includes) {
            auto label = make_source_relative(include, config.project_root);
            if(label.empty()) {
                label = include;
            }
            include_labels.push_back(std::move(label));
        }
        std::sort(include_labels.begin(), include_labels.end());
        include_labels.erase(std::unique(include_labels.begin(), include_labels.end()),
                             include_labels.end());

        auto symbols = collect_implementation_symbols_for_diagram(
            plan,
            model,
            [](const extract::SymbolInfo& sym) {
                return is_type_kind(sym.kind) || is_variable_kind_local(sym.kind) ||
                       is_function_kind(sym.kind);
            });

        auto edge_count = include_labels.size() + symbols.size();
        auto node_count = 1 + edge_count;
        if(!should_emit_mermaid(node_count, edge_count)) {
            return std::string{};
        }

        auto file_label = make_source_relative(plan.owner_keys.front(), config.project_root);
        if(file_label.empty()) {
            file_label = plan.owner_keys.front();
        }

        std::string result = "graph LR\n";
        result += "    F[\"" + escape_mermaid_label(file_label) + "\"]\n";
        for(std::size_t i = 0; i < include_labels.size(); ++i) {
            auto node_id = "I" + std::to_string(i);
            result += "    " + node_id + "[\"" + escape_mermaid_label(include_labels[i]) + "\"]\n";
            result += "    " + node_id + " --> F\n";
        }
        for(std::size_t i = 0; i < symbols.size(); ++i) {
            auto node_id = "S" + std::to_string(i);
            auto symbol_label = short_name_of_local(symbols[i]->qualified_name);
            if(symbol_label.empty()) {
                symbol_label =
                    symbols[i]->name.empty() ? symbols[i]->qualified_name : symbols[i]->name;
            }
            result += "    " + node_id + "[\"" + escape_mermaid_label(symbol_label) + "\"]\n";
            result += "    F --> " + node_id + "\n";
        }

        return result;
    });
}

auto render_module_dependency_diagram_code(const extract::ProjectModel& model) -> std::string {
    return render_cached_diagram([&]() {
        auto top_module = [](std::string_view name) -> std::string {
            auto colon = name.find(':');
            return std::string(colon == std::string_view::npos ? name : name.substr(0, colon));
        };

        std::unordered_map<std::string, std::unordered_set<std::string>> deps;
        std::unordered_set<std::string> modules;
        for(const auto& [_, mod_unit]: model.modules) {
            if(!mod_unit.is_interface) {
                continue;
            }
            auto from = top_module(mod_unit.name);
            if(is_std_name(from)) {
                continue;
            }
            modules.insert(from);
            for(const auto& imp: mod_unit.imports) {
                auto to = top_module(imp);
                if(to != from && !is_std_name(to)) {
                    deps[from].insert(to);
                    modules.insert(to);
                }
            }
        }
        if(modules.size() < 2) {
            return std::string{};
        }

        std::size_t edge_count = 0;
        for(const auto& [_, to_set]: deps) {
            edge_count += to_set.size();
        }
        if(!should_emit_mermaid(modules.size(), edge_count)) {
            return std::string{};
        }

        std::vector<std::string> sorted(modules.begin(), modules.end());
        std::sort(sorted.begin(), sorted.end());
        std::unordered_map<std::string, std::string> node_ids;
        std::string result = "graph LR\n";
        for(std::size_t i = 0; i < sorted.size(); ++i) {
            auto node_id = std::format("M{}", i);
            node_ids.emplace(sorted[i], node_id);
            result += "    " + node_id + "[\"" + escape_mermaid_label(sorted[i]) + "\"]\n";
        }
        for(const auto& from: sorted) {
            auto dep_it = deps.find(from);
            if(dep_it == deps.end()) {
                continue;
            }
            std::vector<std::string> targets(dep_it->second.begin(), dep_it->second.end());
            std::sort(targets.begin(), targets.end());
            for(const auto& to: targets) {
                result += "    " + node_ids.at(to) + " --> " + node_ids.at(from) + "\n";
            }
        }
        return result;
    });
}

}  // namespace clore::generate
