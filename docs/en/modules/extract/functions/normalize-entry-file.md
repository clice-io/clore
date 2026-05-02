---
title: 'clore::extract::normalizeentryfile'
description: 'The function clore::extract::normalize_entry_file resolves a compile entry’s file path to a stable, normalized string. It first converts entry.file into a std::filesystem::path. If the path is relative, it is prefixed with entry.directory to form an absolute path; an explicit fs::absolute call is then attempted (falling back to the prefixed path on error). The path is then run through lexically_normal() to eliminate redundant separators and dot segments. To materialize symlinks and produce a fully resolved absolute path, fs::weakly_canonical is applied; if it fails (e.g., the file does not exist), the lexically normalized path is used as the final fallback. The result is returned as a generic string via generic_string(). The algorithm relies solely on std::filesystem operations and the fields of CompileEntry (file and directory).'
layout: doc
template: doc
---

# `clore::extract::normalize_entry_file`

Owner: [Module extract:compiler](../compiler.md)

Declaration: `extract/compiler.cppm:56`

Definition: `extract/compiler.cppm:91`

Declaration: [`Namespace clore::extract`](../../../namespaces/clore/extract/index.md)

## Implementation

```cpp
auto normalize_entry_file(const CompileEntry& entry) -> std::string {
    namespace fs = std::filesystem;
    auto path = fs::path(entry.file);
    if(path.is_relative()) {
        path = fs::path(entry.directory) / path;
    }
    std::error_code ec;
    auto absolute = fs::absolute(path, ec);
    if(!ec) {
        path = std::move(absolute);
    }
    path = path.lexically_normal();
    auto canonical = fs::weakly_canonical(path, ec);
    if(!ec) {
        return canonical.generic_string();
    }
    return path.generic_string();
}
```

The function `clore::extract::normalize_entry_file` resolves a compile entry’s file path to a stable, normalized string. It first converts `entry.file` into a `std::filesystem::path`. If the path is relative, it is prefixed with `entry.directory` to form an absolute path; an explicit `fs::absolute` call is then attempted (falling back to the prefixed path on error). The path is then run through `lexically_normal()` to eliminate redundant separators and dot segments. To materialize symlinks and produce a fully resolved absolute path, `fs::weakly_canonical` is applied; if it fails (e.g., the file does not exist), the lexically normalized path is used as the final fallback. The result is returned as a generic string via `generic_string()`. The algorithm relies solely on `std::filesystem` operations and the fields of `CompileEntry` (`file` and `directory`).

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `entry.file`
- `entry.directory`

## Usage Patterns

- Used by `build_compile_signature` to normalize the entry file path before hashing
- Used by `ensure_cache_key_impl` to produce a consistent file path representation

## Called By

- function `clore::extract::build_compile_signature`
- function `clore::extract::ensure_cache_key_impl`

