---
title: 'Namespace clore::generate::cache'
description: 'The clore::generate::cache namespace provides a caching subsystem for generated content, storing prompt–response pairs to avoid redundant computation. Its core operations include find_cached_response, which performs a read-only lookup in a CacheIndex to retrieve a previously cached response, and save_cache_entry, which persists a new entry synchronously. Asynchronous variants (load_cache_index_async, save_cache_entry_async) use a kota::event_loop for non‑blocking I/O, while normalize_text_for_hashing and make_prompt_response_cache_key ensure deterministic, hash‑based keys for cache entries.'
layout: doc
template: doc
---

# Namespace `clore::generate::cache`

## Summary

The `clore::generate::cache` namespace provides a caching subsystem for generated content, storing prompt–response pairs to avoid redundant computation. Its core operations include `find_cached_response`, which performs a read-only lookup in a `CacheIndex` to retrieve a previously cached response, and `save_cache_entry`, which persists a new entry synchronously. Asynchronous variants (`load_cache_index_async`, `save_cache_entry_async`) use a `kota::event_loop` for non‑blocking I/O, while `normalize_text_for_hashing` and `make_prompt_response_cache_key` ensure deterministic, hash‑based keys for cache entries.

The namespace uses `CacheIndex` and `CacheError` types to manage index data and report failures. It relies on a `workspace_root` to locate cache files and includes a mutex (`cache_file_mutex`) for thread‑safe access. Architecturally, this namespace sits between the generation pipeline and the file system, offering both synchronous and asynchronous cache operations that improve performance by reducing repeated identical requests.

## Types

### `clore::generate::cache::CacheError`

Declaration: `generate/cache.cppm:16`

Definition: `generate/cache.cppm:16`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Insufficient evidence to summarize; provide more EVIDENCE.

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

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Insufficient evidence to summarize; provide more EVIDENCE.

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

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

The function `clore::generate::cache::find_cached_response` performs a lookup in the provided `CacheIndex` using the given key, returning the cached response as a `std::optional<std::string_view>` if present. The caller is responsible for ensuring the `CacheIndex` is valid and the key corresponds to an entry previously stored via a save function (such as `clore::generate::cache::save_cache_entry`). If no matching entry exists, the function returns `std::nullopt`. The function does not modify the cache index and is safe to call concurrently with read-only access.

#### Usage Patterns

- Check existence of cached response before generation
- Retrieve cached response by key from index

### `clore::generate::cache::load_cache_index`

Declaration: `generate/cache.cppm:29`

Definition: `generate/cache.cppm:252`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Loads the cache index from the location identified by the provided `std::string_view`. Returns a `std::expected<CacheIndex, CacheError>`; on success the caller receives a valid `CacheIndex` that can be passed to functions such as `clore::generate::cache::find_cached_response`. On failure a `CacheError` is returned indicating the reason the index could not be loaded.

#### Usage Patterns

- called to load cache index from disk
- used at initialization to populate in-memory cache index

### `clore::generate::cache::load_cache_index_async`

Declaration: `generate/cache.cppm:38`

Definition: `generate/cache.cppm:356`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

`clore::generate::cache::load_cache_index_async` initiates an asynchronous load of the cache index from a given file path. The caller provides a `std::string` specifying the location of the index file and a reference to a `kota::event_loop` on which the operation will be scheduled. The function returns an `int` that identifies the pending asynchronous load request, allowing the caller to correlate with the eventual completion notification. The caller must ensure that the provided `kota::event_loop` remains alive until the asynchronous operation finishes.

#### Usage Patterns

- asynchronous loading of cache index
- wrapping synchronous `load_cache_index` into a coroutine

### `clore::generate::cache::make_prompt_response_cache_key`

Declaration: `generate/cache.cppm:24`

Definition: `generate/cache.cppm:219`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

The function `clore::generate::cache::make_prompt_response_cache_key` constructs a deterministic cache key from a prompt, a response, and an integer parameter. Callers provide the prompt text as a `std::string_view`, the response text as a `std::string_view`, and a version or configuration identifier as a `const int &`. On success, the function returns a `std::expected<std::string, CacheError>` containing the hashed key string; on failure, it returns a `CacheError` indicating why key generation could not be completed.

The resulting key is intended for use with other cache operations such as `clore::generate::cache::find_cached_response` and `clore::generate::cache::save_cache_entry`. The function relies on `clore::generate::cache::normalize_text_for_hashing` to normalize both input texts before combining them, ensuring consistency across different representations of the same content.

#### Usage Patterns

- Used by `find_cached_response` and `save_cache_entry` to generate a unique key for cache operations.

### `clore::generate::cache::normalize_text_for_hashing`

Declaration: `generate/cache.cppm:192`

Definition: `generate/cache.cppm:192`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Declaration: [Declaration](functions/normalize-text-for-hashing.md)

Normalizes a given input text so that functionally equivalent strings produce an identical result, enabling consistent hashing within the caching system. The caller supplies a `std::string_view` and receives a `std::string` that has been transformed according to internal rules (for example, trimming whitespace and reducing letter case) to eliminate benign variations. This normalized string is intended to be used as part of a cache key, ensuring that the same conceptual prompt or system prompt always maps to the same hash regardless of incidental formatting differences.

#### Usage Patterns

- called by `make_prompt_response_cache_key` to normalize prompt and response text before deriving a cache key

### `clore::generate::cache::save_cache_entry`

Declaration: `generate/cache.cppm:31`

Definition: `generate/cache.cppm:303`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

The function `clore::generate::cache::save_cache_entry` persists a prompt–response pair into the cache storage. It accepts three `std::string_view` arguments, which correspond to a cache key (derived from the prompt), the prompt text, and the associated response text. On success the function returns `std::expected<void, CacheError>` with an empty value; on failure it returns a `CacheError` indicating the reason for the failure. Callers must ensure that the provided strings remain valid for the duration of the call, and that the cache storage is accessible and writable.

#### Usage Patterns

- called to persist a generated response to the cache
- used after a successful generation to update the cache
- likely invoked synchronously from a code path that just produced a response

### `clore::generate::cache::save_cache_entry_async`

Declaration: `generate/cache.cppm:41`

Definition: `generate/cache.cppm:376`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

The function `clore::generate::cache::save_cache_entry_async` initiates an asynchronous save of a cache entry. The caller provides three `std::string` arguments—typically representing the cache key, the value to store, and an additional identifier such as a model or hash string—along with a `kota::event_loop &` on which the completion will be delivered. It returns an `int` that signals whether the operation was successfully enqueued; the caller must ensure the given event loop remains active until the save completes.

#### Usage Patterns

- Called to asynchronously persist a generated response to the cache

## Related Pages

- [Namespace clore::generate](../index.md)

