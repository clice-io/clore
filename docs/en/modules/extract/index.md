---
title: 'Module extract'
description: 'The extract module is responsible for orchestrating the asynchronous extraction of project metadata from source code. It parses translation units, builds a dependency graph of modules and files, and produces a structured model of the project’s symbols and their relationships. The module owns the public entry point extract_project_async, which accepts a project identifier and an event loop to drive the extraction pipeline, returning an integer handle for the operation. It also exposes the ExtractError struct as a uniform error type for extraction failures.'
layout: doc
template: doc
---

# Module `extract`

## Summary

The `extract` module is responsible for orchestrating the asynchronous extraction of project metadata from source code. It parses translation units, builds a dependency graph of modules and files, and produces a structured model of the project’s symbols and their relationships. The module owns the public entry point `extract_project_async`, which accepts a project identifier and an event loop to drive the extraction pipeline, returning an integer handle for the operation. It also exposes the `ExtractError` struct as a uniform error type for extraction failures.

Internally, the module manages a multi‑stage pipeline: loading and saving caches (scan caches, clice caches, and extract‑cache records), evaluating cache validity, performing parallel AST extraction via batching, resolving module dependencies, and building grouped module information. Publicly visible types and functions are limited to the main extraction interface and error reporting; all caching, batching, and parallel logic is encapsulated within anonymous namespaces to keep the public API minimal and focused on asynchronous project extraction.

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

The `clore::extract::ExtractError` struct is implemented as a simple wrapper around a single `std::string` member named `message`. Its internal structure imposes no invariants beyond those of `std::string`; the `message` may be empty or contain any descriptive text. No custom constructor, assignment `operator`, or other member functions are defined, so the compiler provides default implementations for copy, move, and destruction. This minimal design makes the type lightweight and suitable for use as a straightforward error object that carries a human-readable explanation of an extraction failure.

#### Invariants

- The `message` member conforms to all invariants of `std::string` (e.g., valid state, no null pointer).
- The `message` may be empty, indicating a generic error.

#### Key Members

- `std::string message`

#### Usage Patterns

- Used as the `what()` or error payload in exception types or `std::error_code`-based error handling.
- May be constructed with a string literal or localised error description.
- Potentially returned as part of a `std::expected` or similar outcome type.

## Functions

### `clore::extract::extract_project_async`

Declaration: `extract/extract.cppm:25`

Definition: `extract/extract.cppm:539`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first loads and filters the compilation database, then loads persisted cache records and the clice cache via `load_caches_async`. For each filtered entry it computes a cache key, compile signature, and source hash, then evaluates whether the scan and AST results are cached (checking `CacheEvaluation::scan_valid` and `CacheEvaluation::ast_valid`). It builds a `DependencyGraph` using the seeded scan cache, computes a topological file order, and launches `extract_ast_batch_async` to obtain `ParallelASTResult` objects for files whose AST cache is invalid. For each file it either reuses the cached `ASTResult` or takes the parallel result, then resolves symbol declaration and definition locations using `resolve_symbol_location_path`, filters symbols and includes against the project filter, and merges them into the `ProjectModel`. After all files are processed it calls `rebuild_model_indexes`, `build_module_info`, and resolves source snippets, then persists caches via `save_caches_async`. The function uses `kota::event_loop` for all asynchronous operations and relies on `kota::fail` for error propagation, with a special `fail_after_persist` lambda that attempts to save caches before yielding the error.

#### Side Effects

- loads compilation database from disk
- loads caches from disk (`cache_records`, `clice_cache`)
- saves caches to disk on success or failure
- logs progress, timing, and diagnostic information
- creates and populates `ProjectModel` (symbols, files, includes, relations)
- resolves source snippets asynchronously
- rebuilds lookup maps and model indexes

#### Reads From

- config (`config::TaskConfig`)
- compilation database (`std::string_view` `compile_commands_path`)
- cache files on disk
- source files (via AST extraction and dependency scanning)
- dependency graph (`DependencyGraph`)

#### Writes To

- `ProjectModel` (model`.symbols`, model`.files`, model`.includes`, model`.relations`, etc.)
- `cache_records` (`std::unordered_map<std::string, CacheRecord>`)
- `clice_cache` (`CliceCache`)
- log output (via `logging::info`, `logging::warn`, `logging::cache_hit_rate`)

#### Usage Patterns

- top-level entry point for project extraction
- asynchronously called with a task configuration and event loop
- used in clore tool to generate project model from compile commands

## Internal Structure

The `extract` module is structured around a single public entry point, `extract_project_async`, which accepts a project identifier and an event loop to drive asynchronous work. All core logic resides in an anonymous namespace, decomposing the extraction into distinct phases: cache I/O (load/save caches), worker task scheduling (via `run_worker_task_async` and `run_cache_io_async`), parallel AST extraction (`extract_ast_entry`, `extract_ast_batch_async`), and module‑info construction (`build_module_info`). Data flows through transient structs such as `CacheEvaluation`, `PreparedEntryState`, `ParallelASTResult`, and `LoadedCaches`, which encapsulate per‑file states, cache records, and dependency snapshots. Asynchronous coordination uses `kota::event_loop` and lightweight task handles (`int` return values), enabling concurrent cache operations and AST processing without blocking the caller.

The module imports `config` for extraction parameters, `support` for foundational utilities (logging, cache‑key generation, path normalization), and `std` for standard types. Internally, the code is layered: low‑level helper functions (e.g., `make_exception_error`, `unexpected_extract_error`) provide uniform error handling with `ExtractError`; the cache layer (load/save functions) isolates persistence from the core extraction logic; and the AST layer (`extract_ast_entry`, `extract_ast_batch_async`) performs per‑file symbol extraction, feeding results into module building (`build_module_info`). This separation of concerns allows independent evolution of caching, I/O parallelism, and symbol analysis while keeping the public interface minimal.

## Related Pages

- [Module config](../config/index.md)
- [Module support](../support/index.md)

