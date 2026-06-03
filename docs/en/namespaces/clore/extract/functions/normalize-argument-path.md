---
title: 'clore::extract::normalizeargumentpath'
description: 'The function clore::extract::normalize_argument_path accepts two std::string_view arguments representing a base directory and a path string, and returns a std::filesystem::path. It resolves the given path relative to the base directory and produces a normalized, absolute filesystem path. Callers can rely on the result being in a canonical form suitable for subsequent comparison or deduplication of file arguments within a compilation entry.'
layout: doc
template: doc
---

# `clore::extract::normalize_argument_path`

Owner: [Namespace clore::extract](../index.md)

Declaration: `src/extract/compiler.cppm:65`

Definition: `src/extract/compiler.cppm:204`

Implementation: [`Module extract:compiler`](../../../../modules/extract/compiler.md)

## Declaration

```cpp
auto (std::string_view, std::string_view) -> std::filesystem::path;
```

The function `clore::extract::normalize_argument_path` accepts two `std::string_view` arguments representing a base directory and a path string, and returns a `std::filesystem::path`. It resolves the given path relative to the base directory and produces a normalized, absolute filesystem path. Callers can rely on the result being in a canonical form suitable for subsequent comparison or deduplication of file arguments within a compilation entry.

## Usage Patterns

- Called from `clore::extract::sanitize_driver_arguments` to normalize file arguments

## Called By

- function `clore::extract::sanitize_driver_arguments`

