module;

#include <algorithm>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

export module extract:model;

import :symbol;

export namespace clore::extract {

struct TransparentStringHash {
    using is_transparent = void;

    [[nodiscard]] auto operator()(std::string_view value) const noexcept -> std::size_t {
        return std::hash<std::string_view>{}(value);
    }

    [[nodiscard]] auto operator()(const std::string& value) const noexcept -> std::size_t {
        return (*this)(std::string_view{value});
    }

    [[nodiscard]] auto operator()(const char* value) const noexcept -> std::size_t {
        return (*this)(std::string_view{value});
    }
};

struct SourceLocation {
    std::string file;
    /// Line 0 means the location is unknown; valid source lines start at 1.
    std::uint32_t line = 0;
    std::uint32_t column = 0;

    [[nodiscard]] bool is_known() const noexcept { return line != 0; }
};

struct SourceRange {
    SourceLocation begin;
    SourceLocation end;
};

struct SymbolInfo {
    SymbolID id;
    SymbolKind kind = SymbolKind::Unknown;
    std::string name;
    std::string qualified_name;
    std::string enclosing_namespace;
    std::string signature;

    SourceLocation declaration_location;
    std::optional<SourceLocation> definition_location;

    std::string doc_comment;
    std::string source_snippet;

    std::optional<SymbolID> parent;
    std::string lexical_parent_name;
    SymbolKind lexical_parent_kind = SymbolKind::Unknown;
    std::vector<SymbolID> children;

    std::vector<SymbolID> bases;
    std::vector<SymbolID> derived;

    std::vector<SymbolID> calls;
    std::vector<SymbolID> called_by;

    std::vector<SymbolID> references;
    std::vector<SymbolID> referenced_by;

    std::string access;

    bool is_template = false;
    std::string template_params;
};

struct FileInfo {
    std::string path;
    std::vector<SymbolID> symbols;
    std::vector<std::string> includes;
};

struct NamespaceInfo {
    std::string name;
    std::vector<SymbolID> symbols;
    std::vector<std::string> children;
};

// ── module information ──────────────────────────────────────────────

/// Represents a single C++20 module unit (interface or partition).
struct ModuleUnit {
    std::string name;              ///< Full module name, e.g. "foo" or "foo:bar"
    bool is_interface = false;     ///< true for `export module`, false for `module`
    std::string source_file;       ///< Normalized path to the source file
    std::vector<std::string> imports;  ///< Module imports
    std::vector<SymbolID> symbols;     ///< Symbols declared in this module unit
};

struct ProjectModel {
    std::unordered_map<SymbolID, SymbolInfo> symbols;
    std::unordered_map<std::string, FileInfo> files;
    std::unordered_map<std::string, NamespaceInfo> namespaces;

    std::vector<std::string> file_order;

    /// Module units indexed by normalized source file path.
    std::unordered_map<std::string, ModuleUnit, TransparentStringHash, std::equal_to<>> modules;

    /// Exact qualified-name lookup for generation and evidence building.
    /// A qualified name can map to multiple SymbolIDs when overloads exist.
    std::unordered_map<std::string, std::vector<SymbolID>, TransparentStringHash, std::equal_to<>>
        symbol_ids_by_qualified_name;

    /// Exact module-name lookup for generation and cross-linking.
    std::unordered_map<std::string, std::vector<std::string>, TransparentStringHash, std::equal_to<>>
        module_name_to_sources;

    /// True if the project uses C++20 modules (at least one module declaration found).
    bool uses_modules = false;
};

auto lookup_symbol(const ProjectModel& model, SymbolID id) -> const SymbolInfo*;

auto find_symbol(const ProjectModel& model, std::string_view qualified_name)
    -> const SymbolInfo*;

auto find_symbol(const ProjectModel& model,
                 std::string_view qualified_name,
                 std::string_view signature)
    -> const SymbolInfo*;

auto find_symbols(const ProjectModel& model, std::string_view qualified_name)
    -> std::vector<const SymbolInfo*>;

auto find_module_by_name(const ProjectModel& model, std::string_view module_name)
    -> const ModuleUnit*;

auto find_modules_by_name(const ProjectModel& model, std::string_view module_name)
    -> std::vector<const ModuleUnit*>;

auto find_module_by_source(const ProjectModel& model, std::string_view source_file)
    -> const ModuleUnit*;

}  // namespace clore::extract

namespace clore::extract {

auto lookup_symbol(const ProjectModel& model, SymbolID id) -> const SymbolInfo* {
    auto it = model.symbols.find(id);
    return it != model.symbols.end() ? &it->second : nullptr;
}

auto find_symbols(const ProjectModel& model, std::string_view qualified_name)
    -> std::vector<const SymbolInfo*> {
    std::vector<const SymbolInfo*> matches;
    auto it = model.symbol_ids_by_qualified_name.find(qualified_name);
    if(it == model.symbol_ids_by_qualified_name.end()) {
        return matches;
    }

    matches.reserve(it->second.size());
    for(auto symbol_id : it->second) {
        if(auto* symbol = lookup_symbol(model, symbol_id)) {
            matches.push_back(symbol);
        }
    }

    std::sort(matches.begin(), matches.end(), [](const SymbolInfo* lhs, const SymbolInfo* rhs) {
        if(lhs->signature != rhs->signature) {
            return lhs->signature < rhs->signature;
        }
        if(lhs->declaration_location.file != rhs->declaration_location.file) {
            return lhs->declaration_location.file < rhs->declaration_location.file;
        }
        if(lhs->declaration_location.line != rhs->declaration_location.line) {
            return lhs->declaration_location.line < rhs->declaration_location.line;
        }
        return lhs->id < rhs->id;
    });

    return matches;
}

auto find_symbol(const ProjectModel& model, std::string_view qualified_name)
    -> const SymbolInfo* {
    auto matches = find_symbols(model, qualified_name);
    if(matches.size() != 1) {
        return nullptr;
    }
    return matches.front();
}

auto find_symbol(const ProjectModel& model,
                 std::string_view qualified_name,
                 std::string_view signature)
    -> const SymbolInfo* {
    if(signature.empty()) {
        return find_symbol(model, qualified_name);
    }

    auto matches = find_symbols(model, qualified_name);
    for(auto* symbol : matches) {
        if(symbol->signature == signature) {
            return symbol;
        }
    }
    return nullptr;
}

auto find_modules_by_name(const ProjectModel& model, std::string_view module_name)
    -> std::vector<const ModuleUnit*> {
    std::vector<const ModuleUnit*> modules;
    auto it = model.module_name_to_sources.find(module_name);
    if(it == model.module_name_to_sources.end() || it->second.empty()) {
        return modules;
    }

    modules.reserve(it->second.size());
    for(const auto& source : it->second) {
        if(auto* module = find_module_by_source(model, source)) {
            modules.push_back(module);
        }
    }

    std::sort(modules.begin(), modules.end(), [](const ModuleUnit* lhs, const ModuleUnit* rhs) {
        return lhs->source_file < rhs->source_file;
    });
    return modules;
}

auto find_module_by_name(const ProjectModel& model, std::string_view module_name)
    -> const ModuleUnit* {
    auto modules = find_modules_by_name(model, module_name);
    if(modules.empty()) {
        return nullptr;
    }
    if(modules.size() == 1) {
        return modules.front();
    }

    const ModuleUnit* single_interface = nullptr;
    std::size_t interface_count = 0;
    for(auto* module : modules) {
        if(module->is_interface) {
            single_interface = module;
            ++interface_count;
        }
    }
    if(interface_count == 1) {
        return single_interface;
    }

    return nullptr;
}

auto find_module_by_source(const ProjectModel& model, std::string_view source_file)
    -> const ModuleUnit* {
    auto it = model.modules.find(source_file);
    return it != model.modules.end() ? &it->second : nullptr;
}

}  // namespace clore::extract
