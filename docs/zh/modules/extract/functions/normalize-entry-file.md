---
title: 'clore::extract::normalizeentryfile'
description: '该函数解析 entry.file 路径并尝试将其转换为绝对形式，随后进行词法规范化和弱规范化。具体流程为：从 std::filesystem::path 构造开始，若路径是相对路径则将其与 entry.directory 拼接；接着调用 fs::absolute 将其转为绝对路径（若转换失败则保留原路径）；然后调用 lexically_normal() 消除冗余的 .. 和 . 组件。最后尝试 fs::weakly_canonical 以解析符号链接并将结果转换为通用字符串格式返回；若弱规范化失败则直接返回词法规范化后的通用字符串。整个函数完全依赖 <filesystem> 标准库，未涉及其他内部函数或复杂错误处理。'
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

该函数解析 `entry.file` 路径并尝试将其转换为绝对形式，随后进行词法规范化和弱规范化。具体流程为：从 `std::filesystem::path` 构造开始，若路径是相对路径则将其与 `entry.directory` 拼接；接着调用 `fs::absolute` 将其转为绝对路径（若转换失败则保留原路径）；然后调用 `lexically_normal()` 消除冗余的 `..` 和 `.` 组件。最后尝试 `fs::weakly_canonical` 以解析符号链接并将结果转换为通用字符串格式返回；若弱规范化失败则直接返回词法规范化后的通用字符串。整个函数完全依赖 `<filesystem>` 标准库，未涉及其他内部函数或复杂错误处理。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `entry.file`
- `entry.directory`
- filesystem state for path resolution and canonicalization

## Usage Patterns

- used in `clore::extract::build_compile_signature` to generate a hash key
- used in `clore::extract::ensure_cache_key_impl` to normalize the entry file before caching

## Called By

- function `clore::extract::build_compile_signature`
- function `clore::extract::ensure_cache_key_impl`

