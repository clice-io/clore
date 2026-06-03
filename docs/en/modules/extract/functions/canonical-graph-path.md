---
title: 'clore::extract::canonicalgraphpath'
description: 'The function attempts to produce a canonical, platform‑generic string representation of a filesystem path. It first tries to resolve the input via std::filesystem::absolute and then applies std::filesystem::weakly_canonical on the lexically‑normalised absolute path. If either step fails (indicated by the std::error_code ec), it falls back to returning the generic string of the absolute, lexically‑normalised path. If absolute itself fails, the function repeats the same two‑step attempt using only path.lexically_normal() as the basis, eventually returning the generic string of the normalised path as a last resort. The algorithm depends entirely on std::filesystem operations and uses ec to detect errors at each stage without throwing exceptions.'
layout: doc
template: doc
---

# `clore::extract::canonical_graph_path`

Owner: [Module extract:filter](../filter.md)

Declaration: `src/extract/filter.cppm:30`

Definition: `src/extract/filter.cppm:112`

Declaration: [`Namespace clore::extract`](../../../namespaces/clore/extract/index.md)

## Implementation

```cpp
auto canonical_graph_path(const std::filesystem::path& path) -> std::string {
    namespace fs = std::filesystem;
    std::error_code ec;
    auto absolute = fs::absolute(path, ec);
    if(!ec) {
        auto canonical = fs::weakly_canonical(absolute.lexically_normal(), ec);
        if(!ec) {
            return canonical.generic_string();
        }
        return absolute.lexically_normal().generic_string();
    }

    auto normalized = path.lexically_normal();
    auto canonical = fs::weakly_canonical(normalized, ec);
    if(!ec) {
        return canonical.generic_string();
    }

    return normalized.generic_string();
}
```

The function attempts to produce a canonical, platform‑generic string representation of a filesystem path. It first tries to resolve the input via `std::filesystem::absolute` and then applies `std::filesystem::weakly_canonical` on the lexically‑normalised absolute path. If either step fails (indicated by the `std::error_code` `ec`), it falls back to returning the generic string of the absolute, lexically‑normalised path. If `absolute` itself fails, the function repeats the same two‑step attempt using only `path.lexically_normal()` as the basis, eventually returning the generic string of the normalised path as a last resort. The algorithm depends entirely on `std::filesystem` operations and uses `ec` to detect errors at each stage without throwing exceptions.

## Side Effects

- Allocates a `std::string` for the return value
- Performs filesystem reads to resolve symlinks and determine the current working directory via absolute and `weakly_canonical`

## Reads From

- Input path parameter
- Filesystem state via `std::filesystem::absolute` and `std::filesystem::weakly_canonical`
- Current working directory implicitly via `std::filesystem::absolute`

## Usage Patterns

- Used by `clore::extract::matches_filter` to obtain a canonical string representation of a path for comparison

## Called By

- function `clore::extract::matches_filter`

