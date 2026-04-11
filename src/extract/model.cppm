module;

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

export module clore.extract:model;

import :symbol;

export namespace clore::extract {

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
    std::string signature;

    SourceLocation declaration_location;
    std::optional<SourceLocation> definition_location;

    std::string doc_comment;
    std::string source_snippet;

    std::optional<SymbolID> parent;
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
    std::unordered_map<std::string, ModuleUnit> modules;

    /// True if the project uses C++20 modules (at least one module declaration found).
    bool uses_modules = false;
};

}  // namespace clore::extract
