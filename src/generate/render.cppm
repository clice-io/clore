module;

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

export module generate:render;

import :model;
import config;
import extract;
import support;

export namespace clore::generate {

struct RenderError {
    std::string message;
};

auto load_page_template(std::string_view path) -> std::expected<std::string, RenderError>;

auto build_link_resolver(const PagePlanSet& plan_set) -> LinkResolver;

auto render_deterministic_block(std::string_view block_name,
                                const PagePlan& plan,
                                const extract::ProjectModel& model,
                                const config::TaskConfig& config,
                                const LinkResolver& links) -> std::string;

auto assemble_page(const std::string& page_template,
                   const std::string& title,
                   const std::unordered_map<std::string, std::string>& blocks,
                   const std::unordered_map<std::string, std::string>& slots,
                   bool fail_on_empty_section = false)
    -> std::expected<std::string, RenderError>;

auto validate_output(const std::string& content,
                     const config::ValidationConfig& validation)
    -> std::expected<void, RenderError>;

auto write_page(const GeneratedPage& page, std::string_view output_root)
    -> std::expected<void, RenderError>;

}  // namespace clore::generate

namespace clore::generate {

auto load_page_template(std::string_view path) -> std::expected<std::string, RenderError> {
    namespace fs = std::filesystem;

    if(path.empty()) {
        return std::unexpected(RenderError{.message = "page template path is empty"});
    }

    auto content = clore::support::read_utf8_text_file(fs::path(path));
    if(!content.has_value()) {
        return std::unexpected(RenderError{
            .message = std::format("failed to read page template: {}",
                                   content.error())});
    }

    if(content->empty()) {
        return std::unexpected(RenderError{
            .message = std::format("page template is empty: {}", path)});
    }

    return std::move(*content);
}

namespace {

auto lookup_sym(const extract::ProjectModel& model, extract::SymbolID id)
    -> const extract::SymbolInfo* {
    auto it = model.symbols.find(id);
    return it != model.symbols.end() ? &it->second : nullptr;
}

auto is_type_kind(extract::SymbolKind kind) -> bool {
    switch(kind) {
        case extract::SymbolKind::Class:
        case extract::SymbolKind::Struct:
        case extract::SymbolKind::Enum:
        case extract::SymbolKind::Union:
        case extract::SymbolKind::Concept:
        case extract::SymbolKind::Template:
        case extract::SymbolKind::TypeAlias:
            return true;
        default:
            return false;
    }
}

auto make_source_relative(const std::string& path, const std::string& project_root) -> std::string {
    namespace fs = std::filesystem;
    if(path.empty() || project_root.empty()) return path;
    auto abs = fs::path(path).lexically_normal();
    auto root = fs::path(project_root).lexically_normal();
    auto rel = abs.lexically_relative(root);
    if(rel.empty() || rel.string().starts_with("..")) return path;
    return rel.generic_string();
}

auto make_relative_link_target(std::string_view current_page_path,
                               std::string_view target_page_path) -> std::string {
    namespace fs = std::filesystem;

    auto current = fs::path(current_page_path).lexically_normal();
    auto target = fs::path(target_page_path).lexically_normal();
    auto base = current.has_parent_path() ? current.parent_path() : fs::path{"."};
    auto rel = target.lexically_relative(base);
    if(rel.empty()) {
        return target.generic_string();
    }
    return rel.generic_string();
}

auto make_link(const std::string& name, std::string_view current_page_path,
               const LinkResolver& links) -> std::string {
    if(auto* path = links.resolve(name)) {
        auto pos = name.rfind("::");
        auto display = (pos != std::string::npos) ? name.substr(pos + 2) : name;
        return "[`" + display + "`](" +
               make_relative_link_target(current_page_path, *path) + ")";
    }
    return "`" + name + "`";
}

auto make_link_full(const std::string& name, std::string_view current_page_path,
                    const LinkResolver& links) -> std::string {
    if(auto* path = links.resolve(name)) {
        return "[`" + name + "`](" +
               make_relative_link_target(current_page_path, *path) + ")";
    }
    return "`" + name + "`";
}

auto render_declaration_block(const PagePlan& plan, const extract::ProjectModel& model) -> std::string {
    std::string result;
    for(auto& key : plan.owner_keys) {
        for(auto& [id, sym] : model.symbols) {
            if(sym.qualified_name == key) {
                if(!sym.source_snippet.empty()) {
                    result += "```cpp\n" + sym.source_snippet + "\n```\n\n";
                } else if(!sym.signature.empty()) {
                    result += "```cpp\n" + sym.signature + "\n```\n\n";
                }
                break;
            }
        }
    }
    return result;
}

auto render_template_parameters_block(const PagePlan& plan, const extract::ProjectModel& model) -> std::string {
    std::string result;
    for(auto& key : plan.owner_keys) {
        for(auto& [id, sym] : model.symbols) {
            if(sym.qualified_name == key && sym.is_template && !sym.template_params.empty()) {
                result += "| Parameter | Description |\n|---|---|\n";
                result += "| `" + sym.template_params + "` | |\n\n";
                break;
            }
        }
    }
    return result;
}

auto render_base_types_block(const PagePlan& plan, const extract::ProjectModel& model,
                             const LinkResolver& links) -> std::string {
    std::string result;
    for(auto& key : plan.owner_keys) {
        for(auto& [id, sym] : model.symbols) {
            if(sym.qualified_name == key && !sym.bases.empty()) {
                for(auto& base_id : sym.bases) {
                    if(auto* base = lookup_sym(model, base_id)) {
                        result += "- " +
                                  make_link_full(base->qualified_name, plan.relative_path, links) +
                                  "\n";
                    }
                }
                result += "\n";
                break;
            }
        }
    }
    return result;
}

auto render_derived_types_block(const PagePlan& plan, const extract::ProjectModel& model,
                                const LinkResolver& links) -> std::string {
    std::string result;
    for(auto& key : plan.owner_keys) {
        for(auto& [id, sym] : model.symbols) {
            if(sym.qualified_name == key && !sym.derived.empty()) {
                for(auto& derived_id : sym.derived) {
                    if(auto* derived = lookup_sym(model, derived_id)) {
                        result += "- " +
                                  make_link_full(derived->qualified_name, plan.relative_path, links) +
                                  "\n";
                    }
                }
                result += "\n";
                break;
            }
        }
    }
    return result;
}

auto render_members_block(const PagePlan& plan, const extract::ProjectModel& model,
                          std::string_view access_filter) -> std::string {
    std::string result;
    for(auto& key : plan.owner_keys) {
        for(auto& [id, sym] : model.symbols) {
            if(sym.qualified_name != key) continue;
            for(auto& child_id : sym.children) {
                auto* child = lookup_sym(model, child_id);
                if(!child) continue;
                bool match = access_filter.empty() ||
                             child->access == access_filter ||
                             (access_filter == "public" && child->access.empty());
                if(!match) continue;
                result += "- ";
                result += std::string(extract::symbol_kind_name(child->kind));
                result += " `" + child->name + "`";
                if(!child->signature.empty()) {
                    result += ": `" + child->signature + "`";
                }
                result += "\n";
            }
            break;
        }
    }
    if(!result.empty()) result += "\n";
    return result;
}

auto render_imports_block(const PagePlan& plan, const extract::ProjectModel& model,
                          const LinkResolver& links) -> std::string {
    std::string result;
    for(auto& key : plan.owner_keys) {
        for(auto& [source_file, mod_unit] : model.modules) {
            if(mod_unit.name == key) {
                for(auto& imp : mod_unit.imports) {
                    result += "- " + make_link_full(imp, plan.relative_path, links) + "\n";
                }
                break;
            }
        }
    }
    if(!result.empty()) result += "\n";
    return result;
}

auto render_subnamespaces_block(const PagePlan& plan, const extract::ProjectModel& model,
                                const LinkResolver& links) -> std::string {
    std::string result;
    for(auto& key : plan.owner_keys) {
        auto ns_it = model.namespaces.find(key);
        if(ns_it == model.namespaces.end()) continue;
        for(const auto& child_ns : ns_it->second.children) {
            if(child_ns.find("(anonymous namespace)") != std::string::npos) continue;
            if(!links.resolve(child_ns)) continue;
            result += "- " + make_link_full(child_ns, plan.relative_path, links) + "\n";
        }
    }
    if(!result.empty()) result += "\n";
    return result;
}

auto render_types_index_block(const PagePlan& plan, const extract::ProjectModel& model,
                              const LinkResolver& links) -> std::string {
    std::vector<extract::SymbolID> symbol_ids;
    std::string result;

    for(auto& key : plan.owner_keys) {
        if(plan.page_type == PageType::Module) {
            for(auto& [source_file, mod_unit] : model.modules) {
                if(mod_unit.name != key) continue;
                symbol_ids.insert(symbol_ids.end(), mod_unit.symbols.begin(), mod_unit.symbols.end());
            }
            continue;
        }

        auto ns_it = model.namespaces.find(key);
        if(ns_it == model.namespaces.end()) continue;
        symbol_ids.insert(symbol_ids.end(), ns_it->second.symbols.begin(), ns_it->second.symbols.end());
    }

    std::unordered_set<extract::SymbolID> seen;
    seen.reserve(symbol_ids.size());
    for(auto sym_id : symbol_ids) {
        if(!seen.insert(sym_id).second) continue;
        auto* sym = lookup_sym(model, sym_id);
        if(!sym || !is_type_kind(sym->kind)) continue;
        if(!links.resolve(sym->qualified_name)) continue;

        result += "- ";
        result += std::string(extract::symbol_kind_name(sym->kind));
        result += " " + make_link_full(sym->qualified_name, plan.relative_path, links) + "\n";
    }

    if(!result.empty()) result += "\n";
    return result;
}

auto render_functions_index_block(const PagePlan& plan, const extract::ProjectModel& model) -> std::string {
    std::string result;
    for(auto& key : plan.owner_keys) {
        auto ns_it = model.namespaces.find(key);
        if(ns_it == model.namespaces.end()) continue;
        for(auto& sym_id : ns_it->second.symbols) {
            auto* sym = lookup_sym(model, sym_id);
            if(!sym) continue;
            if(sym->kind == extract::SymbolKind::Function) {
                result += "- `" + sym->qualified_name + "`";
                if(!sym->signature.empty()) {
                    result += ": `" + sym->signature + "`";
                }
                result += "\n";
            }
        }
    }
    if(!result.empty()) result += "\n";
    return result;
}

auto render_related_pages_block(const PagePlan& plan, const LinkResolver& links) -> std::string {
    std::string result;
    for(auto& linked : plan.linked_pages) {
        auto colon_pos = linked.find(':');
        if(colon_pos == std::string::npos) {
            continue;
        }

        auto entity_name = linked.substr(colon_pos + 1);
        auto* path = links.resolve(entity_name);
        if(!path) {
            continue;
        }

        result += "- [`" + entity_name + "`](" +
                  make_relative_link_target(plan.relative_path, *path) + ")\n";
    }
    if(!result.empty()) result += "\n";
    return result;
}

auto render_source_locations_block(const PagePlan& plan, const extract::ProjectModel& model,
                                   const config::TaskConfig& config) -> std::string {
    std::string result;
    for(auto& key : plan.owner_keys) {
        for(auto& [id, sym] : model.symbols) {
            if(sym.qualified_name != key) continue;
            if(sym.declaration_location.is_known()) {
                auto rel = make_source_relative(sym.declaration_location.file, config.project_root);
                result += "- Declared at: `" + rel + ":" +
                          std::to_string(sym.declaration_location.line) + "`\n";
            }
            if(sym.definition_location.has_value() && sym.definition_location->is_known()) {
                auto rel = make_source_relative(sym.definition_location->file, config.project_root);
                result += "- Defined at: `" + rel + ":" +
                          std::to_string(sym.definition_location->line) + "`\n";
            }
            break;
        }
    }
    if(!result.empty()) result += "\n";
    return result;
}

auto render_includes_block(const PagePlan& plan, const extract::ProjectModel& model,
                           const config::TaskConfig& config) -> std::string {
    std::string result;
    for(auto& key : plan.owner_keys) {
        auto file_it = model.files.find(key);
        if(file_it == model.files.end()) continue;
        for(auto& inc : file_it->second.includes) {
            auto rel = make_source_relative(inc, config.project_root);
            result += "- `" + rel + "`\n";
        }
    }
    if(!result.empty()) result += "\n";
    return result;
}

enum class FileSymbolRenderMode : std::uint8_t {
    Declared,
    Defined,
};

auto normalize_file_key(std::string_view file_path) -> std::string {
    return std::filesystem::path(file_path).lexically_normal().generic_string();
}

auto symbol_matches_file_mode(const extract::SymbolInfo& sym,
                              std::string_view owner_file_key,
                              FileSymbolRenderMode mode) -> bool {
    if(mode == FileSymbolRenderMode::Declared) {
        return !sym.declaration_location.file.empty() &&
               normalize_file_key(sym.declaration_location.file) == owner_file_key;
    }
    if(!sym.definition_location.has_value()) {
        return false;
    }
    return !sym.definition_location->file.empty() &&
           normalize_file_key(sym.definition_location->file) == owner_file_key;
}

auto render_file_symbols_block(const PagePlan& plan, const extract::ProjectModel& model,
                               FileSymbolRenderMode mode) -> std::string {
    std::vector<const extract::SymbolInfo*> matched_symbols;
    matched_symbols.reserve(model.symbols.size());
    std::unordered_set<extract::SymbolID> seen;
    seen.reserve(model.symbols.size());

    for(auto& key : plan.owner_keys) {
        auto owner_file_key = normalize_file_key(key);
        for(auto& [symbol_id, sym] : model.symbols) {
            if(seen.contains(symbol_id)) continue;
            if(!symbol_matches_file_mode(sym, owner_file_key, mode)) continue;
            matched_symbols.push_back(&sym);
            seen.insert(symbol_id);
        }
    }

    std::sort(matched_symbols.begin(), matched_symbols.end(),
              [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  if(lhs->qualified_name != rhs->qualified_name) {
                      return lhs->qualified_name < rhs->qualified_name;
                  }
                  if(lhs->kind != rhs->kind) {
                      return static_cast<int>(lhs->kind) < static_cast<int>(rhs->kind);
                  }
                  return lhs->name < rhs->name;
              });

    std::string result;
    for(auto* sym : matched_symbols) {
        result += "- ";
        result += std::string(extract::symbol_kind_name(sym->kind));
        result += " `" + sym->qualified_name + "`\n";
    }
    if(!result.empty()) result += "\n";
    return result;
}

auto render_declared_symbols_block(const PagePlan& plan, const extract::ProjectModel& model) -> std::string {
    return render_file_symbols_block(plan, model, FileSymbolRenderMode::Declared);
}

auto render_defined_symbols_block(const PagePlan& plan, const extract::ProjectModel& model) -> std::string {
    return render_file_symbols_block(plan, model, FileSymbolRenderMode::Defined);
}

auto render_module_info_block(const PagePlan& plan, const extract::ProjectModel& model) -> std::string {
    std::string result;
    for(auto& key : plan.owner_keys) {
        auto mod_it = model.modules.find(key);
        if(mod_it == model.modules.end()) continue;
        result += "- Module: `" + mod_it->second.name + "`\n";
        result += "- Interface: " + std::string(mod_it->second.is_interface ? "yes" : "no") + "\n";
        for(auto& imp : mod_it->second.imports) {
            result += "- Imports: `" + imp + "`\n";
        }
    }
    if(!result.empty()) result += "\n";
    return result;
}

auto render_all_modules_index(const extract::ProjectModel& model,
                              const LinkResolver& links,
                              std::string_view current_page_path) -> std::string {
    std::string result;
    std::vector<std::string> module_names;
    std::unordered_set<std::string> seen;
    for(auto& [source_file, mod_unit] : model.modules) {
        if(!mod_unit.is_interface) continue;
        if(!seen.insert(mod_unit.name).second) continue;
        if(!links.resolve(mod_unit.name)) continue;
        module_names.push_back(mod_unit.name);
    }
    std::sort(module_names.begin(), module_names.end());
    for(auto& name : module_names) {
        result += "- " + make_link_full(name, current_page_path, links) + "\n";
    }
    if(!result.empty()) result += "\n";
    return result;
}

auto render_all_namespaces_index(const extract::ProjectModel& model,
                                 const LinkResolver& links,
                                 std::string_view current_page_path) -> std::string {
    std::string result;
    std::vector<std::string> ns_names;
    for(auto& [ns_name, ns_info] : model.namespaces) {
        if(ns_name.find("(anonymous namespace)") != std::string::npos) continue;
        if(!links.resolve(ns_name)) continue;
        ns_names.push_back(ns_name);
    }
    std::sort(ns_names.begin(), ns_names.end());
    for(auto& name : ns_names) {
        result += "- " + make_link_full(name, current_page_path, links) + "\n";
    }
    if(!result.empty()) result += "\n";
    return result;
}

auto render_all_types_index(const extract::ProjectModel& model,
                            const LinkResolver& links,
                            std::string_view current_page_path) -> std::string {
    std::string result;
    std::vector<std::string> type_names;
    for(auto& [id, sym] : model.symbols) {
        if(sym.qualified_name.find("(anonymous namespace)") != std::string::npos) continue;
        if(!is_type_kind(sym.kind)) continue;
        if(!links.resolve(sym.qualified_name)) continue;
        type_names.push_back(sym.qualified_name);
    }
    std::sort(type_names.begin(), type_names.end());
    for(auto& name : type_names) {
        result += "- " + make_link_full(name, current_page_path, links) + "\n";
    }
    if(!result.empty()) result += "\n";
    return result;
}

auto render_all_files_index(const extract::ProjectModel& model,
                            const config::TaskConfig& config,
                            const LinkResolver& links,
                            std::string_view current_page_path) -> std::string {
    std::string result;
    std::vector<std::pair<std::string, std::string>> file_entries;
    for(auto& [path, file_info] : model.files) {
        if(!links.resolve(path)) continue;
        auto rel = make_source_relative(path, config.project_root);
        file_entries.emplace_back(rel, path);
    }
    std::sort(file_entries.begin(), file_entries.end());
    for(auto& [rel, abs] : file_entries) {
        auto* page_path = links.resolve(abs);
        if(!page_path) continue;
        result += "- [`" + rel + "`](" +
                  make_relative_link_target(current_page_path, *page_path) + ")\n";
    }
    if(!result.empty()) result += "\n";
    return result;
}

auto strip_trailing_carriage_return(std::string_view line) -> std::string_view {
    while(!line.empty() && line.back() == '\r') {
        line.remove_suffix(1);
    }
    return line;
}

auto trim_line(std::string_view line) -> std::string_view {
    line = strip_trailing_carriage_return(line);
    while(!line.empty() && (line.back() == ' ' || line.back() == '\t')) {
        line.remove_suffix(1);
    }
    while(!line.empty() && (line.front() == ' ' || line.front() == '\t')) {
        line.remove_prefix(1);
    }
    return line;
}

auto is_blank_line(std::string_view line) -> bool {
    line = trim_line(line);
    if(line.empty()) {
        return true;
    }
    return std::ranges::all_of(line, [](char ch) {
        return std::isspace(static_cast<unsigned char>(ch)) != 0;
    });
}

}  // namespace

auto build_link_resolver(const PagePlanSet& plan_set) -> LinkResolver {
    LinkResolver resolver;
    for(auto& plan : plan_set.plans) {
        for(auto& key : plan.owner_keys) {
            resolver.name_to_path[key] = plan.relative_path;
        }
        auto colon_pos = plan.page_id.find(':');
        if(colon_pos != std::string::npos) {
            auto entity_name = plan.page_id.substr(colon_pos + 1);
            resolver.name_to_path[entity_name] = plan.relative_path;
        }
    }
    return resolver;
}

auto render_deterministic_block(std::string_view block_name,
                                const PagePlan& plan,
                                const extract::ProjectModel& model,
                                const config::TaskConfig& config,
                                const LinkResolver& links) -> std::string {
    if(block_name == "declaration") return render_declaration_block(plan, model);
    if(block_name == "template_parameters") return render_template_parameters_block(plan, model);
    if(block_name == "base_types") return render_base_types_block(plan, model, links);
    if(block_name == "derived_types") return render_derived_types_block(plan, model, links);
    if(block_name == "public_members") return render_members_block(plan, model, "public");
    if(block_name == "protected_members") return render_members_block(plan, model, "protected");
    if(block_name == "private_members") return render_members_block(plan, model, "private");
    if(block_name == "imports") return render_imports_block(plan, model, links);
    if(block_name == "subnamespaces") return render_subnamespaces_block(plan, model, links);
    if(block_name == "types_index") return render_types_index_block(plan, model, links);
    if(block_name == "functions_index") return render_functions_index_block(plan, model);
    if(block_name == "related_pages") return render_related_pages_block(plan, links);
    if(block_name == "source_locations") return render_source_locations_block(plan, model, config);
    if(block_name == "source") return render_source_locations_block(plan, model, config);
    if(block_name == "includes") return render_includes_block(plan, model, config);
    if(block_name == "declared_symbols") return render_declared_symbols_block(plan, model);
    if(block_name == "defined_symbols") return render_defined_symbols_block(plan, model);
    if(block_name == "module_info") return render_module_info_block(plan, model);
    if(block_name == "all_modules") return render_all_modules_index(model, links, plan.relative_path);
    if(block_name == "all_namespaces") return render_all_namespaces_index(model, links, plan.relative_path);
    if(block_name == "all_types") return render_all_types_index(model, links, plan.relative_path);
    if(block_name == "all_files") return render_all_files_index(model, config, links, plan.relative_path);
    return {};
}

auto assemble_page(const std::string& page_template,
                   const std::string& title,
                   const std::unordered_map<std::string, std::string>& blocks,
                   const std::unordered_map<std::string, std::string>& slots,
                   bool fail_on_empty_section)
    -> std::expected<std::string, RenderError> {

    std::string result;
    result.reserve(page_template.size() + 4096);

    std::size_t pos = 0;
    while(pos < page_template.size()) {
        auto marker_start = page_template.find("{{", pos);
        if(marker_start == std::string::npos) {
            result.append(page_template, pos, page_template.size() - pos);
            break;
        }
        result.append(page_template, pos, marker_start - pos);

        auto marker_end = page_template.find("}}", marker_start);
        if(marker_end == std::string::npos) {
            result.append(page_template, marker_start, page_template.size() - marker_start);
            break;
        }

        auto key = page_template.substr(marker_start + 2, marker_end - marker_start - 2);

        if(key == "title") {
            result.append(title);
        } else if(key.starts_with("block:")) {
            auto block_name = key.substr(6);
            auto it = blocks.find(block_name);
            if(it != blocks.end()) {
                result.append(it->second);
            }
        } else if(key.starts_with("slot:")) {
            auto slot_name = key.substr(5);
            auto it = slots.find(slot_name);
            if(it != slots.end()) {
                result.append(it->second);
            }
        } else {
            result.append("{{");
            result.append(key);
            result.append("}}");
        }

        pos = marker_end + 2;
    }

    std::string cleaned;
    cleaned.reserve(result.size());
    std::istringstream stream(result);
    std::string line;
    std::vector<std::string> lines;
    while(std::getline(stream, line)) {
        lines.push_back(line);
    }

    std::size_t i = 0;
    while(i < lines.size()) {
        auto current_line = trim_line(lines[i]);
        if(current_line.starts_with("## ") || current_line.starts_with("### ")) {
            std::size_t j = i + 1;
            while(j < lines.size() && is_blank_line(lines[j])) {
                ++j;
            }

            if(j >= lines.size() ||
               trim_line(lines[j]).starts_with("# ") ||
               trim_line(lines[j]).starts_with("## ") ||
               trim_line(lines[j]).starts_with("### ")) {
                if(fail_on_empty_section) {
                    return std::unexpected(RenderError{
                        std::format("empty section detected: {}", current_line)
                    });
                }
                i = j;
                continue;
            }
        }

        cleaned += std::string(strip_trailing_carriage_return(lines[i])) + "\n";
        ++i;
    }

    return cleaned;
}

auto validate_output(const std::string& content,
                     const config::ValidationConfig& validation)
    -> std::expected<void, RenderError> {
    if(content.empty()) {
        return std::unexpected(RenderError{.message = "LLM output is empty"});
    }

    if(std::ranges::all_of(content, [](char ch) {
        return std::isspace(static_cast<unsigned char>(ch)) != 0;
    })) {
        return std::unexpected(RenderError{.message = "LLM output contains only whitespace"});
    }

    if(validation.fail_on_h1_in_output) {
        if(content.starts_with("# ")) {
            return std::unexpected(RenderError{
                .message = "LLM output contains H1 heading '# '"});
        }
        if(content.find("\n# ") != std::string::npos) {
            return std::unexpected(RenderError{
                .message = "LLM output contains H1 heading '# '"});
        }
    }

    return {};
}

auto write_page(const GeneratedPage& page, std::string_view output_root)
    -> std::expected<void, RenderError> {
    namespace fs = std::filesystem;

    auto root = fs::path(output_root);
    auto rel = fs::path(page.relative_path);

    if(rel.is_absolute()) {
        return std::unexpected(RenderError{
            .message = std::format("page output path must be relative: {}", page.relative_path)});
    }
    for(const auto& part : rel) {
        if(part == "." || part == "..") {
            return std::unexpected(RenderError{
                .message = std::format("page output path must not contain '.' or '..': {}",
                                       page.relative_path)});
        }
    }

    auto target = (root / rel).lexically_normal();
    auto parent = target.parent_path();

    if(!fs::exists(parent)) {
        std::error_code ec;
        fs::create_directories(parent, ec);
        if(ec) {
            return std::unexpected(RenderError{
                .message = std::format("failed to create directory {}: {}",
                                       parent.generic_string(), ec.message())});
        }
    }

    auto write_result = clore::support::write_utf8_text_file(target, page.content);
    if(!write_result.has_value()) {
        return std::unexpected(RenderError{
            .message = std::move(write_result.error())});
    }

    return {};
}

}  // namespace clore::generate