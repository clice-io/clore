---
title: 'clore::support::striputf8bom'
description: '函数 clore::support::strip_utf8_bom 在接收到一个 std::string_view 后，首先判断其长度是否不小于 UTF-8 BOM 字节序列（即常量 kUtf8Bom，对应 0xEF, 0xBB, 0xBF）的长度。若满足条件，则依次将输入的前三个字节通过 static_cast<unsigned char> 转换为无符号字符，并与 kUtf8Bom 中的对应字节进行相等性比较。仅在三个字节全部匹配时，才调用 text.substr(...) 跳过前三个字节并返回剩余部分的视图；否则直接返回原始 text 视图。该函数不涉及外部依赖，仅依赖同一匿名命名空间内定义的 kUtf8Bom 常量。'
layout: doc
template: doc
---

# `clore::support::strip_utf8_bom`

Owner: [Module support](../index.md)

Declaration: `support/logging.cppm:83`

Definition: `support/logging.cppm:470`

Declaration: [`Namespace clore::support`](../../../namespaces/clore/support/index.md)

## Implementation

```cpp
auto strip_utf8_bom(std::string_view text) -> std::string_view {
    if(text.size() >= std::size(kUtf8Bom) && static_cast<unsigned char>(text[0]) == kUtf8Bom[0] &&
       static_cast<unsigned char>(text[1]) == kUtf8Bom[1] &&
       static_cast<unsigned char>(text[2]) == kUtf8Bom[2]) {
        return text.substr(std::size(kUtf8Bom));
    }

    return text;
}
```

函数 `clore::support::strip_utf8_bom` 在接收到一个 `std::string_view` 后，首先判断其长度是否不小于 UTF-8 BOM 字节序列（即常量 `kUtf8Bom`，对应 `0xEF, 0xBB, 0xBF`）的长度。若满足条件，则依次将输入的前三个字节通过 `static_cast<unsigned char>` 转换为无符号字符，并与 `kUtf8Bom` 中的对应字节进行相等性比较。仅在三个字节全部匹配时，才调用 `text.substr(...)` 跳过前三个字节并返回剩余部分的视图；否则直接返回原始 `text` 视图。该函数不涉及外部依赖，仅依赖同一匿名命名空间内定义的 `kUtf8Bom` 常量。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- the parameter `text` of type `std::string_view`
- the internal constant `kUtf8Bom` (presumably a byte array)

## Usage Patterns

- used by `clore::support::read_utf8_text_file` to remove a BOM before further text processing

## Called By

- function `clore::support::read_utf8_text_file`

