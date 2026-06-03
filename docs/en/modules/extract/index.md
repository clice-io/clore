---
title: 'Module extract'
description: 'The extract module orchestrates the asynchronous extraction of project data, including symbols, dependencies, and compilation units, for a given project identified by an integer handle. It serves as the core engine that coordinates scanning, parsing, caching, and result assembly. Publicly, the module exposes the main entry point extract_project_async, which schedules all extraction work on a kota::event_loop and returns an operation handle; internal functions such as extract_ast_batch_async, extract_ast_entry, and cache‑I/O helpers (load_caches_async, save_caches_async) form the private implementation scope.'
layout: doc
template: doc
---

# Module `extract`

## Summary

The `extract` module orchestrates the asynchronous extraction of project data, including symbols, dependencies, and compilation units, for a given project identified by an integer handle. It serves as the core engine that coordinates scanning, parsing, caching, and result assembly. Publicly, the module exposes the main entry point `extract_project_async`, which schedules all extraction work on a `kota::event_loop` and returns an operation handle; internal functions such as `extract_ast_batch_async`, `extract_ast_entry`, and cache‑I/O helpers (`load_caches_async`, `save_caches_async`) form the private implementation scope.

Behind the scenes, the module manages several interlocking data structures: `ExtractError` for reporting failures; `ParallelASTResult`, `LoadedCaches`, `CacheEvaluation`, `PreparedEntryState`, and `GroupedModuleInfo` for tracking parallel AST processing, cache coherence, and module information. It imports the `config` and `support` modules for configuration and low‑level utilities, and its internal logic handles dependency graph construction (`dep_graph`), database filtering (`filtered_db`), and both scan‑cache and extract‑cache validation. The result is an efficient, cache‑aware pipeline that produces a consistent project model from source files.

## Imports

- [`config`](../config/index.md)
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

Declaration: `src/extract/extract.cppm:43`

Definition: `src/extract/extract.cppm:43`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::ExtractError` is a lightweight error type comprising a single data member, `message`, of type `std::string`. This member stores a human-readable description of the extraction failure. The struct imposes no special invariants; the string is assumed to be in a valid state as defined by `std::string` semantics. No additional constructors, assignment `operator`s, or member functions are present, making the type a plain data carrier with default copy, move, and destruction behavior supplied by the compiler.

#### Invariants

- `message` may be empty or contain a descriptive error text

#### Key Members

- `message` member

#### Usage Patterns

- Returned from extraction functions when an error occurs
- Accessed to display or log the error message

## Functions

### `clore::extract::extract_project_async`

Declaration: `src/extract/extract.cppm:47`

Definition: `src/extract/extract.cppm:561`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function is a coroutine (`kota::task`) that orchestrates the multi‑phase extraction of a `ProjectModel` from a compilation database. It begins by loading and filtering the database entries using `matches_filter` and `filter_root_path`, then asynchronously loads existing caches via `load_caches_async`. For each filtered entry, it computes a `compile_signature`, `cache_key`, and `CacheEvaluation` record; if the entry’s source hash, signature, and AST dependencies match a prior cache entry, the scan and AST are reused (incrementing `ast_cache_hits`), otherwise a miss is recorded. A `DependencyGraph` is built asynchronously (`build_dependency_graph_async`), seeded with valid scan results, and a topological order is computed.

Next, the function launches an asynchronous batch AST extraction (`extract_ast_batch_async`) for entries that are not fully cached. After obtaining the parallel AST results, it iterates over all entries in topological order. For each entry, it either uses the cached AST (`ast_view` from `cache_records`) or the freshly extracted `parallel_asts`. Symbol declarations and definitions are resolved via `resolve_symbol_location_path`, filtered against the project filter, and inserted or merged into `model.symbols`. Relations (inheritance, calls, references) are added to the model. Cache records are updated or inserted based on source hash availability. After processing all entries, `rebuild_model_indexes`, `build_module_info`, and `rebuild_lookup_maps` are called, followed by asynchronous source snippet resolution. Finally, all caches are persisted via `save_caches_async`, and the assembled `ProjectModel` is returned. Throughout, timing and diagnostic logs are emitted, and errors trigger a `fail_after_persist` lambda that attempts to save caches before failing the coroutine.

#### Side Effects

- loads and saves cache files via `load_caches_async` and `save_caches_async`
- modifies and returns the `ProjectModel`
- performs logging at multiple stages
- reads source files and compilation database from disk
- resolves filesystem paths, possibly performing I/O

#### Reads From

- `config::TaskConfig` (config)
- `kota::event_loop` (loop)
- compilation database file (via `load_compdb`)
- cache files (via `load_caches_async`)
- source files (via `extract_ast_batch_async`)
- filesystem for path resolution

#### Writes To

- returned `ProjectModel`
- cache files (via `save_caches_async`)
- logging output

#### Usage Patterns

- top-level entry point for project extraction
- used with an event loop for asynchronous execution
- integrated with caching for incremental extraction
- called once per project analysis session

## Internal Structure

The `extract` module is the core asynchronous extraction engine, organized around several internal layers that handle cache I/O, parallel AST processing, and dependency resolution. At the top level, `extract_project_async` orchestrates the entire pipeline by coordinating cache loading via `load_caches_async`, batch AST extraction through `extract_ast_batch_async`, and final cache persistence with `save_caches_async`. Internally, the module defines anonymous‑namespace structures such as `CacheEvaluation`, `LoadedCaches`, `ParallelASTResult`, and `PreparedEntryState` to represent intermediate state, while functions like `extract_ast_entry` and `build_module_info` encapsulate per‑entry and module‑level logic. Concurrency is managed through `kota::event_loop` handles, and all I/O‑bound operations (loading and saving clice/extract caches) are isolated in `run_cache_io_async` and `run_worker_task_async` to separate them from compute‑intensive AST work.

The module imports from `config` and `support`, relying on `config` for application settings and `support` for foundational utilities such as string normalization, caching helpers, and file handling. This import structure provides clear separation: configuration drives the extraction parameters, while support utilities handle low‑level data manipulation and logging. The implementation structure thus layers cache management at the top, batch and entry‑level extraction in the middle, and utility functions at the bottom, all tied together by the asynchronous event loop pattern that enables non‑blocking extraction of project symbol, dependency, and compilation‑unit data.

## Related Pages

- [Module config](../config/index.md)
- [Module support](../support/index.md)

