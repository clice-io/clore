---
title: 'clore::support::striputf8bom'
description: '接受一个 std::string_view 输入，返回一个新的 std::string_view，该视图指向移除头部 UTF-8 字节顺序标记（BOM）后的原始数据。如果输入不以 BOM 开头，则返回与输入相同的视图。函数不分配内存也不修改数据；调用者必须确保输入视图的生命周期覆盖返回的视图。'
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

接受一个 `std::string_view` 输入，返回一个新的 `std::string_view`，该视图指向移除头部 UTF-8 字节顺序标记（BOM）后的原始数据。如果输入不以 BOM 开头，则返回与输入相同的视图。函数不分配内存也不修改数据；调用者必须确保输入视图的生命周期覆盖返回的视图。

## Usage Patterns

- 用于从文件读取的 UTF-8 文本中剥离 BOM 前缀

## Called By

- function `clore::support::read_utf8_text_file`

