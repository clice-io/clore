---
title: 'Module extract:compiler'
description: 'The extract:compiler module is responsible for managing and processing compilation-related data during the extraction pipeline. Its core public data structures, CompileEntry and CompilationDatabase, represent an individual translation unit’s compilation parameters and the overall set of entries loaded from a compilation database, respectively. The module owns a set of public functions that load a compilation database (load_compdb), look up entries by file path (lookup), normalize file paths and arguments (normalize_entry_file, normalize_argument_path), sanitize driver and tool arguments (sanitize_driver_arguments, sanitize_tool_arguments), build content-based compile signatures (build_compile_signature), create compiler instances (create_compiler_instance), strip compiler paths (strip_compiler_path), ensure cache keys on entries (ensure_cache_key), and query toolchain information with caching (query_toolchain_cached). Together, these provide a consistent interface for preparing, normalizing, and caching compilation data before further extraction steps.'
layout: doc
template: doc
---

# Module `extract:compiler`

## Summary

The `extract:compiler` module is responsible for managing and processing compilation-related data during the extraction pipeline. Its core public data structures, `CompileEntry` and `CompilationDatabase`, represent an individual translation unit’s compilation parameters and the overall set of entries loaded from a compilation database, respectively. The module owns a set of public functions that load a compilation database (`load_compdb`), look up entries by file path (`lookup`), normalize file paths and arguments (`normalize_entry_file`, `normalize_argument_path`), sanitize driver and tool arguments (`sanitize_driver_arguments`, `sanitize_tool_arguments`), build content-based compile signatures (`build_compile_signature`), create compiler instances (`create_compiler_instance`), strip compiler paths (`strip_compiler_path`), ensure cache keys on entries (`ensure_cache_key`), and query toolchain information with caching (`query_toolchain_cached`). Together, these provide a consistent interface for preparing, normalizing, and caching compilation data before further extraction steps.

Internally, the module implements utility functions for parsing compiler invocations, hashing source files, normalizing entry files, and building cache keys, all of which support the public API. The `CompDbError` struct encapsulates database-related errors, while the `CompilationDatabase` maintains a toolchain cache and a list of entries. By depending on the `support` module for file I/O, path handling, and logging, `extract:compiler` abstracts away the low-level details of compilation database reading and argument normalization, offering a focused public scope for managing compile entries and their associated compiler instances.

## Imports

- `std`
- [`support`](../support/index.md)

## Imported By

- [`extract:ast`](ast.md)
- [`extract:cache`](cache.md)
- [`extract:scan`](scan.md)

## Types

### `clore::extract::CompDbError`

Declaration: `extract/compiler.cppm:38`

Definition: `extract/compiler.cppm:38`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::CompDbError` is implemented as a thin wrapper around a single `std::string` member named `message`. There are no user‑defined constructors, destructors, or assignment `operator`s; the struct relies entirely on the compiler‑generated defaults. The sole invariant is that `message` holds a human‑readable description of the error. Because the implementation does not add any custom resource management or validation logic, its correctness depends entirely on the invariants of `std::string` — namely exception‑safe allocation and copy/move semantics. The struct is trivially copyable and movable via the compiler‑provided operations, making it efficient to pass by value in error‑handling paths.

#### Invariants

- The `message` member holds a valid `std::string` (default-constructed or assigned).

#### Key Members

- `message`: a `std::string` representing the error description.

#### Usage Patterns

- Returned as a result type from extraction operations to indicate failure.
- Inspected by callers to retrieve detailed error text.

### `clore::extract::CompilationDatabase`

Declaration: `extract/compiler.cppm:31`

Definition: `extract/compiler.cppm:31`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::CompilationDatabase` is an internal container that aggregates a flat list of `CompileEntry` objects in `entries` and maintains a `toolchain_cache` mapping toolchain identifiers to their resolved argument sequences. The two fields are default-initialised to empty; `entries` stores all parsed compilation commands while `toolchain_cache` acts as a memoization table for toolchain-related lookups, preventing redundant re‑computation. The member `has_cached_toolchain` simply checks whether the cache is non‑empty, providing an efficient way to test if any toolchain data has been populated without examining the cache contents. This design keeps the database self‑contained and allows quick short‑circuit checks during extraction.

#### Invariants

- entries may be empty
- `toolchain_cache` maps toolchain identifiers to flag lists
- `has_cached_toolchain()` returns true if `toolchain_cache` contains any keys

#### Key Members

- entries
- `toolchain_cache`
- `has_cached_toolchain`

#### Usage Patterns

- Used to store and pass around parsed compilation database data
- `toolchain_cache` can be populated and queried to avoid repeated toolchain resolution

#### Member Functions

##### `clore::extract::CompilationDatabase::has_cached_toolchain`

Declaration: `extract/compiler.cppm:35`

Definition: `extract/compiler.cppm:229`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
auto CompilationDatabase::has_cached_toolchain() const -> bool {
    return !toolchain_cache.empty();
}
```

### `clore::extract::CompileEntry`

Declaration: `extract/compiler.cppm:21`

Definition: `extract/compiler.cppm:21`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::CompileEntry` aggregates the data describing a single compilation unit extracted from a build system. The fundamental raw inputs are `file` (the source file path), `directory` (the working directory), and `arguments` (the compiler command‑line tokens). The `normalized_file` member stores a canonicalized version of the source path, while `compile_signature` is a hash computed from the combination of file, directory, and arguments, serving as an efficient discriminator for duplicate or equivalent compilations. An optional `source_hash` may hold a content hash of the source file; its presence is not guaranteed. The `cache_key` string is a derived, deterministic identifier used for caching or result lookup. An invariant of the structure is that once populated, `normalized_file` and `compile_signature` remain consistent with the raw fields, and `cache_key` is uniquely determined by the compilation signature (and possibly the source hash). The struct is an aggregate with all fields default‑initialized, allowing straightforward brace‑initialization and copying.

#### Invariants

- All string fields may be empty
- `compile_signature` is zero-initialized if not set
- `source_hash` is `std::nullopt` if not available

#### Key Members

- file
- directory
- arguments
- `normalized_file`
- `compile_signature`
- `source_hash`
- `cache_key`

#### Usage Patterns

- Used to store compilation entries from `clore::extract::Extractor`
- Populated from build system output like `compile_commands.json`
- Accessed by caching and reproducibility systems

## Functions

### `clore::extract::build_compile_signature`

Declaration: `extract/compiler.cppm:58`

Definition: `extract/compiler.cppm:110`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first checks whether the `CompileEntry` already holds a non-zero `compile_signature` and a non-empty `normalized_file`; if so, it short‑circuits by returning that cached value. Otherwise, it determines the normalized file path: if `entry.normalized_file` is empty, it calls `clore::extract::normalize_entry_file` to compute it; otherwise it reuses the existing one. It then delegates to the anonymous‑namespace helper `clore::extract::(anonymous namespace)::build_compile_signature_impl`, passing both the entry and the normalized file path, and returns the resulting `std::uint64_t` signature. This design centralizes the actual signature computation in the private helper while providing a caching layer that avoids redundant work when the signature and normalized file have already been established.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- entry`.normalized_file`
- entry`.compile_signature`
- entry (via `normalize_entry_file` and `build_compile_signature_impl`)

#### Usage Patterns

- computing a unique hash for compile entries
- caching compile signatures to avoid redundant computation

### `clore::extract::create_compiler_instance`

Declaration: `extract/compiler.cppm:65`

Definition: `extract/compiler.cppm:297`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first sanitizes the command-line arguments via `sanitize_driver_arguments`; an empty result causes an immediate `nullptr` return. A physical file system is created, and a diagnostic engine is constructed using `clang::CompilerInstance::createDiagnostics` with an ignoring consumer. If diagnostics creation fails, the function returns `nullptr`. The sanitized arguments, together with `entry.file`, the virtual file system, and the diagnostics, are passed to `parse_compiler_invocation` to produce a `clang::CompilerInvocation`. If parsing fails, `nullptr` is returned. The invocation's frontend options are adjusted: `DisableFree` is set to false, and `WorkingDir` is set to `entry.directory`. If the invocation has no frontend inputs, the function returns `nullptr`. Finally, a `clang::CompilerInstance` is constructed from the invocation, its virtual file system is set, diagnostics are recreated for the instance, a file manager is created, and target creation is attempted. Failure at any of these steps yields `nullptr`; otherwise, the fully initialized instance is returned. Key dependencies include `sanitize_driver_arguments`, `parse_compiler_invocation`, `llvm::vfs::FileSystem`, `clang::DiagnosticsEngine`, `clang::CompilerInvocation`, and `clang::CompilerInstance`.

#### Side Effects

- Allocates a `clang::CompilerInstance` and related objects (VFS, diagnostics, invocation).
- Writes to the diagnostics object via `IgnoringDiagConsumer` during invocation parsing.
- Potentially performs I/O when parsing the compiler invocation (e.g., reading source files or configuration).

#### Reads From

- `entry` (the `CompileEntry` parameter), specifically `entry.file` and `entry.directory`
- `driver_args` returned from `sanitize_driver_arguments(entry)`

#### Writes To

- The returned `std::unique_ptr<clang::CompilerInstance>`
- Diagnostics object created internally (written by `parse_compiler_invocation`)

#### Usage Patterns

- Called in extraction pipeline to obtain a Clang compiler instance for a compilation unit.
- Used as part of the process to analyze source code and extract symbol information.

### `clore::extract::ensure_cache_key`

Declaration: `extract/compiler.cppm:60`

Definition: `extract/compiler.cppm:225`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/ensure-cache-key.md)

The function `clore::extract::ensure_cache_key` serves as a thin entry point that delegates entirely to the implementation function `ensure_cache_key_impl`. Its purpose is to separate the public interface from the actual cache-key computation logic, allowing the implementation details—such as argument sanitization, compiler invocation parsing, file normalization, and hash generation—to evolve without affecting callers. The single call to `ensure_cache_key_impl` handles all internal control flow, including conditional checks for cached toolchain data, error handling for missing or invalid compilation databases, and the final assignment of the `CompileEntry::cache_key` field.

#### Side Effects

- modifies the cache key field of the `CompileEntry`

#### Reads From

- the `CompileEntry` object (fields may be read to compute the cache key)

#### Writes To

- the `CompileEntry` object (specifically the cache key field)

#### Usage Patterns

- called by `query_toolchain_cached` before caching or querying toolchain for a compile entry

### `clore::extract::ensure_cache_key_impl`

Declaration: `extract/compiler.cppm:119`

Definition: `extract/compiler.cppm:119`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/ensure-cache-key-impl.md)

The function `clore::extract::ensure_cache_key_impl` computes and stores the caching metadata for a given `CompileEntry`. It begins by obtaining a normalized file path via `clore::extract::normalize_entry_file`, storing the result into the `normalized_file` field. Next, it derives a compile signature by calling `clore::extract::(anonymous namespace)::build_compile_signature_impl` with the entry and the normalized path, saving the result into `compile_signature`. An optional source file hash is produced by `clore::extract::(anonymous namespace)::try_hash_source_file` using the normalized file, and that heuristics result is written to `source_hash`. Finally, the function assembles the `cache_key` by invoking `clore::support::build_cache_key` with the normalized file and the compile signature. The control flow is strictly sequential; each step depends on the output of the previous one, and all key values are later used by the caching and lookup mechanisms.

#### Side Effects

- mutates entry fields: `normalized_file`, `compile_signature`, `source_hash`, `cache_key`
- potentially reads source file via `try_hash_source_file`

#### Reads From

- entry (`CompileEntry`& parameter)

#### Writes To

- entry`.normalized_file`
- entry`.compile_signature`
- entry`.source_hash`
- entry`.cache_key`

#### Usage Patterns

- called by `ensure_cache_key` to populate cache key for a compile entry

### `clore::extract::load_compdb`

Declaration: `extract/compiler.cppm:42`

Definition: `extract/compiler.cppm:127`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::load_compdb` begins by checking that the given `path` exists as a filesystem path; if not, it returns an error via `CompDbError`. It then loads the JSON compilation database using `clang::tooling::JSONCompilationDatabase::loadFromFile`, passing the raw `path` and an `error_message` output parameter. On failure, it returns an error describing the load failure. On success, it iterates over all compile commands from the database. For each command, it constructs a `CompileEntry`, populating `file`, `directory`, and `arguments` from the command’s `Filename`, `Directory`, and `CommandLine` fields. The helper `ensure_cache_key` is called on each entry to compute and store a `cache_key` (which later participates in deduplication and toolchain caching). The entries are moved into `CompilationDatabase::entries`. Finally, the function logs the number of loaded commands and returns the populated `CompilationDatabase`. The algorithm depends on `std::filesystem` for existence checks, `clang::tooling::JSONCompilationDatabase` for parsing, and internal routines such as `ensure_cache_key` for key derivation.

#### Side Effects

- Reads file system to check existence of compilation database file
- Opens and parses JSON compilation database file
- Allocates memory for `CompileEntry` objects
- Calls `ensure_cache_key` for each entry, mutating the entry's internal state
- Logs an informational message with the number of loaded commands

#### Reads From

- File system via the `path` parameter
- Contents of the JSON file at `path`
- Global or local `logging` facility (for `logging::info`)

#### Writes To

- Local `CompilationDatabase` object (`db`) and its `entries` vector
- Each `CompileEntry` within `db` (mutated by `ensure_cache_key`)
- Log output (via `logging::info`)

#### Usage Patterns

- Loading a compilation database from a path to `compile_commands.json`
- Initializing extraction processes for a project
- Providing compile commands to build dependency graphs or symbol indexes

### `clore::extract::lookup`

Declaration: `extract/compiler.cppm:44`

Definition: `extract/compiler.cppm:164`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::lookup` iterates over all entries in the provided `clore::extract::CompilationDatabase` and identifies those whose compiled source file matches the given `file` path. For each entry, it calls `normalize_argument_path` to resolve the input `file` relative to the entry’s `directory`, producing a canonical absolute path. It then compares this candidate against the entry’s normalized file, which is either pre‑computed in `entry.normalized_file` or obtained on‑the‑fly via `normalize_entry_file`. If the two paths are equal (using `std::filesystem::path` equality), a pointer to the entry is appended to the result vector.

The algorithm relies entirely on `normalize_argument_path` and `normalize_entry_file` for path normalization, and on `std::filesystem` for path comparison. The function returns a possibly empty `std::vector<const CompileEntry*>`, allowing callers to handle multiple entries that map to the same source file. No error handling is performed; if no match is found, an empty vector is returned.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- db`.entries`
- file (parameter)
- entry`.directory`
- entry`.normalized_file`

#### Usage Patterns

- Used to find compile entries corresponding to a source file path
- Called during extraction to associate a source file with its build configuration

### `clore::extract::normalize_argument_path`

Declaration: `extract/compiler.cppm:49`

Definition: `extract/compiler.cppm:188`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function constructs a normalized filesystem path from a raw `path` and a `directory`. It first creates a `std::filesystem::path` from `path`, and if it is relative, prepends `directory` by concatenating `std::filesystem::path(directory) / normalized`. It then attempts to convert the result to an absolute path via `std::filesystem::absolute`, capturing any `std::error_code`; on success, it replaces `normalized` with the absolute path. After calling `normalized.lexically_normal()` to remove redundant elements (e.g., `..` or `.`), it tries to produce a canonical, symlink‑resolved path using `std::filesystem::weakly_canonical`. If that operation does not fail, it returns the canonical path; otherwise, it falls back to the lexically‑normalized path. The function relies solely on the C++ standard library’s `std::filesystem` facilities and does not invoke any other `clore::extract` routines.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `path` parameter
- `directory` parameter
- filesystem state (via `std::filesystem::absolute` and `std::filesystem::weakly_canonical`)

#### Usage Patterns

- Normalize compiler argument paths for consistent processing

### `clore::extract::normalize_entry_file`

Declaration: `extract/compiler.cppm:56`

Definition: `extract/compiler.cppm:91`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/normalize-entry-file.md)

The function `clore::extract::normalize_entry_file` takes a `CompileEntry` and returns a normalized file path as a `std::string`. It begins by constructing a `std::filesystem::path` from `entry.file`. If the path is relative, it is prefixed with `entry.directory`. After converting to an absolute path using `fs::absolute` (with error checking), the path is normalized via `lexically_normal()`. A subsequent call to `fs::weakly_canonical` attempts to resolve any symbolic links and produce a canonical path; if this fails (e.g., the file does not exist), the function falls back to the lexically-normalized path. The result is always returned as a generic string (forward slashes).

Internally, the function relies solely on the `std::filesystem` library and the `CompileEntry` fields `file` and `directory`. Error codes from `weakly_canonical` are captured but not propagated—the function degrades gracefully to the non-canonical form. This ensures robust behavior when the input file does not exist on disk or when filesystem resolution fails. The algorithm prioritizes accuracy (canonical form when possible) over strict failure reporting.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `entry.file`
- `entry.directory`

#### Usage Patterns

- Called by `build_compile_signature` to derive a unique signature for a compile entry.
- Called by `ensure_cache_key_impl` to produce a normalized file path for cache key computation.

### `clore::extract::query_toolchain_cached`

Declaration: `extract/compiler.cppm:62`

Definition: `extract/compiler.cppm:233`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first checks if `entry.arguments` is empty and returns an empty vector if so. It then attempts to obtain a cache key from `entry.cache_key`; if that field is empty, it creates a copy of the entry and calls `ensure_cache_key` on the copy to populate its `cache_key` field, then uses that key for lookup. Using the key, it searches `db.toolchain_cache`; if a cached value exists, it is returned immediately. Otherwise, the function calls `sanitize_tool_arguments` on the original entry to produce the toolchain arguments, inserts the result into the cache under the key, and returns the computed vector. This design ensures that each unique compilation entry (as identified by its cache key) triggers the potentially expensive sanitization only once per database session.

#### Side Effects

- Inserts or updates entries in `db.toolchain_cache`

#### Reads From

- `entry.arguments`
- `entry.cache_key`
- `db.toolchain_cache`

#### Writes To

- `db.toolchain_cache`

#### Usage Patterns

- Used to obtain sanitized tool arguments with memoization
- Callers rely on caching to avoid redundant calls to `sanitize_tool_arguments`

### `clore::extract::sanitize_driver_arguments`

Declaration: `extract/compiler.cppm:52`

Definition: `extract/compiler.cppm:207`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::sanitize_driver_arguments` begins by obtaining a copy of the `CompileEntry`’s `arguments` into `adjusted`. It then normalizes the original input file path by calling `normalize_argument_path` with `entry.file` and `entry.directory`. Using `std::erase_if`, it scans every argument in `adjusted`; any non‑empty argument that does not start with a dash (`-`) is further checked by normalizing it relative to `entry.directory`. If the normalized result matches the normalized source path, that argument is removed from the vector. The resulting `adjusted` list, which no longer contains the explicit input file argument, is returned.

The algorithm relies on `normalize_argument_path` (defined in the anonymous namespace) to produce a canonical absolute path from a given path and a working directory. It assumes all relevant arguments are already present in the `CompileEntry::arguments` list and that the input file appears there as a non‑flag argument. No other data structures or global state are modified; the function works purely on local copies and the provided `entry`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `entry.arguments`
- `entry.file`
- `entry.directory`
- `clore::extract::normalize_argument_path`

#### Usage Patterns

- used to remove the source file from compiler arguments
- called before building compile signature or invoking compiler

### `clore::extract::sanitize_tool_arguments`

Declaration: `extract/compiler.cppm:54`

Definition: `extract/compiler.cppm:221`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::sanitize_tool_arguments` is a direct two‑stage pipeline. It first calls `sanitize_driver_arguments`, passing the `const CompileEntry& entry`. That function is responsible for parsing and normalizing the driver‑level arguments stored in the entry’s `arguments` field. The result from `sanitize_driver_arguments` is then immediately forwarded to `strip_compiler_path`, which removes the compiler executable path (the first argument of the invocation) from the argument vector. The final returned value is a `std::vector<std::string>` containing the remaining, sanitized tool arguments. No branching or further transformation occurs inside `sanitize_tool_arguments` itself; all logic is delegated to these two dependency functions.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- entry

#### Usage Patterns

- Used when normalizing compile arguments
- Called during extraction pipeline

### `clore::extract::strip_compiler_path`

Declaration: `extract/compiler.cppm:47`

Definition: `extract/compiler.cppm:181`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::strip_compiler_path` applies a simple truncation algorithm: when the input `std::vector<std::string>` of arguments contains one or fewer elements, it returns an empty vector. Otherwise, it constructs a new vector containing every element except the first by copying from `args.begin() + 1` to `args.end()`. This effectively removes the compiler executable path from a command-line argument list.

The sole control flow is an early return on `args.size() <= 1`. The function depends only on the standard library’s `std::vector`, its `size` and `begin` / `end` iterators, and the iterator-range constructor used for the copy. No external project dependencies are required.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `args` parameter

#### Writes To

- return value (new vector)

#### Usage Patterns

- used to obtain compiler flags without the program name

## Internal Structure

The `extract:compiler` module is the intermediary between raw compilation database entries and the actual Clang compiler invocations used during extraction. It imports only the `std` library and the local `support` module, which supplies foundational utilities for file I/O, path normalization, and cache key management. The module decomposes into three layers: data structures (`CompileEntry`, `CompilationDatabase`, `CompDbError`), public API functions, and private helpers in an anonymous namespace. Public functions like `load_compdb`, `lookup`, and `sanitize_driver_arguments` operate on the database and entries, while private helpers such as `parse_compiler_invocation`, `try_hash_source_file`, and `build_compile_signature_impl` handle path resolution, argument stripping, signature hashing, and compiler instance creation. This layering ensures that caching and normalization logic is encapsulated, leaving the public API stable and focused on extracting ready‑to‑use compiler invocations from concrete compile entries.

## Related Pages

- [Module support](../support/index.md)

