---
title: 'clore::generate::cache::normalizetextforhashing'
description: 'Normalizes a given input text so that functionally equivalent strings produce an identical result, enabling consistent hashing within the caching system. The caller supplies a std::string_view and receives a std::string that has been transformed according to internal rules (for example, trimming whitespace and reducing letter case) to eliminate benign variations. This normalized string is intended to be used as part of a cache key, ensuring that the same conceptual prompt or system prompt always maps to the same hash regardless of incidental formatting differences.'
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

Normalizes a given input text so that functionally equivalent strings produce an identical result, enabling consistent hashing within the caching system. The caller supplies a `std::string_view` and receives a `std::string` that has been transformed according to internal rules (for example, trimming whitespace and reducing letter case) to eliminate benign variations. This normalized string is intended to be used as part of a cache key, ensuring that the same conceptual prompt or system prompt always maps to the same hash regardless of incidental formatting differences.

## Usage Patterns

- called by `make_prompt_response_cache_key` to normalize prompt and response text before deriving a cache key

## Called By

- function `clore::generate::cache::make_prompt_response_cache_key`

