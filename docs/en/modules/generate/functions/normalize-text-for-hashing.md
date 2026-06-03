---
title: 'clore::generate::cache::normalizetextforhashing'
description: 'The function clore::generate::cache::normalize_text_for_hashing normalizes input text to a canonical form suitable for generating hash-based cache keys. It first trims all leading whitespace by advancing a start index past any characters for which std::isspace returns true. It then iterates over the remaining characters, using a prev_space flag to collapse any contiguous sequence of whitespace characters into a single space character inserted just before the next non‑space character. If the very first non‑space character after trimming would have been preceded by a space, the space is omitted because result is empty at that point; this effectively suppresses leading whitespace that was already trimmed. The resulting string contains no leading whitespace, no repeated spaces, and no trailing whitespace (because trailing whitespace is skipped by the collapse logic and not added back). Normalization relies solely on the C++ locale‑independent std::isspace check after casting each character to unsigned char, and uses only standard library facilities.'
layout: doc
template: doc
---

# `clore::generate::cache::normalize_text_for_hashing`

Owner: [Module generate:cache](../cache.md)

Declaration: `src/generate/cache.cppm:211`

Definition: `src/generate/cache.cppm:211`

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

The function `clore::generate::cache::normalize_text_for_hashing` normalizes input text to a canonical form suitable for generating hash-based cache keys. It first trims all leading whitespace by advancing a `start` index past any characters for which `std::isspace` returns true. It then iterates over the remaining characters, using a `prev_space` flag to collapse any contiguous sequence of whitespace characters into a single space character inserted just before the next non‑space character. If the very first non‑space character after trimming would have been preceded by a space, the space is omitted because `result` is empty at that point; this effectively suppresses leading whitespace that was already trimmed. The resulting string contains no leading whitespace, no repeated spaces, and no trailing whitespace (because trailing whitespace is skipped by the collapse logic and not added back). Normalization relies solely on the C++ locale‑independent `std::isspace` check after casting each character to `unsigned char`, and uses only standard library facilities.

## Side Effects

- Allocates a new string and returns it

## Reads From

- text parameter

## Writes To

- returned result string

## Usage Patterns

- Called by `make_prompt_response_cache_key` to normalize prompt and response texts before generating the cache key

## Called By

- function `clore::generate::cache::make_prompt_response_cache_key`

