#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace clore::extract {

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
    /// A hash of 0 is the invalid/null sentinel.  Valid IDs are always non-zero
    /// (guaranteed by compute_symbol_id, which rejects Decls with no USR).
    std::uint64_t hash = 0;

    [[nodiscard]] bool is_valid() const noexcept { return hash != 0; }

    bool operator==(const SymbolID&) const = default;
    auto operator<=>(const SymbolID&) const = default;
};

}  // namespace clore::extract

template <>
struct std::hash<clore::extract::SymbolID> {
    std::size_t operator()(const clore::extract::SymbolID& id) const noexcept {
        return std::hash<std::uint64_t>{}(id.hash);
    }
};
