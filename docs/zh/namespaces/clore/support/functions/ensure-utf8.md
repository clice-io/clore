---
title: 'clore::support::ensureutf8'
description: '函数 clore::support::ensure_utf8 接受一个 std::string_view 输入，返回一个 std::string，其内容保证为正确的 UTF‑8 编码。调用方可以依赖返回值在需要 UTF‑8 合规性的场景（如写入 UTF‑8 文本文件或进行 UTF‑8 字符串操作）中安全使用；若输入已为有效 UTF‑8，则实现可能直接生成副本，否则会修正无效序列以达到 UTF‑8 合规。'
layout: doc
template: doc
---

# `clore::support::ensure_utf8`

Owner: [Namespace clore::support](../index.md)

Declaration: `support/logging.cppm:75`

Definition: `support/logging.cppm:405`

Implementation: [`Module support`](../../../../modules/support/index.md)

## Declaration

```cpp
auto (std::string_view) -> std::string;
```

函数 `clore::support::ensure_utf8` 接受一个 `std::string_view` 输入，返回一个 `std::string`，其内容保证为正确的 UTF‑8 编码。调用方可以依赖返回值在需要 UTF‑8 合规性的场景（如写入 UTF‑8 文本文件或进行 UTF‑8 字符串操作）中安全使用；若输入已为有效 UTF‑8，则实现可能直接生成副本，否则会修正无效序列以达到 UTF‑8 合规。

## Usage Patterns

- called by `write_utf8_text_file` to ensure valid UTF-8 before writing
- called by `truncate_utf8` to sanitize input before truncation

## Calls

- function `clore::support::(anonymous namespace)::valid_utf8_sequence_length`

## Called By

- function `clore::support::truncate_utf8`
- function `clore::support::write_utf8_text_file`

