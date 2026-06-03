---
title: 'clore::extract::ensurecachekeyimpl'
description: 'The function clore::extract::ensure_cache_key_impl computes and stores four cache-related fields into a CompileEntry. It first normalizes the entry’s file path via clore::extract::normalize_entry_file and writes the result into entry.normalized_file. Using that normalized path, it calls clore::extract::(anonymous namespace)::build_compile_signature_impl to produce a compile_signature. An optional source hash is computed by clore::extract::(anonymous namespace)::try_hash_source_file and stored in entry.source_hash. Finally, it assembles the final entry.cache_key by calling clore::support::build_cache_key with the normalized file and the compile signature.'
layout: doc
template: doc
---

# `clore::extract::ensure_cache_key_impl`

Owner: [Module extract:compiler](../compiler.md)

Declaration: `src/extract/compiler.cppm:135`

Definition: `src/extract/compiler.cppm:135`

Declaration: [`Namespace clore::extract`](../../../namespaces/clore/extract/index.md)

## Implementation

```cpp
auto ensure_cache_key_impl(CompileEntry& entry) -> void {
    entry.normalized_file = normalize_entry_file(entry);
    entry.compile_signature = build_compile_signature_impl(entry, entry.normalized_file);
    entry.source_hash = try_hash_source_file(entry.normalized_file);
    entry.cache_key =
        clore::support::build_cache_key(entry.normalized_file, entry.compile_signature);
}
```

The function `clore::extract::ensure_cache_key_impl` computes and stores four cache-related fields into a `CompileEntry`. It first normalizes the entry’s file path via `clore::extract::normalize_entry_file` and writes the result into `entry.normalized_file`. Using that normalized path, it calls `clore::extract::(anonymous namespace)::build_compile_signature_impl` to produce a `compile_signature`. An optional source hash is computed by `clore::extract::(anonymous namespace)::try_hash_source_file` and stored in `entry.source_hash`. Finally, it assembles the final `entry.cache_key` by calling `clore::support::build_cache_key` with the normalized file and the compile signature.

The algorithm is a serial data‑preparation process: each step depends on the results of the previous one. No branching or error handling is performed within this function; it relies on its callees to handle failures (e.g., `try_hash_source_file` may return `std::nullopt`, which is stored directly). The primary dependencies are the `normalize_entry_file` and `build_compile_signature_impl` functions, which respectively resolve the entry’s file to an absolute path and derive a hash from the compiler invocation arguments.

## Side Effects

- Mutates `entry.normalized_file`
- Mutates `entry.compile_signature`
- Mutates `entry.source_hash`
- Mutates `entry.cache_key`
- May perform file I/O via `try_hash_source_file` to compute source hash

## Reads From

- `entry` parameter (reads fields used by `normalize_entry_file` and `build_compile_signature_impl`)
- The source file referenced by `entry.normalized_file` (read by `try_hash_source_file`)

## Writes To

- `entry.normalized_file`
- `entry.compile_signature`
- `entry.source_hash`
- `entry.cache_key`

## Usage Patterns

- Called by `clore::extract::ensure_cache_key` to ensure a compile entry's cache key is computed

## Calls

- function `clore::extract::(anonymous namespace)::build_compile_signature_impl`
- function `clore::extract::(anonymous namespace)::try_hash_source_file`
- function `clore::extract::normalize_entry_file`

## Called By

- function `clore::extract::ensure_cache_key`

