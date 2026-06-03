---
title: 'clore::extract::normalizeentryfile'
description: '该函数负责将编译条目中的源文件路径规范化为一个稳定、可移植的字符串表示。它首先将 entry.file 构造为 std::filesystem::path，若路径为相对路径，则将其与 entry.directory 拼接。随后调用 fs::absolute 并忽略错误地转为绝对路径，再执行 lexically_normal() 去除冗余的 . 与 .. 组件。接着尝试 fs::weakly_canonical 以解析符号链接并进一步归一化（该调用可能失败，例如路径不存在时）。成功时返回结果的 generic_string()；若 weakly_canonical 失败，则回退返回 lexically_normal 之后的通用格式字符串。整个过程仅依赖标准文件系统库与 CompileEntry 的两个字段 file 和 directory，不涉及外部数据库或缓存。'
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

该函数负责将编译条目中的源文件路径规范化为一个稳定、可移植的字符串表示。它首先将 `entry.file` 构造为 `std::filesystem::path`，若路径为相对路径，则将其与 `entry.directory` 拼接。随后调用 `fs::absolute` 并忽略错误地转为绝对路径，再执行 `lexically_normal()` 去除冗余的 `.` 与 `..` 组件。接着尝试 `fs::weakly_canonical` 以解析符号链接并进一步归一化（该调用可能失败，例如路径不存在时）。成功时返回结果的 `generic_string()`；若 `weakly_canonical` 失败，则回退返回 `lexically_normal` 之后的通用格式字符串。整个过程仅依赖标准文件系统库与 `CompileEntry` 的两个字段 `file` 和 `directory`，不涉及外部数据库或缓存。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `entry.file` and `entry.directory` members of the `CompileEntry` parameter
- Filesystem via `std::filesystem::absolute` and `std::filesystem::weakly_canonical`

## Usage Patterns

- Called by `clore::extract::build_compile_signature` to produce a normalized file string for hashing
- Called by `clore::extract::ensure_cache_key_impl` to normalize the entry file path before caching

## Called By

- function `clore::extract::build_compile_signature`
- function `clore::extract::ensure_cache_key_impl`

