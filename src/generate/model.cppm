module;

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

export module generate:model;

import extract;

export namespace clore::generate {

// ── page types ──────────────────────────────────────────────────────

enum class PageType : std::uint8_t {
    Index,
    Module,
    Namespace,
    Type,
    File,
    Workflow,
};

auto page_type_name(PageType type) -> std::string_view;

// ── page plan ───────────────────────────────────────────────────────

struct SlotPlan {
    std::string slot_id;
    std::string page_id;
    std::string slot_kind;
    std::string prompt_template_path;
    std::string insertion_marker;
};

struct PagePlan {
    std::string page_id;
    PageType page_type = PageType::File;
    std::string title;
    std::string relative_path;
    std::vector<std::string> owner_keys;
    std::vector<std::string> depends_on_pages;
    std::vector<std::string> linked_pages;
    std::vector<std::string> deterministic_blocks;
    std::vector<SlotPlan> slot_plans;
};

struct PagePlanSet {
    std::vector<PagePlan> plans;
    std::vector<std::string> generation_order;
};

// ── evidence pack ───────────────────────────────────────────────────

struct SymbolFact {
    extract::SymbolID id;
    std::string qualified_name;
    std::string signature;
    std::string kind_label;
    std::string access;
    bool is_template = false;
    std::string template_params;
    std::string declaration_file;
    std::uint32_t declaration_line = 0;
    std::string doc_comment;
};

struct EvidencePack {
    std::string page_id;
    std::string slot_kind;
    std::string subject_name;
    std::string subject_kind;
    std::vector<SymbolFact> target_facts;
    std::vector<SymbolFact> local_context;
    std::vector<SymbolFact> dependency_context;
    std::vector<SymbolFact> reverse_usage_context;
    std::vector<std::string> related_page_summaries;
    std::vector<std::string> source_snippets;
};

// ── rendered page ───────────────────────────────────────────────────

struct SlotOutput {
    std::string slot_id;
    std::string content;
    bool is_failure = false;
};

struct RenderedPage {
    std::string page_id;
    std::string relative_path;
    std::string deterministic_markdown;
    std::vector<SlotOutput> slot_outputs;
    std::string final_markdown;
};

// ── generated page ──────────────────────────────────────────────────

struct GeneratedPage {
    std::string relative_path;
    std::string content;
};

struct GenerateError {
    std::string message;
};

// ── summary cache ───────────────────────────────────────────────────

using PageSummaryCache = std::unordered_map<std::string, std::string>;

// ── link resolver ───────────────────────────────────────────────────

/// Maps entity names (qualified type/namespace names, module names, file paths)
/// to their page relative paths within the output directory.
/// Used to generate markdown cross-reference links.
struct LinkResolver {
    /// qualified_name or module_name or ns_name -> page relative_path
    std::unordered_map<std::string, std::string> name_to_path;

    [[nodiscard]] auto resolve(const std::string& name) const
        -> const std::string* {
        auto it = name_to_path.find(name);
        return it != name_to_path.end() ? &it->second : nullptr;
    }
};

// ── shared helpers ──────────────────────────────────────────────────

auto lookup_sym(const extract::ProjectModel& model, extract::SymbolID id)
    -> const extract::SymbolInfo*;

auto is_type_kind(extract::SymbolKind kind) -> bool;

auto make_source_relative(const std::string& path, const std::string& project_root)
    -> std::string;

}  // namespace clore::generate

// ── implementation ──────────────────────────────────────────────────

namespace clore::generate {

auto page_type_name(PageType type) -> std::string_view {
    switch(type) {
        case PageType::Index:      return "index";
        case PageType::Module:     return "module";
        case PageType::Namespace:  return "namespace";
        case PageType::Type:       return "type";
        case PageType::File:       return "file";
        case PageType::Workflow:   return "workflow";
    }
    return "unknown";
}

auto lookup_sym(const extract::ProjectModel& model, extract::SymbolID id)
    -> const extract::SymbolInfo* {
    auto it = model.symbols.find(id);
    return it != model.symbols.end() ? &it->second : nullptr;
}

auto is_type_kind(extract::SymbolKind kind) -> bool {
    switch(kind) {
        case extract::SymbolKind::Class:
        case extract::SymbolKind::Struct:
        case extract::SymbolKind::Enum:
        case extract::SymbolKind::Union:
        case extract::SymbolKind::Concept:
        case extract::SymbolKind::Template:
        case extract::SymbolKind::TypeAlias:
            return true;
        default:
            return false;
    }
}

auto make_source_relative(const std::string& path, const std::string& project_root)
    -> std::string {
    namespace fs = std::filesystem;
    if(path.empty() || project_root.empty()) return path;
    auto abs = fs::path(path).lexically_normal();
    auto root = fs::path(project_root).lexically_normal();
    auto rel = abs.lexically_relative(root);
    if(rel.empty() || rel.string().starts_with("..")) return path;
    return rel.generic_string();
}

}  // namespace clore::generate
