---
title: 'Namespace clore::generate::cache'
description: 'The clore::generate::cache namespace provides a caching subsystem for the generation module, responsible for storing, retrieving, and indexing responses to avoid redundant recomputation. It defines key data structures such as CacheIndex, which holds the in-memory mapping of cache keys to stored responses, and CacheError, an error type for reporting failures during cache operations. Core synchronous functions include save_cache_entry to persist a cache entry, load_cache_index to read the index from disk, and find_cached_response to perform a non‑owning lookup in an already loaded index. Asynchronous variants (save_cache_entry_async, load_cache_index_async) allow non‑blocking file I/O via kota::event_loop. Two utility functions support robust key generation: make_prompt_response_cache_key constructs a deterministic cache key from a prompt, response, and version discriminator, while normalize_text_for_hashing canonicalizes text (e.g., prompts) to ensure semantically equivalent inputs produce identical keys. The namespace’s architectural role is to encapsulate all cache logic, providing both synchronous and asynchronous interfaces that higher‑level generation code can use to accelerate repeated queries while maintaining consistent key derivation and error handling.'
layout: doc
template: doc
---

# Namespace `clore::generate::cache`

## Summary

The `clore::generate::cache` namespace provides a caching subsystem for the generation module, responsible for storing, retrieving, and indexing responses to avoid redundant recomputation. It defines key data structures such as `CacheIndex`, which holds the in-memory mapping of cache keys to stored responses, and `CacheError`, an error type for reporting failures during cache operations. Core synchronous functions include `save_cache_entry` to persist a cache entry, `load_cache_index` to read the index from disk, and `find_cached_response` to perform a non‑owning lookup in an already loaded index. Asynchronous variants (`save_cache_entry_async`, `load_cache_index_async`) allow non‑blocking file I/O via `kota::event_loop`. Two utility functions support robust key generation: `make_prompt_response_cache_key` constructs a deterministic cache key from a prompt, response, and version discriminator, while `normalize_text_for_hashing` canonicalizes text (e.g., prompts) to ensure semantically equivalent inputs produce identical keys. The namespace’s architectural role is to encapsulate all cache logic, providing both synchronous and asynchronous interfaces that higher‑level generation code can use to accelerate repeated queries while maintaining consistent key derivation and error handling.

## Types

### `clore::generate::cache::CacheError`

Declaration: `src/generate/cache.cppm:35`

Definition: `src/generate/cache.cppm:35`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- The `message` member can be any string, including an empty string.

#### Key Members

- `message`

#### Usage Patterns

- Used to store an error description string; likely returned from functions to indicate failure.

### `clore::generate::cache::CacheIndex`

Declaration: `src/generate/cache.cppm:39`

Definition: `src/generate/cache.cppm:39`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Insufficient evidence to summarize; provide more EVIDENCE.

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

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

The function `clore::generate::cache::find_cached_response` accepts a `const CacheIndex &` and a `std::string_view` representing a cache key. It searches the provided cache index for a matching entry and, if found, returns the associated cached response as an `std::optional<std::string_view>`. If no entry matches the given key, the function returns `std::nullopt`. This lookup is non‑owning and does not perform I/O, making it suitable for read‑only cache queries within the caller's control flow.

#### Usage Patterns

- Check if a response is cached before generating a new one
- Used internally by higher-level cache query functions

### `clore::generate::cache::load_cache_index`

Declaration: `src/generate/cache.cppm:48`

Definition: `src/generate/cache.cppm:271`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

The function `clore::generate::cache::load_cache_index` is the synchronous interface for loading a cache index from persistent storage. The caller provides a `std::string_view` identifying the cache index to load; the function returns a `std::expected<CacheIndex, CacheError>`. On success, the returned `CacheIndex` contains the full cached state, ready for lookups via `find_cached_response`. On failure, the `CacheError` indicates why loading failed (e.g., missing data, corrupt format). This operation blocks the calling thread — use `load_cache_index_async` for asynchronous behavior. The identifier should match the one previously used when saving entries.

#### Usage Patterns

- Initialize cache state at startup
- Load existing cache entries from disk

### `clore::generate::cache::load_cache_index_async`

Declaration: `src/generate/cache.cppm:57`

Definition: `src/generate/cache.cppm:375`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Initiates an asynchronous load of the cache index from the specified file path using the provided `kota::event_loop`. The function returns an `int` that serves as an operation handle or status identifier for the ongoing asynchronous load. The caller must ensure that the event loop remains active and is properly driven while the operation is in flight, and that the given `std::string` path refers to a valid cache index file. Completion, success, or failure of the asynchronous operation is communicated through the event loop's mechanisms, and the returned integer can be used to correlate this load request with its eventual outcome. This function is the non‑blocking counterpart of `clore::generate::cache::load_cache_index`.

#### Usage Patterns

- called to asynchronously retrieve the cache index before performing cache queries or updates

### `clore::generate::cache::make_prompt_response_cache_key`

Declaration: `src/generate/cache.cppm:43`

Definition: `src/generate/cache.cppm:238`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

The caller provides a prompt, a response, and an integer parameter (likely a version or configuration discriminator). The function returns a `std::expected<std::string, CacheError>` containing a deterministic cache key string on success, or a `CacheError` on failure (for example, if text normalization fails internally). The resulting key is suitable for use with other cache operations such as `save_cache_entry` or `find_cached_response`.

#### Usage Patterns

- called during cache key generation for prompt-response pairs
- used by `find_cached_response` and related cache functions

### `clore::generate::cache::normalize_text_for_hashing`

Declaration: `src/generate/cache.cppm:211`

Definition: `src/generate/cache.cppm:211`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Declaration: [Declaration](functions/normalize-text-for-hashing.md)

`clore::generate::cache::normalize_text_for_hashing` accepts a `std::string_view` representing text such as a prompt or system prompt, and returns a `std::string` that has been normalized to a canonical form. This normalized form is intended to be used as input for hashing when constructing cache keys, ensuring that semantically equivalent text yields the same hash regardless of inconsequential variations like whitespace or formatting. Callers rely on this function to produce a consistent representation of textual inputs before combining them into a cache key in `make_prompt_response_cache_key`.

#### Usage Patterns

- Called by `make_prompt_response_cache_key` to normalize prompt and response texts before generating the cache key

### `clore::generate::cache::save_cache_entry`

Declaration: `src/generate/cache.cppm:50`

Definition: `src/generate/cache.cppm:322`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

The function `clore::generate::cache::save_cache_entry` persists a cache entry identified by three `std::string_view` parameters. It returns `std::expected<void, CacheError>`; on success the entry is durably stored, and on failure a `CacheError` is reported. Callers must ensure the provided strings are valid and non‑empty; no further guarantees about thread safety or asynchronous behavior are provided by this synchronous interface.

#### Usage Patterns

- used to persist a cache entry for later retrieval via `load_cache_index` and `find_cached_response`
- wrapped by `save_cache_entry_async` for asynchronous execution

### `clore::generate::cache::save_cache_entry_async`

Declaration: `src/generate/cache.cppm:60`

Definition: `src/generate/cache.cppm:395`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

The function `clore::generate::cache::save_cache_entry_async` accepts three `std::string` arguments representing the cache entry data and a `kota::event_loop &` under which the asynchronous operation executes. It returns an `int` that identifies or indicates the result of the asynchronous save attempt. Callers must supply owning string values and a live event loop; the function does not block the calling thread. The returned integer serves as a token or status for the started operation, consistent with the module’s async cache utility pattern.

#### Usage Patterns

- asynchronous caching of prompt-response
- offloading blocking cache save to event loop

## Related Pages

- [Namespace clore::generate](../index.md)

