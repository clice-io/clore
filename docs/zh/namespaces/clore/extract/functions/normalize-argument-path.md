---
title: 'clore::extract::normalizeargumentpath'
description: 'clore::extract::normalize_argument_path 接受两个 std::string_view 参数，返回一个 std::filesystem::path。调用者应提供原始路径字符串及上下文（如基目录或工作目录），函数返回该路径的规范化形式，例如解析相对路径、去除冗余分隔符或统一格式。该函数被 clore::extract::sanitize_driver_arguments 用于处理编译参数中的路径，确保后续路径比较和匹配的一致性。'
layout: doc
template: doc
---

# `clore::extract::normalize_argument_path`

Owner: [Namespace clore::extract](../index.md)

Declaration: `src/extract/compiler.cppm:65`

Definition: `src/extract/compiler.cppm:204`

Implementation: [`Module extract:compiler`](../../../../modules/extract/compiler.md)

## Declaration

```cpp
auto (std::string_view, std::string_view) -> std::filesystem::path;
```

`clore::extract::normalize_argument_path` 接受两个 `std::string_view` 参数，返回一个 `std::filesystem::path`。调用者应提供原始路径字符串及上下文（如基目录或工作目录），函数返回该路径的规范化形式，例如解析相对路径、去除冗余分隔符或统一格式。该函数被 `clore::extract::sanitize_driver_arguments` 用于处理编译参数中的路径，确保后续路径比较和匹配的一致性。

## Usage Patterns

- Used by `clore::extract::sanitize_driver_arguments` to normalize path arguments from compilation entries.

## Called By

- function `clore::extract::sanitize_driver_arguments`

