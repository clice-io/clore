#include "extract/symbol.h"

namespace clore::extract {

auto symbol_kind_name(SymbolKind kind) -> std::string_view {
    switch(kind) {
        case SymbolKind::Namespace: return "namespace";
        case SymbolKind::Class: return "class";
        case SymbolKind::Struct: return "struct";
        case SymbolKind::Union: return "union";
        case SymbolKind::Enum: return "enum";
        case SymbolKind::EnumMember: return "enum_member";
        case SymbolKind::Function: return "function";
        case SymbolKind::Method: return "method";
        case SymbolKind::Variable: return "variable";
        case SymbolKind::Field: return "field";
        case SymbolKind::TypeAlias: return "type_alias";
        case SymbolKind::Macro: return "macro";
        case SymbolKind::Template: return "template";
        case SymbolKind::Concept: return "concept";
        case SymbolKind::Unknown: return "unknown";
    }
    return "unknown";
}

}  // namespace clore::extract
