---
title: 'clore::generate::makecode'
description: '函数 clore::generate::make_code 接受一个 std::string 类型的代码文本，并返回一个 InlineFragment。它负责将原始代码字符串封装为一个专门表示内联代码片段的文档节点，供页面的行内内容构建流程使用。调用者应当提供一个合法的代码字符串，该函数返回的 InlineFragment 可以直接注入到文档片段序列中，无需额外处理。'
layout: doc
template: doc
---

# `clore::generate::make_code`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/markdown.cppm:112`

Definition: `src/generate/markdown.cppm:147`

Implementation: [`Module generate:markdown`](../../../../modules/generate/markdown.md)

## Declaration

```cpp
auto (std::string) -> InlineFragment;
```

函数 `clore::generate::make_code` 接受一个 `std::string` 类型的代码文本，并返回一个 `InlineFragment`。它负责将原始代码字符串封装为一个专门表示内联代码片段的文档节点，供页面的行内内容构建流程使用。调用者应当提供一个合法的代码字符串，该函数返回的 `InlineFragment` 可以直接注入到文档片段序列中，无需额外处理。

## Usage Patterns

- used to wrap raw code strings as `InlineFragment` objects
- called by `append_existing_code_span` and `append_code_aware_text` to produce code spans

## Called By

- function `clore::generate::(anonymous namespace)::append_code_aware_text`
- function `clore::generate::(anonymous namespace)::append_existing_code_span`

