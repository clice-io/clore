---
title: 'clore::extract::normalizeentryfile'
description: 'Given a CompileEntry, clore::extract::normalize_entry_file produces a canonical std::string representing the entry''s source file path. The caller can rely on the returned string being suitable as a stable, normalized key for caching and signature computation; it is used by clore::extract::build_compile_signature and clore::extract::ensure_cache_key_impl to populate the entry''s normalized_file field. The function expects the CompileEntry to contain a valid file reference.'
layout: doc
template: doc
---

# `clore::extract::normalize_entry_file`

Owner: [Namespace clore::extract](../index.md)

Declaration: `src/extract/compiler.cppm:72`

Definition: `src/extract/compiler.cppm:107`

Implementation: [`Module extract:compiler`](../../../../modules/extract/compiler.md)

## Declaration

```cpp
auto (const CompileEntry &) -> std::string;
```

Given a `CompileEntry`, `clore::extract::normalize_entry_file` produces a canonical `std::string` representing the entry's source file path. The caller can rely on the returned string being suitable as a stable, normalized key for caching and signature computation; it is used by `clore::extract::build_compile_signature` and `clore::extract::ensure_cache_key_impl` to populate the entry's `normalized_file` field. The function expects the `CompileEntry` to contain a valid file reference.

## Usage Patterns

- normalize file path for compile entries
- used in `build_compile_signature`
- used in `ensure_cache_key_impl`

## Called By

- function `clore::extract::build_compile_signature`
- function `clore::extract::ensure_cache_key_impl`

