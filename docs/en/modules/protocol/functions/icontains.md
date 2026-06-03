---
title: 'clore::net::icontains'
description: 'The function clore::net::icontains performs a case‑insensitive substring search over two std::string_view parameters haystack and needle. The algorithm first short‑circuits when needle.size() exceeds haystack.size(), immediately returning false. Otherwise, it slides a window of size needle.size() across haystack using a pair of nested loops: the outer loop advances a starting index i up to haystack.size() - needle.size(), and the inner loop compares each corresponding character after normalizing both to lowercase via std::tolower with a cast to unsigned char to avoid undefined behavior for negative char values. If all characters in a window match, the function returns true; if no window completes, it returns false.'
layout: doc
template: doc
---

# `clore::net::icontains`

Owner: [Module protocol](../index.md)

Declaration: `src/network/protocol.cppm:780`

Definition: `src/network/protocol.cppm:780`

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

The function `clore::net::icontains` performs a case‑insensitive substring search over two `std::string_view` parameters `haystack` and `needle`. The algorithm first short‑circuits when `needle.size()` exceeds `haystack.size()`, immediately returning `false`. Otherwise, it slides a window of size `needle.size()` across `haystack` using a pair of nested loops: the outer loop advances a starting index `i` up to `haystack.size() - needle.size()`, and the inner loop compares each corresponding character after normalizing both to lowercase via `std::tolower` with a cast to `unsigned char` to avoid undefined behavior for negative `char` values. If all characters in a window match, the function returns `true`; if no window completes, it returns `false`.

The implementation relies only on the C++ standard library (`std::tolower`, `std::size_t`, `std::string_view`) and contains no project‑specific dependencies. Its control flow is linear: an early size guard, a forward scan with a character‑by‑character equality check, and an immediate early exit on success.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `haystack`
- `needle`

## Usage Patterns

- Used in `is_feature_rejection_error` to perform case-insensitive substring checks on error messages.

## Called By

- function `clore::net::is_feature_rejection_error`

