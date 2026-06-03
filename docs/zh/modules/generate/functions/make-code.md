---
title: 'clore::generate::makecode'
description: '函数 clore::generate::make_code 的实现直接创建一个 CodeFragment 对象，通过聚合初始化将传入的 code 字符串移动赋值到其 code 字段，然后返回该片段作为 InlineFragment。该函数不包含任何解析或验证逻辑，其全部行为委托给 CodeFragment 的构造过程，使用 std::move 避免不必要的字符串拷贝，确保零额外开销。'
layout: doc
template: doc
---

# `clore::generate::make_code`

Owner: [Module generate:markdown](../markdown.md)

Declaration: `src/generate/markdown.cppm:112`

Definition: `src/generate/markdown.cppm:147`

Declaration: [`Namespace clore::generate`](../../../namespaces/clore/generate/index.md)

## Implementation

```cpp
auto make_code(std::string code) -> InlineFragment {
    return CodeFragment{.code = std::move(code)};
}
```

函数 `clore::generate::make_code` 的实现直接创建一个 `CodeFragment` 对象，通过聚合初始化将传入的 `code` 字符串移动赋值到其 `code` 字段，然后返回该片段作为 `InlineFragment`。该函数不包含任何解析或验证逻辑，其全部行为委托给 `CodeFragment` 的构造过程，使用 `std::move` 避免不必要的字符串拷贝，确保零额外开销。

该函数是内联代码片段生成的最小构建单元，依赖 `CodeFragment` 和 `InlineFragment` 的类型定义。它所创建的片段随后被 `render_inline` 等高层函数消费，用于将代码跨度渲染为最终输出（如反引号包裹或语法高亮形式）。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- parameter `code`

## Writes To

- the `code` field of the returned `CodeFragment`

## Usage Patterns

- used to wrap raw code strings as `InlineFragment` objects
- called by `append_existing_code_span` and `append_code_aware_text` to produce code spans

## Called By

- function `clore::generate::(anonymous namespace)::append_code_aware_text`
- function `clore::generate::(anonymous namespace)::append_existing_code_span`

