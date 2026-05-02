---
title: 'Module generate'
description: 'The generate module owns the documentation page generation pipeline. It provides four public entry points: generate_dry_run for simulating the generation outcome without side effects, generate_pages as the synchronous orchestrator that processes resource identifiers, a concurrency limit, and an output directory, write_pages to commit the rendered pages to a specified location, and generate_pages_async which returns a task to be scheduled on an event loop for asynchronous execution. The module internally manages configuration, rate limiting, output paths, and LLM models, and depends on the config and extract modules for compilation settings and project metadata.'
layout: doc
template: doc
---

# Module `generate`

## Summary

The `generate` module owns the documentation page generation pipeline. It provides four public entry points: `generate_dry_run` for simulating the generation outcome without side effects, `generate_pages` as the synchronous orchestrator that processes resource identifiers, a concurrency limit, and an output directory, `write_pages` to commit the rendered pages to a specified location, and `generate_pages_async` which returns a task to be scheduled on an event loop for asynchronous execution. The module internally manages configuration, rate limiting, output paths, and LLM models, and depends on the `config` and `extract` modules for compilation settings and project metadata.

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

Definition: `generate/scheduler.cppm:1932`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::generate_dry_run` orchestrates a complete simulation of the page generation pipeline without persisting any rendered output. It begins by constructing a `PreparedGenerationContext` via `prepare_generation_context` and then calls `prepare_symbol_analyses_for_dry_run` to set up the required symbol analyses. A `PageGenerationScheduler` is created with the `dry_run` flag set to `true`, which causes the internal `PageRenderer` to skip file emission and instead accumulate generated page data in memory via its `dry_run_pages_` field. The scheduler then executes its `run` method, which drives the full workflow: it submits prompts, performs symbol analysis tasks, handles dependency tracking through the `DependencyTracker`, enqueues work in the `WorkQueue`, and manages LLM requests via `request_llm_async`. All prompt outputs are parsed and cached, but the `PageRenderer` never writes to disk because its `dry_run_` flag suppresses I/O. The function ultimately returns the number of pages that would have been generated, as reported by `PageRenderer::dry_run_pages`.

#### Side Effects

No observable side effects are evident from the extracted code.

### `clore::generate::generate_pages`

Declaration: `generate/generate.cppm:28`

Definition: `generate/scheduler.cppm:1991`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::generate_pages` orchestrates the entire page generation pipeline. It begins by invoking `prepare_generation_context` to produce a `PreparedGenerationContext` from the given configuration and model settings. A `PageGenerationScheduler` is then constructed with the context, LLM model identifier, rate limit, output root path, event loop, and optional dry-run flag. The scheduler coordinates symbol analysis and page prompt work through its internal `DependencyTracker`, `WorkQueue`, and `PageRenderer`. It first prepares symbol analyses (including a dry-run variant via `prepare_symbol_analyses_for_dry_run`), then schedules and executes symbol analysis tasks, managing dependencies and caching via `prompt_cache_identity_for_page_request` and persistent cache keys. After symbol dependencies are satisfied, the scheduler submits page prompt work, limiting concurrency with a semaphore from the `WorkQueue` and respecting the rate limit for LLM requests.

Inside the scheduler’s worker task loop, each worker dequeues work items (either symbol analysis or page prompt tasks), performs LLM requests via `request_llm_async`, handles retries and failures through `record_consecutive_failure` and `retry_limit_exceeded`, and caches results. Once a page’s prompts are completed, its output is rendered using `PageRenderer::emit_pages_async` or `emit_pages`, and summaries are updated via `update_page_summaries`. The pipeline also handles directory index page generation through `build_directory_index_pages`, deduplication of prompt requests via `deduplicate_prompt_requests`, and final rendering of all generated pages. The function returns an integral result indicating success or the number of generated pages.

#### Side Effects

- Writes documentation pages to the filesystem via `write_pages` and `write_page`

#### Reads From

- the two `const int &` context parameters (analysis state and plan set)
- the first `std::string_view` parameter (output directory or base path)
- the `std::uint32_t` parameter (seed for reproducibility)
- the second `std::string_view` parameter (identifier for the generation run)

#### Writes To

- output files under the provided `std::string_view` path (written through `write_pages` and `write_page`)

#### Usage Patterns

- Primary invocation point for generating all documentation pages after analysis is complete
- Called by higher-level generation orchestration code, possibly `generate_pages_async` for asynchronous execution

### `clore::generate::generate_pages_async`

Declaration: `generate/generate.cppm:37`

Definition: `generate/scheduler.cppm:1969`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function constructs a `PreparedGenerationContext` from its parameters by calling `prepare_generation_context`, then creates a `PageGenerationScheduler` that owns a `DependencyTracker`, a `WorkQueue`, and a `PageRenderer`. It invokes the scheduler's `run` method, which starts a fixed number of background worker tasks on the provided `kota::event_loop`. Each worker loops, dequeues `ScheduledWork` from the `WorkQueue`, and dispatches either symbol‑analysis or page‑prompt tasks. Symbol analyses call `request_llm_async`, record results in the `DependencyTracker`, and release dependent pages when a symbol becomes ready. Page‑prompt tasks use the prepared template, check a persistent cache via `prompt_cache_identity_for_page_request`, and on completion call `finish_page_prompt_work` to update the page state and trigger rendering. The scheduler respects retry limits, tracks consecutive failures, and flushes deferred work as dependencies resolve. Once all work is done or stopped, `run` returns an aggregate result indicating success, failure, or cancellation.

#### Side Effects

- schedules asynchronous tasks on the provided event loop
- likely performs file I/O to write generated pages

#### Reads From

- all parameters: const int references, `string_view` arguments, `uint32_t`, event loop reference

#### Writes To

- external filesystem via generated pages
- event loop task queue

#### Usage Patterns

- callers must schedule the returned task on the loop and run it
- used to run page generation asynchronously in a cooperating event loop

### `clore::generate::write_pages`

Declaration: `generate/generate.cppm:44`

Definition: `generate/scheduler.cppm:2010`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::write_pages` orchestrates the end-to-end generation pipeline by first preparing the generation context via `prepare_generation_context`, which builds a `PreparedGenerationContext` containing plan sets, symbol analysis targets, prompt requests, and page structures. It then constructs a `PageGenerationScheduler` with the provided config, model, `PreparedGenerationContext`, LLM model identifier, rate limit, output root, event loop, and dry-run flag. The scheduler coordinates concurrent work through its `run` method: symbol analysis tasks are submitted via `schedule_symbol_analysis` and executed by `run_symbol_analysis_task`, while page prompt tasks are dispatched through `run_page_prompt_task`. A `DependencyTracker` instance inside the scheduler manages state per page, tracking pending symbol analyses, unsatisfied dependencies, and ready candidates via `pop_ready_candidate`. The `WorkQueue` handles deferred symbol analysis work and worker synchronization using `available_` semaphore and `stopped_` flag. LLM requests are issued asynchronously through `request_llm_async` on the event loop, with caching via `prompt_cache_identity_for_page_request`. After all prompts complete, the scheduler calls `render_generated_pages` to produce final Markdown output through the `PageRenderer`, including building directory index pages via `build_directory_index_pages`. Error handling records consecutive failures via `record_consecutive_failure` and respects retry limits; upon completion, the function returns the total written page count from `written_page_count`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const int &` parameter
- `std::string_view` parameter

#### Usage Patterns

- likely invoked during the page generation pipeline
- probably called from higher-level generators such as `clore::generate::generate_pages`

## Internal Structure

The `generate` module provides the documentation page generation pipeline within the `clore` project. It is decomposed into several public entry points: `generate_dry_run` for previewing results without output, `generate_pages` as the synchronous generation command, `write_pages` to commit rendered pages to disk, and `generate_pages_async` for non‑blocking generation on an external event loop. Internally, the module manages shared generation state via module‑level variables such as `config`, `model`, `llm_model`, `rate_limit`, `output_root`, `pages`, and a `loop` reference, which together control model selection, concurrency limits, output location, and the asynchronous execution context. The module imports `config` for configuration parameters and `extract` for the extraction data model, forming a clear internal layering where generation relies on pre‑extracted metadata and user‑supplied settings. Asynchronous support is separated from the synchronous path: `generate_pages_async` returns a task object that must be explicitly scheduled on the provided `kota::event_loop`, while the synchronous functions handle blocking execution directly.

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)

