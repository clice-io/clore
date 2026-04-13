module;

#include <algorithm>
#include <cctype>
#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

export module generate:path;

import :model;

export namespace clore::generate {

struct PathError {
    std::string message;
};

struct PageIdentity {
    PageType page_type = PageType::File;
    std::string normalized_owner_key;
    std::string qualified_name;
    std::string source_relative_path;
};

auto compute_page_path(const PageIdentity& identity) -> std::expected<std::string, PathError>;

auto validate_no_path_conflicts(const std::vector<std::pair<std::string, std::string>>& path_to_id)
    -> std::expected<void, PathError>;

}  // namespace clore::generate

// ── implementation ──────────────────────────────────────────────────

namespace clore::generate {

namespace {

constexpr std::string_view kIndexPath = "index.md";
constexpr std::string_view kModulePrefix = "modules";
constexpr std::string_view kNamespacePrefix = "namespaces";
constexpr std::string_view kFilePrefix = "files";
constexpr std::string_view kWorkflowPrefix = "workflows";

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

    auto trim_ascii = [](std::string_view text) -> std::string_view {
        while(!text.empty() && std::isspace(static_cast<unsigned char>(text.front())) != 0) {
            text.remove_prefix(1);
        }
        while(!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
            text.remove_suffix(1);
        }
        return text;
    };

    auto sanitize_workflow_slug = [&](std::string_view raw_slug) -> std::string {
        auto trimmed = trim_ascii(raw_slug);
        std::string sanitized;
        sanitized.reserve(trimmed.size());

        bool prev_dash = false;
        for(auto ch: trimmed) {
            auto uc = static_cast<unsigned char>(ch);
            if(ch == '/' || ch == '\\' || std::isspace(uc) != 0) {
                if(!prev_dash) {
                    sanitized.push_back('-');
                    prev_dash = true;
                }
                continue;
            }
            sanitized.push_back(ch);
            prev_dash = (ch == '-');
        }

        while(!sanitized.empty() && sanitized.front() == '-')
            sanitized.erase(sanitized.begin());
        while(!sanitized.empty() && sanitized.back() == '-')
            sanitized.pop_back();
        if(sanitized.empty()) {
            sanitized = "unnamed";
        }
        return sanitized;
    };

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
        case PageType::Workflow: {
            auto slug = sanitize_workflow_slug(normalize_name(identity.normalized_owner_key));
            std::vector<std::string> norm_parts;
            norm_parts.push_back(std::string(kWorkflowPrefix));
            norm_parts.push_back(slug + ".md");
            result = join_path(norm_parts);
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
