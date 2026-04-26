---
title: 'Module extract:scan'
description: 'The extract:scan module is responsible for scanning C++ source files to extract module declaration and include information, as well as constructing inter-file dependency graphs. Its primary public functions are scan_file, which returns a ScanResult or a ScanError for a single file; scan_module_decl, which quickly populates module name, interface unit status, and imports using Clang’s dependency directives scanner; and build_dependency_graph_async, which asynchronously builds a complete DependencyGraph from a compilation database handle, optionally using a ScanCache for persistent caching across scans. A helper function topological_order computes a topological ordering of the graph’s nodes.'
layout: doc
template: doc
---

# Module `extract:scan`

## Summary

The `extract:scan` module is responsible for scanning C++ source files to extract module declaration and include information, as well as constructing inter-file dependency graphs. Its primary public functions are `scan_file`, which returns a `ScanResult` or a `ScanError` for a single file; `scan_module_decl`, which quickly populates module name, interface unit status, and imports using Clang’s dependency directives scanner; and `build_dependency_graph_async`, which asynchronously builds a complete `DependencyGraph` from a compilation database handle, optionally using a `ScanCache` for persistent caching across scans. A helper function `topological_order` computes a topological ordering of the graph’s nodes.

The module owns the public types `ScanResult` (holding module name, interface flag, imports, includes, and errors), `ScanCache` (shared cache invalidated on compilation DB or filesystem changes), `DependencyGraph` (with file lists and directed edges), `DependencyEdge` (representing a single dependency), `IncludeInfo` (with path and angled flag), and `ScanError` (with a message string). Supporting internal machinery includes `ScanAction`, `ScanPPCallbacks`, `PreparedScanEntry`, and `MissingScanTask`, all of which work together to drive the scanning and graph-building pipeline.

## Imports

- [`extract:compiler`](compiler.md)
- `std`
- [`support`](../support/index.md)

## Imported By

- [`extract:cache`](cache.md)

## Types

### `clore::extract::DependencyEdge`

Declaration: `extract/scan.cppm:51`

Definition: `extract/scan.cppm:51`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::DependencyEdge` models a directed dependency relationship between two named entities. It aggregates two `std::string` members, `from` and `to`, indicating the source and target of the dependency edge. The structure imposes no invariants beyond standard string validity; both fields can be empty, but in practice they are expected to contain non-empty identifiers to form meaningful edges. This aggregate type provides no custom constructors or member functions, relying on default copy, move, and comparison semantics. Its primary purpose is to serve as a lightweight value type within dependency graph construction and traversal logic.

#### Invariants

- `from` and `to` are distinct identifiers (implied by edge semantics, not enforced)
- Members are public and may be mutated directly
- No ownership or lifetime constraints beyond those of `std::string`

#### Key Members

- `from`: the source node identifier
- `to`: the target node identifier

#### Usage Patterns

- Used to construct dependency graphs or lists
- Stored in containers such as `std::vector<DependencyEdge>`
- Iterated over to extract source/target pairs for further processing

### `clore::extract::DependencyGraph`

Declaration: `extract/scan.cppm:56`

Definition: `extract/scan.cppm:56`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct serves as a flat container for the extracted dependency graph, holding two parallel vectors: `files` stores the canonical paths of all source files encountered, and `edges` stores the directed dependencies between them. The invariant that each `DependencyEdge` must reference indices valid within `files` is expected to be maintained by the functions that populate the graph. No special construction or assignment logic is provided; the struct is a plain aggregate.

#### Invariants

- No explicitly documented invariants.
- All members are public and default-initialized to empty vectors.

#### Key Members

- `files`
- `edges`

#### Usage Patterns

- Instantiated and populated by dependency extraction routines.
- Consumed by downstream analysis or serialization code.

### `clore::extract::IncludeInfo`

Declaration: `extract/scan.cppm:24`

Definition: `extract/scan.cppm:24`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::IncludeInfo` is a lightweight aggregate data structure that captures the two essential characteristics of a C++ `#include` directive: the included `path` and whether the directive used angle brackets (`<>`) versus quotes (`""`), stored in the `is_angled` flag. The default member initializers ensure that a default‑constructed instance holds an empty `path` and `is_angled` set to `false`.

As an implementation detail, the struct imposes no invariants beyond the natural constraints of its members: `path` is an arbitrary `std::string`, and `is_angled` is a `bool`. No custom constructors, assignment `operator`s, or other special member functions are defined, so the compiler‑generated defaults apply. The primary role of this type is to serve as a simple data carrier between parsing steps, where its fields are populated by the include‑directive recognition logic and later consumed by higher‑level extraction routines.

#### Invariants

- `is_angled` distinguishes angle-bracket includes from quoted includes.
- `path` can be any string, including empty.

#### Key Members

- `path`
- `is_angled`

#### Usage Patterns

- Returned by parsing functions to represent a single include directive.
- Consumed by downstream logic to determine include search behavior.

### `clore::extract::ScanCache`

Declaration: `extract/scan.cppm:40`

Definition: `extract/scan.cppm:40`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct contains a single data member: an `std::unordered_map<std::string, ScanResult>` named `scan_results`. This map serves as the persistent storage for previously computed scan outcomes, keyed by the file path (as a `std::string`). The only invariant is that the map’s entries remain valid only while the compilation database and file system state are unchanged; when either of those changes, the entire cache must be cleared or discarded. Because the struct has no other members, all operations are forwarded to the underlying unordered map, and the implementation’s correctness depends on callers respecting the invalidation contract.

#### Invariants

- Cache entries are valid only until compilation DB or file system state changes.
- The `scan_results` map is unordered; iteration order is not guaranteed.

#### Key Members

- `scan_results`: maps file paths to cached `ScanResult` objects.

#### Usage Patterns

- Shared across successive dependency scans to avoid redundant work.
- Callers must clear or discard the cache when compilation DB or file system state changes.

### `clore::extract::ScanError`

Declaration: `extract/scan.cppm:20`

Definition: `extract/scan.cppm:20`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::ScanError` is defined with a single data member `message` of type `std::string`. It is an aggregate, so all special member functions (default constructor, copy, move, and assignment) are implicitly declared and defined by the compiler. No invariants are enforced within the struct; the content of `message` is arbitrary and is stored exactly as provided. This minimal implementation serves as a lightweight container for error descriptions, intended to be used as the error type in return values or thrown objects without any additional logic or validation.

#### Invariants

- `message` may be empty or contain any valid string

#### Key Members

- `std::string message`

#### Usage Patterns

- Returned as an error result from scanning functions
- Likely used with `std::expected` or similar error-handling mechanisms

### `clore::extract::ScanResult`

Declaration: `extract/scan.cppm:29`

Definition: `extract/scan.cppm:29`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::ScanResult` is an aggregate data container designed to hold the outcome of scanning a C++ source file for module and dependency information. Its internal structure consists of four public fields: `module_name` (a `std::string` defaulting to empty), `is_interface_unit` (a `bool` defaulting to `false`), `includes` (a `std::vector<IncludeInfo>` defaulting to empty), and `module_imports` (a `std::vector<std::string>` defaulting to empty). The default member initializers ensure that every default‑constructed `ScanResult` starts with a cleared state, avoiding any uninitialized values. No invariants are enforced beyond the guarantees of the underlying types; the fields are directly mutable and intended to be populated during scanning and read afterward. There are no custom constructors, assignment `operator`s, or member functions, making the struct a plain data carrier that serves as the output from the extraction process.

#### Invariants

- Fields are default-initialized to empty strings, false, or empty vectors.

#### Key Members

- `module_name`
- `is_interface_unit`
- includes
- `module_imports`

#### Usage Patterns

- Used as a return type from scanning functions
- Consumed to process module information

## Functions

### `clore::extract::build_dependency_graph_async`

Declaration: `extract/scan.cppm:61`

Definition: `extract/scan.cppm:370`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first clears the existing `graph.files` and `graph.edges`, then iterates over each compilation database entry to call `prepare_scan_entry` and populate a vector of `PreparedScanEntry` objects. During this pass it deduplicates normalized file paths into `graph.files`. It then partitions entries into those with cached results (looked up in `ScanCache::scan_results` if a `cache` pointer is provided) and those requiring re‑scanning. For the latter, it deduplicates by `cache_key` to create a set of `MissingScanTask` objects.

For each distinct missing task, it launches an asynchronous `run_scan_task` on the provided `kota::event_loop` and collects the resulting `kota::task` objects into a vector. The function then co‑awaits `kota::when_all` on all those tasks, handling a possible `ScanError` and verifying the result count. With all scan results available (either from cache or from the newly scanned batch), it iterates over every original entry. For each entry it normalizes include paths (from `scan_result->includes`) and adds a `DependencyEdge` to `graph.edges` if the target file is among the entry files and the edge has not already been emitted (deduplicated via a compound key). Finally, if a cache pointer exists, it stores every scan result back into `cache->scan_results`. The function co‑returns `void` upon success.

#### Side Effects

- Clears and populates `graph.files` and `graph.edges`
- Inserts or assigns scan results into `cache->scan_results` if cache is non-null

#### Reads From

- `db` (compilation database entries)
- `cache` (previously cached scan results)
- `prepared_entries` (normalized file paths and cache keys)
- `cached_results` and `scanned_results` vectors

#### Writes To

- `graph.files`
- `graph.edges`
- `cache->scan_results` (if cache is non-null)
- Local containers: `entry_files`, `file_set`, `prepared_entries`, `cached_results`, `missing_task_indices`, `missing_task_by_cache_key`, `missing_tasks`, `scanned_results`, `seen_files`, `emitted_edges`

#### Usage Patterns

- Called to asynchronously compute a dependency graph for a project given a compilation database

### `clore::extract::scan_file`

Declaration: `extract/scan.cppm:44`

Definition: `extract/scan.cppm:238`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first validates that the compile entry has a non-empty argument list, returning a `ScanError` if not. It then reads the source file content from `entry.normalized_file` (falling back to `normalize_entry_file`) and passes it to `scan_module_decl` to extract the module declaration and module name, populating the `ScanResult` early. Next, it creates a compiler instance via `create_compiler_instance`, configures it for preprocessor‑only execution, and instantiates a `ScanAction` bound to the `ScanResult`. The `ScanAction` (which internally uses `ScanPPCallbacks`) is called to preprocess the file, capturing include directives and module imports into `ScanResult::includes` and `ScanResult::module_imports`. After successful execution, the result is returned; any failure at any stage—file I/O errors, compiler instance creation failure, or preprocessor errors—produces a `ScanError` describing the issue. Dependencies include `scan_module_decl`, `normalize_entry_file`, `ScanAction`/`ScanPPCallbacks`, and the `CompileEntry`/`ScanResult`/`ScanError` types.

#### Side Effects

- reads source file content from the filesystem
- creates a compiler instance (may allocate resources or spawn processes)
- modifies the `ScanResult` object passed via the action
- modifies frontend options on the compiler instance

#### Reads From

- `entry.arguments`
- `entry.file`
- `entry.normalized_file`
- source file content via `std::ifstream`

#### Writes To

- `result` (of type `ScanResult`)
- `instance->getInvocation().getFrontendOpts()` fields

#### Usage Patterns

- called for each source file during project extraction
- used in conjunction with `extract_project_async` and dependency graph building
- typically invoked after compiling a compilation database entry
- may be called from worker threads in a parallel extraction pipeline

### `clore::extract::scan_module_decl`

Declaration: `extract/scan.cppm:49`

Definition: `extract/scan.cppm:141`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/scan-module-decl.md)

The implementation of `clore::extract::scan_module_decl` relies entirely on Clang's dependency directives scanner (`clang::scanSourceForDependencyDirectives`) to quickly tokenize the file content without running the full preprocessor. This produces a list of `dependency_directives_scan::Directive` entries, which are then iterated to identify module and import declarations. For each `cxx_module_decl` or `cxx_export_module_decl` directive, the function walks the associated tokens past the `module` keyword, collecting name tokens (identifiers, dots, colons) while skipping whitespace and ignoring punctuation-only tokens that indicate a module fragment. The collected name becomes `result.module_name`, and `result.is_interface_unit` is set based on whether the declaration was `export module`. For each `cxx_import_decl` directive, tokens after the `import` keyword are concatenated into an import name, which is then normalized via `normalize_partition_import` and inserted into `result.module_imports` after a duplicate check. If the scanner fails (returns non‑zero), the function silently returns without modifying the result, leaving it in its default state.

#### Side Effects

- modifies the `module_name`, `is_interface_unit`, and `module_imports` fields of the provided `ScanResult` object

#### Reads From

- the `file_content` string parameter
- tokens and directives produced by `clang::scanSourceForDependencyDirectives`

#### Writes To

- `result.module_name`
- `result.is_interface_unit`
- `result.module_imports`

#### Usage Patterns

- called by `clore::extract::scan_file` to extract module information from source content
- used as a fast alternative to full preprocessing for module detection

### `clore::extract::topological_order`

Declaration: `extract/scan.cppm:66`

Definition: `extract/scan.cppm:495`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function constructs an adjacency list and an in‑degree map from the provided `graph.files` and `graph.edges`. Each file is initialized with an in‑degree of zero, then every `DependencyEdge` is processed by adding `edge.from` to the adjacency list of `edge.to` and incrementing the in‑degree of `edge.from`. This inverted adjacency (mapping a file to its dependents) matches the expected interface of `clore::support::topological_order`, which receives the file list, the adjacency map, and the in‑degree map. If the support function returns a cycle (no valid order), the function returns a `std::unexpected` containing a `ScanError` with a descriptive message. Otherwise, the computed topological order is forwarded to the caller.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- graph`.files`
- graph`.edges`

#### Usage Patterns

- Ordering files for sequential processing according to include dependencies.

## Internal Structure

The `extract:scan` module builds on three direct imports—`extract:compiler` for compilation database entries, `support` for file I/O and cache-key utilities, and the C++ standard library—to provide the core scanning pipeline. Its public surface is split into two major operations: synchronous file scanning via `scan_file` and `scan_module_decl` for lightweight module declaration extraction, and asynchronous dependency‑graph construction through `build_dependency_graph_async` and `topological_order`. These functions rely on shared data types (`ScanResult`, `ScanError`, `DependencyGraph`, `DependencyEdge`, `IncludeInfo`) and a persistent `ScanCache` that callers must invalidate when the compilation database or filesystem state changes.

Internally, the module is layered into an anonymous‑namespace implementation that encapsulates the Clang‑based preprocessor interaction. A `ScanPPCallbacks` class captures inclusion directives and module imports during a preprocessor run, driven by a `ScanAction` that wraps the scanning logic for a single file. Preparation and deduplication are handled by `PreparedScanEntry` and `MissingScanTask` structures, while `normalize_partition_import` and `prepare_scan_entry` provide lightweight normalization and cache‑key computation. The asynchronous graph builder uses a `kota::event_loop` to schedule scan tasks, manage a work queue of missing entries, and produce a topological ordering, with all intermediate state kept in local variables (e.g., `prepared_entries`, `missing_task_by_cache_key`, `scanned_results`) rather than exposed in the public interface.

## Related Pages

- [Module extract:compiler](compiler.md)
- [Module support](../support/index.md)

