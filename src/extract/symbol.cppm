module;

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

export module clore.extract:symbol;

export namespace clore::extract {

enum class SymbolKind : std::uint8_t {
    Namespace,
    Class,
    Struct,
    Union,
    Enum,
    EnumMember,
    Function,
    Method,
    Variable,
    Field,
    TypeAlias,
    Macro,
    Template,
    Concept,
    Unknown,
};

auto symbol_kind_name(SymbolKind kind) -> std::string_view;

struct SymbolID {
    /// A hash of 0 is the invalid/null sentinel.  Valid IDs are always non-zero.
    std::uint64_t hash = 0;

    [[nodiscard]] bool is_valid() const noexcept { return hash != 0; }

    bool operator==(const SymbolID&) const = default;
    auto operator<=>(const SymbolID&) const = default;
};

}  // namespace clore::extract

export template <>
struct std::hash<clore::extract::SymbolID> {
    std::size_t operator()(const clore::extract::SymbolID& id) const noexcept {
        return std::hash<std::uint64_t>{}(id.hash);
    }
};

// ── implementation ──────────────────────────────────────────────────

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
