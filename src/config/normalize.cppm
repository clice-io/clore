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

    // Normalize prompt template paths (relative to workspace_root)
    auto make_absolute_if_nonempty = [&](std::string& path, std::string_view field)
        -> std::expected<void, NormalizeError> {
        if(!path.empty()) {
            return make_absolute(path, field, workspace_root);
        }
        return {};
    };

    if(auto r = make_absolute_if_nonempty(config.prompt_templates.type_overview, "prompt_templates.type_overview"); !r) return r;
    if(auto r = make_absolute_if_nonempty(config.prompt_templates.type_usage_notes, "prompt_templates.type_usage_notes"); !r) return r;
    if(auto r = make_absolute_if_nonempty(config.prompt_templates.namespace_summary, "prompt_templates.namespace_summary"); !r) return r;
    if(auto r = make_absolute_if_nonempty(config.prompt_templates.module_summary, "prompt_templates.module_summary"); !r) return r;
    if(auto r = make_absolute_if_nonempty(config.prompt_templates.module_architecture, "prompt_templates.module_architecture"); !r) return r;
    if(auto r = make_absolute_if_nonempty(config.prompt_templates.repository_overview, "prompt_templates.repository_overview"); !r) return r;
    if(auto r = make_absolute_if_nonempty(config.prompt_templates.reading_guide, "prompt_templates.reading_guide"); !r) return r;

    // Normalize page template paths
    if(auto r = make_absolute_if_nonempty(config.page_templates.repository, "page_templates.repository"); !r) return r;
    if(auto r = make_absolute_if_nonempty(config.page_templates.index, "page_templates.index"); !r) return r;
    if(auto r = make_absolute_if_nonempty(config.page_templates.module_page, "page_templates.module_page"); !r) return r;
    if(auto r = make_absolute_if_nonempty(config.page_templates.namespace_page, "page_templates.namespace_page"); !r) return r;
    if(auto r = make_absolute_if_nonempty(config.page_templates.type_page, "page_templates.type_page"); !r) return r;
    if(auto r = make_absolute_if_nonempty(config.page_templates.file_page, "page_templates.file_page"); !r) return r;

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
    normalize_separators(config.prompt_templates.type_overview);
    normalize_separators(config.prompt_templates.type_usage_notes);
    normalize_separators(config.prompt_templates.namespace_summary);
    normalize_separators(config.prompt_templates.module_summary);
    normalize_separators(config.prompt_templates.module_architecture);
    normalize_separators(config.prompt_templates.repository_overview);
    normalize_separators(config.prompt_templates.reading_guide);
    normalize_separators(config.page_templates.repository);
    normalize_separators(config.page_templates.index);
    normalize_separators(config.page_templates.module_page);
    normalize_separators(config.page_templates.namespace_page);
    normalize_separators(config.page_templates.type_page);
    normalize_separators(config.page_templates.file_page);

    return {};
}

}  // namespace clore::config
