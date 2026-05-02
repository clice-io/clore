---
title: 'clore::net::icontains'
description: 'The function clore::net::icontains determines whether the first std::string_view argument contains the second std::string_view argument as a substring, performing a case‑insensitive comparison. It returns true if the second string is found within the first, and false otherwise. Callers can rely on this function for matching patterns in text without regard to letter case.'
layout: doc
template: doc
---

# `clore::net::icontains`

Owner: [Namespace clore::net](../index.md)

Declaration: `network/protocol.cppm:768`

Definition: `network/protocol.cppm:768`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

## Declaration

```cpp
auto (std::string_view, std::string_view) -> bool;
```

The function `clore::net::icontains` determines whether the first `std::string_view` argument contains the second `std::string_view` argument as a substring, performing a case‑insensitive comparison. It returns `true` if the second string is found within the first, and `false` otherwise. Callers can rely on this function for matching patterns in text without regard to letter case.

## Usage Patterns

- invoked by `clore::net::is_feature_rejection_error` to detect feature-rejection keywords inside error message text
- general case-insensitive substring matching within the `clore::net` module

## Called By

- function `clore::net::is_feature_rejection_error`

