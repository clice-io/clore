---
title: 'clore::support::ensureutf8'
description: '接受一个 std::string_view，返回一个 std::string。该函数保证输出字符串是有效的 UTF-8 编码，无论输入是否已符合该编码。它内部会验证输入序列的合法性，并在必要时进行规范化或修复，确保调用者获得一个可以安全用于任何期望 UTF-8 文本的上下文的字符串。'
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

接受一个 `std::string_view`，返回一个 `std::string`。该函数保证输出字符串是有效的 UTF-8 编码，无论输入是否已符合该编码。它内部会验证输入序列的合法性，并在必要时进行规范化或修复，确保调用者获得一个可以安全用于任何期望 UTF-8 文本的上下文的字符串。

## Usage Patterns

- 在输出或进一步处理前清理字符串
- 被 `write_utf8_text_file` 和 `truncate_utf8` 调用

## Calls

- function `clore::support::(anonymous namespace)::valid_utf8_sequence_length`

## Called By

- function `clore::support::truncate_utf8`
- function `clore::support::write_utf8_text_file`

