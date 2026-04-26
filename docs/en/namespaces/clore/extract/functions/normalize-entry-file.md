---
title: 'clore::extract::normalizeentryfile'
description: 'The function clore::extract::normalize_entry_file accepts a const CompileEntry & and returns a std::string representing the resolved, canonical path to the source file associated with that entry. Callers rely on this function to obtain a normalized file identifier that serves as a stable, comparable key for caching and deduplication. The returned string is used by clore::extract::build_compile_signature and clore::extract::ensure_cache_key_impl to compute compile signatures and cache entries; when the entry.normalized_file field is empty or the compile signature has not yet been computed, this function is invoked to derive the normalized file path before proceeding with further processing. The contract guarantees a consistent, file‑system‑independent representation suitable for look‑up in the compilation database and for use as part of a cache key.'
layout: doc
template: doc
---

# `clore::extract::normalize_entry_file`

Owner: [Namespace clore::extract](../index.md)

Declaration: `extract/compiler.cppm:56`

Definition: `extract/compiler.cppm:91`

Implementation: [`Module extract:compiler`](../../../../modules/extract/compiler.md)

## Declaration

```cpp
auto (const CompileEntry &) -> std::string;
```

The function `clore::extract::normalize_entry_file` accepts a `const CompileEntry &` and returns a `std::string` representing the resolved, canonical path to the source file associated with that entry. Callers rely on this function to obtain a normalized file identifier that serves as a stable, comparable key for caching and deduplication. The returned string is used by `clore::extract::build_compile_signature` and `clore::extract::ensure_cache_key_impl` to compute compile signatures and cache entries; when the `entry.normalized_file` field is empty or the compile signature has not yet been computed, this function is invoked to derive the normalized file path before proceeding with further processing. The contract guarantees a consistent, file‑system‑independent representation suitable for look‑up in the compilation database and for use as part of a cache key.

## Usage Patterns

- Called by `build_compile_signature` to derive a unique signature for a compile entry.
- Called by `ensure_cache_key_impl` to produce a normalized file path for cache key computation.

## Called By

- function `clore::extract::build_compile_signature`
- function `clore::extract::ensure_cache_key_impl`

