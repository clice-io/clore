export module generate:model;

import std;
import extract;
import support;

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
    FunctionAnalysis,
    TypeAnalysis,
    VariableAnalysis,
    FunctionDeclarationSummary,
    FunctionImplementationSummary,
    TypeDeclarationSummary,
    TypeImplementationSummary,
};

auto prompt_kind_name(PromptKind kind) -> std::string_view;

struct PromptRequest {
    PromptKind kind = PromptKind::NamespaceSummary;
    std::string target_key{};
};

struct PagePlan {
    std::string page_id{};
    PageType page_type = PageType::File;
    std::string title{};
    std::string relative_path{};
    std::vector<std::string> owner_keys{};
    std::vector<std::string> depends_on_pages{};
    std::vector<std::string> linked_pages{};
    std::vector<PromptRequest> prompt_requests{};
};

struct PagePlanSet {
    std::vector<PagePlan> plans{};
    std::vector<std::string> generation_order{};
};

struct GeneratedPage {
    std::string title{};
    std::string relative_path{};
    std::string content{};
};

struct GenerationSummary {
    std::size_t written_output_count = 0;
    std::size_t symbol_analysis_cache_hits = 0;
    std::size_t symbol_analysis_cache_misses = 0;
    std::size_t page_prompt_cache_hits = 0;
    std::size_t page_prompt_cache_misses = 0;
};

struct GenerateError {
    std::string message;
};

struct RenderError {
    std::string message;
};

struct MarkdownFragmentResponse {
    std::string markdown;
};

struct FunctionAnalysis {
    std::string overview_markdown;
    std::string details_markdown;
    bool has_side_effects = false;
    std::vector<std::string> side_effects;
    std::vector<std::string> reads_from;
    std::vector<std::string> writes_to;
    std::vector<std::string> usage_patterns;
};

struct TypeAnalysis {
    std::string overview_markdown;
    std::string details_markdown;
    std::vector<std::string> invariants;
    std::vector<std::string> key_members;
    std::vector<std::string> usage_patterns;
};

struct VariableAnalysis {
    std::string overview_markdown;
    std::string details_markdown;
    bool is_mutated = false;
    std::vector<std::string> mutation_sources;
    std::vector<std::string> usage_patterns;
};

using SummaryTextCache = std::unordered_map<std::string,
                                            std::string,
                                            clore::support::TransparentStringHash,
                                            clore::support::TransparentStringEqual>;
using PageSummaryCache = SummaryTextCache;
using FunctionAnalysisCache = std::unordered_map<std::string,
                                                 FunctionAnalysis,
                                                 clore::support::TransparentStringHash,
                                                 clore::support::TransparentStringEqual>;
using TypeAnalysisCache = std::unordered_map<std::string,
                                             TypeAnalysis,
                                             clore::support::TransparentStringHash,
                                             clore::support::TransparentStringEqual>;
using VariableAnalysisCache = std::unordered_map<std::string,
                                                 VariableAnalysis,
                                                 clore::support::TransparentStringHash,
                                                 clore::support::TransparentStringEqual>;

struct SymbolAnalysisStore {
    FunctionAnalysisCache functions;
    TypeAnalysisCache types;
    VariableAnalysisCache variables;
};

auto prompt_request_key(const PromptRequest& request) -> std::string;

auto is_page_summary_prompt(PromptKind kind) -> bool;
auto is_symbol_analysis_prompt(PromptKind kind) -> bool;

struct SymbolTargetKeyView {
    std::string_view qualified_name;
    std::string_view signature;
};

auto make_symbol_target_key(const extract::SymbolInfo& symbol) -> std::string;

auto parse_symbol_target_key(std::string_view target_key) -> SymbolTargetKeyView;

auto find_function_analysis(const SymbolAnalysisStore& analyses, std::string_view symbol_target_key)
    -> const FunctionAnalysis*;

auto find_type_analysis(const SymbolAnalysisStore& analyses, std::string_view symbol_target_key)
    -> const TypeAnalysis*;

auto find_variable_analysis(const SymbolAnalysisStore& analyses, std::string_view symbol_target_key)
    -> const VariableAnalysis*;

auto analysis_overview_markdown(const SymbolAnalysisStore& analyses,
                                const extract::SymbolInfo& symbol) -> const std::string*;

auto analysis_details_markdown(const SymbolAnalysisStore& analyses,
                               const extract::SymbolInfo& symbol) -> const std::string*;

auto is_type_kind(extract::SymbolKind kind) -> bool;

auto is_function_kind(extract::SymbolKind kind) -> bool;

auto is_variable_kind(extract::SymbolKind kind) -> bool;

auto is_page_level_symbol(const extract::ProjectModel& model, const extract::SymbolInfo& sym)
    -> bool;

auto make_source_relative(const std::string& path, const std::string& project_root) -> std::string;

/// Maps entity names (qualified type/namespace names, module names, file paths)
/// to their page relative paths within the output directory.
/// Used to generate markdown cross-reference links.
struct LinkResolver {
    std::unordered_map<std::string, std::string> name_to_path;
    std::unordered_map<std::string, std::string> namespace_to_path;
    std::unordered_map<std::string, std::string> module_to_path;
    std::unordered_map<std::string, std::string> page_id_to_title;

    [[nodiscard]] auto resolve(const std::string& name) const -> const std::string* {
        auto it = name_to_path.find(name);
        return it != name_to_path.end() ? &it->second : nullptr;
    }

    [[nodiscard]] auto resolve_namespace(const std::string& name) const -> const std::string* {
        auto it = namespace_to_path.find(name);
        return it != namespace_to_path.end() ? &it->second : nullptr;
    }

    [[nodiscard]] auto resolve_module(const std::string& name) const -> const std::string* {
        auto it = module_to_path.find(name);
        return it != module_to_path.end() ? &it->second : nullptr;
    }

    [[nodiscard]] auto resolve_page_title(const std::string& page_id) const -> const std::string* {
        auto it = page_id_to_title.find(page_id);
        return it != page_id_to_title.end() ? &it->second : nullptr;
    }
};

auto build_link_resolver(const PagePlanSet& plan_set) -> LinkResolver;

struct PathError {
    std::string message;
};

struct PageIdentity {
    PageType page_type = PageType::File;
    std::string normalized_owner_key{};
    std::string qualified_name{};
    std::string source_relative_path{};
};

auto compute_page_path(const PageIdentity& identity) -> std::expected<std::string, PathError>;

auto validate_no_path_conflicts(const std::vector<std::pair<std::string, std::string>>& path_to_id)
    -> std::expected<void, PathError>;

}  // namespace clore::generate

namespace clore::generate {

namespace {

struct SourceRelativeCache {
    std::shared_mutex mutex;
    std::unordered_map<std::string, std::string> relative_by_key;
};

auto source_relative_cache() -> SourceRelativeCache& {
    static SourceRelativeCache cache;
    return cache;
}

auto parse_length_prefixed_symbol_target_key(std::string_view target_key)
    -> std::optional<SymbolTargetKeyView> {
    auto delimiter = target_key.find(':');
    if(delimiter == std::string_view::npos) {
        return std::nullopt;
    }

    std::size_t qualified_name_size = 0;
    auto* begin = target_key.data();
    auto* end = begin + delimiter;
    auto [ptr, ec] = std::from_chars(begin, end, qualified_name_size);
    if(ec != std::errc{} || ptr != end) {
        return std::nullopt;
    }

    auto qualified_name_offset = delimiter + 1;
    if(qualified_name_offset + qualified_name_size > target_key.size()) {
        return std::nullopt;
    }

    return SymbolTargetKeyView{
        .qualified_name = target_key.substr(qualified_name_offset, qualified_name_size),
        .signature = target_key.substr(qualified_name_offset + qualified_name_size),
    };
}

}  // namespace

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
        case PromptKind::FunctionAnalysis: return "function_analysis";
        case PromptKind::TypeAnalysis: return "type_analysis";
        case PromptKind::VariableAnalysis: return "variable_analysis";
        case PromptKind::FunctionDeclarationSummary: return "function_declaration_summary";
        case PromptKind::FunctionImplementationSummary: return "function_implementation_summary";
        case PromptKind::TypeDeclarationSummary: return "type_declaration_summary";
        case PromptKind::TypeImplementationSummary: return "type_implementation_summary";
    }
    return "unknown_prompt";
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

auto is_symbol_analysis_prompt(PromptKind kind) -> bool {
    return kind == PromptKind::FunctionAnalysis || kind == PromptKind::TypeAnalysis ||
           kind == PromptKind::VariableAnalysis;
}

auto make_symbol_target_key(const extract::SymbolInfo& symbol) -> std::string {
    auto target_key = std::format("{}:{}", symbol.qualified_name.size(), symbol.qualified_name);
    target_key += symbol.signature;
    return target_key;
}

auto parse_symbol_target_key(std::string_view target_key) -> SymbolTargetKeyView {
    if(auto parsed = parse_length_prefixed_symbol_target_key(target_key); parsed.has_value()) {
        return *parsed;
    }

    return SymbolTargetKeyView{
        .qualified_name = target_key,
        .signature = {},
    };
}

auto find_function_analysis(const SymbolAnalysisStore& analyses, std::string_view symbol_target_key)
    -> const FunctionAnalysis* {
    auto it = analyses.functions.find(symbol_target_key);
    return it != analyses.functions.end() ? &it->second : nullptr;
}

auto find_type_analysis(const SymbolAnalysisStore& analyses, std::string_view symbol_target_key)
    -> const TypeAnalysis* {
    auto it = analyses.types.find(symbol_target_key);
    return it != analyses.types.end() ? &it->second : nullptr;
}

auto find_variable_analysis(const SymbolAnalysisStore& analyses, std::string_view symbol_target_key)
    -> const VariableAnalysis* {
    auto it = analyses.variables.find(symbol_target_key);
    return it != analyses.variables.end() ? &it->second : nullptr;
}

template <typename FieldAccessor>
auto analysis_markdown(const SymbolAnalysisStore& analyses,
                       const extract::SymbolInfo& symbol,
                       FieldAccessor&& field_accessor) -> const std::string* {
    auto target_key = make_symbol_target_key(symbol);
    if(is_function_kind(symbol.kind)) {
        if(auto* analysis = find_function_analysis(analyses, target_key)) {
            return field_accessor(*analysis);
        }
        return nullptr;
    }
    if(is_type_kind(symbol.kind)) {
        if(auto* analysis = find_type_analysis(analyses, target_key)) {
            return field_accessor(*analysis);
        }
        return nullptr;
    }
    if(is_variable_kind(symbol.kind)) {
        if(auto* analysis = find_variable_analysis(analyses, target_key)) {
            return field_accessor(*analysis);
        }
    }
    return nullptr;
}

auto analysis_overview_markdown(const SymbolAnalysisStore& analyses,
                                const extract::SymbolInfo& symbol) -> const std::string* {
    return analysis_markdown(analyses, symbol, [](const auto& analysis) -> const std::string* {
        return &analysis.overview_markdown;
    });
}

auto analysis_details_markdown(const SymbolAnalysisStore& analyses,
                               const extract::SymbolInfo& symbol) -> const std::string* {
    return analysis_markdown(analyses, symbol, [](const auto& analysis) -> const std::string* {
        return &analysis.details_markdown;
    });
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

auto is_variable_kind(extract::SymbolKind kind) -> bool {
    return kind == extract::SymbolKind::Variable || kind == extract::SymbolKind::EnumMember;
}

auto is_page_level_symbol(const extract::ProjectModel& model, const extract::SymbolInfo& sym)
    -> bool {
    if(sym.lexical_parent_kind != extract::SymbolKind::Unknown &&
       sym.lexical_parent_kind != extract::SymbolKind::Namespace) {
        return false;
    }

    // Symbols inside anonymous namespaces are implementation details,
    // not page-level public API.
    if(sym.enclosing_namespace.find("(anonymous namespace)") != std::string::npos) {
        return false;
    }
    if(sym.lexical_parent_name.find("(anonymous namespace)") != std::string::npos) {
        return false;
    }

    if(sym.parent.has_value()) {
        if(auto* parent = extract::lookup_symbol(model, *sym.parent)) {
            if(is_type_kind(parent->kind) || is_function_kind(parent->kind)) {
                return false;
            }
        }
    }

    return true;
}

auto make_source_relative(const std::string& path, const std::string& project_root) -> std::string {
    namespace fs = std::filesystem;
    if(path.empty() || project_root.empty()) {
        return path;
    }

    std::string key;
    key.reserve(project_root.size() + path.size() + 1);
    key.append(project_root);
    key.push_back('\n');
    key.append(path);

    auto& cache = source_relative_cache();
    {
        std::shared_lock lock(cache.mutex);
        auto it = cache.relative_by_key.find(key);
        if(it != cache.relative_by_key.end()) {
            return it->second;
        }
    }

    auto absolute = fs::path(path).lexically_normal();
    auto root = fs::path(project_root).lexically_normal();
    auto relative = absolute.lexically_relative(root);

    std::string resolved;
    if(relative.empty() || relative.string().starts_with("..")) {
        resolved = path;
    } else {
        resolved = relative.generic_string();
    }

    {
        std::unique_lock lock(cache.mutex);
        cache.relative_by_key.insert_or_assign(std::move(key), resolved);
    }
    return resolved;
}

auto build_link_resolver(const PagePlanSet& plan_set) -> LinkResolver {
    LinkResolver resolver;

    for(const auto& plan: plan_set.plans) {
        resolver.page_id_to_title[plan.page_id] = plan.title;
        for(const auto& key: plan.owner_keys) {
            // Use emplace to avoid overwriting: if a module and namespace share
            // the same name, the first one registered wins in the generic map.
            // Callers should use resolve_module/resolve_namespace for disambiguation.
            resolver.name_to_path.emplace(key, plan.relative_path);

            if(plan.page_type == PageType::Namespace) {
                resolver.namespace_to_path[key] = plan.relative_path;
            } else if(plan.page_type == PageType::Module) {
                resolver.module_to_path[key] = plan.relative_path;
            }
        }
        auto colon = plan.page_id.find(':');
        if(colon != std::string::npos) {
            auto suffix = plan.page_id.substr(colon + 1);
            resolver.name_to_path.emplace(suffix, plan.relative_path);

            if(plan.page_type == PageType::Namespace) {
                resolver.namespace_to_path[suffix] = plan.relative_path;
            } else if(plan.page_type == PageType::Module) {
                resolver.module_to_path[suffix] = plan.relative_path;
            }
        }
    }

    return resolver;
}

namespace {

constexpr std::string_view kIndexPath = "index.md";
constexpr std::string_view kModulePrefix = "modules";
constexpr std::string_view kNamespacePrefix = "namespaces";
constexpr std::string_view kFilePrefix = "files";

auto normalize_name(std::string_view name) -> std::string {
    std::string result(name);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

auto split_qualified(std::string_view name, std::string_view sep) -> std::vector<std::string> {
    std::vector<std::string> parts;
    std::size_t pos = 0;
    while(pos < name.size()) {
        auto found = name.find(sep, pos);
        if(found == std::string_view::npos) {
            parts.emplace_back(name.substr(pos));
            break;
        }
        parts.emplace_back(name.substr(pos, found - pos));
        pos = found + sep.size();
    }
    return parts;
}

auto join_path(const std::vector<std::string>& parts) -> std::string {
    std::string result;
    for(std::size_t i = 0; i < parts.size(); ++i) {
        if(i > 0)
            result += '/';
        result += parts[i];
    }
    return result;
}

auto validate_path_component(std::string_view path) -> std::expected<void, PathError> {
    if(path.empty()) {
        return std::unexpected(PathError{.message = "generated path is empty"});
    }
    if(path.find("..") != std::string_view::npos) {
        return std::unexpected(
            PathError{.message = std::format("generated path contains '..': {}", path)});
    }
    if(path.starts_with('/') || path.starts_with('\\')) {
        return std::unexpected(
            PathError{.message = std::format("generated path is absolute: {}", path)});
    }
    return {};
}

// Replace characters illegal in Windows file/directory names
auto sanitize_path_chars(std::string& path) -> void {
    for(auto& c: path) {
        switch(c) {
            case '<':
            case '>':
            case '"':
            case '|':
            case '?':
            case '*': c = '_'; break;
            default: break;
        }
    }
}

}  // namespace

auto compute_page_path(const PageIdentity& identity) -> std::expected<std::string, PathError> {
    std::string result;

    switch(identity.page_type) {
        case PageType::Index: {
            result = std::string(kIndexPath);
            break;
        }
        case PageType::Module: {
            auto parts = split_qualified(identity.qualified_name, ".");
            std::string partition;
            // Check for partition: "foo.bar:baz"
            if(!parts.empty()) {
                auto& last = parts.back();
                auto colon = last.find(':');
                if(colon != std::string::npos) {
                    partition = last.substr(colon + 1);
                    last = last.substr(0, colon);
                }
            }
            std::vector<std::string> norm_parts;
            norm_parts.push_back(std::string(kModulePrefix));
            for(auto& p: parts) {
                norm_parts.push_back(normalize_name(p));
            }
            if(partition.empty()) {
                norm_parts.push_back("index.md");
            } else {
                norm_parts.push_back(normalize_name(partition) + ".md");
            }
            result = join_path(norm_parts);
            break;
        }
        case PageType::Namespace: {
            auto parts = split_qualified(identity.qualified_name, "::");
            std::vector<std::string> norm_parts;
            norm_parts.push_back(std::string(kNamespacePrefix));
            for(auto& p: parts) {
                norm_parts.push_back(normalize_name(p));
            }
            norm_parts.push_back("index.md");
            result = join_path(norm_parts);
            break;
        }
        case PageType::File: {
            auto& source_rel = identity.source_relative_path;
            // Strip extension and add .md
            auto dot = source_rel.rfind('.');
            std::string stem;
            if(dot != std::string::npos) {
                stem = source_rel.substr(0, dot);
            } else {
                stem = source_rel;
            }
            result = std::string(kFilePrefix) + "/" + stem + ".md";
            break;
        }
    }

    sanitize_path_chars(result);

    if(auto r = validate_path_component(result); !r.has_value()) {
        return std::unexpected(std::move(r.error()));
    }

    return result;
}

auto validate_no_path_conflicts(const std::vector<std::pair<std::string, std::string>>& path_to_id)
    -> std::expected<void, PathError> {
    std::unordered_map<std::string, std::string> seen;
    for(auto& [path, id]: path_to_id) {
        auto [it, inserted] = seen.emplace(path, id);
        if(!inserted) {
            return std::unexpected(
                PathError{.message = std::format("path conflict: '{}' and '{}' both map to '{}'",
                                                 it->second,
                                                 id,
                                                 path)});
        }
    }
    return {};
}

}  // namespace clore::generate
