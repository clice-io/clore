module;

#include "kota/codec/json/json.h"

export module agent:tools;

import std;
import extract;
import generate;
import protocol;
import schema;
import support;

export namespace clore::agent {

struct ToolError {
    std::string message;
};

auto extract_string_arg(const kota::codec::json::Value& arguments, std::string_view field_name)
    -> std::expected<std::string, ToolError>;

auto build_tool_definitions()
    -> std::expected<std::vector<clore::net::FunctionToolDefinition>, ToolError>;

auto dispatch_tool_call(std::string_view tool_name,
                        const kota::codec::json::Value& arguments,
                        const extract::ProjectModel& model,
                        std::string_view project_root,
                        std::string_view output_root) -> std::expected<std::string, ToolError>;

}  // namespace clore::agent

namespace clore::agent {

namespace json = kota::codec::json;

namespace {

auto normalize_guide_filename(std::string_view name) -> std::expected<std::string, ToolError> {
    std::string normalized(name);
    if(normalized.empty()) {
        return std::unexpected(ToolError{.message = "guide filename must not be empty"});
    }

    constexpr std::string_view prefix = "guides/";
    if(normalized.starts_with(prefix)) {
        normalized.erase(0, prefix.size());
    }

    if(normalized.empty()) {
        return std::unexpected(ToolError{.message = "guide filename must not be empty"});
    }
    if(normalized.find('/') != std::string::npos || normalized.find('\\') != std::string::npos) {
        return std::unexpected(
            ToolError{.message = "guide filename must be a markdown file directly under guides/"});
    }
    if(normalized == "." || normalized == "..") {
        return std::unexpected(ToolError{.message = "invalid guide filename"});
    }
    if(!normalized.ends_with(".md")) {
        normalized += ".md";
    }
    if(!normalized.ends_with(".md")) {
        return std::unexpected(ToolError{.message = "guide filename must end with .md"});
    }

    return normalized;
}

auto write_guide(std::string_view output_root,
                 std::string_view name,
                 std::string_view title,
                 std::string_view content) -> std::expected<std::string, ToolError> {
    namespace fs = std::filesystem;
    auto normalized_name = normalize_guide_filename(name);
    if(!normalized_name.has_value()) {
        return std::unexpected(std::move(normalized_name.error()));
    }

    auto guides_dir = fs::path(std::string(output_root)) / "guides";
    std::error_code ec;
    fs::create_directories(guides_dir, ec);
    if(ec) {
        return std::unexpected(ToolError{
            .message = std::format("failed to create guides directory: {}", ec.message())});
    }

    auto file_path = guides_dir / *normalized_name;

    clore::generate::MarkdownDocument doc;
    doc.frontmatter = clore::generate::Frontmatter{
        .title = std::string(title),
        .description = std::format("Guide: {}", title),
    };
    doc.children.push_back(clore::generate::make_raw_markdown(std::string(content)));
    auto rendered = clore::generate::render_markdown(doc);

    auto write_result = clore::support::write_utf8_text_file(file_path, rendered);
    if(!write_result.has_value()) {
        return std::unexpected(ToolError{.message = std::format("failed to write guide '{}': {}",
                                                                *normalized_name,
                                                                write_result.error())});
    }

    logging::info("  written guide: guides/{}", *normalized_name);
    return std::move(*normalized_name);
}

auto read_guide(std::string_view output_root, std::string_view name)
    -> std::expected<std::string, ToolError> {
    namespace fs = std::filesystem;

    auto normalized_name = normalize_guide_filename(name);
    if(!normalized_name.has_value()) {
        return std::unexpected(std::move(normalized_name.error()));
    }

    auto guides_dir = fs::path(std::string(output_root)) / "guides";
    auto file_path = guides_dir / *normalized_name;

    std::error_code ec;
    auto canonical_guides = fs::weakly_canonical(guides_dir, ec);
    if(ec) {
        return std::unexpected(ToolError{
            .message = std::format("failed to resolve guides directory: {}", ec.message())});
    }

    auto canonical_file = fs::weakly_canonical(file_path, ec);
    if(ec) {
        return std::unexpected(
            ToolError{.message = std::format("guide '{}' does not exist", *normalized_name)});
    }

    auto relative = canonical_file.lexically_relative(canonical_guides);
    auto relative_string = relative.generic_string();
    if(relative.empty() || relative_string.empty() || relative_string == "." ||
       relative_string.starts_with("..")) {
        return std::unexpected(ToolError{.message = "guide path escapes the guides directory"});
    }
    if(canonical_file.extension() != ".md") {
        return std::unexpected(
            ToolError{.message = "only markdown files under guides/ may be read"});
    }

    auto read_result = clore::support::read_utf8_text_file(canonical_file);
    if(!read_result.has_value()) {
        return std::unexpected(ToolError{.message = std::format("failed to read guide '{}': {}",
                                                                *normalized_name,
                                                                read_result.error())});
    }

    return std::format("Guide: guides/{}\n\n{}", *normalized_name, *read_result);
}

auto format_symbol_brief(const extract::SymbolInfo& sym, std::string_view project_root)
    -> std::string {
    auto rel_file = clore::generate::make_source_relative(sym.declaration_location.file,
                                                          std::string(project_root));
    std::string result = std::format("- {} `{}` ({}) at {}:{}",
                                     extract::symbol_kind_name(sym.kind),
                                     sym.qualified_name,
                                     sym.signature.empty() ? "-" : sym.signature,
                                     rel_file,
                                     sym.declaration_location.line);
    if(!sym.doc_comment.empty()) {
        result += std::format(
            "\n  doc: {}",
            sym.doc_comment.substr(0, std::min<std::size_t>(sym.doc_comment.size(), 200)));
    }
    return result;
}

auto format_symbol_detail(const extract::SymbolInfo& sym,
                          const extract::ProjectModel& model,
                          std::string_view project_root) -> std::string {
    std::string result;
    auto append_symbol_list = [&](std::string_view heading,
                                  const std::vector<extract::SymbolID>& ids,
                                  std::size_t limit = 0) -> void {
        if(ids.empty()) {
            return;
        }
        auto count = ids.size();
        if(limit > 0) {
            count = std::min<std::size_t>(count, limit);
            result += std::format("{} ({}):\n", heading, count);
        } else {
            result += std::format("{}:\n", heading);
        }

        std::size_t emitted = 0;
        for(auto symbol_id: ids) {
            if(limit > 0 && emitted >= limit) {
                break;
            }
            if(auto* related = extract::lookup_symbol(model, symbol_id)) {
                result += std::format("  - {}\n", related->qualified_name);
                emitted++;
            }
        }
    };

    auto rel_file = clore::generate::make_source_relative(sym.declaration_location.file,
                                                          std::string(project_root));
    result += std::format("kind: {}\n", extract::symbol_kind_name(sym.kind));
    result += std::format("qualified_name: {}\n", sym.qualified_name);
    if(!sym.signature.empty()) {
        result += std::format("signature: {}\n", sym.signature);
    }
    result += std::format("declaration: {}:{}\n", rel_file, sym.declaration_location.line);
    if(sym.definition_location.has_value() && sym.definition_location->is_known()) {
        auto def_file = clore::generate::make_source_relative(sym.definition_location->file,
                                                              std::string(project_root));
        result += std::format("definition: {}:{}\n", def_file, sym.definition_location->line);
    }
    if(!sym.enclosing_namespace.empty()) {
        result += std::format("namespace: {}\n", sym.enclosing_namespace);
    }
    if(!sym.access.empty()) {
        result += std::format("access: {}\n", sym.access);
    }
    if(sym.is_template) {
        result += std::format("template_params: {}\n", sym.template_params);
    }
    if(sym.parent.has_value()) {
        if(auto* parent_sym = extract::lookup_symbol(model, *sym.parent)) {
            result += std::format("parent: {}\n", parent_sym->qualified_name);
        }
    }
    if(!sym.lexical_parent_name.empty()) {
        result += std::format("lexical_parent: {} ({}):\n",
                              sym.lexical_parent_name,
                              extract::symbol_kind_name(sym.lexical_parent_kind));
    }
    if(!sym.doc_comment.empty()) {
        result += std::format("doc_comment: {}\n", sym.doc_comment);
    }
    std::string source_snippet = sym.source_snippet;
    if(source_snippet.empty() && sym.source_snippet_length > 0) {
        auto resolved = sym;
        if(extract::resolve_source_snippet(resolved)) {
            source_snippet = std::move(resolved.source_snippet);
        }
    }
    if(!source_snippet.empty()) {
        result += std::format("source:\n```cpp\n{}\n```\n", source_snippet);
    }

    append_symbol_list("bases", sym.bases);
    append_symbol_list("derived", sym.derived);
    append_symbol_list("calls", sym.calls, 200);
    append_symbol_list("called_by", sym.called_by, 200);
    append_symbol_list("references", sym.references, 50);
    append_symbol_list("referenced_by", sym.referenced_by, 50);
    if(!sym.children.empty()) {
        result += "members:\n";
        for(auto& child_id: sym.children) {
            if(auto* child = extract::lookup_symbol(model, child_id)) {
                auto child_rel =
                    clore::generate::make_source_relative(child->declaration_location.file,
                                                          std::string(project_root));
                result += std::format("  - {} `{}` ({}) at {}:{}\n",
                                      extract::symbol_kind_name(child->kind),
                                      child->name,
                                      child->signature.empty() ? "-" : child->signature,
                                      child_rel,
                                      child->declaration_location.line);
            }
        }
    }

    return result;
}

auto append_symbol_briefs(std::string& out,
                          const std::vector<extract::SymbolID>& ids,
                          const extract::ProjectModel& model,
                          std::string_view project_root,
                          std::string_view indent = "",
                          std::size_t limit = 0) -> void {
    std::size_t emitted = 0;
    for(auto sym_id: ids) {
        if(limit > 0 && emitted >= limit) {
            break;
        }
        if(auto* sym = extract::lookup_symbol(model, sym_id)) {
            out += indent;
            out += format_symbol_brief(*sym, project_root);
            out.push_back('\n');
            ++emitted;
        }
    }
}

auto append_symbol_briefs(std::string& out,
                          const std::vector<const extract::SymbolInfo*>& symbols,
                          std::string_view project_root,
                          std::size_t limit = 0,
                          std::string_view indent = "") -> void {
    std::size_t max_index = limit > 0 ? std::min(symbols.size(), limit) : symbols.size();
    for(std::size_t i = 0; i < max_index; ++i) {
        if(auto* sym = symbols[i]; sym != nullptr) {
            out += indent;
            out += format_symbol_brief(*sym, project_root);
            out.push_back('\n');
        }
    }
}

auto append_symbol_section(std::string& out,
                           std::string_view heading,
                           const std::vector<extract::SymbolID>& ids,
                           const extract::ProjectModel& model,
                           std::string_view project_root,
                           std::size_t limit = 0) -> void {
    if(ids.empty()) {
        return;
    }
    out += '\n';
    out += heading;
    out += ":\n";
    append_symbol_briefs(out, ids, model, project_root, "", limit);
}

auto sort_symbol_matches(std::vector<const extract::SymbolInfo*>& matches) -> void {
    std::ranges::sort(matches, [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
        return std::tie(lhs->qualified_name,
                        lhs->signature,
                        lhs->declaration_location.file,
                        lhs->declaration_location.line,
                        lhs->declaration_location.column,
                        lhs->id.hash,
                        lhs->id.signature) < std::tie(rhs->qualified_name,
                                                      rhs->signature,
                                                      rhs->declaration_location.file,
                                                      rhs->declaration_location.line,
                                                      rhs->declaration_location.column,
                                                      rhs->id.hash,
                                                      rhs->id.signature);
    });
}

auto format_overload_candidates(std::string_view qualified_name,
                                std::vector<const extract::SymbolInfo*> matches,
                                std::string_view project_root) -> std::string {
    sort_symbol_matches(matches);

    std::string result =
        std::format("Symbol '{}' is overloaded; provide the exact signature.\nCandidates:\n",
                    qualified_name);
    append_symbol_briefs(result, matches, project_root);
    return result;
}

auto tool_list_namespaces(const extract::ProjectModel& model) -> std::string {
    if(model.namespaces.empty()) {
        return "No namespaces found.";
    }
    std::vector<std::string> names;
    names.reserve(model.namespaces.size());
    for(auto& [name, _]: model.namespaces) {
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());
    std::string result;
    for(auto& name: names) {
        auto& ns = model.namespaces.at(name);
        result += std::format("- {} ({} symbols)\n", name, ns.symbols.size());
    }
    return result;
}

auto tool_list_modules(const extract::ProjectModel& model) -> std::string {
    if(model.module_name_to_sources.empty()) {
        return "No C++20 modules found.";
    }
    std::vector<std::string> names;
    names.reserve(model.module_name_to_sources.size());
    for(auto& [name, _]: model.module_name_to_sources) {
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());
    std::string result;
    for(auto& name: names) {
        auto modules = extract::find_modules_by_name(model, name);
        std::size_t sym_count = 0;
        for(auto* mod: modules) {
            sym_count += mod->symbols.size();
        }
        result += std::format("- {} ({} units, {} symbols)\n", name, modules.size(), sym_count);
    }
    return result;
}

auto tool_list_files(const extract::ProjectModel& model, std::string_view project_root)
    -> std::string {
    if(model.file_order.empty()) {
        return "No files found.";
    }
    std::string result;
    for(auto& path: model.file_order) {
        auto rel = clore::generate::make_source_relative(path, std::string(project_root));
        auto& file = model.files.at(path);
        result += std::format("- {} ({} symbols)\n", rel, file.symbols.size());
    }
    return result;
}

auto tool_get_namespace(const extract::ProjectModel& model,
                        std::string_view name,
                        std::string_view project_root) -> std::string {
    auto it = model.namespaces.find(std::string(name));
    if(it == model.namespaces.end()) {
        return std::format("Namespace '{}' not found.", name);
    }
    auto& ns = it->second;
    std::string result = std::format("Namespace: {}\nSymbols ({}):\n", ns.name, ns.symbols.size());
    append_symbol_briefs(result, ns.symbols, model, project_root);
    if(!ns.children.empty()) {
        result += "Child namespaces:\n";
        for(auto& child: ns.children) {
            result += std::format("  - {}\n", child);
        }
    }
    return result;
}

auto tool_get_module(const extract::ProjectModel& model,
                     std::string_view name,
                     std::string_view project_root) -> std::string {
    auto modules = extract::find_modules_by_name(model, name);
    if(modules.empty()) {
        return std::format("Module '{}' not found.", name);
    }
    std::string result;
    for(auto* mod: modules) {
        auto rel =
            clore::generate::make_source_relative(mod->source_file, std::string(project_root));
        result += std::format("Module unit: {} ({})\n", mod->name, rel);
        result += std::format("  interface: {}\n", mod->is_interface ? "yes" : "no");
        if(!mod->imports.empty()) {
            result += "  imports:\n";
            for(auto& imp: mod->imports) {
                result += std::format("    - {}\n", imp);
            }
        }
        result += std::format("  symbols ({}):\n", mod->symbols.size());
        append_symbol_briefs(result, mod->symbols, model, project_root, "    ");
    }
    return result;
}

auto tool_get_symbol(const extract::ProjectModel& model,
                     std::string_view qualified_name,
                     std::string_view project_root) -> std::string {
    auto matches = extract::find_symbols(model, qualified_name);
    if(matches.empty()) {
        return std::format("Symbol '{}' not found.", qualified_name);
    }
    std::string result;
    for(auto* sym: matches) {
        result += format_symbol_detail(*sym, model, project_root);
        result += "\n---\n\n";
    }
    return result;
}

auto tool_search_symbols(const extract::ProjectModel& model,
                         std::string_view pattern,
                         std::string_view project_root) -> std::string {
    std::vector<const extract::SymbolInfo*> matches;
    for(auto& [_, sym]: model.symbols) {
        if(sym.qualified_name.find(pattern) != std::string::npos ||
           sym.name.find(pattern) != std::string::npos) {
            matches.push_back(&sym);
        }
    }
    std::sort(matches.begin(),
              matches.end(),
              [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  return lhs->qualified_name < rhs->qualified_name;
              });

    if(matches.empty()) {
        return std::format("No symbols matching '{}'.", pattern);
    }
    constexpr std::size_t kMaxResults = 50;
    std::string result = std::format("Found {} symbols matching '{}'{}:\n",
                                     matches.size(),
                                     pattern,
                                     matches.size() > kMaxResults ? " (showing first 50)" : "");
    append_symbol_briefs(result, matches, project_root, kMaxResults);
    return result;
}

auto tool_get_file_symbols(const extract::ProjectModel& model,
                           std::string_view file_path,
                           std::string_view project_root) -> std::string {
    for(auto& [path, file]: model.files) {
        auto rel = clore::generate::make_source_relative(path, std::string(project_root));
        if(path == file_path || rel == file_path) {
            std::string result = std::format("File: {}\nSymbols ({}):\n", rel, file.symbols.size());
            append_symbol_briefs(result, file.symbols, model, project_root);
            if(!file.includes.empty()) {
                result += "Includes:\n";
                for(auto& inc: file.includes) {
                    result += std::format("  - {}\n", inc);
                }
            }
            return result;
        }
    }
    return std::format("File '{}' not found.", file_path);
}

auto tool_get_dependencies(const extract::ProjectModel& model,
                           std::string_view qualified_name,
                           std::string_view signature,
                           std::string_view project_root) -> std::string {
    auto matches = extract::find_symbols(model, qualified_name);
    if(matches.empty()) {
        return std::format("Symbol '{}' not found.", qualified_name);
    }
    sort_symbol_matches(matches);

    const extract::SymbolInfo* sym = nullptr;
    if(signature.empty()) {
        if(matches.size() != 1) {
            return format_overload_candidates(qualified_name, std::move(matches), project_root);
        }
        sym = matches.front();
    } else {
        auto match_it =
            std::ranges::find_if(matches, [signature](const extract::SymbolInfo* candidate) {
                return candidate->signature == signature;
            });
        if(match_it == matches.end()) {
            std::string result = std::format("Symbol '{}' with signature '{}' not found.",
                                             qualified_name,
                                             signature);
            result += "\nCandidates:\n";
            append_symbol_briefs(result, matches, project_root);
            return result;
        }
        sym = *match_it;
    }

    std::string result = std::format("Dependencies of `{}`", sym->qualified_name);
    if(!sym->signature.empty()) {
        result += std::format(" ({})", sym->signature);
    }
    result += ":\n";

    append_symbol_section(result, "Calls", sym->calls, model, project_root, 30);
    append_symbol_section(result, "Called by", sym->called_by, model, project_root, 30);
    append_symbol_section(result, "References", sym->references, model, project_root, 30);
    append_symbol_section(result, "Referenced by", sym->referenced_by, model, project_root, 30);
    append_symbol_section(result, "Base types", sym->bases, model, project_root);
    return result;
}

auto tool_project_overview(const extract::ProjectModel& model) -> std::string {
    std::string result;
    result += std::format("Total symbols: {}\n", model.symbols.size());
    result += std::format("Total files: {}\n", model.files.size());
    result += std::format("Total namespaces: {}\n", model.namespaces.size());
    result += std::format("Uses C++20 modules: {}\n", model.uses_modules ? "yes" : "no");
    if(model.uses_modules) {
        result += std::format("Module names: {}\n", model.module_name_to_sources.size());
    }

    std::unordered_map<extract::SymbolKind, std::size_t> kind_counts;
    for(auto& [_, sym]: model.symbols) {
        kind_counts[sym.kind]++;
    }
    result += "\nSymbol breakdown:\n";
    for(auto& [kind, count]: kind_counts) {
        result += std::format("  {}: {}\n", extract::symbol_kind_name(kind), count);
    }
    return result;
}

struct EmptyArgs {};

struct NameArgs {
    std::string name;
};

struct SymbolQueryArgs {
    std::string name;
    std::optional<std::string> signature;
};

struct GuideArgs {
    std::string name;
    std::string title;
    std::string content;
};

struct ToolContext {
    const extract::ProjectModel& model;
    std::string_view project_root;
    std::string_view output_root;
};

struct ToolResultCache {
    std::shared_mutex mutex;
    std::unordered_map<std::string, std::string> result_by_key;
};

auto tool_result_cache() -> ToolResultCache& {
    static ToolResultCache cache;
    return cache;
}

struct ToolSpec {
    std::string_view name;
    bool cacheable = true;
    auto (*build_definition)() -> std::expected<clore::net::FunctionToolDefinition, ToolError>;
    auto (*dispatch)(const json::Value& arguments, const ToolContext& context)
        -> std::expected<std::string, ToolError>;
};

template <typename ToolImpl>
auto build_reflected_tool_definition()
    -> std::expected<clore::net::FunctionToolDefinition, ToolError> {
    auto definition = clore::net::schema::function_tool<typename ToolImpl::Args>(
        std::string(ToolImpl::name),
        std::string(ToolImpl::description));
    if(!definition.has_value()) {
        return std::unexpected(
            ToolError{.message = std::format("failed to build tool definition '{}': {}",
                                             ToolImpl::name,
                                             definition.error().message)});
    }
    return std::move(*definition);
}

template <typename ToolImpl>
auto dispatch_reflected_tool(const json::Value& arguments, const ToolContext& context)
    -> std::expected<std::string, ToolError> {
    auto encoded = arguments.to_json_string();
    if(!encoded.has_value()) {
        return std::unexpected(ToolError{
            .message = std::format("failed to serialize arguments for '{}': {}",
                                   ToolImpl::name,
                                   json::error_message(json::make_write_error(encoded.error()))),
        });
    }

    auto parsed = json::from_json<typename ToolImpl::Args>(*encoded);
    if(!parsed.has_value()) {
        return std::unexpected(
            ToolError{.message = std::format("failed to parse arguments for '{}': {}",
                                             ToolImpl::name,
                                             parsed.error().to_string())});
    }

    return ToolImpl::run(*parsed, context);
}

template <typename ToolImpl>
constexpr auto make_tool_spec() -> ToolSpec {
    return ToolSpec{
        .name = ToolImpl::name,
        .cacheable = ToolImpl::cacheable,
        .build_definition = &build_reflected_tool_definition<ToolImpl>,
        .dispatch = &dispatch_reflected_tool<ToolImpl>,
    };
}

struct ProjectOverviewTool {
    using Args = EmptyArgs;
    constexpr static std::string_view name = "project_overview";
    constexpr static bool cacheable = true;
    constexpr static std::string_view description =
        "Get a high-level overview of the project: symbol counts, file counts, " "namespace counts, module usage, and symbol kind breakdown.";

    static auto run(const Args&, const ToolContext& context)
        -> std::expected<std::string, ToolError> {
        return tool_project_overview(context.model);
    }
};

struct ListNamespacesTool {
    using Args = EmptyArgs;
    constexpr static std::string_view name = "list_namespaces";
    constexpr static bool cacheable = true;
    constexpr static std::string_view description =
        "List all namespaces in the project with their symbol counts.";

    static auto run(const Args&, const ToolContext& context)
        -> std::expected<std::string, ToolError> {
        return tool_list_namespaces(context.model);
    }
};

struct ListModulesTool {
    using Args = EmptyArgs;
    constexpr static std::string_view name = "list_modules";
    constexpr static bool cacheable = true;
    constexpr static std::string_view description =
        "List all C++20 modules in the project with unit and symbol counts.";

    static auto run(const Args&, const ToolContext& context)
        -> std::expected<std::string, ToolError> {
        return tool_list_modules(context.model);
    }
};

struct ListFilesTool {
    using Args = EmptyArgs;
    constexpr static std::string_view name = "list_files";
    constexpr static bool cacheable = true;
    constexpr static std::string_view description =
        "List all source files in the project with their symbol counts.";

    static auto run(const Args&, const ToolContext& context)
        -> std::expected<std::string, ToolError> {
        return tool_list_files(context.model, context.project_root);
    }
};

struct GetNamespaceTool {
    using Args = NameArgs;
    constexpr static std::string_view name = "get_namespace";
    constexpr static bool cacheable = true;
    constexpr static std::string_view description =
        "Get detailed information about a namespace: its symbols and child " "namespaces.";

    static auto run(const Args& args, const ToolContext& context)
        -> std::expected<std::string, ToolError> {
        return tool_get_namespace(context.model, args.name, context.project_root);
    }
};

struct GetModuleTool {
    using Args = NameArgs;
    constexpr static std::string_view name = "get_module";
    constexpr static bool cacheable = true;
    constexpr static std::string_view description =
        "Get detailed information about a C++20 module: its units, imports, " "and exported symbols.";

    static auto run(const Args& args, const ToolContext& context)
        -> std::expected<std::string, ToolError> {
        return tool_get_module(context.model, args.name, context.project_root);
    }
};

struct GetSymbolTool {
    using Args = NameArgs;
    constexpr static std::string_view name = "get_symbol";
    constexpr static bool cacheable = true;
    constexpr static std::string_view description =
      "Get complete information about a symbol: kind, signature, location, "
      "doc comment, source snippet, relationships (bases, derived, calls, "
      "called_by, members).";

    static auto run(const Args& args, const ToolContext& context)
        -> std::expected<std::string, ToolError> {
        return tool_get_symbol(context.model, args.name, context.project_root);
    }
};

struct SearchSymbolsTool {
    using Args = NameArgs;
    constexpr static std::string_view name = "search_symbols";
    constexpr static bool cacheable = true;
    constexpr static std::string_view description =
        "Search for symbols by substring matching against qualified names. " "Returns up to 50 matching symbols.";

    static auto run(const Args& args, const ToolContext& context)
        -> std::expected<std::string, ToolError> {
        return tool_search_symbols(context.model, args.name, context.project_root);
    }
};

struct GetFileSymbolsTool {
    using Args = NameArgs;
    constexpr static std::string_view name = "get_file_symbols";
    constexpr static bool cacheable = true;
    constexpr static std::string_view description =
        "Get all symbols declared in a specific source file, plus its include " "dependencies.";

    static auto run(const Args& args, const ToolContext& context)
        -> std::expected<std::string, ToolError> {
        return tool_get_file_symbols(context.model, args.name, context.project_root);
    }
};

struct GetDependenciesTool {
    using Args = SymbolQueryArgs;
    constexpr static std::string_view name = "get_dependencies";
    constexpr static bool cacheable = true;
    constexpr static std::string_view description =
        "Get the dependency graph of a symbol: what it calls, references, and "
        "inherits from. For overloaded functions or methods, provide the "
        "optional signature field to select the exact overload.";

    static auto run(const Args& args, const ToolContext& context)
        -> std::expected<std::string, ToolError> {
        return tool_get_dependencies(context.model,
                                     args.name,
                                     args.signature.value_or(""),
                                     context.project_root);
    }
};

struct CreateGuideTool {
    using Args = GuideArgs;
    constexpr static std::string_view name = "create_guide";
    constexpr static bool cacheable = false;
    constexpr static std::string_view description =
      "Create a markdown guide document. The guide will be written to the "
      "output directory under guides/. Use this after exploring the codebase "
      "to produce architectural guides, module overviews, getting-started "
      "guides, or design documents.";

    static auto run(const Args& args, const ToolContext& context)
        -> std::expected<std::string, ToolError> {
        auto write_result = write_guide(context.output_root, args.name, args.title, args.content);
        if(!write_result.has_value()) {
            return std::unexpected(std::move(write_result.error()));
        }
        return std::format("Guide '{}' created successfully at guides/{}",
                           args.title,
                           *write_result);
    }
};

struct ReadGuideTool {
    using Args = NameArgs;
    constexpr static std::string_view name = "read_guide";
    constexpr static bool cacheable = false;
    constexpr static std::string_view description =
        "Read an existing markdown guide file under guides/. Only direct .md files in that " "directory are allowed. Pass the filename, for example 'architecture-overview.md'.";

    static auto run(const Args& args, const ToolContext& context)
        -> std::expected<std::string, ToolError> {
        return read_guide(context.output_root, args.name);
    }
};

auto tool_registry() -> const std::array<ToolSpec, 12>& {
    const static auto registry = std::array{
        make_tool_spec<ProjectOverviewTool>(),
        make_tool_spec<ListNamespacesTool>(),
        make_tool_spec<ListModulesTool>(),
        make_tool_spec<ListFilesTool>(),
        make_tool_spec<GetNamespaceTool>(),
        make_tool_spec<GetModuleTool>(),
        make_tool_spec<GetSymbolTool>(),
        make_tool_spec<SearchSymbolsTool>(),
        make_tool_spec<GetFileSymbolsTool>(),
        make_tool_spec<GetDependenciesTool>(),
        make_tool_spec<CreateGuideTool>(),
        make_tool_spec<ReadGuideTool>(),
    };
    return registry;
}

}  // namespace

auto extract_string_arg(const json::Value& arguments, std::string_view field_name)
    -> std::expected<std::string, ToolError> {
    if(!arguments.is_object()) {
        return std::unexpected(ToolError{.message = "arguments is not an object"});
    }
    auto object = arguments.get_object();
    if(!object.has_value()) {
        return std::unexpected(ToolError{.message = "failed to read arguments object"});
    }
    for(auto entry: *object) {
        if(entry.key == field_name) {
            auto str = entry.value.get_string();
            if(str.has_value()) {
                return std::string(*str);
            }
            return std::unexpected(
                ToolError{.message = std::format("field '{}' is not a string", field_name)});
        }
    }
    return std::unexpected(ToolError{.message = std::format("missing field '{}'", field_name)});
}

auto build_tool_definitions()
    -> std::expected<std::vector<clore::net::FunctionToolDefinition>, ToolError> {
    std::vector<clore::net::FunctionToolDefinition> tools;
    tools.reserve(tool_registry().size());
    for(const auto& tool: tool_registry()) {
        auto definition = tool.build_definition();
        if(!definition.has_value()) {
            return std::unexpected(std::move(definition.error()));
        }
        tools.push_back(std::move(*definition));
    }

    return tools;
}

auto dispatch_tool_call(std::string_view tool_name,
                        const json::Value& arguments,
                        const extract::ProjectModel& model,
                        std::string_view project_root,
                        std::string_view output_root) -> std::expected<std::string, ToolError> {
    auto encoded_arguments = arguments.to_json_string();
    if(!encoded_arguments.has_value()) {
        return std::unexpected(ToolError{
            .message =
                std::format("failed to serialize arguments for '{}': {}",
                            tool_name,
                            json::error_message(json::make_write_error(encoded_arguments.error()))),
        });
    }

    auto cache_key = std::format("{}:{}", tool_name, *encoded_arguments);
    auto& cache = tool_result_cache();
    {
        std::shared_lock lock(cache.mutex);
        auto cached_it = cache.result_by_key.find(cache_key);
        if(cached_it != cache.result_by_key.end()) {
            return cached_it->second;
        }
    }

    auto context = ToolContext{
        .model = model,
        .project_root = project_root,
        .output_root = output_root,
    };

    for(const auto& tool: tool_registry()) {
        if(tool.name == tool_name) {
            auto dispatch_result = tool.dispatch(arguments, context);
            if(tool.cacheable && dispatch_result.has_value()) {
                std::unique_lock lock(cache.mutex);
                cache.result_by_key.insert_or_assign(cache_key, *dispatch_result);
            }
            return dispatch_result;
        }
    }

    return std::unexpected(ToolError{.message = std::format("unknown tool '{}'", tool_name)});
}

}  // namespace clore::agent
