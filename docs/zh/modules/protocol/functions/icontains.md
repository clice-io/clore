---
title: 'clore::net::icontains'
description: '函数 clore::net::icontains 实现了一个不区分大小写的子串搜索算法。它首先快速退出：若待查找的 needle 长度大于被搜索的 haystack 长度，则直接返回 false。否则，外层循环遍历 haystack 中所有可能的起始位置（从 0 到 haystack.size() - needle.size()），内层循环逐字符比较两个字符串对应字符的 std::tolower 结果（字符类型通过 static_cast<unsigned char> 提升，以避免负值导致的未定义行为）。一旦内层循环完全匹配，函数立即返回 true；若所有起始位置均不匹配，则最终返回 false。该函数仅依赖标准库的 <cctype> 和 <string_view>，不引入外部依赖。'
layout: doc
template: doc
---

# `clore::net::icontains`

Owner: [Module protocol](../index.md)

Declaration: `src/network/protocol.cppm:780`

Definition: `src/network/protocol.cppm:780`

Declaration: [`Namespace clore::net`](../../../namespaces/clore/net/index.md)

## Implementation

```cpp
auto icontains(std::string_view haystack, std::string_view needle) -> bool {
    if(needle.size() > haystack.size()) {
        return false;
    }
    for(std::size_t i = 0; i <= haystack.size() - needle.size(); ++i) {
        bool match = true;
        for(std::size_t j = 0; j < needle.size(); ++j) {
            if(std::tolower(static_cast<unsigned char>(haystack[i + j])) !=
               std::tolower(static_cast<unsigned char>(needle[j]))) {
                match = false;
                break;
            }
        }
        if(match) {
            return true;
        }
    }
    return false;
}
```

函数 `clore::net::icontains` 实现了一个不区分大小写的子串搜索算法。它首先快速退出：若待查找的 `needle` 长度大于被搜索的 `haystack` 长度，则直接返回 `false`。否则，外层循环遍历 `haystack` 中所有可能的起始位置（从 `0` 到 `haystack.size() - needle.size()`），内层循环逐字符比较两个字符串对应字符的 `std::tolower` 结果（字符类型通过 `static_cast<unsigned char>` 提升，以避免负值导致的未定义行为）。一旦内层循环完全匹配，函数立即返回 `true`；若所有起始位置均不匹配，则最终返回 `false`。该函数仅依赖标准库的 `<cctype>` 和 `<string_view>`，不引入外部依赖。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `haystack`
- `needle`

## Usage Patterns

- Used by `clore::net::is_feature_rejection_error` to perform case-insensitive substring matching on error strings.

## Called By

- function `clore::net::is_feature_rejection_error`

