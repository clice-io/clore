---
title: 'Module generate:cache'
description: 'The generate:cache module provides a persistent caching layer for generated responses in the LLM completion workflow. Its core responsibility is to store, retrieve, and index results keyed by the combination of prompt, system prompt, tool choice, and response format, enabling efficient reuse of previously computed outputs. The public-facing implementation includes synchronous interfaces (save_cache_entry, load_cache_index, find_cached_response) for direct cache access, asynchronous counterparts (save_cache_entry_async, load_cache_index_async) for non‑blocking operations, and utility functions (make_prompt_response_cache_key, normalize_text_for_hashing) that produce deterministic cache keys from request parameters. The module also exposes two structs — CacheIndex holding the in‑memory index of cached entries, and CacheError for reporting failures — all under the clore::generate::cache namespace.'
layout: doc
template: doc
---

# Module `generate:cache`

## Summary

The `generate:cache` module provides a persistent caching layer for generated responses in the LLM completion workflow. Its core responsibility is to store, retrieve, and index results keyed by the combination of prompt, system prompt, tool choice, and response format, enabling efficient reuse of previously computed outputs. The public-facing implementation includes synchronous interfaces (`save_cache_entry`, `load_cache_index`, `find_cached_response`) for direct cache access, asynchronous counterparts (`save_cache_entry_async`, `load_cache_index_async`) for non‑blocking operations, and utility functions (`make_prompt_response_cache_key`, `normalize_text_for_hashing`) that produce deterministic cache keys from request parameters. The module also exposes two structs — `CacheIndex` holding the in‑memory index of cached entries, and `CacheError` for reporting failures — all under the `clore::generate::cache` namespace.

## Imports

- [`protocol`](../protocol/index.md)
- [`support`](../support/index.md)

## Imported By

- [`generate:scheduler`](scheduler.md)

## Types

### `clore::generate::cache::CacheError`

Declaration: `src/generate/cache.cppm:35`

Definition: `src/generate/cache.cppm:35`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The struct `clore::generate::cache::CacheError` is a minimal error wrapper defined in the implementation namespace. Its sole data member, `message` of type `std::string`, holds a human-readable description of the error. The struct carries no invariants beyond the string being in a valid state, and it relies entirely on the default special member functions provided by the compiler and the `std::string` class. This lightweight design makes it suitable for use as an error result or exception payload within the `clore::generate::cache` module, where no additional error metadata or custom behavior is required.

#### Invariants

- The `message` member can be any string, including an empty string.

#### Key Members

- `message`

#### Usage Patterns

- Used to store an error description string; likely returned from functions to indicate failure.

### `clore::generate::cache::CacheIndex`

Declaration: `src/generate/cache.cppm:39`

Definition: `src/generate/cache.cppm:39`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The struct `clore::generate::cache::CacheIndex` is implemented as a thin wrapper around a single public member `entries`, which is an `std::unordered_map` mapping strings to strings. This container stores the mapping from cache keys (likely resource identifiers or paths) to their corresponding cached values (e.g., file hashes or generated output). The only structural invariant is that each key is unique and maps to exactly one value, enforced by the underlying associative container. There are no additional methods or constraints beyond those inherent to the hash map; the struct serves as a direct index into the cached data.

#### Invariants

- The `entries` map is an unordered mapping from string keys to string values
- No invariants beyond those of `std::unordered_map` are guaranteed by this struct

#### Key Members

- entries

#### Usage Patterns

- Used by the cache system to store and retrieve string-based index entries
- Likely manipulated via direct access to the `entries` member

## Functions

### `clore::generate::cache::find_cached_response`

Declaration: `src/generate/cache.cppm:54`

Definition: `src/generate/cache.cppm:366`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The implementation of `clore::generate::cache::find_cached_response` performs a direct hash-map lookup into the pre‑loaded `clore::generate::cache::CacheIndex` object. The function converts the incoming `std::string_view` `cache_key` into a `std::string` and calls `std::unordered_map::find` on the `clore::generate::cache::CacheIndex::entries` member. If the key is absent, it returns `std::nullopt`; otherwise it returns `it->second`, which is the cached response string (wrapped in `std::optional<std::string_view>`). No additional parsing, validation, or file I/O occurs—the entire retrieval path depends on the integrity of the `clore::generate::cache::CacheIndex` that was previously assembled by `clore::generate::cache::load_cache_index` or its async counterpart. The control flow is therefore purely sequential and constant‑time (amortized) with respect to the number of cached entries.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `index.entries` (the map inside `CacheIndex`)
- `cache_key` (the lookup string)

#### Usage Patterns

- Check if a response is cached before generating a new one
- Used internally by higher-level cache query functions

### `clore::generate::cache::load_cache_index`

Declaration: `src/generate/cache.cppm:48`

Definition: `src/generate/cache.cppm:271`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The implementation of `clore::generate::cache::load_cache_index` begins by calling `all_jsonl_files` on the provided `workspace_root`; if this call fails, the function immediately returns an unexpected `CacheError`.  It then iterates over every file path in the returned list, reading each as UTF‑8 text with `clore::support::read_utf8_text_file`.  For each file that is successfully read, the function splits the content into lines and processes each non‑empty line.  Every such line is parsed as a JSON object using `kota::codec::json::parse`.  If parsing succeeds, the function looks for the `"key"` and `"resp"` string fields in the parsed object; a line is skipped if either field is missing, not a string, or empty.  When both fields are valid, the pair is inserted into the `CacheIndex`’s `entries` map, overwriting any existing entry for the same key.  The function returns the assembled `CacheIndex` after all files have been processed, gracefully ignoring any file‑ or line‑level errors encountered along the way.

#### Side Effects

- Reads multiple JSONL files from the filesystem under `workspace_root`
- Parses file contents into JSON objects

#### Reads From

- `workspace_root` parameter
- JSONL files on disk

#### Usage Patterns

- Initialize cache state at startup
- Load existing cache entries from disk

### `clore::generate::cache::load_cache_index_async`

Declaration: `src/generate/cache.cppm:57`

Definition: `src/generate/cache.cppm:375`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The implementation of `load_cache_index_async` is a coroutine that wraps the synchronous `load_cache_index` function for asynchronous execution on a `kota::event_loop`. It calls `kota::queue` to offload the synchronous work to a background thread, passing a lambda that captures the moved `workspace_root` and delegates to `load_cache_index`. After `co_await`, it invokes `catch_cancel` to handle task cancellation gracefully. If the `queued_result` contains an error at the top level (e.g., the task was cancelled), it `co_awaits` `kota::fail` with a `CacheError` describing the failure. Otherwise, it unwraps the inner `std::expected` from `load_cache_index`: if that expected holds an error, it `co_awaits` `kota::fail` with the moved `CacheError`; on success, it `co_returns` the moved `CacheIndex` value.

#### Side Effects

- dispatches a blocking disk read operation to an event loop
- may load and parse a cache index file from disk into memory

#### Reads From

- disk via the synchronous `load_cache_index` call
- event loop parameter loop

#### Writes To

- allocates and populates a `CacheIndex` object returned via the task

#### Usage Patterns

- called to asynchronously retrieve the cache index before performing cache queries or updates

### `clore::generate::cache::make_prompt_response_cache_key`

Declaration: `src/generate/cache.cppm:43`

Definition: `src/generate/cache.cppm:238`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The function first processes the `request.response_format` by calling `response_format_fingerprint`, propagating any error immediately via `std::unexpected`. It then normalizes both the prompt and the system prompt using `normalize_text_for_hashing`, and computes 64‑bit hashes of each with `llvm::xxh3_64bits`. A fingerprint for `request.tool_choice` is obtained through `tool_choice_fingerprint`. The final cache key is built by appending the `request_key`, the two hash values as decimal strings, the response format fingerprint, the tool choice fingerprint, and a single character encoding `request.output_contract`, each component separated by a tab character. The string is constructed with a pre‑reserved capacity to minimize allocations.

#### Side Effects

- allocates heap memory for the returned string
- calls `normalize_text_for_hashing` which may allocate
- calls `llvm::xxh3_64bits` which is a pure hash but may use internal state

#### Reads From

- parameter `request_key`
- parameter `system_prompt`
- parameter `request` fields: `request.response_format`, `request.prompt`, `request.tool_choice`, `request.output_contract`

#### Writes To

- local `response_format`
- local `normalized_prompt`
- local `normalized_system_prompt`
- local `prompt_hash`
- local `system_prompt_hash`
- local `tool_choice`
- local `key`
- return value (new heap-allocated string)

#### Usage Patterns

- called during cache key generation for prompt-response pairs
- used by `find_cached_response` and related cache functions

### `clore::generate::cache::normalize_text_for_hashing`

Declaration: `src/generate/cache.cppm:211`

Definition: `src/generate/cache.cppm:211`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

Implementation: [Implementation](functions/normalize-text-for-hashing.md)

The function `clore::generate::cache::normalize_text_for_hashing` normalizes input text to a canonical form suitable for generating hash-based cache keys. It first trims all leading whitespace by advancing a `start` index past any characters for which `std::isspace` returns true. It then iterates over the remaining characters, using a `prev_space` flag to collapse any contiguous sequence of whitespace characters into a single space character inserted just before the next non‑space character. If the very first non‑space character after trimming would have been preceded by a space, the space is omitted because `result` is empty at that point; this effectively suppresses leading whitespace that was already trimmed. The resulting string contains no leading whitespace, no repeated spaces, and no trailing whitespace (because trailing whitespace is skipped by the collapse logic and not added back). Normalization relies solely on the C++ locale‑independent `std::isspace` check after casting each character to `unsigned char`, and uses only standard library facilities.

#### Side Effects

- Allocates a new string and returns it

#### Reads From

- text parameter

#### Writes To

- returned result string

#### Usage Patterns

- Called by `make_prompt_response_cache_key` to normalize prompt and response texts before generating the cache key

### `clore::generate::cache::save_cache_entry`

Declaration: `src/generate/cache.cppm:50`

Definition: `src/generate/cache.cppm:322`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The function first acquires a static `std::mutex` via `std::lock_guard lock(cache_file_mutex)` to serialize concurrent cache writes. It resolves the cache directory by calling `cache_directory(workspace_root)`; if that fails, the error is forwarded. After ensuring the directory exists with `fs::create_directories`, it constructs the target JSONL file path by appending the result of `current_jsonl_filename()` to the directory. A JSONL line is built from `cache_key` and `response` using `build_jsonl_line`, and the file is opened in binary append mode. The line is written, flushed, and the write status is checked; any failure (open or write) returns a `CacheError` with a descriptive message. On success, the function returns `{}`. This implementation depends on the internal helpers `cache_directory`, `current_jsonl_filename`, and `build_jsonl_line`, and uses a static mutex to provide thread-safe append to the rotating JSONL log files.

#### Side Effects

- acquires a static mutex
- creates directories on the filesystem
- opens and appends to a JSONL file
- flushes the file stream

#### Reads From

- parameter `workspace_root`
- parameter `cache_key`
- parameter `response`
- `current_jsonl_filename()`
- `cache_directory(workspace_root)`
- `build_jsonl_line(cache_key, response)`

#### Writes To

- filesystem: cache directory
- filesystem: JSONL file at `*dir / current_jsonl_filename()`

#### Usage Patterns

- used to persist a cache entry for later retrieval via `load_cache_index` and `find_cached_response`
- wrapped by `save_cache_entry_async` for asynchronous execution

### `clore::generate::cache::save_cache_entry_async`

Declaration: `src/generate/cache.cppm:60`

Definition: `src/generate/cache.cppm:395`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

The function `clore::generate::cache::save_cache_entry_async` offloads the synchronous `clore::generate::cache::save_cache_entry` onto a background task via `kota::queue` on the provided `kota::event_loop`. The three string arguments (`workspace_root`, `cache_key`, and `response`) are moved into the lambda closure to avoid copies. After awaiting the queued operation, it first checks for cancellation — if the task was cancelled, it fails with a `CacheError` containing a generic message. Otherwise, it inspects the inner `std::expected`; if the save produced an error, that error is forwarded via `kota::fail`. On success, the coroutine returns `void`. The implementation relies on the synchronous `save_cache_entry` for the actual file‑system write, and on the `kota` concurrency framework for asynchronous dispatch and error propagation.

#### Side Effects

- I/O to disk via file writes
- synchronization via event loop queue
- allocation of task, lambda, and moved strings
- error propagation through coroutine failure

#### Reads From

- `workspace_root`
- `cache_key`
- `response`
- `loop`

#### Writes To

- disk cache entry via `save_cache_entry`
- error state via `kota::fail`

#### Usage Patterns

- asynchronous caching of prompt-response
- offloading blocking cache save to event loop

## Internal Structure

The `generate:cache` module is implemented as a single C++20 module partition (`src/generate/cache.cppm`) that imports the `protocol` and `support` modules. It provides both synchronous and asynchronous `APIs` for persisting and retrieving LLM response caches. Internally, the module is decomposed into a public interface (functions like `load_cache_index`, `save_cache_entry`, `find_cached_response`, `make_prompt_response_cache_key`) and an anonymous namespace containing private helpers—for example, `escape_json_string`, `format_iso_timestamp`, and fingerprint generators for response format and tool choice. Cache entries are stored as JSONL files, and cache keys are constructed by normalizing textual inputs (via `normalize_text_for_hashing`) and combining hashes of the system prompt, user request, and tool configuration. Asynchronous operations (`load_cache_index_async`, `save_cache_entry_async`) delegate to a `kota::event_loop` for non‑blocking I/O, while a `cache_file_mutex` guards concurrent file access. This layered structure keeps I/O and key‑generation logic separated, making the module easy to extend with new caching strategies without affecting the public contract.

## Related Pages

- [Module protocol](../protocol/index.md)
- [Module support](../support/index.md)

