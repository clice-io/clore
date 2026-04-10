#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <vector>

#include "extract/compdb.h"
#include "extract/model.h"

namespace clore::extract {

struct ASTError {
    std::string message;
};

struct ExtractedRelation {
    SymbolID from;
    SymbolID to;
    bool is_call;  ///< true = call edge, false = reference edge
};

struct ASTResult {
    std::vector<SymbolInfo> symbols;
    std::vector<ExtractedRelation> relations;
};

/// Extract all named symbols and their call/reference relations from `entry`,
/// capturing at most `max_snippet_bytes` of raw source text per symbol.
auto extract_symbols(const CompileEntry& entry, std::uint32_t max_snippet_bytes)
    -> std::expected<ASTResult, ASTError>;

}  // namespace clore::extract
