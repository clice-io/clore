export module config:schema;

import std;

export namespace clore::config {

struct FilterRule {
    std::vector<std::string> include;
    std::vector<std::string> exclude;
};

struct LLMConfig {
    std::string system_prompt;
    std::uint32_t retry_limit = 0;
};

struct TaskConfig {
    std::string compile_commands_path;
    std::string project_root;
    std::string output_root;
    std::string workspace_root;

    FilterRule filter;
    LLMConfig llm;

};

}  // namespace clore::config
