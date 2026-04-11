module;

#include <expected>
#include <filesystem>
#include <format>
#include <optional>
#include <string>

export module config:normalize;

import :schema;

export namespace clore::config {

struct NormalizeError {
    std::string message;
};

auto normalize(TaskConfig& config) -> std::expected<void, NormalizeError>;

}  // namespace clore::config

// ── implementation ──────────────────────────────────────────────────

namespace clore::config {

auto normalize(TaskConfig& config) -> std::expected<void, NormalizeError> {
    namespace fs = std::filesystem;

    auto make_absolute = [](std::string& path,
                            std::string_view field,
                            const std::optional<fs::path>& base = std::nullopt)
        -> std::expected<void, NormalizeError> {
        if(path.empty()) {
            return std::unexpected(
                NormalizeError{.message = std::format("{} must not be empty", field)});
        }
        auto p = fs::path(path);
        if(p.is_relative()) {
            p = base.has_value() ? (*base / p) : fs::absolute(p);
        }
        path = p.lexically_normal().string();
        return {};
    };

    if(config.workspace_root.empty()) {
        config.workspace_root = fs::current_path().string();
    }
    if(auto r = make_absolute(config.workspace_root, "workspace_root"); !r.has_value()) {
        return r;
    }
    auto workspace_root = fs::path(config.workspace_root);

    if(auto r = make_absolute(config.compile_commands_path, "compile_commands_path");
       !r.has_value()) {
        return r;
    }
    if(auto r = make_absolute(config.project_root, "project_root"); !r.has_value()) {
        return r;
    }
    if(auto r = make_absolute(config.output_root, "output_root"); !r.has_value()) {
        return r;
    }

    auto make_absolute_opt = [&](std::optional<std::string>& opt,
                                 std::string_view field,
                                 const std::optional<fs::path>& base = std::nullopt)
        -> std::expected<void, NormalizeError> {
        if(opt.has_value()) {
            return make_absolute(*opt, field, base);
        }
        return {};
    };

    if(auto r = make_absolute_opt(config.frontmatter.template_path,
                                  "frontmatter.template_path",
                                  workspace_root);
       !r.has_value()) {
        return r;
    }

    auto normalize_separators = [](std::string& path) {
        for(auto& c : path) {
            if(c == '\\') {
                c = '/';
            }
        }
    };

    normalize_separators(config.compile_commands_path);
    normalize_separators(config.project_root);
    normalize_separators(config.output_root);
    normalize_separators(config.workspace_root);
    for(auto& p : config.filter.include) normalize_separators(p);
    for(auto& p : config.filter.exclude) normalize_separators(p);
    if(config.frontmatter.template_path.has_value()) {
        normalize_separators(*config.frontmatter.template_path);
    }

    return {};
}

}  // namespace clore::config
