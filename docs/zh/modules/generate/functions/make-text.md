---
title: 'clore::generate::maketext'
description: '函数 clore::generate::make_text 接收一个 std::string 类型的参数 text，并将其直接转移到一个 TextFragment 对象的 text 成员中，最终返回该对象作为 InlineFragment。该函数是创建纯文本内联片段的便捷方法，没有分支或循环，仅依赖 std::move 进行资源转移，以及 TextFragment 和 InlineFragment 类型的构造。'
layout: doc
template: doc
---

# `clore::generate::make_text`

Owner: [Module generate:markdown](../markdown.md)

Declaration: `src/generate/markdown.cppm:110`

Definition: `src/generate/markdown.cppm:143`

Declaration: [`Namespace clore::generate`](../../../namespaces/clore/generate/index.md)

## Implementation

```cpp
auto make_text(std::string text) -> InlineFragment {
    return TextFragment{.text = std::move(text)};
}
```

函数 `clore::generate::make_text` 接收一个 `std::string` 类型的参数 `text`，并将其直接转移到一个 `TextFragment` 对象的 `text` 成员中，最终返回该对象作为 `InlineFragment`。该函数是创建纯文本内联片段的便捷方法，没有分支或循环，仅依赖 `std::move` 进行资源转移，以及 `TextFragment` 和 `InlineFragment` 类型的构造。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- the `text` parameter

## Writes To

- the `text` member of the returned `TextFragment`

## Usage Patterns

- building text fragments for markdown nodes
- creating inline fragments from plain strings

## Called By

- function `clore::generate::(anonymous namespace)::append_text_fragment`
- function `clore::generate::make_blockquote`
- function `clore::generate::make_paragraph`

