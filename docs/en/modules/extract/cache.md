---
title: 'Module extract:cache'
description: 'The extract:cache module is responsible for persisting and retrieving extraction results to enable incremental processing. It implements a cache subsystem that stores structured data for C++ modules (PCM and PCH entries), dependency snapshots, and build‑time signatures. The public interface provides functions to construct and decompose cache keys (build_cache_key, split_cache_key), compute compile‑command signatures (build_compile_signature), capture and compare dependency states (capture_dependency_snapshot, dependencies_changed), and load or save both simple integer entries (load_extract_cache, save_extract_cache) and full CliceCacheData structures (load_clice_cache, save_clice_cache). Additional utilities such as hash_file support content‑based change detection.'
layout: doc
template: doc
---

# Module `extract:cache`

## Summary

The `extract:cache` module is responsible for persisting and retrieving extraction results to enable incremental processing. It implements a cache subsystem that stores structured data for C++ modules (PCM and PCH entries), dependency snapshots, and build‑time signatures. The public interface provides functions to construct and decompose cache keys (`build_cache_key`, `split_cache_key`), compute compile‑command signatures (`build_compile_signature`), capture and compare dependency states (`capture_dependency_snapshot`, `dependencies_changed`), and load or save both simple integer entries (`load_extract_cache`, `save_extract_cache`) and full `CliceCacheData` structures (`load_clice_cache`, `save_clice_cache`). Additional utilities such as `hash_file` support content‑based change detection.

The module owns a set of public types that form the cache contract, including `CacheKeyParts`, `CacheRecord`, `DependencySnapshot`, `CliceCacheDepEntry`, `CliceCachePCMEntry`, `CliceCachePCHEntry`, `CliceCacheData`, and `CacheError`. These types are designed for schema‑compatible serialization with the broader clice workspace caching system. Internal helpers in an anonymous namespace handle path normalization, serialization format versioning, and atomic file writes, while the public API exposes a coherent set of operations for building, storing, and verifying cached extraction data.

## Imports

- [`extract:ast`](ast.md)
- [`extract:compiler`](compiler.md)
- [`extract:scan`](scan.md)
- `std`
- [`support`](../support/index.md)

## Types

### `clore::extract::cache::CacheError`

Declaration: `extract/cache.cppm:20`

Definition: `extract/cache.cppm:20`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The `clore::extract::cache::CacheError` struct is implemented as a simple wrapper around a single `std::string` member named `message`. This internal structure stores a human-readable description of the error condition. No additional invariants or constraints are imposed on the `message` value beyond those inherent to `std::string`. The compiler implicitly provides the default constructor, destructor, copy, and move operations, making the type trivially copyable and movable. The implementation deliberately keeps no further state, ensuring that error representation remains lightweight and directly accessible via the `message` field.

#### Invariants

- `message` contains a human-readable error description
- The struct is trivially constructible and copyable

#### Key Members

- `message`

#### Usage Patterns

- Returned or thrown to indicate an error during cache extraction
- Used as an error type in fallible operations within the `clore::extract::cache` namespace

### `clore::extract::cache::CacheKeyParts`

Declaration: `extract/cache.cppm:24`

Definition: `extract/cache.cppm:24`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The struct aggregates two members that together identify a unique cache entry: a `path` string representing the file or resource location, and a `compile_signature` that records a hash or versioning token from the compilation context. Both fields are set once during construction and remain unchanged for the lifetime of the object, forming an immutable key. No additional invariants are enforced by the struct itself; the caller is responsible for ensuring that the combination of `path` and `compile_signature` is unique within the containing cache.

#### Invariants

- Both `path` and `compile_signature` must be populated before use as a cache key.
- The `compile_signature` is expected to be derived deterministically from compilation parameters.

#### Key Members

- `path` member
- `compile_signature` member

#### Usage Patterns

- Used as a key in associative containers (e.g., `std::map` or `std::unordered_map`) for caching extracted data.
- Compared via default equality `operator`s for cache lookup.
- Copied or moved when inserting or retrieving cache entries.

### `clore::extract::cache::CacheRecord`

Declaration: `extract/cache.cppm:36`

Definition: `extract/cache.cppm:36`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The `clore::extract::cache::CacheRecord` struct bundles all data produced during extraction into a single record. Its fields are ordered such that the two identity hashes—`compile_signature` and `source_hash`—come first and are zero‑initialized by default, signalling a missing or invalid cache entry. The remaining fields, `ast_deps`, `scan`, and `ast`, hold the dependency snapshot, scan result, and abstract syntax tree result, respectively, and are only meaningful when the hashes match the source being loaded.

An invariant of the record is that the pair (`compile_signature`, `source_hash`) uniquely identifies the compilation context and input source; a cache hit requires both values to equal the caller’s computed hashes. The struct provides no custom constructors or assignment `operator`s, relying on default member initialization and aggregate initialization for simplicity. All fields are public, allowing direct access for efficient serialisation and comparison.

#### Invariants

- All fields are public and default-initialized.
- `compile_signature` and `source_hash` are zero-initialized by default.
- The types `DependencySnapshot`, `ScanResult`, and `ASTResult` are assumed to be default-constructible.

#### Key Members

- `compile_signature`
- `source_hash`
- `ast_deps`
- `scan`
- `ast`

#### Usage Patterns

- Used as a record type for caching extracted data.
- Instances are likely stored in a map or container keyed by source hash or compile signature.

### `clore::extract::cache::CliceCacheData`

Declaration: `extract/cache.cppm:68`

Definition: `extract/cache.cppm:68`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The implementation summary for `clore::extract::cache::CliceCacheData`: The struct aggregates three vectors that together represent the entire state of the clice cache. The `paths` field stores the list of source file paths that have been processed; the `pch` and `pcm` fields hold the corresponding cache entries for precompiled headers and precompiled modules, respectively. The vectors are expected to be populated in a coordinated manner, ensuring that indices across the three collections remain consistent relative to the same extraction session. No invariants are enforced beyond standard vector operations; the struct serves as a plain data container for serialization and runtime access.

#### Invariants

- No explicit invariants are documented in the evidence.

#### Key Members

- `paths`
- `pch`
- `pcm`

#### Usage Patterns

- Used to store and pass around cached extraction data.

### `clore::extract::cache::CliceCacheDepEntry`

Declaration: `extract/cache.cppm:46`

Definition: `extract/cache.cppm:46`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The struct `clore::extract::cache::CliceCacheDepEntry` holds a single dependency mapping within the clice workspace cache. Its two fields — `path` (a `std::uint32_t`) and `hash` (a `std::uint64_t`) — together form a compact, fixed‑size record that is deliberately kept schema‑compatible with the external `CacheData` structure in `clice/src/server/workspace.cpp`. The `path` field stores an identifier (likely an index into a path table or a directory‑relative token), while `hash` stores a checksum or content‑hash for the referenced file. Both fields are zero‑initialized by default, so an entry with `path == 0` and `hash == 0` represents an empty or invalid slot. The struct is trivially copyable and intended for use in contiguous storage (e.g., `std::vector<CliceCacheDepEntry>`), enabling efficient serialisation and on‑disk compatibility with the upstream workspace cache format. No extra invariants are enforced beyond the default‑constructed state and the schema‑matching layout.

#### Invariants

- Fields are default-initialized to zero
- Schema compatible with external `CacheData`

#### Key Members

- `path`
- `hash`

#### Usage Patterns

- Used in clice workspace cache structures
- Schema-compatible with clice/server/workspace`.cpp` `CacheData`

### `clore::extract::cache::CliceCachePCHEntry`

Declaration: `extract/cache.cppm:51`

Definition: `extract/cache.cppm:51`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The struct `clore::extract::cache::CliceCachePCHEntry` stores metadata for a cached precompiled header (PCH) file. Its fields include `filename` (the PCH path), `source_file` (a zero‑based file index), `hash` (a content hash for the PCH), `bound` (an unsigned counter or index), `build_at` (a timestamp), and `deps` (a vector of `CliceCacheDepEntry` objects listing dependencies). The invariant is that `hash` uniquely represents the PCH content, while `deps` captures all file dependencies that influenced its production. The combination of `source_file` and `filename` ties the entry to a specific source translation unit.

#### Invariants

- `hash` uniquely identifies the PCH content
- `source_file` references a valid source file index
- `deps` contains all source-level dependencies

#### Key Members

- `filename`
- `source_file`
- `hash`
- `bound`
- `build_at`
- `deps`

#### Usage Patterns

- looked up by `hash` to find cached PCH
- compared against current source file dependencies

### `clore::extract::cache::CliceCachePCMEntry`

Declaration: `extract/cache.cppm:60`

Definition: `extract/cache.cppm:60`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The struct `clore::extract::cache::CliceCachePCMEntry` aggregates the persistent metadata of a cached precompiled module (PCM) entry. Internally it holds a `filename` identifying the PCM file path, a `source_file` index (defaulting to `0`), the `module_name`, a `build_at` timestamp (defaulting to `0`), and a `deps` vector of `CliceCacheDepEntry` objects representing its dependencies. The numeric defaults (`0` for `source_file` and `build_at`) serve as sentinel values indicating an uninitialised or invalid state, while the `deps` vector remains empty until populated. The struct is a plain aggregate with no custom constructors, assignment `operator`s, or invariants enforced beyond the zero‑initialisation of its scalar fields; all member data is directly accessible.

#### Invariants

- `filename` and `module_name` together should uniquely identify a PCM entry.
- `source_file` is an index referencing an entry in an external source file table.
- `build_at` stores a timestamp in `int64_t` format, likely milliseconds or seconds since epoch.
- `deps` contains all direct dependencies of this PCM entry.

#### Key Members

- `filename`
- `module_name`
- `build_at`
- `source_file`
- `deps`

#### Usage Patterns

- Stored in a cache container indexed by `filename` or `module_name`.
- Used to compare timestamps and dependencies against the current build state.
- Serialized and deserialized for persistent caching between builds.
- Populated by the extraction phase and consulted during compilation to reuse `PCMs`.

### `clore::extract::cache::DependencySnapshot`

Declaration: `extract/cache.cppm:29`

Definition: `extract/cache.cppm:29`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The `clore::extract::cache::DependencySnapshot` struct captures a consistent view of a dependency set at a specific build time. Internally, it stores three parallel vectors — `files`, `hashes`, and `mtimes` — each of which must contain the same number of elements, with the index into one vector corresponding to the same dependency file in the others. The `build_at` member records the timestamp (as `std::int64_t`) when the snapshot was created, defaulting to `0`. This structure guarantees that file paths, content hashes, and modification times are recorded atomically; the absence of any separate constructor or mutation method means the snapshot is effectively immutable after aggregate initialization, preserving the invariant that the three vectors remain aligned and represent a single coherent dependency state.

#### Invariants

- The `files`, `hashes`, and `mtimes` vectors have the same size when representing a consistent snapshot.
- `build_at` is expected to be a monotonic timestamp indicating when the snapshot was captured.

#### Key Members

- `files` - the list of dependency file paths.
- `hashes` - the hash values for each dependency file.
- `mtimes` - the last modification times for each dependency file.
- `build_at` - the timestamp of the snapshot creation.

#### Usage Patterns

- Used to cache and compare the state of dependency files across builds.
- Stored or serialized to detect changes in dependencies since the last build.

## Functions

### `clore::extract::cache::build_cache_key`

Declaration: `extract/cache.cppm:76`

Definition: `extract/cache.cppm:228`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function constructs a cache key by concatenating the normalized path and the compile signature, separated by a delimiter constant, `kCacheKeyDelimiter`. It first reserves sufficient memory to avoid reallocation, then appends the path, the delimiter, and the string representation of the signature. The resulting string serves as a unique identifier for the cache entry. The only external dependency is the delimiter constant, which is used to later split the key when parsing cache entries (e.g., in `split_cache_key`).

#### Side Effects

- Allocates and returns a new `std::string`

#### Reads From

- `normalized_path` parameter
- `compile_signature` parameter
- `kCacheKeyDelimiter`

#### Writes To

- Returned `std::string`'s internal buffer

#### Usage Patterns

- Used to generate a unique key for caching extraction results
- Called when saving or loading cache entries

### `clore::extract::cache::build_compile_signature`

Declaration: `extract/cache.cppm:74`

Definition: `extract/cache.cppm:224`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function `clore::extract::cache::build_compile_signature` acts as a thin delegation wrapper: it forwards the provided `CompileEntry` argument to the core function `clore::extract::build_compile_signature` and returns its `std::uint64_t` result directly. No additional computation, validation, or transformation is performed at this level. The internal control flow is therefore a single call, with the entire algorithmic responsibility residing in the referenced `clore::extract::build_compile_signature`. Dependencies are minimal—only the target function and the type `CompileEntry`—reflecting its role as a cache‑layer adapter that reuses an existing signature computation for cache‑key generation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- entry (const `CompileEntry`&)

#### Usage Patterns

- Computes a signature for cache key generation
- Called by `load_extract_cache` and `save_extract_cache`

### `clore::extract::cache::capture_dependency_snapshot`

Declaration: `extract/cache.cppm:83`

Definition: `extract/cache.cppm:282`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function `clore::extract::cache::capture_dependency_snapshot` begins by normalising the provided file list via `normalize_path_string`, sorting the result, and removing duplicates. It then initialises a `DependencySnapshot` with a current timestamp stored in `snapshot.build_at`. For parallel hashing, it computes a thread count based on `std::thread::hardware_concurrency` and partitions work into `per_thread` chunks, spawning `std::thread` workers. Each worker iterates over its assigned range, checking a mutex‑protected `first_error` at the start of every iteration to bail early on failure. For each file it uses `llvm::sys::fs::status` to obtain the last‑modification time and `hash_file` to compute a content hash. If `hash_file` fails (e.g., a generated header was cleaned between runs), the worker records a hash of `0` instead of aborting, so the cache entry will appear changed on the next check. After all threads join, the results—`files`, `hashes`, and `mtimes`—are collected into the snapshot from the per‑index `task_results` vector. A single `CacheError` (if any) is propagated via `std::unexpected`.

#### Side Effects

- Reads file system to compute file hashes and modification times

#### Reads From

- Input parameter `files` (vector of file paths)
- File system via `hash_file` and `llvm::sys::fs::status`

#### Writes To

- Local variables: `snapshot`, `task_results`, `normalized`, `first_error`
- Memory allocation for `DependencySnapshot` and internal containers

#### Usage Patterns

- Called to capture the current state of dependency files before caching
- Used to compare against previously stored snapshots to detect changes

### `clore::extract::cache::dependencies_changed`

Declaration: `extract/cache.cppm:86`

Definition: `extract/cache.cppm:401`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function begins with guard checks: if `build_at` is non‑positive or `files` is empty, or if the sizes of `files`, `hashes`, and `mtimes` are inconsistent, it returns `true` (i.e., dependencies have changed). For small dependency sets (up to `kParallelThreshold` of 16), it iterates sequentially over each index, calling `check_single_dependency`; on the first `true` result it returns immediately. For larger sets, it spawns a number of threads equal to the hardware concurrency (defaulting to 1), each processing a contiguous chunk of indices. An `std::atomic<bool>` flag `changed` allows early exit: each worker checks `changed.load` with `memory_order_relaxed` before calling `check_single_dependency` and sets the flag if a change is detected. After joining all threads, the function returns the final value of `changed`. The only internal dependency is the helper `check_single_dependency`, which evaluates whether a single dependency file has changed relative to the stored snapshot data.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `snapshot.build_at`
- `snapshot.files`
- `snapshot.hashes`
- `snapshot.mtimes`
- `std::thread::hardware_concurrency()`

#### Usage Patterns

- used to decide whether to reuse cached compilation results
- called after loading a dependency snapshot to detect changes

### `clore::extract::cache::hash_file`

Declaration: `extract/cache.cppm:81`

Definition: `extract/cache.cppm:270`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The implementation of `clore::extract::cache::hash_file` reads the entire file at the given path into a memory buffer via `llvm::MemoryBuffer::getFile`. If the file cannot be read (e.g., due to permission errors or missing file), it returns a `std::unexpected` containing a `CacheError` with a descriptive message generated using `std::format`. On success, the function applies the `xxHash` 64-bit algorithm (`llvm::xxh3_64bits`) to the raw buffer contents and returns the resulting hash value wrapped in a `std::expected`. The function depends on the LLVM support library for file I/O and hashing, and on the `CacheError` type defined within the `clore::extract::cache` namespace for error reporting.

#### Side Effects

- Reads file content from the filesystem via `llvm::MemoryBuffer::getFile`.

#### Reads From

- parameter `path`
- file at `path`

#### Usage Patterns

- Used to compute hash of source files for cache key generation
- Likely called by cache build or verification logic

### `clore::extract::cache::load_clice_cache`

Declaration: `extract/cache.cppm:95`

Definition: `extract/cache.cppm:670`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function first calls `clice_cache_file_path` to obtain the expected cache file path for the given `workspace_root`. If this fails, it returns the error immediately. Otherwise, it checks whether the file exists via `fs::exists`; if the file is absent (or an `std::error_code` signals an error), it returns an empty `CliceCacheData` – meaning no prior cache is available. When the file does exist, it reads the entire contents with `clore::support::read_utf8_text_file`; a read failure yields a `CacheError` describing the issue. The JSON content is then deserialised into `CliceCacheData` using `json::from_json`. If deserialisation fails, the function logs a warning via `logging::warn` and returns an empty `CliceCacheData` (treating the cache as stale). On success, the populated `CliceCacheData` is returned.

Key dependencies are `clice_cache_file_path` for path resolution, `read_utf8_text_file` for I/O, and `json::from_json` for structured deserialisation. The function deliberately absorbs parse failures into a silent empty result rather than propagating errors, on the assumption that a corrupted cache should be discarded without blocking the caller.

#### Side Effects

- filesystem existence check
- file I/O reading the clice cache file
- logging a warning on JSON parse failure

#### Reads From

- the clice cache file at the path derived from `workspace_root`

#### Usage Patterns

- loading cached clice data for incremental compilation
- checking cache validity before extraction

### `clore::extract::cache::load_extract_cache`

Declaration: `extract/cache.cppm:88`

Definition: `extract/cache.cppm:457`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function `clore::extract::cache::load_extract_cache` computes the on‑disk cache file path via `cache_file_path` and checks for its existence. If the file is missing or inaccessible, it returns an empty map or a `CacheError`. Otherwise, it reads the file as UTF‑8 text using `clore::support::read_utf8_text_file`, then deserialises the JSON content into a `SerializedCacheData` struct via `json::from_json`. A format‑version mismatch against `kExtractCacheFormatVersion` (or a parse failure) causes a warning and an empty map to be returned, effectively treating the cache as stale.

After validation, the function iterates over `data.entries`. For each entry it validates the `source_file` index against `data.paths`, decodes the dependency snapshot using `decode_dependency_snapshot`, normalises the source path via `normalize_path_string`, and builds a cache key with `build_cache_key`. It then populates the result map with a `CacheRecord` holding the `compile_signature`, `source_hash`, decoded `ast_deps`, `scan`, and `ast` from the entry. The function returns the assembled map of cache keys to records.

#### Side Effects

- reads a file from disk
- logs warnings via `logging::warn`

#### Reads From

- `workspace_root` parameter
- cache file on disk (path derived from `workspace_root`)
- global constant `kExtractCacheFormatVersion`
- contents of the cache file (JSON, paths, entries)

#### Usage Patterns

- called during initialization to restore previously saved extract cache
- typically used in conjunction with `save_extract_cache` and `dependencies_changed`

### `clore::extract::cache::save_clice_cache`

Declaration: `extract/cache.cppm:97`

Definition: `extract/cache.cppm:710`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The implementation of `clore::extract::cache::save_clice_cache` first computes the target cache file path via `clore::extract::cache::(anonymous namespace)::clice_cache_file_path` and ensures its parent directory exists. It serializes the provided `CliceCacheData` struct to a JSON string using `json::to_json`; any serialization failure is immediately returned as a `CacheError`.  

To achieve atomic file replacement, the function constructs a unique temporary file path by combining the final cache path with a suffix built from the process ID, a nanosecond‑precision timestamp, and a nonce derived via `llvm::xxh3_64bits`. It retries up to 32 times to avoid collisions, checking each candidate’s existence with `fs::exists`. Once a fresh temporary path is obtained, the serialized content is written using `clore::support::write_utf8_text_file`. Finally, the temporary file is renamed to the final cache path via `fs::rename`. If the rename fails with `permission_denied`, `file_exists`, or `operation_not_permitted`, the function attempts to remove the existing cache file and retries the rename; any remaining error is propagated. On success, an empty `std::expected` is returned.

#### Side Effects

- creates directories in the file system
- writes a UTF-8 text file to a temporary path
- renames a temporary file to the final cache path
- may remove an existing cache file before rename

#### Reads From

- `workspace_root` parameter
- `data` parameter
- system clock for timestamp
- process ID via `llvm::sys::Process::getProcessId`
- xxhash for nonce generation

#### Writes To

- clice cache directory and file beneath `workspace_root`
- temporary files in the same directory

#### Usage Patterns

- Called to persist extracted clice data to disk
- Used after successful compilation or extraction phases

### `clore::extract::cache::save_extract_cache`

Declaration: `extract/cache.cppm:91`

Definition: `extract/cache.cppm:533`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function first resolves the cache directory via `cache_directory(workspace_root)` and ensures it exists using `fs::create_directories`. It then constructs a `SerializedCacheData` object: it interns each path from the record map with `normalize_path_string` and assigns a unique index, validates that each record’s `compile_signature` matches the signature extracted by `split_cache_key` (returning an error on mismatch), and encodes the dependency snapshot via `encode_dependency_snapshot`. The resulting `SerializedCacheEntry` objects are serialized to JSON with `json::to_json`. A unique temporary file path is generated by iterating up to 32 candidate names composed from the cache path, process ID, timestamp, and an xxhash3 nonce; the first candidate that does not already exist is chosen. The serialized content is written to this temp file using `clore::support::write_utf8_text_file`, then atomically renamed to the target `cache.json`. If the rename fails due to permission or existing file errors, the target file is removed and the rename retried; any other rename failure returns an error. On success, the function returns an empty expected.

#### Side Effects

- Creates cache directory if missing
- Writes temporary file to filesystem
- Renames temporary file to final cache path (atomic on same filesystem)
- Removes existing cache file if rename fails due to permission
- May update directory metadata

#### Reads From

- Parameter `workspace_root`
- Parameter `records` (the `unordered_map` of `CacheRecord`)
- Helper function `split_cache_key` returns internal key parts
- Helper function `json::to_json` processes the data structure

#### Writes To

- Filesystem cache directory (created by `create_directories`)
- Temporary file near the cache path
- Final cache file `cache.json` in the cache directory
- Local variables: `path_ids` map and `data` structure (internal)

#### Usage Patterns

- Called to persist the extract cache after building or updating cache records
- Used in the extract pipeline to save results for future reuse

### `clore::extract::cache::split_cache_key`

Declaration: `extract/cache.cppm:79`

Definition: `extract/cache.cppm:238`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function first locates the separator between the path and the compile signature by calling `cache_key.rfind(kCacheKeyDelimiter)`. If the delimiter is not found, or if either the left-hand (path) or right-hand (signature) substring is empty, it returns a `CacheError` with a descriptive message. Otherwise, it parses the signature portion using `std::from_chars` with a `std::uint64_t` target. Parsing must consume the entire substring and succeed with no error; otherwise an error is returned. On success, the function returns a `CacheKeyParts` containing the path string and the parsed signature.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `cache_key`
- constant `kCacheKeyDelimiter`

#### Writes To

- returned `std::expected<CacheKeyParts, CacheError>` value

#### Usage Patterns

- used in cache load/save operations to decompose a combined cache key

## Internal Structure

The `extract:cache` module provides a persistent caching subsystem for extraction results, reducing redundant work by storing computed dependency snapshots, compile signatures, AST data, and clice workspace records. It is decomposed into a public API surface—`load_extract_cache`, `save_extract_cache`, `load_clice_cache`, `save_clice_cache`, `capture_dependency_snapshot`, `dependencies_changed`, and related key‑building functions—and an internal implementation that handles serialization, path normalization, and file I/O. Public types such as `CacheError`, `CacheKeyParts`, `DependencySnapshot`, and `CliceCacheData` define the data contracts, while anonymous‑namespace helpers like `SerializedCacheEntry`, `SerializedDependencySnapshot`, `CachedPathHash`, and functions (`normalize_path_string`, `clice_cache_file_path`, `encode_dependency_snapshot`, `decode_dependency_snapshot`) implement the low‑level serialization and disk access logic.

The module imports `extract:ast`, `extract:compiler`, and `extract:scan` for AST extraction, compilation entry handling, and source scanning respectively, together with the `support` module for foundational utilities (e.g., file I/O, path normalization, hashing). Internally, the cache is organized around workspace‑compatible structures (`CliceCacheDepEntry`, `CliceCachePCMEntry`, `CliceCachePCHEntry`) that mirror the schema defined in the larger clice workspace cache (`clice/src/server/workspace.cpp`). The implementation layers separate serialization concerns from cache‑lookup logic, using a parallel worker pool (with tunable thresholds) to hash dependencies concurrently, and retry mechanisms for atomic temporary‑file writes to ensure data integrity.

## Related Pages

- [Module extract:ast](ast.md)
- [Module extract:compiler](compiler.md)
- [Module extract:scan](scan.md)
- [Module support](../support/index.md)

