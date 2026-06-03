---
title: 'Module extract:scan'
description: 'The extract:scan module is responsible for scanning C++ source files to extract module‑related metadata, include directives, and inter‑file dependencies. It provides the core scanning operations: a fast module‑declaration pass via scan_module_decl that reads source text directly, and a full file scan via scan_file that returns a ScanResult (containing module name, interface flag, imports, and includes) or a ScanError. For building dependency graphs across many translation units, the module offers build_dependency_graph_async (which uses an optional ScanCache for performance across repeated runs) and topological_order to compute a valid ordering of the resulting DependencyGraph. Public data structures include ScanResult, ScanError, ScanCache, DependencyGraph, DependencyEdge, and IncludeInfo. The module depends on extract:compiler for compilation database interactions and on support for utility functions.'
layout: doc
template: doc
---

# Module `extract:scan`

## Summary

The `extract:scan` module is responsible for scanning C++ source files to extract module‑related metadata, include directives, and inter‑file dependencies. It provides the core scanning operations: a fast module‑declaration pass via `scan_module_decl` that reads source text directly, and a full file scan via `scan_file` that returns a `ScanResult` (containing module name, interface flag, imports, and includes) or a `ScanError`. For building dependency graphs across many translation units, the module offers `build_dependency_graph_async` (which uses an optional `ScanCache` for performance across repeated runs) and `topological_order` to compute a valid ordering of the resulting `DependencyGraph`. Public data structures include `ScanResult`, `ScanError`, `ScanCache`, `DependencyGraph`, `DependencyEdge`, and `IncludeInfo`. The module depends on `extract:compiler` for compilation database interactions and on `support` for utility functions.

## Imports

- [`extract:compiler`](compiler.md)
- [`support`](../support/index.md)

## Imported By

- [`extract:cache`](cache.md)

## Types

### `clore::extract::DependencyEdge`

Declaration: `src/extract/scan.cppm:69`

Definition: `src/extract/scan.cppm:69`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The `clore::extract::DependencyEdge` struct is a straightforward data holder representing a directed relationship between two nodes. It contains two `std::string` members, `from` and `to`, which store the identifiers of the source and target nodes respectively. No invariants are enforced beyond the natural properties of strings; the struct serves purely to bundle these two identifiers together for use in dependency graph construction or traversal within the extraction pipeline.

#### Invariants

- No explicit invariants
- Fields are user-supplied strings with no constraints on content

#### Key Members

- `from`
- `to`

#### Usage Patterns

- Used to represent a direct dependency relationship in extraction analysis
- Likely stored in a container as part of a larger dependency graph structure

### `clore::extract::DependencyGraph`

Declaration: `src/extract/scan.cppm:74`

Definition: `src/extract/scan.cppm:74`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::DependencyGraph` maintains a dependency graph as two separate vectors: a `std::vector<std::string>` named `files` holding the node set, and a `std::vector<DependencyEdge>` named `edges` holding the directed relationships between nodes. No other fields are present, so the graph’s entire state is captured by the ordered pairing of file paths and edge objects.

An implicit invariant is that every `DependencyEdge` should reference valid indices into the `files` vector (or otherwise identify files in a consistent manner). The default member initializers ensure both vectors are empty upon construction; important member implementations, such as adding a node or an edge, must keep the two collections synchronized to preserve the invariant that each edge refers to an existing file entry.

#### Invariants

- `files` and `edges` are maintained in a consistent state
- Indices in `edges` are valid with respect to the size of `files`

#### Key Members

- `files` vector stores the list of file paths
- `edges` vector stores the dependency relationships

#### Usage Patterns

- Populated during the dependency scanning phase
- Consumed by downstream processing or serialization

### `clore::extract::IncludeInfo`

Declaration: `src/extract/scan.cppm:42`

Definition: `src/extract/scan.cppm:42`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::IncludeInfo` is a lightweight aggregate that captures the parsed components of a preprocessor `#include` directive. Its internal layout consists of exactly two fields: `path`, a `std::string` default‑initialised to an empty string, and `is_angled`, a `bool` default‑initialised to `false`. The default values establish a well‑defined empty state, ensuring that any default‑constructed instance represents an invalid or uninitialised include directive. The invariant maintained by the struct is simple: `path` holds the file name or path as it appears in the directive, and `is_angled` distinguishes between angle‑bracket includes (e.g. `<file>`) and quoted includes (e.g. `"file"`). As a plain‑old‑data type with no custom constructors, destructors, or special member functions, `clore::extract::IncludeInfo` relies on compiler‑generated defaults and is intended to be copied, moved, and assigned trivially.

#### Invariants

- `is_angled` is true iff the include was written with angle brackets.
- path may be empty if not set.

#### Key Members

- path
- `is_angled`

#### Usage Patterns

- Used by the scanner to represent parsed include directives.
- Likely consumed by the extraction system to process includes.

### `clore::extract::ScanCache`

Declaration: `src/extract/scan.cppm:58`

Definition: `src/extract/scan.cppm:58`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The class `clore::extract::ScanCache` is implemented as a thin wrapper around a single `std::unordered_map<std::string, ScanResult>` member named `scan_results`. The map serves as a persistent store that associates a file path (the key) with its previously computed dependency scan result. The core invariant is that the cache remains valid only as long as the compilation database and file system state are unchanged; external callers are responsible for clearing or discarding the entire cache when either of these conditions is violated. No other state or complex logic exists within the struct — all cached data is held directly in `scan_results`, which provides amortized constant‑time lookups and insertions for file‑to‑result mappings.

#### Invariants

- Cache is valid only as long as the compilation database and file system state remain unchanged.
- Callers must clear or discard the cache after any relevant state change.

#### Key Members

- `scan_results`: the underlying unordered map storing cached `ScanResult` objects.

#### Usage Patterns

- Shared across successive dependency scans to avoid recomputation.
- Cleared or replaced by callers when the compilation DB or file system state changes.

### `clore::extract::ScanError`

Declaration: `src/extract/scan.cppm:38`

Definition: `src/extract/scan.cppm:38`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::ScanError` is implemented as a simple aggregate type with a single data member: `message`, of type `std::string`. No custom constructors, destructors, or assignment `operator`s are defined; the compiler generates all special member functions, making the type trivially copyable and movable. The sole invariant is that `message` contains an arbitrary string, which may be empty. Because the struct has no other internal state or validation logic, its implementation is minimal and relies entirely on the `std::string` member for storage and lifetime management.

#### Invariants

- The `message` member may be empty or non-empty; no constraints are enforced.

#### Key Members

- `std::string message` - stores the error description.

#### Usage Patterns

- Returned or thrown by scanning functions to indicate an error.
- Used in `clore::extract` operations to communicate failure details.

### `clore::extract::ScanResult`

Declaration: `src/extract/scan.cppm:47`

Definition: `src/extract/scan.cppm:47`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The `clore::extract::ScanResult` struct is a plain aggregate that captures the outcome of scanning a C++ source file. Its fields separate the module identity (`module_name`, `is_interface_unit`) from the dependency lists (`includes` and `module_imports`). The invariant is that `module_name` is the detected module declaration, empty if no module was named; `is_interface_unit` indicates whether the file is a module interface unit (versus a partition or implementation unit). The two vectors store resolved include paths (as `IncludeInfo`) and imported module names, respectively. No additional constructors or member functions exist—initialization relies on default member initializers, with `module_name` defaulting to an empty string and `is_interface_unit` to `false`.

#### Invariants

- No explicit invariants are documented or implied by the struct definition.

#### Key Members

- `module_name`
- `is_interface_unit`
- `includes`
- `module_imports`

#### Usage Patterns

- Used to aggregate and pass around scan results from module extraction.
- Consumed by downstream processing to query module name, imports, includes, and interface unit status.

## Functions

### `clore::extract::build_dependency_graph_async`

Declaration: `src/extract/scan.cppm:79`

Definition: `src/extract/scan.cppm:388`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::build_dependency_graph_async` begins by clearing `graph.files` and `graph.edges`, then iterates over entries from the `CompilationDatabase` to build a list of `PreparedScanEntry` objects via `prepare_scan_entry`. Each normalized file is added to a global `file_set` and appended to `graph.files` if novel. A cache lookup against `ScanCache::scan_results` (if provided) determines which entries have already been scanned; those without a cached result are queued as `MissingScanTask` objects, deduplicated by `cache_key`. All missing tasks are launched concurrently using `kota::when_all` over a vector of tasks returned by `run_scan_task`. If the batch scan fails or produces a size mismatch, a `ScanError` is propagated via `kota::fail`.

After scanning completes (or cached results are retrieved), the function processes each database entry in order. For each entry, it retrieves the corresponding `ScanResult` (either from `cached_results` or `scanned_results`), and for each `IncludeInfo` in `scan_result->includes`, it normalizes the include path via `normalize_argument_path`. If the normalized path belongs to an entry file (`entry_files`) and the edge has not already been emitted (tracked by `emitted_edges`), a `DependencyEdge` is appended to `graph.edges` with `.from` and `.to` set to the normalized file names. The `seen_files` set ensures each file contributes its includes only once. Finally, if a `ScanCache` was provided, each entry’s `scan_result` is inserted or assigned into the cache by `cache_key`. The function returns a `kota::task<void, ScanError>` that completes when all asynchronous dependencies have been resolved.

#### Side Effects

- Clears and repopulates the `DependencyGraph` (files and edges edges).
- Inserts or updates entries in the `ScanCache` `scan_results` map when cache pointer is non-null.

#### Reads From

- `CompilationDatabase db` (entries, their fields)
- `ScanCache* cache` (to look up cached scan results)
- `kota::event_loop &loop` (for scheduling async tasks)
- `PreparedScanEntry` fields (`normalized_file`, `cache_key`)
- `ScanResult` structs (includes list)

#### Writes To

- `DependencyGraph &graph` (files and edges members)
- `ScanCache scan_results` (if cache is provided)

#### Usage Patterns

- Called to build a dependency graph from a compilation database asynchronously.
- Part of the project extraction pipeline to capture file dependencies.
- Typically invoked once per project analysis session.

### `clore::extract::scan_file`

Declaration: `src/extract/scan.cppm:62`

Definition: `src/extract/scan.cppm:256`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function begins by creating an empty `ScanResult` and performing a fast preliminary scan of the source file’s text content. It normalizes the file path (via `normalize_entry_file` if `entry.normalized_file` is empty), reads the entire file with `std::ifstream`, and passes the raw text to `scan_module_decl`. That call extracts module and partition declarations without a full preprocessor run, populating fields such as `result.module_name` and `result.is_interface_unit`.

After the text‑based scan, the function constructs a Clang compiler instance from the compile entry and configures its frontend options to run only the preprocessor (`clang::frontend::RunPreprocessorOnly`). It then wraps `result` inside a `ScanAction` object (whose callbacks record include directives and module import directives), begins the source file, and executes the preprocessor pass. If any step fails—empty argument list, file read failure, missing compiler instance, or preprocessor error—the function returns a `ScanError` via `std::unexpected`. On success it ends the source file and returns the populated `result`. The two‑phase approach allows the function to capture module declarations quickly from disk, and then rely on Clang’s preprocessor for a detailed extraction of includes and imports.

#### Side Effects

- reads source file from filesystem
- creates a Clang compiler instance (may involve toolchain querying)
- runs preprocessor-only scan (may read additional include files)

#### Reads From

- entry`.arguments`
- entry`.file`
- entry`.normalized_file`
- source file content on disk

#### Writes To

- local variable `result` (returned as `ScanResult`)

#### Usage Patterns

- called for each file in a compilation database to build project model
- used in extraction pipeline

### `clore::extract::scan_module_decl`

Declaration: `src/extract/scan.cppm:67`

Definition: `src/extract/scan.cppm:159`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/scan-module-decl.md)

The function `clore::extract::scan_module_decl` leverages Clang's low-level dependency directives scanner via `clang::scanSourceForDependencyDirectives` to parse the raw file content into a sequence of tokens and directives without invoking the full preprocessor. It iterates over the resulting directive list, handling two directive kinds: module declarations (`cxx_export_module_decl` and `cxx_module_decl`) and import declarations (`cxx_import_decl`). For module declarations, it skips the `export` and `module` tokens, then collects subsequent non‑whitespace tokens into the module name, stopping at a semicolon or early‑fragment indicator (e.g., `";"` or `":"`); it sets `result.module_name` and `result.is_interface_unit` accordingly. For import declarations, it extracts the import name after the `import` keyword, normalizes it via `normalize_partition_import` using the already‑established module name, and appends it to `result.module_imports` if not already present. The entire scan is fast because it operates on a pre‑tokenized stream and does no semantic analysis.

#### Side Effects

- Modifies `result.module_name` by assigning the detected module name
- Modifies `result.is_interface_unit` by setting it to true for export declarations
- Appends unique import names to `result.module_imports`

#### Reads From

- `file_content` (`std::string_view` parameter)
- `result.module_name` and `result.module_imports` for duplicate checking
- Output of `clang::scanSourceForDependencyDirectives`

#### Writes To

- `result.module_name`
- `result.is_interface_unit`
- `result.module_imports`

#### Usage Patterns

- Called by `scan_file` to quickly obtain module metadata
- Used in the extraction pipeline for module scanning without full preprocessing

### `clore::extract::topological_order`

Declaration: `src/extract/scan.cppm:84`

Definition: `src/extract/scan.cppm:513`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first constructs an internal adjacency map and an in-degree map from the provided `DependencyGraph`. For each file listed in `graph.files`, the in-degree is initialised to zero. It then iterates over every `DependencyEdge` in `graph.edges` and populates the adjacency map with edges directed from `edge.to` to `edge.from`, while incrementing the in-degree of `edge.from`. This reversed representation encodes the dependency direction, so that a topological ordering places each file after its dependents. The actual ordering is delegated to `clore::support::topological_order`, which receives the complete set of files, the adjacency map, and the in-degree map. If that helper detects a cycle and returns no value, the function returns an error of type `ScanError` with a message indicating a dependency cycle. Otherwise, it returns the successfully computed order as a vector of file path strings.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- graph`.files`
- graph`.edges`

#### Usage Patterns

- Obtain a valid processing order of files after building the dependency graph
- Detect cycles in the include graph

## Internal Structure

The `extract:scan` module is responsible for discovering module declarations, imports, includes, and file-level dependencies from C++ source files. The module is decomposed around a few core functions and supporting internal types. The main public entry point `scan_file` performs a full preprocessor scan of a single compilation unit and returns a `ScanResult` containing the module name, interface‑unit flag, imports, and include directives. A lighter‑weight function `scan_module_decl` extracts only module metadata (name, interface flag, import list) using Clang’s dependency directives scanner, without running the full preprocessor. An asynchronous graph builder `build_dependency_graph_async` constructs a `DependencyGraph` by scanning a set of entry files, caching results in a `ScanCache` to avoid redundant work, and then optionally computes a topological ordering via `topological_order`.

Internally, the scanning pipeline is layered into preparation, caching, and execution. The `prepare_scan_entry` helper normalizes a file path and computes a cache key from the compile signature, producing a `PreparedScanEntry`. A `ScanAction` class encapsulates a single scan operation, using a `ScanPPCallbacks` class that intercepts Clang’s `InclusionDirective` callback to record include and import directives into the `ScanResult`. Missing scans are represented as `MissingScanTask` objects and processed by `run_scan_task`. The module imports `extract:compiler` for compilation‑database types and `support` for string utilities and caching primitives, forming a clean layering where scan logic depends only on these lower‑level modules.

## Related Pages

- [Module extract:compiler](compiler.md)
- [Module support](../support/index.md)

