---
title: 'clore::support::striputf8bom'
description: '如果输入以 UTF‑8 字节顺序标记（U+FEFF，编码为 \xEF\xBB\xBF）开头，clore::support::strip_utf8_bom 返回一个指向紧随 BOM 之后的子串的 std::string_view；否则原样返回输入的视图。此函数不进行任何内存分配或修改。调用者有责任确保传入的 std::string_view 所引用的字符缓冲区的生命周期在返回值的使用期间内保持有效。'
layout: doc
template: doc
---

# `clore::support::strip_utf8_bom`

Owner: [Namespace clore::support](../index.md)

Declaration: `src/support/logging.cppm:106`

Definition: `src/support/logging.cppm:493`

Implementation: [`Module support`](../../../../modules/support/index.md)

## Declaration

```cpp
auto (std::string_view) -> std::string_view;
```

如果输入以 UTF‑8 字节顺序标记（U+FEFF，编码为 `\xEF\xBB\xBF`）开头，`clore::support::strip_utf8_bom` 返回一个指向紧随 BOM 之后的子串的 `std::string_view`；否则原样返回输入的视图。此函数不进行任何内存分配或修改。调用者有责任确保传入的 `std::string_view` 所引用的字符缓冲区的生命周期在返回值的使用期间内保持有效。

## Usage Patterns

- Stripping BOM from text loaded by `clore::support::read_utf8_text_file`

## Called By

- function `clore::support::read_utf8_text_file`

