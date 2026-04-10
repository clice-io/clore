#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "extract/symbol.h"

namespace clore::extract {

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
    /// Unknown is the only kind that is never emitted for valid symbols;
    /// it indicates the Decl passed classify_decl but could not be mapped.
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

    /// Symbols this symbol calls (direct callees).
    std::vector<SymbolID> calls;
    /// Symbols that call this symbol (direct callers).  Populated as a reverse
    /// edge during model assembly — not filled by the AST visitor.
    std::vector<SymbolID> called_by;

    /// Symbols this symbol references (type uses, member accesses, etc.)
    /// excluding call targets which go into `calls`.
    std::vector<SymbolID> references;
    /// Symbols that reference this symbol.  Reverse edge, populated during
    /// model assembly.
    std::vector<SymbolID> referenced_by;

    std::string access;

    /// Explicitly false until the symbol is confirmed to be a template.
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

struct ProjectModel {
    std::unordered_map<SymbolID, SymbolInfo> symbols;
    std::unordered_map<std::string, FileInfo> files;
    std::unordered_map<std::string, NamespaceInfo> namespaces;

    std::vector<std::string> file_order;
};

}  // namespace clore::extract
