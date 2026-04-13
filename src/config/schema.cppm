module;

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

export module config:schema;

export namespace clore::config {

// ── filter ──────────────────────────────────────────────────────────

struct FilterRule {
    std::vector<std::string> include;
    std::vector<std::string> exclude;
};

// ── extract ─────────────────────────────────────────────────────────

struct ExtractConfig {
    /// Maximum number of bytes captured per source snippet.
    /// Must be set explicitly — there is no built-in default.
    std::optional<std::uint32_t> max_snippet_bytes;
};

// ── page types ──────────────────────────────────────────────────────

struct PageTypesConfig {
    bool index = false;
    bool module_page = false;
    bool namespace_page = false;
    bool type_page = false;
    bool file_page = false;
    bool workflow_page = false;
};

// ── path rules ──────────────────────────────────────────────────────

struct PathRulesConfig {
    std::string index_path;
    std::string module_prefix;
    std::string namespace_prefix;
    std::string type_prefix;
    std::string file_prefix;
    std::string workflow_prefix;
    std::string name_normalize;
};

// ── prompt templates ────────────────────────────────────────────────

struct PromptTemplatesConfig {
    std::string type_overview;
    std::string type_usage_notes;
    std::string namespace_summary;
    std::string module_summary;
    std::string module_architecture;
    std::string index_overview;
    std::string index_reading_guide;
    std::string workflow;
};

// ── page templates ──────────────────────────────────────────────────

struct PageTemplatesConfig {
    std::string index;
    std::string module_page;
    std::string namespace_page;
    std::string type_page;
    std::string file_page;
    std::string workflow_page;
};

// ── evidence rules ──────────────────────────────────────────────────

struct EvidenceRulesConfig {
    std::uint32_t max_callers = 0;
    std::uint32_t max_callees = 0;
    std::uint32_t max_siblings = 0;
    std::uint32_t max_source_bytes = 0;
    std::uint32_t max_related_summaries = 0;
};

// ── workflow rules ──────────────────────────────────────────────────

struct WorkflowRulesConfig {
    std::uint32_t min_chain_symbols = 0;
    std::uint32_t min_new_symbols = 0;
    std::uint32_t max_symbol_overlap_ratio_percent = 0;
    std::uint32_t max_workflow_pages = 0;
    std::uint32_t llm_review_top_k = 0;
    std::uint32_t llm_selected_count = 0;
};

// ── llm ─────────────────────────────────────────────────────────────

struct LLMConfig {
    std::string system_prompt;
    std::uint32_t retry_count = 0;
    std::uint32_t retry_initial_backoff_ms = 0;
};

// ── validation ──────────────────────────────────────────────────────

struct ValidationConfig {
    bool fail_on_empty_section = false;
    bool fail_on_h1_in_output = false;
};

// ── navigation ──────────────────────────────────────────────────────

struct NavigationConfig {
    bool consume_dependency_summaries = false;
};

// ── builtin format ──────────────────────────────────────────────────

struct BuiltinConfig {
    bool vitepress = false;
};

// ── section order ───────────────────────────────────────────────────

struct SectionOrderConfig {
    std::vector<std::string> type_page;
    std::vector<std::string> namespace_page;
    std::vector<std::string> module_page;
    std::vector<std::string> index_page;
    std::vector<std::string> file_page;
};

// ── top-level config ────────────────────────────────────────────────

struct TaskConfig {
    std::string compile_commands_path;
    std::string project_root;
    std::string output_root;

    /// Base directory for config-relative assets and filter patterns.
    std::string workspace_root;

    FilterRule filter;
    ExtractConfig extract;

    PageTypesConfig page_types;
    PathRulesConfig path_rules;
    PromptTemplatesConfig prompt_templates;
    PageTemplatesConfig page_templates;
    EvidenceRulesConfig evidence_rules;
    WorkflowRulesConfig workflow_rules;
    LLMConfig llm;
    ValidationConfig validation;
    NavigationConfig navigation;
    SectionOrderConfig section_order;
    BuiltinConfig builtin;

    std::optional<std::string> log_level;
};

}  // namespace clore::config
