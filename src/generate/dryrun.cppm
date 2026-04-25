export module generate:dryrun;

import std;
import :analysis;
import :model;
import config;
import extract;

export namespace clore::generate {

auto build_dry_run_page_summary_texts(const PagePlan& plan,
                                      const std::vector<PromptRequest>& prompt_requests)
    -> std::unordered_map<std::string, std::string>;

auto build_request_estimate_page(const PagePlanSet& plan_set,
                                 const extract::ProjectModel& model,
                                 const config::TaskConfig& config) -> GeneratedPage;

auto build_llms_page(const PagePlanSet& plan_set,
                     const config::TaskConfig& config,
                     std::string_view request_estimate_path = {}) -> GeneratedPage;

auto page_summary_cache_key_for_request(const PagePlan& plan, const PromptRequest& request)
    -> std::optional<std::string>;

}  // namespace clore::generate

namespace clore::generate {

namespace {

struct RequestEstimate {
    std::size_t total_requests = 0;
    std::size_t page_requests = 0;
    std::size_t symbol_requests = 0;
    std::size_t namespace_page_requests = 0;
    std::size_t module_summary_requests = 0;
    std::size_t module_architecture_requests = 0;
    std::size_t index_overview_requests = 0;
    std::size_t function_symbols = 0;
    std::size_t type_symbols = 0;
    std::size_t variable_symbols = 0;
    std::size_t function_analysis_requests = 0;
    std::size_t function_declaration_summary_requests = 0;
    std::size_t function_implementation_summary_requests = 0;
    std::size_t type_analysis_requests = 0;
    std::size_t type_declaration_summary_requests = 0;
    std::size_t type_implementation_summary_requests = 0;
    std::size_t variable_analysis_requests = 0;
};

auto project_name_from_config(const config::TaskConfig& config) -> std::string {
    auto project_name = std::filesystem::path(config.project_root).filename().generic_string();
    if(project_name.empty()) {
        return "project";
    }
    return project_name;
}

auto estimate_request_count(const PagePlanSet& plan_set, const extract::ProjectModel& model)
    -> RequestEstimate {
    RequestEstimate estimate;

    for(const auto& plan: plan_set.plans) {
        std::unordered_set<std::string> seen;
        seen.reserve(plan.prompt_requests.size());
        for(const auto& request: plan.prompt_requests) {
            if(!seen.insert(prompt_request_key(request)).second) {
                continue;
            }
            estimate.page_requests++;
            switch(request.kind) {
                case PromptKind::NamespaceSummary: estimate.namespace_page_requests++; break;
                case PromptKind::ModuleSummary: estimate.module_summary_requests++; break;
                case PromptKind::ModuleArchitecture: estimate.module_architecture_requests++; break;
                case PromptKind::IndexOverview: estimate.index_overview_requests++; break;
                case PromptKind::FunctionAnalysis:
                case PromptKind::TypeAnalysis:
                case PromptKind::VariableAnalysis:
                case PromptKind::FunctionDeclarationSummary:
                case PromptKind::FunctionImplementationSummary:
                case PromptKind::TypeDeclarationSummary:
                case PromptKind::TypeImplementationSummary: break;
            }
        }
    }

    for(const auto& [_, symbol]: model.symbols) {
        if(!is_page_level_symbol(model, symbol)) {
            continue;
        }
        auto prompt_kinds = symbol_prompt_kinds_for_symbol(symbol);
        if(prompt_kinds.empty()) {
            continue;
        }

        if(is_function_kind(symbol.kind)) {
            estimate.function_symbols++;
        } else if(is_type_kind(symbol.kind)) {
            estimate.type_symbols++;
        } else if(is_variable_kind(symbol.kind)) {
            estimate.variable_symbols++;
        }

        for(auto kind: prompt_kinds) {
            estimate.symbol_requests++;
            switch(kind) {
                case PromptKind::FunctionAnalysis: estimate.function_analysis_requests++; break;
                case PromptKind::FunctionDeclarationSummary:
                    estimate.function_declaration_summary_requests++;
                    break;
                case PromptKind::FunctionImplementationSummary:
                    estimate.function_implementation_summary_requests++;
                    break;
                case PromptKind::TypeAnalysis: estimate.type_analysis_requests++; break;
                case PromptKind::TypeDeclarationSummary:
                    estimate.type_declaration_summary_requests++;
                    break;
                case PromptKind::TypeImplementationSummary:
                    estimate.type_implementation_summary_requests++;
                    break;
                case PromptKind::VariableAnalysis: estimate.variable_analysis_requests++; break;
                case PromptKind::NamespaceSummary:
                case PromptKind::ModuleSummary:
                case PromptKind::ModuleArchitecture:
                case PromptKind::IndexOverview: break;
            }
        }
    }

    estimate.total_requests = estimate.page_requests + estimate.symbol_requests;
    return estimate;
}

auto fallback_page_summary_for_request(const PagePlan& plan, const PromptRequest& request)
    -> std::string {
    switch(request.kind) {
        case PromptKind::NamespaceSummary:
            if(!plan.owner_keys.empty()) {
                return std::format(
                    "Namespace `{}` groups related declarations " "documented in this reference.",
                    plan.owner_keys.front());
            }
            return {};
        case PromptKind::ModuleSummary:
            if(!plan.owner_keys.empty()) {
                return std::format(
                    "Module `{}` exposes the APIs and relationships " "documented on its page.",
                    plan.owner_keys.front());
            }
            return {};
        case PromptKind::ModuleArchitecture:
        case PromptKind::IndexOverview:
        case PromptKind::FunctionAnalysis:
        case PromptKind::TypeAnalysis:
        case PromptKind::VariableAnalysis:
        case PromptKind::FunctionDeclarationSummary:
        case PromptKind::FunctionImplementationSummary:
        case PromptKind::TypeDeclarationSummary:
        case PromptKind::TypeImplementationSummary: return {};
    }

    return {};
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
    struct LabeledPage {
        std::string label;
        const PagePlan* plan = nullptr;
    };

    std::vector<LabeledPage> pages;
    pages.reserve(plan_set.plans.size());
    for(const auto& plan: plan_set.plans) {
        if(plan.page_type == page_type) {
            pages.push_back(LabeledPage{
                .label = llms_entry_label(plan, config),
                .plan = &plan,
            });
        }
    }
    if(pages.empty()) {
        return;
    }

    std::sort(pages.begin(), pages.end(), [](const LabeledPage& lhs, const LabeledPage& rhs) {
        if(lhs.label != rhs.label) {
            return lhs.label < rhs.label;
        }
        return lhs.plan->relative_path < rhs.plan->relative_path;
    });

    content += "## ";
    content += heading;
    content += "\n\n";
    for(const auto& page: pages) {
        content += "- [";
        content += page.label;
        content += "](";
        content += page.plan->relative_path;
        content += ")\n";
    }
    content += "\n";
}

}  // namespace

auto build_request_estimate_page(const PagePlanSet& plan_set,
                                 const extract::ProjectModel& model,
                                 const config::TaskConfig& config) -> GeneratedPage {
    auto estimate = estimate_request_count(plan_set, model);
    auto project_name = project_name_from_config(config);

    std::string content;
    content.reserve(2048);
    content += "---\n";
    content += "title: Dry Run Request Estimate\n";
    content += "description: Estimated prompt task count for this documentation run.\n";
    content += "layout: doc\n";
    content += "page_template: doc\n";
    content += "---\n\n";
    content += "# Dry Run Request Estimate\n\n";
    content += "Project: `";
    content += project_name;
    content += "`\n\n";
    content +=
        "Dry-run does not call the model. This page reports how many prompt tasks the current plan would schedule before cache hits.\n\n";
    content += "## Summary\n\n";
    content += std::format("- Total prompt tasks: {}\n", estimate.total_requests);
    content += std::format("- Page prompt tasks: {}\n", estimate.page_requests);
    content += std::format("- Symbol prompt tasks: {}\n\n", estimate.symbol_requests);
    content += "## Page Prompt Tasks\n\n";
    content += std::format("- Namespace summaries: {}\n", estimate.namespace_page_requests);
    content += std::format("- Module summaries: {}\n", estimate.module_summary_requests);
    content +=
        std::format("- Module architecture prompts: {}\n", estimate.module_architecture_requests);
    content += std::format("- Index overviews: {}\n\n", estimate.index_overview_requests);
    content += "## Symbol Prompt Tasks\n\n";
    content += std::format("- Function symbols: {} -> {} requests\n",
                           estimate.function_symbols,
                           estimate.function_analysis_requests +
                               estimate.function_declaration_summary_requests +
                               estimate.function_implementation_summary_requests);
    content +=
        std::format("- Type symbols: {} -> {} requests\n",
                    estimate.type_symbols,
                    estimate.type_analysis_requests + estimate.type_declaration_summary_requests +
                        estimate.type_implementation_summary_requests);
    content += std::format("- Variable symbols: {} -> {} requests\n\n",
                           estimate.variable_symbols,
                           estimate.variable_analysis_requests);
    content += std::format("- Function analysis: {}\n", estimate.function_analysis_requests);
    content += std::format("- Function declaration summaries: {}\n",
                           estimate.function_declaration_summary_requests);
    content += std::format("- Function implementation summaries: {}\n",
                           estimate.function_implementation_summary_requests);
    content += std::format("- Type analysis: {}\n", estimate.type_analysis_requests);
    content += std::format("- Type declaration summaries: {}\n",
                           estimate.type_declaration_summary_requests);
    content += std::format("- Type implementation summaries: {}\n",
                           estimate.type_implementation_summary_requests);
    content += std::format("- Variable analysis: {}\n", estimate.variable_analysis_requests);

    return GeneratedPage{
        .title = "Dry Run Request Estimate",
        .relative_path = "request-estimate.md",
        .content = std::move(content),
    };
}

auto page_summary_cache_key_for_request(const PagePlan& plan, const PromptRequest& request)
    -> std::optional<std::string> {
    switch(request.kind) {
        case PromptKind::NamespaceSummary:
        case PromptKind::ModuleSummary:
            if(!plan.owner_keys.empty()) {
                return plan.owner_keys.front();
            }
            return std::nullopt;
        case PromptKind::ModuleArchitecture:
        case PromptKind::IndexOverview:
        case PromptKind::FunctionAnalysis:
        case PromptKind::TypeAnalysis:
        case PromptKind::VariableAnalysis:
        case PromptKind::FunctionDeclarationSummary:
        case PromptKind::FunctionImplementationSummary:
        case PromptKind::TypeDeclarationSummary:
        case PromptKind::TypeImplementationSummary: return std::nullopt;
    }

    return std::nullopt;
}

auto build_dry_run_page_summary_texts(const PagePlan& plan,
                                      const std::vector<PromptRequest>& prompt_requests)
    -> std::unordered_map<std::string, std::string> {
    std::unordered_map<std::string, std::string> summary_texts;
    for(const auto& request: prompt_requests) {
        if(!page_summary_cache_key_for_request(plan, request).has_value()) {
            continue;
        }
        auto summary = fallback_page_summary_for_request(plan, request);
        if(summary.empty()) {
            continue;
        }
        summary_texts.insert_or_assign(prompt_request_key(request), std::move(summary));
    }
    return summary_texts;
}

auto build_llms_page(const PagePlanSet& plan_set,
                     const config::TaskConfig& config,
                     std::string_view request_estimate_path) -> GeneratedPage {
    auto project_name = project_name_from_config(config);

    std::string content;
    content.reserve(2048);
    content += "# ";
    content += project_name;
    content += "\n\n";
    content += "> Machine-readable index for the generated C++ reference.\n\n";
    content += "- [API Reference](index.md)\n\n";
    if(!request_estimate_path.empty()) {
        content += "- [Dry Run Request Estimate](";
        content += request_estimate_path;
        content += ")\n\n";
    }
    content +=
        "The primary reference is organized into modules, namespaces, and " "source files.\n\n";

    append_llms_section(content, "Modules", plan_set, PageType::Module, config);
    append_llms_section(content, "Namespaces", plan_set, PageType::Namespace, config);
    append_llms_section(content, "Files", plan_set, PageType::File, config);

    return GeneratedPage{
        .title = project_name,
        .relative_path = "llms.txt",
        .content = std::move(content),
    };
}

}  // namespace clore::generate
