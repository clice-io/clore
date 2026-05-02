---
title: 'clore::generate::cache::normalizetextforhashing'
description: '函数 clore::generate::cache::normalize_text_for_hashing 实现了对输入文本的标准化，旨在为后续哈希计算提供一致、可比较的字符串。算法通过两步完成：首先跳过所有前导空白字符，然后遍历剩余字符，使用一个 prev_space 标志记录是否遇到了空白；每当遇到非空白字符时，如果之前有空白且结果字符串非空，则插入一个空格，再添加该字符，从而将任意连续的空白序列（包括换行、制表等）压缩为单个标准空格，同时自动消除尾部空白。控制流基于单次线性扫描与条件判断，依赖标准库的 std::isspace 检测空白字符，不涉及外部系统或复杂数据结构，保证了轻量且确定性的规范化效果。'
layout: doc
template: doc
---

# `clore::generate::cache::normalize_text_for_hashing`

Owner: [Module generate:cache](../cache.md)

Declaration: `generate/cache.cppm:192`

Definition: `generate/cache.cppm:192`

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

函数 `clore::generate::cache::normalize_text_for_hashing` 实现了对输入文本的标准化，旨在为后续哈希计算提供一致、可比较的字符串。算法通过两步完成：首先跳过所有前导空白字符，然后遍历剩余字符，使用一个 `prev_space` 标志记录是否遇到了空白；每当遇到非空白字符时，如果之前有空白且结果字符串非空，则插入一个空格，再添加该字符，从而将任意连续的空白序列（包括换行、制表等）压缩为单个标准空格，同时自动消除尾部空白。控制流基于单次线性扫描与条件判断，依赖标准库的 `std::isspace` 检测空白字符，不涉及外部系统或复杂数据结构，保证了轻量且确定性的规范化效果。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `text` parameter (`std::string_view`)

## Usage Patterns

- Called by `make_prompt_response_cache_key` to normalize input strings before combining into a cache key.

## Called By

- function `clore::generate::cache::make_prompt_response_cache_key`

