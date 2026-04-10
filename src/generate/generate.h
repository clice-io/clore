#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "config/schema.h"
#include "extract/model.h"
#include "extract/symbol.h"

namespace clore::generate {

// ── page hierarchy ──────────────────────────────────────────────────

enum class PageLevel : std::uint8_t {
    Symbol,      ///< A single function or variable (used only for very large files).
    ClassStruct, ///< A class/struct and all its members.
    File,        ///< One source/header file.
    Namespace,   ///< A namespace aggregating multiple files.
    Repository,  ///< Top-level project overview.
};

// ── structured page context ─────────────────────────────────────────

/// Pre-assembled context slice for one symbol — ready for prompt
/// serialization without touching ProjectModel again.
struct SymbolContext {
    extract::SymbolInfo self;
    std::vector<extract::SymbolInfo> direct_callers;
    std::vector<extract::SymbolInfo> direct_callees;
    std::vector<extract::SymbolInfo> siblings;
    std::optional<extract::SymbolInfo> parent_class;
};

struct StructuredPagePlan {
    std::string relative_path;
    std::string title;
    PageLevel level = PageLevel::File;
    std::vector<SymbolContext> contexts;
    std::vector<std::string> linked_pages;
};

// ── page graph ──────────────────────────────────────────────────────

struct PageNode {
    StructuredPagePlan plan;
    std::vector<std::string> depends_on;
    std::vector<std::string> depended_by;
};

struct PageGraph {
    std::unordered_map<std::string, PageNode> nodes;
    std::vector<std::string> generation_order;
};

// ── output ──────────────────────────────────────────────────────────

struct PromptPage {
    std::string relative_path;
    std::string title;
    std::string prompt;
};

struct GeneratedPage {
    std::string relative_path;
    std::string content;
};

struct GenerateError {
    std::string message;
};

// ── public API ──────────────────────────────────────────────────────

auto build_page_graph(const config::TaskConfig& config, const extract::ProjectModel& model)
    -> PageGraph;

auto build_prompts(const config::TaskConfig& config, const extract::ProjectModel& model)
    -> std::expected<std::vector<PromptPage>, GenerateError>;

auto generate_pages(const config::TaskConfig& config, const extract::ProjectModel& model,
                    std::string_view llm_model)
    -> std::expected<std::vector<GeneratedPage>, GenerateError>;

auto write_prompts(const std::vector<PromptPage>& prompts, std::string_view output_root)
    -> std::expected<void, GenerateError>;

auto write_pages(const std::vector<GeneratedPage>& pages, std::string_view output_root)
    -> std::expected<void, GenerateError>;

}  // namespace clore::generate
