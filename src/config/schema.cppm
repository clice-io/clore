module;

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

export module config:schema;

export namespace clore::config {

struct FilterRule {
    std::vector<std::string> include;
    std::vector<std::string> exclude;
};

struct ExtractConfig {
    std::optional<std::uint32_t> max_snippet_bytes;
};

struct EvidenceRulesConfig {
    std::uint32_t max_callers = 0;
    std::uint32_t max_callees = 0;
    std::uint32_t max_siblings = 0;
    std::uint32_t max_source_bytes = 0;
    std::uint32_t max_related_summaries = 0;
    std::uint32_t max_top_modules = 0;
    std::uint32_t max_top_namespaces = 0;
};

struct LLMConfig {
    std::string system_prompt;
    std::uint32_t retry_count = 0;
    std::uint32_t retry_initial_backoff_ms = 0;
};

struct TaskConfig {
    std::string compile_commands_path;
    std::string project_root;
    std::string output_root;
    std::string workspace_root;

    FilterRule filter;
    ExtractConfig extract;
    EvidenceRulesConfig evidence_rules;
    LLMConfig llm;

    std::optional<std::string> log_level;
};

}  // namespace clore::config
