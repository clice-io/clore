---
title: 'Module generate:cache'
description: 'The clore::generate::cache module owns the caching subsystem for generated LLM responses, providing a persistent store that maps prompt–response pairs to deterministic keys. It exposes a public interface for saving and loading cache entries synchronously (save_cache_entry, load_cache_index, find_cached_response) and asynchronously (save_cache_entry_async, load_cache_index_async). Cache keys are constructed from normalized text via make_prompt_response_cache_key, which uses normalize_text_for_hashing to produce repeatable representations of semantically equivalent inputs.'
layout: doc
template: doc
---

# Module `generate:cache`

## Summary

The `clore::generate::cache` module owns the caching subsystem for generated LLM responses, providing a persistent store that maps prompt–response pairs to deterministic keys. It exposes a public interface for saving and loading cache entries synchronously (`save_cache_entry`, `load_cache_index`, `find_cached_response`) and asynchronously (`save_cache_entry_async`, `load_cache_index_async`). Cache keys are constructed from normalized text via `make_prompt_response_cache_key`, which uses `normalize_text_for_hashing` to produce repeatable representations of semantically equivalent inputs.

The module stores its data in JSONL files within a cache directory, maintaining an in-memory index (`CacheIndex`) of entries for efficient lookups. It handles file I/O, directory creation, and concurrency with mutex‑based locking, returning `CacheError` values for any failures. Internally it uses helpers such as `all_jsonl_files`, `cache_directory`, `format_iso_timestamp`, and `escape_json_string` to manage the persistent representation. The module depends on the `protocol` and `support` modules for its data types and foundational utilities.

## Imports

- [`protocol`](../protocol/index.md)
- `std`
- [`support`](../support/index.md)

## Imported By

- [`generate:scheduler`](scheduler.md)

## Types

### `clore::generate::cache::CacheError`

Declaration: `generate/cache.cppm:16`

Definition: `generate/cache.cppm:16`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The struct `clore::generate::cache::CacheError` is defined as an aggregate type containing a single field `message` of type `std::string`. No user‑declared constructors, destructors, or assignment `operator`s are provided; the default memberwise initialization and copy/move semantics apply. The only invariant is that `message` holds a valid `std::string` instance (including the empty string). Since the struct is defined within a C++20 module, its name is subject to module linkage, and no additional member functions or friend declarations are present.

#### Invariants

- No invariants beyond those of `std::string`.

#### Key Members

- message

#### Usage Patterns

- Defined as a simple error type for caching operations.

### `clore::generate::cache::CacheIndex`

Declaration: `generate/cache.cppm:20`

Definition: `generate/cache.cppm:20`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The struct `clore::generate::cache::CacheIndex` is a thin wrapper around a single member, a `std::unordered_map<std::string, std::string>` named `entries`. The map serves as the entire internal storage of the index, associating string keys with string values. The primary invariant is the same as that of the underlying unordered map: each key is unique within the map, and the hash and equality operations on the keys are determined by the default standard library specializations for `std::string`. The struct itself has no custom constructors, assignment `operator`s, or other member functions, so all behavioral guarantees—such as the average constant-time lookup, insertion, and erasure—are inherited directly from the `std::unordered_map` implementation. The absence of additional members or invariants makes `CacheIndex` a straightforward, data-only point of use for clients that require an unordered mapping of strings to strings within the cache subsystem.

#### Invariants

- Keys in `entries` are unique by definition of `std::unordered_map`

#### Key Members

- `entries`

#### Usage Patterns

- Instantiated and populated with key-value pairs representing cached index data
- Accessed directly via its public member to insert, look up, or iterate over cache entries
- Likely used as a building block within a larger cache manager or cache file representation

## Functions

### `clore::generate::cache::find_cached_response`

Declaration: `generate/cache.cppm:35`

Definition: `generate/cache.cppm:347`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The `find_cached_response` function performs a straightforward lookup in the provided `CacheIndex`. It converts the input `cache_key` from `std::string_view` to a `std::string` and uses it as the key in the `index.entries` map (which is likely a `std::unordered_map` or similar associative container). If the key is not found via `std::map::find`, the function returns `std::nullopt`; otherwise it returns the stored `std::string_view` value associated with that key. This implementation has no branching beyond the existence check and no external dependencies beyond the `CacheIndex` type and standard library containers.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `index.entries` (the internal map of the `CacheIndex`)
- `cache_key` parameter

#### Usage Patterns

- checking for an existing cached response before generating a new one
- lookup by cache key in a `CacheIndex`

### `clore::generate::cache::load_cache_index`

Declaration: `generate/cache.cppm:29`

Definition: `generate/cache.cppm:252`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The function begins by calling `all_jsonl_files(workspace_root)` to obtain a list of pathnames for every `.jsonl` file under the given workspace directory. If that helper fails, the error is immediately forwarded as the return value. For each successfully retrieved file, `load_cache_index` reads the entire file content through `clore::support::read_utf8_text_file`; if the read attempt itself fails, that file is silently skipped. The content is then split line by line, and empty lines are discarded. Each non‑empty line is parsed as a `kota::codec::json::Object`; lines that fail to parse are also skipped without terminating the loop. For a successfully parsed object, the implementation looks for a `"key"` string field and a `"resp"` string field: if either is missing or empty, that entry is ignored. Otherwise, the key‑value pair is inserted into `index.entries`, with the key serving as the map key and the response as the associated value. After all files have been processed, the populated `CacheIndex` is returned. The only dependency that can produce an early `std::unexpected` is `all_jsonl_files`; all other errors are recovered by simply continuing to the next candidate entry.

#### Side Effects

- reads multiple files from the filesystem
- allocates memory for `CacheIndex::entries` and contained strings

#### Reads From

- `workspace_root` parameter
- filesystem files matching `*.jsonl` under `workspace_root`
- contents of those files as UTF-8 text
- the `"key"` and `"resp"` JSON fields from each non-empty line

#### Writes To

- constructs and returns a `CacheIndex` value
- populates `CacheIndex::entries` with key-response pairs

#### Usage Patterns

- cache index initialization on application startup
- reloading cache index from disk
- building a lookup structure for quickly retrieving cached responses by key

### `clore::generate::cache::load_cache_index_async`

Declaration: `generate/cache.cppm:38`

Definition: `generate/cache.cppm:356`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The implementation of `load_cache_index_async` is a coroutine that delegates the synchronous loading work to the event loop. It calls `kota::queue` with a lambda that invokes `clore::generate::cache::load_cache_index` on the given `workspace_root`, and then awaits the resulting queued task with cancellation support. If the queuing itself fails (e.g., due to cancellation), it constructs a `CacheError` with a descriptive message and fails via `kota::fail`. Otherwise, it inspects the inner `std::expected<CacheIndex, CacheError>` returned from `load_cache_index`: if that expected result holds an error, the coroutine fails with the moved error object; if it holds a value, the coroutine co-returns that `CacheIndex`. This pattern cleanly offloads the blocking I/O‑bound index loading to the event loop’s thread pool while preserving error propagation through the asynchronous task chain.

#### Side Effects

- reads cache index from filesystem via `load_cache_index`
- schedules work on the provided event loop
- may propagate a `CacheError` through `kota::fail`

#### Reads From

- the `workspace_root` parameter
- the `loop` parameter (for scheduling)
- the cache index file (indirectly through `load_cache_index`)
- the error state of the queued result

#### Usage Patterns

- asynchronous cache index loading before response caching
- non‑blocking initialization in event‑loop driven applications
- part of the cache layer that integrates with `save_cache_entry_async`

### `clore::generate::cache::make_prompt_response_cache_key`

Declaration: `generate/cache.cppm:24`

Definition: `generate/cache.cppm:219`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The function constructs a deterministic cache key by concatenating several components separated by tab characters. The `request_key` is taken as given, while both the `request.prompt` and `system_prompt` are first normalized via `normalize_text_for_hashing` and then hashed with `llvm::xxh3_64bits` to produce 64‑bit integer strings. The `response_format_fingerprint` and `tool_choice_fingerprint` helpers each yield a fingerprint string; the former may fail, and that error is propagated immediately by returning `std::unexpected`. Finally a single character derived from `request.output_contract` is appended. The result string is built with a reserved capacity sufficient for the known components.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `request_key` parameter (string view)
- `system_prompt` parameter (string view)
- `request.prompt` (string view)
- `request.response_format` (some type, read for fingerprint)
- `request.tool_choice` (some type, read for fingerprint)
- `request.output_contract` (integral, converted to char)

#### Writes To

- The returned `std::string` (heap allocation and construction)

#### Usage Patterns

- Used before `clore::generate::cache::find_cached_response` to generate key for lookup
- Used before `clore::generate::cache::save_cache_entry` to generate key for storing
- Employed by asynchronous cache operations such as `save_cache_entry_async` when constructing keys

### `clore::generate::cache::normalize_text_for_hashing`

Declaration: `generate/cache.cppm:192`

Definition: `generate/cache.cppm:192`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

Implementation: [Implementation](functions/normalize-text-for-hashing.md)

The implementation of `clore::generate::cache::normalize_text_for_hashing` applies a two‑pass normalization to an arbitrary input `text` to produce a canonical form suitable for hashing in cache‑key construction. The first pass trims all leading whitespace characters using `std::isspace`. The second pass iterates over the remaining characters and collapses any contiguous run of whitespace into a single ASCII space character (`' '`). The internal control flow uses a boolean flag `prev_space` to track whether the previous character was whitespace; when a non‑space character is encountered and `prev_space` is true, a single space is appended to the result `std::string` only if the result is not empty. The function depends only on the C++ standard library, specifically the `<cctype>` facilities via `std::isspace`, and uses `result.reserve(text.size())` to minimise reallocations. This normalisation ensures that differing amounts of whitespace do not produce distinct hash keys, while preserving the distinction between words separated by any amount of whitespace versus no whitespace.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `text` parameter

#### Usage Patterns

- Used by `make_prompt_response_cache_key` to normalize text before forming a cache key

### `clore::generate::cache::save_cache_entry`

Declaration: `generate/cache.cppm:31`

Definition: `generate/cache.cppm:303`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The function `clore::generate::cache::save_cache_entry` serializes a new cache entry by appending a single JSONL line to the current daily cache file. It first acquires a static `std::mutex` named `cache_file_mutex` to ensure exclusive access. The directory path is resolved by calling `cache_directory` on `workspace_root`; if that fails, the error is forwarded. The directory is then created if absent using `fs::create_directories`, with any creation failure reported as a `CacheError`. The target file path is formed by concatenating the directory with `current_jsonl_filename()`. A JSONL line is built from `cache_key` and `response` via the helper `build_jsonl_line`. The file is opened in binary append mode and, if successful, the line is written and flushed. Write errors are captured and returned as a `CacheError`. On success the function returns an empty `std::expected<void>`. All intermediate operations check for failure and propagate errors as `std::unexpected(CacheError{...})`.

#### Side Effects

- acquires a static mutex for synchronization
- creates directories under `workspace_root` if they do not exist
- appends a JSONL line to the cache file in the cache directory

#### Reads From

- `workspace_root` parameter
- `cache_key` parameter
- `response` parameter
- filesystem through `cache_directory` and `current_jsonl_filename`
- result of `build_jsonl_line` call

#### Writes To

- filesystem cache directory (created if missing)
- JSONL cache file in that directory (appended)
- static mutex lock state

#### Usage Patterns

- persist a generated response in the cache after completion of a generation request
- synchronous alternative to `save_cache_entry_async` for environments where async is not desired

### `clore::generate::cache::save_cache_entry_async`

Declaration: `generate/cache.cppm:41`

Definition: `generate/cache.cppm:376`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

This function is a coroutine that offloads the synchronous `clore::generate::cache::save_cache_entry` onto the provided `kota::event_loop` via `kota::queue`, then awaits the result. The captured arguments (`workspace_root`, `cache_key`, `response`) are moved into the lambda to avoid copies. If the queue operation is cancelled, `catch_cancel` returns an error; the function then fails with a `CacheError` containing a descriptive message. If the inner `save_cache_entry` call returned a `std::expected` containing an error, that error is forwarded by failing with it. On success, the coroutine completes normally. The control flow is linear: queue the work, check for cancellation, check the expected result, and either re-raise the error or `co_return`. The primary dependency is the blocking `save_cache_entry` (which performs filesystem I/O and JSONL appending), the `kota::queue` asynchronous dispatch mechanism, and the `CacheError` type used for error propagation.

#### Side Effects

- Initiates a write to the cache storage via the synchronous `save_cache_entry`
- Potentially modifies the state of the event loop's task queue

#### Reads From

- Parameter `std::string workspace_root`
- Parameter `std::string cache_key`
- Parameter `std::string response`
- Parameter `kota::event_loop& loop`
- Result of `save_cache_entry(workspace_root, cache_key, response)`

#### Writes To

- Cache storage (via `save_cache_entry`)

#### Usage Patterns

- Used to asynchronously persist a generated response into a cache indexed by key and workspace
- Called when a cache miss occurs and a new response must be stored without blocking the caller
- Part of the `clore::generate::cache` module's asynchronous API

## Internal Structure

The `generate:cache` module is decomposed into a public API surface and a set of anonymous-namespace helpers. The public API exposes cache operations that are either synchronous (`load_cache_index`, `save_cache_entry`) or asynchronous (`load_cache_index_async`, `save_cache_entry_async`) using a `kota::event_loop`, alongside utility functions for constructing deterministic cache keys (`make_prompt_response_cache_key`) and normalizing text for hashing (`normalize_text_for_hashing`). The module imports `protocol` (for completion‑related types such as tool choice and response format), `std` (for standard library types and filesystem support), and `support` (for foundational utilities like logging, path normalization, and transparent hash lookups).

Internally, the cache is organized as a set of JSONL files in a workspace‑rooted cache directory. A `CacheIndex` struct holds parsed entries, while helpers such as `cache_directory`, `all_jsonl_files`, and `current_jsonl_filename` manage file discovery and creation on disk. Key construction relies on fingerprinting functions (`tool_choice_fingerprint`, `response_format_fingerprint`) and text normalization (`normalize_text_for_hashing`), which produce stable hashes from prompt, system prompt, tool choice, and response format components. Write operations are protected by a `cache_file_mutex` and use `build_jsonl_line` to format entries; `format_iso_timestamp` provides timestamps for cache‑file naming. This layering separates the public cache contract from filesystem details and fingerprint generation, enabling both synchronous and asynchronous access while keeping the core logic independent of any particular I/O strategy.

## Related Pages

- [Module protocol](../protocol/index.md)
- [Module support](../support/index.md)

