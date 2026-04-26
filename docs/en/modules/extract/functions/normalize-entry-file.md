---
title: 'clore::extract::normalizeentryfile'
description: 'The function clore::extract::normalize_entry_file takes a CompileEntry and returns a normalized file path as a std::string. It begins by constructing a std::filesystem::path from entry.file. If the path is relative, it is prefixed with entry.directory. After converting to an absolute path using fs::absolute (with error checking), the path is normalized via lexically_normal(). A subsequent call to fs::weakly_canonical attempts to resolve any symbolic links and produce a canonical path; if this fails (e.g., the file does not exist), the function falls back to the lexically-normalized path. The result is always returned as a generic string (forward slashes).'
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

The function `clore::extract::normalize_entry_file` takes a `CompileEntry` and returns a normalized file path as a `std::string`. It begins by constructing a `std::filesystem::path` from `entry.file`. If the path is relative, it is prefixed with `entry.directory`. After converting to an absolute path using `fs::absolute` (with error checking), the path is normalized via `lexically_normal()`. A subsequent call to `fs::weakly_canonical` attempts to resolve any symbolic links and produce a canonical path; if this fails (e.g., the file does not exist), the function falls back to the lexically-normalized path. The result is always returned as a generic string (forward slashes).

Internally, the function relies solely on the `std::filesystem` library and the `CompileEntry` fields `file` and `directory`. Error codes from `weakly_canonical` are captured but not propagated—the function degrades gracefully to the non-canonical form. This ensures robust behavior when the input file does not exist on disk or when filesystem resolution fails. The algorithm prioritizes accuracy (canonical form when possible) over strict failure reporting.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `entry.file`
- `entry.directory`

## Usage Patterns

- Called by `build_compile_signature` to derive a unique signature for a compile entry.
- Called by `ensure_cache_key_impl` to produce a normalized file path for cache key computation.

## Called By

- function `clore::extract::build_compile_signature`
- function `clore::extract::ensure_cache_key_impl`

