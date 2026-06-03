---
title: 'clore::generate::promptkindname'
description: 'clore::generate::prompt_kind_name 接受一个 PromptKind 枚举值，并返回一个指向内部静态存储的字符串视图。该函数将枚举常量映射为对应的人可读名称，主要用于需要将 prompt 种类转换为文本标识的场景，例如在 clore::generate::prompt_request_key 中组合生成唯一请求键。调用者应确保传入有效的枚举值，返回的 std::string_view 在整个程序生命周期内保持有效。'
layout: doc
template: doc
---

# `clore::generate::prompt_kind_name`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/model.cppm:48`

Definition: `src/generate/model.cppm:289`

Implementation: [`Module generate:model`](../../../../modules/generate/model.md)

## Declaration

```cpp
auto (PromptKind) -> std::string_view;
```

`clore::generate::prompt_kind_name` 接受一个 `PromptKind` 枚举值，并返回一个指向内部静态存储的字符串视图。该函数将枚举常量映射为对应的人可读名称，主要用于需要将 prompt 种类转换为文本标识的场景，例如在 `clore::generate::prompt_request_key` 中组合生成唯一请求键。调用者应确保传入有效的枚举值，返回的 `std::string_view` 在整个程序生命周期内保持有效。

## Usage Patterns

- called by `clore::generate::prompt_request_key` to produce a string-based key for a prompt request

## Called By

- function `clore::generate::prompt_request_key`

