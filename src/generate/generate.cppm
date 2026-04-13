module;

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <expected>
#include <format>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "eventide/serde/json/json.h"

export module generate;

export import :model;
export import :path;
export import :evidence;
export import :prompt;
export import :render;
export import :planner;

import config;
import extract;
import network;
import support;

export namespace clore::generate {

auto generate_dry_run(const config::TaskConfig& config, const extract::ProjectModel& model)
    -> std::expected<std::vector<GeneratedPage>, GenerateError>;

/// Non-blocking page generation with structured concurrency.
/// Pages are written to output_root immediately when all their LLM prompts complete.
/// Returns the number of pages written.
auto generate_pages(const config::TaskConfig& config, const extract::ProjectModel& model,
                    std::string_view llm_model,
                    std::uint32_t rate_limit,
                    std::string_view output_root)
    -> std::expected<std::size_t, GenerateError>;

auto write_pages(const std::vector<GeneratedPage>& pages, std::string_view output_root)
    -> std::expected<void, GenerateError>;

}  // namespace clore::generate

namespace clore::generate {

namespace {

struct WorkflowReviewCandidate {
    std::size_t plan_index = 0;
    std::string page_id;
    std::string current_title;
    std::vector<std::string> symbols;
    std::vector<std::pair<std::string, std::string>> edges;
    std::vector<std::string> namespaces;
    std::vector<std::string> files;
    std::string entry_symbol;
    std::string exit_symbol;
    std::size_t chain_length = 0;
    std::size_t rank_score = 0;
};

struct PageState {
    std::size_t plan_index = 0;
    std::size_t total_prompts = 0;
    std::size_t completed_prompts = 0;
    std::size_t unsatisfied_deps = 0;
    bool submitted = false;
    bool written = false;
    std::unordered_map<std::string, std::string> prompt_outputs;
};

struct TagInfo {
    std::size_t state_index = 0;
    std::string output_key;
};

auto trim_ascii(std::string_view text) -> std::string_view {
    while(!text.empty() && std::isspace(static_cast<unsigned char>(text.front())) != 0) {
        text.remove_prefix(1);
    }
    while(!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
        text.remove_suffix(1);
    }
    return text;
}

auto collapse_whitespace(std::string_view text) -> std::string {
    std::string out;
    out.reserve(text.size());
    bool in_space = false;
    for(auto ch : text) {
        if(std::isspace(static_cast<unsigned char>(ch)) != 0) {
            if(!in_space) {
                out.push_back(' ');
                in_space = true;
            }
            continue;
        }
        out.push_back(ch);
        in_space = false;
    }
    return std::string(trim_ascii(out));
}

auto namespace_of_qname(std::string_view qname) -> std::string {
    auto pos = qname.rfind("::");
    if(pos == std::string_view::npos) {
        return {};
    }
    return std::string(qname.substr(0, pos));
}

auto json_escape(std::string_view text) -> std::string {
    std::string out;
    out.reserve(text.size() + 8);
    for(auto ch : text) {
        switch(ch) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out.push_back(ch); break;
        }
    }
    return out;
}

auto build_json_string_array(const std::vector<std::string>& values) -> std::string {
    std::string out = "[";
    for(std::size_t i = 0; i < values.size(); ++i) {
        if(i > 0) {
            out += ", ";
        }
        out += "\"";
        out += json_escape(values[i]);
        out += "\"";
    }
    out += "]";
    return out;
}

auto deduplicate_prompt_requests(const PagePlan& plan) -> std::vector<PromptRequest> {
    std::vector<PromptRequest> unique;
    unique.reserve(plan.prompt_requests.size());
    std::unordered_set<std::string> seen;
    seen.reserve(plan.prompt_requests.size());
    for(const auto& request : plan.prompt_requests) {
        auto key = prompt_request_key(request);
        if(seen.insert(key).second) {
            unique.push_back(request);
        }
    }
    return unique;
}

auto set_evidence_metadata(EvidencePack pack,
                           const PagePlan& plan,
                           const PromptRequest& request) -> EvidencePack {
    pack.page_id = plan.page_id;
    pack.prompt_kind = std::string(prompt_kind_name(request.kind));
    if(pack.subject_name.empty()) {
        if(!request.target_key.empty()) {
            pack.subject_name = request.target_key;
        } else if(!plan.owner_keys.empty()) {
            pack.subject_name = plan.owner_keys.front();
        }
    }
    return pack;
}

auto build_evidence_for_request(const PromptRequest& request,
                                const PagePlan& plan,
                                const extract::ProjectModel& model,
                                const config::TaskConfig& config,
                                const PageSummaryCache& summaries) -> EvidencePack {
    const auto& rules = config.evidence_rules;

    switch(request.kind) {
        case PromptKind::NamespaceSummary:
            if(!plan.owner_keys.empty()) {
                auto ns_it = model.namespaces.find(plan.owner_keys.front());
                if(ns_it != model.namespaces.end()) {
                    return set_evidence_metadata(
                        build_evidence_for_namespace_summary(ns_it->second, model, rules,
                                                             summaries, config.project_root),
                        plan, request);
                }
            }
            break;
        case PromptKind::ModuleSummary:
            if(!plan.owner_keys.empty()) {
                if(auto* mod = extract::find_module_by_name(model, plan.owner_keys.front())) {
                    return set_evidence_metadata(
                        build_evidence_for_module_summary(*mod, model, rules, summaries,
                                                          config.project_root),
                        plan, request);
                }
            }
            break;
        case PromptKind::ModuleArchitecture:
            if(!plan.owner_keys.empty()) {
                if(auto* mod = extract::find_module_by_name(model, plan.owner_keys.front())) {
                    return set_evidence_metadata(
                        build_evidence_for_module_architecture(*mod, model, rules, summaries,
                                                               config.project_root),
                        plan, request);
                }
            }
            break;
        case PromptKind::IndexOverview:
        case PromptKind::IndexReadingGuide:
            return set_evidence_metadata(
                build_evidence_for_index_overview(model, rules, summaries), plan, request);
        case PromptKind::Workflow: {
            auto pack = build_evidence_for_workflow(plan.owner_keys, model, rules, summaries,
                                                    config.project_root);
            auto title_view = std::string_view(plan.title);
            constexpr std::string_view workflow_prefix = "Workflow: ";
            if(title_view.starts_with(workflow_prefix)) {
                title_view.remove_prefix(workflow_prefix.size());
            }
            pack.subject_name = std::string(title_view);
            return set_evidence_metadata(std::move(pack), plan, request);
        }
        case PromptKind::FunctionDeclarationSummary:
            if(!request.target_key.empty()) {
                if(auto* sym = find_sym(model, request.target_key)) {
                    return set_evidence_metadata(
                        build_evidence_for_function_declaration_summary(
                            *sym, model, rules, summaries, config.project_root),
                        plan, request);
                }
            }
            break;
        case PromptKind::FunctionImplementationSummary:
            if(!request.target_key.empty()) {
                if(auto* sym = find_sym(model, request.target_key)) {
                    return set_evidence_metadata(
                        build_evidence_for_function_implementation_summary(
                            *sym, model, rules, config.project_root),
                        plan, request);
                }
            }
            break;
        case PromptKind::TypeDeclarationSummary:
            if(!request.target_key.empty()) {
                if(auto* sym = find_sym(model, request.target_key)) {
                    return set_evidence_metadata(
                        build_evidence_for_type_declaration_summary(
                            *sym, model, rules, summaries, config.project_root),
                        plan, request);
                }
            }
            break;
        case PromptKind::TypeImplementationSummary:
            if(!request.target_key.empty()) {
                if(auto* sym = find_sym(model, request.target_key)) {
                    return set_evidence_metadata(
                        build_evidence_for_type_implementation_summary(
                            *sym, model, rules, config.project_root),
                        plan, request);
                }
            }
            break;
    }

    return set_evidence_metadata(EvidencePack{}, plan, request);
}

auto wrap_prompt_output_for_embed(std::string_view request_key, std::string_view prompt)
    -> std::string {
    std::string wrapped;
    wrapped.reserve(request_key.size() + prompt.size() + 32);
    wrapped += "> Prompt (`";
    wrapped += request_key;
    wrapped += "`)\n";

    std::istringstream stream{std::string(prompt)};
    std::string line;
    bool has_line = false;
    while(std::getline(stream, line)) {
        wrapped += "> ";
        wrapped += line;
        wrapped += "\n";
        has_line = true;
    }

    if(!has_line) {
        wrapped += "> \n";
    }

    return wrapped;
}

auto extract_summary_from_prompt_output(const std::string& output) -> std::string {
    auto end = output.find("\n\n");
    if(end != std::string::npos) {
        return clore::support::ensure_utf8(output.substr(0, end));
    }
    if(output.size() > 300) {
        return clore::support::truncate_utf8(output, 300);
    }
    return clore::support::ensure_utf8(output);
}

auto extract_workflow_title(const std::string& output)
    -> std::optional<std::pair<std::string, std::string>> {
    constexpr std::string_view prefix = "Title: ";
    if(!output.starts_with(prefix)) {
        return std::nullopt;
    }

    auto nl = output.find('\n');
    if(nl == std::string::npos) {
        return std::nullopt;
    }

    auto title = output.substr(prefix.size(), nl - prefix.size());
    while(!title.empty() && (title.back() == ' ' || title.back() == '\r')) {
        title.pop_back();
    }
    if(title.empty()) {
        return std::nullopt;
    }

    auto rest_start = nl + 1;
    while(rest_start < output.size() &&
          (output[rest_start] == '\n' || output[rest_start] == '\r')) {
        ++rest_start;
    }

    if(trim_ascii(output.substr(rest_start)).empty()) {
        return std::nullopt;
    }

    return std::pair{std::move(title), output.substr(rest_start)};
}

auto workflow_output_key() -> std::string {
    return prompt_request_key(PromptRequest{.kind = PromptKind::Workflow});
}

auto render_generated_page(const PagePlan& plan,
                           const config::TaskConfig& config,
                           const extract::ProjectModel& model,
                           const std::unordered_map<std::string, std::string>& prompt_outputs,
                           const LinkResolver& links)
    -> std::expected<GeneratedPage, GenerateError> {
    PagePlan adjusted_plan = plan;
    std::unordered_map<std::string, std::string> adjusted_outputs;
    const auto* effective_outputs = &prompt_outputs;

    if(plan.page_type == PageType::Workflow) {
        auto workflow_key = workflow_output_key();
        auto it = prompt_outputs.find(workflow_key);
        if(it != prompt_outputs.end()) {
            if(auto extracted = extract_workflow_title(it->second)) {
                adjusted_plan.title = extracted->first;
                adjusted_outputs = prompt_outputs;
                adjusted_outputs[workflow_key] = std::move(extracted->second);
                effective_outputs = &adjusted_outputs;
            }
        }
    }

    auto render_result = render_page_markdown(adjusted_plan, config, model,
                                              *effective_outputs, links);
    if(!render_result.has_value()) {
        return std::unexpected(GenerateError{
            .message = std::format("failed to render page '{}': {}",
                                   adjusted_plan.page_id,
                                   render_result.error().message),
        });
    }

    return GeneratedPage{
        .relative_path = adjusted_plan.relative_path,
        .content = std::move(*render_result),
    };
}

auto update_summary_cache(PageSummaryCache& summaries,
                          const PagePlan& plan,
                          const std::vector<PromptRequest>& prompt_requests,
                          const std::unordered_map<std::string, std::string>& prompt_outputs)
    -> void {
    if(plan.owner_keys.empty()) {
        return;
    }

    for(const auto& request : prompt_requests) {
        if(!is_page_summary_prompt(request.kind)) {
            continue;
        }
        auto key = prompt_request_key(request);
        auto it = prompt_outputs.find(key);
        if(it == prompt_outputs.end()) {
            continue;
        }
        auto summary = extract_summary_from_prompt_output(it->second);
        if(summary.empty()) {
            continue;
        }
        summaries[plan.owner_keys.front()] = std::move(summary);
        return;
    }
}

auto build_workflow_review_candidates(const PagePlanSet& plan_set,
                                      const extract::ProjectModel& model,
                                      const config::TaskConfig& config)
    -> std::vector<WorkflowReviewCandidate> {
    std::vector<WorkflowReviewCandidate> candidates;
    for(std::size_t i = 0; i < plan_set.plans.size(); ++i) {
        const auto& plan = plan_set.plans[i];
        if(plan.page_type != PageType::Workflow || plan.owner_keys.size() < 2) {
            continue;
        }

        WorkflowReviewCandidate candidate;
        candidate.plan_index = i;
        candidate.page_id = plan.page_id;
        candidate.current_title = plan.title;
        candidate.symbols = plan.owner_keys;
        candidate.entry_symbol = plan.owner_keys.front();
        candidate.exit_symbol = plan.owner_keys.back();
        candidate.chain_length = plan.owner_keys.size();

        std::unordered_set<std::string> seen_namespaces;
        std::unordered_set<std::string> seen_files;
        for(std::size_t j = 0; j < plan.owner_keys.size(); ++j) {
            const auto& qname = plan.owner_keys[j];
            if(j + 1 < plan.owner_keys.size()) {
                candidate.edges.emplace_back(qname, plan.owner_keys[j + 1]);
            }

            auto* sym = find_sym(model, qname);
            if(sym == nullptr) {
                continue;
            }

            auto ns = sym->enclosing_namespace.empty() ? namespace_of_qname(sym->qualified_name)
                                                       : sym->enclosing_namespace;
            if(!ns.empty() && seen_namespaces.insert(ns).second) {
                candidate.namespaces.push_back(ns);
            }

            auto rel_file = make_source_relative(sym->declaration_location.file,
                                                 config.project_root);
            if(!rel_file.empty() && seen_files.insert(rel_file).second) {
                candidate.files.push_back(rel_file);
            }
        }

        candidate.rank_score = candidate.chain_length * 1000 +
                               candidate.files.size() * 100 +
                               candidate.namespaces.size() * 10;
        candidates.push_back(std::move(candidate));
    }

    std::sort(candidates.begin(), candidates.end(),
              [](const WorkflowReviewCandidate& lhs,
                 const WorkflowReviewCandidate& rhs) {
                  if(lhs.rank_score != rhs.rank_score) {
                      return lhs.rank_score > rhs.rank_score;
                  }
                  if(lhs.chain_length != rhs.chain_length) {
                      return lhs.chain_length > rhs.chain_length;
                  }
                  return lhs.page_id < rhs.page_id;
              });

    return candidates;
}

auto build_workflow_review_payload(const std::vector<WorkflowReviewCandidate>& candidates)
    -> std::string {
    std::string json = "{\n  \"candidates\": [\n";
    for(std::size_t i = 0; i < candidates.size(); ++i) {
        const auto& candidate = candidates[i];
        json += "    {\n";
        json += "      \"page_id\": \"" + json_escape(candidate.page_id) + "\",\n";
        json += "      \"current_title\": \"" + json_escape(candidate.current_title) + "\",\n";
        json += "      \"entry_symbol\": \"" + json_escape(candidate.entry_symbol) + "\",\n";
        json += "      \"exit_symbol\": \"" + json_escape(candidate.exit_symbol) + "\",\n";
        json += "      \"chain_length\": " + std::to_string(candidate.chain_length) + ",\n";
        json += "      \"symbols\": " + build_json_string_array(candidate.symbols) + ",\n";

        std::string edge_json = "[";
        for(std::size_t edge_index = 0; edge_index < candidate.edges.size(); ++edge_index) {
            const auto& edge = candidate.edges[edge_index];
            if(edge_index > 0) {
                edge_json += ", ";
            }
            edge_json += "{\"from\":\"" + json_escape(edge.first) + "\",\"to\":\"" +
                         json_escape(edge.second) + "\"}";
        }
        edge_json += "]";
        json += "      \"edges\": " + edge_json + ",\n";
        json += "      \"namespaces\": " + build_json_string_array(candidate.namespaces) + ",\n";
        json += "      \"files\": " + build_json_string_array(candidate.files) + "\n";
        json += "    }";
        if(i + 1 < candidates.size()) {
            json += ",";
        }
        json += "\n";
    }
    json += "  ]\n}";
    return json;
}

auto build_workflow_review_prompt(const std::vector<WorkflowReviewCandidate>& candidates,
                                  std::size_t selected_count) -> std::string {
    auto payload = build_workflow_review_payload(candidates);
    std::string prompt;
    prompt.reserve(payload.size() + 2048);
    prompt += "Review workflow candidates extracted from a function-level call graph.\n";
    prompt += std::format(
        "Select exactly {} workflows that are major and mutually distinct.\n",
        selected_count);
    prompt += "You must reason only from structured semantics.\n";
    prompt += "Input JSON:\n```json\n";
    prompt += payload;
    prompt += "\n```\n\n";
    prompt += "Return ONLY JSON with this exact schema:\n";
    prompt += "{\n";
    prompt += "  \"selected\": [\n";
    prompt += "    {\"page_id\": \"workflow:...\", \"title\": \"...\"}\n";
    prompt += "  ]\n";
    prompt += "}\n";
    prompt += "Rules:\n";
    prompt += std::format("- `selected` length must be exactly {}\n", selected_count);
    prompt += "- `page_id` must be chosen from the input candidates\n";
    prompt += "- no duplicate `page_id`\n";
    prompt += "- title must be concise and descriptive\n";
    prompt += "- no extra keys, no markdown, no explanation\n";
    return prompt;
}

auto append_json_candidate(std::vector<std::string>& candidates,
                           std::unordered_set<std::string>& dedupe,
                           std::string_view raw) -> void {
    auto trimmed = trim_ascii(raw);
    if(trimmed.empty()) {
        return;
    }

    auto candidate = std::string(trimmed);
    if(dedupe.emplace(candidate).second) {
        candidates.push_back(std::move(candidate));
    }
}

auto collect_workflow_review_json_candidates(std::string_view response)
    -> std::vector<std::string> {
    std::vector<std::string> candidates;
    std::unordered_set<std::string> dedupe;
    append_json_candidate(candidates, dedupe, response);

    std::size_t fence_search = 0;
    while(true) {
        auto fence_start = response.find("```", fence_search);
        if(fence_start == std::string_view::npos) {
            break;
        }

        auto header_end = response.find('\n', fence_start + 3);
        if(header_end == std::string_view::npos) {
            break;
        }

        auto header = trim_ascii(response.substr(
            fence_start + 3, header_end - (fence_start + 3)));

        auto fence_end = response.find("```", header_end + 1);
        if(fence_end == std::string_view::npos) {
            break;
        }

        auto is_json_fence = header.empty() || header == "json" || header == "JSON";
        if(is_json_fence) {
            append_json_candidate(candidates, dedupe,
                                  response.substr(header_end + 1,
                                                  fence_end - (header_end + 1)));
        }
        fence_search = fence_end + 3;
    }

    for(std::size_t start = 0; start < response.size(); ++start) {
        if(response[start] != '{') {
            continue;
        }

        std::size_t depth = 0;
        bool in_string = false;
        bool escaped = false;

        for(std::size_t index = start; index < response.size(); ++index) {
            auto ch = response[index];
            if(in_string) {
                if(escaped) {
                    escaped = false;
                    continue;
                }
                if(ch == '\\') {
                    escaped = true;
                    continue;
                }
                if(ch == '"') {
                    in_string = false;
                }
                continue;
            }

            if(ch == '"') {
                in_string = true;
                continue;
            }
            if(ch == '{') {
                ++depth;
                continue;
            }
            if(ch == '}') {
                if(depth == 0) {
                    break;
                }
                --depth;
                if(depth == 0) {
                    append_json_candidate(candidates, dedupe,
                                          response.substr(start, index - start + 1));
                    break;
                }
            }
        }
    }

    return candidates;
}

auto parse_workflow_review_object(const eventide::serde::json::Object& parsed,
                                  const std::unordered_set<std::string>& allowed_page_ids,
                                  std::size_t selected_count)
    -> std::expected<std::unordered_map<std::string, std::string>, GenerateError> {
    auto selected_value = parsed.get("selected");
    if(!selected_value.has_value()) {
        return std::unexpected(GenerateError{
            .message = "workflow review JSON missing 'selected'",
        });
    }

    auto selected_array = selected_value->get_array();
    if(!selected_array.has_value()) {
        return std::unexpected(GenerateError{
            .message = "workflow review 'selected' must be an array",
        });
    }

    if(selected_array->size() != selected_count) {
        return std::unexpected(GenerateError{
            .message = std::format("workflow review returned {} selections, expected {}",
                                   selected_array->size(), selected_count),
        });
    }

    std::unordered_map<std::string, std::string> selected_titles;
    selected_titles.reserve(selected_array->size());

    for(auto entry : *selected_array) {
        auto entry_object = entry.get_object();
        if(!entry_object.has_value()) {
            return std::unexpected(GenerateError{
                .message = "workflow review selection entry must be an object",
            });
        }

        auto page_id_value = entry_object->get("page_id");
        auto title_value = entry_object->get("title");
        if(!page_id_value.has_value() || !title_value.has_value()) {
            return std::unexpected(GenerateError{
                .message = "workflow review selection entry missing 'page_id' or 'title'",
            });
        }

        auto page_id = page_id_value->get_string();
        auto title = title_value->get_string();
        if(!page_id.has_value() || !title.has_value()) {
            return std::unexpected(GenerateError{
                .message = "workflow review 'page_id' and 'title' must be strings",
            });
        }

        auto page_id_text = std::string(*page_id);
        auto title_text = collapse_whitespace(*title);
        if(title_text.empty()) {
            return std::unexpected(GenerateError{
                .message = std::format("workflow review returned empty title for '{}'",
                                       page_id_text),
            });
        }
        if(!allowed_page_ids.contains(page_id_text)) {
            return std::unexpected(GenerateError{
                .message = std::format("workflow review returned unknown page_id '{}'",
                                       page_id_text),
            });
        }
        if(!selected_titles.emplace(page_id_text, title_text).second) {
            return std::unexpected(GenerateError{
                .message = std::format("workflow review returned duplicate page_id '{}'",
                                       page_id_text),
            });
        }
    }

    return selected_titles;
}

auto parse_workflow_review_response(std::string_view response,
                                    const std::unordered_set<std::string>& allowed_page_ids,
                                    std::size_t selected_count)
    -> std::expected<std::unordered_map<std::string, std::string>, GenerateError> {
    namespace json = eventide::serde::json;

    auto candidates = collect_workflow_review_json_candidates(response);
    if(candidates.empty()) {
        return std::unexpected(GenerateError{
            .message = "workflow review returned invalid JSON: empty response",
        });
    }

    std::optional<GenerateError> last_validation_error;
    std::string last_parse_error = "unknown JSON parse error";

    for(const auto& candidate : candidates) {
        auto parsed = json::Object::parse(candidate);
        if(!parsed.has_value()) {
            last_parse_error = json::error_message(json::make_read_error(parsed.error()));
            continue;
        }

        auto validation_result =
            parse_workflow_review_object(*parsed, allowed_page_ids, selected_count);
        if(validation_result.has_value()) {
            return validation_result;
        }
        last_validation_error = std::move(validation_result.error());
    }

    if(last_validation_error.has_value()) {
        return std::unexpected(std::move(*last_validation_error));
    }

    return std::unexpected(GenerateError{
        .message = std::format("workflow review returned invalid JSON: {}",
                               last_parse_error),
    });
}

auto build_workflow_review_calibration_prompt(
    const std::vector<WorkflowReviewCandidate>& candidates,
    std::size_t selected_count,
    std::string_view previous_output,
    std::string_view validation_error) -> std::string {
    auto payload = build_workflow_review_payload(candidates);
    std::string prompt;
    prompt.reserve(payload.size() + previous_output.size() + validation_error.size() +
                   3072);
    prompt += "Fix the workflow review JSON so it strictly matches the required schema.\n";
    prompt += "Do not explain. Do not wrap with markdown.\n";
    prompt += std::format("Validation error:\n{}\n\n", validation_error);
    prompt += "Original invalid output:\n```text\n";
    prompt += std::string(previous_output);
    prompt += "\n```\n\n";
    prompt += "Input candidates JSON:\n```json\n";
    prompt += payload;
    prompt += "\n```\n\n";
    prompt += "Return ONLY JSON with this exact schema:\n";
    prompt += "{\n";
    prompt += "  \"selected\": [\n";
    prompt += "    {\"page_id\": \"workflow:...\", \"title\": \"...\"}\n";
    prompt += "  ]\n";
    prompt += "}\n";
    prompt += "Rules:\n";
    prompt += std::format("- `selected` length must be exactly {}\n", selected_count);
    prompt += "- each `page_id` must be chosen from the candidate list\n";
    prompt += "- no duplicate `page_id`\n";
    prompt += "- `title` must be non-empty, concise, and descriptive\n";
    prompt += "- no extra keys\n";
    return prompt;
}

auto build_fallback_workflow_review_selection(
    const std::vector<WorkflowReviewCandidate>& candidates,
    std::size_t selected_count) -> std::unordered_map<std::string, std::string> {
    std::unordered_map<std::string, std::string> selected_titles;
    selected_titles.reserve(selected_count);

    auto count = std::min(selected_count, candidates.size());
    for(std::size_t i = 0; i < count; ++i) {
        auto title = collapse_whitespace(candidates[i].current_title);
        if(title.empty()) {
            title = candidates[i].page_id;
        }
        selected_titles.emplace(candidates[i].page_id, std::move(title));
    }

    return selected_titles;
}

auto request_workflow_review_selection(
    const std::vector<WorkflowReviewCandidate>& candidates,
    const std::unordered_set<std::string>& allowed_page_ids,
    std::size_t selected_count,
    const config::TaskConfig& config,
    std::string_view llm_model)
    -> std::expected<std::unordered_map<std::string, std::string>, GenerateError> {
    auto fallback_titles = build_fallback_workflow_review_selection(candidates,
                                                                    selected_count);
    auto prompt = build_workflow_review_prompt(candidates, selected_count);
    auto response = clore::net::call_llm_with_retries(
        llm_model,
        config.llm.system_prompt,
        prompt,
        config.llm.retry_count,
        config.llm.retry_initial_backoff_ms);
    if(!response.has_value()) {
        logging::warn("workflow review LLM request failed; using ranked fallback: {}",
                      response.error().message);
        return fallback_titles;
    }

    auto selected_titles =
        parse_workflow_review_response(*response, allowed_page_ids, selected_count);
    if(selected_titles.has_value()) {
        return selected_titles;
    }

    auto current_error = std::move(selected_titles.error());
    auto current_output = std::move(*response);
    logging::warn("workflow review validation failed: {}", current_error.message);

    auto calibration_attempt_limit = config.llm.retry_count;
    for(std::uint32_t attempt = 0; attempt < calibration_attempt_limit; ++attempt) {
        auto calibration_prompt = build_workflow_review_calibration_prompt(
            candidates, selected_count, current_output, current_error.message);
        auto calibration = clore::net::call_llm_with_retries(
            llm_model,
            config.llm.system_prompt,
            calibration_prompt,
            config.llm.retry_count,
            config.llm.retry_initial_backoff_ms);
        if(!calibration.has_value()) {
            logging::warn("workflow review calibration request failed; using ranked fallback: {}",
                          calibration.error().message);
            return fallback_titles;
        }

        current_output = std::move(*calibration);
        auto calibrated_titles = parse_workflow_review_response(
            current_output, allowed_page_ids, selected_count);
        if(calibrated_titles.has_value()) {
            logging::info("workflow review calibration succeeded on attempt {}/{}",
                          attempt + 1, calibration_attempt_limit);
            return calibrated_titles;
        }

        current_error = std::move(calibrated_titles.error());
        logging::warn("workflow review calibration attempt {}/{} failed: {}",
                      attempt + 1, calibration_attempt_limit, current_error.message);
    }

    logging::warn(
        "workflow review output remained invalid after {} calibration attempts; using ranked fallback: {}",
        calibration_attempt_limit,
        current_error.message);
    return fallback_titles;
}

auto apply_workflow_review_selection(
    PagePlanSet& plan_set,
    const std::unordered_map<std::string, std::string>& selected_titles)
    -> std::expected<void, GenerateError> {
    if(selected_titles.empty()) {
        return std::unexpected(GenerateError{
            .message = "workflow review produced no selected workflows",
        });
    }

    std::unordered_set<std::string> selected_ids;
    selected_ids.reserve(selected_titles.size());
    for(const auto& [page_id, _] : selected_titles) {
        selected_ids.insert(page_id);
    }

    std::vector<PagePlan> filtered_plans;
    filtered_plans.reserve(plan_set.plans.size());
    for(auto& plan : plan_set.plans) {
        if(plan.page_type == PageType::Workflow) {
            auto title_it = selected_titles.find(plan.page_id);
            if(title_it == selected_titles.end()) {
                continue;
            }
            plan.title = title_it->second;
        }
        filtered_plans.push_back(std::move(plan));
    }

    std::unordered_set<std::string> kept_page_ids;
    kept_page_ids.reserve(filtered_plans.size());
    for(const auto& plan : filtered_plans) {
        kept_page_ids.insert(plan.page_id);
    }

    std::vector<std::string> filtered_order;
    filtered_order.reserve(plan_set.generation_order.size());
    for(const auto& page_id : plan_set.generation_order) {
        if(kept_page_ids.contains(page_id)) {
            filtered_order.push_back(page_id);
        }
    }

    for(auto& plan : filtered_plans) {
        std::erase_if(plan.depends_on_pages,
                      [&](const std::string& page_id) {
                          return !kept_page_ids.contains(page_id);
                      });
        std::erase_if(plan.linked_pages,
                      [&](const std::string& page_id) {
                          return page_id.starts_with("workflow:") &&
                                 !selected_ids.contains(page_id);
                      });
    }

    plan_set.plans = std::move(filtered_plans);
    plan_set.generation_order = std::move(filtered_order);
    return {};
}

auto review_workflow_pages_with_llm(PagePlanSet& plan_set,
                                    const config::TaskConfig& config,
                                    const extract::ProjectModel& model,
                                    std::string_view llm_model)
    -> std::expected<void, GenerateError> {
    auto candidates = build_workflow_review_candidates(plan_set, model, config);
    if(candidates.empty()) {
        return {};
    }

    auto review_count = std::min<std::size_t>(config.workflow_rules.llm_review_top_k,
                                              candidates.size());
    candidates.resize(review_count);

    auto selected_count = std::min<std::size_t>(config.workflow_rules.llm_selected_count,
                                                candidates.size());
    if(selected_count == 0) {
        return std::unexpected(GenerateError{
            .message = "workflow review requires at least one selectable candidate",
        });
    }

    std::unordered_set<std::string> allowed_page_ids;
    allowed_page_ids.reserve(candidates.size());
    for(const auto& candidate : candidates) {
        allowed_page_ids.insert(candidate.page_id);
    }

    auto selected_titles = request_workflow_review_selection(
        candidates, allowed_page_ids, selected_count, config, llm_model);
    if(!selected_titles.has_value()) {
        return std::unexpected(std::move(selected_titles.error()));
    }

    auto apply_result = apply_workflow_review_selection(plan_set, *selected_titles);
    if(!apply_result.has_value()) {
        return std::unexpected(std::move(apply_result.error()));
    }

    logging::info("workflow review: {} candidates, {} selected",
                  candidates.size(), selected_titles->size());
    return {};
}

}  // namespace

auto generate_dry_run(const config::TaskConfig& config, const extract::ProjectModel& model)
    -> std::expected<std::vector<GeneratedPage>, GenerateError> {
    auto plan_result = build_page_plan_set(config, model);
    if(!plan_result.has_value()) {
        return std::unexpected(GenerateError{.message = plan_result.error().message});
    }

    auto& plan_set = *plan_result;
    logging::info("page plan: {} pages, generation order size {}",
                  plan_set.plans.size(), plan_set.generation_order.size());

    std::unordered_map<std::string, std::size_t> id_to_index;
    id_to_index.reserve(plan_set.plans.size());
    for(std::size_t i = 0; i < plan_set.plans.size(); ++i) {
        id_to_index[plan_set.plans[i].page_id] = i;
    }

    PageSummaryCache summaries;
    std::vector<GeneratedPage> pages;
    pages.reserve(plan_set.generation_order.size());

    auto links = build_link_resolver(plan_set);

    for(const auto& page_id : plan_set.generation_order) {
        auto idx_it = id_to_index.find(page_id);
        if(idx_it == id_to_index.end()) {
            continue;
        }

        const auto& plan = plan_set.plans[idx_it->second];
        auto prompt_requests = deduplicate_prompt_requests(plan);
        std::unordered_map<std::string, std::string> prompt_outputs;
        prompt_outputs.reserve(prompt_requests.size());

        for(const auto& request : prompt_requests) {
            auto evidence = build_evidence_for_request(request, plan, model, config, summaries);
            auto prompt_result = build_prompt(request.kind, evidence);
            if(!prompt_result.has_value()) {
                return std::unexpected(GenerateError{
                    .message = std::format("failed to build prompt '{}' for '{}': {}",
                                           prompt_request_key(request), page_id,
                                           prompt_result.error().message),
                });
            }
            auto key = prompt_request_key(request);
            prompt_outputs.emplace(key,
                                   wrap_prompt_output_for_embed(key, *prompt_result));
        }

        auto page_result = render_generated_page(plan, config, model, prompt_outputs, links);
        if(!page_result.has_value()) {
            return std::unexpected(std::move(page_result.error()));
        }

        pages.push_back(std::move(*page_result));
    }

    return pages;
}

auto generate_pages(const config::TaskConfig& config, const extract::ProjectModel& model,
                    std::string_view llm_model,
                    std::uint32_t rate_limit,
                    std::string_view output_root)
    -> std::expected<std::size_t, GenerateError> {
    auto plan_result = build_page_plan_set(config, model);
    if(!plan_result.has_value()) {
        return std::unexpected(GenerateError{.message = plan_result.error().message});
    }

    auto& plan_set = *plan_result;
    if(auto review_result = review_workflow_pages_with_llm(plan_set, config, model, llm_model);
       !review_result.has_value()) {
        return std::unexpected(std::move(review_result.error()));
    }

    logging::info("page plan: {} pages, generation order size {}",
                  plan_set.plans.size(), plan_set.generation_order.size());

    std::unordered_map<std::string, std::size_t> id_to_plan;
    id_to_plan.reserve(plan_set.plans.size());
    for(std::size_t i = 0; i < plan_set.plans.size(); ++i) {
        id_to_plan[plan_set.plans[i].page_id] = i;
    }

    std::vector<std::vector<PromptRequest>> prompt_requests_by_plan;
    prompt_requests_by_plan.reserve(plan_set.plans.size());
    for(const auto& plan : plan_set.plans) {
        prompt_requests_by_plan.push_back(deduplicate_prompt_requests(plan));
    }

    auto links = build_link_resolver(plan_set);

    std::vector<PageState> states;
    states.reserve(plan_set.generation_order.size());
    std::unordered_map<std::string, std::size_t> id_to_state;
    id_to_state.reserve(plan_set.generation_order.size());

    for(const auto& page_id : plan_set.generation_order) {
        auto plan_it = id_to_plan.find(page_id);
        if(plan_it == id_to_plan.end()) {
            continue;
        }

        auto state_index = states.size();
        id_to_state[page_id] = state_index;
        states.push_back(PageState{
            .plan_index = plan_it->second,
            .total_prompts = prompt_requests_by_plan[plan_it->second].size(),
        });
    }

    std::unordered_map<std::string, std::vector<std::size_t>> dependents;
    for(std::size_t i = 0; i < states.size(); ++i) {
        const auto& plan = plan_set.plans[states[i].plan_index];
        for(const auto& dep_id : plan.depends_on_pages) {
            if(id_to_state.contains(dep_id)) {
                states[i].unsatisfied_deps++;
                dependents[dep_id].push_back(i);
            }
        }
    }

    PageSummaryCache summaries;
    std::size_t written_count = 0;
    std::unordered_map<std::uint64_t, TagInfo> tag_map;
    std::uint64_t next_tag = 0;
    std::optional<GenerateError> schedule_error;

    clore::net::LLMClient client(llm_model, config.llm.system_prompt, rate_limit,
                                 config.llm.retry_count,
                                 config.llm.retry_initial_backoff_ms);

    auto write_completed_page = [&](std::size_t state_idx)
        -> std::expected<void, GenerateError> {
        auto& state = states[state_idx];
        auto& plan = plan_set.plans[state.plan_index];
        const auto& prompt_requests = prompt_requests_by_plan[state.plan_index];

        if(state.prompt_outputs.size() != state.total_prompts) {
            return std::unexpected(GenerateError{
                .message = std::format("page '{}' completed with {} of {} prompt outputs",
                                       plan.page_id, state.prompt_outputs.size(),
                                       state.total_prompts),
            });
        }

        if(plan.page_type == PageType::Workflow) {
            auto workflow_key = workflow_output_key();
            auto it = state.prompt_outputs.find(workflow_key);
            if(it != state.prompt_outputs.end()) {
                if(auto extracted = extract_workflow_title(it->second)) {
                    plan.title = extracted->first;
                    links.page_id_to_title[plan.page_id] = plan.title;
                }
            }
        }

        auto page_result = render_generated_page(plan, config, model,
                                                 state.prompt_outputs, links);
        if(!page_result.has_value()) {
            return std::unexpected(std::move(page_result.error()));
        }

        auto write_result = write_page(*page_result, output_root);
        if(!write_result.has_value()) {
            return std::unexpected(GenerateError{
                .message = write_result.error().message,
            });
        }

        state.written = true;
        ++written_count;
        logging::info("  written {}", page_result->relative_path);

        update_summary_cache(summaries, plan, prompt_requests, state.prompt_outputs);

        auto dep_it = dependents.find(plan.page_id);
        if(dep_it != dependents.end()) {
            for(auto dep_state_idx : dep_it->second) {
                if(states[dep_state_idx].unsatisfied_deps > 0) {
                    states[dep_state_idx].unsatisfied_deps--;
                }
            }
        }

        return {};
    };

    auto submit_ready_pages = [&]() -> bool {
        bool submitted_any_llm = false;
        bool changed = true;

        while(changed) {
            changed = false;
            for(std::size_t i = 0; i < states.size(); ++i) {
                auto& state = states[i];
                if(state.submitted || state.unsatisfied_deps > 0) {
                    continue;
                }
                state.submitted = true;

                const auto& plan = plan_set.plans[state.plan_index];
                const auto& prompt_requests = prompt_requests_by_plan[state.plan_index];

                if(state.total_prompts == 0) {
                    auto write_result = write_completed_page(i);
                    if(!write_result.has_value()) {
                        schedule_error = std::move(write_result.error());
                        return submitted_any_llm;
                    }
                    changed = true;
                    continue;
                }

                std::size_t submitted_prompts = 0;
                for(const auto& request : prompt_requests) {
                    auto evidence = build_evidence_for_request(request, plan, model, config,
                                                               summaries);
                    auto prompt_result = build_prompt(request.kind, evidence);
                    if(!prompt_result.has_value()) {
                        schedule_error = GenerateError{
                            .message = std::format("failed to build prompt '{}' for '{}': {}",
                                                   prompt_request_key(request), plan.page_id,
                                                   prompt_result.error().message),
                        };
                        return submitted_any_llm;
                    }

                    auto output_key = prompt_request_key(request);
                    auto tag = next_tag++;
                    tag_map[tag] = TagInfo{
                        .state_index = i,
                        .output_key = output_key,
                    };
                    auto submit_result = client.submit(tag, std::move(*prompt_result));
                    if(!submit_result.has_value()) {
                        schedule_error = GenerateError{
                            .message = submit_result.error().message,
                        };
                        return submitted_any_llm;
                    }
                    ++submitted_prompts;
                    submitted_any_llm = true;
                }

                logging::info("submitted LLM for '{}': {} requests",
                              plan.page_id, submitted_prompts);
            }
        }

        return submitted_any_llm;
    };

    bool has_llm_work = submit_ready_pages();
    if(schedule_error.has_value()) {
        return std::unexpected(std::move(*schedule_error));
    }

    if(has_llm_work) {
        auto run_result = client.run(
            [&](std::uint64_t tag,
                std::expected<std::string, clore::net::LLMError> result) {
                if(schedule_error.has_value()) {
                    return;
                }

                auto tag_it = tag_map.find(tag);
                if(tag_it == tag_map.end()) {
                    return;
                }

                auto tag_info = std::move(tag_it->second);
                tag_map.erase(tag_it);

                auto& state = states[tag_info.state_index];
                const auto& plan = plan_set.plans[state.plan_index];

                if(result.has_value()) {
                    auto output_check = validate_output(*result);
                    if(!output_check.has_value()) {
                        schedule_error = GenerateError{
                            .message = std::format(
                                "output validation failed for prompt '{}' in '{}': {}",
                                tag_info.output_key, plan.page_id,
                                output_check.error().message),
                        };
                        logging::warn(schedule_error->message);
                        client.request_stop();
                        return;
                    }

                    state.prompt_outputs[tag_info.output_key] = std::move(*result);
                } else {
                    schedule_error = GenerateError{
                        .message = std::format("LLM failed for prompt '{}' in '{}': {}",
                                               tag_info.output_key, plan.page_id,
                                               result.error().message),
                    };
                    logging::warn(schedule_error->message);
                    client.request_stop();
                    return;
                }

                state.completed_prompts++;

                if(state.completed_prompts == state.total_prompts) {
                    auto write_result = write_completed_page(tag_info.state_index);
                    if(!write_result.has_value()) {
                        schedule_error = std::move(write_result.error());
                        logging::warn(schedule_error->message);
                        client.request_stop();
                        return;
                    }

                    submit_ready_pages();
                    if(schedule_error.has_value()) {
                        logging::warn(schedule_error->message);
                        client.request_stop();
                    }
                }
            });

        if(!run_result.has_value()) {
            return std::unexpected(GenerateError{
                .message = std::format("LLM event loop error: {}",
                                       run_result.error().message),
            });
        }
    }

    if(schedule_error.has_value()) {
        return std::unexpected(std::move(*schedule_error));
    }

    if(written_count == 0) {
        return std::unexpected(GenerateError{.message = "no pages were generated"});
    }

    return written_count;
}

auto write_pages(const std::vector<GeneratedPage>& pages, std::string_view output_root)
    -> std::expected<void, GenerateError> {
    for(const auto& page : pages) {
        auto result = write_page(page, output_root);
        if(!result.has_value()) {
            return std::unexpected(GenerateError{.message = result.error().message});
        }
    }
    return {};
}

}  // namespace clore::generate