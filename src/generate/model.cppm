module;

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

export module generate:model;

import extract;

export namespace clore::generate {

enum class PageType : std::uint8_t {
    Index,
    Module,
    Namespace,
    File,
};

auto page_type_name(PageType type) -> std::string_view;

enum class PromptKind : std::uint8_t {
    NamespaceSummary,
    ModuleSummary,
    ModuleArchitecture,
    IndexOverview,
    FunctionDeclarationSummary,
    FunctionImplementationSummary,
    TypeDeclarationSummary,
    TypeImplementationSummary,
};

auto prompt_kind_name(PromptKind kind) -> std::string_view;

struct PromptRequest {
    PromptKind kind = PromptKind::NamespaceSummary;
    std::string target_key;
};

struct SymbolTargetKeyView {
    std::string_view qualified_name;
    std::string_view signature;
};

struct PagePlan {
    std::string page_id;
    PageType page_type = PageType::File;
    std::string title;
    std::string relative_path;
    std::vector<std::string> owner_keys;
    std::vector<std::string> depends_on_pages;
    std::vector<std::string> linked_pages;
    std::vector<PromptRequest> prompt_requests;
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
    std::string prompt_kind;
    std::string subject_name;
    std::string subject_kind;
    std::vector<SymbolFact> target_facts;
    std::vector<SymbolFact> local_context;
    std::vector<SymbolFact> dependency_context;
    std::vector<SymbolFact> reverse_usage_context;
    std::vector<std::string> related_page_summaries;
    std::vector<std::string> source_snippets;
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
    /// page_id -> rendered title
    std::unordered_map<std::string, std::string> page_id_to_title;

    [[nodiscard]] auto resolve(const std::string& name) const -> const std::string* {
        auto it = name_to_path.find(name);
        return it != name_to_path.end() ? &it->second : nullptr;
    }

    [[nodiscard]] auto resolve_page_title(const std::string& page_id) const -> const std::string* {
        auto it = page_id_to_title.find(page_id);
        return it != page_id_to_title.end() ? &it->second : nullptr;
    }
};

// ── shared helpers ──────────────────────────────────────────────────

auto lookup_sym(const extract::ProjectModel& model, extract::SymbolID id)
    -> const extract::SymbolInfo*;

auto make_symbol_target_key(const extract::SymbolInfo& symbol) -> std::string;

auto parse_symbol_target_key(std::string_view target_key) -> SymbolTargetKeyView;

auto find_sym(const extract::ProjectModel& model, std::string_view symbol_target_key)
    -> const extract::SymbolInfo*;

auto is_type_kind(extract::SymbolKind kind) -> bool;

auto is_function_kind(extract::SymbolKind kind) -> bool;

auto is_page_level_symbol(const extract::ProjectModel& model, const extract::SymbolInfo& sym)
    -> bool;

auto prompt_request_key(const PromptRequest& request) -> std::string;

auto is_page_summary_prompt(PromptKind kind) -> bool;

auto make_source_relative(const std::string& path, const std::string& project_root) -> std::string;

}  // namespace clore::generate

// ── implementation ──────────────────────────────────────────────────

namespace clore::generate {

auto page_type_name(PageType type) -> std::string_view {
    switch(type) {
        case PageType::Index: return "index";
        case PageType::Module: return "module";
        case PageType::Namespace: return "namespace";
        case PageType::File: return "file";
    }
    return "unknown";
}

auto prompt_kind_name(PromptKind kind) -> std::string_view {
    switch(kind) {
        case PromptKind::NamespaceSummary: return "namespace_summary";
        case PromptKind::ModuleSummary: return "module_summary";
        case PromptKind::ModuleArchitecture: return "module_architecture";
        case PromptKind::IndexOverview: return "index_overview";
        case PromptKind::FunctionDeclarationSummary: return "function_declaration_summary";
        case PromptKind::FunctionImplementationSummary: return "function_implementation_summary";
        case PromptKind::TypeDeclarationSummary: return "type_declaration_summary";
        case PromptKind::TypeImplementationSummary: return "type_implementation_summary";
    }
    return "unknown_prompt";
}

auto lookup_sym(const extract::ProjectModel& model, extract::SymbolID id)
    -> const extract::SymbolInfo* {
    return extract::lookup_symbol(model, id);
}

auto make_symbol_target_key(const extract::SymbolInfo& symbol) -> std::string {
    if(symbol.signature.empty()) {
        return symbol.qualified_name;
    }
    return symbol.qualified_name + "\t" + symbol.signature;
}

auto parse_symbol_target_key(std::string_view target_key) -> SymbolTargetKeyView {
    auto delimiter = target_key.rfind('\t');
    if(delimiter == std::string_view::npos) {
        return SymbolTargetKeyView{
            .qualified_name = target_key,
            .signature = {},
        };
    }
    return SymbolTargetKeyView{
        .qualified_name = target_key.substr(0, delimiter),
        .signature = target_key.substr(delimiter + 1),
    };
}

auto find_sym(const extract::ProjectModel& model, std::string_view symbol_target_key)
    -> const extract::SymbolInfo* {
    auto target = parse_symbol_target_key(symbol_target_key);
    return extract::find_symbol(model, target.qualified_name, target.signature);
}

auto is_type_kind(extract::SymbolKind kind) -> bool {
    switch(kind) {
        case extract::SymbolKind::Class:
        case extract::SymbolKind::Struct:
        case extract::SymbolKind::Enum:
        case extract::SymbolKind::Union:
        case extract::SymbolKind::Concept:
        case extract::SymbolKind::Template:
        case extract::SymbolKind::TypeAlias: return true;
        default: return false;
    }
}

auto is_function_kind(extract::SymbolKind kind) -> bool {
    switch(kind) {
        case extract::SymbolKind::Function:
        case extract::SymbolKind::Method: return true;
        default: return false;
    }
}

auto is_page_level_symbol(const extract::ProjectModel& model, const extract::SymbolInfo& sym)
    -> bool {
    if(sym.lexical_parent_kind != extract::SymbolKind::Unknown &&
       sym.lexical_parent_kind != extract::SymbolKind::Namespace) {
        return false;
    }

    if(sym.parent.has_value()) {
        if(auto* parent = lookup_sym(model, *sym.parent)) {
            if(is_type_kind(parent->kind) || is_function_kind(parent->kind)) {
                return false;
            }
        }
    }

    return true;
}

auto prompt_request_key(const PromptRequest& request) -> std::string {
    if(request.target_key.empty()) {
        return std::string(prompt_kind_name(request.kind));
    }
    return std::string(prompt_kind_name(request.kind)) + ":" + request.target_key;
}

auto is_page_summary_prompt(PromptKind kind) -> bool {
    return kind == PromptKind::NamespaceSummary || kind == PromptKind::ModuleSummary;
}

auto make_source_relative(const std::string& path, const std::string& project_root) -> std::string {
    namespace fs = std::filesystem;
    if(path.empty() || project_root.empty())
        return path;
    auto abs = fs::path(path).lexically_normal();
    auto root = fs::path(project_root).lexically_normal();
    auto rel = abs.lexically_relative(root);
    if(rel.empty() || rel.string().starts_with(".."))
        return path;
    return rel.generic_string();
}

}  // namespace clore::generate
