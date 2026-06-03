---
title: 'clore::generate::normalizefrontmattertitle'
description: '该函数先调用 strip_inline_markdown 从 page_title 中去除内联 Markdown 标记（例如 **bold**、 code ），得到纯文本结果 plain。若 plain 非空，则直接将其返回，这是为了优先使用去除了格式后仍有实际内容的标题。若 plain 为空（即原字符串完全由 Markdown 标记构成或者本身就是空白），则回退到 trim_ascii 处理原始 page_title，去除首尾 ASCII 空白后返回。整个控制流以一次条件判断和两种清理策略为基础，依赖的两个辅助函数 strip_inline_markdown 和 trim_ascii 均在相同匿名命名空间内定义，前者负责 Markdown 语法解析，后者仅处理空白修剪。'
layout: doc
template: doc
---

# `clore::generate::normalize_frontmatter_title`

Owner: [Module generate:symbol](../symbol.md)

Declaration: `src/generate/render/symbol.cppm:51`

Definition: `src/generate/render/symbol.cppm:903`

Declaration: [`Namespace clore::generate`](../../../namespaces/clore/generate/index.md)

## Implementation

```cpp
auto normalize_frontmatter_title(std::string_view page_title) -> std::string {
    auto plain = strip_inline_markdown(page_title);
    if(!plain.empty()) {
        return plain;
    }
    return std::string(trim_ascii(page_title));
}
```

该函数先调用 `strip_inline_markdown` 从 `page_title` 中去除内联 Markdown 标记（例如 `**bold**`、`` `code` ``），得到纯文本结果 `plain`。若 `plain` 非空，则直接将其返回，这是为了优先使用去除了格式后仍有实际内容的标题。若 `plain` 为空（即原字符串完全由 Markdown 标记构成或者本身就是空白），则回退到 `trim_ascii` 处理原始 `page_title`，去除首尾 ASCII 空白后返回。整个控制流以一次条件判断和两种清理策略为基础，依赖的两个辅助函数 `strip_inline_markdown` 和 `trim_ascii` 均在相同匿名命名空间内定义，前者负责 Markdown 语法解析，后者仅处理空白修剪。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- parameter `page_title`

## Writes To

- returned `std::string` value

## Usage Patterns

- used by `build_symbol_frontmatter` to produce a plain title for frontmatter

## Called By

- function `clore::generate::(anonymous namespace)::build_symbol_frontmatter`

