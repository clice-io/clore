---
title: 'clore::extract::normalizeentryfile'
description: 'The function first constructs a std::filesystem::path from entry.file. If the path is relative, it is joined with entry.directory to form an absolute path. The function then attempts to resolve the path using fs::absolute, capturing any std::error_code; if the operation succeeds, the absolute path replaces the intermediate value. Next, fs::weakly_canonical is tried—on success the result is returned as a generic string, providing a minimal, existing-path-normalized form. If weakly_canonical fails (e.g., the path does not exist on the filesystem), the function falls back to path.lexically_normal() to remove . and .. components without checking existence, and returns that generic string. The implementation relies entirely on std::filesystem primitives and the CompileEntry fields file and directory; no other project-specific functions or types are called directly.'
layout: doc
template: doc
---

# `clore::extract::normalize_entry_file`

Owner: [Module extract:compiler](../compiler.md)

Declaration: `src/extract/compiler.cppm:72`

Definition: `src/extract/compiler.cppm:107`

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

The function first constructs a `std::filesystem::path` from `entry.file`. If the path is relative, it is joined with `entry.directory` to form an absolute path. The function then attempts to resolve the path using `fs::absolute`, capturing any `std::error_code`; if the operation succeeds, the absolute path replaces the intermediate value. Next, `fs::weakly_canonical` is tried—on success the result is returned as a generic string, providing a minimal, existing-path-normalized form. If `weakly_canonical` fails (e.g., the path does not exist on the filesystem), the function falls back to `path.lexically_normal()` to remove `.` and `..` components without checking existence, and returns that generic string. The implementation relies entirely on `std::filesystem` primitives and the `CompileEntry` fields `file` and `directory`; no other project-specific functions or types are called directly.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `entry.file`
- `entry.directory`

## Usage Patterns

- normalize file path for compile entries
- used in `build_compile_signature`
- used in `ensure_cache_key_impl`

## Called By

- function `clore::extract::build_compile_signature`
- function `clore::extract::ensure_cache_key_impl`

