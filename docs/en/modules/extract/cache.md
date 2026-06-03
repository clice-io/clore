---
title: 'Module extract:cache'
description: 'The extract:cache module is responsible for providing caching infrastructure for the extraction workflow. It owns the public API for computing deterministic cache keys (build_cache_key, split_cache_key), hashing source files (hash_file, build_compile_signature), and capturing dependency snapshots (capture_dependency_snapshot, dependencies_changed). It also manages persistence of both the extraction cache (via load_extract_cache and save_extract_cache) and the Clice workspace cache (via load_clice_cache and save_clice_cache). Public data structures include CacheRecord, CacheError, CacheKeyParts, DependencySnapshot, and the Clice-specific CliceCacheData, CliceCacheDepEntry, CliceCachePCMEntry, and CliceCachePCHEntry. The module builds on utilities from extract:ast, extract:compiler, extract:scan, and support to serialize, store, and retrieve cached results that accelerate repeated extractions and detect stale dependencies.'
layout: doc
template: doc
---

# Module `extract:cache`

## Summary

The `extract:cache` module is responsible for providing caching infrastructure for the extraction workflow. It owns the public API for computing deterministic cache keys (`build_cache_key`, `split_cache_key`), hashing source files (`hash_file`, `build_compile_signature`), and capturing dependency snapshots (`capture_dependency_snapshot`, `dependencies_changed`). It also manages persistence of both the extraction cache (via `load_extract_cache` and `save_extract_cache`) and the Clice workspace cache (via `load_clice_cache` and `save_clice_cache`). Public data structures include `CacheRecord`, `CacheError`, `CacheKeyParts`, `DependencySnapshot`, and the Clice-specific `CliceCacheData`, `CliceCacheDepEntry`, `CliceCachePCMEntry`, and `CliceCachePCHEntry`. The module builds on utilities from `extract:ast`, `extract:compiler`, `extract:scan`, and `support` to serialize, store, and retrieve cached results that accelerate repeated extractions and detect stale dependencies.

## Imports

- [`extract:ast`](ast.md)
- [`extract:compiler`](compiler.md)
- [`extract:scan`](scan.md)
- [`support`](../support/index.md)

## Types

### `clore::extract::cache::CacheError`

Declaration: `src/extract/cache.cppm:36`

Definition: `src/extract/cache.cppm:36`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

`clore::extract::cache::CacheError` is implemented as a simple wrapper around a single `std::string` member `message`. Its internal structure carries no additional invariants beyond those imposed by `std::string` itself; the `message` field is the sole carrier of error information. Because the struct has no user‑defined constructors, assignment `operator`s, or destructor, the compiler generates default implementations for all special member functions, relying on `std::string`’s own copy/move semantics. This design keeps the type lightweight and suitable for use as an error result in contexts where a human‑readable description is sufficient.

### `clore::extract::cache::CacheKeyParts`

Declaration: `src/extract/cache.cppm:40`

Definition: `src/extract/cache.cppm:40`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The `clore::extract::cache::CacheKeyParts` struct aggregates two fundamental components that collectively form the key for a cached compilation result. The `path` member stores the filesystem path of the source file as a `std::string`, while the `compile_signature` member holds a `std::uint64_t` that encodes a hash or identifier of the compilation environment and options used. This pairing ensures that each unique combination of source file and compilation context maps to a distinct cache entry, and the struct itself serves as a plain data container with no additional invariants beyond the values assigned to its two fields.

#### Invariants

- `path` is expected to be a valid file path
- `compile_signature` is expected to be a unique identifier for a compilation configuration

#### Key Members

- `path`
- `compile_signature`

#### Usage Patterns

- Used as a key in caching mechanisms
- Combined to uniquely identify cache entries

### `clore::extract::cache::CacheRecord`

Declaration: `src/extract/cache.cppm:52`

Definition: `src/extract/cache.cppm:52`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The struct `clore::extract::cache::CacheRecord` aggregates the data required to represent a single cache entry for a clang‑based extraction unit. Its two hash fields, `compile_signature` and `source_hash`, serve as cache keys: `compile_signature` captures the compilation context (e.g., compiler flags and toolchain settings) while `source_hash` stores a content digest of the main source file. The remaining three fields hold the stored results: `ast_deps` (a `DependencySnapshot` recording the file dependencies discovered during AST generation), `scan` (the `ScanResult` from the pre‑extraction scan pass), and `ast` (the final `ASTResult` containing the extracted declarations). A cache hit occurs only when both hash values match the currently computed values, ensuring that any change in either source content or compilation environment invalidates the record. The struct carries no explicit invariants beyond the implicit requirement that all result fields remain consistent with the hashed inputs.

### `clore::extract::cache::CliceCacheData`

Declaration: `src/extract/cache.cppm:84`

Definition: `src/extract/cache.cppm:84`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The struct `clore::extract::cache::CliceCacheData` is a plain aggregate container that holds the harvested output of a cache extraction pass. Its three fields—`paths`, `pch`, and `pcm`—are independent `std::vector` instances storing extracted file paths, precompiled‑header entries (type `CliceCachePCHEntry`), and precompiled‑module entries (type `CliceCachePCMEntry`), respectively. No cross‑field invariants are enforced; each vector grows independently as entries are appended during the extraction process. The design intentionally separates different cache artifact types into contiguous, resizable sequences to facilitate straightforward serialisation, deserialisation, and iteration by downstream logic. Because it is an aggregate, instances can be constructed with brace‑initialisation or default‑constructed and filled via `push_back` or similar operations, making the struct a minimal, data‑focused building block of the caching subsystem.

#### Invariants

- No invariants are enforced beyond the types of the members.

#### Key Members

- `paths`
- `pch`
- `pcm`

#### Usage Patterns

- Used to hold extracted cache data for PCH and PCM entries along with their paths.

### `clore::extract::cache::CliceCacheDepEntry`

Declaration: `src/extract/cache.cppm:62`

Definition: `src/extract/cache.cppm:62`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The `clore::extract::cache::CliceCacheDepEntry` struct is a plain-old-data type that pairs a `path` identifier (stored as `std::uint32_t` and defaulting to `0`) with a content `hash` (`std::uint64_t`, also defaulting to `0`). Its design is primarily driven by the need for schema compatibility with an existing workspace cache format, implying the member layout and sizes are fixed and directly serializable. The zero-initialized members serve as a canonical invalid or empty state, and the absence of any user‑defined constructors or special member functions ensures the struct remains trivially copyable and suitable for direct memory‑mapped or binary storage.

#### Invariants

- Both fields are default-initialized to 0.
- The struct layout is compatible with an external schema.

#### Key Members

- `path`
- `hash`

#### Usage Patterns

- Used as an element in cache structures for clice workspace.
- Likely stored in collections (e.g., vectors) and serialized/deserialized for cache persistence.

### `clore::extract::cache::CliceCachePCHEntry`

Declaration: `src/extract/cache.cppm:67`

Definition: `src/extract/cache.cppm:67`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The `clore::extract::cache::CliceCachePCHEntry` struct captures a cached precompiled header (PCH) file along with metadata needed to validate and manage reuse. The `filename` string stores the PCH’s path, while `source_file` is an internal identifier (likely an index) for the source that produced it. The `hash` field holds a content fingerprint of the input, and `build_at` records the timestamp of the last build. The `bound` field tracks a related version or binding count, and the vector `deps` contains `CliceCacheDepEntry` objects representing dependencies that must remain unchanged for the cache to be valid.

Key invariants include that `hash` and `build_at` together determine staleness: a mismatch in `hash` or a `build_at` older than that of any dependency invalidates the entry. The `source_file` and `filename` should be consistent, and the `deps` vector is expected to be non‑empty if the PCH depends on other units. The `bound` field is used to control how many times the entry can be reused before re‑validation, acting as a simple refcount or lease counter. No special alignment or ordering invariants are enforced beyond the natural byte layout of the fields.

#### Invariants

- filename corresponds to a valid PCH file path
- hash is computed from PCH content
- `build_at` stores a valid timestamp

#### Key Members

- hash
- deps
- filename
- `source_file`
- `build_at`
- bound

#### Usage Patterns

- Stored in a cache container keyed by PCH identity
- Checked for staleness using hash and deps
- Serialized/deserialized for persistent caching

### `clore::extract::cache::CliceCachePCMEntry`

Declaration: `src/extract/cache.cppm:76`

Definition: `src/extract/cache.cppm:76`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The struct `clore::extract::cache::CliceCachePCMEntry` bundles the metadata for a single cached Precompiled Module entry. Its data members are all public, with `filename` and `module_name` storing the PCM path and the module’s logical name, `source_file` acting as an index into a file table (defaulting to `0`), `build_at` holding a timestamp (default `0` indicates “not built”), and `deps` containing a vector of `CliceCacheDepEntry` objects that record the entry’s dependencies. The implementation is a plain aggregate; no constructors, destructors, or invariants are enforced beyond the initializers. The zero-based defaults for `source_file` and `build_at` serve as sentinel values, and an empty `deps` vector is the expected representation of an entry with no dependencies.

#### Invariants

- Default initializers for numeric fields (`source_file`, `build_at`) are zero.
- String fields (`filename`, `module_name`) are default-constructed as empty strings.
- The `deps` vector is default-constructed as empty.

#### Key Members

- `filename` (`std::string`)
- `source_file` (`std::uint32_t`)
- `module_name` (`std::string`)
- `build_at` (`std::int64_t`)
- `deps` (`std::vector<CliceCacheDepEntry>`)

### `clore::extract::cache::DependencySnapshot`

Declaration: `src/extract/cache.cppm:45`

Definition: `src/extract/cache.cppm:45`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The struct `clore::extract::cache::DependencySnapshot` stores the cached state of a set of dependency files as three parallel vectors: `files`, `hashes`, and `mtimes`. A core invariant is that `files`, `hashes`, and `mtimes` always have the same length, representing a single logical snapshot of the dependencies. The field `build_at` records the time (as a Unix epoch in seconds) when the snapshot was taken, enabling invalidation logic based on file‑system modification times. The implementation treats the three vectors as an atomic unit: any update to the snapshot must assign all three vectors together to maintain consistency. The `build_at` value is a simple scalar that can be set independently, but zero is used as a sentinel for uninitialized or empty snapshots.

## Functions

### `clore::extract::cache::build_cache_key`

Declaration: `src/extract/cache.cppm:92`

Definition: `src/extract/cache.cppm:244`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function constructs a cache key by concatenating the normalized path and the compile signature, separated by the delimiter stored in `kCacheKeyDelimiter`. It first reserves enough memory to hold the input strings plus one delimiter character, then appends each part in order: the `normalized_path`, the delimiter, and the string representation of `compile_signature`. This avoids reallocations by pre‑reserving the exact required capacity. The resulting key is then used to index cache entries; the two components can later be recovered via `split_cache_key`. No external dependencies beyond standard string operations are involved, and the function is designed for efficiency within the cache subsystem.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `normalized_path` parameter (type `std::string_view`)
- `compile_signature` parameter (type `std::uint64_t`)
- `kCacheKeyDelimiter` (presumably a global constant)

#### Writes To

- the returned `std::string` object

#### Usage Patterns

- called to generate a cache lookup key from a file path and its compile signature
- used prior to storing or retrieving cached extraction data

### `clore::extract::cache::build_compile_signature`

Declaration: `src/extract/cache.cppm:90`

Definition: `src/extract/cache.cppm:240`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function `clore::extract::cache::build_compile_signature` is a thin delegation wrapper that forwards its `CompileEntry` argument to `clore::extract::build_compile_signature`. It receives the returned `std::uint64_t` hash and passes it back to the caller. No additional processing or error handling is performed at this layer; the algorithm, control flow, and dependencies are entirely inherited from the underlying `clore::extract::build_compile_signature`, which computes a deterministic compile‑signature based on the entry’s configuration and inputs. This separation allows the cache‑specific signature builder to be substituted or extended without altering the core extraction logic.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `entry` parameter (`const CompileEntry&`)

#### Usage Patterns

- called to obtain a hash-based signature for a compile entry
- used internally by cache operations to identify compilation units

### `clore::extract::cache::capture_dependency_snapshot`

Declaration: `src/extract/cache.cppm:99`

Definition: `src/extract/cache.cppm:298`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function begins by normalizing each input file path using `normalize_path_string`, then sorting and deduplicating the list. A `DependencySnapshot` is initialized with a `build_at` timestamp obtained from `std::chrono::system_clock`. To compute the hash and modification time values required by the snapshot, the function spawns up to `hardware_threads` parallel workers. Each worker processes a contiguous chunk of the normalized paths. For each file, it calls `llvm::sys::fs::status` to retrieve its last modification time and `hash_file` to compute a content hash. If `hash_file` fails (e.g., the file was deleted between runs), a hash of zero is recorded to force a change detection on the next check rather than an outright failure.

Error propagation across threads is guarded by a `std::mutex` and an optional `CacheError`; if any worker encounters an error set in `first_error`, all remaining workers quickly exit. Once all threads join, the function checks for that first error and returns `std::unexpected` if present. Otherwise, the results from each `DependencyHashTaskResult` are moved into the snapshot's `files`, `hashes`, and `mtimes` vectors in order. The completed `DependencySnapshot` is returned, containing the sorted list of dependencies along with their computed hashes and modification times.

#### Side Effects

- reads file metadata via `llvm::sys::fs::status`
- reads file contents via `hash_file`
- creates and joins multiple threads

#### Reads From

- input parameter `files` (vector of strings)
- filesystem (via `llvm::sys::fs::status` and `hash_file`)

#### Writes To

- local `DependencySnapshot` object (returned)
- local vectors and results populated by threads

#### Usage Patterns

- capturing current dependency state for cache validation
- used before checking whether cached extraction data is stale

### `clore::extract::cache::dependencies_changed`

Declaration: `src/extract/cache.cppm:102`

Definition: `src/extract/cache.cppm:417`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function first performs basic invariant checks: if `snapshot.build_at` is non-positive or `snapshot.files` is empty, it immediately returns `true`. It also verifies that the lengths of `snapshot.files`, `snapshot.hashes`, and `snapshot.mtimes` match; if they do not, it again returns `true`. For small dependency sets (up to `kParallelThreshold`, 16), it iterates sequentially over each index, calling `check_single_dependency`; the first detection of a change causes an early return of `true`, otherwise returns `false`.

For larger sets, the function parallelizes the work. It determines `num_threads` from `std::thread::hardware_concurrency` (minimum of 1). An `std::atomic<bool> changed` tracks whether any dependency has changed. Each worker thread processes a contiguous range of indices; within its loop it reads `changed` under relaxed memory ordering to allow early exit, and calls `check_single_dependency` for each file. If a change is found, the worker stores `true` in `changed` and returns. Threads are created by partitioning `snapshot.files.size()` across `num_threads` and are joined after all have completed. The function returns the final value of `changed`. The sole dependency for per-file checking is `check_single_dependency`, which encapsulates the logic of verifying a single dependency’s current state against the snapshot record (e.g., mtime or hash comparison).

#### Side Effects

- spawns and joins worker threads via `std::thread`
- performs atomic load and store operations on a local `std::atomic<bool>`

#### Reads From

- `snapshot.build_at`
- `snapshot.files`
- `snapshot.hashes`
- `snapshot.mtimes`

#### Usage Patterns

- called to test whether a cached extract is still valid
- invoked after loading a cached dependency snapshot to detect changes

### `clore::extract::cache::hash_file`

Declaration: `src/extract/cache.cppm:97`

Definition: `src/extract/cache.cppm:286`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function attempts to read the entire file identified by `path` into memory using `llvm::MemoryBuffer::getFile`. If the file cannot be read, it returns `std::unexpected` containing a `CacheError` with a descriptive message that includes the original error. On success, it computes a 64‑bit hash of the file content via `llvm::xxh3_64bits` and returns that value. The control flow is linear: file I/O (via the LLVM utility) followed by a single hash computation; no iteration or branching beyond the error check. Dependencies are limited to `llvm::MemoryBuffer` and `llvm::xxh3_64bits`, with the result wrapped in `std::expected` and error type `CacheError`.

#### Side Effects

- reads a file from disk

#### Reads From

- the file at the given `path`

#### Usage Patterns

- used to compute a content hash for caching decisions
- used by cache validation or key building routines

### `clore::extract::cache::load_clice_cache`

Declaration: `src/extract/cache.cppm:111`

Definition: `src/extract/cache.cppm:686`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function begins by resolving the cache file path using `clice_cache_file_path`. If path resolution fails, it returns the error immediately. Otherwise, it checks whether the cache file exists via `fs::exists`. If the file does not exist (and no system error occurred during the check), it returns an empty `CliceCacheData` to indicate a cache miss. If the existence check itself fails with a system error, that error is returned. When the cache file is present, the function reads its entire contents using `clore::support::read_utf8_text_file`; a read failure also results in a `CacheError`. Finally, the file contents are deserialized from JSON into a `CliceCacheData` object using `json::from_json`. If deserialization fails, a warning is logged via `logging::warn` and an empty `CliceCacheData` is returned, effectively treating the corrupt cache as a miss. On success, the parsed `data` is returned. The control flow is thus a linear sequence of fallible steps, each short‑circuiting on error, with a graceful degradation (empty result) for missing or unparseable cache files.

#### Side Effects

- reads the cache file from disk
- logs a warning via `logging::warn` when JSON parsing fails

#### Reads From

- `workspace_root` parameter
- filesystem state via `fs::exists`
- the cache file content via `clore::support::read_utf8_text_file`

#### Writes To

- logging system (warning message on JSON parse failure)

#### Usage Patterns

- loading clice cache data from a workspace
- initializing clice cache from disk before extraction

### `clore::extract::cache::load_extract_cache`

Declaration: `src/extract/cache.cppm:104`

Definition: `src/extract/cache.cppm:473`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function first resolves the cache file path by calling `cache_file_path(workspace_root)`. If that fails, it returns the error immediately. It then checks for file existence; if the file does not exist it returns an empty map (unless a filesystem error occurs, which is propagated). If the file exists, its content is read via `clore::support::read_utf8_text_file`, and the result is deserialized from JSON into a `SerializedCacheData` structure using `json::from_json`. Deserialization failure or a format version mismatch with `kExtractCacheFormatVersion` causes a warning log and an empty map to be returned, effectively ignoring the stale cache.

For each entry in `data.entries`, the function validates that `entry.source_file` is within the bounds of `data.paths`. It decodes the dependency snapshot by calling `decode_dependency_snapshot(entry.ast_deps, data.paths)`. A normalized path string is produced via `normalize_path_string(data.paths[entry.source_file])`, and a cache key is built using `build_cache_key(normalized, entry.compile_signature)`. A `CacheRecord` is constructed from the entry's fields (`compile_signature`, `source_hash`, the decoded `ast_deps`, `scan`, and `ast`), emplaced into the result map under the generated key. The function returns the populated map on success, or an unexpected error if any entry index or dependency decoding fails.

#### Side Effects

- reads file from disk
- logs warnings for stale or invalid cache

#### Reads From

- `workspace_root`
- file system
- `kExtractCacheFormatVersion`

#### Usage Patterns

- called to load existing cache before extraction
- returns empty map if cache file does not exist

### `clore::extract::cache::save_clice_cache`

Declaration: `src/extract/cache.cppm:113`

Definition: `src/extract/cache.cppm:726`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function first determines the canonical cache file path via `clice_cache_file_path`, creating any missing parent directories. It then serializes the entire `CliceCacheData` into JSON using `json::to_json`. To ensure atomic writes, it generates a unique temporary file path by iterating through candidate names (up to 32 attempts) built from the cache path, process ID, timestamp, and an incremented nonce, checking for existence via `fs::exists`. The JSON content is written to this temporary file using `clore::support::write_utf8_text_file`. Finally, it attempts an atomic rename from the temporary file to the target cache path using `fs::rename`. If the rename fails due to permission or file‑existence errors (common on Windows), the function removes any existing cache file and retries the rename. All intermediate failures (path resolution, serialization, directory creation, temp file allocation, write errors, or final rename) are returned as a `CacheError`.

#### Side Effects

- Creates directories on filesystem
- Writes a temporary file
- Renames file to final path
- May remove existing cache file

#### Reads From

- `workspace_root` parameter
- `data` parameter (`CliceCacheData`)
- filesystem state (via `fs::exists`, `fs::create_directories` errors)
- system clock (for timestamp)
- process ID

#### Writes To

- filesystem directories (parent of cache path)
- temporary file on disk
- final cache file on disk (via rename)
- possibly removes existing cache file

#### Usage Patterns

- Persists compiled cache data to disk
- Called after cache computation to save results
- Used in conjunction with `load_clice_cache` for cache retrieval

### `clore::extract::cache::save_extract_cache`

Declaration: `src/extract/cache.cppm:107`

Definition: `src/extract/cache.cppm:549`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function first obtains the cache root via `cache_directory` and ensures the directory exists. It then builds a `SerializedCacheData` structure by iterating over the input `records` map. For each entry, it normalizes the path using `normalize_path_string` and interns it into a path ID table (`intern_path` lambda) to reduce storage redundancy. The compile signature from the cache key (parsed via `split_cache_key`) is validated against the record’s `compile_signature`; a mismatch causes an early failure. Each record’s `ast_deps` dependency snapshot is encoded into a serialized form via `encode_dependency_snapshot`, and the flattened `SerializedCacheEntry` is appended to the data structure.

After populating the data, the function serializes it to JSON using `json::to_json`. To write the cache atomically, it creates a unique temporary file path (up to 32 attempts with a timestamp, PID, and nonce-based naming scheme) and writes the JSON content via `clore::support::write_utf8_text_file`. The temporary file is then renamed to the final cache path (`cache.json` inside the cache root) using `fs::rename`. If the rename fails with permission or file-exists errors, the function removes the existing cache file and retries. Any intermediate or final error is wrapped in a `CacheError` and returned as an unexpected result. On success, the function returns a void expected.

#### Side Effects

- creates cache directories via `fs::create_directories`
- writes a temporary file via `clore::support::write_utf8_text_file`
- renames temporary file to final cache path via `fs::rename`
- removes existing cache file when rename fails with `permission_denied` or `file_exists`
- checks file existence via `fs::exists`

#### Reads From

- `workspace_root` parameter
- `records` parameter
- `cache_directory` function result
- `split_cache_key` results
- `normalize_path_string` results
- `json::to_json` serialization
- `clore::support::write_utf8_text_file` error
- filesystem state via `fs::exists`, `fs::create_directories`, `fs::rename`, `fs::remove`
- `llvm::sys::Process::getProcessId()`
- `llvm::xxh3_64bits` hash
- `std::chrono::system_clock::now()` timestamp

#### Writes To

- cache directory (created if absent)
- temporary file under cache directory
- final `cache.json` file under cache directory
- existing `cache.json` file (removed on conflict)

#### Usage Patterns

- called after completing extraction to persist cache records
- used to update the extract cache with new data
- invoked with workspace root and collected `CacheRecord` map

### `clore::extract::cache::split_cache_key`

Declaration: `src/extract/cache.cppm:95`

Definition: `src/extract/cache.cppm:254`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The implementation of `split_cache_key` performs a reverse search for the delimiter constant `kCacheKeyDelimiter` within the input `cache_key`. If the delimiter is not found, an error is returned. The key is split into a path prefix and a signature suffix. Both parts must be non‑empty; an empty path or signature triggers an error. The signature portion is then parsed from characters into a `std::uint64_t` value using `std::from_chars`. If parsing fails or unconverted characters remain, an error is produced. On success, the function returns a `CacheKeyParts` structure containing the path as a `std::string` and the parsed `compile_signature`.

All error paths construct a `CacheError` with a descriptive `message` via `std::format` and return it as an unexpected result. The function depends on `CacheKeyParts`, `CacheError`, `kCacheKeyDelimiter`, and the standard library facilities `std::from_chars` and `std::format`. No other internal helpers or I/O are involved; the logic is a direct sequence of string manipulation and numeric conversion with validation at each step.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `cache_key`
- `kCacheKeyDelimiter`

#### Usage Patterns

- called to decompose a cache key into path and signature components for cache lookup or verification

## Internal Structure

The `extract::cache` module provides the caching layer for C++ source extraction, building on the `extract::ast`, `extract::compiler`, and `extract::scan` modules along with the foundational `support` utilities. Internally, it is decomposed into a public API for loading and persisting two related cache formats—a lightweight `extract` cache and a more detailed `clice` cache that records per‑module and per‑PCH entries—alongside functions for generating deterministic cache keys, computing file hashes, and capturing dependency snapshots. The implementation is layered into an anonymous namespace that handles low‑level details: serialization structures (`SerializedCacheData`, `SerializedCacheEntry`, `SerializedDependencySnapshot`), path normalization, atomic file operations (using temporary files and rename), and parallelized dependency checking with a configurable threshold (`kParallelThreshold`). Public types like `CacheRecord`, `CacheError`, `DependencySnapshot`, and the various `CliceCache*Entry` structs define the data contracts, while helper functions such as `encode_dependency_snapshot` and `decode_dependency_snapshot` manage the conversion between in‑memory snapshots and their serialized forms. This layered structure keeps cache‑invariant logic (key construction, snapshot comparison) separate from I/O and serialization, allowing the module to support both synchronous single‑threaded operations and concurrent file‑hash computation via worker threads.

## Related Pages

- [Module extract:ast](ast.md)
- [Module extract:compiler](compiler.md)
- [Module extract:scan](scan.md)
- [Module support](../support/index.md)

