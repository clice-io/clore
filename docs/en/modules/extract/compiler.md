---
title: 'Module extract:compiler'
description: 'The extract:compiler module is responsible for loading, representing, and normalising compilation databases, providing the foundational data types and functions that drive source-code extraction. It owns the public API for reading a compile_commands.json file via load_compdb, returning a CompilationDatabase or a CompDbError. Once a database is available, callers can look up entries by file, normalise argument paths and files, compute deterministic compile signatures for caching, create compiler instances, and manage toolchain caches. Key public structures include CompilationDatabase (holding entries and a toolchain cache), CompileEntry (with fields for file, directory, arguments, normalised file, compile signature, source hash, and cache key), and CompDbError (with a message field).'
layout: doc
template: doc
---

# Module `extract:compiler`

## Summary

The `extract:compiler` module is responsible for loading, representing, and normalising compilation databases, providing the foundational data types and functions that drive source-code extraction. It owns the public API for reading a `compile_commands.json` file via `load_compdb`, returning a `CompilationDatabase` or a `CompDbError`. Once a database is available, callers can look up entries by file, normalise argument paths and files, compute deterministic compile signatures for caching, create compiler instances, and manage toolchain caches. Key public structures include `CompilationDatabase` (holding entries and a toolchain cache), `CompileEntry` (with fields for file, directory, arguments, normalised file, compile signature, source hash, and cache key), and `CompDbError` (with a message field).

The module’s public implementation scope also covers functions that sanitise driver and tool arguments, strip compiler paths, normalise argument paths and entry file references, and build compile signatures. It provides caching support through `ensure_cache_key` and `query_toolchain_cached`, and offers `create_compiler_instance` for constructing a compiler context from a given entry. These public routines together form the entry point and preparatory layer for any further extraction work, ensuring that compile entries are consistent, comparable, and ready for use by downstream processing stages.

## Imports

- [`support`](../support/index.md)

## Imported By

- [`extract:ast`](ast.md)
- [`extract:cache`](cache.md)
- [`extract:scan`](scan.md)

## Types

### `clore::extract::CompDbError`

Declaration: `src/extract/compiler.cppm:54`

Definition: `src/extract/compiler.cppm:54`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::CompDbError` contains a single data member, `message`, of type `std::string`. This internal structure serves as a lightweight error type, storing a human‑readable description of a failure that occurred during compilation database extraction. As a simple aggregate with no invariants beyond those of `std::string`, the struct imposes no additional constraints on its content; the `message` may be empty or contain any valid UTF‑8 text. No custom constructors, assignment `operator`s, or other member functions are defined, so the compiler generates default implementations for those operations.

#### Invariants

- The `message` member always contains a non-empty string when an error occurs
- The struct is trivially constructible and destructible

#### Key Members

- `message`: a `std::string` storing the error description

#### Usage Patterns

- Thrown as an exception in error paths of compilation database extraction
- Returned as part of a `std::expected` or similar error-handling mechanism
- Logged or displayed to the user to indicate what went wrong

### `clore::extract::CompilationDatabase`

Declaration: `src/extract/compiler.cppm:47`

Definition: `src/extract/compiler.cppm:47`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::CompilationDatabase` holds the primary data of the extraction process. Its internal structure consists of two fields: `entries`, a vector of `CompileEntry` objects representing the individual compilation commands, and `toolchain_cache`, an unordered map that associates toolchain keys (strings) with their corresponding argument vectors. An invariant is that `toolchain_cache` is used to memoize resolved toolchain information for reuse across entries, thus avoiding repeated lookups. The member function `has_cached_toolchain` returns a boolean indicating whether any toolchain data has been cached; its implementation typically checks `!toolchain_cache.empty()`.

#### Invariants

- `entries` stores the compilation entries
- `toolchain_cache` maps a key (e.g., compiler path) to a list of command-line arguments
- `has_cached_toolchain` reflects the presence of cache entries

#### Key Members

- `entries`
- `toolchain_cache`
- `has_cached_toolchain`

#### Usage Patterns

- Used to represent a compilation database with associated toolchain caching
- Other code populates `entries` and may query `has_cached_toolchain` before accessing the cache

#### Member Functions

##### `clore::extract::CompilationDatabase::has_cached_toolchain`

Declaration: `src/extract/compiler.cppm:51`

Definition: `src/extract/compiler.cppm:245`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
auto CompilationDatabase::has_cached_toolchain() const -> bool {
    return !toolchain_cache.empty();
}
```

### `clore::extract::CompileEntry`

Declaration: `src/extract/compiler.cppm:37`

Definition: `src/extract/compiler.cppm:37`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::CompileEntry` is a plain data aggregate that holds all information required to represent a single compilation command after extraction from a build system. Its members include the original `file` path, the working `directory`, and the full `arguments` vector. The `normalized_file` member stores a canonical form of the file path, and `compile_signature` is a computed hash covering the command’s key components (used for deduplication). The optional `source_hash` may later hold a content hash of the source file, while `cache_key` provides a string key for caching lookups.

The struct enforces no invariants of its own; instead, it is filled incrementally during the extraction pipeline. Important internal invariants are maintained by the code that populates it: `compile_signature` must be computed before it is read, `cache_key` is derived from `compile_signature` and other fields, and `normalized_file` is always expected to be set whenever `file` is present. The optional `source_hash` is only populated when content-based hashing is enabled.

#### Invariants

- Each entry corresponds to exactly one compiler invocation.
- `file` and `directory` are expected to be non-empty for a valid entry.
- `compile_signature` is derived from the command-line arguments and file paths for deduplication.
- `cache_key` is computed from `normalized_file`, `compile_signature`, and optionally `source_hash`.

#### Key Members

- `file`
- `directory`
- `arguments`
- `normalized_file`
- `compile_signature`
- `source_hash`
- `cache_key`

#### Usage Patterns

- Populated by the compilation database extractor from JSON compile commands.
- Used as input to caching or deduplication logic via `compile_signature` and `cache_key`.
- `source_hash` is optionally computed during caching to detect source file changes.
- Stored in a container for export or analysis.

## Functions

### `clore::extract::build_compile_signature`

Declaration: `src/extract/compiler.cppm:74`

Definition: `src/extract/compiler.cppm:126`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function implements a two‑phase computation with an early‑exit caching strategy. It first checks whether the input `CompileEntry` already holds a non‑zero `compile_signature` and a non‑empty `normalized_file`; if both conditions are satisfied, the stored `compile_signature` is returned immediately, avoiding redundant work. When the cache is absent, it obtains the canonical source‑file path by calling `clore::extract::normalize_entry_file` (unless the entry already provides a `normalized_file`), then delegates the actual hashing logic to the internal helper `clore::extract::(anonymous namespace)::build_compile_signature_impl`.

The control flow is thus a guard followed by a single delegation: the only substantial dependency is `normalize_entry_file` for path resolution, and the heavy lifting is done by `build_compile_signature_impl`, which is responsible for parsing the compiler invocation, processing arguments, and computing the final `uint64_t` hash. This design keeps the public function lightweight and cachable while isolating the complex, algorithmic core in a private implementation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- entry`.normalized_file`
- entry`.compile_signature`

#### Usage Patterns

- called during compilation database processing to generate unique signatures for compile entries
- used for caching and deduplication of compile commands

### `clore::extract::create_compiler_instance`

Declaration: `src/extract/compiler.cppm:81`

Definition: `src/extract/compiler.cppm:313`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function begins by calling `sanitize_driver_arguments` on the input `CompileEntry`; if the returned arguments handle is empty, it immediately returns `nullptr`. It then sets up a physical `llvm::vfs::FileSystem` and creates a `clang::DiagnosticsEngine` backed by an ignoring consumer. Using these, it invokes the internal helper `parse_compiler_invocation` with the sanitized arguments handle, the entry’s file path, the VFS, and the diagnostics engine. Failure at this stage also yields `nullptr`.

Upon obtaining a valid `clang::CompilerInvocation`, the function adjusts its frontend options (`DisableFree` set to `false` and `WorkingDir` set to `entry.directory`) and checks that the invocation’s input list is non-empty. It then constructs a `clang::CompilerInstance`, attaches the VFS, creates a fresh ignoring diagnostics consumer, and initialises the file manager. Finally, it attempts to create the target; if this step fails, the function returns `nullptr`, otherwise it returns the fully initialised compiler instance. The entire flow is a linear sequence of guarded steps, each depending on the success of the previous one, with early returns on any error.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `entry` parameter (specifically `entry.file` and `entry.directory`)
- `sanitize_driver_arguments` (internal, reads `entry`)
- `parse_compiler_invocation` (internal, reads sanitized arguments and file)

#### Writes To

- returns a `std::unique_ptr<clang::CompilerInstance>` (heap-allocated object)

#### Usage Patterns

- Used to create a Clang compiler instance for further analysis or extraction.
- Typically called with a compile entry from a `CompilationDatabase`.
- Returned instance is passed to other functions like `extract_symbols` or `scan_file`.

### `clore::extract::ensure_cache_key`

Declaration: `src/extract/compiler.cppm:76`

Definition: `src/extract/compiler.cppm:241`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/ensure-cache-key.md)

The function delegates directly to `ensure_cache_key_impl`, which performs the core algorithm for populating `CompileEntry::cache_key` and `CompileEntry::source_hash`. The implementation normalizes the entry’s file path via `clore::extract::normalize_entry_file` and builds a compile signature using `clore::extract::build_compile_signature`, also querying the toolchain cache through `clore::extract::query_toolchain_cached` when available. File hashing is attempted through `clore::extract::(anonymous namespace)::try_hash_source_file`, and argument paths are normalized with `clore::extract::normalize_argument_path`. The computed key and hash are then stored directly into the entry’s fields, ensuring that subsequent operations can rely on a cached, deterministic identifier for the compilation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- entry (`CompileEntry`&)

#### Usage Patterns

- Called by `query_toolchain_cached` to ensure a cache key is set for a compile entry before use.

### `clore::extract::ensure_cache_key_impl`

Declaration: `src/extract/compiler.cppm:135`

Definition: `src/extract/compiler.cppm:135`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/ensure-cache-key-impl.md)

The function `clore::extract::ensure_cache_key_impl` computes and stores four cache-related fields into a `CompileEntry`. It first normalizes the entry’s file path via `clore::extract::normalize_entry_file` and writes the result into `entry.normalized_file`. Using that normalized path, it calls `clore::extract::(anonymous namespace)::build_compile_signature_impl` to produce a `compile_signature`. An optional source hash is computed by `clore::extract::(anonymous namespace)::try_hash_source_file` and stored in `entry.source_hash`. Finally, it assembles the final `entry.cache_key` by calling `clore::support::build_cache_key` with the normalized file and the compile signature.

The algorithm is a serial data‑preparation process: each step depends on the results of the previous one. No branching or error handling is performed within this function; it relies on its callees to handle failures (e.g., `try_hash_source_file` may return `std::nullopt`, which is stored directly). The primary dependencies are the `normalize_entry_file` and `build_compile_signature_impl` functions, which respectively resolve the entry’s file to an absolute path and derive a hash from the compiler invocation arguments.

#### Side Effects

- Mutates `entry.normalized_file`
- Mutates `entry.compile_signature`
- Mutates `entry.source_hash`
- Mutates `entry.cache_key`
- May perform file I/O via `try_hash_source_file` to compute source hash

#### Reads From

- `entry` parameter (reads fields used by `normalize_entry_file` and `build_compile_signature_impl`)
- The source file referenced by `entry.normalized_file` (read by `try_hash_source_file`)

#### Writes To

- `entry.normalized_file`
- `entry.compile_signature`
- `entry.source_hash`
- `entry.cache_key`

#### Usage Patterns

- Called by `clore::extract::ensure_cache_key` to ensure a compile entry's cache key is computed

### `clore::extract::load_compdb`

Declaration: `src/extract/compiler.cppm:58`

Definition: `src/extract/compiler.cppm:143`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::load_compdb` first checks whether the file at the given `path` exists; if not, it returns a `CompDbError` with a descriptive message. It then uses `clang::tooling::JSONCompilationDatabase::loadFromFile` to parse the JSON database, passing a local `error_message` string and the `AutoDetect` command‑line syntax. If parsing fails, an error `CompDbError` is returned. On success, it iterates over all compile commands obtained via `json_db->getAllCompileCommands()`. For each command, it creates a `CompileEntry` by copying the `Filename` into `entry.file`, the `Directory` into `entry.directory`, and the entire `CommandLine` vector into `entry.arguments`. Each entry is then passed to `ensure_cache_key` (which computes the `cache_key` field) before being moved into `db.entries`. Finally, the populated `CompilationDatabase` is returned.

#### Side Effects

- file I/O: reads the `compile_commands``.json` file at `path`
- logging: records the number of loaded compile commands via `logging::info`

#### Reads From

- parameter `path`
- file system state to check existence of `path`
- contents of the JSON file at `path`

#### Writes To

- local variable `db` which is returned (populated with entries)
- local variable `error_message` (temporary, used for error reporting)
- log output

#### Usage Patterns

- called to load a compilation database from a JSON file for further processing
- used in extraction pipeline to obtain compile commands for symbol extraction
- typically invoked early in the extraction workflow

### `clore::extract::lookup`

Declaration: `src/extract/compiler.cppm:60`

Definition: `src/extract/compiler.cppm:180`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::lookup` iterates over every entry in `db.entries` of the provided `CompilationDatabase`. For each `CompileEntry`, it normalizes the input `file` relative to that entry’s `directory` by calling `normalize_argument_path`. The entry’s own file path is resolved from `entry.normalized_file` if already computed, otherwise via `normalize_entry_file`. Both paths are compared as `std::filesystem::path` objects; when they match, a pointer to the entry is appended to a `results` vector. The function returns the collected vector of matching entries.

The control flow is a straightforward linear scan over the database entries, relying on the helper functions `normalize_argument_path` and `normalize_entry_file` to produce canonical paths for comparison. No fallback ordering or early termination is applied—every entry is evaluated, so the result contains all entries whose normalized file matches the normalized input path.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `db.entries`
- `file` parameter
- `entry.directory`
- `entry.normalized_file`

#### Usage Patterns

- look up compile entries for a given source file
- used to find compilation commands matching a file

### `clore::extract::normalize_argument_path`

Declaration: `src/extract/compiler.cppm:65`

Definition: `src/extract/compiler.cppm:204`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/normalize-argument-path.md)

The function constructs a normalized `std::filesystem::path` from the `path` argument using a multi-stage resolution. First, if `path` is relative, it is made absolute by joining it with the `directory` argument. An `std::error_code`‑based call to `std::filesystem::absolute` then forces an absolute representation, replacing the intermediate result on success. Lexical normalization via `lexically_normal()` cleans up redundant separators and `..` components. Finally, `std::filesystem::weakly_canonical` is attempted to resolve symbolic links and produce a true canonical path; if that call fails (e.g., due to missing components), the function falls back to the lexically‑normalized path. The entire resolution depends on `std::filesystem` operations and `std::error_code` for error‑safe branching, ensuring that even when symlink resolution is impossible a usable path is returned.

#### Side Effects

- accesses filesystem state via `std::filesystem::absolute`
- accesses filesystem state via `std::filesystem::weakly_canonical`

#### Reads From

- filesystem state for path resolution
- parameters `path` and `directory`

#### Usage Patterns

- Called from `clore::extract::sanitize_driver_arguments` to normalize file arguments

### `clore::extract::normalize_entry_file`

Declaration: `src/extract/compiler.cppm:72`

Definition: `src/extract/compiler.cppm:107`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/normalize-entry-file.md)

The function first constructs a `std::filesystem::path` from `entry.file`. If the path is relative, it is joined with `entry.directory` to form an absolute path. The function then attempts to resolve the path using `fs::absolute`, capturing any `std::error_code`; if the operation succeeds, the absolute path replaces the intermediate value. Next, `fs::weakly_canonical` is tried—on success the result is returned as a generic string, providing a minimal, existing-path-normalized form. If `weakly_canonical` fails (e.g., the path does not exist on the filesystem), the function falls back to `path.lexically_normal()` to remove `.` and `..` components without checking existence, and returns that generic string. The implementation relies entirely on `std::filesystem` primitives and the `CompileEntry` fields `file` and `directory`; no other project-specific functions or types are called directly.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `entry.file`
- `entry.directory`

#### Usage Patterns

- normalize file path for compile entries
- used in `build_compile_signature`
- used in `ensure_cache_key_impl`

### `clore::extract::query_toolchain_cached`

Declaration: `src/extract/compiler.cppm:78`

Definition: `src/extract/compiler.cppm:249`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first short-circuits by returning an empty vector if the entry's `arguments` field is empty. It then obtains a cache key from `entry.cache_key`; if the key is empty, it creates a copy of the entry and calls `ensure_cache_key` to populate a valid key, taking ownership of that key. With the key in hand, it performs a lookup in the `db.toolchain_cache` map. If a cached result exists, it is returned directly. Otherwise, the function computes the sanitized tool arguments via `sanitize_tool_arguments` on the original entry, stores the result in the cache under the computed key, and returns that result.

#### Side Effects

- Inserts or assigns a new entry into `db.toolchain_cache`

#### Reads From

- `entry.arguments`
- `entry.cache_key`
- `db.toolchain_cache` (lookup)

#### Writes To

- `db.toolchain_cache` (`insert_or_assign`)

#### Usage Patterns

- Called to retrieve or compute sanitized tool arguments with caching
- Used internally by toolchain processing functions

### `clore::extract::sanitize_driver_arguments`

Declaration: `src/extract/compiler.cppm:68`

Definition: `src/extract/compiler.cppm:223`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first creates a mutable copy of the input compile entry's argument list. It normalizes the entry's file path relative to its directory using `clore::extract::normalize_argument_path` to produce a canonical source path. It then applies `std::erase_if` to remove any argument that, after normalization, matches that source path, provided the argument is non-empty and does not begin with `-`. This ensures that explicit source file references in the driver command line are stripped, leaving only compiler options for further processing. The result is the sanitized argument vector.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- entry`.arguments`
- entry`.file`
- entry`.directory`

#### Usage Patterns

- Used to clean compilation arguments before processing

### `clore::extract::sanitize_tool_arguments`

Declaration: `src/extract/compiler.cppm:70`

Definition: `src/extract/compiler.cppm:237`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::sanitize_tool_arguments` is a straightforward composition of two lower-level utilities. It first invokes `sanitize_driver_arguments` on the provided `CompileEntry`, which applies driver-level argument normalization and filtering, then passes the resulting argument list through `strip_compiler_path` to remove the compiler executable path that is typically the first element. The function thus produces a cleaned, ready-to-use list of compiler flags without any leading tool path, relying entirely on the internal logic of those two dependencies for the actual sanitization and stripping. No branching or error handling is performed at this level; the control flow is purely sequential.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- const `CompileEntry`& entry

#### Usage Patterns

- Used as part of tool argument sanitization pipeline in compilation database processing

### `clore::extract::strip_compiler_path`

Declaration: `src/extract/compiler.cppm:63`

Definition: `src/extract/compiler.cppm:197`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first checks whether the input `args` container holds one or fewer elements. If the condition `args.size() <= 1` is true, it returns an empty `std::vector<std::string>`; otherwise it constructs and returns a new vector containing all elements except the first, effectively removing the compiler executable path. The implementation relies solely on standard library utilities (`std::vector` and its iterator constructors) and has no external dependencies beyond the C++ standard library. No special control flow beyond the single branch is required.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `args` (const `std::vector<std::string>`&)

#### Writes To

- returned vector of strings (allocated and filled with elements from `args` excluding the first)

#### Usage Patterns

- Stripping compiler executable path from compilation command entries in a compilation database

## Internal Structure

The `extract:compiler` module is responsible for loading, parsing, and normalizing compilation database entries, as well as constructing compiler instances from them. It depends on the `support` module for foundational utilities (string handling, caching, logging). The public API is decomposed into several coherent groups: database loading (`load_compdb`), entry normalization (`normalize_entry_file`, `normalize_argument_path`, `sanitize_driver_arguments`, `sanitize_tool_arguments`), signature computation (`build_compile_signature`, `ensure_cache_key`, `ensure_cache_key_impl`), compiler instance creation (`create_compiler_instance`), and cache‑aware toolchain retrieval (`query_toolchain_cached`). Supporting types (`CompileEntry`, `CompilationDatabase`, `CompDbError`) encapsulate the data and caching state.

Internally, the module uses anonymous‑namespace helper functions (`parse_compiler_invocation`, `try_hash_source_file`, `build_compile_signature_impl`) to isolate implementation details such as argument parsing, file hashing, and Clang invocation construction. The layering follows a pipeline: load JSON database → parse entries → normalize each entry (paths, signatures, keys) → optionally create a compiler instance for extraction. Error handling returns `std::expected` with `CompDbError` for database failures and integer status codes for per‑entry operations, allowing callers to recover or skip problematic entries without aborting the entire pipeline.

## Related Pages

- [Module support](../support/index.md)

