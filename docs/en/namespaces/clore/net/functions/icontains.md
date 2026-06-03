---
title: 'clore::net::icontains'
description: 'clore::net::icontains returns true if the first std::string_view contains the second std::string_view as a substring, ignoring case. The function accepts two views and performs a case‑insensitive substring search.'
layout: doc
template: doc
---

# `clore::net::icontains`

Owner: [Namespace clore::net](../index.md)

Declaration: `src/network/protocol.cppm:780`

Definition: `src/network/protocol.cppm:780`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

## Declaration

```cpp
auto (std::string_view, std::string_view) -> bool;
```

`clore::net::icontains` returns `true` if the first `std::string_view` contains the second `std::string_view` as a substring, ignoring case. The function accepts two views and performs a case‑insensitive substring search.

It is designed as a utility for pattern‑matching within diagnostic or error strings, such as determining whether an error message indicates a known condition. The caller supplies a full string (typically an error message) and a needle to search for, and `icontains` reports whether the needle appears anywhere in the input, disregarding character casing.

## Usage Patterns

- Used in `is_feature_rejection_error` to perform case-insensitive substring checks on error messages.

## Called By

- function `clore::net::is_feature_rejection_error`

