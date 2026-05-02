---
title: 'clore::extract::normalizeentryfile'
description: 'clore::extract::normalize_entry_file accepts a const CompileEntry & and returns a std::string representing the normalized source file path for that entry. The function is responsible for producing a canonical, consistent form of the file path used by the caller to enable reliable caching, signature computation, and key derivation. Callers rely on this normalized path as a prerequisite for operations such as build signature calculation and cache entry construction. The contract guarantees that the returned string uniquely identifies the entry’s source file in a platform‑independent and directory‑resolved manner.'
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

`clore::extract::normalize_entry_file` accepts a `const CompileEntry &` and returns a `std::string` representing the normalized source file path for that entry. The function is responsible for producing a canonical, consistent form of the file path used by the caller to enable reliable caching, signature computation, and key derivation. Callers rely on this normalized path as a prerequisite for operations such as build signature calculation and cache entry construction. The contract guarantees that the returned string uniquely identifies the entry’s source file in a platform‑independent and directory‑resolved manner.

## Usage Patterns

- Used by `build_compile_signature` to normalize the entry file path before hashing
- Used by `ensure_cache_key_impl` to produce a consistent file path representation

## Called By

- function `clore::extract::build_compile_signature`
- function `clore::extract::ensure_cache_key_impl`

