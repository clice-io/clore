module;

#include <cstdint>
#include <expected>
#include <format>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

export module generate;

export import :model;
export import :path;
export import :evidence;
export import :prompt;
export import :render;
export import :planner;
export import :llm;

import config;
import extract;
import support;

export namespace clore::generate {

auto generate_dry_run(const config::TaskConfig& config, const extract::ProjectModel& model)
    -> std::expected<std::vector<GeneratedPage>, GenerateError>;

/// Non-blocking page generation with structured concurrency.
/// Pages are written to output_root immediately when all their LLM slots complete.
/// Returns the number of pages written.
auto generate_pages(const config::TaskConfig& config, const extract::ProjectModel& model,
                    std::string_view llm_model,
                    std::uint32_t rate_limit,
                    std::string_view output_root)
    -> std::expected<std::size_t, GenerateError>;

auto write_pages(const std::vector<GeneratedPage>& pages, std::string_view output_root)
    -> std::expected<void, GenerateError>;

}  // namespace clore::generate

// ── implementation ──────────────────────────────────────────────────

namespace clore::generate {

namespace {

auto get_page_template_path(const config::TaskConfig& config, PageType type) -> std::string {
    switch(type) {
        case PageType::Index:      return config.page_templates.index;
        case PageType::Module:     return config.page_templates.module_page;
        case PageType::Namespace:  return config.page_templates.namespace_page;
        case PageType::Type:       return config.page_templates.type_page;
        case PageType::File:       return config.page_templates.file_page;
    }
    return {};
}

auto build_blocks(const PagePlan& plan, const extract::ProjectModel& model,
                  const config::TaskConfig& config,
                  const LinkResolver& links) -> std::unordered_map<std::string, std::string> {
    std::unordered_map<std::string, std::string> blocks;
    for(auto& block_name : plan.deterministic_blocks) {
        blocks[block_name] = render_deterministic_block(block_name, plan, model, config, links);
    }
    return blocks;
}

auto build_evidence_for_slot(const SlotPlan& slot, const PagePlan& plan,
                             const extract::ProjectModel& model,
                             const config::TaskConfig& config,
                             const PageSummaryCache& summaries) -> EvidencePack {
    auto& slot_kind = slot.slot_kind;
    auto& rules = config.evidence_rules;

    if(slot_kind == "type_overview") {
        if(!plan.owner_keys.empty()) {
            for(auto& [id, sym] : model.symbols) {
                if(sym.qualified_name == plan.owner_keys[0]) {
                    return build_evidence_for_type_overview(sym, model, rules, summaries, config.project_root);
                }
            }
        }
    }

    if(slot_kind == "type_usage_notes") {
        if(!plan.owner_keys.empty()) {
            for(auto& [id, sym] : model.symbols) {
                if(sym.qualified_name == plan.owner_keys[0]) {
                    return build_evidence_for_type_usage_notes(sym, model, rules, config.project_root);
                }
            }
        }
    }

    if(slot_kind == "namespace_summary") {
        if(!plan.owner_keys.empty()) {
            auto it = model.namespaces.find(plan.owner_keys[0]);
            if(it != model.namespaces.end()) {
                return build_evidence_for_namespace_summary(it->second, model, rules, summaries, config.project_root);
            }
        }
    }

    if(slot_kind == "module_summary") {
        if(!plan.owner_keys.empty()) {
            for(auto& [source_file, mod_unit] : model.modules) {
                if(mod_unit.name == plan.owner_keys[0]) {
                    return build_evidence_for_module_summary(mod_unit, model, rules, summaries, config.project_root);
                }
            }
        }
    }

    if(slot_kind == "module_architecture") {
        if(!plan.owner_keys.empty()) {
            for(auto& [source_file, mod_unit] : model.modules) {
                if(mod_unit.name == plan.owner_keys[0]) {
                    return build_evidence_for_module_architecture(mod_unit, model, rules, summaries, config.project_root);
                }
            }
        }
    }

    if(slot_kind == "repository_overview" || slot_kind == "reading_guide") {
        return build_evidence_for_repository_overview(model, rules, summaries);
    }

    return EvidencePack{.slot_kind = slot_kind};
}

auto wrap_prompt_output_for_embed(std::string_view slot_kind, std::string_view prompt)
    -> std::string {
    std::string wrapped;
    wrapped.reserve(slot_kind.size() + prompt.size() + 32);
    wrapped += "> Prompt (`";
    wrapped += slot_kind;
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

auto render_single_page(const PagePlan& plan,
                        const config::TaskConfig& config,
                        const extract::ProjectModel& model,
                        const std::unordered_map<std::string, std::string>& slot_outputs,
                        bool embed_prompts,
                        const LinkResolver& links)
    -> std::expected<GeneratedPage, GenerateError> {

    auto tmpl_path = get_page_template_path(config, plan.page_type);
    auto tmpl_result = load_page_template(tmpl_path);
    if(!tmpl_result.has_value()) {
        return std::unexpected(GenerateError{
            .message = std::format("failed to load page template for '{}': {}",
                                   plan.page_id, tmpl_result.error().message)});
    }

    auto blocks = build_blocks(plan, model, config, links);

    std::unordered_map<std::string, std::string> embedded_slot_outputs;
    auto* slots_for_render = &slot_outputs;
    if(embed_prompts) {
        embedded_slot_outputs.reserve(slot_outputs.size());
        for(auto& [slot_kind, output] : slot_outputs) {
            embedded_slot_outputs[slot_kind] = wrap_prompt_output_for_embed(slot_kind, output);
        }
        slots_for_render = &embedded_slot_outputs;
    }

    auto assembly_result = assemble_page(*tmpl_result, plan.title, blocks, *slots_for_render, false);
    if(!assembly_result.has_value()) {
        return std::unexpected(GenerateError{
            .message = std::format("failed to assemble page '{}': {}",
                                   plan.page_id, assembly_result.error().message)});
    }

    return GeneratedPage{
        .relative_path = plan.relative_path,
        .content = *assembly_result,
    };
}

auto extract_summary_from_slot_output(const std::string& overview_output) -> std::string {
    // Take first paragraph as summary
    auto end = overview_output.find("\n\n");
    if(end != std::string::npos) {
        return overview_output.substr(0, end);
    }
    if(overview_output.size() > 300) {
        return overview_output.substr(0, 300);
    }
    return overview_output;
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

    // Build page_id -> plan index
    std::unordered_map<std::string, std::size_t> id_to_index;
    for(std::size_t i = 0; i < plan_set.plans.size(); ++i) {
        id_to_index[plan_set.plans[i].page_id] = i;
    }

    PageSummaryCache summaries;
    std::vector<GeneratedPage> pages;
    pages.reserve(plan_set.generation_order.size());

    auto links = build_link_resolver(plan_set);

    for(auto& page_id : plan_set.generation_order) {
        auto idx_it = id_to_index.find(page_id);
        if(idx_it == id_to_index.end()) continue;
        auto& plan = plan_set.plans[idx_it->second];

        // Build prompts for dry-run display
        std::unordered_map<std::string, std::string> slot_outputs;
        for(auto& slot : plan.slot_plans) {
            auto evidence = build_evidence_for_slot(slot, plan, model, config, summaries);

            auto tmpl_result = load_prompt_template(slot.prompt_template_path);
            if(!tmpl_result.has_value()) {
                slot_outputs[slot.slot_kind] = "> [prompt template load failed: " + tmpl_result.error().message + "]\n";
                continue;
            }

            auto prompt = instantiate_prompt(*tmpl_result, evidence);
            if(prompt.empty()) {
                return std::unexpected(GenerateError{
                    .message = std::format(
                        "prompt rendering produced empty output for slot '{}' in page '{}'",
                        slot.slot_kind, page_id)});
            }

            slot_outputs[slot.slot_kind] = std::move(prompt);
        }

        auto page_result = render_single_page(plan, config, model, slot_outputs, true, links);
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
    logging::info("page plan: {} pages, generation order size {}",
                  plan_set.plans.size(), plan_set.generation_order.size());

    // Build page_id -> plan index
    std::unordered_map<std::string, std::size_t> id_to_plan;
    for(std::size_t i = 0; i < plan_set.plans.size(); ++i) {
        id_to_plan[plan_set.plans[i].page_id] = i;
    }

    auto links = build_link_resolver(plan_set);

    // ── dependency graph ────────────────────────────────────────────

    struct PageState {
        std::size_t plan_index;
        std::size_t total_slots;
        std::size_t completed_slots = 0;
        std::size_t unsatisfied_deps = 0;
        bool submitted = false;
        bool written = false;
        std::unordered_map<std::string, std::string> slot_outputs;
    };

    std::vector<PageState> states;
    states.reserve(plan_set.generation_order.size());
    std::unordered_map<std::string, std::size_t> id_to_state;

    for(auto& page_id : plan_set.generation_order) {
        auto plan_it = id_to_plan.find(page_id);
        if(plan_it == id_to_plan.end()) continue;
        auto state_idx = states.size();
        id_to_state[page_id] = state_idx;
        states.push_back(PageState{
            .plan_index = plan_it->second,
            .total_slots = plan_set.plans[plan_it->second].slot_plans.size(),
        });
    }

    // Compute unsatisfied deps + build dependents adjacency list
    std::unordered_map<std::string, std::vector<std::size_t>> dependents;
    for(std::size_t i = 0; i < states.size(); ++i) {
        auto& plan = plan_set.plans[states[i].plan_index];
        for(auto& dep_id : plan.depends_on_pages) {
            if(id_to_state.contains(dep_id)) {
                states[i].unsatisfied_deps++;
                dependents[dep_id].push_back(i);
            }
        }
    }

    // ── shared scheduler state ──────────────────────────────────────

    std::unordered_map<std::string, std::string> prompt_tmpl_cache;
    PageSummaryCache summaries;
    std::size_t written_count = 0;

    struct TagInfo {
        std::size_t state_index;
        std::string slot_kind;
    };
    std::unordered_map<std::uint64_t, TagInfo> tag_map;
    std::uint64_t next_tag = 0;
    std::optional<GenerateError> schedule_error;

    LLMClient client(llm_model, config.llm.system_prompt, rate_limit,
                     config.llm.retry_count, config.llm.retry_initial_backoff_ms);

    // ── write a completed page immediately ──────────────────────────

    auto write_completed_page = [&](std::size_t state_idx)
        -> std::expected<void, GenerateError> {
        auto& state = states[state_idx];
        auto& plan = plan_set.plans[state.plan_index];

        auto page_result = render_single_page(plan, config, model, state.slot_outputs, false, links);
        if(!page_result.has_value()) {
            return std::unexpected(std::move(page_result.error()));
        }

        auto write_r = write_page(*page_result, output_root);
        if(!write_r.has_value()) {
            return std::unexpected(GenerateError{.message = write_r.error().message});
        }

        state.written = true;
        ++written_count;
        logging::info("  written {}", page_result->relative_path);

        // Extract summary for dependent pages
        for(auto& [kind, content] : state.slot_outputs) {
            if(kind == "type_overview" || kind == "module_summary" ||
               kind == "namespace_summary") {
                if(!plan.owner_keys.empty()) {
                    summaries[plan.owner_keys[0]] = extract_summary_from_slot_output(content);
                }
            }
        }

        // Unlock dependents
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

    // ── submit ready pages (deterministic pages written inline) ─────

    auto submit_ready_pages = [&]() -> bool {
        bool submitted_any_llm = false;
        bool changed = true;

        while(changed) {
            changed = false;
            for(std::size_t i = 0; i < states.size(); ++i) {
                auto& state = states[i];
                if(state.submitted || state.unsatisfied_deps > 0) continue;
                state.submitted = true;

                auto& plan = plan_set.plans[state.plan_index];

                if(state.total_slots == 0) {
                    // Deterministic page: render + write immediately
                    auto r = write_completed_page(i);
                    if(!r.has_value()) {
                        schedule_error = std::move(r.error());
                        return submitted_any_llm;
                    }
                    changed = true;  // might unlock more pages
                    continue;
                }

                // LLM page: build evidence + prompts, submit to client
                for(auto& slot : plan.slot_plans) {
                    auto evidence = build_evidence_for_slot(
                        slot, plan, model, config, summaries);

                    auto tmpl_it = prompt_tmpl_cache.find(slot.prompt_template_path);
                    if(tmpl_it == prompt_tmpl_cache.end()) {
                        auto tmpl_r = load_prompt_template(slot.prompt_template_path);
                        if(!tmpl_r.has_value()) {
                            schedule_error = GenerateError{
                                .message = std::format(
                                    "failed to load prompt template '{}': {}",
                                    slot.prompt_template_path, tmpl_r.error().message)};
                            return submitted_any_llm;
                        }
                        tmpl_it = prompt_tmpl_cache.emplace(
                            slot.prompt_template_path, std::move(*tmpl_r)).first;
                    }

                    auto prompt = instantiate_prompt(tmpl_it->second, evidence);
                    if(prompt.empty()) {
                        schedule_error = GenerateError{
                            .message = std::format(
                                "prompt rendering produced empty output for slot '{}' in '{}'",
                                slot.slot_kind, plan.page_id)};
                        return submitted_any_llm;
                    }

                    auto tag = next_tag++;
                    tag_map[tag] = TagInfo{.state_index = i, .slot_kind = slot.slot_kind};
                    auto submit_r = client.submit(tag, std::move(prompt));
                    if(!submit_r.has_value()) {
                        schedule_error = GenerateError{.message = submit_r.error().message};
                        return submitted_any_llm;
                    }
                    submitted_any_llm = true;
                }

                logging::info("submitted LLM for '{}': {} slots",
                              plan.page_id, plan.slot_plans.size());
            }
        }

        return submitted_any_llm;
    };

    // ── drive the event loop ────────────────────────────────────────

    bool has_llm_work = submit_ready_pages();
    if(schedule_error.has_value()) {
        return std::unexpected(std::move(*schedule_error));
    }

    if(has_llm_work) {
        auto run_result = client.run(
            [&](std::uint64_t tag, std::expected<std::string, LLMError> result) {
                if(schedule_error.has_value()) return;

                auto tag_it = tag_map.find(tag);
                if(tag_it == tag_map.end()) return;

                auto state_idx = tag_it->second.state_index;
                auto& slot_kind = tag_it->second.slot_kind;
                auto& state = states[state_idx];
                auto& plan = plan_set.plans[state.plan_index];

                if(result.has_value()) {
                    auto output_check = validate_output(
                        *result, config.validation);
                    if(output_check.has_value()) {
                        state.slot_outputs[slot_kind] = std::move(*result);
                    } else {
                        logging::warn("output validation failed for slot '{}' in '{}': {}",
                                      slot_kind, plan.page_id,
                                      output_check.error().message);
                    }
                } else {
                    logging::warn("LLM failed for slot '{}' in '{}': {}",
                                  slot_kind, plan.page_id, result.error().message);
                }

                state.completed_slots++;

                if(state.completed_slots == state.total_slots) {
                    auto r = write_completed_page(state_idx);
                    if(!r.has_value()) {
                        schedule_error = std::move(r.error());
                        return;
                    }
                    submit_ready_pages();
                }
            });

        if(!run_result.has_value()) {
            return std::unexpected(GenerateError{
                .message = std::format("LLM event loop error: {}",
                                       run_result.error().message)});
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
    for(auto& page : pages) {
        auto r = write_page(page, output_root);
        if(!r.has_value()) {
            return std::unexpected(GenerateError{.message = r.error().message});
        }
    }
    return {};
}

}  // namespace clore::generate
