---
title: 'Module extract:compiler'
description: 'The extract:compiler module is responsible for loading, representing, and manipulating compilation databases and their entries, which form the primary input to Clore’s extraction pipeline. It owns the core data types (CompileEntry, CompilationDatabase, CompDbError) and provides the public interface for reading compilation databases from disk, normalizing source file paths and command-line arguments, computing deterministic compilation signatures, and constructing validated Clang compiler invocations. The module also handles argument sanitization and caching of toolchain configurations, ensuring that downstream extraction steps operate on a consistent, reproducible representation of each translation unit’s build context.'
layout: doc
template: doc
---

# Module `extract:compiler`

## Summary

The `extract:compiler` module is responsible for loading, representing, and manipulating compilation databases and their entries, which form the primary input to Clore’s extraction pipeline. It owns the core data types (`CompileEntry`, `CompilationDatabase`, `CompDbError`) and provides the public interface for reading compilation databases from disk, normalizing source file paths and command-line arguments, computing deterministic compilation signatures, and constructing validated Clang compiler invocations. The module also handles argument sanitization and caching of toolchain configurations, ensuring that downstream extraction steps operate on a consistent, reproducible representation of each translation unit’s build context.

The public-facing implementation scope includes functions such as `load_compdb`, `lookup`, `normalize_entry_file`, `normalize_argument_path`, `sanitize_driver_arguments`, `sanitize_tool_arguments`, `strip_compiler_path`, `ensure_cache_key`, `build_compile_signature`, `create_compiler_instance`, and `query_toolchain_cached`. These functions together offer a complete API for parsing and preparing compilation entries, from raw database loading through to the creation of a fully resolved compiler invocation ready for use by the extraction backend.

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

The struct `clore::extract::CompDbError` is a minimal error type comprising a single data member `message` of type `std::string`. No other fields, constructors, destructors, or member functions are defined, making the type a trivial aggregator of an error description. The only implicit invariant is that the `message` member stores the textual explanation of the error; the struct provides no validation or ownership semantics beyond those of `std::string`. All member implementations are compiler-generated, and the type serves purely as a lightweight container for error information within the extraction layer.

#### Invariants

- The `message` member always contains a textual description of the error.
- No additional error codes or other data are stored.

#### Key Members

- `message` – the error description string

#### Usage Patterns

- Returned or thrown to indicate errors in compiler database extraction code.
- Likely used in conjunction with exception handling or error propagation in the `clore::extract` module.

### `clore::extract::CompilationDatabase`

Declaration: `extract/compiler.cppm:31`

Definition: `extract/compiler.cppm:31`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::CompilationDatabase` stores two primary fields: `entries`, a container of `clore::extract::CompileEntry` objects representing the parsed compilation records, and `toolchain_cache`, an `std::unordered_map` that maps toolchain identifiers to cached command-line argument sequences. The key invariant is that `toolchain_cache` is populated lazily when toolchain data is resolved, allowing repeated queries for the same toolchain without re‑parsing. The query method `has_cached_toolchain` returns `true` if at least one toolchain entry is present in the cache, reflecting whether any toolchain resolution has already been performed.

#### Invariants

- The `entries` vector holds all compile entries for the database and may be empty.
- The `toolchain_cache` maps toolchain identifiers to associated strings; its contents are managed externally.
- The struct does not enforce any relationship between `entries` and `toolchain_cache`.

#### Key Members

- `entries` : `std::vector<clore::extract::CompileEntry>`
- `toolchain_cache` : `std::unordered_map<std::string, std::vector<std::string>>`
- `has_cached_toolchain() const -> bool`

#### Usage Patterns

- Instantiated to store a set of compile commands and an optional toolchain cache.
- The `has_cached_toolchain()` method is used to query whether toolchain data has been previously stored.
- External code populates the `entries` vector and manages the `toolchain_cache` map.

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

The `clore::extract::CompileEntry` struct is the core data type for representing a single compilation command during extraction. Its internal structure consists of the original command fields—`file`, `directory`, and `arguments`—together with several derived fields that support deduplication and caching. The `normalized_file` member stores a canonicalized version of the input path, while `compile_signature` holds a hash computed from the command parameters (typically including `directory` and `arguments`) and uniquely identifies an equivalent compilation. The optional `source_hash` records a hash of the source file's contents, and `cache_key` is a string representation that combines `compile_signature` and, when present, `source_hash` to serve as a lookup key. A key invariant is that for any two entries that represent the same logical compilation, `compile_signature` must be equal; the `cache_key` further distinguishes entries with the same signature but different source content.

#### Invariants

- All fields are default-initialized as shown in the definition.
- `compile_signature` defaults to `0`, and `source_hash` is an empty `std::optional`.

#### Key Members

- `file`
- `directory`
- `arguments`
- `normalized_file`
- `compile_signature`
- `source_hash`
- `cache_key`

#### Usage Patterns

- Used throughout the extraction pipeline to represent individual compile commands from a compilation database.
- Derived fields are populated after the initial raw entry is read, enabling further processing such as deduplication and caching.

## Functions

### `clore::extract::build_compile_signature`

Declaration: `extract/compiler.cppm:58`

Definition: `extract/compiler.cppm:110`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first checks whether `entry.normalized_file` is non-empty and `entry.compile_signature` is already set to a non‑zero value; if so, it returns the cached signature immediately. Otherwise, it derives the normalized file path from `entry.normalized_file` if available, or falls back to calling `normalize_entry_file` on the entry. It then delegates all actual signature computation to `build_compile_signature_impl`, passing the entry and the resolved normalized path. This design centralizes the caching decision (avoiding re‑computation when the signature is already known) while keeping the core algorithmic work—hashing compiler arguments, source files, and toolchain data—inside the implementation function.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `entry.normalized_file`
- `entry.compile_signature`
- `normalize_entry_file(entry)` result

#### Usage Patterns

- Called to retrieve or compute a compile signature for a `CompileEntry`
- Used for caching and deduplication of compilation invocations

### `clore::extract::create_compiler_instance`

Declaration: `extract/compiler.cppm:65`

Definition: `extract/compiler.cppm:297`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function begins by sanitizing the compile entry’s driver arguments via `sanitize_driver_arguments`; if the resulting argument list is empty, it immediately returns `nullptr`. A physical filesystem (`llvm::vfs::createPhysicalFileSystem`) is created, and a `clang::CompilerInstance` diagnostics engine is constructed with an `IgnoringDiagConsumer`. The core parsing step calls `parse_compiler_invocation` (an internal helper) with the sanitized arguments, the entry’s source file path, the filesystem, and the diagnostics engine to produce a `clang::CompilerInvocation`. If parsing fails, the function returns `nullptr`. Otherwise, it configures the invocation: sets `DisableFree` to `false`, overrides the working directory with the entry’s `directory`, and checks that at least one frontend input is present.

Using the validated invocation, a `clang::CompilerInstance` is allocated and its virtual filesystem, diagnostics (again with an ignoring consumer), and file manager are initialized. Finally, `createTarget` is called; if target creation fails, `nullptr` is returned. On success, the function returns a fully constructed `CompilerInstance` ready for further extraction. Key dependencies include the argument sanitizer, the internal invocation parser, and Clang’s `CompilerInstance` and `CompilerInvocation` frameworks.

#### Side Effects

- creates diagnostic engine with ignoring consumer
- creates virtual file system
- parses compiler invocation
- modifies invocation frontend options
- creates compiler instance
- sets up file manager
- creates target

#### Reads From

- `CompileEntry` entry (file, directory)
- driver arguments from `sanitize_driver_arguments`
- filesystem for parsing
- maybe toolchain configuration

#### Writes To

- new `clang::CompilerInstance` (returned)
- invocation object fields (`DisableFree`, `WorkingDir`)
- diagnostic engine state

#### Usage Patterns

- used to obtain a configured clang compiler instance for a compile entry
- typically called during extraction to set up a clang tool

### `clore::extract::ensure_cache_key`

Declaration: `extract/compiler.cppm:60`

Definition: `extract/compiler.cppm:225`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/ensure-cache-key.md)

The function `clore::extract::ensure_cache_key` acts as a thin public wrapper around the implementation function `clore::extract::ensure_cache_key_impl`. It receives a mutable reference to a `CompileEntry` and immediately forwards it to `ensure_cache_key_impl`, which performs the actual work of computing and storing the cache key. This design separates the public interface from the implementation details, allowing `ensure_cache_key_impl` to reside in an anonymous namespace or internal translation unit while `ensure_cache_key` is declared in the module interface. The control flow is therefore trivial: a single delegation call with no additional logic, error handling, or preprocessing.

#### Side Effects

- Calls `ensure_cache_key_impl(entry)`, which may modify the cache key fields of the `CompileEntry` object

#### Reads From

- The `entry` parameter (passed by mutable reference to `ensure_cache_key_impl`)

#### Writes To

- The `entry` parameter (through the delegated call to `ensure_cache_key_impl`)

#### Usage Patterns

- Called by `query_toolchain_cached` to prepare a cache key before toolchain lookup

### `clore::extract::ensure_cache_key_impl`

Declaration: `extract/compiler.cppm:119`

Definition: `extract/compiler.cppm:119`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/ensure-cache-key-impl.md)

The implementation of `ensure_cache_key_impl` populates a `CompileEntry` object with the data needed to derive its cache key. It first calls `normalize_entry_file` to resolve the entry’s source file path and stores the result in `entry.normalized_file`. Using that normalized path and the `entry`, it invokes `build_compile_signature_impl` to compute a signature representing the compilation’s effective arguments and stores it in `entry.compile_signature`. It then attempts to hash the normalized source file via `try_hash_source_file`, writing the result (if available) into `entry.source_hash`. Finally, it constructs the cache key by calling `clore::support::build_cache_key` with the normalized file path and the compile signature, and assigns it to `entry.cache_key`.

The function’s control flow is strictly sequential: each step depends on the preceding one, and the cache key is the combined output of the earlier computations. The dependencies `normalize_entry_file`, `build_compile_signature_impl`, and `try_hash_source_file` are called exactly once, and their results are stored directly into the entry’s fields.

#### Side Effects

- Modifies the `CompileEntry` object by setting its `normalized_file`, `compile_signature`, `source_hash`, and `cache_key` fields.
- Reads the source file via `try_hash_source_file`, which performs I/O.
- Calls `normalize_entry_file` and `build_compile_signature_impl`, which may have their own side effects such as file access or computation.

#### Reads From

- the `entry` parameter (its existing fields)
- the source file identified by `entry.normalized_file` via `try_hash_source_file`
- the compile entry data used by `build_compile_signature_impl`

#### Writes To

- the `normalized_file` field of `entry`
- the `compile_signature` field of `entry`
- the `source_hash` field of `entry`
- the `cache_key` field of `entry`

#### Usage Patterns

- Called by `clore::extract::ensure_cache_key` to populate cache-related fields on a `CompileEntry`.

### `clore::extract::load_compdb`

Declaration: `extract/compiler.cppm:42`

Definition: `extract/compiler.cppm:127`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::load_compdb` first validates the existence of the provided `path` via `std::filesystem::path`, returning a `CompDbError` with a descriptive message if the file is missing. It then attempts to parse the JSON compilation database using `clang::tooling::JSONCompilationDatabase::loadFromFile` with auto‑detected command‑line syntax; failure again yields a `CompDbError` containing the parser diagnostics.  

On success, the function constructs a `CompilationDatabase` and iterates over every compile command returned by `getAllCompileCommands()`. For each command, a `CompileEntry` is populated with its `file`, `directory`, and the full `arguments` vector copied from the command line. Each entry is then passed to `ensure_cache_key` to derive a unique `cache_key` before being moved into `db.entries`. Finally, the function logs the total number of loaded commands and returns the populated `CompilationDatabase`.

#### Side Effects

- reads file system to check existence and load JSON file
- allocates memory for `CompilationDatabase` entries and strings
- calls `ensure_cache_key` which modifies each entry's internal state
- logs the number of loaded commands via `logging::info`

#### Reads From

- the `path` parameter
- the filesystem for file existence and content
- the parsed JSON commands from `json_db->getAllCompileCommands()`

#### Writes To

- the local `CompilationDatabase db` object
- the `entry.arguments` vector
- the `db.entries` vector
- each entry's internal state via `ensure_cache_key`
- the log output via `logging::info`

#### Usage Patterns

- Called to initialize a compilation database from a file path
- Used before extracting symbols or building dependency graphs

### `clore::extract::lookup`

Declaration: `extract/compiler.cppm:44`

Definition: `extract/compiler.cppm:164`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function performs a linear scan over `db.entries`. For each `entry`, it normalizes the given `file` relative to the `entry.directory` using `normalize_argument_path`. It then obtains the entry’s normalized file path, using `normalize_entry_file` if `entry.normalized_file` is empty, and compares the two results via `std::filesystem::path` equality. All entries that match are appended to a `results` vector. The function depends on `normalize_argument_path`, `normalize_entry_file`, and the `CompilationDatabase` / `CompileEntry` structures; it does not alter the database.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `db.entries`
- `entry.directory`
- `entry.normalized_file`
- `file` parameter

#### Usage Patterns

- Used to retrieve compile entries matching a given file path from the compilation database.

### `clore::extract::normalize_argument_path`

Declaration: `extract/compiler.cppm:49`

Definition: `extract/compiler.cppm:188`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first constructs a `std::filesystem::path` from the input `path` and, if it is relative, prepends the `directory` by combining them with the `/` `operator`. It then calls `std::filesystem::absolute` (with error‑code handling) to obtain an absolute form, falling back to the original if that operation fails. The resulting path is normalized via `lexically_normal()` to collapse redundant `.` and `..` components. Finally, the function attempts `std::filesystem::weakly_canonical`, which resolves as many symbolic‑link components as possible without requiring the whole path to exist; on success it returns that resolved path, otherwise it returns the lexically‑normalized path as a fallback. Error handling throughout uses `std::error_code` to detect and absorb filesystem‑level failures without throwing.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `path`
- parameter `directory`

#### Usage Patterns

- normalizing file path arguments from compilation entries
- combining a relative path with a directory to obtain an absolute path
- producing a canonical path for argument paths that may contain symbolic links

### `clore::extract::normalize_entry_file`

Declaration: `extract/compiler.cppm:56`

Definition: `extract/compiler.cppm:91`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/normalize-entry-file.md)

The function `clore::extract::normalize_entry_file` resolves a compile entry’s file path to a stable, normalized string. It first converts `entry.file` into a `std::filesystem::path`. If the path is relative, it is prefixed with `entry.directory` to form an absolute path; an explicit `fs::absolute` call is then attempted (falling back to the prefixed path on error). The path is then run through `lexically_normal()` to eliminate redundant separators and dot segments. To materialize symlinks and produce a fully resolved absolute path, `fs::weakly_canonical` is applied; if it fails (e.g., the file does not exist), the lexically normalized path is used as the final fallback. The result is returned as a generic string via `generic_string()`. The algorithm relies solely on `std::filesystem` operations and the fields of `CompileEntry` (`file` and `directory`).

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `entry.file`
- `entry.directory`

#### Usage Patterns

- Used by `build_compile_signature` to normalize the entry file path before hashing
- Used by `ensure_cache_key_impl` to produce a consistent file path representation

### `clore::extract::query_toolchain_cached`

Declaration: `extract/compiler.cppm:62`

Definition: `extract/compiler.cppm:233`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function begins by checking whether the `entry.arguments` vector is empty and returns an empty vector immediately if so. It then retrieves a cache key from `entry.cache_key`. If that key is empty, the function creates a copy of the entry, invokes `clore::extract::ensure_cache_key` on the copy to compute a key, and moves the result into the local `key` variable. The `key` is used to look up an existing result in the `db.toolchain_cache` map; a hit returns the cached `std::vector<std::string>` directly. On a cache miss, it calls `clore::extract::sanitize_tool_arguments` with the original `entry` to produce the sanitized tool arguments, inserts the result into the cache under the computed key, and returns that result. This design ensures that the potentially expensive argument‑sanitization step is performed only once per unique compile entry signature, relying on `clore::extract::ensure_cache_key` to provide a deterministic key and on `CompilationDatabase::toolchain_cache` as a memoization store.

#### Side Effects

- Modifies `db.toolchain_cache` by inserting or assigning a new key-value pair
- If the entry does not have a cache key, it creates a copy and modifies the copy's `cache_key` via `ensure_cache_key`

#### Reads From

- `entry.arguments`
- `entry.cache_key`
- `db.toolchain_cache`

#### Writes To

- `db.toolchain_cache`
- local copy's `cache_key` via `ensure_cache_key`

#### Usage Patterns

- Used when a caller wants sanitized tool arguments with automatic caching
- Called internally by extraction routines to avoid redundant sanitization

### `clore::extract::sanitize_driver_arguments`

Declaration: `extract/compiler.cppm:52`

Definition: `extract/compiler.cppm:207`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::sanitize_driver_arguments` begins by copying the `arguments` from the provided `CompileEntry` into a local vector `adjusted`. It then normalizes the entry’s source file path by calling `normalize_argument_path` with `entry.file` and `entry.directory`. Using `std::erase_if`, it removes from `adjusted` any argument that is non‑empty, does not start with a `-` (i.e., is not a flag), and whose normalized path equals the normalized source path. This effectively strips the explicit source file argument from the driver‑level command line, leaving only compiler flags and other options. The resulting vector is returned. The function depends on `normalize_argument_path` for path resolution and on the `CompileEntry` fields `arguments`, `file`, and `directory`. It is typically invoked prior to building a compile signature or constructing a compiler instance, ensuring driver arguments are free of redundant source‑file entries.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `entry.arguments`
- `entry.file`
- `entry.directory`

#### Usage Patterns

- Called to strip the source file argument from a driver argument list
- Used to sanitize compile command arguments before further analysis

### `clore::extract::sanitize_tool_arguments`

Declaration: `extract/compiler.cppm:54`

Definition: `extract/compiler.cppm:221`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::sanitize_tool_arguments` is a straightforward composition of two internal steps. It first invokes `clore::extract::sanitize_driver_arguments` on the provided `CompileEntry`, which produces a filtered, normalized list of driver-level arguments. The resulting sequence is then passed to `clore::extract::strip_compiler_path`, which removes the compiler executable path entry (the first argument) to yield a clean argument vector suitable for tool usage. No branching or conditional logic is present; the entire function delegates to these two helper functions in sequence, depending on their correctness and the underlying data in `CompileEntry`. The return type is `std::vector<std::string>`, representing the fully sanitized argument list.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `entry` (the `CompileEntry` parameter)

#### Usage Patterns

- normalize compile arguments before toolchain query
- strip compiler path and sanitize driver flags
- prepare argument list for further extraction steps

### `clore::extract::strip_compiler_path`

Declaration: `extract/compiler.cppm:47`

Definition: `extract/compiler.cppm:181`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::strip_compiler_path` accepts a vector of string arguments representing a compiler invocation. It removes the first element, which is assumed to be the compiler executable path, and returns the remaining arguments. If the input vector contains one or fewer entries, an empty vector is returned immediately; otherwise, a new vector is constructed by copying all elements from index 1 to the end. The implementation depends only on the C++ standard library containers and algorithms, with no calls to other project functions. The control flow consists of a single early-return check followed by a range-based copy.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `args` parameter

#### Usage Patterns

- Extracting compile flags after discarding the compiler path
- Used in preprocessing compile entries like `sanitize_driver_arguments`
- Building compile signatures without the executable name

## Internal Structure

The `extract:compiler` module is a C++20 module responsible for extracting and normalizing compiler invocations from compilation databases. It is decomposed into a public API surface and a set of internal helpers. The public layer provides functions to load a compilation database (`load_compdb`), look up entries (`lookup`), normalize entry file paths (`normalize_entry_file`), sanitize driver and tool arguments (`sanitize_driver_arguments`, `sanitize_tool_arguments`), ensure cache keys for entries (`ensure_cache_key`), compute compile signatures (`build_compile_signature`), query cached toolchains (`query_toolchain_cached`), create compiler instances (`create_compiler_instance`), and strip compiler paths from arguments (`strip_compiler_path`). These functions operate on core data structures: `CompileEntry` (source file, directory, arguments, normalized file, cache key, compile signature, source hash), `CompilationDatabase` (entries and toolchain cache), and `CompDbError` (error reporting). Internally, the module uses an anonymous namespace to house implementation details such as `parse_compiler_invocation`, `try_hash_source_file`, `build_compile_signature_impl`, and `ensure_cache_key_impl`, which handle argument parsing, path normalization, signature computation, and key derivation. The module imports the `std` and `support` modules; the latter provides foundational utilities for text processing, caching, and logging. This layered structure separates high‑level database and entry management from low‑level compiler invocation parsing and normalization, enabling clear dependency flow from the public interface down to internal utilities.

## Related Pages

- [Module support](../support/index.md)

