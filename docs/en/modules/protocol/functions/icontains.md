---
title: 'clore::net::icontains'
description: 'The function clore::net::icontains implements a case‑insensitive substring search using a straightforward brute‑force algorithm. It first performs an early‑exit check: if the length of needle exceeds the length of haystack, it returns false immediately. Otherwise, it iterates over each possible starting index i from 0 to haystack.size() - needle.size() inclusive. For each i, it sets a match flag to true and enters an inner loop over index j from 0 to needle.size() - 1. Inside the inner loop it compares haystack[i + j] and needle[j] after converting both to lowercase via std::tolower(static_cast<unsigned char>(…)). If any pair differs, match is set to false and the inner loop is broken. After the inner loop, if match remains true the function returns true immediately. If no starting position yields a full match, the function returns false. The only external dependency is std::tolower from the C++ standard library; no project‑specific utilities are used.'
layout: doc
template: doc
---

# `clore::net::icontains`

Owner: [Module protocol](../index.md)

Declaration: `network/protocol.cppm:758`

Definition: `network/protocol.cppm:758`

Declaration: [`Namespace clore::net`](../../../namespaces/clore/net/index.md)

## Implementation

```cpp
auto icontains(std::string_view haystack, std::string_view needle) -> bool {
    if(needle.size() > haystack.size()) {
        return false;
    }
    for(std::size_t i = 0; i <= haystack.size() - needle.size(); ++i) {
        bool match = true;
        for(std::size_t j = 0; j < needle.size(); ++j) {
            if(std::tolower(static_cast<unsigned char>(haystack[i + j])) !=
               std::tolower(static_cast<unsigned char>(needle[j]))) {
                match = false;
                break;
            }
        }
        if(match) {
            return true;
        }
    }
    return false;
}
```

The function `clore::net::icontains` implements a case‑insensitive substring search using a straightforward brute‑force algorithm. It first performs an early‑exit check: if the length of `needle` exceeds the length of `haystack`, it returns `false` immediately. Otherwise, it iterates over each possible starting index `i` from `0` to `haystack.size() - needle.size()` inclusive. For each `i`, it sets a `match` flag to `true` and enters an inner loop over index `j` from `0` to `needle.size() - 1`. Inside the inner loop it compares `haystack[i + j]` and `needle[j]` after converting both to lowercase via `std::tolower(static_cast<unsigned char>(…))`. If any pair differs, `match` is set to `false` and the inner loop is broken. After the inner loop, if `match` remains `true` the function returns `true` immediately. If no starting position yields a full match, the function returns `false`. The only external dependency is `std::tolower` from the C++ standard library; no project‑specific utilities are used.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `haystack` parameter
- `needle` parameter

## Usage Patterns

- Used by `is_feature_rejection_error` to check for substring patterns in error messages.

## Called By

- function `clore::net::is_feature_rejection_error`

