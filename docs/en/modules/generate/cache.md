---
title: 'Module generate:cache'
description: 'The generate:cache module provides a caching layer for LLM prompt–response pairs. It is responsible for constructing deterministic cache keys from normalized prompt and response text, storing entries in JSONL files alongside an in‑memory index, and performing both synchronous and asynchronous lookups and saves. The public interface includes functions such as make_prompt_response_cache_key, normalize_text_for_hashing, save_cache_entry, save_cache_entry_async, load_cache_index, load_cache_index_async, and find_cached_response, together with the CacheIndex and CacheError types that clients use to interact with the cache.'
layout: doc
template: doc
---

# Module `generate:cache`

## Summary

The `generate:cache` module provides a caching layer for LLM prompt–response pairs. It is responsible for constructing deterministic cache keys from normalized prompt and response text, storing entries in JSONL files alongside an in‑memory index, and performing both synchronous and asynchronous lookups and saves. The public interface includes functions such as `make_prompt_response_cache_key`, `normalize_text_for_hashing`, `save_cache_entry`, `save_cache_entry_async`, `load_cache_index`, `load_cache_index_async`, and `find_cached_response`, together with the `CacheIndex` and `CacheError` types that clients use to interact with the cache.

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

The struct `clore::generate::cache::CacheError` is a lightweight error type that stores a descriptive `std::string` in its only data member `message`. Internally, it relies solely on the invariants of `std::string`—the string can be empty (indicating no detail) or contain an error explanation. No additional constructors, assignment `operator`s, or member functions are defined beyond the implicitly generated ones, making `CacheError` a trivially copyable and movable aggregate. Its design serves as a minimal, self-contained error representation for cache operations, providing a single point of failure information without any internal state management or resource ownership beyond the string itself.

#### Invariants

- `message` is a valid `std::string` (may be empty if no error details are provided)

#### Key Members

- `message`

#### Usage Patterns

- Returned or thrown to indicate cache operation failures
- Inspected by callers to retrieve error details

### `clore::generate::cache::CacheIndex`

Declaration: `generate/cache.cppm:20`

Definition: `generate/cache.cppm:20`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The struct `clore::generate::cache::CacheIndex` is a thin wrapper around a single data member: a `std::unordered_map<std::string, std::string>` named `entries`. Its role is to serve as a simple, standalone index mapping string keys to string values. Because the struct contains no additional logic or constraints, the only invariants are those inherited from the underlying unordered map—namely, that keys are unique (per the map’s semantics) and that both keys and values are valid `std::string` objects. No special member functions are defined, so the compiler-generated default constructor, copy/move operations, and destructor apply, preserving the map’s usual behavior. This minimal internal structure makes `CacheIndex` a straightforward value type whose entire state resides in the `entries` map.

#### Invariants

- The map is unordered; iteration order is unspecified.
- Keys and values are strings.

#### Key Members

- `entries`

#### Usage Patterns

- Used as a lightweight index for caching or lookup purposes.

## Functions

### `clore::generate::cache::find_cached_response`

Declaration: `generate/cache.cppm:35`

Definition: `generate/cache.cppm:347`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The function performs a synchronous lookup of a cached response by converting the provided `cache_key` to a `std::string` and searching the `index.entries` map (an associative container, likely `std::unordered_map`). If the key is found, the associated value is returned as an `std::optional<std::string_view>`, otherwise `std::nullopt` is returned. The algorithm is a straightforward map lookup with no file I/O or external dependencies beyond the `CacheIndex` structure, making it a lightweight query that relies on the index having been previously loaded.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `index.entries` (the cache index map)
- `cache_key` (the lookup key)

#### Usage Patterns

- Check existence of cached response before generation
- Retrieve cached response by key from index

### `clore::generate::cache::load_cache_index`

Declaration: `generate/cache.cppm:29`

Definition: `generate/cache.cppm:252`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The function `clore::generate::cache::load_cache_index` implements a cache index loader that reconstructs the in‑memory `CacheIndex` from persisted JSONL files. It first delegates to the helper `all_jsonl_files` to enumerate all JSONL files in the given `workspace_root`. If that enumeration fails, the error is propagated immediately via `std::unexpected`. For each file path returned, it reads the entire file content using `clore::support::read_utf8_text_file`; if reading fails for any file, that file is silently skipped.

The actual index construction iterates line by line through the file content using a `std::istringstream`. Empty lines are ignored. Each non‑empty line is parsed as a JSON object via `kota::codec::json::parse`. If parsing fails, the line is skipped. For a successful parse, the function extracts the `"key"` and `"resp"` fields as strings. If either field is missing or has an empty key, the line is silently ignored. Otherwise, it inserts a mapping from the key string to the response string into `index.entries` using `insert_or_assign`. This process repeats for all files, accumulating all cache entries. The final populated `CacheIndex` is returned on success. The function depends on `all_jsonl_files` for file discovery, `clore::support::read_utf8_text_file` for file I/O, `kota::codec::json::parse` for JSON deserialization, and the `CacheIndex` aggregate type.

#### Side Effects

- reads files from the file system
- allocates memory for cache entries

#### Reads From

- `workspace_root` parameter
- JSONL files on disk

#### Usage Patterns

- called to load cache index from disk
- used at initialization to populate in-memory cache index

### `clore::generate::cache::load_cache_index_async`

Declaration: `generate/cache.cppm:38`

Definition: `generate/cache.cppm:356`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The function `clore::generate::cache::load_cache_index_async` implements an asynchronous coroutine that wraps the synchronous `load_cache_index` by offloading its execution to a background thread via `kota::queue`. It accepts a `workspace_root` string and a `kota::event_loop` reference. Inside the coroutine, a lambda captures `workspace_root` by move and calls `load_cache_index` on it; the lambda is then queued on the provided `loop`. The resulting `kota::task` is `co_awaited`, and the returned `queued_result` is inspected for errors at two levels. If the queue task itself failed (for example due to cancellation), a new `CacheError` is produced with a descriptive message. Otherwise, if the inner `std::expected` contains an error, that error is forwarded via `kota::fail`. On success, the contained `CacheIndex` value is moved out with `co_return`. This design ensures that the blocking file‑system work of `load_cache_index` does not stall the event loop, while error propagation remains uniform through `CacheError`.

#### Side Effects

- reads cache index file from disk

#### Reads From

- `workspace_root` parameter
- filesystem cache index file

#### Usage Patterns

- asynchronous loading of cache index
- wrapping synchronous `load_cache_index` into a coroutine

### `clore::generate::cache::make_prompt_response_cache_key`

Declaration: `generate/cache.cppm:24`

Definition: `generate/cache.cppm:219`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The function computes a deterministic cache key by concatenating the `request_key` with hashed and fingerprinted representations of the request’s semantic components. It first obtains a fingerprint for the response format via `response_format_fingerprint`, immediately propagating any error as an unexpected `CacheError`. The prompt and system prompt are normalized using `normalize_text_for_hashing` and then hashed with `llvm::xxh3_64bits` to produce fixed-size digests. A separate `tool_choice_fingerprint` is computed for the tool‑choice field. The final key is assembled by appending each hash and fingerprint, separated by tab characters, along with a single‑character encoding of the `output_contract` flag. The key is built in a pre‑reserved string to minimize reallocations. This design ensures that structurally equivalent requests (after normalization) always map to the same cache key, while variations in any tracked property produce distinct keys.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `request_key`
- `system_prompt`
- `request.prompt`
- `request.response_format`
- `request.tool_choice`
- `request.output_contract`
- `normalize_text_for_hashing`

#### Usage Patterns

- Used by `find_cached_response` and `save_cache_entry` to generate a unique key for cache operations.

### `clore::generate::cache::normalize_text_for_hashing`

Declaration: `generate/cache.cppm:192`

Definition: `generate/cache.cppm:192`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

Implementation: [Implementation](functions/normalize-text-for-hashing.md)

The function first trims leading whitespace by advancing a `start` index past any characters for which `std::isspace` returns true. It then iterates over the remaining characters from `start` to the end of the input, using a `prev_space` flag to track whether the previous character was whitespace. When a non‑space character is encountered, if `prev_space` is true and the result string is not empty, a single space character is appended before the character; otherwise only the character is appended. This collapses any run of internal whitespace into exactly one space. The result is a `std::string` with leading whitespace removed and interior whitespace sequences normalized to single spaces. The implementation avoids allocating extra memory by calling `reserve` on the result with the original input size, and relies solely on the standard library (`std::isspace`, `std::string`, `std::size_t`).

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `text`

#### Writes To

- local variable `result` (returned by value)

#### Usage Patterns

- called by `make_prompt_response_cache_key` to normalize prompt and response text before deriving a cache key

### `clore::generate::cache::save_cache_entry`

Declaration: `generate/cache.cppm:31`

Definition: `generate/cache.cppm:303`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The function serialises the write operation by first acquiring a static `std::mutex` (`cache_file_mutex`) via a `std::lock_guard`. It then retrieves the cache directory path by calling `cache_directory(workspace_root)`; on failure, the error is forwarded. The directory is created with `fs::create_directories`, and any failure produces a `CacheError` with a descriptive message. Next, the target JSONL file path is formed by appending `current_jsonl_filename()` to the directory. The entry line is built by `build_jsonl_line(cache_key, response)`, which formats the key‑value pair as a JSON object. The file is opened in binary append mode (`std::ofstream` with `std::ios::binary | std::ios::app`). The line is written via `file.write` and immediately flushed; if the write operation fails, a `CacheError` is returned. On success, an empty `std::expected<void, CacheError>` is returned.

#### Side Effects

- acquires a static mutex for synchronization
- creates directories on the filesystem if they do not exist
- opens a JSONL file for append binary writing
- writes a line to the file and flushes the stream
- may return observable error states in the result

#### Reads From

- parameter `workspace_root`
- parameter `cache_key`
- parameter `response`
- result of `cache_directory`
- result of `current_jsonl_filename`
- result of `build_jsonl_line`

#### Writes To

- filesystem directories at path derived from `cache_directory`
- JSONL file at path combining `cache_directory` and `current_jsonl_filename`
- the return value (success or error state)

#### Usage Patterns

- called to persist a generated response to the cache
- used after a successful generation to update the cache
- likely invoked synchronously from a code path that just produced a response

### `clore::generate::cache::save_cache_entry_async`

Declaration: `generate/cache.cppm:41`

Definition: `generate/cache.cppm:376`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The function `clore::generate::cache::save_cache_entry_async` is an asynchronous wrapper that offloads the synchronous `clore::generate::cache::save_cache_entry` to a background task via `kota::queue`, using the provided `kota::event_loop` for scheduling. After capturing `workspace_root`, `cache_key`, and `response` by move, it `co_awaits` the queued lambda; if the task was cancelled, it fails with a `CacheError` with a descriptive message. If the queued operation itself produces an error (i.e., `save_cache_entry` returned `std::unexpected`), that error is forwarded by failing with the same `CacheError`. On success, the function `co_returns` without a value, effectively publishing the cache entry asynchronously. Its internal control flow depends entirely on the result of the synchronous `save_cache_entry` and the `kota` concurrency primitives (`kota::queue`, `kota::fail`, `kota::task`).

#### Side Effects

- Writes a cache entry to persistent storage via `save_cache_entry`

#### Reads From

- `workspace_root` parameter
- `cache_key` parameter
- response parameter
- loop parameter

#### Writes To

- Persistent cache storage (via `save_cache_entry`)

#### Usage Patterns

- Called to asynchronously persist a generated response to the cache

## Internal Structure

The `generate:cache` module is decomposed into a public API and an internal helper layer. The public surface provides synchronous and asynchronous functions for loading cache indices (`load_cache_index`, `load_cache_index_async`), saving entries (`save_cache_entry`, `save_cache_entry_async`), and looking up cached responses (`find_cached_response`). Key construction is exposed via `make_prompt_response_cache_key`, which normalises prompt and response text before building a deterministic hash. Internally, the module relies on anonymous‑namespace helpers for fingerprinting `tool_choice` and `response_format`, enumerating JSONL files (`all_jsonl_files`), escaping JSON strings, and managing timestamped cache filenames.

The implementation imports `protocol` for LLM‑related types, `support` for foundational utilities (UTF‑8 I/O, path normalisation, hashing), and `std` for standard library facilities. Internal layering separates concerns: normalisation functions (`normalize_text_for_hashing`) feed into key generation, which is then used for index lookups and file‑based storage. The cache stores entries in JSONL files within a directory derived from the workspace root; a `CacheIndex` structure holds in‑memory entries, and file access is serialised via mutexes. Asynchronous variants schedule work on a `kota::event_loop`, while synchronous paths perform blocking I/O. Error handling is unified through the `CacheError` type, returned via `std::expected`.

## Related Pages

- [Module protocol](../protocol/index.md)
- [Module support](../support/index.md)

