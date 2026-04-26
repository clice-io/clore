---
title: 'Module extract'
description: 'The extract module is responsible for orchestrating the asynchronous extraction of project data, including symbols, modules, and dependency graphs. It manages the entire extraction pipeline: loading and validating scan and AST caches, building module information from scan results, constructing dependency graphs, and persisting extraction outputs back into cache records. The public-facing interface is limited to the entry point clore::extract::extract_project_async, which initiates non‑blocking extraction given a project identifier and event loop.'
layout: doc
template: doc
---

# Module `extract`

## Summary

The `extract` module is responsible for orchestrating the asynchronous extraction of project data, including symbols, modules, and dependency graphs. It manages the entire extraction pipeline: loading and validating scan and AST caches, building module information from scan results, constructing dependency graphs, and persisting extraction outputs back into cache records. The public-facing interface is limited to the entry point `clore::extract::extract_project_async`, which initiates non‑blocking extraction given a project identifier and event loop.

Internally, the module owns parallel AST batch extraction (`extract_ast_batch_async`, `extract_ast_entry`), cache I/O operations (loading and saving both extract and clice caches), and data structures such as `ParallelASTResult`, `LoadedCaches`, `CacheEvaluation`, and `ExtractError` that track extraction state and cache validity. It also provides utility helpers for computing module grouping, dependency ordering, and timing metrics, all within the `clore::extract` namespace.

## Imports

- [`config`](../config/index.md)
- `std`
- [`support`](../support/index.md)

## Imported By

- [`agent`](../agent/index.md)
- [`agent:tools`](../agent/tools.md)
- [`generate`](../generate/index.md)
- [`generate:analysis`](../generate/analysis.md)
- [`generate:common`](../generate/common.md)
- [`generate:diagram`](../generate/diagram.md)
- [`generate:dryrun`](../generate/dryrun.md)
- [`generate:evidence`](../generate/evidence.md)
- [`generate:evidence_builder`](../generate/index.md)
- [`generate:model`](../generate/model.md)
- [`generate:page`](../generate/page.md)
- [`generate:planner`](../generate/planner.md)
- [`generate:scheduler`](../generate/scheduler.md)
- [`generate:symbol`](../generate/symbol.md)

## Types

### `clore::extract::ExtractError`

Declaration: `extract/extract.cppm:21`

Definition: `extract/extract.cppm:21`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The `clore::extract::ExtractError` struct is a lightweight error type designed to carry a descriptive error message. Its only data member is `message`, a `std::string` that stores the textual description of the failure. There are no other fields, invariants, or complex member functions; the struct serves as a simple, self-contained error representation that can be constructed, inspected, and propagated as needed within the extraction logic. The absence of additional state keeps the type trivial to manage and ensures that the error message is the sole source of diagnostic information.

#### Invariants

- No invariants beyond the validity of the underlying `std::string`

#### Key Members

- `message`

#### Usage Patterns

- Thrown or returned by extraction functions to indicate failure
- Caught or inspected by callers to obtain an error description

## Functions

### `clore::extract::extract_project_async`

Declaration: `extract/extract.cppm:25`

Definition: `extract/extract.cppm:539`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::extract_project_async` proceeds in two main phases. First, it loads and filters the compilation database, then asynchronously loads existing extract and clice caches via `load_caches_async`. For each entry, it computes a cache key and compile signature, evaluates cache validity (scan and AST) using `cache_evaluations` and `prepared_entries`, and builds a dependency graph via `build_dependency_graph_async` with a topology sort. A batch of parallel AST extractions is launched through `extract_ast_batch_async`, which either reuses cached AST results or invokes fresh extraction.

In the second phase, the function iterates over all filtered entries sequentially. For each entry, it either retrieves the cached AST or takes the parallel result, then resolves symbol declaration and definition file paths using `resolve_symbol_location_path`, which delegates to `run_worker_task_async` for worker‑isolation. Symbols and includes are filtered against the given `config.filter` and `filter_root`, merged into the `ProjectModel`, and relations (inheritance, calls, references) are linked. Cache records are updated after each file. After all files are processed, the function rebuilds model indexes, constructs module info via `build_module_info`, and asynchronously resolves source snippets using `kota::queue`. Caches are persisted at the end via `save_caches_async`, even on failure (through the `fail_after_persist` lambda). The final `ProjectModel` is returned, containing the extracted symbols, files, relations, and timing statistics.

#### Side Effects

- reads compilation database file specified in config`.compile_commands_path`
- reads cache files from `workspace_root` via `load_caches_async`
- writes cache files to `workspace_root` via `save_caches_async`
- modifies `cache_records` and `clice_cache` in memory
- logs extensive timing and statistics information
- may produce errors that are propagated or handled by persisting caches before failing

#### Reads From

- config (`TaskConfig`) with fields `compile_commands_path`, `workspace_root`, filter
- loop (`kota::event_loop`) for asynchronous operations
- compilation database entries (db`.entries`)
- cache records loaded from disk (`cache_records`, `clice_cache`)
- source files via scan and AST extraction
- file system for path resolution and existence checks

#### Writes To

- `cache_records` and `clice_cache` (in memory, later persisted)
- model (`ProjectModel`) which is returned and contains symbols, files, namespaces
- log output via `logging::info` calls
- `resolved_path_cache` (local map for caching path resolutions)

#### Usage Patterns

- called as the main extraction function in the clore extraction pipeline
- typically invoked from a command-line tool that provides config and event loop
- used in conjunction with async caching and AST extraction utilities

## Internal Structure

The extract module is decomposed around a single public entry point, `extract_project_async`, which orchestrates an asynchronous pipeline for extracting project metadata. Internally, the module is layered into a set of anonymous‑namespace helpers that handle cache I/O (`load_caches_async`, `save_caches_async`), parallel AST extraction (`extract_ast_batch_async`, `extract_ast_entry`), and module‑info construction (`build_module_info`). Supporting types such as `CacheEvaluation`, `PreparedEntryState`, `ParallelASTResult`, and `LoadedCaches` encapsulate cache‑hit/miss state, per‑file preparation, extraction results, and loaded cache records, respectively. The module imports `config` for settings, `support` for file I/O, hashing, path normalization, and logging, and `std` for containers and error propagation. Error handling is centralized through `ExtractError` and helpers like `unexpected_extract_error`, while timing variables (e.g., `dt_ast`, `dt_file`) reveal internal instrumentation for performance monitoring.

## Related Pages

- [Module config](../config/index.md)
- [Module support](../support/index.md)

