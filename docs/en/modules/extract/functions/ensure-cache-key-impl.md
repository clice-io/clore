---
title: 'clore::extract::ensurecachekeyimpl'
description: 'The implementation of ensure_cache_key_impl populates a CompileEntry object with the data needed to derive its cache key. It first calls normalize_entry_file to resolve the entry’s source file path and stores the result in entry.normalized_file. Using that normalized path and the entry, it invokes build_compile_signature_impl to compute a signature representing the compilation’s effective arguments and stores it in entry.compile_signature. It then attempts to hash the normalized source file via try_hash_source_file, writing the result (if available) into entry.source_hash. Finally, it constructs the cache key by calling clore::support::build_cache_key with the normalized file path and the compile signature, and assigns it to entry.cache_key.'
layout: doc
template: doc
---

# `clore::extract::ensure_cache_key_impl`

Owner: [Module extract:compiler](../compiler.md)

Declaration: `extract/compiler.cppm:119`

Definition: `extract/compiler.cppm:119`

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

The implementation of `ensure_cache_key_impl` populates a `CompileEntry` object with the data needed to derive its cache key. It first calls `normalize_entry_file` to resolve the entry’s source file path and stores the result in `entry.normalized_file`. Using that normalized path and the `entry`, it invokes `build_compile_signature_impl` to compute a signature representing the compilation’s effective arguments and stores it in `entry.compile_signature`. It then attempts to hash the normalized source file via `try_hash_source_file`, writing the result (if available) into `entry.source_hash`. Finally, it constructs the cache key by calling `clore::support::build_cache_key` with the normalized file path and the compile signature, and assigns it to `entry.cache_key`.

The function’s control flow is strictly sequential: each step depends on the preceding one, and the cache key is the combined output of the earlier computations. The dependencies `normalize_entry_file`, `build_compile_signature_impl`, and `try_hash_source_file` are called exactly once, and their results are stored directly into the entry’s fields.

## Side Effects

- Modifies the `CompileEntry` object by setting its `normalized_file`, `compile_signature`, `source_hash`, and `cache_key` fields.
- Reads the source file via `try_hash_source_file`, which performs I/O.
- Calls `normalize_entry_file` and `build_compile_signature_impl`, which may have their own side effects such as file access or computation.

## Reads From

- the `entry` parameter (its existing fields)
- the source file identified by `entry.normalized_file` via `try_hash_source_file`
- the compile entry data used by `build_compile_signature_impl`

## Writes To

- the `normalized_file` field of `entry`
- the `compile_signature` field of `entry`
- the `source_hash` field of `entry`
- the `cache_key` field of `entry`

## Usage Patterns

- Called by `clore::extract::ensure_cache_key` to populate cache-related fields on a `CompileEntry`.

## Calls

- function `clore::extract::(anonymous namespace)::build_compile_signature_impl`
- function `clore::extract::(anonymous namespace)::try_hash_source_file`
- function `clore::extract::normalize_entry_file`

## Called By

- function `clore::extract::ensure_cache_key`

