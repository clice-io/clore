---
title: 'clore::generate::cache::normalizetextforhashing'
description: 'clore::generate::cache::normalize_text_for_hashing accepts a std::string_view representing text such as a prompt or system prompt, and returns a std::string that has been normalized to a canonical form. This normalized form is intended to be used as input for hashing when constructing cache keys, ensuring that semantically equivalent text yields the same hash regardless of inconsequential variations like whitespace or formatting. Callers rely on this function to produce a consistent representation of textual inputs before combining them into a cache key in make_prompt_response_cache_key.'
layout: doc
template: doc
---

# `clore::generate::cache::normalize_text_for_hashing`

Owner: [Namespace clore::generate::cache](../index.md)

Declaration: `src/generate/cache.cppm:211`

Definition: `src/generate/cache.cppm:211`

Implementation: [`Module generate:cache`](../../../../../modules/generate/cache.md)

## Declaration

```cpp
auto (std::string_view) -> std::string;
```

`clore::generate::cache::normalize_text_for_hashing` accepts a `std::string_view` representing text such as a prompt or system prompt, and returns a `std::string` that has been normalized to a canonical form. This normalized form is intended to be used as input for hashing when constructing cache keys, ensuring that semantically equivalent text yields the same hash regardless of inconsequential variations like whitespace or formatting. Callers rely on this function to produce a consistent representation of textual inputs before combining them into a cache key in `make_prompt_response_cache_key`.

## Usage Patterns

- Called by `make_prompt_response_cache_key` to normalize prompt and response texts before generating the cache key

## Called By

- function `clore::generate::cache::make_prompt_response_cache_key`

