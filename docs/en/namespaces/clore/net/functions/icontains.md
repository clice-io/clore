---
title: 'clore::net::icontains'
description: 'clore::net::icontains is a free function that performs a case‑insensitive substring search. It accepts a std::string_view haystack and a std::string_view needle, and returns true if needle is found within haystack when case is ignored. The caller can rely on this function to decide whether a given substring appears anywhere in the larger text, without any restriction on position or surrounding characters. The function is intended for text‑matching tasks where upper/lowercase differences should not affect the result.'
layout: doc
template: doc
---

# `clore::net::icontains`

Owner: [Namespace clore::net](../index.md)

Declaration: `network/protocol.cppm:758`

Definition: `network/protocol.cppm:758`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

## Declaration

```cpp
auto (std::string_view, std::string_view) -> bool;
```

`clore::net::icontains` is a free function that performs a case‑insensitive substring search. It accepts a `std::string_view` `haystack` and a `std::string_view` `needle`, and returns `true` if `needle` is found within `haystack` when case is ignored. The caller can rely on this function to decide whether a given substring appears anywhere in the larger text, without any restriction on position or surrounding characters. The function is intended for text‑matching tasks where upper/lowercase differences should not affect the result.

## Usage Patterns

- Used by `is_feature_rejection_error` to check for substring patterns in error messages.

## Called By

- function `clore::net::is_feature_rejection_error`

