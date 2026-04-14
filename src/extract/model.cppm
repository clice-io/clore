module;

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
    std::unordered_map<std::string, SymbolID, TransparentStringHash, std::equal_to<>>
        symbol_ids_by_qualified_name;

    /// Exact module-name lookup for generation and cross-linking.
    std::unordered_map<std::string, std::string, TransparentStringHash, std::equal_to<>>
        module_name_to_source;

    /// True if the project uses C++20 modules (at least one module declaration found).
    bool uses_modules = false;
};

auto lookup_symbol(const ProjectModel& model, SymbolID id) -> const SymbolInfo*;

auto find_symbol(const ProjectModel& model, std::string_view qualified_name)
    -> const SymbolInfo*;

auto find_module_by_name(const ProjectModel& model, std::string_view module_name)
    -> const ModuleUnit*;

auto find_module_by_source(const ProjectModel& model, std::string_view source_file)
    -> const ModuleUnit*;

}  // namespace clore::extract

namespace clore::extract {

auto lookup_symbol(const ProjectModel& model, SymbolID id) -> const SymbolInfo* {
    auto it = model.symbols.find(id);
    return it != model.symbols.end() ? &it->second : nullptr;
}

auto find_symbol(const ProjectModel& model, std::string_view qualified_name)
    -> const SymbolInfo* {
    auto it = model.symbol_ids_by_qualified_name.find(qualified_name);
    if(it == model.symbol_ids_by_qualified_name.end()) {
        return nullptr;
    }
    return lookup_symbol(model, it->second);
}

auto find_module_by_name(const ProjectModel& model, std::string_view module_name)
    -> const ModuleUnit* {
    auto it = model.module_name_to_source.find(module_name);
    if(it == model.module_name_to_source.end()) {
        return nullptr;
    }
    return find_module_by_source(model, it->second);
}

auto find_module_by_source(const ProjectModel& model, std::string_view source_file)
    -> const ModuleUnit* {
    auto it = model.modules.find(source_file);
    return it != model.modules.end() ? &it->second : nullptr;
}

}  // namespace clore::extract
