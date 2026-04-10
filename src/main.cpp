#include <cstdint>
#include <iostream>
#include <print>
#include <string>

#include "eventide/deco/deco.h"

#include "config/config.h"
#include "extract/extract.h"
#include "generate/generate.h"
#include "support/logging.h"

namespace clore {

using deco::decl::KVStyle;

struct Options {
    DecoKV(style = KVStyle::JoinedOrSeparate,
           help = "Path to clore configuration file (TOML)",
           required = false)
    <std::string> config;

    DecoKV(style = KVStyle::JoinedOrSeparate,
           help = "Path to compile_commands.json",
           required = false)
    <std::string> compile_commands;

    DecoKV(style = KVStyle::JoinedOrSeparate,
           help = "Project root directory",
           required = false)
    <std::string> project_root;

    DecoKV(style = KVStyle::JoinedOrSeparate,
           help = "Output root directory",
           required = false)
    <std::string> output;

    DecoKV(style = KVStyle::JoinedOrSeparate,
           names = {"--log-level", "--log-level="},
           help = "Log level: trace, debug, info, warn, error, off",
           required = false)
    <std::string> log_level;

    DecoKV(style = KVStyle::JoinedOrSeparate,
           names = {"--max-snippet-bytes", "--max-snippet-bytes="},
           help = "Maximum bytes of source captured per symbol snippet",
           required = false)
    <std::uint32_t> max_snippet_bytes;

    DecoKV(style = KVStyle::JoinedOrSeparate,
           names = {"--llm-model", "--llm-model="},
           help = "LLM model name for generation",
           required = false)
    <std::string> llm_model;

    DecoFlag(names = {"-h", "--help"}, help = "Show help message", required = false)
    help;

    DecoFlag(names = {"-v", "--version"}, help = "Show version", required = false)
    version;

    DecoFlag(names = {"--dry-run"}, help = "Validate config and extract without generating output",
             required = false)
    dry_run;
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

    if(opts.help.value_or(false)) {
        deco::cli::write_usage_for<clore::Options>(std::cout, "clore [OPTIONS]");
        return 0;
    }

    if(opts.version.value_or(false)) {
        std::println("clore version 0.1.0");
        return 0;
    }

    // Configure logging
    if(opts.log_level.has_value()) {
        auto level = spdlog::level::from_str(*opts.log_level);
        clore::logging::options.level = level;
    }
    clore::logging::stderr_logger("clore");

    // Load optional configuration
    clore::config::TaskConfig task_config;

    if(opts.config.has_value()) {
        auto cfg_result = clore::config::load_config(*opts.config);
        if(!cfg_result.has_value()) {
            clore::logging::err("failed to load config: {}", cfg_result.error().message);
            return 1;
        }
        task_config = std::move(*cfg_result);
    }

    if(!opts.compile_commands.has_value()) {
        clore::logging::err("--compile-commands is required");
        return 1;
    }
    if(!opts.project_root.has_value()) {
        clore::logging::err("--project-root is required");
        return 1;
    }
    if(!opts.output.has_value()) {
        clore::logging::err("--output is required");
        return 1;
    }
    if(!opts.dry_run.value_or(false) && !opts.llm_model.has_value()) {
        clore::logging::err("generation requires --llm-model");
        return 1;
    }

    task_config.compile_commands_path = *opts.compile_commands;
    task_config.project_root = *opts.project_root;
    task_config.output_root = *opts.output;
    task_config.extract.max_snippet_bytes =
        opts.max_snippet_bytes.has_value() ? std::optional{*opts.max_snippet_bytes}
                                           : std::optional<std::uint32_t>{8192};

    std::string llm_model;
    if(opts.llm_model.has_value()) {
        llm_model = *opts.llm_model;
    }
    if(!opts.dry_run.value_or(false) && llm_model.empty()) {
        clore::logging::err("llm model must not be empty");
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
    clore::logging::info("  project_root: {}", task_config.project_root);
    clore::logging::info("  output_root: {}", task_config.output_root);
    clore::logging::info("  max_snippet_bytes: {}", *task_config.extract.max_snippet_bytes);
    if(!opts.dry_run.value_or(false)) {
        clore::logging::info("  llm_model: {}", llm_model);
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

    if(opts.dry_run.value_or(false)) {
        clore::logging::info("dry-run mode: skipping output generation");
        return 0;
    }

    // Generate
    auto gen_result = clore::generate::generate_pages(task_config, model, llm_model);
    if(!gen_result.has_value()) {
        clore::logging::err("generation failed: {}", gen_result.error().message);
        return 1;
    }

    auto& pages = *gen_result;
    clore::logging::info("generated {} pages", pages.size());
    clore::logging::info("documentation written to {}", task_config.output_root);

    return 0;
}
