export module extract:model;

import std;
import support;
import config;

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
    /// Additional signature derived from the same USR source, used to
    /// disambiguate the astronomically unlikely case of a 64-bit hash collision.
    std::uint32_t signature = 0;

    [[nodiscard]] bool is_valid() const noexcept {
        return hash != 0;
    }

    bool operator==(const SymbolID&) const = default;
    auto operator<=>(const SymbolID&) const = default;
};

}  // namespace clore::extract

export template <>
struct std::hash<clore::extract::SymbolID> {
    std::size_t operator()(const clore::extract::SymbolID& id) const noexcept {
        // Combine the 64-bit hash with the 32-bit signature into a single
        // std::size_t hash value.  Using the full 96 bits means the chance
        // of an undetected collision is negligible (2^-96).
        return std::hash<std::uint64_t>{}(id.hash) ^ (static_cast<std::size_t>(id.signature) << 1);
    }
};

export namespace clore::extract {

auto split_top_level_qualified_name(std::string_view qualified_name) -> std::vector<std::string>;

auto join_qualified_name_parts(const std::vector<std::string>& parts, std::size_t count)
    -> std::string;

auto namespace_prefix_from_qualified_name(std::string_view qualified_name) -> std::string;

struct SourceLocation {
    std::string file;
    /// Line 0 means the location is unknown; valid source lines start at 1.
    std::uint32_t line = 0;
    std::uint32_t column = 0;

    [[nodiscard]] bool is_known() const noexcept {
        return line != 0;
    }
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
    std::string enclosing_namespace;
    std::string signature;

    SourceLocation declaration_location;
    std::optional<SourceLocation> definition_location;

    std::string doc_comment;
    std::string source_snippet;

    /// When source_snippet is empty, these fields reference the raw source
    /// text in declaration_location.file so the snippet can be resolved
    /// on demand instead of being stored in memory for every symbol.
    std::uint32_t source_snippet_offset = 0;
    std::uint32_t source_snippet_length = 0;
    std::uint64_t source_snippet_file_size = 0;
    std::uint64_t source_snippet_hash = 0;

    std::optional<SymbolID> parent;
    std::string lexical_parent_name;
    SymbolKind lexical_parent_kind = SymbolKind::Unknown;
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
    std::vector<std::string> children;
};

/// Represents a single C++20 module unit (interface or partition).
struct ModuleUnit {
    std::string name;                  ///< Full module name, e.g. "foo" or "foo:bar"
    bool is_interface = false;         ///< true for `export module`, false for `module`
    std::string source_file;           ///< Normalized path to the source file
    std::vector<std::string> imports;  ///< Module imports
    std::vector<SymbolID> symbols;     ///< Symbols declared in this module unit
};

struct ProjectModel {
    std::unordered_map<SymbolID, SymbolInfo> symbols;
    std::unordered_map<std::string, FileInfo> files;
    std::unordered_map<std::string, NamespaceInfo> namespaces;

    std::vector<std::string> file_order;

    /// Module units indexed by normalized source file path.
    std::unordered_map<std::string,
                       ModuleUnit,
                       clore::support::TransparentStringHash,
                       clore::support::TransparentStringEqual>
        modules;

    /// Exact qualified-name lookup for generation and evidence building.
    /// A qualified name can map to multiple SymbolIDs when overloads exist.
    std::unordered_map<std::string,
                       std::vector<SymbolID>,
                       clore::support::TransparentStringHash,
                       clore::support::TransparentStringEqual>
        symbol_ids_by_qualified_name;

    /// Exact module-name lookup for generation and cross-linking.
    std::unordered_map<std::string,
                       std::vector<std::string>,
                       clore::support::TransparentStringHash,
                       clore::support::TransparentStringEqual>
        module_name_to_sources;

    /// True if the project uses C++20 modules (at least one module declaration
    /// found).
    bool uses_modules = false;
};

auto lookup_symbol(const ProjectModel& model, SymbolID id) -> const SymbolInfo*;

auto find_symbol(const ProjectModel& model, std::string_view qualified_name) -> const SymbolInfo*;

auto find_symbol(const ProjectModel& model,
                 std::string_view qualified_name,
                 std::string_view signature) -> const SymbolInfo*;

auto find_symbols(const ProjectModel& model, std::string_view qualified_name)
    -> std::vector<const SymbolInfo*>;

auto find_module_by_name(const ProjectModel& model, std::string_view module_name)
    -> const ModuleUnit*;

auto find_modules_by_name(const ProjectModel& model, std::string_view module_name)
    -> std::vector<const ModuleUnit*>;

auto find_module_by_source(const ProjectModel& model, std::string_view source_file)
    -> const ModuleUnit*;

/// Populate source_snippet from the on-disk file using the recorded
/// source_snippet_offset / source_snippet_length fields.  Returns true
/// if the snippet was successfully resolved (or already cached).
auto resolve_source_snippet(SymbolInfo& sym) -> bool;

/// Returns true if the symbol should be included in documentation output
/// according to the given symbol filter configuration.
auto matches_symbol_filter(const SymbolInfo& sym,
                           const config::SymbolFilterConfig& rule) -> bool;

}  // namespace clore::extract

namespace clore::extract {

namespace {

constexpr std::size_t kSplitQualifiedNameCacheMaxEntries = 10'000;
constexpr std::uint64_t kSourceSnippetHashOffsetBasis = 14695981039346656037ULL;
constexpr std::uint64_t kSourceSnippetHashPrime = 1099511628211ULL;
constexpr std::size_t kSourceSnippetMutexStripeCount = 64;

struct SplitQualifiedNameCache {
    std::shared_mutex mutex;
    std::unordered_map<std::string,
                       std::vector<std::string>,
                       clore::support::TransparentStringHash,
                       clore::support::TransparentStringEqual>
        parts_by_qualified_name;
};

auto split_qualified_name_cache() -> SplitQualifiedNameCache& {
    static SplitQualifiedNameCache cache;
    return cache;
}

auto source_snippet_mutex_for(const SymbolInfo& sym) -> std::mutex& {
    static std::array<std::mutex, kSourceSnippetMutexStripeCount> mutexes;
    auto key = std::hash<SymbolID>{}(sym.id);
    return mutexes[key % mutexes.size()];
}

auto hash_source_snippet_bytes(std::string_view bytes) -> std::uint64_t {
    auto hash = kSourceSnippetHashOffsetBasis;
    for(auto ch: bytes) {
        hash ^= static_cast<std::uint64_t>(static_cast<unsigned char>(ch));
        hash *= kSourceSnippetHashPrime;
    }
    return hash;
}

}  // namespace

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

auto split_top_level_qualified_name(std::string_view qualified_name) -> std::vector<std::string> {
    if(qualified_name.empty()) {
        return {};
    }

    auto& cache = split_qualified_name_cache();
    {
        std::shared_lock lock(cache.mutex);
        auto it = cache.parts_by_qualified_name.find(qualified_name);
        if(it != cache.parts_by_qualified_name.end()) {
            return it->second;
        }
    }

    std::vector<std::string> parts;
    std::string current;
    current.reserve(qualified_name.size());

    std::size_t template_depth = 0;
    for(std::size_t index = 0; index < qualified_name.size(); ++index) {
        auto ch = qualified_name[index];
        if(ch == '<') {
            template_depth++;
            current.push_back(ch);
            continue;
        }
        if(ch == '>') {
            if(template_depth > 0) {
                template_depth--;
            }
            current.push_back(ch);
            continue;
        }
        if(ch == ':' && template_depth == 0 && index + 1 < qualified_name.size() &&
           qualified_name[index + 1] == ':') {
            if(!current.empty()) {
                parts.push_back(current);
            }
            current.clear();
            ++index;
            continue;
        }
        current.push_back(ch);
    }

    if(!current.empty()) {
        parts.push_back(std::move(current));
    }

    if(!parts.empty() && parts.front().empty()) {
        parts.erase(parts.begin());
    }

    {
        std::unique_lock lock(cache.mutex);
        if(cache.parts_by_qualified_name.size() >= kSplitQualifiedNameCacheMaxEntries) {
            cache.parts_by_qualified_name.clear();
        }
        cache.parts_by_qualified_name.insert_or_assign(std::string(qualified_name), parts);
    }
    return parts;
}

auto join_qualified_name_parts(const std::vector<std::string>& parts, std::size_t count)
    -> std::string {
    std::string joined;
    auto safe_count = count < parts.size() ? count : parts.size();
    for(std::size_t index = 0; index < safe_count; ++index) {
        if(index > 0) {
            joined += "::";
        }
        joined += parts[index];
    }
    return joined;
}

auto namespace_prefix_from_qualified_name(std::string_view qualified_name) -> std::string {
    auto parts = split_top_level_qualified_name(qualified_name);
    if(parts.size() <= 1) {
        return {};
    }
    return join_qualified_name_parts(parts, parts.size() - 1);
}

auto lookup_symbol(const ProjectModel& model, SymbolID id) -> const SymbolInfo* {
    auto it = model.symbols.find(id);
    return it != model.symbols.end() ? &it->second : nullptr;
}

auto find_symbols(const ProjectModel& model, std::string_view qualified_name)
    -> std::vector<const SymbolInfo*> {
    std::vector<const SymbolInfo*> matches;
    auto it = model.symbol_ids_by_qualified_name.find(qualified_name);
    if(it == model.symbol_ids_by_qualified_name.end()) {
        return matches;
    }

    matches.reserve(it->second.size());
    for(auto symbol_id: it->second) {
        if(auto* symbol = lookup_symbol(model, symbol_id)) {
            matches.push_back(symbol);
        }
    }
    return matches;
}

auto find_symbol(const ProjectModel& model, std::string_view qualified_name) -> const SymbolInfo* {
    auto matches = find_symbols(model, qualified_name);
    if(matches.size() != 1) {
        return nullptr;
    }
    return matches.front();
}

auto find_symbol(const ProjectModel& model,
                 std::string_view qualified_name,
                 std::string_view signature) -> const SymbolInfo* {
    if(signature.empty()) {
        return find_symbol(model, qualified_name);
    }

    auto matches = find_symbols(model, qualified_name);
    for(auto* symbol: matches) {
        if(symbol->signature == signature) {
            return symbol;
        }
    }
    return nullptr;
}

auto find_modules_by_name(const ProjectModel& model, std::string_view module_name)
    -> std::vector<const ModuleUnit*> {
    std::vector<const ModuleUnit*> modules;
    auto it = model.module_name_to_sources.find(module_name);
    if(it == model.module_name_to_sources.end() || it->second.empty()) {
        return modules;
    }

    modules.reserve(it->second.size());
    for(const auto& source: it->second) {
        if(auto* module = find_module_by_source(model, source)) {
            modules.push_back(module);
        }
    }

    std::sort(modules.begin(), modules.end(), [](const ModuleUnit* lhs, const ModuleUnit* rhs) {
        return lhs->source_file < rhs->source_file;
    });
    return modules;
}

auto find_module_by_name(const ProjectModel& model, std::string_view module_name)
    -> const ModuleUnit* {
    auto modules = find_modules_by_name(model, module_name);
    if(modules.empty()) {
        return nullptr;
    }
    if(modules.size() == 1) {
        return modules.front();
    }

    const ModuleUnit* single_interface = nullptr;
    std::size_t interface_count = 0;
    for(auto* module: modules) {
        if(module->is_interface) {
            single_interface = module;
            ++interface_count;
        }
    }
    if(interface_count == 1) {
        return single_interface;
    }
    if(interface_count == 0) {
        logging::warn(
            "module '{}' has {} implementation units but no interface unit; " "using first unit as fallback",
            module_name,
            modules.size());
        return modules.front();
    }

    logging::warn("module '{}' is ambiguous: {} interface units", module_name, interface_count);
    return nullptr;
}

auto find_module_by_source(const ProjectModel& model, std::string_view source_file)
    -> const ModuleUnit* {
    auto it = model.modules.find(source_file);
    return it != model.modules.end() ? &it->second : nullptr;
}

auto resolve_source_snippet(SymbolInfo& sym) -> bool {
    std::lock_guard lock(source_snippet_mutex_for(sym));

    if(!sym.source_snippet.empty()) {
        return true;
    }
    if(sym.source_snippet_length == 0 || sym.declaration_location.file.empty()) {
        return false;
    }

    namespace fs = std::filesystem;
    std::error_code file_size_error;
    auto file_size = fs::file_size(sym.declaration_location.file, file_size_error);
    if(file_size_error) {
        return false;
    }
    auto file_size_value = static_cast<std::size_t>(file_size);
    auto offset = static_cast<std::size_t>(sym.source_snippet_offset);
    auto length = static_cast<std::size_t>(sym.source_snippet_length);

    if(sym.source_snippet_file_size != 0 && sym.source_snippet_file_size != file_size_value) {
        return false;
    }

    if(offset > file_size_value || length > file_size_value - offset) {
        return false;
    }

    std::ifstream file(sym.declaration_location.file, std::ios::binary);
    if(!file) {
        return false;
    }

    file.seekg(static_cast<std::streamoff>(offset));
    std::string result(length, '\0');
    file.read(result.data(), static_cast<std::streamsize>(length));
    if(!file) {
        return false;
    }

    if(sym.source_snippet_hash != 0 &&
       hash_source_snippet_bytes(result) != sym.source_snippet_hash) {
        return false;
    }

    // Normalize \r\n to \n
    std::string normalized;
    normalized.reserve(result.size());
    for(std::size_t i = 0; i < result.size(); ++i) {
        if(result[i] == '\r' && i + 1 < result.size() && result[i + 1] == '\n') {
            continue;
        }
        normalized += result[i];
    }

    sym.source_snippet = std::move(normalized);
    return true;
}

auto matches_symbol_filter(const SymbolInfo& sym,
                           const config::SymbolFilterConfig& rule) -> bool {
    if(rule.hide_private && sym.access == "private") {
        return false;
    }
    if(rule.hide_protected && sym.access == "protected") {
        return false;
    }
    for(const auto& prefix: rule.exclude_name_prefixes) {
        if(!prefix.empty() && sym.name.starts_with(prefix)) {
            return false;
        }
    }
    if(!rule.exclude_namespace_segments.empty()) {
        auto parts = split_top_level_qualified_name(sym.enclosing_namespace);
        for(const auto& part: parts) {
            for(const auto& segment: rule.exclude_namespace_segments) {
                if(part == segment) {
                    return false;
                }
            }
        }
    }
    return true;
}

}  // namespace clore::extract
