---
title: 'clore::support::normalizepathstring'
description: 'clore::support::normalize_path_string accepts a path string and returns a normalized form suitable for use as a stable key in hashing or equality comparisons. The caller may provide a path in any typical filesystem format; the returned std::string is a normalized representation that ensures consistent results regardless of input variations such as separator style, case (on case-insensitive systems), or redundant components. This normalization is employed internally by clore::support::build_compile_signature to produce deterministic compile‑signature keys.'
layout: doc
template: doc
---

# `clore::support::normalize_path_string`

Owner: [Namespace clore::support](../index.md)

Declaration: `src/support/logging.cppm:87`

Definition: `src/support/logging.cppm:371`

Implementation: [`Module support`](../../../../modules/support/index.md)

## Declaration

```cpp
auto (std::string_view) -> std::string;
```

`clore::support::normalize_path_string` accepts a path string and returns a normalized form suitable for use as a stable key in hashing or equality comparisons. The caller may provide a path in any typical filesystem format; the returned `std::string` is a normalized representation that ensures consistent results regardless of input variations such as separator style, case (on case-insensitive systems), or redundant components. This normalization is employed internally by `clore::support::build_compile_signature` to produce deterministic compile‑signature keys.

## Usage Patterns

- normalizing paths for signature computation
- ensuring consistent path representation

## Called By

- function `clore::support::build_compile_signature`

