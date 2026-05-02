---
title: 'clore::generate::cache::normalizetextforhashing'
description: 'The function clore::generate::cache::normalize_text_for_hashing accepts a std::string_view and returns a std::string. Its responsibility is to transform an arbitrary text input into a canonical, deterministic form suitable for use as input to a hashing or key‑derivation step. Callers rely on this normalization to ensure that semantically equivalent texts produce the same normalized output, regardless of superficial formatting differences (such as extra whitespace, casing, or control characters).'
layout: doc
template: doc
---

# `clore::generate::cache::normalize_text_for_hashing`

Owner: [Namespace clore::generate::cache](../index.md)

Declaration: `generate/cache.cppm:192`

Definition: `generate/cache.cppm:192`

Implementation: [`Module generate:cache`](../../../../../modules/generate/cache.md)

## Declaration

```cpp
auto (std::string_view) -> std::string;
```

The function `clore::generate::cache::normalize_text_for_hashing` accepts a `std::string_view` and returns a `std::string`. Its responsibility is to transform an arbitrary text input into a canonical, deterministic form suitable for use as input to a hashing or key‑derivation step. Callers rely on this normalization to ensure that semantically equivalent texts produce the same normalized output, regardless of superficial formatting differences (such as extra whitespace, casing, or control characters).

This function is a low‑level utility invoked during cache‑key construction. It is used internally by `clore::generate::cache::make_prompt_response_cache_key` to produce a stable hash key for prompt–response pairs. The contract guarantees that the returned string is a well‑defined, repeatable representation of the original text; the caller must not assume any particular transform (e.g., lower‑casing, trimming) beyond the general property that identical inputs always yield identical outputs.

## Usage Patterns

- Used by `make_prompt_response_cache_key` to normalize text before forming a cache key

## Called By

- function `clore::generate::cache::make_prompt_response_cache_key`

