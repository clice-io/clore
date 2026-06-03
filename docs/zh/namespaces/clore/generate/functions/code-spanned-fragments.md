---
title: 'clore::generate::codespannedfragments'
description: 'clore::generate::code_spanned_fragments 接受一个 std::string_view，并将内容解析为一系列 InlineFragment 对象，每个对象表示代码跨度中的一个逻辑元素（例如标记、分隔符或纯文本）。调用者负责提供格式正确的代码字符串；函数会划分片段以支持后续的语义渲染或链接处理。'
layout: doc
template: doc
---

# `clore::generate::code_spanned_fragments`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/markdown.cppm:135`

Definition: `src/generate/markdown.cppm:704`

Implementation: [`Module generate:markdown`](../../../../modules/generate/markdown.md)

## Declaration

```cpp
auto (std::string_view) -> std::vector<InlineFragment>;
```

`clore::generate::code_spanned_fragments` 接受一个 `std::string_view`，并将内容解析为一系列 `InlineFragment` 对象，每个对象表示代码跨度中的一个逻辑元素（例如标记、分隔符或纯文本）。调用者负责提供格式正确的代码字符串；函数会划分片段以支持后续的语义渲染或链接处理。

该函数是渲染管道的一部分，为外部客户端或内部渲染过程（如 `append_rendered_text`）提供结构化的代码表示，但不修改输入字符串本身。

## Usage Patterns

- Called by `append_rendered_text` to convert text into inline fragments during rendering
- Used to produce a sequence of `InlineFragment` objects from plain markdown text for further processing

## Calls

- function `clore::generate::(anonymous namespace)::append_code_spanned_fragments`

## Called By

- function `clore::generate::(anonymous namespace)::append_rendered_text`

