---
title: 'Module extract:cache'
description: 'The extract:cache module is responsible for managing persisted extraction results to avoid redundant re‑parsing of unchanged C++ source files and their dependencies. It provides the data structures and algorithms to represent cached compilation artifacts—such as precompiled modules (PCM), precompiled headers (PCH), and dependency snapshots—along with facilities to construct deterministic cache keys, hash file contents, and capture current dependency state.'
layout: doc
template: doc
---

# Module `extract:cache`

## Summary

The `extract:cache` module is responsible for managing persisted extraction results to avoid redundant re‑parsing of unchanged C++ source files and their dependencies. It provides the data structures and algorithms to represent cached compilation artifacts—such as precompiled modules (PCM), precompiled headers (PCH), and dependency snapshots—along with facilities to construct deterministic cache keys, hash file contents, and capture current dependency state.

The public‑facing scope includes core types like `CacheRecord`, `CacheError`, `CacheKeyParts`, `DependencySnapshot`, and the clice‑compatible entry structs (`CliceCachePCMEntry`, `CliceCachePCHEntry`, `CliceCacheDepEntry`, `CliceCacheData`). Exported functions handle key generation (`build_cache_key`, `build_compile_signature`, `split_cache_key`), file hashing (`hash_file`), dependency tracking (`capture_dependency_snapshot`, `dependencies_changed`), and persistent I/O (`load_extract_cache`, `save_extract_cache`, `load_clice_cache`, `save_clice_cache`). Together, these abstractions allow the extraction pipeline to cache and invalidate results based on file‑content and compilation‑signature changes.

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

The struct `clore::extract::cache::CacheError` is a simple error type that stores a single `std::string message` field. It contains no additional invariants beyond the validity of the contained string; the `message` is expected to hold a human-readable description of the error condition. There are no user‑defined constructors, assignment `operator`s, or other member functions, so the type relies on the default compiler‑generated special member functions. Its role is to serve as a lightweight, self‑contained error representation within the caching subsystem.

#### Invariants

- The `message` member stores a human-readable error description.
- No other fields or invariants are implied by the evidence.

#### Key Members

- `message`

#### Usage Patterns

- Used to represent errors in cache operations.

### `clore::extract::cache::CacheKeyParts`

Declaration: `extract/cache.cppm:24`

Definition: `extract/cache.cppm:24`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The struct `clore::extract::cache::CacheKeyParts` serves as a simple aggregate bundling the identifying components of a cached extraction result. It holds two public data members: `path` – a `std::string` representing the source file path, and `compile_signature` – a `std::uint64_t` acting as a hash or fingerprint of the compilation context. No invariants or special member functions are defined; the struct is a plain aggregate, so its fields are initialised and compared directly. External code typically uses both fields together to form or match a cache key, relying on value semantics and trivial copyability. Because no custom comparison `operator`s are provided, callers must implement their own equality or ordering if needed.

#### Invariants

- The struct is a trivial aggregate; all members are default‑initialized.
- `path` and `compile_signature` together form the complete cache key; no other fields contribute.
- The `compile_signature` is expected to be a non‑negative integer, but no range constraint is enforced by the type.

#### Key Members

- `path` of type `std::string`
- `compile_signature` of type `std::uint64_t`

#### Usage Patterns

- Instances of `CacheKeyParts` are created directly via aggregate initialization.
- The struct is used as a lookup key in cache maps or hash tables for extracted data.
- The two members are combined (e.g., by hashing) to produce a single cache key in cache implementations.

### `clore::extract::cache::CacheRecord`

Declaration: `extract/cache.cppm:36`

Definition: `extract/cache.cppm:36`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The struct `clore::extract::cache::CacheRecord` aggregates the data necessary to represent a single cached extraction result. Its five fields consist of two integrity hashes and three result objects: `compile_signature` and `source_hash` together serve as the unique key that identifies the compilation environment and source content; `ast_deps` records the dependency snapshot taken during extraction; `scan` holds the scan result; and `ast` holds the parsed AST result. An implicit invariant is that `compile_signature` and `source_hash` must both be non‑zero for a valid, populated record, and that the three result fields are coherent with the hashes—meaning `ast_deps`, `scan`, and `ast` were produced from the same source and compilation context. The struct is a plain aggregate with no custom constructors or member functions beyond defaults, relying on the caller to initialize fields in the correct order.

#### Key Members

- `compile_signature`
- `source_hash`
- `ast_deps`
- `scan`
- `ast`

### `clore::extract::cache::CliceCacheData`

Declaration: `extract/cache.cppm:68`

Definition: `extract/cache.cppm:68`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The struct `clore::extract::cache::CliceCacheData` is an internal aggregate that bundles together the three primary arrays of cached extraction data: a vector of file path strings (`paths`), a vector of precompiled header entries (`pch` of type `CliceCachePCHEntry`), and a vector of precompiled module entries (`pcm` of type `CliceCachePCMEntry`). The intended invariant is that the entries in `pch` and `pcm` are indexed into the `paths` vector (or otherwise reference those paths by position), ensuring that path storage is deduplicated and that every cache entry has a corresponding path. The struct itself provides no logic; it serves solely as a flat container for serialization, deserialization, or in-memory storage of the cache state.

#### Invariants

- All vectors are default-constructible.
- Members are publicly accessible and directly modifiable.
- No member functions impose additional constraints.

#### Key Members

- paths
- pch
- pcm

#### Usage Patterns

- Constructed as a simple container for cache entries.
- Likely populated during cache loading and consumed by extraction logic.
- May be serialized or passed between components in the caching subsystem.

### `clore::extract::cache::CliceCacheDepEntry`

Declaration: `extract/cache.cppm:46`

Definition: `extract/cache.cppm:46`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The struct `clore::extract::cache::CliceCacheDepEntry` is a plain‑data type that stores a single cache dependency entry, schema‑compatible with the `CacheData` structure in `clice/src/server/workspace.cpp`. Internally it provides two fields: `path` (`std::uint32_t`) holds an index into an external path table, default‑initialized to `0`; `hash` (`std::uint64_t`) stores the content hash of the corresponding file, default‑initialized to `0`. There are no user‑defined constructors or member functions; the struct relies on aggregate initialization and trivial copy/move semantics. The invariant that the default values produce a valid “no‑entry” sentinel is maintained, and both fields are laid out with natural alignment to support direct serialization.

#### Invariants

- Fields are POD types with zero-initialized defaults
- Struct layout is meant to be compatible with an existing schema

#### Key Members

- `path` (`std::uint32_t`)
- `hash` (`std::uint64_t`)

#### Usage Patterns

- Used as an entry in the clice workspace cache
- Stores an association between a path identifier and a content hash

### `clore::extract::cache::CliceCachePCHEntry`

Declaration: `extract/cache.cppm:51`

Definition: `extract/cache.cppm:51`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The struct aggregates metadata for a single cached precompiled header (PCH) entry. Internally, it stores the PCH `filename` as a `std::string`, a `source_file` index (a `std::uint32_t` referencing an entry in a separate source‑file table), a `hash` (`std::uint64_t`) representing the content digest of the PCH, a `bound` counter (`std::uint32_t`) that tracks how many consumers still require this entry, a `build_at` timestamp (`std::int64_t`) recording when the PCH was built, and a `deps` vector of `CliceCacheDepEntry` objects enumerating the dependencies of the PCH. Key invariants include that the `hash` uniquely identifies the PCH content, that `source_file` is a valid index into the owning cache’s source‑file table, and that `bound` is never negative (and is decremented only when a consumer releases the entry). The members are all default‑initialised (with `source_file` and `hash` zero‑initialised, `bound` zero, `build_at` zero, and `deps` empty), and no custom constructors, assignment `operator`s, or destructors are declared; the struct relies on the compiler‑generated special members for trivial copy and move semantics.

#### Invariants

- hash is an integral value uniquely identifying the PCH content
- `source_file` indexes a source file in the associated source file table
- `build_at` stores a timestamp of when the PCH was built
- bound likely represents a count or limit for dependency tracking
- deps contains zero or more `CliceCacheDepEntry` objects that are valid dependencies

#### Key Members

- filename
- `source_file`
- hash
- bound
- `build_at`
- deps

#### Usage Patterns

- Used as an element in a cache container for PCH entries
- Likely compared or hashed via the `hash` field
- Serialized/deserialized for persistent caching
- Accessed by cache lookup or insertion routines

### `clore::extract::cache::CliceCachePCMEntry`

Declaration: `extract/cache.cppm:60`

Definition: `extract/cache.cppm:60`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The struct `clore::extract::cache::CliceCachePCMEntry` stores metadata for a single cached precompiled module (PCM). Its internal structure consists of the PCM file path (`filename`), a compressed representation of the source file identity (`source_file`, a `std::uint32_t` presumably indexing into a file table), the declared module name (`module_name`), the build timestamp (`build_at`, a `std::int64_t`), and a vector of dependency entries (`deps` of type `std::vector<CliceCacheDepEntry>`). Key invariants include: `source_file` should correspond to a valid entry in the owning cache’s source‑file registry, and `build_at` is expected to represent a monotonic clock value for staleness checks. The entry relies on default member initializers (`source_file = 0`, `build_at = 0`) and implicitly‑defined special member functions; no user‑defined constructors, assignment `operator`s, or destructor are provided, so copy/move operations simply transfer all fields. The `deps` vector’s ordering is not prescribed, but each `CliceCacheDepEntry` must itself satisfy its own invariants (not detailed here).

#### Invariants

- `source_file` and `build_at` are default-initialized to `0`.
- `deps` may be empty; no size constraints are specified.

#### Key Members

- `filename`
- `module_name`
- `source_file`
- `build_at`
- `deps`

#### Usage Patterns

- Used as an element in a cache container for `PCMs`.
- Likely serialized or deserialized for persistence.
- Iterated over or accessed by other cache management functions.

### `clore::extract::cache::DependencySnapshot`

Declaration: `extract/cache.cppm:29`

Definition: `extract/cache.cppm:29`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The struct `clore::extract::cache::DependencySnapshot` is a plain data aggregate that holds a point‑in‑time record of all dependencies for a cache entry. Its three vectors—`files`, `hashes`, and `mtimes`—are maintained in parallel: each index corresponds to one dependency file, storing its path, content hash, and last modification time respectively. The `build_at` member stores the timestamp (as a signed 64‑bit integer) when this snapshot was taken, with a default of `0` typically indicating that the snapshot has not been fully initialized. The key invariant is that `files`, `hashes`, and `mtimes` always have the same length; this property is relied upon by consumers that iterate over the snapshot to compare dependency states.

#### Invariants

- `build_at` defaults to `0`
- No explicit invariants enforced on vector sizes or values

#### Key Members

- `files`
- `hashes`
- `mtimes`
- `build_at`

#### Usage Patterns

- Used to serialize and cache dependency data
- Captures the state of dependencies at a specific build time

## Functions

### `clore::extract::cache::build_cache_key`

Declaration: `extract/cache.cppm:76`

Definition: `extract/cache.cppm:228`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function accepts a normalized file path (`normalized_path`) and a compile-time signature (`compile_signature`), then constructs a single string key by concatenating the path, the delimiter constant `kCacheKeyDelimiter`, and the decimal representation of `compile_signature`. A `reserve` call pre‑allocates storage for the expected key length, avoiding intermediate reallocations during `append` and `push_back`. The resulting key unambiguously pairs a source file with a specific compilation configuration, serving as a uniform identifier for cache lookups across the module system.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `normalized_path` parameter
- `compile_signature` parameter

#### Usage Patterns

- used to generate a cache key before storing or retrieving extract cache data
- called from `save_extract_cache` and `load_extract_cache` related flows

### `clore::extract::cache::build_compile_signature`

Declaration: `extract/cache.cppm:74`

Definition: `extract/cache.cppm:224`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function `clore::extract::cache::build_compile_signature` is a thin forwarding wrapper. It accepts a `const int &` parameter and immediately delegates to `clore::extract::build_compile_signature` to produce the `std::uint64_t` compile signature. This indirection allows the cache module (in namespace `clore::extract::cache`) to expose its own interface without depending directly on the internal compilation-layer implementation details. No additional computation, caching, or error handling is performed within this function.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `entry` parameter of type `const CompileEntry&`

#### Usage Patterns

- Used within the cache system to compute a unique identifier for a compile entry.

### `clore::extract::cache::capture_dependency_snapshot`

Declaration: `extract/cache.cppm:83`

Definition: `extract/cache.cppm:282`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The implementation begins by creating a `DependencySnapshot` and recording the current time as `build_at`. It normalizes each input file path using `normalize_path_string`, sorts the resulting list, and removes duplicates. A per-file result structure `DependencyHashTaskResult` holds the file name, its content hash, last modification time (`mtime`), and an optional error message. The function then distributes the normalized file list across a number of threads determined by `std::thread::hardware_concurrency`, with a chunk size `per_thread`. Each worker thread iterates over its assigned range; before processing each file it checks a mutex‑guarded `first_error` to abort early if a previous error has occurred. For each file it calls `llvm::sys::fs::status` to obtain the modification time and `hash_file` to compute the content hash. If `hash_file` fails (e.g., the file was deleted), the hash is recorded as 0 so that the cache entry is treated as changed on subsequent checks rather than causing a hard failure. After all threads complete, if any error was captured the function returns `std::unexpected` with that error; otherwise the results are moved into `snapshot.files`, `snapshot.hashes`, and `snapshot.mtimes`, and the `DependencySnapshot` is returned. Dependencies include the path normalizer, file hasher, file status query, and standard threading primitives.

#### Side Effects

- Reads file metadata and content via `llvm::sys::fs::status` and `hash_file`
- Creates and synchronizes multiple worker threads using a mutex
- Allocates dynamic memory for intermediate vectors and the result snapshot
- Records the first error in an `std::optional<CacheError>` and stops further work on that error

#### Reads From

- Input parameter `files` (a `std::vector<std::string>` of file paths)
- File system state via `llvm::sys::fs::status` and `hash_file`
- System clock via `std::chrono::system_clock::now()`
- Hardware concurrency via `std::thread::hardware_concurrency()`
- Local variables: `normalized`, `task_results`, `first_error`

#### Writes To

- Returned `std::expected<DependencySnapshot, CacheError>` object
- Fields of the `DependencySnapshot`: `build_at`, `files`, `hashes`, `mtimes`
- Mutex-protected `first_error` optional if an error occurs
- Local variables: `normalized`, `task_results`, `first_error`, `snapshot`

#### Usage Patterns

- Used to snapshot file dependencies before cache insertion or validation
- Called in conjunction with `dependencies_changed` to determine whether cached data is stale
- Supports cache invalidation logic in the extract pipeline by computing file hashes and modification times

### `clore::extract::cache::dependencies_changed`

Declaration: `extract/cache.cppm:86`

Definition: `extract/cache.cppm:401`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function first validates the incoming `snapshot`: if `snapshot.files` is empty or `snapshot.build_at` is non‑positive, or if the sizes of `snapshot.files`, `snapshot.hashes`, and `snapshot.mtimes` are inconsistent, it returns `true` (dependencies have changed). For small dependency sets (at most `kParallelThreshold`, defined as 16), it iterates sequentially over `snapshot.files`, calling `check_single_dependency` on each index; if any call returns `true` the function immediately returns `true`. For larger sets, the work is parallelised: it determines the number of worker threads from `std::thread::hardware_concurrency`, creates that many `std::thread` objects, and divides the index range equally among them. Each worker reads from a shared `std::atomic<bool> changed` (initialised to `false`) and, for its assigned indices, calls `check_single_dependency`. If a worker detects a change, it stores `true` in `changed` using relaxed memory order and returns early. All other workers poll the flag at the top of each iteration and exit if it has been set. After joining all threads, the function returns the final value of `changed`. The only external helper used is `check_single_dependency`, which encapsulates the filesystem check for a single dependency entry.

#### Side Effects

- Creates and joins multiple `std::thread` objects for parallel execution
- Uses `std::atomic<bool>` with relaxed memory ordering for inter-thread coordination

#### Reads From

- `snapshot.build_at`
- `snapshot.files`
- `snapshot.hashes`
- `snapshot.mtimes`
- `std::thread::hardware_concurrency()`
- `check_single_dependency(snapshot, index)` (reads file metadata or contents, but exact reads are not specified in evidence)

#### Usage Patterns

- Called during cache validation to determine if a cached extraction is still valid
- Used in conjunction with `capture_dependency_snapshot` and `load_extract_cache`/`save_extract_cache` to manage cache freshness
- Employed in places where an up-to-date dependency check is needed before using cached data

### `clore::extract::cache::hash_file`

Declaration: `extract/cache.cppm:81`

Definition: `extract/cache.cppm:270`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function reads the file at the given `path` into a memory buffer using `llvm::MemoryBuffer::getFile`. If the file cannot be read, it returns a `std::unexpected` containing a `CacheError` with a formatted message including the error from LLVM. On success, it passes the buffer's contents to `llvm::xxh3_64bits` and returns the resulting 64‑bit hash. The implementation depends on LLVM’s file I/O (`llvm::MemoryBuffer`) and the `xxHash`‑3 algorithm (`llvm::xxh3_64bits`), and uses `std::uint64_t` for the hash value.

#### Side Effects

- reads a file from the filesystem
- allocates memory for the file buffer

#### Reads From

- the `path` parameter identifying the file
- the file contents on disk

#### Usage Patterns

- hashing source files for cache key generation
- computing file hashes for change detection in the caching system

### `clore::extract::cache::load_clice_cache`

Declaration: `extract/cache.cppm:95`

Definition: `extract/cache.cppm:670`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function first computes the expected cache file path by calling `clice_cache_file_path`. If that call fails, the error is forwarded immediately. Otherwise, it checks whether the file exists using `fs::exists`; if an error occurs during the existence check, a `CacheError` is returned. If the file does not exist, an empty `CliceCacheData` is returned, indicating no prior cache.

When the file is present, the function reads its entire contents as UTF‑8 text via `clore::support::read_utf8_text_file`. A read failure also produces a `CacheError` with a descriptive message. On successful reading, the content is deserialized from JSON into a `CliceCacheData` object using `json::from_json`. If JSON parsing fails, the function logs a warning and returns an empty `CliceCacheData` (treating the cache as stale). Otherwise, the populated `CliceCacheData` is returned.

#### Side Effects

- checks file existence on the filesystem
- reads the entire cache file from the filesystem

#### Reads From

- `workspace_root` parameter
- the cache file at path derived from `workspace_root`

#### Usage Patterns

- call to load previously cached clice extraction data
- used before performing extraction to check for valid cache
- paired with `save_clice_cache` to persist cache data

### `clore::extract::cache::load_extract_cache`

Declaration: `extract/cache.cppm:88`

Definition: `extract/cache.cppm:457`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function resolves the extract cache file path via `cache_file_path` and returns an empty map if the file does not exist. On a read failure or JSON parsing error, it returns an error or logs a warning and returns empty, respectively. After validating the `format_version` against `kExtractCacheFormatVersion`, it iterates over each `SerializedCacheEntry` in `data.entries`. For each entry it validates the `source_file` index within `data.paths`, decodes the dependency snapshot with `decode_dependency_snapshot`, normalizes the source path via `normalize_path_string`, and constructs a cache key using `build_cache_key` from the normalized path and `compile_signature`. The resulting `CacheRecord` — holding `compile_signature`, `source_hash`, decoded `ast_deps`, `scan`, and `ast` — is emplaced into the output map. The function ultimately returns a mapping from cache keys to `CacheRecord`.

#### Side Effects

- reads a cache file from the filesystem
- logs warnings via logging subsystem for stale or malformed caches
- allocates and returns a new `unordered_map` with heap-allocated strings and `CacheRecords`

#### Reads From

- `workspace_root` parameter
- cache file path derived from `workspace_root` via `cache_file_path`
- file system state (existence and content of cache file)
- constant `kExtractCacheFormatVersion`
- results of `clore::support::read_utf8_text_file`, `json::from_json`, `decode_dependency_snapshot`, `build_cache_key`

#### Writes To

- logging output (via `logging::warn`)
- returned map (ownership transferred to caller)

#### Usage Patterns

- called to initialize or refresh the extract cache before performing extraction
- used in conjunction with `save_extract_cache` for cache persistence
- invoked to reuse previously cached extraction results across build sessions

### `clore::extract::cache::save_clice_cache`

Declaration: `extract/cache.cppm:97`

Definition: `extract/cache.cppm:710`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The implementation of `clore::extract::cache::save_clice_cache` begins by resolving the target cache file path via `clice_cache_file_path`, ensuring its parent directory exists with `fs::create_directories`. The provided `CliceCacheData` is serialized to JSON through `json::to_json`, and any serialization failure is immediately converted to a `CacheError`. To guarantee an atomic replacement, a unique temporary file path is generated inside a retry loop (up to 32 attempts) using a lambda that combines the process ID, a high‑resolution timestamp, and a deterministic nonce produced by `llvm::xxh3_64bits` over a formatted string. The loop checks `fs::exists` for the candidate path and breaks as soon as a non‑existing path is found. The serialized content is written to this temporary file via `clore::support::write_utf8_text_file`. Finally, `fs::rename` moves the temporary file to the final cache path; if the rename fails with permission/conflict errors (`permission_denied`, `file_exists`, or `operation_not_permitted`), the existing target is removed with `fs::remove` and the rename is retried once. Any other rename error, or failure at any prior step, is wrapped in a `CacheError` and returned. If all steps succeed, an empty `expected` with `void` value is returned.

#### Side Effects

- creates directories on the filesystem if missing
- writes a temporary JSON file to a temporary path
- renames the temporary file to the final cache path
- may remove an existing cache file during replacement

#### Reads From

- `workspace_root` parameter determining the cache directory
- data parameter as the payload to serialize
- filesystem to check existence of candidate temporary file paths

#### Writes To

- filesystem temporary file path (near the final cache path)
- filesystem at the final clice cache path
- filesystem parent directory of the cache path (if created)

#### Usage Patterns

- Called after extracting build data to persist the `CliceCacheData` for later reuse
- Used with `workspace_root` identifying the project root

### `clore::extract::cache::save_extract_cache`

Declaration: `extract/cache.cppm:91`

Definition: `extract/cache.cppm:533`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function first resolves the cache directory via `clore::extract::cache::(anonymous namespace)::cache_directory` and ensures it exists. It then builds a `SerializedCacheData` by iterating over the provided `records`. For each record, it validates the `compile_signature` against the corresponding key part obtained from `clore::extract::cache::split_cache_key`; if mismatched, an error is returned. The path associated with the cache key is normalized through `clore::extract::cache::(anonymous namespace)::normalize_path_string` and interned into a `path_ids` mapping, producing an index into `SerializedCacheData::paths`. The dependency snapshot is encoded via `clore::extract::cache::(anonymous namespace)::encode_dependency_snapshot` using the same path‑id mapping, and a `SerializedCacheEntry` is appended.

After all entries are prepared, the data structure is serialized to JSON using `clore::extract::cache::json::to_json`. A temporary file path is generated near `cache_path` (the target file) by iterating up to 32 attempts, each based on a hash of the cache path, process ID, timestamp, and attempt number; the loop ensures the candidate does not already exist. The JSON content is written to this temporary file via `clore::support::write_utf8_text_file`. Finally, an atomic rename (`fs::rename`) replaces the target cache file with the temporary file. If the rename fails due to permission or file‑existence issues, the function attempts to remove the existing target before retrying the rename. All file‑system operations propagate errors through `CacheError`. The resulting behavior ensures a consistent, corruption‑resistant update of the extract cache while verifying signature integrity and handling concurrent write contention.

#### Side Effects

- Creates cache directories via `fs::create_directories`
- Reads filesystem to check existence of temporary paths via `fs::exists`
- Writes JSON content to a temporary file via `clore::support::write_utf8_text_file`
- Renames the temporary file to `cache.json` via `fs::rename`
- Potentially removes the existing `cache.json` via `fs::remove` on rename failure

#### Reads From

- Parameter `workspace_root`
- Parameter `records`
- Return value of `cache_directory`
- Return value of `split_cache_key`
- `CacheRecord` fields from the records map
- Return value of `fs::exists` for temp path collision detection

#### Writes To

- Cache directory determined by `cache_directory`
- Temporary file in the same directory
- Final `cache.json` file after rename

#### Usage Patterns

- Used to persist extract cache data after extraction
- Called to save a snapshot of cache records to disk for later reloading

### `clore::extract::cache::split_cache_key`

Declaration: `extract/cache.cppm:79`

Definition: `extract/cache.cppm:238`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function `clore::extract::cache::split_cache_key` reverses the encoding performed by `build_cache_key`. It receives a `std::string_view` and attempts to separate it into a path component and a 64‑bit compile signature. The algorithm first locates the last occurrence of the constant `kCacheKeyDelimiter` using `rfind`. If the delimiter is absent, it returns `std::unexpected` with a descriptive `CacheError`. Otherwise it splits the key into a `path_part` (up to the delimiter) and a `signature_part` (after the delimiter). Both parts must be non-empty; an empty part also yields an error.

The signature portion is parsed into a `std::uint64_t` via `std::from_chars`. The function verifies that conversion consumed the entire substring and succeeded without error. If the parse fails or leaves extra characters, it returns `std::unexpected` with an appropriate `CacheError`. On success it assembles a `CacheKeyParts` value containing the extracted path as a `std::string` and the parsed signature. The function depends on the constant `kCacheKeyDelimiter`, the error type `CacheError`, and the result type `CacheKeyParts`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `cache_key` (`std::string_view`)
- `kCacheKeyDelimiter`

#### Usage Patterns

- Used to decompose cache keys built by `build_cache_key`
- Called during cache lookups to extract the path and signature from a composite key
- Validates cache key format before further processing

## Internal Structure

The `extract:cache` module is responsible for persisting and retrieving extraction results to avoid redundant work. It is decomposed into a public API for saving and loading cache entries, and internal helpers that handle file I/O, serialization, and dependency tracking. The module imports `extract:ast`, `extract:compiler`, and `extract:scan` to cache the outputs of these stages, and depends on `support` for utilities like path normalization and hashing.

Internally, the module defines a serialization layer with structs such as `SerializedCacheData` and `SerializedDependencySnapshot`, and helper functions to encode and decode them. A file‑path resolution layer uses the workspace root to compute cache directory and file paths. The public interface offers functions like `load_extract_cache` and `save_extract_cache` for general extraction results, and `load_clice_cache` / `save_clice_cache` for Clice‑specific PCM/ PCH data. Dependency snapshots capture file hashes and modification times; `dependencies_changed` checks if any file has changed since the snapshot was taken. Cache keys are built from a source identifier and a compile signature, and can be split into parts via `split_cache_key`.

## Related Pages

- [Module extract:ast](ast.md)
- [Module extract:compiler](compiler.md)
- [Module extract:scan](scan.md)
- [Module support](../support/index.md)

