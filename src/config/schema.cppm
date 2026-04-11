module;

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

export module clore.config:schema;

export namespace clore::config {

struct FrontmatterField {
    std::string key;
    std::string value;
};

struct FrontmatterConfig {
    std::vector<FrontmatterField> fields;
    std::optional<std::string> template_path;
};

struct PageRule {
    std::string pattern;
    std::string layout;
};

struct FilterRule {
    std::vector<std::string> include;
    std::vector<std::string> exclude;
};

struct ExtractConfig {
    /// Maximum number of bytes captured per source snippet.
    /// Must be set explicitly — there is no built-in default.
    std::optional<std::uint32_t> max_snippet_bytes;
};

struct TaskConfig {
    std::string compile_commands_path;
    std::string project_root;
    std::string output_root;

    /// Base directory for config-relative assets and filter patterns.
    /// Defaults to the current working directory when no config file is loaded.
    std::string workspace_root;

    FilterRule filter;
    ExtractConfig extract;

    FrontmatterConfig frontmatter;

    std::vector<PageRule> page_rules;

    /// Output language hint passed through to the LLM prompt verbatim.
    /// Optional; defaults to English when omitted.
    std::optional<std::string> language;

    std::optional<std::string> log_level;
};

}  // namespace clore::config
