#include "config/normalize.h"

#include <filesystem>
#include <format>

namespace clore::config {

auto normalize(TaskConfig& config) -> std::expected<void, NormalizeError> {
    namespace fs = std::filesystem;

    // Reject empty required path fields before any filesystem operations.
    // fs::absolute("") silently resolves to cwd, which would bypass validation.
    auto make_absolute = [](std::string& path,
                            std::string_view field) -> std::expected<void, NormalizeError> {
        if(path.empty()) {
            return std::unexpected(
                NormalizeError{.message = std::format("{} must not be empty", field)});
        }
        auto p = fs::path(path);
        if(p.is_relative()) {
            path = fs::absolute(p).string();
        }
        path = fs::path(path).lexically_normal().string();
        return {};
    };

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
                                 std::string_view field) -> std::expected<void, NormalizeError> {
        if(opt.has_value()) {
            return make_absolute(*opt, field);
        }
        return {};
    };

    if(auto r = make_absolute_opt(config.frontmatter.template_path, "frontmatter.template_path");
       !r.has_value()) {
        return r;
    }

    // Normalize path separators to forward slashes.
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
    for(auto& p : config.filter.include) normalize_separators(p);
    for(auto& p : config.filter.exclude) normalize_separators(p);

    return {};
}

}  // namespace clore::config
