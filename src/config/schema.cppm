export module config:schema;

import std;

export namespace clore::config {

struct SymbolFilterConfig {
    bool hide_private = false;
    bool hide_protected = false;
    std::vector<std::string> exclude_name_prefixes;
    std::vector<std::string> exclude_namespace_segments;
};

struct FilterRule {
    std::vector<std::string> include;
    std::vector<std::string> exclude;
    std::optional<SymbolFilterConfig> symbols;
};

struct LLMConfig {
    std::string system_prompt;
    std::uint32_t retry_limit = 0;
};

struct ProjectConfig {
    std::optional<std::string> source_base;
};

struct TaskConfig {
    std::string compile_commands_path;
    std::string project_root;
    std::string output_root;
    std::string workspace_root;

    FilterRule filter;
    LLMConfig llm;
    ProjectConfig project;

};

}  // namespace clore::config
