---
title: 'clore::generate::cache::normalizetextforhashing'
description: '函数 clore::generate::cache::normalize_text_for_hashing 实现了一个简单的文本规范化流程，其核心目的是为后续哈希计算生成一个格式一致的键。算法分两步：首先跳过输入字符串 text 开头的所有空白字符（使用 std::isspace 并以 unsigned char 避免符号扩展问题）；然后从第一个非空白字符起遍历剩余部分，维护一个 prev_space 布尔标志。当遇到空白字符时，仅将 prev_space 置为 true；遇到非空白字符时，若 prev_space 为 true 且结果字符串 result 非空，则在添加当前字符前先插入一个空格，并重置 prev_space。整个过程将任意连续的空白序列压缩为单个空格，同时去除首尾空白，最终返回规范化后的字符串。该函数的全部控制流仅依赖标准库的 std::isspace 和 std::string 的内存管理，无外部依赖，时间复杂度为 O(n)。'
layout: doc
template: doc
---

# `clore::generate::cache::normalize_text_for_hashing`

Owner: [Module generate:cache](../cache.md)

Declaration: `src/generate/cache.cppm:211`

Definition: `src/generate/cache.cppm:211`

Declaration: [`Namespace clore::generate::cache`](../../../namespaces/clore/generate/cache/index.md)

## Implementation

```cpp
auto normalize_text_for_hashing(std::string_view text) -> std::string {
    std::string result;
    result.reserve(text.size());

    // Trim leading whitespace
    std::size_t start = 0;
    while(start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) {
        ++start;
    }

    bool prev_space = false;
    for(std::size_t i = start; i < text.size(); ++i) {
        auto ch = static_cast<unsigned char>(text[i]);
        if(std::isspace(ch)) {
            prev_space = true;
        } else {
            if(prev_space && !result.empty()) {
                result.push_back(' ');
            }
            result.push_back(static_cast<char>(ch));
            prev_space = false;
        }
    }

    return result;
}
```

函数 `clore::generate::cache::normalize_text_for_hashing` 实现了一个简单的文本规范化流程，其核心目的是为后续哈希计算生成一个格式一致的键。算法分两步：首先跳过输入字符串 `text` 开头的所有空白字符（使用 `std::isspace` 并以 `unsigned char` 避免符号扩展问题）；然后从第一个非空白字符起遍历剩余部分，维护一个 `prev_space` 布尔标志。当遇到空白字符时，仅将 `prev_space` 置为 `true`；遇到非空白字符时，若 `prev_space` 为 `true` 且结果字符串 `result` 非空，则在添加当前字符前先插入一个空格，并重置 `prev_space`。整个过程将任意连续的空白序列压缩为单个空格，同时去除首尾空白，最终返回规范化后的字符串。该函数的全部控制流仅依赖标准库的 `std::isspace` 和 `std::string` 的内存管理，无外部依赖，时间复杂度为 O(n)。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- parameter `text` (`std::string_view`)

## Writes To

- local `result` string (returned by value)

## Usage Patterns

- Called by `make_prompt_response_cache_key` to normalize text before hashing.

## Called By

- function `clore::generate::cache::make_prompt_response_cache_key`

