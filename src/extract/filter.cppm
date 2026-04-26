export module extract:filter;

import std;
import config;

export namespace clore::extract {

struct PathResolveError {
    std::string message;
};

auto path_prefix_matches(std::string_view relative, std::string_view pattern) -> bool;

auto project_relative_path(const std::filesystem::path& file,
                           const std::filesystem::path& root_path)
    -> std::optional<std::filesystem::path>;

auto resolve_path_under_directory(const std::string& path, const std::string& directory)
    -> std::expected<std::filesystem::path, PathResolveError>;

auto canonical_graph_path(const std::filesystem::path& path) -> std::string;

auto matches_filter(const std::string& file,
                    const config::FilterRule& filter,
                    const std::filesystem::path& filter_root) -> bool;

auto filter_root_path(const config::TaskConfig& config) -> std::filesystem::path;

}  // namespace clore::extract

namespace clore::extract {

auto path_prefix_matches(std::string_view relative, std::string_view pattern) -> bool {
    if(pattern.empty()) {
        return false;
    }

    while(!pattern.empty() && pattern.back() == '/') {
        pattern.remove_suffix(1);
    }
    if(pattern.empty()) {
        return false;
    }

    if(pattern.find('/') != std::string_view::npos) {
        if(!relative.starts_with(pattern)) {
            return false;
        }
        return relative.size() == pattern.size() || relative[pattern.size()] == '/';
    }

    if(relative == pattern) {
        return true;
    }
    if(relative.size() < pattern.size() + 1) {
        return false;
    }
    if(!relative.starts_with(pattern)) {
        return false;
    }
    return relative[pattern.size()] == '/';
}

auto project_relative_path(const std::filesystem::path& file,
                           const std::filesystem::path& root_path)
    -> std::optional<std::filesystem::path> {
    auto rel = file.lexically_relative(root_path);
    if(rel.empty()) {
        return std::nullopt;
    }
    for(const auto& part: rel) {
        if(part == "..") {
            return std::nullopt;
        }
    }
    return rel;
}

auto resolve_path_under_directory(const std::string& path, const std::string& directory)
    -> std::expected<std::filesystem::path, PathResolveError> {
    namespace fs = std::filesystem;

    if(path.empty()) {
        return std::unexpected(
            PathResolveError{.message = "compilation database entry has empty file path"});
    }

    auto p = fs::path(path);
    if(p.is_relative()) {
        if(directory.empty()) {
            return std::unexpected(PathResolveError{
                .message = std::format(
                    "relative file path '{}' requires a non-empty " "'directory' in compile_commands.json",
                    path),
            });
        }
        p = fs::path(directory) / p;
    }

    return p.lexically_normal();
}

auto canonical_graph_path(const std::filesystem::path& path) -> std::string {
    namespace fs = std::filesystem;
    std::error_code ec;
    auto absolute = fs::absolute(path, ec);
    if(!ec) {
        auto canonical = fs::weakly_canonical(absolute.lexically_normal(), ec);
        if(!ec) {
            return canonical.generic_string();
        }
        return absolute.lexically_normal().generic_string();
    }

    auto normalized = path.lexically_normal();
    auto canonical = fs::weakly_canonical(normalized, ec);
    if(!ec) {
        return canonical.generic_string();
    }

    return normalized.generic_string();
}

auto matches_filter(const std::string& file,
                    const config::FilterRule& filter,
                    const std::filesystem::path& filter_root) -> bool {
    namespace fs = std::filesystem;

    auto file_path = fs::path(canonical_graph_path(fs::path(file)));
    auto root_path = fs::path(canonical_graph_path(filter_root));

    auto rel_opt = project_relative_path(file_path, root_path);
    if(!rel_opt.has_value()) {
        return false;
    }

    auto relative_str = rel_opt->generic_string();

    if(!filter.include.empty()) {
        bool matched = false;
        for(const auto& pattern: filter.include) {
            if(path_prefix_matches(relative_str, pattern)) {
                matched = true;
                break;
            }
        }
        if(!matched) {
            return false;
        }
    }

    for(const auto& pattern: filter.exclude) {
        if(path_prefix_matches(relative_str, pattern)) {
            return false;
        }
    }

    return true;
}

auto filter_root_path(const config::TaskConfig& config) -> std::filesystem::path {
    namespace fs = std::filesystem;

    if(!config.workspace_root.empty()) {
        return fs::path(config.workspace_root).lexically_normal();
    }
    return fs::path(config.project_root).lexically_normal();
}

}  // namespace clore::extract
