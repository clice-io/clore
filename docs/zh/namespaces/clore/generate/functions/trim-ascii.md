---
title: 'clore::generate::trimascii'
description: 'clore::generate::trim_ascii 接受一个 std::string_view 并返回该视图的修剪副本，去除前导和尾随的 ASCII 空白字符。调用者可以依赖返回值是一个指向原始字符串有效子范围的视图，该子范围不包含任何开头或结尾的空白字符。返回的视图在其底层字符数组的生命周期内保持有效。'
layout: doc
template: doc
---

# `clore::generate::trim_ascii`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/render/common.cppm:34`

Definition: `src/generate/render/common.cppm:34`

Implementation: [`Module generate:common`](../../../../modules/generate/common.md)

## Declaration

```cpp
auto (std::string_view) -> std::string_view;
```

`clore::generate::trim_ascii` 接受一个 `std::string_view` 并返回该视图的修剪副本，去除前导和尾随的 ASCII 空白字符。调用者可以依赖返回值是一个指向原始字符串有效子范围的视图，该子范围不包含任何开头或结尾的空白字符。返回的视图在其底层字符数组的生命周期内保持有效。

## Usage Patterns

- Used by `build_prompt_section`, `strip_inline_markdown`, `add_prompt_output` to trim whitespace from string views

## Called By

- function `clore::generate::add_prompt_output`
- function `clore::generate::build_prompt_section`
- function `clore::generate::strip_inline_markdown`

