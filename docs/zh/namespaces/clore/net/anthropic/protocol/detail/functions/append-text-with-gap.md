---
title: 'clore::net::anthropic::protocol::detail::appendtextwithgap'
description: '向目标字符串追加给定的文本视图，并在嵌入时自动维持适当的格式间隙（例如换行与缩进），使拼接结果符合 Anthropic 协议对 JSON 结构的布局要求。调用方须提供可变的 std::string 引用作为接收缓冲区，以及一个 std::string_view 作为待附加的文本片段；函数独立处理间隙逻辑，调用方无需关心 JSON 格式化细节。'
layout: doc
template: doc
---

# `clore::net::anthropic::protocol::detail::append_text_with_gap`

Owner: [Namespace clore::net::anthropic::protocol::detail](../index.md)

Declaration: `src/network/anthropic.cppm:34`

Definition: `src/network/anthropic.cppm:34`

Implementation: [`Module anthropic`](../../../../../../../modules/anthropic/index.md)

## Declaration

```cpp
auto (std::string &, std::string_view) -> void;
```

向目标字符串追加给定的文本视图，并在嵌入时自动维持适当的格式间隙（例如换行与缩进），使拼接结果符合 Anthropic 协议对 JSON 结构的布局要求。调用方须提供可变的 `std::string` 引用作为接收缓冲区，以及一个 `std::string_view` 作为待附加的文本片段；函数独立处理间隙逻辑，调用方无需关心 JSON 格式化细节。

## Usage Patterns

- Called by `clore::net::anthropic::protocol::build_request_json` to add text blocks.

## Called By

- function `clore::net::anthropic::protocol::build_request_json`

