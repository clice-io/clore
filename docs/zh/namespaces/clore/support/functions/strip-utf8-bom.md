---
title: 'clore::support::striputf8bom'
description: 'clore::support::strip_utf8_bom 接受一个以 std::string_view 表示的 UTF-8 编码文本，并移除可能出现在开头的 UTF-8 字节顺序标记（BOM, 0xEF 0xBB 0xBF）。如果输入的开始处存在 BOM，函数返回指向紧随 BOM 之后内容的 std::string_view；否则返回与输入相同的 std::string_view。调用者保证输入字符串在返回的视图生命周期内保持有效，且函数本身不复制或修改底层数据。'
layout: doc
template: doc
---

# `clore::support::strip_utf8_bom`

Owner: [Namespace clore::support](../index.md)

Declaration: `support/logging.cppm:83`

Definition: `support/logging.cppm:470`

Implementation: [`Module support`](../../../../modules/support/index.md)

## Declaration

```cpp
auto (std::string_view) -> std::string_view;
```

`clore::support::strip_utf8_bom` 接受一个以 `std::string_view` 表示的 UTF-8 编码文本，并移除可能出现在开头的 UTF-8 字节顺序标记（BOM, `0xEF 0xBB 0xBF`）。如果输入的开始处存在 BOM，函数返回指向紧随 BOM 之后内容的 `std::string_view`；否则返回与输入相同的 `std::string_view`。调用者保证输入字符串在返回的视图生命周期内保持有效，且函数本身不复制或修改底层数据。

## Usage Patterns

- used by `clore::support::read_utf8_text_file` to remove a BOM before further text processing

## Called By

- function `clore::support::read_utf8_text_file`

