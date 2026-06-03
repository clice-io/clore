---
title: 'clore::extract::canonicalgraphpath'
description: '将给定的文件系统路径转换为一个规范化（canonical）的字符串，该字符串适合在图结构中作为唯一标识符使用。返回的 std::string 是绝对且已解析符号链接等冗余的路径表示，用于 clore::extract::matches_filter 等依赖它的函数进行高效匹配与过滤。调用者应确保传入的 std::filesystem::path 是有效的，且结果字符串可用于跨平台一致地与 config::FilterRule 中的模式进行比较。'
layout: doc
template: doc
---

# `clore::extract::canonical_graph_path`

Owner: [Namespace clore::extract](../index.md)

Declaration: `src/extract/filter.cppm:30`

Definition: `src/extract/filter.cppm:112`

Implementation: [`Module extract:filter`](../../../../modules/extract/filter.md)

## Declaration

```cpp
auto (const std::filesystem::path &) -> std::string;
```

将给定的文件系统路径转换为一个规范化（canonical）的字符串，该字符串适合在图结构中作为唯一标识符使用。返回的 `std::string` 是绝对且已解析符号链接等冗余的路径表示，用于 `clore::extract::matches_filter` 等依赖它的函数进行高效匹配与过滤。调用者应确保传入的 `std::filesystem::path` 是有效的，且结果字符串可用于跨平台一致地与 `config::FilterRule` 中的模式进行比较。

## Usage Patterns

- called by `matches_filter` to obtain a consistent path for filtering

## Called By

- function `clore::extract::matches_filter`

