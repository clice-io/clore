---
title: 'clore::generate::cache::normalizetextforhashing'
description: 'The implementation of clore::generate::cache::normalize_text_for_hashing applies a two‑pass normalization to an arbitrary input text to produce a canonical form suitable for hashing in cache‑key construction. The first pass trims all leading whitespace characters using std::isspace. The second pass iterates over the remaining characters and collapses any contiguous run of whitespace into a single ASCII space character ('' ''). The internal control flow uses a boolean flag prev_space to track whether the previous character was whitespace; when a non‑space character is encountered and prev_space is true, a single space is appended to the result std::string only if the result is not empty. The function depends only on the C++ standard library, specifically the <cctype> facilities via std::isspace, and uses result.reserve(text.size()) to minimise reallocations. This normalisation ensures that differing amounts of whitespace do not produce distinct hash keys, while preserving the distinction between words separated by any amount of whitespace versus no whitespace.'
layout: doc
template: doc
---

# `clore::generate::cache::normalize_text_for_hashing`

Owner: [Module generate:cache](../cache.md)

Declaration: `generate/cache.cppm:192`

Definition: `generate/cache.cppm:192`

Declaration: [`Namespace clore::generate::cache`](../../../namespaces/clore/generate/cache/index.md)

## Implementation

```cpp
auto normalize_text_for_hashing(std::string_view text) -> std::string {
    std::string result;
    result.reserve(text.size());

    // Trim leading whitespace
    std::size_t start = 0;
    while(start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) {
        ++start;
    }

    bool prev_space = false;
    for(std::size_t i = start; i < text.size(); ++i) {
        auto ch = static_cast<unsigned char>(text[i]);
        if(std::isspace(ch)) {
            prev_space = true;
        } else {
            if(prev_space && !result.empty()) {
                result.push_back(' ');
            }
            result.push_back(static_cast<char>(ch));
            prev_space = false;
        }
    }

    return result;
}
```

The implementation of `clore::generate::cache::normalize_text_for_hashing` applies a two‑pass normalization to an arbitrary input `text` to produce a canonical form suitable for hashing in cache‑key construction. The first pass trims all leading whitespace characters using `std::isspace`. The second pass iterates over the remaining characters and collapses any contiguous run of whitespace into a single ASCII space character (`' '`). The internal control flow uses a boolean flag `prev_space` to track whether the previous character was whitespace; when a non‑space character is encountered and `prev_space` is true, a single space is appended to the result `std::string` only if the result is not empty. The function depends only on the C++ standard library, specifically the `<cctype>` facilities via `std::isspace`, and uses `result.reserve(text.size())` to minimise reallocations. This normalisation ensures that differing amounts of whitespace do not produce distinct hash keys, while preserving the distinction between words separated by any amount of whitespace versus no whitespace.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- the `text` parameter

## Usage Patterns

- Used by `make_prompt_response_cache_key` to normalize text before forming a cache key

## Called By

- function `clore::generate::cache::make_prompt_response_cache_key`

