module;

#include "kota/async/async.h"

export module generate:scheduler;

import std;
import :analysis;
import :dryrun;
import :evidence;
import :markdown;
import :model;
import :planner;
import :diagram;
import :page;
import :symbol;
import config;
import extract;
import :cache;
import http;
import network;
import protocol;
import support;

namespace clore::generate {

namespace {

namespace fs = std::filesystem;

auto make_generate_error(cache::CacheError error) -> GenerateError {
    return GenerateError{.message = std::move(error.message)};
}

auto request_llm_async(std::string_view model,
                       std::string_view system_prompt,
                       clore::net::PromptRequest request,
                       kota::event_loop& loop) -> kota::task<std::string, clore::net::LLMError> {
    co_return co_await clore::net::call_llm_async(model, system_prompt, std::move(request), loop)
        .or_fail();
}

struct PreparedSymbolAnalysisTarget {
    extract::SymbolID symbol_id{};
    std::string target_key;
    std::vector<PromptKind> prompt_kinds;
};

struct PreparedGenerationContext {
    PagePlanSet plan_set;
    LinkResolver links;
    std::unordered_map<std::string, std::size_t> id_to_plan;
    std::vector<std::vector<PromptRequest>> prompt_requests_by_plan;
    std::vector<std::vector<std::string>> symbol_targets_by_plan;
    std::vector<PreparedSymbolAnalysisTarget> symbol_analysis_targets;
};

struct PreparedSymbolAnalyses {
    SymbolAnalysisStore analyses;
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
            pack.subject_name =
                std::string(parse_symbol_target_key(request.target_key).qualified_name);
        } else if(!plan.owner_keys.empty()) {
            pack.subject_name = plan.owner_keys.front();
        }
    }
    return pack;
}

auto require_owned_module(const PagePlan& plan,
                          const extract::ProjectModel& model,
                          std::string_view prompt_label)
    -> std::expected<const extract::ModuleUnit*, GenerateError> {
    if(plan.owner_keys.empty()) {
        return std::unexpected(GenerateError{
            .message = std::format("{} request missing owner_keys", prompt_label),
        });
    }

    auto modules = extract::find_modules_by_name(model, plan.owner_keys.front());
    if(modules.empty()) {
        return std::unexpected(GenerateError{
            .message = std::format("module '{}' not found", plan.owner_keys.front()),
        });
    }
    if(modules.size() > 1) {
        auto interface_count =
            std::ranges::count_if(modules, [](auto* m) { return m->is_interface; });
        if(interface_count != 1) {
            return std::unexpected(GenerateError{
                .message = std::format("module '{}' is ambiguous: {} units, {} interfaces",
                                       plan.owner_keys.front(),
                                       modules.size(),
                                       interface_count),
            });
        }
    }

    if(auto* mod = extract::find_module_by_name(model, plan.owner_keys.front())) {
        return mod;
    }

    return std::unexpected(GenerateError{
        .message = std::format("module '{}' could not be resolved", plan.owner_keys.front()),
    });
}

auto build_evidence_for_request(const PromptRequest& request,
                                const PagePlan& plan,
                                const extract::ProjectModel& model,
                                const config::TaskConfig& config,
                                const SymbolAnalysisStore& analyses,
                                const PageSummaryCache& page_summaries)
    -> std::expected<EvidencePack, GenerateError> {
    auto target_name = std::string(parse_symbol_target_key(request.target_key).qualified_name);

    switch(request.kind) {
        case PromptKind::NamespaceSummary:
            if(!plan.owner_keys.empty()) {
                auto ns_it = model.namespaces.find(plan.owner_keys.front());
                if(ns_it != model.namespaces.end()) {
                    return set_evidence_metadata(
                        build_evidence_for_namespace_summary(ns_it->second,
                                                             model,
                                                             analyses,
                                                             config.project_root),
                        plan,
                        request);
                }
                return std::unexpected(GenerateError{
                    .message = std::format("namespace '{}' not found", plan.owner_keys.front()),
                });
            }
            return std::unexpected(GenerateError{
                .message = "namespace summary request missing owner_keys",
            });
        case PromptKind::ModuleSummary:
        case PromptKind::ModuleArchitecture: {
            auto module_result = require_owned_module(plan, model, prompt_kind_name(request.kind));
            if(!module_result.has_value()) {
                return std::unexpected(std::move(module_result.error()));
            }
            if(request.kind == PromptKind::ModuleSummary) {
                return set_evidence_metadata(build_evidence_for_module_summary(**module_result,
                                                                               model,
                                                                               analyses,
                                                                               page_summaries,
                                                                               config.project_root),
                                             plan,
                                             request);
            }
            return set_evidence_metadata(
                build_evidence_for_module_architecture(**module_result,
                                                       model,
                                                       analyses,
                                                       page_summaries,
                                                       config.project_root),
                plan,
                request);
        }
        case PromptKind::IndexOverview:
            return set_evidence_metadata(build_evidence_for_index_overview(model, page_summaries),
                                         plan,
                                         request);
        case PromptKind::FunctionAnalysis:
        case PromptKind::TypeAnalysis:
        case PromptKind::VariableAnalysis:
        case PromptKind::FunctionDeclarationSummary:
        case PromptKind::FunctionImplementationSummary:
        case PromptKind::TypeDeclarationSummary:
        case PromptKind::TypeImplementationSummary:
            return std::unexpected(GenerateError{
                .message = std::format("unsupported page prompt '{}' for '{}'",
                                       prompt_request_key(request),
                                       target_name.empty() ? plan.page_id : target_name),
            });
    }

    return std::unexpected(GenerateError{
        .message = std::format("unknown prompt kind"),
    });
}

auto prompt_cache_identity_for_page_request(const PagePlan& plan, const PromptRequest& request)
    -> std::string {
    return plan.page_id + ":" + prompt_request_key(request);
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

struct DirectoryEntry {
    std::string relative_path;
    std::string title;
};

auto make_relative_link_target_local(std::string_view current_page_path,
                                     std::string_view target_page_path) -> std::string {
    namespace fs = std::filesystem;

    auto current = fs::path(current_page_path).lexically_normal();
    auto target = fs::path(target_page_path).lexically_normal();
    auto base = current.has_parent_path() ? current.parent_path() : fs::path{"."};
    auto rel = target.lexically_relative(base);
    if(rel.empty()) {
        return target.generic_string();
    }
    return rel.generic_string();
}

auto directory_index_title(std::string_view relative_dir) -> std::string {
    if(relative_dir.empty()) {
        return "API Reference";
    }
    return std::format("Directory `{}`", relative_dir);
}

auto build_directory_index_page(std::string relative_dir,
                                const std::vector<DirectoryEntry>& child_directories,
                                const std::vector<DirectoryEntry>& child_pages) -> GeneratedPage {
    auto title = directory_index_title(relative_dir);

    std::string content;
    content.reserve(1024);
    content += "---\n";
    content += "title: ";
    content += title;
    content += "\n";
    content += "description: Generated directory index.\n";
    content += "layout: doc\n";
    content += "page_template: doc\n";
    content += "---\n\n";
    content += "# ";
    content += title;
    content += "\n\n";

    if(!child_directories.empty()) {
        content += "## Directories\n\n";
        for(const auto& entry: child_directories) {
            content += "- [";
            content += entry.title;
            content += "](";
            content +=
                make_relative_link_target_local(std::string(relative_dir).append("/index.md"),
                                                entry.relative_path);
            content += ")\n";
        }
        content += "\n";
    }

    if(!child_pages.empty()) {
        content += "## Pages\n\n";
        for(const auto& entry: child_pages) {
            content += "- [";
            content += entry.title;
            content += "](";
            content +=
                make_relative_link_target_local(std::string(relative_dir).append("/index.md"),
                                                entry.relative_path);
            content += ")\n";
        }
        content += "\n";
    }

    return GeneratedPage{
        .title = std::move(title),
        .relative_path = std::string(relative_dir).append("/index.md"),
        .content = std::move(content),
    };
}

auto build_directory_index_pages(const std::vector<GeneratedPage>& pages)
    -> std::vector<GeneratedPage> {
    namespace fs = std::filesystem;

    std::unordered_map<std::string, const GeneratedPage*> page_by_path;
    page_by_path.reserve(pages.size());
    std::unordered_set<std::string> all_dirs;

    for(const auto& page: pages) {
        page_by_path.insert_or_assign(page.relative_path, &page);

        auto rel = fs::path(page.relative_path).lexically_normal();
        auto parent = rel.parent_path();
        while(!parent.empty()) {
            all_dirs.insert(parent.generic_string());
            parent = parent.parent_path();
        }
    }

    std::vector<std::string> dirs(all_dirs.begin(), all_dirs.end());
    std::sort(dirs.begin(), dirs.end());

    std::vector<GeneratedPage> generated_indexes;
    generated_indexes.reserve(dirs.size());

    for(const auto& dir: dirs) {
        auto index_path = (fs::path(dir) / "index.md").generic_string();
        if(page_by_path.contains(index_path)) {
            continue;
        }

        std::map<std::string, DirectoryEntry> child_directories;
        std::map<std::string, DirectoryEntry> child_pages;

        for(const auto& page: pages) {
            auto rel = fs::path(page.relative_path).lexically_normal();
            auto parent = rel.parent_path();
            if(parent == fs::path(dir) && rel.filename() != "index.md") {
                auto label = page.title.empty() ? rel.filename().generic_string() : page.title;
                child_pages.emplace(page.relative_path,
                                    DirectoryEntry{
                                        .relative_path = page.relative_path,
                                        .title = std::move(label),
                                    });
            }

            auto maybe_relative = rel.lexically_relative(dir);
            if(maybe_relative.empty()) {
                continue;
            }
            auto it = maybe_relative.begin();
            if(it == maybe_relative.end()) {
                continue;
            }
            auto first = *it;
            ++it;
            if(it == maybe_relative.end()) {
                continue;
            }
            auto child_dir = (fs::path(dir) / first).lexically_normal().generic_string();
            auto child_index_path = (fs::path(child_dir) / "index.md").generic_string();
            auto child_title = directory_index_title(child_dir);
            if(auto explicit_index = page_by_path.find(child_index_path);
               explicit_index != page_by_path.end()) {
                if(!explicit_index->second->title.empty()) {
                    child_title = explicit_index->second->title;
                }
            }
            child_directories.emplace(child_dir,
                                      DirectoryEntry{
                                          .relative_path = child_index_path,
                                          .title = std::move(child_title),
                                      });
        }

        if(child_directories.empty() && child_pages.empty()) {
            continue;
        }

        generated_indexes.push_back(build_directory_index_page(
            dir,
            [&]() {
                std::vector<DirectoryEntry> items;
                items.reserve(child_directories.size());
                for(const auto& [_, entry]: child_directories) {
                    items.push_back(entry);
                }
                return items;
            }(),
            [&]() {
                std::vector<DirectoryEntry> items;
                items.reserve(child_pages.size());
                for(const auto& [_, entry]: child_pages) {
                    items.push_back(entry);
                }
                return items;
            }()));
    }

    return generated_indexes;
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

auto collect_page_summary_texts_from_outputs(
    const PagePlan& plan,
    const std::vector<PromptRequest>& prompt_requests,
    const std::unordered_map<std::string, std::string>& prompt_outputs)
    -> std::unordered_map<std::string, std::string> {
    std::unordered_map<std::string, std::string> summary_texts;
    for(const auto& request: prompt_requests) {
        if(!page_summary_cache_key_for_request(plan, request).has_value()) {
            continue;
        }
        auto output_it = prompt_outputs.find(prompt_request_key(request));
        if(output_it == prompt_outputs.end()) {
            continue;
        }
        auto summary = extract_summary_from_prompt_output(output_it->second);
        if(summary.empty()) {
            continue;
        }
        summary_texts.insert_or_assign(prompt_request_key(request), std::move(summary));
    }
    return summary_texts;
}

auto update_page_summary_cache(PageSummaryCache& summaries,
                               const PagePlan& plan,
                               const std::vector<PromptRequest>& prompt_requests,
                               const std::unordered_map<std::string, std::string>& summary_texts)
    -> void {
    for(const auto& request: prompt_requests) {
        auto cache_key = page_summary_cache_key_for_request(plan, request);
        if(!cache_key.has_value()) {
            continue;
        }
        auto it = summary_texts.find(prompt_request_key(request));
        if(it == summary_texts.end() || it->second.empty()) {
            continue;
        }
        summaries[*cache_key] = it->second;
    }
}

auto collect_page_symbols(const PagePlan& plan, const extract::ProjectModel& model)
    -> std::vector<const extract::SymbolInfo*> {
    std::vector<const extract::SymbolInfo*> symbols;
    std::unordered_set<extract::SymbolID> seen;

    auto append_symbol = [&](extract::SymbolID sym_id) {
        if(!seen.insert(sym_id).second) {
            return;
        }
        auto* sym = extract::lookup_symbol(model, sym_id);
        if(sym == nullptr || !is_page_level_symbol(model, *sym)) {
            return;
        }
        if(!is_type_kind(sym->kind) && !is_function_kind(sym->kind) &&
           !is_variable_kind(sym->kind)) {
            return;
        }
        symbols.push_back(sym);
    };

    switch(plan.page_type) {
        case PageType::Namespace:
            if(!plan.owner_keys.empty()) {
                auto ns_it = model.namespaces.find(plan.owner_keys.front());
                if(ns_it != model.namespaces.end()) {
                    for(auto sym_id: ns_it->second.symbols) {
                        append_symbol(sym_id);
                    }
                }
            }
            break;
        case PageType::Module:
            for(const auto& owner_key: plan.owner_keys) {
                for(auto* module: extract::find_modules_by_name(model, owner_key)) {
                    for(auto sym_id: module->symbols) {
                        append_symbol(sym_id);
                    }
                }
            }
            break;
        case PageType::File:
            for(const auto& owner_key: plan.owner_keys) {
                auto file_it = model.files.find(owner_key);
                if(file_it == model.files.end()) {
                    continue;
                }
                for(auto sym_id: file_it->second.symbols) {
                    append_symbol(sym_id);
                }
            }
            break;
        case PageType::Index: break;
    }

    std::sort(symbols.begin(),
              symbols.end(),
              [](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  if(lhs->qualified_name != rhs->qualified_name) {
                      return lhs->qualified_name < rhs->qualified_name;
                  }
                  if(lhs->signature != rhs->signature) {
                      return lhs->signature < rhs->signature;
                  }
                  return lhs->id < rhs->id;
              });

    return symbols;
}

auto collect_documentable_symbols(const extract::ProjectModel& model)
    -> std::vector<const extract::SymbolInfo*> {
    std::unordered_map<std::string,
                       std::size_t,
                       clore::support::TransparentStringHash,
                       clore::support::TransparentStringEqual>
        file_rank;
    file_rank.reserve(model.file_order.size());
    for(std::size_t index = 0; index < model.file_order.size(); ++index) {
        file_rank.emplace(model.file_order[index], index);
    }

    std::vector<const extract::SymbolInfo*> symbols;
    symbols.reserve(model.symbols.size());
    for(const auto& [symbol_id, symbol]: model.symbols) {
        static_cast<void>(symbol_id);
        if(!is_page_level_symbol(model, symbol)) {
            continue;
        }
        if(symbol_prompt_kinds_for_symbol(symbol).empty()) {
            continue;
        }
        symbols.push_back(&symbol);
    }

    std::sort(symbols.begin(),
              symbols.end(),
              [&](const extract::SymbolInfo* lhs, const extract::SymbolInfo* rhs) {
                  auto lhs_rank_it = file_rank.find(lhs->declaration_location.file);
                  auto rhs_rank_it = file_rank.find(rhs->declaration_location.file);
                  auto lhs_rank = lhs_rank_it != file_rank.end()
                                      ? lhs_rank_it->second
                                      : std::numeric_limits<std::size_t>::max();
                  auto rhs_rank = rhs_rank_it != file_rank.end()
                                      ? rhs_rank_it->second
                                      : std::numeric_limits<std::size_t>::max();
                  if(lhs_rank != rhs_rank) {
                      return lhs_rank < rhs_rank;
                  }
                  if(lhs->declaration_location.line != rhs->declaration_location.line) {
                      return lhs->declaration_location.line < rhs->declaration_location.line;
                  }
                  if(lhs->qualified_name != rhs->qualified_name) {
                      return lhs->qualified_name < rhs->qualified_name;
                  }
                  if(lhs->signature != rhs->signature) {
                      return lhs->signature < rhs->signature;
                  }
                  return lhs->id < rhs->id;
              });

    return symbols;
}

auto render_generated_pages(const PagePlan& plan,
                            const config::TaskConfig& config,
                            const extract::ProjectModel& model,
                            const std::unordered_map<std::string, std::string>& prompt_outputs,
                            const SymbolAnalysisStore& analyses,
                            const LinkResolver& links)
    -> std::expected<std::vector<GeneratedPage>, GenerateError> {
    auto render_result = render_page_bundle(plan, config, model, prompt_outputs, analyses, links);
    if(!render_result.has_value()) {
        return std::unexpected(GenerateError{
            .message = std::format("failed to render page '{}': {}",
                                   plan.page_id,
                                   render_result.error().message),
        });
    }

    return std::move(*render_result);
}

auto prepare_generation_context(const config::TaskConfig& config,
                                const extract::ProjectModel& model)
    -> std::expected<PreparedGenerationContext, GenerateError> {
    auto plan_result = build_page_plan_set(config, model);
    if(!plan_result.has_value()) {
        return std::unexpected(GenerateError{.message = plan_result.error().message});
    }

    PreparedGenerationContext context{
        .plan_set = std::move(*plan_result),
        .links = {},
        .id_to_plan = {},
        .prompt_requests_by_plan = {},
        .symbol_targets_by_plan = {},
        .symbol_analysis_targets = {},
    };
    context.links = build_link_resolver(context.plan_set);
    context.id_to_plan.reserve(context.plan_set.plans.size());
    context.prompt_requests_by_plan.reserve(context.plan_set.plans.size());
    context.symbol_targets_by_plan.reserve(context.plan_set.plans.size());

    for(const auto* sym: collect_documentable_symbols(model)) {
        auto prompt_kinds = symbol_prompt_kinds_for_symbol(*sym);
        if(prompt_kinds.empty()) {
            continue;
        }
        context.symbol_analysis_targets.push_back(PreparedSymbolAnalysisTarget{
            .symbol_id = sym->id,
            .target_key = make_symbol_target_key(*sym),
            .prompt_kinds = std::move(prompt_kinds),
        });
    }

    for(std::size_t i = 0; i < context.plan_set.plans.size(); ++i) {
        context.id_to_plan[context.plan_set.plans[i].page_id] = i;
        context.prompt_requests_by_plan.push_back(
            deduplicate_prompt_requests(context.plan_set.plans[i]));

        std::vector<std::string> symbol_targets;
        for(const auto* sym: collect_page_symbols(context.plan_set.plans[i], model)) {
            symbol_targets.push_back(make_symbol_target_key(*sym));
        }
        context.symbol_targets_by_plan.push_back(std::move(symbol_targets));
    }

    std::sort(context.symbol_analysis_targets.begin(),
              context.symbol_analysis_targets.end(),
              [](const PreparedSymbolAnalysisTarget& lhs, const PreparedSymbolAnalysisTarget& rhs) {
                  return lhs.target_key < rhs.target_key;
              });

    logging::info("page plan: {} pages, generation order size {}",
                  context.plan_set.plans.size(),
                  context.plan_set.generation_order.size());
    std::size_t symbol_prompt_count = 0;
    for(const auto& target: context.symbol_analysis_targets) {
        symbol_prompt_count += target.prompt_kinds.size();
    }
    logging::info("symbol analysis: {} unique targets, {} prompt tasks",
                  context.symbol_analysis_targets.size(),
                  symbol_prompt_count);

    return context;
}

auto prepare_symbol_analyses_for_dry_run(const PreparedGenerationContext& context,
                                         const extract::ProjectModel& model)
    -> PreparedSymbolAnalyses {
    PreparedSymbolAnalyses prepared;
    for(const auto& target: context.symbol_analysis_targets) {
        auto* sym = extract::lookup_symbol(model, target.symbol_id);
        if(sym == nullptr) {
            continue;
        }
        store_fallback_analysis(prepared.analyses, *sym, model);
    }
    return prepared;
}

struct SymbolAnalysisWork {
    extract::SymbolID symbol_id{};
    std::string target_key;
    PromptKind kind = PromptKind::FunctionAnalysis;
};

struct PagePromptWork {
    std::size_t state_index = 0;
    std::string output_key;
    std::string cache_identity;
    PromptRequest template_request;
};

struct RenderPageWork {
    std::size_t state_index = 0;
};

struct PreparedPrompt {
    clore::net::PromptRequest request;
    std::string persistent_cache_key;
};

using ScheduledWork = std::variant<SymbolAnalysisWork, PagePromptWork, RenderPageWork>;

struct PageState {
    std::size_t plan_index = 0;
    std::size_t total_prompts = 0;
    std::size_t completed_prompts = 0;
    std::size_t pending_symbol_analyses = 0;
    std::size_t unsatisfied_deps = 0;
    std::string failure;
    bool submitted = false;
    bool written = false;
    bool failed = false;
    std::unordered_map<std::string, std::string> prompt_outputs;
};

using SymbolWaitMap = std::unordered_map<std::string,
                                         std::vector<std::size_t>,
                                         clore::support::TransparentStringHash,
                                         clore::support::TransparentStringEqual>;
using SymbolPendingMap = std::unordered_map<std::string,
                                            std::size_t,
                                            clore::support::TransparentStringHash,
                                            clore::support::TransparentStringEqual>;

// ---------------------------------------------------------------------------
// WorkQueue: priority deque with deferred-symbol staging and close/abort signalling.
// Page prompts are enqueued at the front for early rendering; symbol analysis
// goes to the back. A semaphore is used as a counted wakeup so one enqueued
// item resumes one worker; kota::event is broadcast-style and can re-enter all
// workers for a single item.
// ---------------------------------------------------------------------------
class WorkQueue {
public:
    auto enqueue(ScheduledWork work) -> void {
        if(stopped_) {
            return;
        }
        if(std::holds_alternative<PagePromptWork>(work)) {
            queue_.push_front(std::move(work));
        } else {
            queue_.push_back(std::move(work));
        }
        available_.release();
    }

    auto enqueue_deferred(SymbolAnalysisWork work) -> void {
        if(stopped_) {
            return;
        }
        deferred_.push_back(std::move(work));
    }

    auto flush_deferred() -> void {
        if(stopped_) {
            return;
        }
        while(!deferred_.empty()) {
            enqueue(std::move(deferred_.front()));
            deferred_.pop_front();
        }
    }

    auto has_deferred() const -> bool {
        return !deferred_.empty();
    }

    auto empty() const -> bool {
        return queue_.empty();
    }

    auto stopped() const -> bool {
        return stopped_;
    }

    auto dequeue() -> std::optional<ScheduledWork> {
        if(stopped_ || queue_.empty()) {
            return std::nullopt;
        }
        auto work = std::move(queue_.front());
        queue_.pop_front();
        return work;
    }

    auto close(std::size_t worker_count) -> void {
        if(stopped_) {
            return;
        }
        stopped_ = true;
        available_.release(static_cast<std::ptrdiff_t>(worker_count));
    }

    auto abort(std::size_t worker_count) -> void {
        if(stopped_) {
            return;
        }
        stopped_ = true;
        queue_.clear();
        deferred_.clear();
        available_.release(static_cast<std::ptrdiff_t>(worker_count));
    }

    auto available() -> kota::semaphore& {
        return available_;
    }

private:
    std::deque<ScheduledWork> queue_;
    std::deque<SymbolAnalysisWork> deferred_;
    kota::semaphore available_;
    bool stopped_ = false;
};

// ---------------------------------------------------------------------------
// DependencyTracker: page state, inter-page dependencies, symbol readiness.
// ---------------------------------------------------------------------------
class DependencyTracker {
public:
    explicit DependencyTracker(const PreparedGenerationContext& context) {
        states_.reserve(context.plan_set.generation_order.size());
        id_to_state_.reserve(context.plan_set.generation_order.size());

        for(const auto& page_id: context.plan_set.generation_order) {
            auto plan_it = context.id_to_plan.find(page_id);
            if(plan_it == context.id_to_plan.end()) {
                continue;
            }

            auto state_index = states_.size();
            id_to_state_[page_id] = state_index;
            states_.push_back(PageState{
                .plan_index = plan_it->second,
                .total_prompts = context.prompt_requests_by_plan[plan_it->second].size(),
                .pending_symbol_analyses = context.symbol_targets_by_plan[plan_it->second].size(),
                .prompt_outputs = {},
            });

            for(const auto& target_key: context.symbol_targets_by_plan[plan_it->second]) {
                pages_waiting_on_symbol_[target_key].push_back(state_index);
            }
        }

        for(std::size_t i = 0; i < states_.size(); ++i) {
            const auto& plan = context.plan_set.plans[states_[i].plan_index];
            for(const auto& dep_id: plan.depends_on_pages) {
                if(id_to_state_.contains(dep_id)) {
                    states_[i].unsatisfied_deps++;
                    dependents_[dep_id].push_back(i);
                }
            }
        }
    }

    auto state_at(std::size_t idx) -> PageState& {
        return states_[idx];
    }

    auto state_count() const -> std::size_t {
        return states_.size();
    }

    auto initialize_ready_candidates() -> void {
        ready_candidates_.reserve(states_.size());
        for(std::size_t i = 0; i < states_.size(); ++i) {
            ready_candidates_.push_back(i);
        }
    }

    auto add_pending_symbols(std::string_view target_key, std::size_t count) -> void {
        pending_symbol_prompts_[std::string(target_key)] = count;
    }

    auto mark_symbol_ready(std::string_view target_key) -> void {
        auto it = pages_waiting_on_symbol_.find(target_key);
        if(it == pages_waiting_on_symbol_.end()) {
            return;
        }
        for(auto state_index: it->second) {
            if(states_[state_index].pending_symbol_analyses > 0) {
                --states_[state_index].pending_symbol_analyses;
                if(states_[state_index].pending_symbol_analyses == 0) {
                    ready_candidates_.push_back(state_index);
                }
            }
        }
    }

    auto finish_symbol_prompt(std::string_view target_key) -> void {
        auto pending_it = pending_symbol_prompts_.find(target_key);
        if(pending_it == pending_symbol_prompts_.end()) {
            mark_symbol_ready(target_key);
            return;
        }

        if(pending_it->second > 1) {
            --pending_it->second;
            return;
        }

        pending_symbol_prompts_.erase(pending_it);
        mark_symbol_ready(target_key);
    }

    auto mark_page_written(std::size_t state_idx, std::string_view page_id) -> void {
        if(states_[state_idx].written || states_[state_idx].failed) {
            return;
        }
        states_[state_idx].written = true;
        release_dependents(page_id);
    }

    auto mark_page_failed(std::size_t state_idx, std::string_view page_id, std::string_view reason)
        -> void {
        if(states_[state_idx].written || states_[state_idx].failed) {
            return;
        }
        states_[state_idx].failed = true;
        states_[state_idx].failure = std::string(reason);
        release_dependents(page_id);
    }

    auto finished_count() const -> std::size_t {
        return std::ranges::count_if(states_, [](const PageState& state) {
            return state.written || state.failed;
        });
    }

    auto failed_count() const -> std::size_t {
        return std::ranges::count_if(states_, [](const PageState& state) { return state.failed; });
    }

    auto failures(const PreparedGenerationContext& context) const -> std::vector<std::string> {
        std::vector<std::string> messages;
        for(const auto& state: states_) {
            if(!state.failed) {
                continue;
            }
            const auto& page_id = context.plan_set.plans.at(state.plan_index).page_id;
            messages.push_back(std::format("{}: {}", page_id, state.failure));
        }
        return messages;
    }

private:
    auto release_dependents(std::string_view page_id) -> void {
        auto dep_it = dependents_.find(std::string(page_id));
        if(dep_it != dependents_.end()) {
            for(auto dep_state_idx: dep_it->second) {
                if(states_[dep_state_idx].unsatisfied_deps > 0) {
                    --states_[dep_state_idx].unsatisfied_deps;
                    ready_candidates_.push_back(dep_state_idx);
                }
            }
        }
    }

public:
    auto pop_ready_candidate() -> std::optional<std::size_t> {
        while(!ready_candidates_.empty()) {
            auto i = ready_candidates_.back();
            ready_candidates_.pop_back();
            if(i >= states_.size()) {
                continue;
            }
            auto& state = states_[i];
            if(state.written || state.failed || state.submitted || state.unsatisfied_deps > 0 ||
               state.pending_symbol_analyses > 0) {
                continue;
            }
            return i;
        }
        return std::nullopt;
    }

private:
    std::vector<PageState> states_;
    std::unordered_map<std::string, std::size_t> id_to_state_;
    SymbolWaitMap pages_waiting_on_symbol_;
    SymbolPendingMap pending_symbol_prompts_;
    std::unordered_map<std::string, std::vector<std::size_t>> dependents_;
    std::vector<std::size_t> ready_candidates_;
};

// ---------------------------------------------------------------------------
// PageRenderer: page output writing, dry-run collection, page summary cache.
// ---------------------------------------------------------------------------
class PageRenderer {
public:
    PageRenderer(std::string_view output_root, bool dry_run) :
        output_root_(output_root), dry_run_(dry_run) {}

    auto emit_pages(std::span<const GeneratedPage> pages)
        -> std::expected<std::size_t, GenerateError> {
        for(const auto& page: pages) {
            if(dry_run_) {
                dry_run_pages_.push_back(page);
            } else {
                auto write_result = write_page(page, output_root_);
                if(!write_result.has_value()) {
                    return std::unexpected(GenerateError{.message = write_result.error().message});
                }
                logging::info("  written {}", page.relative_path);
            }
        }
        ++written_page_count_;
        written_output_count_ += pages.size();
        return pages.size();
    }

    auto emit_pages_async(std::span<const GeneratedPage> pages, kota::event_loop& loop)
        -> kota::task<std::size_t, GenerateError> {
        for(const auto& page: pages) {
            if(dry_run_) {
                dry_run_pages_.push_back(page);
            } else {
                auto write_result = co_await kota::queue(
                                        [page, output_root = output_root_]() {
                                            return write_page(page, output_root);
                                        },
                                        loop)
                                        .catch_cancel();
                if(write_result.is_cancelled()) {
                    co_await kota::fail(GenerateError{
                        .message = std::format("page write cancelled for '{}'", page.relative_path),
                    });
                }
                if(write_result.has_error()) {
                    co_await kota::fail(GenerateError{
                        .message = std::format("page write task failed for '{}': {}",
                                               page.relative_path,
                                               write_result.error().message()),
                    });
                }
                if(!write_result->has_value()) {
                    co_await kota::fail(GenerateError{.message = write_result->error().message});
                }
                logging::info("  written {}", page.relative_path);
            }
        }
        ++written_page_count_;
        written_output_count_ += pages.size();
        co_return pages.size();
    }

    auto update_summaries(const PagePlan& plan,
                          const std::vector<PromptRequest>& prompt_requests,
                          const std::unordered_map<std::string, std::string>& prompt_outputs)
        -> void {
        std::unique_lock lock(page_summaries_mutex_);
        update_page_summary_cache(
            page_summaries_,
            plan,
            prompt_requests,
            collect_page_summary_texts_from_outputs(plan, prompt_requests, prompt_outputs));
    }

    auto summaries_snapshot() -> PageSummaryCache {
        std::shared_lock lock(page_summaries_mutex_);
        return page_summaries_;
    }

    auto written_page_count() const -> std::size_t {
        return written_page_count_;
    }

    auto written_output_count() const -> std::size_t {
        return written_output_count_;
    }

    auto increment_output_count() -> void {
        ++written_output_count_;
    }

    auto dry_run() const -> bool {
        return dry_run_;
    }

    auto output_root() const -> std::string_view {
        return output_root_;
    }

    auto dry_run_pages() const -> const std::vector<GeneratedPage>& {
        return dry_run_pages_;
    }

private:
    std::string output_root_;
    bool dry_run_;
    std::size_t written_page_count_ = 0;
    std::size_t written_output_count_ = 0;
    std::vector<GeneratedPage> dry_run_pages_;
    PageSummaryCache page_summaries_;
    mutable std::shared_mutex page_summaries_mutex_;
};

// ---------------------------------------------------------------------------
// PageGenerationScheduler: orchestrates work queue, dependency tracking, and
// page rendering for the full generation pipeline.
// ---------------------------------------------------------------------------
class PageGenerationScheduler {
public:
    PageGenerationScheduler(const config::TaskConfig& config,
                            const extract::ProjectModel& model,
                            const PreparedGenerationContext& context,
                            std::string_view llm_model,
                            std::uint32_t rate_limit,
                            std::string_view output_root,
                            kota::event_loop& loop,
                            bool dry_run = false) :
        config_(config), model_(model), context_(context), model_version_(llm_model),
        worker_count_(rate_limit), loop_(loop), scope_(loop), tracker_(context), renderer_(output_root, dry_run) {
    }

    auto run() -> kota::task<GenerationSummary, GenerateError> {
        {
            auto cache_result =
                co_await cache::load_cache_index_async(std::string(config_.workspace_root), loop_)
                    .catch_cancel();
            if(cache_result.has_value()) {
                cache_index_ = std::move(*cache_result);
                logging::info("loaded prompt response cache: {} entries",
                              cache_index_->entries.size());
            } else if(cache_result.is_cancelled()) {
                logging::warn("prompt response cache load cancelled");
            } else {
                logging::warn("failed to load prompt response cache: {}",
                              cache_result.error().message);
            }
        }

        {
            std::size_t total_symbol_prompts = 0;
            for(const auto& target: context_.symbol_analysis_targets) {
                total_symbol_prompts += target.prompt_kinds.size();
            }
            std::size_t total_page_prompts = 0;
            for(const auto& requests: context_.prompt_requests_by_plan) {
                total_page_prompts += requests.size();
            }
            auto total = total_symbol_prompts + total_page_prompts;
            expected_llm_requests_.store(total, std::memory_order_relaxed);
            if(total > 0) {
                logging::info("expected LLM requests: {} symbol + {} page = {} total",
                              total_symbol_prompts,
                              total_page_prompts,
                              total);
            }
        }

        for(std::size_t i = 0; i < worker_count_; ++i) {
            scope_.spawn(worker_task());
        }

        for(const auto& target: context_.symbol_analysis_targets) {
            auto schedule_result = schedule_symbol_analysis(target);
            if(!schedule_result.has_value()) {
                co_await kota::fail(std::move(schedule_result.error()));
            }
        }

        maybe_flush_deferred();

        tracker_.initialize_ready_candidates();

        co_await try_submit_ready_pages().or_fail();

        maybe_stop_workers();

        auto scope_result = co_await scope_.join();
        if(scope_result.has_error()) {
            co_await kota::fail(std::move(scope_result.error()[0]));
        }

        if(retry_limit_exceeded()) {
            co_await kota::fail(GenerateError{
                .message = std::format("generation stopped after {} consecutive failures",
                                       config_.llm.retry_limit),
            });
        }

        if(tracker_.finished_count() != tracker_.state_count()) {
            co_await kota::fail(GenerateError{
                .message = std::format("generation stopped with {}/{} page states finished",
                                       tracker_.finished_count(),
                                       tracker_.state_count()),
            });
        }

        if(!renderer_.dry_run()) {
            auto llms_page = build_llms_page(context_.plan_set, config_, {});
            auto relative_path = llms_page.relative_path;
            auto llms_result = co_await kota::queue(
                                   [page = std::move(llms_page),
                                    output_root = std::string(renderer_.output_root())]() {
                                       return write_page(page, output_root);
                                   },
                                   loop_)
                                   .catch_cancel();
            if(llms_result.is_cancelled()) {
                co_await kota::fail(GenerateError{
                    .message = std::format("LLMs page write cancelled for '{}'", relative_path),
                });
            }
            if(llms_result.has_error()) {
                co_await kota::fail(GenerateError{
                    .message = std::format("LLMs page write task failed for '{}': {}",
                                           relative_path,
                                           llms_result.error().message()),
                });
            }
            if(!llms_result->has_value()) {
                co_await kota::fail(GenerateError{.message = llms_result->error().message});
            }
            logging::info("  written {}", relative_path);
            renderer_.increment_output_count();
        }

        co_return GenerationSummary{
            .written_output_count = renderer_.written_output_count(),
            .symbol_analysis_cache_hits =
                symbol_analysis_cache_hits_.load(std::memory_order_relaxed),
            .symbol_analysis_cache_misses =
                symbol_analysis_cache_misses_.load(std::memory_order_relaxed),
            .page_prompt_cache_hits = page_prompt_cache_hits_.load(std::memory_order_relaxed),
            .page_prompt_cache_misses = page_prompt_cache_misses_.load(std::memory_order_relaxed),
        };
    }

    auto dry_run_pages() const -> const std::vector<GeneratedPage>& {
        return renderer_.dry_run_pages();
    }

private:
    struct WorkerActivity {
        explicit WorkerActivity(PageGenerationScheduler& scheduler) : scheduler_(scheduler) {
            scheduler_.in_flight_work_.fetch_add(1, std::memory_order_relaxed);
        }

        WorkerActivity(const WorkerActivity&) = delete;
        auto operator=(const WorkerActivity&) -> WorkerActivity& = delete;

        WorkerActivity(WorkerActivity&&) = delete;
        auto operator=(WorkerActivity&&) -> WorkerActivity& = delete;

        ~WorkerActivity() {
            scheduler_.in_flight_work_.fetch_sub(1, std::memory_order_relaxed);
            scheduler_.maybe_stop_workers();
        }

    private:
        PageGenerationScheduler& scheduler_;
    };

    auto reset_consecutive_failures(std::string_view reason) -> void {
        auto previous = consecutive_failures_.exchange(0, std::memory_order_relaxed);
        if(previous > 0) {
            logging::info("reset consecutive generation failures after {}: {} -> 0",
                          reason,
                          previous);
        }
    }

    auto record_consecutive_failure(std::string_view owner, std::string_view reason) -> bool {
        auto count = consecutive_failures_.fetch_add(1, std::memory_order_relaxed) + 1;
        logging::warn("consecutive generation failure {}/{} at '{}': {}",
                      count,
                      config_.llm.retry_limit,
                      owner,
                      reason);
        if(count <= config_.llm.retry_limit) {
            return false;
        }

        retry_limit_exceeded_.store(true, std::memory_order_relaxed);
        work_queue_.abort(worker_count_);
        logging::err("generation failure limit exceeded after {} consecutive failures", count);
        return true;
    }

    auto retry_limit_exceeded() const -> bool {
        return retry_limit_exceeded_.load(std::memory_order_relaxed);
    }

    auto schedule_symbol_analysis(const PreparedSymbolAnalysisTarget& target)
        -> std::expected<void, GenerateError> {
        auto* sym = extract::lookup_symbol(model_, target.symbol_id);
        if(sym == nullptr) {
            logging::warn("skipping missing symbol analysis target '{}'", target.target_key);
            tracker_.mark_symbol_ready(target.target_key);
            return {};
        }

        store_fallback_analysis(prepared_analyses_.analyses, *sym, model_);

        std::size_t pending_prompt_count = 0;
        for(auto kind: target.prompt_kinds) {
            SymbolAnalysisWork work{
                .symbol_id = sym->id,
                .target_key = target.target_key,
                .kind = kind,
            };

            ++pending_prompt_count;
            if(is_declaration_summary_prompt(kind)) {
                work_queue_.enqueue_deferred(std::move(work));
                continue;
            }

            if(is_base_symbol_analysis_prompt(kind)) {
                ++pending_base_symbol_prompts_;
            }

            work_queue_.enqueue(std::move(work));
        }

        if(pending_prompt_count == 0) {
            tracker_.mark_symbol_ready(target.target_key);
            return {};
        }

        tracker_.add_pending_symbols(target.target_key, pending_prompt_count);
        return {};
    }

    auto perform_prompt_request(std::string_view owner, clore::net::PromptRequest request)
        -> kota::task<std::string, GenerateError> {
        if(renderer_.dry_run()) {
            co_return wrap_prompt_output_for_embed(std::string(owner), request.prompt);
        }

        auto issued = llm_requests_issued_.fetch_add(1, std::memory_order_relaxed) + 1;
        auto expected = expected_llm_requests_.load(std::memory_order_relaxed);
        if(expected > 0) {
            logging::info("generation prompt ({}/{}): {}", issued, expected, owner);
        } else {
            logging::info("generation prompt #{}: {}", issued, owner);
        }

        auto result = co_await request_llm_async(model_version_,
                                                 config_.llm.system_prompt,
                                                 request,
                                                 loop_)
                          .catch_cancel();

        auto completed = llm_requests_completed_.fetch_add(1, std::memory_order_relaxed) + 1;
        auto expected_after = expected_llm_requests_.load(std::memory_order_relaxed);
        if(expected_after > 0) {
            logging::info("completed generation prompt ({}/{}): {}",
                          completed,
                          expected_after,
                          owner);
        }

        if(!result.is_cancelled() && !result.has_error()) {
            reset_consecutive_failures("successful LLM request");
            co_return clore::support::ensure_utf8(*result);
        }

        auto message = result.is_cancelled() ? std::string("LLM request cancelled")
                                             : result.error().message;
        co_await kota::fail(GenerateError{
            .message = std::format("LLM request failed for '{}': {}", owner, message),
        });
    }

    template <typename Callable>
    auto run_queued_worker_call(Callable callable, std::string operation_name)
        -> kota::task<std::invoke_result_t<Callable>, GenerateError> {
        auto queued = co_await kota::queue(std::move(callable), loop_).catch_cancel();
        if(queued.is_cancelled()) {
            co_await kota::fail(
                GenerateError{.message = std::format("{} cancelled", operation_name)});
        }
        if(queued.has_error()) {
            co_await kota::fail(GenerateError{
                .message =
                    std::format("{} worker failed: {}", operation_name, queued.error().message()),
            });
        }
        co_return std::move(*queued);
    }

    auto worker_task() -> kota::task<void, GenerateError> {
        while(true) {
            co_await work_queue_.available().acquire();

            auto work = work_queue_.dequeue();
            if(!work.has_value()) {
                if(work_queue_.stopped()) {
                    co_return;
                }
                co_await kota::fail(
                    GenerateError{.message = "generation worker woke without queued work"});
            }

            WorkerActivity activity(*this);

            if(std::holds_alternative<SymbolAnalysisWork>(*work)) {
                auto item = std::move(std::get<SymbolAnalysisWork>(*work));
                auto result = co_await run_symbol_analysis_task(item).catch_cancel();
                if(!result.is_cancelled() && !result.has_error()) {
                    reset_consecutive_failures("successful symbol analysis");
                } else {
                    auto reason = result.is_cancelled() ? std::string("symbol analysis was cancelled")
                                                        : result.error().message;
                    logging::warn("symbol analysis work '{}' failed: {}", item.target_key, reason);
                    auto limit_reached = record_consecutive_failure(item.target_key, reason);
                    if(limit_reached) {
                        co_return;
                    }
                    work_queue_.enqueue(std::move(item));
                }
            } else if(std::holds_alternative<PagePromptWork>(*work)) {
                auto item = std::move(std::get<PagePromptWork>(*work));
                auto result = co_await run_page_prompt_task(item).catch_cancel();
                if(!result.is_cancelled() && !result.has_error()) {
                    reset_consecutive_failures("successful page prompt");
                } else {
                    auto reason = result.is_cancelled() ? std::string("page prompt was cancelled")
                                                        : result.error().message;
                    logging::warn("page prompt work '{}' failed: {}", item.cache_identity, reason);
                    auto limit_reached = record_consecutive_failure(item.cache_identity, reason);
                    if(limit_reached) {
                        co_await fail_page_work(item.state_index, reason).catch_cancel();
                        co_return;
                    }
                    work_queue_.enqueue(std::move(item));
                }
            } else {
                auto item = std::get<RenderPageWork>(*work);
                auto result = co_await render_ready_page(item.state_index).catch_cancel();
                if(!result.is_cancelled() && !result.has_error()) {
                    reset_consecutive_failures("successful render");
                } else {
                    auto reason = result.is_cancelled() ? std::string("render was cancelled")
                                                        : result.error().message;
                    logging::warn("render work for state {} failed: {}", item.state_index, reason);
                    auto limit_reached = record_consecutive_failure("render", reason);
                    if(limit_reached) {
                        co_await fail_page_work(item.state_index, reason).catch_cancel();
                        co_return;
                    }
                    work_queue_.enqueue(std::move(item));
                }
            }
        }
    }

    auto submit_after_symbol_analysis(const SymbolAnalysisWork& work)
        -> kota::task<void, GenerateError> {
        if(is_base_symbol_analysis_prompt(work.kind)) {
            finish_base_symbol_prompt(work.kind);
        }
        tracker_.finish_symbol_prompt(work.target_key);
        co_await try_submit_ready_pages().or_fail();
        co_return;
    }

    auto fail_page_work(std::size_t state_index, std::string_view reason)
        -> kota::task<void, GenerateError> {
        auto& state = tracker_.state_at(state_index);
        auto& plan = context_.plan_set.plans[state.plan_index];
        tracker_.mark_page_failed(state_index, plan.page_id, reason);
        logging::warn("page '{}' failed and dependents were released: {}", plan.page_id, reason);
        maybe_stop_workers();
        co_await try_submit_ready_pages().or_fail();
        co_return;
    }

    auto run_symbol_analysis_task(SymbolAnalysisWork work) -> kota::task<void, GenerateError> {
        auto* sym = extract::lookup_symbol(model_, work.symbol_id);
        if(sym == nullptr) {
            co_await kota::fail(GenerateError{
                .message = std::format("symbol analysis target '{}' no longer resolves",
                                       work.target_key),
            });
        }

        auto output_key = prompt_request_key(PromptRequest{
            .kind = work.kind,
            .target_key = work.target_key,
        });

        auto prompt_result =
            co_await run_queued_worker_call(
                         [this, sym, kind = work.kind]() {
                             std::shared_lock lock(analyses_mutex_);
                             return build_symbol_analysis_prompt(*sym,
                                                                 kind,
                                                                 model_,
                                                                 config_,
                                                                 prepared_analyses_.analyses);
                         },
                         std::format("analysis prompt build for '{}'", work.target_key))
                .or_fail();
        if(!prompt_result.has_value()) {
            co_await kota::fail(GenerateError{
                .message = std::format("analysis prompt build failed for '{}' ({}) : {}",
                                       work.target_key,
                                       prompt_kind_name(work.kind),
                                       prompt_result.error().message),
            });
        }

        if(renderer_.dry_run()) {
            co_await submit_after_symbol_analysis(work).or_fail();
            co_return;
        }

        auto request_result = clore::net::PromptRequest{
            .prompt = std::move(*prompt_result),
            .output_contract = clore::net::PromptOutputContract::Json,
        };
        if(work.kind == PromptKind::FunctionDeclarationSummary ||
           work.kind == PromptKind::FunctionImplementationSummary ||
           work.kind == PromptKind::TypeDeclarationSummary ||
           work.kind == PromptKind::TypeImplementationSummary) {
            request_result.output_contract = clore::net::PromptOutputContract::Markdown;
        }

        auto persistent_cache_key = cache::make_prompt_response_cache_key(output_key,
                                                                          config_.llm.system_prompt,
                                                                          request_result);
        if(!persistent_cache_key.has_value()) {
            co_await kota::fail(make_generate_error(std::move(persistent_cache_key.error())));
        }

        {
            std::unique_lock lock(cache_index_mutex_);
            if(cache_index_.has_value()) {
                auto cached = cache::find_cached_response(*cache_index_, *persistent_cache_key);
                if(cached.has_value() && !cached->empty()) {
                    auto cached_response = std::string(*cached);
                    lock.unlock();
                    auto apply_result = [&]() {
                        std::unique_lock lock(analyses_mutex_);
                        return apply_symbol_analysis_response(prepared_analyses_.analyses,
                                                              *sym,
                                                              model_,
                                                              work.kind,
                                                              cached_response);
                    }();
                    if(apply_result.has_value()) {
                        logging::debug("cache hit for symbol analysis '{}'", output_key);
                        symbol_analysis_cache_hits_.fetch_add(1, std::memory_order_relaxed);
                        auto prev = expected_llm_requests_.fetch_sub(1, std::memory_order_relaxed);
                        if(prev == 0) {
                            expected_llm_requests_.fetch_add(1, std::memory_order_relaxed);
                        }
                        co_await submit_after_symbol_analysis(work).or_fail();
                        co_return;
                    }
                    symbol_analysis_cache_misses_.fetch_add(1, std::memory_order_relaxed);
                    logging::warn("ignoring invalid cached analysis '{}': {}",
                                  output_key,
                                  apply_result.error().message);
                } else {
                    logging::debug("cache miss for symbol analysis '{}'", output_key);
                    symbol_analysis_cache_misses_.fetch_add(1, std::memory_order_relaxed);
                }
            } else {
                logging::debug("cache disabled for symbol analysis '{}'", output_key);
            }
        }

        auto raw_result =
            co_await perform_prompt_request(output_key, std::move(request_result)).catch_cancel();

        if(raw_result.is_cancelled() || raw_result.has_error()) {
            auto message = raw_result.is_cancelled() ? std::string("LLM request cancelled")
                                                     : raw_result.error().message;
            co_await kota::fail(GenerateError{
                .message = std::format("symbol analysis LLM request failed for '{}': {}",
                                       output_key,
                                       message),
            });
        }

        auto apply_result = [&]() {
            std::unique_lock lock(analyses_mutex_);
            return apply_symbol_analysis_response(prepared_analyses_.analyses,
                                                  *sym,
                                                  model_,
                                                  work.kind,
                                                  *raw_result);
        }();
        if(!apply_result.has_value()) {
            co_await kota::fail(GenerateError{
                .message = std::format("symbol analysis response apply failed for '{}': {}",
                                       output_key,
                                       apply_result.error().message),
            });
        }

        if(cache_index_.has_value()) {
            auto save_result =
                co_await cache::save_cache_entry_async(std::string(config_.workspace_root),
                                                       *persistent_cache_key,
                                                       *raw_result,
                                                       loop_)
                    .catch_cancel();
            if(save_result.is_cancelled()) {
                logging::warn("failed to save cached analysis '{}': cache save cancelled",
                              output_key);
            } else if(save_result.has_error()) {
                logging::warn("failed to save cached analysis '{}': {}",
                              output_key,
                              save_result.error().message);
            } else {
                std::unique_lock lock(cache_index_mutex_);
                cache_index_.value().entries.insert_or_assign(std::string(*persistent_cache_key),
                                                              std::string(*raw_result));
            }
        }

        co_await submit_after_symbol_analysis(work).or_fail();
    }

    auto finish_base_symbol_prompt(PromptKind kind) -> void {
        if(!is_base_symbol_analysis_prompt(kind)) {
            return;
        }
        if(pending_base_symbol_prompts_ > 0) {
            --pending_base_symbol_prompts_;
        }
        maybe_flush_deferred();
    }

    auto run_page_prompt_task(PagePromptWork work) -> kota::task<void, GenerateError> {
        auto plan_index = tracker_.state_at(work.state_index).plan_index;
        auto& plan = context_.plan_set.plans[plan_index];

        auto page_summaries_snapshot = renderer_.summaries_snapshot();

        auto evidence_result =
            co_await run_queued_worker_call(
                         [this,
                          plan_index,
                          request = work.template_request,
                          page_summaries_snapshot = std::move(page_summaries_snapshot)]() mutable {
                             std::shared_lock lock(analyses_mutex_);
                             return build_evidence_for_request(request,
                                                               context_.plan_set.plans[plan_index],
                                                               model_,
                                                               config_,
                                                               prepared_analyses_.analyses,
                                                               page_summaries_snapshot);
                         },
                         std::format("page evidence build for '{}'", work.cache_identity))
                .or_fail();

        if(!evidence_result.has_value()) {
            co_await kota::fail(GenerateError{
                .message = std::format("page evidence build failed for '{}': {}",
                                       work.cache_identity,
                                       evidence_result.error().message),
            });
        }

        auto prompt_result = build_prompt(work.template_request.kind, *evidence_result);
        if(!prompt_result.has_value()) {
            co_await kota::fail(GenerateError{
                .message = std::format("page prompt build failed for '{}': {}",
                                       work.cache_identity,
                                       prompt_result.error().message),
            });
        }

        auto request = clore::net::PromptRequest{
            .prompt = std::move(*prompt_result),
            .output_contract = clore::net::PromptOutputContract::Markdown,
        };

        auto persistent_cache_key = cache::make_prompt_response_cache_key(work.cache_identity,
                                                                          config_.llm.system_prompt,
                                                                          request);
        if(!persistent_cache_key.has_value()) {
            co_await kota::fail(GenerateError{
                .message = std::format("page prompt cache key failed for '{}': {}",
                                       work.cache_identity,
                                       persistent_cache_key.error().message),
            });
        }

        {
            std::unique_lock lock(cache_index_mutex_);
            if(cache_index_.has_value()) {
                auto cached = cache::find_cached_response(*cache_index_, *persistent_cache_key);
                if(cached.has_value() && !cached->empty()) {
                    auto cached_response = std::string(*cached);
                    lock.unlock();
                    auto parsed_output =
                        parse_markdown_prompt_output(cached_response, work.cache_identity);
                    if(parsed_output.has_value()) {
                        logging::debug("cache hit for page prompt '{}'", work.cache_identity);
                        page_prompt_cache_hits_.fetch_add(1, std::memory_order_relaxed);
                        auto prev = expected_llm_requests_.fetch_sub(1, std::memory_order_relaxed);
                        if(prev == 0) {
                            expected_llm_requests_.fetch_add(1, std::memory_order_relaxed);
                        }
                        co_await finish_page_prompt_work(work.state_index,
                                                         std::move(work.output_key),
                                                         std::move(*parsed_output))
                            .or_fail();
                        co_return;
                    }
                    page_prompt_cache_misses_.fetch_add(1, std::memory_order_relaxed);
                    logging::warn("ignoring invalid cached page prompt '{}': {}",
                                  work.cache_identity,
                                  parsed_output.error().message);
                } else {
                    logging::debug("cache miss for page prompt '{}'", work.cache_identity);
                    page_prompt_cache_misses_.fetch_add(1, std::memory_order_relaxed);
                }
            } else {
                logging::debug("cache disabled for page prompt '{}'", work.cache_identity);
            }
        }

        auto raw_result =
            co_await perform_prompt_request(work.cache_identity, std::move(request)).catch_cancel();
        if(raw_result.is_cancelled()) {
            co_await kota::fail(GenerateError{
                .message = std::format("LLM request cancelled for '{}'", work.cache_identity),
            });
        }
        if(raw_result.has_error()) {
            co_await kota::fail(std::move(raw_result.error()));
        }

        auto parsed_output = parse_markdown_prompt_output(*raw_result, work.output_key);
        if(!parsed_output.has_value()) {
            co_await kota::fail(GenerateError{
                .message = std::format("page prompt output parse failed for '{}': {}",
                                       work.cache_identity,
                                       parsed_output.error().message),
            });
        }

        co_await finish_page_prompt_work(work.state_index,
                                         std::move(work.output_key),
                                         std::move(*parsed_output))
            .or_fail();

        if(cache_index_.has_value()) {
            auto cache_key_copy = *persistent_cache_key;
            auto save_result =
                co_await cache::save_cache_entry_async(std::string(config_.workspace_root),
                                                       std::move(*persistent_cache_key),
                                                       *raw_result,
                                                       loop_)
                    .catch_cancel();
            if(save_result.is_cancelled()) {
                logging::warn("failed to save cached page prompt '{}': cache save cancelled",
                              work.cache_identity);
            } else if(save_result.has_error()) {
                logging::warn("failed to save cached page prompt '{}': {}",
                              work.cache_identity,
                              save_result.error().message);
            } else {
                std::unique_lock lock(cache_index_mutex_);
                cache_index_.value().entries.insert_or_assign(std::move(cache_key_copy),
                                                              std::string(*raw_result));
            }
        }
    }

    auto render_ready_page(std::size_t state_idx) -> kota::task<void, GenerateError> {
        auto& state = tracker_.state_at(state_idx);
        if(state.written || state.failed) {
            co_return;
        }

        auto plan_index = state.plan_index;
        auto& plan = context_.plan_set.plans[plan_index];
        auto& prompt_outputs = state.prompt_outputs;
        const auto& prompt_requests = context_.prompt_requests_by_plan[state.plan_index];
        if(state.pending_symbol_analyses > 0 || state.completed_prompts != state.total_prompts) {
            co_await kota::fail(GenerateError{
                .message = std::format(
                    "page '{}' is not ready to render: prompts={}/{} " "symbols_pending={}",
                    plan.page_id,
                    state.completed_prompts,
                    state.total_prompts,
                    state.pending_symbol_analyses),
            });
        }

        auto page_result =
            co_await run_queued_worker_call(
                         [this, plan_index, &prompt_outputs]() {
                             std::shared_lock lock(analyses_mutex_);
                             return render_generated_pages(context_.plan_set.plans[plan_index],
                                                           config_,
                                                           model_,
                                                           prompt_outputs,
                                                           prepared_analyses_.analyses,
                                                           context_.links);
                         },
                         std::format("page render for '{}'", plan.page_id))
                .or_fail();
        if(!page_result.has_value()) {
            co_await kota::fail(std::move(page_result.error()));
        }

        co_await renderer_.emit_pages_async(*page_result, loop_).or_fail();

        tracker_.mark_page_written(state_idx, plan.page_id);
        maybe_stop_workers();

        renderer_.update_summaries(plan, prompt_requests, state.prompt_outputs);

        co_await try_submit_ready_pages().or_fail();

        co_return;
    }

    auto finish_page_prompt_work(std::size_t state_index,
                                 std::string output_key,
                                 std::string parsed_output) -> kota::task<void, GenerateError> {
        auto& state = tracker_.state_at(state_index);
        if(state.written || state.failed) {
            co_return;
        }
        state.prompt_outputs[output_key] = std::move(parsed_output);
        state.completed_prompts++;

        if(state.completed_prompts == state.total_prompts) {
            work_queue_.enqueue(RenderPageWork{.state_index = state_index});
        }

        co_return;
    }

    auto try_submit_ready_pages() -> kota::task<void, GenerateError> {
        while(auto candidate = tracker_.pop_ready_candidate()) {
            auto i = *candidate;
            auto& state = tracker_.state_at(i);
            state.submitted = true;
            const auto& plan = context_.plan_set.plans[state.plan_index];
            const auto& prompt_requests = context_.prompt_requests_by_plan[state.plan_index];

            if(state.total_prompts == 0) {
                work_queue_.enqueue(RenderPageWork{.state_index = i});
                continue;
            }

            for(const auto& request: prompt_requests) {
                auto output_key = prompt_request_key(request);
                auto cache_identity = prompt_cache_identity_for_page_request(plan, request);

                PagePromptWork work{
                    .state_index = i,
                    .output_key = std::move(output_key),
                    .cache_identity = std::move(cache_identity),
                    .template_request = request,
                };
                work_queue_.enqueue(std::move(work));
            }
            logging::info("submitted prompt tasks for '{}': {} requests",
                          plan.page_id,
                          prompt_requests.size());
        }

        co_return;
    }

    auto maybe_flush_deferred() -> void {
        if(pending_base_symbol_prompts_ != 0 || !work_queue_.has_deferred()) {
            return;
        }
        work_queue_.flush_deferred();
    }

    auto maybe_stop_workers() -> void {
        // Close only once the graph, queue, and currently executing workers are
        // all drained. Destructive abort is reserved for failure paths.
        if(tracker_.finished_count() != tracker_.state_count()) {
            return;
        }
        if(!work_queue_.empty() || work_queue_.has_deferred()) {
            return;
        }
        if(in_flight_work_.load(std::memory_order_relaxed) != 0) {
            return;
        }
        work_queue_.close(worker_count_);
    }

    const config::TaskConfig& config_;
    const extract::ProjectModel& model_;
    const PreparedGenerationContext& context_;
    std::string model_version_;
    std::size_t worker_count_ = 0;
    std::size_t pending_base_symbol_prompts_ = 0;
    kota::event_loop& loop_;
    kota::task_group<GenerateError> scope_;

    WorkQueue work_queue_;
    DependencyTracker tracker_;
    PageRenderer renderer_;

    PreparedSymbolAnalyses prepared_analyses_;
    mutable std::shared_mutex analyses_mutex_;
    std::optional<cache::CacheIndex> cache_index_;
    mutable std::mutex cache_index_mutex_;
    std::atomic<std::size_t> symbol_analysis_cache_hits_ = 0;
    std::atomic<std::size_t> symbol_analysis_cache_misses_ = 0;
    std::atomic<std::size_t> page_prompt_cache_hits_ = 0;
    std::atomic<std::size_t> page_prompt_cache_misses_ = 0;

    std::atomic<std::size_t> expected_llm_requests_ = 0;
    std::atomic<std::size_t> llm_requests_issued_ = 0;
    std::atomic<std::size_t> llm_requests_completed_ = 0;
    std::atomic<std::size_t> in_flight_work_ = 0;
    std::atomic<std::size_t> consecutive_failures_ = 0;
    std::atomic<bool> retry_limit_exceeded_ = false;
};

}  // namespace

auto generate_dry_run(const config::TaskConfig& config, const extract::ProjectModel& model)
    -> std::expected<std::vector<GeneratedPage>, GenerateError> {
    auto context_result = prepare_generation_context(config, model);
    if(!context_result.has_value()) {
        return std::unexpected(std::move(context_result.error()));
    }

    auto context = std::move(*context_result);
    kota::event_loop loop;
    PageGenerationScheduler
        scheduler(config, model, context, "dry-run", 16, config.output_root, loop, true);
    auto task = scheduler.run();

    loop.schedule(task);
    loop.run();

    if(!task->is_finished() && !task->is_failed() && !task->is_cancelled()) {
        return std::unexpected(GenerateError{
            .message = "dry-run generation stopped before completion with pending work",
        });
    }

    if(task->is_cancelled()) {
        return std::unexpected(GenerateError{.message = "dry-run generation task was cancelled"});
    }

    auto result = task.result();
    if(!result.has_value()) {
        return std::unexpected(std::move(result.error()));
    }

    auto pages = scheduler.dry_run_pages();
    pages.push_back(build_request_estimate_page(context.plan_set, model, config));
    pages.push_back(build_llms_page(context.plan_set, config, "request-estimate.md"));
    return pages;
}

auto generate_pages_async(const config::TaskConfig& config,
                          const extract::ProjectModel& model,
                          std::string_view llm_model,
                          std::uint32_t rate_limit,
                          std::string_view output_root,
                          kota::event_loop& loop) -> kota::task<GenerationSummary, GenerateError> {
    if(rate_limit == 0) {
        co_await kota::fail(GenerateError{.message = "rate_limit must be greater than 0"});
    }

    auto context_result = prepare_generation_context(config, model);
    if(!context_result.has_value()) {
        co_await kota::fail(std::move(context_result.error()));
    }

    auto context = std::move(*context_result);
    PageGenerationScheduler
        scheduler(config, model, context, llm_model, rate_limit, output_root, loop);

    co_return co_await scheduler.run();
}

auto generate_pages(const config::TaskConfig& config,
                    const extract::ProjectModel& model,
                    std::string_view llm_model,
                    std::uint32_t rate_limit,
                    std::string_view output_root)
    -> std::expected<GenerationSummary, GenerateError> {
    kota::event_loop loop;
    auto task = generate_pages_async(config, model, llm_model, rate_limit, output_root, loop);
    loop.schedule(task);
    loop.run();

    auto result = task.result();
    if(!result.has_value()) {
        return std::unexpected(std::move(result.error()));
    }

    return *result;
}

auto write_pages(const std::vector<GeneratedPage>& pages, std::string_view output_root)
    -> std::expected<void, GenerateError> {
    auto expanded_pages = pages;
    auto directory_indexes = build_directory_index_pages(pages);
    expanded_pages.insert(expanded_pages.end(),
                          std::make_move_iterator(directory_indexes.begin()),
                          std::make_move_iterator(directory_indexes.end()));

    for(const auto& page: expanded_pages) {
        auto result = write_page(page, output_root);
        if(!result.has_value()) {
            return std::unexpected(GenerateError{.message = result.error().message});
        }
    }
    return {};
}

}  // namespace clore::generate
