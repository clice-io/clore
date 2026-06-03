---
title: 'clore::support::striputf8bom'
description: '函数 clore::support::strip_utf8_bom 的算法实现简洁直接：首先检查输入字符串视图 text 的长度是否至少等于 UTF‑8 BOM 序列的长度（由常量 kUtf8Bom 定义），然后逐一比较前三个字节与 BOM 的字节值。若完全匹配，则通过 text.substr(std::size(kUtf8Bom)) 返回去掉前导 BOM 后的子视图；否则直接返回原始 text。内部控制流仅为一个条件分支，无循环或递归。该函数依赖于匿名命名空间中的常量 kUtf8Bom 来获取 BOM 字节序列，并依赖标准库的 std::string_view 及其 substr 成员方法执行视图裁剪。'
layout: doc
template: doc
---

# `clore::support::strip_utf8_bom`

Owner: [Module support](../index.md)

Declaration: `src/support/logging.cppm:106`

Definition: `src/support/logging.cppm:493`

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

函数 `clore::support::strip_utf8_bom` 的算法实现简洁直接：首先检查输入字符串视图 `text` 的长度是否至少等于 UTF‑8 BOM 序列的长度（由常量 `kUtf8Bom` 定义），然后逐一比较前三个字节与 BOM 的字节值。若完全匹配，则通过 `text.substr(std::size(kUtf8Bom))` 返回去掉前导 BOM 后的子视图；否则直接返回原始 `text`。内部控制流仅为一个条件分支，无循环或递归。该函数依赖于匿名命名空间中的常量 `kUtf8Bom` 来获取 BOM 字节序列，并依赖标准库的 `std::string_view` 及其 `substr` 成员方法执行视图裁剪。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `text` parameter
- `kUtf8Bom` constant

## Usage Patterns

- Stripping BOM from text loaded by `clore::support::read_utf8_text_file`

## Called By

- function `clore::support::read_utf8_text_file`

