---
title: 'Module generate'
description: 'The generate module is responsible for producing the final documentation pages from previously extracted project data. It acts as the orchestration layer that transforms an analysis store and a page plan into rendered output files, applying configuration settings, concurrency limits, and model parameters as needed. The module depends on the config module for runtime settings and on the extract module for the data that feeds page generation.'
layout: doc
template: doc
---

# Module `generate`

## Summary

The `generate` module is responsible for producing the final documentation pages from previously extracted project data. It acts as the orchestration layer that transforms an analysis store and a page plan into rendered output files, applying configuration settings, concurrency limits, and model parameters as needed. The module depends on the `config` module for runtime settings and on the `extract` module for the data that feeds page generation.

The public API consists of four functions. `generate_dry_run` validates the page plan and generation pipeline without writing any files, returning a status code. `generate_pages` is the primary synchronous entry point that renders all pages and writes them to a specified output directory. `generate_pages_async` provides the same capability but operates asynchronously on a caller‑provided event loop, returning a task that must be scheduled and executed. Finally, `write_pages` writes an already‑constructed page set to a destination, assuming the page structures have been built beforehand. Collectively, these functions expose the full life‑cycle of page generation, from validation through to output, while keeping internal details such as model‑specific logic and loop management private.

## Imports

- [`config`](../config/index.md)
- [`extract`](../extract/index.md)

## Imported By

- [`agent`](../agent/index.md)
- [`agent:tools`](../agent/tools.md)

## Functions

### `clore::generate::generate_dry_run`

Declaration: `src/generate/generate.cppm:42`

Definition: `src/generate/scheduler.cppm:1957`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::generate_dry_run` orchestrates a simulation of the full generation pipeline without issuing any LLM requests or writing output files. It first constructs a `PreparedGenerationContext` via `prepare_generation_context`, then creates a `PageGenerationScheduler` with the `dry_run` flag set to `true` and a `PageRenderer` also configured for dry-run mode. The scheduler’s `run` method is invoked, which drives the entire workflow through its internal `WorkQueue`, `DependencyTracker`, and worker tasks.

Internally, the scheduler iterates over generation plans, deduplicates prompt requests with `deduplicate_prompt_requests`, and submits symbol analysis work via `submit_after_symbol_analysis`. A worker loop dequeues tasks (both prompt and analysis work) and processes them; in dry-run mode, `perform_prompt_request` and `request_llm_async` are bypassed, and page rendering via `PageRenderer::emit_pages` skips actual file writes. The `DependencyTracker` tracks page state (`PageState`), marks symbol prompts as ready, and releases dependents. After all work is completed or failures are detected, the function returns an aggregated result indicating success or failure counts.

#### Side Effects

No observable side effects are evident from the extracted code.

### `clore::generate::generate_pages`

Declaration: `src/generate/generate.cppm:45`

Definition: `src/generate/scheduler.cppm:2016`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::generate_pages` orchestrates the end‑to‑end page generation pipeline. It first calls `prepare_generation_context` to transform the imported `config` and `model` into a `PreparedGenerationContext` that contains all plan metadata, symbol analysis targets, and prompt request descriptors. Using this context, it constructs a `DependencyTracker` to track per‑page and per‑symbol dependency states, and a `PageGenerationScheduler` that owns a `WorkQueue`, a `PageRenderer`, and counters for LLM requests and cache activity. The scheduler is initialized with the `output_root` directory, the LLM model identifier (`llm_model`), a per‑second `rate_limit`, and a `dry_run` flag.

The control flow proceeds by discovering all documentable symbols via `collect_documentable_symbols`, then submitting `SymbolAnalysisWork` items to the work queue. Each analysis task is executed by `run_symbol_analysis_task`, which may issue an LLM request or use cached results. When a symbol analysis finishes, `DependencyTracker::mark_symbol_ready` releases dependent pages, and `try_submit_ready_pages` enqueues `PagePromptWork` for pages whose symbolic dependencies are satisfied. Page prompts are processed by `run_page_prompt_task`, which deduplicates requests via `deduplicate_prompt_requests` and performs the LLM call through `perform_prompt_request`. After obtaining the prompt output, `finish_page_prompt_work` parses the response, updates page summaries, and marks the page for rendering. The scheduler then submits `RenderPageWork` via `render_ready_page`, which calls `PageRenderer::emit_pages` to write the final HTML files. Throughout execution, the scheduler respects rate limits, tracks consecutive failures with `record_consecutive_failure`, and stops workers if `retry_limit_exceeded`. Once all work completes, directory index pages are built by `build_directory_index_pages`, and the function returns a status code indicating success or the nature of any failure.

#### Side Effects

- writes generated page files to the output path specified by the `string_view` parameter
- may modify internal cache or state passed via reference parameters

#### Reads From

- the two `const int &` parameters (likely representing analysis stores or page plans)
- the `std::string_view` parameter for output directory
- the `std::string_view` parameter for additional configuration
- the `std::uint32_t` parameter for concurrency or mode

#### Writes To

- output files via `write_page` or `write_pages` calls
- the referenced integer parameters if they are output or in-out arguments

#### Usage Patterns

- main driver for documentation generation
- called from the top-level CLI or build system entry point

### `clore::generate::generate_pages_async`

Declaration: `src/generate/generate.cppm:54`

Definition: `src/generate/scheduler.cppm:1994`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function builds a `PreparedGenerationContext` from the input configuration, model, and other parameters, then constructs a `PageGenerationScheduler` that owns a `DependencyTracker`, `WorkQueue`, and `PageRenderer`. The scheduler’s `run` method orchestrates the entire pipeline: it first initializes all page states and ready candidates, spawns worker tasks via `WorkerActivity` that repeatedly dequeue work items from the queue, and processes them until completion or failure. Workers handle three kinds of work: symbol analysis (collecting documentable symbols and issuing LLM requests to enrich symbol metadata), page prompts (generating prompt requests for each page plan, with caching via `prompt_cache_identity_for_page_request` and dependency resolution through the `DependencyTracker`), and page rendering (invoking `PageRenderer::emit_pages_async` after all prompts for a page are finished). The `DependencyTracker` maintains per-page state (unsatisfied deps, pending prompts, failure flags) and releases dependents when symbols or prompts complete. The `WorkQueue` manages concurrency with a semaphore, deferred enqueueing for symbol analysis, and flush logic to pace work. The event loop drives all asynchronous LLM requests via `request_llm_async`, and the scheduler tracks consecutive failures and retry limits. Upon completion, the function returns an integer indicating success or an error code propagated through `make_generate_error`.

#### Side Effects

- Schedules tasks on the provided `kota::event_loop`.

#### Reads From

- `int` parameters (page identifiers or sizes)
- `std::string_view` parameters (output path or content)
- `std::uint32_t` parameter (seed or options)
- `kota::event_loop&` (event loop to schedule tasks)

#### Writes To

- The `kota::event_loop` (by scheduling tasks)

#### Usage Patterns

- Callers schedule the returned task on the event loop and run it.
- Used for asynchronous documentation generation.

### `clore::generate::write_pages`

Declaration: `src/generate/generate.cppm:61`

Definition: `src/generate/scheduler.cppm:2035`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::write_pages` orchestrates a multi-stage pipeline for generating documentation pages. It constructs a `PreparedGenerationContext` via `prepare_generation_context`, then instantiates a `PageGenerationScheduler` that owns a `PageRenderer`, a `DependencyTracker`, and a `WorkQueue`. The scheduler iterates over ready candidates from the `DependencyTracker`, submitting symbol‑analysis work (`SymbolAnalysisWork`) to the `WorkQueue`. Workers dequeue tasks, run LLM prompts (with caching via `prompt_cache_identity_for_page_request` and `deduplicate_prompt_requests`), and update page state through `DependencyTracker::mark_symbol_ready` and `DependencyTracker::finish_symbol_prompt`. When all dependencies for a page are satisfied, `render_ready_page` is called to write output through `PageRenderer::emit_pages`.

Control flow depends on the `DependencyTracker` to maintain `PageState` entries and a set of `ready_candidates_`. The `WorkQueue` uses a semaphore (`available_`) and deferred queue to manage concurrency. Failure handling is tracked via `consecutive_failures_` and `retry_limit_exceeded_`. The loop terminates when `WorkQueue::stopped` returns true or all pages are written, after which `maybe_flush_deferred` finalizes any remaining deferred work.

#### Reads From

- const int & parameter (likely a state or context identifier)
- `std::string_view` parameter (likely a path or content direction)

## Internal Structure

The `generate` module is the top‑level orchestration layer for the documentation generation pipeline. It imports `config` for application settings and `extract` for preprocessed analysis data, and it exposes four public entry points: `generate_dry_run` for validation without side effects, `generate_pages` for the synchronous generation workflow, `generate_pages_async` for an asynchronous variant that runs on a `kota::event_loop`, and `write_pages` for persisting the fully built page set. Internally, the module is decomposed into a synchronous path and an async path, with shared helper functionality for page plan construction, LLM invocation, and rendering. The internal state comprises handles for pages, model, LLM model, config, rate limit, output root, and a loop reference, which together support the generation process and are wired through the public functions.

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)

