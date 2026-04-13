#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <print>
#include <string>

#include "eventide/deco/deco.h"
#include "spdlog/spdlog.h"

import config;
import extract;
import generate;
import support;

namespace clore {

using deco::decl::KVStyle;
constexpr std::uint32_t defaultRateLimit = 16;

struct Options {
    DecoKV(style = KVStyle::JoinedOrSeparate,
            help = "Path to clore configuration file (default: clore.toml)",
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

    DecoKV(style = KVStyle::JoinedOrSeparate,
           help = "Output root directory",
           required = false)
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
           help = "Maximum concurrent LLM requests when --model is used (default: 16)",
           required = false)
    <std::uint32_t> rate_limit;

        DecoFlag(names = {"--dry-run"},
              help = "Write assembled prompts to --output-dir and skip LLM requests",
              required = false)
        dry_run;

    DecoFlag(names = {"-h", "--help"}, help = "Show help message", required = false)
    help;

    DecoFlag(names = {"-v", "--version"}, help = "Show version", required = false)
    version;
};

}  // namespace clore

int main(int argc, const char** argv) {
    auto args = deco::util::argvify(argc, argv);
    auto result = deco::cli::parse<clore::Options>(args);

    if(!result.has_value()) {
        std::println(stderr, "error: {}", result.error().message);
        return 1;
    }

    auto& opts = result->options;
    auto prompt_dry_run = opts.dry_run.value_or(false);
    auto has_model = opts.model.has_value();

    if(opts.help.value_or(false)) {
        deco::cli::write_usage_for<clore::Options>(std::cout, "clore [OPTIONS]");
        return 0;
    }

    if(opts.version.value_or(false)) {
        std::println("clore version 0.1.0");
        return 0;
    }

    clore::support::enable_utf8_console();

    // Configure logging
    if(opts.log_level.has_value()) {
        auto level = spdlog::level::from_str(*opts.log_level);
        clore::logging::options.level = level;
    }
    clore::logging::stderr_logger("clore");

    if(prompt_dry_run == has_model) {
        clore::logging::err("exactly one of --dry-run or --model is required");
        return 1;
    }

    namespace fs = std::filesystem;

    const std::string config_path = opts.config.value_or("clore.toml");
    auto cfg_result = clore::config::load_config(config_path);
    if(!cfg_result.has_value()) {
        clore::logging::err("failed to load config: {}", cfg_result.error().message);
        return 1;
    }
    auto task_config = std::move(*cfg_result);

    if(task_config.workspace_root.empty()) {
        task_config.workspace_root = fs::path(config_path).parent_path().string();
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
    auto rate_limit = opts.rate_limit.value_or(clore::defaultRateLimit);
    if(!prompt_dry_run && llm_model.empty()) {
        clore::logging::err("model must not be empty");
        return 1;
    }

    // Normalize
    auto normalize_result = clore::config::normalize(task_config);
    if(!normalize_result.has_value()) {
        clore::logging::err("config normalization failed: {}",
                            normalize_result.error().message);
        return 1;
    }

    // Validate
    auto validate_result = clore::config::validate(task_config);
    if(!validate_result.has_value()) {
        clore::logging::err("config validation failed: {}",
                            validate_result.error().message);
        return 1;
    }

    clore::logging::info("configuration validated successfully");
    clore::logging::info("  compile_commands: {}", task_config.compile_commands_path);
    clore::logging::info("  source_dir: {}", task_config.project_root);
    clore::logging::info("  output_root: {}", task_config.output_root);
    clore::logging::info("  workspace_root: {}", task_config.workspace_root);
    if(task_config.extract.max_snippet_bytes.has_value()) {
        clore::logging::info("  max_snippet_bytes: {}", *task_config.extract.max_snippet_bytes);
    }
    if(!prompt_dry_run) {
        clore::logging::info("  model: {}", llm_model);
        clore::logging::info("  rate_limit: {}", rate_limit);
    }

    // Extract
    auto extract_result = clore::extract::extract_project(task_config);
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

        auto write_result = clore::generate::write_pages(*dry_result,
                                                         task_config.output_root);
        if(!write_result.has_value()) {
            clore::logging::err("dry-run write failed: {}", write_result.error().message);
            return 1;
        }

        clore::logging::info("dry-run complete: API reference written to {}",
                             task_config.output_root);
        return 0;
    }

    // Generate
    auto gen_result = clore::generate::generate_pages(task_config, model, llm_model,
                                                     rate_limit,
                                                     task_config.output_root);
    if(!gen_result.has_value()) {
        clore::logging::err("generation failed: {}", gen_result.error().message);
        return 1;
    }

    clore::logging::info("generated {} pages", *gen_result);
    clore::logging::info("documentation written to {}", task_config.output_root);

    return 0;
}
