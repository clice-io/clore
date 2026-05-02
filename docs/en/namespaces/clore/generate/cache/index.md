---
title: 'Namespace clore::generate::cache'
description: 'The clore::generate::cache namespace provides a caching subsystem for generated responses, enabling the storage and retrieval of prompt–response pairs to avoid redundant computation. Its core functions include load_cache_index (and its asynchronous counterpart load_cache_index_async) for reading the persistent index, find_cached_response for looking up previously stored results, save_cache_entry (and save_cache_entry_async) for persisting new entries, and make_prompt_response_cache_key for constructing deterministic keys from prompts, responses, and configuration parameters. Supporting utilities such as normalize_text_for_hashing canonicalize text inputs to ensure consistent key derivation. Notable declarations include the CacheIndex struct, which represents the in-memory state of the cache, and the CacheError struct for reporting I/O or validation failures. Architecturally, this namespace acts as a transparent caching layer between generation requests and the underlying storage, using a key-based index to avoid recomputation while supporting both synchronous and asynchronous workflows.'
layout: doc
template: doc
---

# Namespace `clore::generate::cache`

## Summary

The `clore::generate::cache` namespace provides a caching subsystem for generated responses, enabling the storage and retrieval of prompt–response pairs to avoid redundant computation. Its core functions include `load_cache_index` (and its asynchronous counterpart `load_cache_index_async`) for reading the persistent index, `find_cached_response` for looking up previously stored results, `save_cache_entry` (and `save_cache_entry_async`) for persisting new entries, and `make_prompt_response_cache_key` for constructing deterministic keys from prompts, responses, and configuration parameters. Supporting utilities such as `normalize_text_for_hashing` canonicalize text inputs to ensure consistent key derivation. Notable declarations include the `CacheIndex` struct, which represents the in-memory state of the cache, and the `CacheError` struct for reporting I/O or validation failures. Architecturally, this namespace acts as a transparent caching layer between generation requests and the underlying storage, using a key-based index to avoid recomputation while supporting both synchronous and asynchronous workflows.

## Types

### `clore::generate::cache::CacheError`

Declaration: `generate/cache.cppm:16`

Definition: `generate/cache.cppm:16`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- No invariants beyond those of `std::string`.

#### Key Members

- message

#### Usage Patterns

- Defined as a simple error type for caching operations.

### `clore::generate::cache::CacheIndex`

Declaration: `generate/cache.cppm:20`

Definition: `generate/cache.cppm:20`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Insufficient evidence to summarize; provide more EVIDENCE.

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

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

The caller uses `clore::generate::cache::find_cached_response` to retrieve a previously stored response from the cache index. The function accepts a constant reference to a `CacheIndex` (obtained from `clore::generate::cache::load_cache_index`) and a `std::string_view` key that presumably matches the key used when saving the entry (for example, a value returned by `clore::generate::cache::make_prompt_response_cache_key`). If a matching entry is found, the function returns a `std::optional<std::string_view>` containing the cached response; otherwise, the optional is empty. No assumptions are made about the lifetime of the pointed-to data beyond the validity of the underlying cache storage that the `CacheIndex` represents.

#### Usage Patterns

- checking for an existing cached response before generating a new one
- lookup by cache key in a `CacheIndex`

### `clore::generate::cache::load_cache_index`

Declaration: `generate/cache.cppm:29`

Definition: `generate/cache.cppm:252`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

The function `clore::generate::cache::load_cache_index` attempts to load and deserialize the complete cache index from the given file path. On success, it returns a `CacheIndex` representing the current state of the cache; on failure, it returns a `CacheError` that describes why the load could not complete (e.g., file not found, malformed data, or I/O error). Calling code should check the returned `std::expected` to determine success before using the index.

#### Usage Patterns

- cache index initialization on application startup
- reloading cache index from disk
- building a lookup structure for quickly retrieving cached responses by key

### `clore::generate::cache::load_cache_index_async`

Declaration: `generate/cache.cppm:38`

Definition: `generate/cache.cppm:356`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

The function `clore::generate::cache::load_cache_index_async` initiates an asynchronous load of the cache index from a persistent store identified by the given file path, using the provided `kota::event_loop &` for scheduling and completion. It returns an `int` handle that can be used to track or cancel the operation; the caller should ensure the event loop remains alive until the operation finishes. This function is the asynchronous counterpart to `clore::generate::cache::load_cache_index`, which performs the same loading synchronously and returns a `std::expected<CacheIndex, CacheError>`.

#### Usage Patterns

- asynchronous cache index loading before response caching
- non‑blocking initialization in event‑loop driven applications
- part of the cache layer that integrates with `save_cache_entry_async`

### `clore::generate::cache::make_prompt_response_cache_key`

Declaration: `generate/cache.cppm:24`

Definition: `generate/cache.cppm:219`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

`make_prompt_response_cache_key` constructs a deterministic cache key string for a given prompt, response, and integer parameter (often representing a generation variant or configuration version). Callers use this key to uniquely identify cached prompt-response pairs, enabling consistent lookups in `find_cached_response` and storage in `save_cache_entry`. The function may fail with a `CacheError` if the inputs cannot be processed (for example, if internal normalization via `normalize_text_for_hashing` fails), and it returns a `std::expected<std::string, CacheError>` to communicate success or error without exceptions.

#### Usage Patterns

- Used before `clore::generate::cache::find_cached_response` to generate key for lookup
- Used before `clore::generate::cache::save_cache_entry` to generate key for storing
- Employed by asynchronous cache operations such as `save_cache_entry_async` when constructing keys

### `clore::generate::cache::normalize_text_for_hashing`

Declaration: `generate/cache.cppm:192`

Definition: `generate/cache.cppm:192`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Declaration: [Declaration](functions/normalize-text-for-hashing.md)

The function `clore::generate::cache::normalize_text_for_hashing` accepts a `std::string_view` and returns a `std::string`. Its responsibility is to transform an arbitrary text input into a canonical, deterministic form suitable for use as input to a hashing or key‑derivation step. Callers rely on this normalization to ensure that semantically equivalent texts produce the same normalized output, regardless of superficial formatting differences (such as extra whitespace, casing, or control characters).

This function is a low‑level utility invoked during cache‑key construction. It is used internally by `clore::generate::cache::make_prompt_response_cache_key` to produce a stable hash key for prompt–response pairs. The contract guarantees that the returned string is a well‑defined, repeatable representation of the original text; the caller must not assume any particular transform (e.g., lower‑casing, trimming) beyond the general property that identical inputs always yield identical outputs.

#### Usage Patterns

- Used by `make_prompt_response_cache_key` to normalize text before forming a cache key

### `clore::generate::cache::save_cache_entry`

Declaration: `generate/cache.cppm:31`

Definition: `generate/cache.cppm:303`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

`clore::generate::cache::save_cache_entry` attempts to store a cache entry formed from three caller-provided string views. The function accepts inputs that likely represent a cache key, a prompt, and a response, or similar grouping, and writes the entry to the underlying cache storage.

On success the function returns `std::expected<void, CacheError>` containing a `void` value. On failure it returns a `CacheError` describing the reason, such as an I/O fault or an invalid argument. The caller must ensure that all arguments outlive the call and that the cache system has been properly initialized.

#### Usage Patterns

- persist a generated response in the cache after completion of a generation request
- synchronous alternative to `save_cache_entry_async` for environments where async is not desired

### `clore::generate::cache::save_cache_entry_async`

Declaration: `generate/cache.cppm:41`

Definition: `generate/cache.cppm:376`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

The function `clore::generate::cache::save_cache_entry_async` initiates an asynchronous save of a cache entry identified by a triple of strings: a key-like prompt identifier, a model identifier, and the response value. It takes a `kota::event_loop &` to schedule the background operation and returns an `int` representing a handle or operation identifier that can later be used to check completion or retrieve the result. The caller is responsible for providing the three string arguments and a running event loop; the function returns immediately and the actual persistent store write proceeds asynchronously on the given loop. No synchronous validation or error reporting is performed at call time; the returned integer is the only handle to the outstanding operation.

#### Usage Patterns

- Used to asynchronously persist a generated response into a cache indexed by key and workspace
- Called when a cache miss occurs and a new response must be stored without blocking the caller
- Part of the `clore::generate::cache` module's asynchronous API

## Related Pages

- [Namespace clore::generate](../index.md)

