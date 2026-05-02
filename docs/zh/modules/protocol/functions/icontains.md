---
title: 'clore::net::icontains'
description: 'clore::net::icontains 采用朴素的 O(n·m) 子串搜索算法来执行不区分大小写的包含检查。函数首先进行快速长度剪枝：若 needle 的长度大于 haystack 则直接返回 false。随后在外层循环中枚举 haystack 内所有可能作为起点的位置，内层循环逐字符进行比较，每次比较前将两字符通过 std::tolower 统一转换为小写，一旦发现完整匹配即返回 true。若所有位置均未匹配，则返回 false。该实现完全依赖标准库的 std::tolower 进行大小写折叠，无其他外部依赖或动态内存分配，适用于短字符串或性能要求不高的场景。'
layout: doc
template: doc
---

# `clore::net::icontains`

Owner: [Module protocol](../index.md)

Declaration: `network/protocol.cppm:768`

Definition: `network/protocol.cppm:768`

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

`clore::net::icontains` 采用朴素的 **O(n·m)** 子串搜索算法来执行不区分大小写的包含检查。函数首先进行快速长度剪枝：若 `needle` 的长度大于 `haystack` 则直接返回 `false`。随后在外层循环中枚举 `haystack` 内所有可能作为起点的位置，内层循环逐字符进行比较，每次比较前将两字符通过 `std::tolower` 统一转换为小写，一旦发现完整匹配即返回 `true`。若所有位置均未匹配，则返回 `false`。该实现完全依赖标准库的 `std::tolower` 进行大小写折叠，无其他外部依赖或动态内存分配，适用于短字符串或性能要求不高的场景。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `haystack`
- `needle`

## Usage Patterns

- Used by `clore::net::is_feature_rejection_error` to perform case-insensitive matching on error messages.

## Called By

- function `clore::net::is_feature_rejection_error`

