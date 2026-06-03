---
title: 'clore::extract::canonicalgraphpath'
description: 'The function clore::extract::canonical_graph_path accepts a const std::filesystem::path & and returns a std::string. Its responsibility is to produce a canonical, portable string representation of the given path that can be used as a stable key or for equality comparisons—for example, when determining whether a file path matches filtering rules relative to a root directory. Callers should supply a valid filesystem path; the returned string is normalized so that semantically identical paths yield the same value, regardless of platform‑specific separators or symlink states. This guarantees consistent, predictable lookup and filtering behaviour across the extraction pipeline.'
layout: doc
template: doc
---

# `clore::extract::canonical_graph_path`

Owner: [Namespace clore::extract](../index.md)

Declaration: `src/extract/filter.cppm:30`

Definition: `src/extract/filter.cppm:112`

Implementation: [`Module extract:filter`](../../../../modules/extract/filter.md)

## Declaration

```cpp
auto (const std::filesystem::path &) -> std::string;
```

The function `clore::extract::canonical_graph_path` accepts a `const std::filesystem::path &` and returns a `std::string`. Its responsibility is to produce a canonical, portable string representation of the given path that can be used as a stable key or for equality comparisons—for example, when determining whether a file path matches filtering rules relative to a root directory. Callers should supply a valid filesystem path; the returned string is normalized so that semantically identical paths yield the same value, regardless of platform‑specific separators or symlink states. This guarantees consistent, predictable lookup and filtering behaviour across the extraction pipeline.

## Usage Patterns

- Used by `clore::extract::matches_filter` to obtain a canonical string representation of a path for comparison

## Called By

- function `clore::extract::matches_filter`

