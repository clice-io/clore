---
title: 'clore::generate::codespannedfragments'
description: '函数 clore::generate::code_spanned_fragments 是一个轻量级的包装入口，其实现仅创建空的 std::vector<InlineFragment> 并调用内部辅助函数 append_code_spanned_fragments 填充该容器，最后直接返回该向量。实际的代码跨度解析与片段生成逻辑完全委托给 append_code_spanned_fragments，该函数逐字符扫描输入文本，识别 backtick 引用的内联代码、代码样式候选（如标识符、运算符及复合符号），并依据边界规则生成对应的 CodeFragment 或 TextFragment，最终构成完整的 InlineFragment 序列。'
layout: doc
template: doc
---

# `clore::generate::code_spanned_fragments`

Owner: [Module generate:markdown](../markdown.md)

Declaration: `src/generate/markdown.cppm:135`

Definition: `src/generate/markdown.cppm:704`

Declaration: [`Namespace clore::generate`](../../../namespaces/clore/generate/index.md)

## Implementation

```cpp
auto code_spanned_fragments(std::string_view text) -> std::vector<InlineFragment> {
    std::vector<InlineFragment> fragments;
    append_code_spanned_fragments(fragments, text);
    return fragments;
}
```

函数 `clore::generate::code_spanned_fragments` 是一个轻量级的包装入口，其实现仅创建空的 `std::vector<InlineFragment>` 并调用内部辅助函数 `append_code_spanned_fragments` 填充该容器，最后直接返回该向量。实际的代码跨度解析与片段生成逻辑完全委托给 `append_code_spanned_fragments`，该函数逐字符扫描输入文本，识别 backtick 引用的内联代码、代码样式候选（如标识符、运算符及复合符号），并依据边界规则生成对应的 `CodeFragment` 或 `TextFragment`，最终构成完整的 `InlineFragment` 序列。

这种分离设计使 `code_spanned_fragments` 对外暴露为简洁的高级接口，而内部复杂的词法分析、跨度和候选判断细节（包括对 `read_backtick_span`、`read_code_candidate`、`read_operator_token` 等子函数的调用）都封装在 `append_code_spanned_fragments` 中，便于后续维护与单元测试。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- parameter `text` of type `std::string_view`

## Usage Patterns

- Called by `append_rendered_text` to convert text into inline fragments during rendering
- Used to produce a sequence of `InlineFragment` objects from plain markdown text for further processing

## Calls

- function `clore::generate::(anonymous namespace)::append_code_spanned_fragments`

## Called By

- function `clore::generate::(anonymous namespace)::append_rendered_text`

