module;

#include "kota/async/async.h"

export module generate;

export import :analysis;
export import :cache;
export import :common;
export import :evidence;
export import :markdown;
export import :model;
export import :planner;
export import :diagram;
export import :page;
export import :symbol;
export import :dryrun;

import std;
import config;
import extract;

export namespace clore::generate {

auto generate_dry_run(const config::TaskConfig& config, const extract::ProjectModel& model)
    -> std::expected<std::vector<GeneratedPage>, GenerateError>;

auto generate_pages(const config::TaskConfig& config,
                    const extract::ProjectModel& model,
                    std::string_view llm_model,
                    std::uint32_t rate_limit,
                    std::string_view output_root)
    -> std::expected<GenerationSummary, GenerateError>;

/// Asynchronous page generation that runs on the provided event loop.
/// Callers must schedule the returned task on the loop and run it.
auto generate_pages_async(const config::TaskConfig& config,
                          const extract::ProjectModel& model,
                          std::string_view llm_model,
                          std::uint32_t rate_limit,
                          std::string_view output_root,
                          kota::event_loop& loop) -> kota::task<GenerationSummary, GenerateError>;

auto write_pages(const std::vector<GeneratedPage>& pages, std::string_view output_root)
    -> std::expected<void, GenerateError>;

}  // namespace clore::generate
