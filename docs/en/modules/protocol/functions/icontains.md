---
title: 'clore::net::icontains'
description: 'The function clore::net::icontains performs a case‑insensitive substring search using a brute‑force sliding‑window algorithm. It first rejects empty or oversized needles with an early return when needle.size() exceeds haystack.size(). The outer loop iterates over every possible starting offset i in haystack, up to haystack.size() - needle.size(). For each offset, an inner loop compares the needle character by character; both characters are lowercased via std::tolower with an explicit cast to unsigned char to avoid implementation‑defined behavior on negative char values. If all positions match, the function returns true; otherwise it continues searching. If no offset produces a full match, it returns false. The implementation relies solely on the standard library (function template std::tolower, class std::string_view); no locale or external data structures are involved, and the algorithm completes in O(n·m) time where n is the length of haystack and m the length of needle.'
layout: doc
template: doc
---

# `clore::net::icontains`

Owner: [Module protocol](../index.md)

Declaration: `network/protocol.cppm:768`

Definition: `network/protocol.cppm:768`

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

The function `clore::net::icontains` performs a case‑insensitive substring search using a brute‑force sliding‑window algorithm. It first rejects empty or oversized needles with an early return when `needle.size()` exceeds `haystack.size()`. The outer loop iterates over every possible starting offset `i` in `haystack`, up to `haystack.size() - needle.size()`. For each offset, an inner loop compares the `needle` character by character; both characters are lowercased via `std::tolower` with an explicit cast to `unsigned char` to avoid implementation‑defined behavior on negative char values. If all positions match, the function returns `true`; otherwise it continues searching. If no offset produces a full match, it returns `false`. The implementation relies solely on the standard library (function template `std::tolower`, class `std::string_view`); no locale or external data structures are involved, and the algorithm completes in `O(n·m)` time where `n` is the length of `haystack` and `m` the length of `needle`.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- parameter `haystack` contents and size
- parameter `needle` contents and size

## Usage Patterns

- invoked by `clore::net::is_feature_rejection_error` to detect feature-rejection keywords inside error message text
- general case-insensitive substring matching within the `clore::net` module

## Called By

- function `clore::net::is_feature_rejection_error`

