---
title: 'clore::generate::trimascii'
description: '函数通过两个 while 循环实现修剪操作。第一个循环从 text 的开头检查每个字符，若 std::isspace 对该字符（经 static_cast<unsigned char> 转换）返回非零值，则调用 text.remove_prefix(1) 移除该字符，直至遇到非空白字符或 text 为空。第二个循环类似地从末尾检查，利用 text.back() 和 text.remove_suffix(1) 移除尾部空白。两个循环均依赖 std::isspace 的 ASCII 行为，并通过强制类型转换避免负值传递给 std::isspace 所导致的未定义行为。最终返回修剪后的 std::string_view。'
layout: doc
template: doc
---

# `clore::generate::trim_ascii`

Owner: [Module generate:common](../common.md)

Declaration: `src/generate/render/common.cppm:34`

Definition: `src/generate/render/common.cppm:34`

Declaration: [`Namespace clore::generate`](../../../namespaces/clore/generate/index.md)

## Implementation

```cpp
auto trim_ascii(std::string_view text) -> std::string_view {
    while(!text.empty() && std::isspace(static_cast<unsigned char>(text.front())) != 0) {
        text.remove_prefix(1);
    }
    while(!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
        text.remove_suffix(1);
    }
    return text;
}
```

函数通过两个 `while` 循环实现修剪操作。第一个循环从 `text` 的开头检查每个字符，若 `std::isspace` 对该字符（经 `static_cast<unsigned char>` 转换）返回非零值，则调用 `text.remove_prefix(1)` 移除该字符，直至遇到非空白字符或 `text` 为空。第二个循环类似地从末尾检查，利用 `text.back()` 和 `text.remove_suffix(1)` 移除尾部空白。两个循环均依赖 `std::isspace` 的 ASCII 行为，并通过强制类型转换避免负值传递给 `std::isspace` 所导致的未定义行为。最终返回修剪后的 `std::string_view`。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- parameter `text` of type `std::string_view`

## Usage Patterns

- Used by `build_prompt_section`, `strip_inline_markdown`, `add_prompt_output` to trim whitespace from string views

## Called By

- function `clore::generate::add_prompt_output`
- function `clore::generate::build_prompt_section`
- function `clore::generate::strip_inline_markdown`

