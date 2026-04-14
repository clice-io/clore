module;

#include <algorithm>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

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
auto generate_pages(const config::TaskConfig& config,
                    const extract::ProjectModel& model,
                    std::string_view llm_model,
                    std::uint32_t rate_limit,
                    std::string_view output_root) -> std::expected<std::size_t, GenerateError>;

auto write_pages(const std::vector<GeneratedPage>& pages, std::string_view output_root)
    -> std::expected<void, GenerateError>;

}  // namespace clore::generate

namespace clore::generate {

namespace {

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

auto deduplicate_prompt_requests(const PagePlan& plan) -> std::vector<PromptRequest> {
    std::vector<PromptRequest> unique;
    unique.reserve(plan.prompt_requests.size());
    std::unordered_set<std::string> seen;
    seen.reserve(plan.prompt_requests.size());
    for(const auto& request: plan.prompt_requests) {
        auto key = prompt_request_key(request);
        if(seen.insert(key).second) {
            unique.push_back(request);
        }
    }
    return unique;
}

auto set_evidence_metadata(EvidencePack pack, const PagePlan& plan, const PromptRequest& request)
    -> EvidencePack {
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
                        build_evidence_for_namespace_summary(ns_it->second,
                                                             model,
                                                             rules,
                                                             summaries,
                                                             config.project_root),
                        plan,
                        request);
                }
            }
            break;
        case PromptKind::ModuleSummary:
            if(!plan.owner_keys.empty()) {
                if(auto* mod = extract::find_module_by_name(model, plan.owner_keys.front())) {
                    return set_evidence_metadata(
                        build_evidence_for_module_summary(*mod,
                                                          model,
                                                          rules,
                                                          summaries,
                                                          config.project_root),
                        plan,
                        request);
                }
            }
            break;
        case PromptKind::ModuleArchitecture:
            if(!plan.owner_keys.empty()) {
                if(auto* mod = extract::find_module_by_name(model, plan.owner_keys.front())) {
                    return set_evidence_metadata(
                        build_evidence_for_module_architecture(*mod,
                                                               model,
                                                               rules,
                                                               summaries,
                                                               config.project_root),
                        plan,
                        request);
                }
            }
            break;
        case PromptKind::IndexOverview:
            return set_evidence_metadata(build_evidence_for_index_overview(model, rules, summaries),
                                         plan,
                                         request);
        case PromptKind::FunctionDeclarationSummary:
            if(!request.target_key.empty()) {
                if(auto* sym = find_sym(model, request.target_key)) {
                    return set_evidence_metadata(
                        build_evidence_for_function_declaration_summary(*sym,
                                                                        model,
                                                                        rules,
                                                                        summaries,
                                                                        config.project_root),
                        plan,
                        request);
                }
            }
            break;
        case PromptKind::FunctionImplementationSummary:
            if(!request.target_key.empty()) {
                if(auto* sym = find_sym(model, request.target_key)) {
                    return set_evidence_metadata(
                        build_evidence_for_function_implementation_summary(*sym,
                                                                           model,
                                                                           rules,
                                                                           config.project_root),
                        plan,
                        request);
                }
            }
            break;
        case PromptKind::TypeDeclarationSummary:
            if(!request.target_key.empty()) {
                if(auto* sym = find_sym(model, request.target_key)) {
                    return set_evidence_metadata(
                        build_evidence_for_type_declaration_summary(*sym,
                                                                    model,
                                                                    rules,
                                                                    summaries,
                                                                    config.project_root),
                        plan,
                        request);
                }
            }
            break;
        case PromptKind::TypeImplementationSummary:
            if(!request.target_key.empty()) {
                if(auto* sym = find_sym(model, request.target_key)) {
                    return set_evidence_metadata(
                        build_evidence_for_type_implementation_summary(*sym,
                                                                       model,
                                                                       rules,
                                                                       config.project_root),
                        plan,
                        request);
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

auto render_generated_pages(const PagePlan& plan,
                            const config::TaskConfig& config,
                            const extract::ProjectModel& model,
                            const std::unordered_map<std::string, std::string>& prompt_outputs,
                            const LinkResolver& links)
    -> std::expected<std::vector<GeneratedPage>, GenerateError> {
    auto render_result = render_page_bundle(plan, config, model, prompt_outputs, links);
    if(!render_result.has_value()) {
        return std::unexpected(GenerateError{
            .message = std::format("failed to render page '{}': {}",
                                   plan.page_id,
                                   render_result.error().message),
        });
    }

    return std::move(*render_result);
}

auto update_summary_cache(PageSummaryCache& summaries,
                          const PagePlan& plan,
                          const std::vector<PromptRequest>& prompt_requests,
                          const std::unordered_map<std::string, std::string>& prompt_outputs)
    -> void {
    if(plan.owner_keys.empty()) {
        return;
    }

    for(const auto& request: prompt_requests) {
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

auto llms_entry_label(const PagePlan& plan, const config::TaskConfig& config) -> std::string {
    switch(plan.page_type) {
        case PageType::Module:
        case PageType::Namespace:
            if(!plan.owner_keys.empty()) {
                return plan.owner_keys.front();
            }
            break;
        case PageType::File:
            if(!plan.owner_keys.empty()) {
                return make_source_relative(plan.owner_keys.front(), config.project_root);
            }
            break;
        case PageType::Index: return plan.title;
    }
    return plan.title;
}

auto append_llms_section(std::string& content,
                         std::string_view heading,
                         const PagePlanSet& plan_set,
                         PageType page_type,
                         const config::TaskConfig& config) -> void {
    std::vector<const PagePlan*> pages;
    pages.reserve(plan_set.plans.size());
    for(const auto& plan: plan_set.plans) {
        if(plan.page_type == page_type) {
            pages.push_back(&plan);
        }
    }
    if(pages.empty()) {
        return;
    }

    std::sort(pages.begin(),
              pages.end(),
              [&](const PagePlan* lhs, const PagePlan* rhs) {
                  auto lhs_label = llms_entry_label(*lhs, config);
                  auto rhs_label = llms_entry_label(*rhs, config);
                  if(lhs_label != rhs_label) {
                      return lhs_label < rhs_label;
                  }
                  return lhs->relative_path < rhs->relative_path;
              });

    content += "## ";
    content += heading;
    content += "\n\n";
    for(const auto* plan: pages) {
        content += "- [";
        content += llms_entry_label(*plan, config);
        content += "](";
        content += plan->relative_path;
        content += ")\n";
    }
    content += "\n";
}

auto build_llms_page(const PagePlanSet& plan_set, const config::TaskConfig& config)
    -> GeneratedPage {
    namespace fs = std::filesystem;

    auto project_name = fs::path(config.project_root).filename().generic_string();
    if(project_name.empty()) {
        project_name = "project";
    }

    std::string content;
    content.reserve(2048);
    content += "# ";
    content += project_name;
    content += "\n\n";
    content += "> Machine-readable index for the generated C++ reference.\n\n";
    content += "- [API Reference](index.md)\n\n";
    content += "The primary reference is organized into modules, namespaces, and source files.\n\n";

    append_llms_section(content, "Modules", plan_set, PageType::Module, config);
    append_llms_section(content, "Namespaces", plan_set, PageType::Namespace, config);
    append_llms_section(content, "Files", plan_set, PageType::File, config);

    return GeneratedPage{
        .relative_path = "llms.txt",
        .content = std::move(content),
    };
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
                  plan_set.plans.size(),
                  plan_set.generation_order.size());

    std::unordered_map<std::string, std::size_t> id_to_index;
    id_to_index.reserve(plan_set.plans.size());
    for(std::size_t i = 0; i < plan_set.plans.size(); ++i) {
        id_to_index[plan_set.plans[i].page_id] = i;
    }

    PageSummaryCache summaries;
    std::vector<GeneratedPage> pages;
    pages.reserve(plan_set.generation_order.size());

    auto links = build_link_resolver(plan_set, model);

    for(const auto& page_id: plan_set.generation_order) {
        auto idx_it = id_to_index.find(page_id);
        if(idx_it == id_to_index.end()) {
            continue;
        }

        const auto& plan = plan_set.plans[idx_it->second];
        auto prompt_requests = deduplicate_prompt_requests(plan);
        std::unordered_map<std::string, std::string> prompt_outputs;
        prompt_outputs.reserve(prompt_requests.size());

        for(const auto& request: prompt_requests) {
            auto evidence = build_evidence_for_request(request, plan, model, config, summaries);
            auto prompt_result = build_prompt(request.kind, evidence);
            if(!prompt_result.has_value()) {
                return std::unexpected(GenerateError{
                    .message = std::format("failed to build prompt '{}' for '{}': {}",
                                           prompt_request_key(request),
                                           page_id,
                                           prompt_result.error().message),
                });
            }
            auto key = prompt_request_key(request);
            prompt_outputs.emplace(key, wrap_prompt_output_for_embed(key, *prompt_result));
        }

        auto page_result = render_generated_pages(plan, config, model, prompt_outputs, links);
        if(!page_result.has_value()) {
            return std::unexpected(std::move(page_result.error()));
        }

        for(auto& page: *page_result) {
            pages.push_back(std::move(page));
        }
    }

    pages.push_back(build_llms_page(plan_set, config));
    return pages;
}

auto generate_pages(const config::TaskConfig& config,
                    const extract::ProjectModel& model,
                    std::string_view llm_model,
                    std::uint32_t rate_limit,
                    std::string_view output_root) -> std::expected<std::size_t, GenerateError> {
    auto plan_result = build_page_plan_set(config, model);
    if(!plan_result.has_value()) {
        return std::unexpected(GenerateError{.message = plan_result.error().message});
    }

    auto& plan_set = *plan_result;

    logging::info("page plan: {} pages, generation order size {}",
                  plan_set.plans.size(),
                  plan_set.generation_order.size());

    std::unordered_map<std::string, std::size_t> id_to_plan;
    id_to_plan.reserve(plan_set.plans.size());
    for(std::size_t i = 0; i < plan_set.plans.size(); ++i) {
        id_to_plan[plan_set.plans[i].page_id] = i;
    }

    std::vector<std::vector<PromptRequest>> prompt_requests_by_plan;
    prompt_requests_by_plan.reserve(plan_set.plans.size());
    for(const auto& plan: plan_set.plans) {
        prompt_requests_by_plan.push_back(deduplicate_prompt_requests(plan));
    }

    auto links = build_link_resolver(plan_set, model);

    std::vector<PageState> states;
    states.reserve(plan_set.generation_order.size());
    std::unordered_map<std::string, std::size_t> id_to_state;
    id_to_state.reserve(plan_set.generation_order.size());

    for(const auto& page_id: plan_set.generation_order) {
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
        for(const auto& dep_id: plan.depends_on_pages) {
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

    clore::net::LLMClient client(llm_model,
                                 config.llm.system_prompt,
                                 rate_limit,
                                 config.llm.retry_count,
                                 config.llm.retry_initial_backoff_ms);

    auto write_completed_page = [&](std::size_t state_idx) -> std::expected<void, GenerateError> {
        auto& state = states[state_idx];
        auto& plan = plan_set.plans[state.plan_index];
        const auto& prompt_requests = prompt_requests_by_plan[state.plan_index];

        if(state.prompt_outputs.size() != state.total_prompts) {
            return std::unexpected(GenerateError{
                .message = std::format("page '{}' completed with {} of {} prompt outputs",
                                       plan.page_id,
                                       state.prompt_outputs.size(),
                                       state.total_prompts),
            });
        }

        auto page_result = render_generated_pages(plan, config, model, state.prompt_outputs, links);
        if(!page_result.has_value()) {
            return std::unexpected(std::move(page_result.error()));
        }

        for(const auto& page: *page_result) {
            auto write_result = write_page(page, output_root);
            if(!write_result.has_value()) {
                return std::unexpected(GenerateError{
                    .message = write_result.error().message,
                });
            }
            logging::info("  written {}", page.relative_path);
        }

        state.written = true;
        written_count += page_result->size();

        update_summary_cache(summaries, plan, prompt_requests, state.prompt_outputs);

        auto dep_it = dependents.find(plan.page_id);
        if(dep_it != dependents.end()) {
            for(auto dep_state_idx: dep_it->second) {
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
                for(const auto& request: prompt_requests) {
                    auto evidence =
                        build_evidence_for_request(request, plan, model, config, summaries);
                    auto prompt_result = build_prompt(request.kind, evidence);
                    if(!prompt_result.has_value()) {
                        schedule_error = GenerateError{
                            .message = std::format("failed to build prompt '{}' for '{}': {}",
                                                   prompt_request_key(request),
                                                   plan.page_id,
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
                              plan.page_id,
                              submitted_prompts);
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
            [&](std::uint64_t tag, std::expected<std::string, clore::net::LLMError> result) {
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
                    auto normalized_output = normalize_prompt_output(*result);
                    if(normalized_output != *result && result->find("```") != std::string::npos) {
                        logging::warn(
                            "normalized fenced markdown from prompt '{}' in '{}' before validation",
                            tag_info.output_key,
                            plan.page_id);
                    }

                    auto output_check = validate_output(normalized_output);
                    if(!output_check.has_value()) {
                        schedule_error = GenerateError{
                            .message =
                                std::format("output validation failed for prompt '{}' in '{}': {}",
                                            tag_info.output_key,
                                            plan.page_id,
                                            output_check.error().message),
                        };
                        logging::warn(schedule_error->message);
                        client.request_stop();
                        return;
                    }

                    state.prompt_outputs[tag_info.output_key] = std::move(normalized_output);
                } else {
                    schedule_error = GenerateError{
                        .message = std::format("LLM failed for prompt '{}' in '{}': {}",
                                               tag_info.output_key,
                                               plan.page_id,
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
                .message = std::format("LLM event loop error: {}", run_result.error().message),
            });
        }
    }

    if(schedule_error.has_value()) {
        return std::unexpected(std::move(*schedule_error));
    }

    if(written_count == 0) {
        return std::unexpected(GenerateError{.message = "no pages were generated"});
    }

    auto llms_page = build_llms_page(plan_set, config);
    if(auto result = write_page(llms_page, output_root); !result.has_value()) {
        return std::unexpected(GenerateError{.message = result.error().message});
    }
    logging::info("  written {}", llms_page.relative_path);
    written_count++;

    return written_count;
}

auto write_pages(const std::vector<GeneratedPage>& pages, std::string_view output_root)
    -> std::expected<void, GenerateError> {
    for(const auto& page: pages) {
        auto result = write_page(page, output_root);
        if(!result.has_value()) {
            return std::unexpected(GenerateError{.message = result.error().message});
        }
    }
    return {};
}

}  // namespace clore::generate
