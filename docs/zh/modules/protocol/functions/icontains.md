---
title: 'clore::net::icontains'
description: '函数 clore::net::icontains 是一种不区分大小写的子串搜索，通过逐字符比较 haystack 和 needle 实现。实现首先检查 needle.size() > haystack.size()，若成立则直接返回 false，以避免不必要的循环。随后外层循环遍历 haystack 中所有可能的起始位置（i 从 0 到 haystack.size() - needle.size()），内层循环对每个字符对进行不区分大小写的匹配：使用 std::tolower 将两个字符均转为小写后比较。一旦内层循环完全匹配，立即返回 true；否则继续扫描；若所有起始位置都检查完毕未匹配，返回 false。该算法不依赖项目内部工具，仅依赖标准库函数 std::tolower 和 static_cast<unsigned char> 保证字符类型安全。'
layout: doc
template: doc
---

# `clore::net::icontains`

Owner: [Module protocol](../index.md)

Declaration: `network/protocol.cppm:758`

Definition: `network/protocol.cppm:758`

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

函数 `clore::net::icontains` 是一种不区分大小写的子串搜索，通过逐字符比较 `haystack` 和 `needle` 实现。实现首先检查 `needle.size() > haystack.size()`，若成立则直接返回 `false`，以避免不必要的循环。随后外层循环遍历 `haystack` 中所有可能的起始位置（`i` 从 `0` 到 `haystack.size() - needle.size()`），内层循环对每个字符对进行不区分大小写的匹配：使用 `std::tolower` 将两个字符均转为小写后比较。一旦内层循环完全匹配，立即返回 `true`；否则继续扫描；若所有起始位置都检查完毕未匹配，返回 `false`。该算法不依赖项目内部工具，仅依赖标准库函数 `std::tolower` 和 `static_cast<unsigned char>` 保证字符类型安全。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `haystack`
- `needle`

## Usage Patterns

- Used by `is_feature_rejection_error` to check if an error message contains a substring case-insensitively

## Called By

- function `clore::net::is_feature_rejection_error`

