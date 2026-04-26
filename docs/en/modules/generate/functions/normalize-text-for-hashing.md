---
title: 'clore::generate::cache::normalizetextforhashing'
description: 'The function first trims leading whitespace by advancing a start index past any characters for which std::isspace returns true. It then iterates over the remaining characters from start to the end of the input, using a prev_space flag to track whether the previous character was whitespace. When a non‑space character is encountered, if prev_space is true and the result string is not empty, a single space character is appended before the character; otherwise only the character is appended. This collapses any run of internal whitespace into exactly one space. The result is a std::string with leading whitespace removed and interior whitespace sequences normalized to single spaces. The implementation avoids allocating extra memory by calling reserve on the result with the original input size, and relies solely on the standard library (std::isspace, std::string, std::size_t).'
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

The function first trims leading whitespace by advancing a `start` index past any characters for which `std::isspace` returns true. It then iterates over the remaining characters from `start` to the end of the input, using a `prev_space` flag to track whether the previous character was whitespace. When a non‑space character is encountered, if `prev_space` is true and the result string is not empty, a single space character is appended before the character; otherwise only the character is appended. This collapses any run of internal whitespace into exactly one space. The result is a `std::string` with leading whitespace removed and interior whitespace sequences normalized to single spaces. The implementation avoids allocating extra memory by calling `reserve` on the result with the original input size, and relies solely on the standard library (`std::isspace`, `std::string`, `std::size_t`).

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- parameter `text`

## Writes To

- local variable `result` (returned by value)

## Usage Patterns

- called by `make_prompt_response_cache_key` to normalize prompt and response text before deriving a cache key

## Called By

- function `clore::generate::cache::make_prompt_response_cache_key`

