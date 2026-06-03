---
title: 'clore::extract::canonicalgraphpath'
description: '该函数通过分层回退策略将任意文件系统路径转换为规范的图形路径表示形式。首先尝试使用 std::filesystem::absolute 和 std::filesystem::weakly_canonical 解析路径，同时通过 std::error_code 捕获错误。若成功，则返回弱规范化路径的 generic_string；若失败，则回退到绝对路径的 lexically_normal 版本。如果绝对路径解析本身失败，则直接对输入路径进行 lexically_normal，再次尝试 weakly_canonical，最终返回最有效的规范化结果。整个过程避免异常抛出，完全依赖于 <filesystem> 库提供的操作系统级路径解析能力。'
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

该函数通过分层回退策略将任意文件系统路径转换为规范的图形路径表示形式。首先尝试使用 `std::filesystem::absolute` 和 `std::filesystem::weakly_canonical` 解析路径，同时通过 `std::error_code` 捕获错误。若成功，则返回弱规范化路径的 `generic_string`；若失败，则回退到绝对路径的 `lexically_normal` 版本。如果绝对路径解析本身失败，则直接对输入路径进行 `lexically_normal`，再次尝试 `weakly_canonical`，最终返回最有效的规范化结果。整个过程避免异常抛出，完全依赖于 `<filesystem>` 库提供的操作系统级路径解析能力。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- filesystem via `fs::absolute` (reads current working directory and path components)
- filesystem via `fs::weakly_canonical` (reads symlinks and directory existence)

## Usage Patterns

- called by `matches_filter` to obtain a consistent path for filtering

## Called By

- function `clore::extract::matches_filter`

