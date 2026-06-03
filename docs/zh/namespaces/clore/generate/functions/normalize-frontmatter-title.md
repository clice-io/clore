---
title: 'clore::generate::normalizefrontmattertitle'
description: 'clore::generate::normalize_frontmatter_title 接受一个原始标题字符串（std::string_view）并返回一个规范化后的 std::string，该返回值适合嵌入到页面的 frontmatter 中。调用者应确保输入的字符串是预期的标题文本；函数会执行必要的清理与标准化转换（例如去除多余空白、调整大小写或处理特殊字符），以确保生成的标题一致且可安全用于文档结构的元数据中。'
layout: doc
template: doc
---

# `clore::generate::normalize_frontmatter_title`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/render/symbol.cppm:51`

Definition: `src/generate/render/symbol.cppm:903`

Implementation: [`Module generate:symbol`](../../../../modules/generate/symbol.md)

## Declaration

```cpp
auto (std::string_view) -> std::string;
```

`clore::generate::normalize_frontmatter_title` 接受一个原始标题字符串（`std::string_view`）并返回一个规范化后的 `std::string`，该返回值适合嵌入到页面的 frontmatter 中。调用者应确保输入的字符串是预期的标题文本；函数会执行必要的清理与标准化转换（例如去除多余空白、调整大小写或处理特殊字符），以确保生成的标题一致且可安全用于文档结构的元数据中。

## Usage Patterns

- used by `build_symbol_frontmatter` to produce a plain title for frontmatter

## Called By

- function `clore::generate::(anonymous namespace)::build_symbol_frontmatter`

