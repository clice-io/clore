---
title: 'clore::generate::cache::normalizetextforhashing'
description: '该函数接受一个 std::string_view 并返回一个 std::string，用于将输入文本规范化为可预测、可重复的格式，以便后续用于哈希运算。调用者可以期望：对于内容相同但细微差异（如空白字符、大小写或标点）的两个输入，函数能生成相同的规范化输出，从而确保哈希键的一致性。此函数是 make_prompt_response_cache_key 内部流程的一部分，用于生成缓存键的组件。'
layout: doc
template: doc
---

# `clore::generate::cache::normalize_text_for_hashing`

Owner: [Namespace clore::generate::cache](../index.md)

Declaration: `generate/cache.cppm:192`

Definition: `generate/cache.cppm:192`

Implementation: [`Module generate:cache`](../../../../../modules/generate/cache.md)

## Declaration

```cpp
auto (std::string_view) -> std::string;
```

该函数接受一个 `std::string_view` 并返回一个 `std::string`，用于将输入文本规范化为可预测、可重复的格式，以便后续用于哈希运算。调用者可以期望：对于内容相同但细微差异（如空白字符、大小写或标点）的两个输入，函数能生成相同的规范化输出，从而确保哈希键的一致性。此函数是 `make_prompt_response_cache_key` 内部流程的一部分，用于生成缓存键的组件。

## Usage Patterns

- called by `clore::generate::cache::make_prompt_response_cache_key` to normalize prompt or response text before hashing
- used to ensure consistent cache keys regardless of whitespace variation

## Called By

- function `clore::generate::cache::make_prompt_response_cache_key`

