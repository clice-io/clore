---
title: 'clore::support::striputf8bom'
description: '函数 clore::support::strip_utf8_bom 通过一次简单的顺序判断移除 UTF-8 字节顺序标记（BOM）。它首先检查输入 text 的长度是否足够容纳 BOM 的三个字节，然后逐字节与常量 kUtf8Bom 比较。若完全匹配，则返回从 BOM 之后开始的子视图；否则直接返回原视图。整个过程不涉及额外内存分配或复杂分支，依赖仅局限于对该匿名命名空间常量的引用。'
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

函数 `clore::support::strip_utf8_bom` 通过一次简单的顺序判断移除 UTF-8 字节顺序标记（BOM）。它首先检查输入 `text` 的长度是否足够容纳 BOM 的三个字节，然后逐字节与常量 `kUtf8Bom` 比较。若完全匹配，则返回从 BOM 之后开始的子视图；否则直接返回原视图。整个过程不涉及额外内存分配或复杂分支，依赖仅局限于对该匿名命名空间常量的引用。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- 参数 `text`

## Usage Patterns

- 用于从文件读取的 UTF-8 文本中剥离 BOM 前缀

## Called By

- function `clore::support::read_utf8_text_file`

