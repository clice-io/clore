---
title: 'Module extract:scan'
description: 'The extract:scan module is responsible for scanning C++ source files to extract module declarations, #include directives, and dependency information using Clang’s dependency directives scanner. It provides the core data structures for representing scan results (ScanResult, ScanError, IncludeInfo), dependency graphs (DependencyGraph, DependencyEdge), and a persistent cache (ScanCache) to avoid redundant scanning across successive runs. The module owns the public functions scan_module_decl (fast module declaration scan without full preprocessing), scan_file (full file scan returning a ScanResult or ScanError), and the asynchronous build_dependency_graph_async (constructs a dependency graph from a compilation database). It also exposes topological_order for computing a topological ordering of a dependency graph. The implementation includes internal helpers (ScanPPCallbacks, ScanAction, PreparedScanEntry) and relies on the extract:compiler and support modules for compilation database access and foundational utilities.'
layout: doc
template: doc
---

# Module `extract:scan`

## Summary

The `extract:scan` module is responsible for scanning C++ source files to extract module declarations, `#include` directives, and dependency information using Clang’s dependency directives scanner. It provides the core data structures for representing scan results (`ScanResult`, `ScanError`, `IncludeInfo`), dependency graphs (`DependencyGraph`, `DependencyEdge`), and a persistent cache (`ScanCache`) to avoid redundant scanning across successive runs. The module owns the public functions `scan_module_decl` (fast module declaration scan without full preprocessing), `scan_file` (full file scan returning a `ScanResult` or `ScanError`), and the asynchronous `build_dependency_graph_async` (constructs a dependency graph from a compilation database). It also exposes `topological_order` for computing a topological ordering of a dependency graph. The implementation includes internal helpers (`ScanPPCallbacks`, `ScanAction`, `PreparedScanEntry`) and relies on the `extract:compiler` and `support` modules for compilation database access and foundational utilities.

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

The internal structure of `clore::extract::DependencyEdge` is a simple aggregate containing two public `std::string` data members, `from` and `to`. These strings represent the source and target nodes of a directed edge within a dependency graph. There are no explicit invariants enforced: the struct imposes no constraints on the contents of `from` or `to` beyond what `std::string` itself provides, and it has no custom constructors, assignment `operator`s, or any other member functions. The implementation is therefore entirely trivial, serving as a lightweight, value-oriented container for edge data that can be aggregate-initialized or default-constructed.

#### Invariants

- both `from` and `to` are `std::string` objects with no additional constraints
- the struct has no user‑defined constructors, destructors, or member functions

#### Key Members

- `from`
- `to`

#### Usage Patterns

- used to model a directed dependency from `from` to `to` in dependency analysis
- likely aggregated into collections or graphs for further processing

### `clore::extract::DependencyGraph`

Declaration: `extract/scan.cppm:56`

Definition: `extract/scan.cppm:56`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct is implemented as a flat adjacency representation using two parallel vectors: `files` holds each unique file path as a `std::string`, and `edges` stores a sequence of `DependencyEdge` objects that define directed dependencies between those files. Each `DependencyEdge` is expected to contain source and target indices that refer to entries in `files`. A key invariant is that all such indices remain within the bounds of `files` for the lifetime of the graph. The implementation relies on this separation of node storage and edge storage to keep lookups by index cheap and to simplify serialisation. Core operations typically manage the integrity of these two vectors, ensuring that `files` is populated before `edges` are added and that no duplicate file paths are inserted.

#### Invariants

- All dependency information is stored in the two vectors.
- No additional constraints on the ordering or content of `files` or `edges` are indicated.

#### Key Members

- `files`: the list of file names involved
- `edges`: the list of dependency connections between files

#### Usage Patterns

- Filled by extraction logic and consumed by downstream processing steps.
- Expected to be passed by value or reference as a complete dependency snapshot.

### `clore::extract::IncludeInfo`

Declaration: `extract/scan.cppm:24`

Definition: `extract/scan.cppm:24`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The `clore::extract::IncludeInfo` struct aggregates two data members: `path` of type `std::string` and `is_angled` of type `bool`. Its default initialisation guarantees that `path` is an empty string and `is_angled` is false, establishing a consistent base state before any value is assigned. Internal invariants are minimal: `path` may hold any valid file‑path string, and `is_angled` should reflect whether the include directive used angle brackets (`<...>`) rather than quotes (`"..."`). No additional member functions or special member definitions are provided, so all construction and assignment rely on compiler‑generated defaults.

#### Invariants

- `path` may be empty or contain a file path string.
- `is_angled` is `true` for angle-bracket includes, `false` otherwise.

#### Key Members

- `path`
- `is_angled`

#### Usage Patterns

- Used as a record type to store include directive data during scanning.
- Likely populated by parsing include lines and then consumed for further processing or reporting.

### `clore::extract::ScanCache`

Declaration: `extract/scan.cppm:40`

Definition: `extract/scan.cppm:40`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::ScanCache` is implemented as a thin wrapper around an `std::unordered_map<std::string, ScanResult>` stored in the `scan_results` member. This map serves as the sole internal storage, keying previously computed scan outcomes by file path (or another string identifier). The central invariant is that each entry remains valid and reusable across successive dependency scans only as long as the underlying compilation database and file system state are unchanged. When those external states are invalidated, the entire cache must be cleared or discarded. No additional logic or housekeeping is present; the implementation relies entirely on the caller to enforce this lifetime invariant.

#### Invariants

- Cache entries remain valid only while the compilation DB and file system state are unchanged.
- The `scan_results` map is initially empty.
- Callers are responsible for invalidating the cache when external state changes.

#### Key Members

- `scan_results` (`std::unordered_map<std::string, clore::extract::ScanResult>`)

#### Usage Patterns

- Stores and retrieves previously computed scan results by file path to avoid redundant scans.
- Passed into scan functions to provide cached results across successive invocations.
- Cleared or replaced by callers upon compilation DB or file system changes.

### `clore::extract::ScanError`

Declaration: `extract/scan.cppm:20`

Definition: `extract/scan.cppm:20`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The `ScanError` struct is implemented as a simple aggregate containing a single `std::string message` field. There are no custom constructors, destructors, or assignment `operator`s; all special member functions are implicitly defined, making the type trivially copyable and movable through its string member. The only invariant is that `message` holds a human‑readable description of the error condition that occurred during extraction. Because the struct has no internal logic, its role is purely to bundle error text for propagation through the error‑handling path. No custom member implementations exist—the field provides direct access to the error string, and the default compiler‑generated operations handle all lifetime and copy semantics.

#### Invariants

- No explicit invariants documented.

#### Key Members

- `message`

#### Usage Patterns

- No usage patterns documented in the evidence.

### `clore::extract::ScanResult`

Declaration: `extract/scan.cppm:29`

Definition: `extract/scan.cppm:29`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::ScanResult` is a plain aggregate designed to hold the output of a scanning pass over a translation unit. Its data members — `module_name`, `is_interface_unit`, `includes`, and `module_imports` — are all default‑initialized to empty or false values, ensuring that every `ScanResult` begins in a well‑defined, zero‑overhead state before population. No custom constructors, destructors, or assignment `operator`s are declared; the class relies entirely on implicit compiler‑generated special members, and there are no internal invariants beyond the natural consistency of the aggregate fields (e.g., `is_interface_unit` should be `true` only if `module_name` is non‑empty). Member functions are absent, making `ScanResult` a pure data container that is filled incrementally during extraction and consumed by later processing stages.

#### Invariants

- Fields are default-initialized when the struct is value-initialized.
- No additional invariants are enforced by the struct.

#### Key Members

- `module_name`
- `is_interface_unit`
- `includes`
- `module_imports`

#### Usage Patterns

- Used as a return type for scanning operations.

## Functions

### `clore::extract::build_dependency_graph_async`

Declaration: `extract/scan.cppm:61`

Definition: `extract/scan.cppm:370`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first clears the output `graph.files` and `graph.edges`, then iterates over each entry in the `CompilationDatabase` to prepare a `PreparedScanEntry` via `prepare_scan_entry`. It deduplicates normalized file paths into `graph.files` and stores per‑entry cache keys. For each entry, it checks the optional `ScanCache` for a cached `ScanResult`; if a cache hit is found, the result is retained; otherwise, a `MissingScanTask` is recorded, deduplicated by cache key. All missing tasks are launched concurrently using `run_scan_task` on the given `kota::event_loop`. The function awaits `kota::when_all` on the task collection, propagating any `ScanError` or a size mismatch via `kota::fail`.

After all scans complete, the function iterates over every original entry, retrieves the corresponding `ScanResult` (from cache or the scanned results). For each include directive in the result, it normalizes the include path and, if that path is in the set of entry files and the edge has not been emitted before, appends a `DependencyEdge` from the normalized source file to the normalized include file in `graph.edges`. Finally, the function writes each used `ScanResult` back into the `ScanCache` (if provided) and returns via `co_return`.

#### Side Effects

- Modifies `graph` by clearing and adding files and edges
- Modifies `ScanCache` if provided by inserting scan results
- Performs asynchronous file I/O and dependency scanning via `co_await`

#### Reads From

- `db.entries` from `CompilationDatabase`
- `cache->scan_results` if `cache` is non-null
- `prepared_entries[i].cache_key`

#### Writes To

- `graph.files`
- `graph.edges`
- `cache->scan_results` if `cache` is non-null

#### Usage Patterns

- Called to asynchronously build a dependency graph for a set of compilation entries
- Used in pipeline that constructs project model for code analysis

### `clore::extract::scan_file`

Declaration: `extract/scan.cppm:44`

Definition: `extract/scan.cppm:238`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first validates that `entry.arguments` is non-empty, returning a `ScanError` on failure. It then attempts a fast pre‑scan by reading the source file content from `entry.normalized_file` and invoking `scan_module_decl` to populate module‑related fields in the `ScanResult`. After this, a compiler instance is created via `create_compiler_instance`; a null instance causes an immediate `ScanError` return. The instance is reconfigured for preprocessor‑only mode by adjusting `frontend_opts`. A `ScanAction` is constructed against the `result`, and preprocessing proceeds through `BeginSourceFile` and `Execute`. Any preprocessing errors are wrapped in a `ScanError` and returned; otherwise the function returns the fully populated `ScanResult`. The internal control flow relies on `scan_module_decl`, the `ScanAction` and `ScanPPCallbacks` types, and the `kota::event_loop` infrastructure for asynchronous tasks if triggered by downstream functions (e.g., `build_dependency_graph_async`), though this entry point itself is synchronous.

#### Side Effects

- Reads the source file from disk using `std::ifstream`
- Creates a Clang compiler instance (`create_compiler_instance`) which may load toolchain and system include files
- Executes `ScanAction` which performs preprocessor scanning (I/O for included headers)
- Allocates strings and other objects via `std::format` and Clang `APIs`

#### Reads From

- `entry.arguments`
- `entry.file`
- `entry.normalized_file`
- Source file content from disk
- Clang compiler instance state via `instance`
- `ScanAction` internal state

#### Usage Patterns

- Called to scan a single compile entry during project extraction
- Used after `normalize_entry_file` to process source files
- Part of the extraction pipeline that populates `ScanResult` with module and symbol information

### `clore::extract::scan_module_decl`

Declaration: `extract/scan.cppm:49`

Definition: `extract/scan.cppm:141`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/scan-module-decl.md)

The implementation of `clore::extract::scan_module_decl` relies on Clang’s dependency directives scanner to quickly parse a source file’s module-related constructs without invoking the full preprocessor. It calls `clang::scanSourceForDependencyDirectives` on the input `file_content`, producing vectors of `Token` and `Directive`. If the scanner fails (returns a non‑zero value), the function returns early, leaving the provided `ScanResult` unchanged. Otherwise, it iterates over each directive. For directives of kind `cxx_export_module_decl` or `cxx_module_decl`, the function extracts the module name by skipping the `export` and `module` keywords and concatenating subsequent non‑whitespace, non‑punctuation tokens until a semicolon. It checks for a global module fragment (e.g., a `:`) and, if the fragment is absent and a module name was found, sets `result.module_name` and `result.is_interface_unit` (true only for `cxx_export_module_decl`). For directives of kind `cxx_import_decl`, the function collects the import name after the `import` keyword, normalizes it using `normalize_partition_import` (which handles partition specifiers), and appends the result to `result.module_imports` if it is not already present. Helper lambdas `is_whitespace_only` and `is_punctuation_only` are used to filter tokens during both module‑name and import‑name extraction.

#### Side Effects

- populates the provided `ScanResult` with module name, interface unit flag, and imports list
- calls `normalize_partition_import` to normalize import names

#### Reads From

- `file_content` (`string_view`)
- `result.module_imports` (when checking for duplicate imports)
- `result.module_name` (passed to `normalize_partition_import`)
- directives and tokens returned by `clang::scanSourceForDependencyDirectives`

#### Writes To

- `result.module_name`
- `result.is_interface_unit`
- `result.module_imports`

#### Usage Patterns

- called by `scan_file` to fill `ScanResult` fields without full preprocessing
- used as a fast module detection step before heavy parsing

### `clore::extract::topological_order`

Declaration: `extract/scan.cppm:66`

Definition: `extract/scan.cppm:495`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first initializes an in-degree map for each file in `graph.files` to zero. It then iterates over every `DependencyEdge` in `graph.edges`, building an adjacency list (`adj`) where each target (`edge.to`) records its dependencies (`edge.from`), and increments the in-degree for each source (`edge.from`). This constructs a directed graph representing file dependencies. Finally, it delegates to `clore::support::topological_order`, passing the full file list, the adjacency map, and the in-degree map. If the delegated function fails to produce a topological ordering (indicating a cycle), the function returns a `ScanError` with a descriptive message; otherwise, it returns the computed order vector.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `graph.files`
- `graph.edges`

#### Usage Patterns

- used to obtain a dependency-resolved sequence of files for compilation or analysis
- topological sort with cycle detection in a build system context

## Internal Structure

The `extract:scan` module is responsible for scanning translation units to extract module declarations, import directives, and file inclusion relationships. It is decomposed into a small public API — `scan_file`, `build_dependency_graph_async`, and `topological_order` — and an internal implementation layer residing in an anonymous namespace. The internal layer contains several helper types and functions: `ScanAction` drives per‑file scanning via Clang’s preprocessor, `ScanPPCallbacks` captures inclusion and import directives, `PreparedScanEntry` normalizes file paths and computes deterministic cache keys, and `MissingScanTask` tracks entries that must be scanned before the full graph can be constructed. This decomposition isolates the asynchronous, callback‑driven scanning logic from the public interface and enables reuse across successive scans.

The module imports `extract:compiler` for compilation database entries and compile signatures, `support` for path normalization, line‑ending handling, and cache key computation, and the C++ standard library for data structures and algorithms. Internally, the implementation is layered: first, raw file content or compilation entries are normalized into `PreparedScanEntry` objects; then, scanning is performed by invoking Clang’s dependency directives scanner (via `scan_module_decl`) or the full preprocessor (via `ScanAction`). Results are stored in a `ScanCache` to avoid redundant work when the compilation database or file system is unchanged. Finally, the collected `ScanResult` objects are assembled into a `DependencyGraph`, and a topological ordering can be computed to guide further processing. This layered design separates the concerns of file preparation, Clang integration, caching, and graph construction, keeping each phase testable and maintainable.

## Related Pages

- [Module extract:compiler](compiler.md)
- [Module support](../support/index.md)

