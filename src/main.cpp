#include "kota/async/async.h"
#include "kota/deco/deco.h"

#include "spdlog/spdlog.h"

import std;
import agent;
import config;
import extract;
import generate;
import network;
import support;

namespace clore {

using kota::deco::decl::KVStyle;

struct Options {
    DecoKV(style = KVStyle::JoinedOrSeparate,
           help = "Path to clore configuration file",
           required = false)
    <std::string> config;

    DecoKV(style = KVStyle::JoinedOrSeparate,
           help = "Path to compile_commands.json",
           required = false)
    <std::string> compile_commands;

    DecoKV(style = KVStyle::JoinedOrSeparate,
           help = "Source root directory used for relative output paths",
           required = false)
    <std::string> source_dir;

    DecoKV(style = KVStyle::JoinedOrSeparate, help = "Output root directory", required = false)
    <std::string> output_dir;

    DecoKV(style = KVStyle::JoinedOrSeparate,
           names = {"--log-level", "--log-level="},
           help = "Log level: trace, debug, info, warn, error, off",
           required = false)
    <std::string> log_level;

    DecoKV(style = KVStyle::JoinedOrSeparate,
           names = {"--model", "--model="},
           help = "Model name for generation",
           required = false)
    <std::string> model;

    DecoKV(style = KVStyle::JoinedOrSeparate,
           names = {"--rate-limit", "--rate-limit="},
           help = "Maximum concurrent LLM requests when --model is used",
           required = false)
    <std::uint32_t> rate_limit;

    DecoFlag(names = {"--dry-run"},
             help = "Write assembled prompts to --output-dir and skip LLM requests",
             required = false)
    dry_run;

    DecoFlag(names = {"--experimental-agent-mode"},
             help =
                 "Enable agent-driven autonomous codebase exploration and " "guide generation",
             required = false)
    agent_mode;

    DecoFlag(names = {"-h", "--help"}, help = "Show help message", required = false)
    help;

    DecoFlag(names = {"-v", "--version"}, help = "Show version", required = false)
    version;
};

auto log_generation_summary(const generate::GenerationSummary& summary) -> void {
    clore::logging::info("generated {} pages", summary.written_output_count);

    if(summary.symbol_analysis_cache_hits + summary.symbol_analysis_cache_misses > 0) {
        clore::logging::cache_hit_rate("symbol analysis",
                                       summary.symbol_analysis_cache_hits,
                                       summary.symbol_analysis_cache_misses);
    }

    if(summary.page_prompt_cache_hits + summary.page_prompt_cache_misses > 0) {
        clore::logging::cache_hit_rate("page prompt",
                                       summary.page_prompt_cache_hits,
                                       summary.page_prompt_cache_misses);
    }
}

template <typename Task,
          typename Value = typename Task::value_type,
          typename Error = typename Task::error_type>
auto await_task_result(Task& task, std::string_view name) -> std::expected<Value, Error> {
    if(!task->is_finished() && !task->is_failed() && !task->is_cancelled()) {
        return std::unexpected(Error{
            .message = std::format("{} stopped before completion with pending work", name),
        });
    }

    try {
        auto result = task.result();
        if(result.is_cancelled()) {
            return std::unexpected(Error{
                .message = std::format("{} was cancelled", name),
            });
        }
        if(result.has_error()) {
            return std::unexpected(std::move(result.error()));
        }
        return std::move(*result);
    } catch(const std::exception& ex) {
        return std::unexpected(Error{
            .message = std::format("{} threw exception: {}", name, ex.what()),
        });
    } catch(...) {
        return std::unexpected(Error{
            .message = std::format("{} threw unknown exception", name),
        });
    }
}

}  // namespace clore

int main(int argc, const char** argv) {
    clore::support::enable_utf8_console();

    struct LLMRateLimitShutdownGuard {
        ~LLMRateLimitShutdownGuard() {
            clore::net::shutdown_llm_rate_limit();
        }
    } llm_rate_limit_shutdown_guard;

    auto args = kota::deco::util::argvify(argc, argv);
    auto result = kota::deco::cli::parse<clore::Options>(args);

    if(!result.has_value()) {
        std::println(stderr, "error: {}", result.error().message);
        return 1;
    }

    auto& opts = result->options;
    auto prompt_dry_run = opts.dry_run.value_or(false);
    auto agent_mode = opts.agent_mode.value_or(false);
    auto has_model = opts.model.has_value();

    if(opts.help.value_or(false)) {
        kota::deco::cli::write_usage_for<clore::Options>(std::cout, "clore [OPTIONS]");
        return 0;
    }

    if(opts.version.value_or(false)) {
        std::println("clore version 0.1.0");
        return 0;
    }

    // Initialize logger early so config/validation failures are visible.
    clore::logging::stderr_logger("clore");

    if(!agent_mode && prompt_dry_run == has_model) {
        clore::logging::err("exactly one of --dry-run or --model is required");
        return 1;
    }

    if(agent_mode && !has_model) {
        clore::logging::err("--experimental-agent-mode requires --model");
        return 1;
    }

    namespace fs = std::filesystem;

    auto log_expected_error = [](const auto& result, std::string_view context) -> bool {
        if(result.has_value()) {
            return false;
        }
        clore::logging::err("{}: {}", context, result.error().message);
        return true;
    };

    if(!opts.config.has_value()) {
        clore::logging::err("--config is required");
        return 1;
    }
    const std::string config_path = *opts.config;
    auto cfg_result = clore::config::load_config(config_path);
    if(log_expected_error(cfg_result, "failed to load config")) {
        return 1;
    }
    auto task_config = std::move(*cfg_result);

    if(task_config.workspace_root.empty()) {
        task_config.workspace_root = fs::path(config_path).parent_path().string();
    }

    auto set_log_level = [](std::string_view value, std::string_view source) {
        auto normalized = clore::support::canonical_log_level_name(value);
        if(!normalized.has_value()) {
            clore::logging::warn("invalid {} log level '{}', keeping current level", source, value);
            return;
        }
        auto level = spdlog::level::from_str(*normalized);
        clore::logging::g_log_level = level;
        spdlog::set_level(level);
    };

    // Logging precedence: clore.toml -> CLI override.
    if(task_config.log_level.has_value()) {
        set_log_level(*task_config.log_level, "config");
    }
    if(opts.log_level.has_value()) {
        set_log_level(*opts.log_level, "CLI");
    }

    if(!opts.compile_commands.has_value()) {
        clore::logging::err("--compile-commands is required");
        return 1;
    }
    if(!opts.source_dir.has_value()) {
        clore::logging::err("--source-dir is required");
        return 1;
    }
    if(!opts.output_dir.has_value()) {
        clore::logging::err("--output-dir is required");
        return 1;
    }

    task_config.compile_commands_path = *opts.compile_commands;
    task_config.project_root = *opts.source_dir;
    task_config.output_root = *opts.output_dir;
    if(task_config.workspace_root.empty()) {
        task_config.workspace_root = fs::current_path().string();
    }

    std::string llm_model;
    if(opts.model.has_value()) {
        llm_model = *opts.model;
    }

    if(!prompt_dry_run && llm_model.empty()) {
        clore::logging::err("model must not be empty");
        return 1;
    }

    std::uint32_t rate_limit = 0;
    if(!prompt_dry_run) {
        if(!opts.rate_limit.has_value()) {
            clore::logging::err("--rate-limit is required when --model is used");
            return 1;
        }
        rate_limit = *opts.rate_limit;
        clore::net::initialize_llm_rate_limit(rate_limit);
    }

    auto normalize_result = clore::config::normalize(task_config);
    if(log_expected_error(normalize_result, "config normalization failed")) {
        return 1;
    }

    auto validate_result = clore::config::validate(task_config);
    if(log_expected_error(validate_result, "config validation failed")) {
        return 1;
    }

    clore::logging::info("configuration validated successfully");
    clore::logging::info("  compile_commands: {}", task_config.compile_commands_path);
    clore::logging::info("  source_dir: {}", task_config.project_root);
    clore::logging::info("  output_root: {}", task_config.output_root);
    clore::logging::info("  workspace_root: {}", task_config.workspace_root);
    if(!prompt_dry_run) {
        clore::logging::info("  model: {}", llm_model);
        clore::logging::info("  rate_limit: {}", rate_limit);
    }

    kota::event_loop loop;
    auto extract_task = clore::extract::extract_project_async(task_config, loop).catch_cancel();
    loop.schedule(extract_task);
    loop.run();

    auto extract_result = clore::await_task_result(extract_task, "extract task");
    if(!extract_result.has_value()) {
        clore::logging::err("extraction failed: {}", extract_result.error().message);
        return 1;
    }

    auto& model = *extract_result;
    clore::logging::info("extraction complete:");
    clore::logging::info("  {} symbols", model.symbols.size());
    clore::logging::info("  {} files", model.files.size());
    clore::logging::info("  {} namespaces", model.namespaces.size());

    if(prompt_dry_run) {
        auto dry_result = clore::generate::generate_dry_run(task_config, model);
        if(!dry_result.has_value()) {
            clore::logging::err("dry-run failed: {}", dry_result.error().message);
            return 1;
        }

        auto write_result = clore::generate::write_pages(*dry_result, task_config.output_root);
        if(!write_result.has_value()) {
            clore::logging::err("dry-run write failed: {}", write_result.error().message);
            return 1;
        }

        clore::logging::info("dry-run complete: API reference written to {}",
                             task_config.output_root);
        return 0;
    }

    // Agent mode: run agent and page generation in parallel on a single event loop
    if(agent_mode) {
        clore::logging::info("agent mode enabled: running agent and generation in parallel");

        kota::event_loop parallel_loop;
        auto agent_task = clore::agent::run_agent_async(task_config,
                                                        model,
                                                        llm_model,
                                                        task_config.output_root,
                                                        parallel_loop)
                              .catch_cancel();
        auto gen_task = clore::generate::generate_pages_async(task_config,
                                                              model,
                                                              llm_model,
                                                              rate_limit,
                                                              task_config.output_root,
                                                              parallel_loop)
                            .catch_cancel();

        parallel_loop.schedule(agent_task);
        parallel_loop.schedule(gen_task);
        parallel_loop.run();

        if(!agent_task->is_finished() && !agent_task->is_failed() && !agent_task->is_cancelled()) {
            clore::logging::err("agent task stopped before completion with pending work");
            return 1;
        }
        if(!gen_task->is_finished() && !gen_task->is_failed() && !gen_task->is_cancelled()) {
            clore::logging::err("generation task stopped before completion with pending work");
            return 1;
        }

        auto agent_result = agent_task.result();
        auto gen_result = gen_task.result();

        auto failed = false;
        if(agent_result.is_cancelled()) {
            clore::logging::err("parallel execution failed: agent task cancelled");
            failed = true;
        } else if(agent_result.has_error()) {
            clore::logging::err("parallel execution failed: {}", agent_result.error().message);
            failed = true;
        }

        if(gen_result.is_cancelled()) {
            clore::logging::err("parallel execution failed: generation task cancelled");
            failed = true;
        } else if(gen_result.has_error()) {
            clore::logging::err("parallel execution failed: {}", gen_result.error().message);
            failed = true;
        }

        if(failed) {
            return 1;
        }

        clore::log_generation_summary(*gen_result);
        clore::logging::info("agent mode complete: {} guides generated", *agent_result);
        clore::logging::info("documentation written to {}", task_config.output_root);

        return 0;
    }

    auto gen_task = clore::generate::generate_pages_async(task_config,
                                                          model,
                                                          llm_model,
                                                          rate_limit,
                                                          task_config.output_root,
                                                          loop)
                        .catch_cancel();
    loop.schedule(gen_task);
    loop.run();

    auto gen_result = clore::await_task_result(gen_task, "generation task");
    if(!gen_result.has_value()) {
        clore::logging::err("generation failed: {}", gen_result.error().message);
        return 1;
    }

    clore::log_generation_summary(*gen_result);
    clore::logging::info("documentation written to {}", task_config.output_root);

    return 0;
}
