---
title: 'clore::generate::cache::normalizetextforhashing'
description: 'clore::generate::cache::normalize_text_for_hashing 函数接受一个 std::string_view 并返回一个 std::string。它负责在文本用作散列输入之前对其进行规范化，从而确保语义上相同的文本在缓存键生成过程中产生一致的规范化表示。此函数由 clore::generate::cache::make_prompt_response_cache_key 调用，用于规范化系统提示和用户请求提示，作为构建复合缓存键的一部分。调用者可以依赖该函数生成的规范化结果来保证缓存查找的确定性。'
layout: doc
template: doc
---

# `clore::generate::cache::normalize_text_for_hashing`

Owner: [Namespace clore::generate::cache](../index.md)

Declaration: `src/generate/cache.cppm:211`

Definition: `src/generate/cache.cppm:211`

Implementation: [`Module generate:cache`](../../../../../modules/generate/cache.md)

## Declaration

```cpp
auto (std::string_view) -> std::string;
```

`clore::generate::cache::normalize_text_for_hashing` 函数接受一个 `std::string_view` 并返回一个 `std::string`。它负责在文本用作散列输入之前对其进行规范化，从而确保语义上相同的文本在缓存键生成过程中产生一致的规范化表示。此函数由 `clore::generate::cache::make_prompt_response_cache_key` 调用，用于规范化系统提示和用户请求提示，作为构建复合缓存键的一部分。调用者可以依赖该函数生成的规范化结果来保证缓存查找的确定性。

## Usage Patterns

- Called by `make_prompt_response_cache_key` to normalize text before hashing.

## Called By

- function `clore::generate::cache::make_prompt_response_cache_key`

