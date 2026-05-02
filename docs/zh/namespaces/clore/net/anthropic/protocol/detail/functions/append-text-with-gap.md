---
title: 'clore::net::anthropic::protocol::detail::appendtextwithgap'
description: '此函数用于将一段文本以协议所需的格式追加到目标字符串中，同时在其前后或内部插入必要的间隔符。调用者应当提供一个可修改的 std::string 对象作为第一个参数，该对象通常代表正在构建的协议消息；第二个参数是一个 std::string_view，表示要追加的文本内容。函数不返回任何值，而是直接修改目标字符串，并且要求调用者确保目标字符串在调用前后保持有效状态。'
layout: doc
template: doc
---

# `clore::net::anthropic::protocol::detail::append_text_with_gap`

Owner: [Namespace clore::net::anthropic::protocol::detail](../index.md)

Declaration: `network/anthropic.cppm:25`

Definition: `network/anthropic.cppm:25`

Implementation: [`Module anthropic`](../../../../../../../modules/anthropic/index.md)

## Declaration

```cpp
auto (std::string &, std::string_view) -> void;
```

此函数用于将一段文本以协议所需的格式追加到目标字符串中，同时在其前后或内部插入必要的间隔符。调用者应当提供一个可修改的 `std::string` 对象作为第一个参数，该对象通常代表正在构建的协议消息；第二个参数是一个 `std::string_view`，表示要追加的文本内容。函数不返回任何值，而是直接修改目标字符串，并且要求调用者确保目标字符串在调用前后保持有效状态。

## Usage Patterns

- called by `build_request_json` to assemble request body
- used for appending text blocks with a separating gap

## Called By

- function `clore::net::anthropic::protocol::build_request_json`

