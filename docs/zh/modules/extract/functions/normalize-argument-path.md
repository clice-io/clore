---
title: 'clore::extract::normalizeargumentpath'
description: '函数 clore::extract::normalize_argument_path 实现了一种容错路径规范化算法。它首先将输入的 path 转换为 std::filesystem::path 对象；若该路径是相对路径，则将其与 directory 连接。接着尝试调用 std::filesystem::absolute 将结果变为绝对路径（若失败则保留原值），随后调用 lexically_normal() 进行词法规范化。最后，尝试使用 weakly_canonical 解析符号链接并进一步规范化；若该操作成功则返回其结果，否则回退到之前词法规范化的路径。该设计在尽可能保证路径可解析的前提下，避免了因文件不存在而完全失败的情况，主要依赖 std::filesystem 库的四个标准化函数和一个错误码检查。'
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

函数 `clore::extract::normalize_argument_path` 实现了一种容错路径规范化算法。它首先将输入的 `path` 转换为 `std::filesystem::path` 对象；若该路径是相对路径，则将其与 `directory` 连接。接着尝试调用 `std::filesystem::absolute` 将结果变为绝对路径（若失败则保留原值），随后调用 `lexically_normal()` 进行词法规范化。最后，尝试使用 `weakly_canonical` 解析符号链接并进一步规范化；若该操作成功则返回其结果，否则回退到之前词法规范化的路径。该设计在尽可能保证路径可解析的前提下，避免了因文件不存在而完全失败的情况，主要依赖 `std::filesystem` 库的四个标准化函数和一个错误码检查。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `path` parameter
- `directory` parameter
- current working directory via `std::filesystem::absolute`

## Usage Patterns

- Used by `clore::extract::sanitize_driver_arguments` to normalize path arguments from compilation entries.

## Called By

- function `clore::extract::sanitize_driver_arguments`

