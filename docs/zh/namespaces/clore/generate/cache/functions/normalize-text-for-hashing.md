---
title: 'clore::generate::cache::normalizetextforhashing'
description: '函数 clore::generate::cache::normalize_text_for_hashing 将输入的文本转换为适合哈希运算的规范化形式。它接受一个 std::string_view 并返回一个 std::string；调用者应传入需要保证哈希一致性的文本，例如在构造缓存键之前对提示或响应文本进行预处理。规范化后的输出是确定性的，对于逻辑上等价的不同输入（例如忽略大小写或空白差异），会生成相同的字符串，从而确保缓存查找的正确性。该函数不涉及外部状态，也不抛出异常，适用于性能敏感的路径。'
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

函数 `clore::generate::cache::normalize_text_for_hashing` 将输入的文本转换为适合哈希运算的规范化形式。它接受一个 `std::string_view` 并返回一个 `std::string`；调用者应传入需要保证哈希一致性的文本，例如在构造缓存键之前对提示或响应文本进行预处理。规范化后的输出是确定性的，对于逻辑上等价的不同输入（例如忽略大小写或空白差异），会生成相同的字符串，从而确保缓存查找的正确性。该函数不涉及外部状态，也不抛出异常，适用于性能敏感的路径。

## Usage Patterns

- Called by `make_prompt_response_cache_key` to normalize input strings before combining into a cache key.

## Called By

- function `clore::generate::cache::make_prompt_response_cache_key`

