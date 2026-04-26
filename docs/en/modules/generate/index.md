---
title: 'Module generate'
description: 'The generate module owns the documentation page generation pipeline, transforming extracted project data into final rendered output. Its public interface comprises four functions: generate_dry_run for validation without disk writes, generate_pages as the synchronous entry point with configurable concurrency and output extension, write_pages for writing already‑generated content to a given output path, and generate_pages_async for non‑blocking generation on a caller‑provided event loop. The module manages internal state such as output root, rate limiting, LLM model selection, and page planning, and it relies on imported configuration and extraction dependencies to drive the full generation workflow.'
layout: doc
template: doc
---

# Module `generate`

## Summary

The `generate` module owns the documentation page generation pipeline, transforming extracted project data into final rendered output. Its public interface comprises four functions: `generate_dry_run` for validation without disk writes, `generate_pages` as the synchronous entry point with configurable concurrency and output extension, `write_pages` for writing already‑generated content to a given output path, and `generate_pages_async` for non‑blocking generation on a caller‑provided event loop. The module manages internal state such as output root, rate limiting, LLM model selection, and page planning, and it relies on imported configuration and extraction dependencies to drive the full generation workflow.

## Imports

- [`config`](../config/index.md)
- [`extract`](../extract/index.md)
- `std`

## Imported By

- [`agent`](../agent/index.md)
- [`agent:tools`](../agent/tools.md)

## Functions

### `clore::generate::generate_dry_run`

Declaration: `generate/generate.cppm:25`

Definition: `generate/scheduler.cppm:1888`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function begins by calling `prepare_generation_context` to construct a `PreparedGenerationContext` from the input configuration and model data. Using `prepare_symbol_analyses_for_dry_run`, it builds a `PreparedSymbolAnalyses` instance that contains the minimal metadata needed for a dry run, omitting actual LLM prompts. A `PageGenerationScheduler` is then constructed with the context, model, output root, event loop, and the `dry_run` flag set to true; the scheduler delegates its internal work to a `DependencyTracker`, a `WorkQueue`, and a `PageRenderer` (which in dry‑run mode sets `dry_run_` to true and records page output in `dry_run_pages_` instead of writing files).

After construction, the scheduler’s `run` method is invoked. During execution, it populates the `DependencyTracker` from the prepared context, iterates over each plan in `context.id_to_plan`, and for each page that has no unsatisfied symbol dependencies, it submits a `PagePromptWork` item into the `WorkQueue`. The scheduler’s worker loop (`worker_task`) dequeues work, calls `run_page_prompt_task` (which bypasses the LLM and directly records the prompt output as a placeholder), and updates page states via `finish_page_prompt_work`. Once all pages are either written or marked failed, the scheduler checks the counts from the `DependencyTracker` and `PageRenderer`. Finally, `generate_dry_run` returns `0` on success or a non‑zero error code produced by `make_generate_error` if any page failures or internal inconsistencies are detected.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- first `const int &` parameter
- second `const int &` parameter

#### Usage Patterns

- Called to test generation logic without outputting generated pages.
- Used in validation or pre-flight checks before a full generation.

### `clore::generate::generate_pages`

Declaration: `generate/generate.cppm:28`

Definition: `generate/scheduler.cppm:1947`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::generate_pages` builds a `PageGenerationScheduler` from a `PreparedGenerationContext` (produced by `prepare_generation_context`) and drives the scheduler’s `run` method to completion. Internally, the scheduler orchestrates a multi‑stage pipeline: it first collects documentable symbols and queues symbol‑analysis tasks via `schedule_symbol_analysis`, feeding results into a `DependencyTracker` that tracks inter‑page and inter‑symbol dependencies. Once dependencies are satisfied, page‑prompt tasks are submitted to a `WorkQueue` and executed by a pool of worker threads running `worker_task`. Each worker calls `run_page_prompt_task` which performs LLM requests (with caching via `perform_prompt_request` and identity computation through `prompt_cache_identity_for_page_request`), then feeds the output into `finish_page_prompt_work`. Completed pages are dispatched to a `PageRenderer` for dry‑run accumulation or final file emission via `emit_pages`. A `DependencyTracker` instance (`tracker_`) manages states via `id_to_state_`, tracks dependents with `dependents_`, and releases blocked pages through `release_dependents` once symbol analysis finishes. Consecutive failures are counted by `record_consecutive_failure` and can trigger early termination through `retry_limit_exceeded`. The function also handles directory index page generation via `build_directory_index_pages` and collects page summaries from prompt outputs for later reuse. The overall flow is LLM‑request driven, dependency‑aware, and resilient to failures and caching.

#### Side Effects

- Writes documentation page files to the filesystem at the path specified by the last `std::string_view` parameter

#### Reads From

- Input analysis handles (first two `const int &` parameters)
- Configuration strings (the `std::string_view` parameters)
- Potential global or cached analysis stores and symbol databases

#### Writes To

- Output documentation page files in the specified directory

#### Usage Patterns

- Main entry point for generating documentation pages after analysis
- Typically called once per documentation generation run

### `clore::generate::generate_pages_async`

Declaration: `generate/generate.cppm:37`

Definition: `generate/scheduler.cppm:1925`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function first prepares the generation context by calling the internal helper `prepare_generation_context` on the two integer parameters, producing a `PreparedGenerationContext` that contains the page plans, prompt requests, symbol analysis targets, and link metadata. It then constructs a `PageGenerationScheduler` bound to the provided `kota::event_loop`, passing the configuration, model, context, LLM model string, rate limit, and output root. The scheduler internally owns a `WorkQueue`, a `DependencyTracker`, and a `PageRenderer`. After construction, the function invokes the scheduler’s `run` method, which orchestrates the entire asynchronous pipeline: it initializes dependency tracking from the prepared context, spawns worker tasks that dequeue symbol analysis work (via `schedule_symbol_analysis` / `run_symbol_analysis_task`) and page prompt work (via `run_page_prompt_task`), manages LLM requests through `request_llm_async`, coordinates state transitions as prompts complete, pages become ready, rendered (via `render_ready_page` and `emit_pages_async`), and output is written. The pipeline respects the configured rate limit, retry limits, and dry-run mode, and updates internal counters for cache hits/misses and consecutive failures. The function returns an integer representing the overall result or task identifier for the asynchronous generation.

#### Side Effects

- Schedules an asynchronous task on the provided `kota::event_loop`
- Writes generated pages to the output directory when the task is executed

#### Reads From

- first `const int&` parameter (likely module or file index)
- second `const int&` parameter
- `std::string_view` output directory
- `std::uint32_t` limit
- `std::string_view` base path
- `kota::event_loop&` event loop

#### Writes To

- the `kota::event_loop` (schedules tasks)
- output directory (via internal page writing functions)

#### Usage Patterns

- Called when asynchronous page generation is needed
- Callers must schedule the returned task on the event loop and run it

### `clore::generate::write_pages`

Declaration: `generate/generate.cppm:44`

Definition: `generate/scheduler.cppm:1966`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function first gathers the documentable symbol set from the configuration via `collect_documentable_symbols`. It then builds the full generation context with `prepare_generation_context`, which populates plan‑level prompt requests, symbol‑analysis targets, and cross‑page link data. Afterward it constructs directory index pages using `build_directory_index_pages` and initializes a `PageGenerationScheduler` object that owns a `DependencyTracker`, a `WorkQueue`, and a `PageRenderer`. The scheduler’s `run` method drives the core pipeline: it submits page‑prompt and symbol‑analysis tasks, respects rate limits and retry limits, resolves dependency chains, caches LLM responses, and eventually calls `render_generated_pages` to emit final output files. Once the scheduler completes, the function sets evidence metadata via `set_evidence_metadata` and returns a status code produced by `make_generate_error` (or zero on success).

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- const int &
- `std::string_view`

#### Writes To

- return int

#### Usage Patterns

- called during page generation to write rendered pages to files

## Internal Structure

The `generate` module is decomposed into synchronous and asynchronous generation pathways, both sharing a common output‑writing step. It depends on the `config` module for generation parameters and on the `extract` module for pre‑processed symbol and page‑plan data. Internally, the module separates concerns by exposing a dry‑run entry point (`generate_dry_run`) for validation, a blocking pipeline (`generate_pages`) that orchestrates rendering and writing via `write_pages`, and an asynchronous variant (`generate_pages_async`) that accepts a `kota::event_loop` to drive non‑blocking execution. Shared state such as rate limits, model selections, and output paths is managed through module‑private variables, keeping the public interface focused on integer result codes and `std::string_view` parameters.

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)

