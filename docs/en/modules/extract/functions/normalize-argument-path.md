---
title: 'clore::extract::normalizeargumentpath'
description: 'The function constructs a normalized std::filesystem::path from the path argument using a multi-stage resolution. First, if path is relative, it is made absolute by joining it with the directory argument. An std::error_code‑based call to std::filesystem::absolute then forces an absolute representation, replacing the intermediate result on success. Lexical normalization via lexically_normal() cleans up redundant separators and .. components. Finally, std::filesystem::weakly_canonical is attempted to resolve symbolic links and produce a true canonical path; if that call fails (e.g., due to missing components), the function falls back to the lexically‑normalized path. The entire resolution depends on std::filesystem operations and std::error_code for error‑safe branching, ensuring that even when symlink resolution is impossible a usable path is returned.'
layout: doc
template: doc
---

# `clore::extract::normalize_argument_path`

Owner: [Module extract:compiler](../compiler.md)

Declaration: `src/extract/compiler.cppm:65`

Definition: `src/extract/compiler.cppm:204`

Declaration: [`Namespace clore::extract`](../../../namespaces/clore/extract/index.md)

## Implementation

```cpp
auto normalize_argument_path(std::string_view path, std::string_view directory)
    -> std::filesystem::path {
    auto normalized = std::filesystem::path(path);
    if(normalized.is_relative()) {
        normalized = std::filesystem::path(directory) / normalized;
    }
    std::error_code ec;
    auto absolute = std::filesystem::absolute(normalized, ec);
    if(!ec) {
        normalized = std::move(absolute);
    }
    normalized = normalized.lexically_normal();
    auto canonical = std::filesystem::weakly_canonical(normalized, ec);
    if(!ec) {
        return canonical;
    }
    return normalized;
}
```

The function constructs a normalized `std::filesystem::path` from the `path` argument using a multi-stage resolution. First, if `path` is relative, it is made absolute by joining it with the `directory` argument. An `std::error_code`‑based call to `std::filesystem::absolute` then forces an absolute representation, replacing the intermediate result on success. Lexical normalization via `lexically_normal()` cleans up redundant separators and `..` components. Finally, `std::filesystem::weakly_canonical` is attempted to resolve symbolic links and produce a true canonical path; if that call fails (e.g., due to missing components), the function falls back to the lexically‑normalized path. The entire resolution depends on `std::filesystem` operations and `std::error_code` for error‑safe branching, ensuring that even when symlink resolution is impossible a usable path is returned.

## Side Effects

- accesses filesystem state via `std::filesystem::absolute`
- accesses filesystem state via `std::filesystem::weakly_canonical`

## Reads From

- filesystem state for path resolution
- parameters `path` and `directory`

## Usage Patterns

- Called from `clore::extract::sanitize_driver_arguments` to normalize file arguments

## Called By

- function `clore::extract::sanitize_driver_arguments`

