---
title: 'clore::generate::maketext'
description: 'clore::generate::make_text 接受一个 std::string 并返回一个 InlineFragment。它创建一个表示纯文本内容的内联片段，不添加任何格式或标记。调用者提供要嵌入的文本字符串，返回的 InlineFragment 可以直接用于构造更复杂的文档节点，如 clore::generate::make_paragraph 或 clore::generate::make_blockquote 所演示的那样。该函数是构建内联内容的基础构建块，用于在文档树中表示无样式文本。'
layout: doc
template: doc
---

# `clore::generate::make_text`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/markdown.cppm:110`

Definition: `src/generate/markdown.cppm:143`

Implementation: [`Module generate:markdown`](../../../../modules/generate/markdown.md)

## Declaration

```cpp
auto (std::string) -> InlineFragment;
```

`clore::generate::make_text` 接受一个 `std::string` 并返回一个 `InlineFragment`。它创建一个表示纯文本内容的内联片段，不添加任何格式或标记。调用者提供要嵌入的文本字符串，返回的 `InlineFragment` 可以直接用于构造更复杂的文档节点，如 `clore::generate::make_paragraph` 或 `clore::generate::make_blockquote` 所演示的那样。该函数是构建内联内容的基础构建块，用于在文档树中表示无样式文本。

## Usage Patterns

- building text fragments for markdown nodes
- creating inline fragments from plain strings

## Called By

- function `clore::generate::(anonymous namespace)::append_text_fragment`
- function `clore::generate::make_blockquote`
- function `clore::generate::make_paragraph`

