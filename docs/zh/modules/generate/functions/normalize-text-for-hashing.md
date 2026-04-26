---
title: 'clore::generate::cache::normalizetextforhashing'
description: '该函数通过两个阶段实现文本规范化。第一阶段从前向后扫描输入并跳过所有前导空白字符（使用 std::isspace 检查每个字符），因此只保留第一个非空白字符之后的文本。第二阶段从第一个非空白字符开始遍历剩余字符：遇到空白字符时，仅设置一个布尔标记 prev_space，不直接向结果追加空白；遇到非空白字符时，若 prev_space 为真且结果非空，则在追加该字符前先插入一个空格，然后重置标记。这一逻辑将任意长度的连续空白序列压缩为一个空格，同时确保结果字符串不会以空格开头（因为前导空白已跳过）且末尾不会有多余空格（只有当非空白字符之后有空白时才可能追加空格，而末尾空白序列永远不会触发追加）。整个过程完全基于标准库函数 std::isspace 和 std::string 的 reserve、push_back，无需其他外部依赖。'
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

该函数通过两个阶段实现文本规范化。第一阶段从前向后扫描输入并跳过所有前导空白字符（使用 `std::isspace` 检查每个字符），因此只保留第一个非空白字符之后的文本。第二阶段从第一个非空白字符开始遍历剩余字符：遇到空白字符时，仅设置一个布尔标记 `prev_space`，不直接向结果追加空白；遇到非空白字符时，若 `prev_space` 为真且结果非空，则在追加该字符前先插入一个空格，然后重置标记。这一逻辑将任意长度的连续空白序列压缩为一个空格，同时确保结果字符串不会以空格开头（因为前导空白已跳过）且末尾不会有多余空格（只有当非空白字符之后有空白时才可能追加空格，而末尾空白序列永远不会触发追加）。整个过程完全基于标准库函数 `std::isspace` 和 `std::string` 的 `reserve`、`push_back`，无需其他外部依赖。

## Side Effects

- allocates a new string

## Reads From

- `text` parameter

## Writes To

- local variable `result`
- return value (new `std::string`)

## Usage Patterns

- called by `clore::generate::cache::make_prompt_response_cache_key` to normalize prompt or response text before hashing
- used to ensure consistent cache keys regardless of whitespace variation

## Called By

- function `clore::generate::cache::make_prompt_response_cache_key`

