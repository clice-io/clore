---
title: 'clore::net::anthropic::protocol::detail::appendtextwithgap'
description: '函数 clore::net::anthropic::protocol::detail::append_text_with_gap 接受一个 std::string & 和一个 std::string_view。它将后者包含的文本内容追加到前者，并在追加的文本前后（或之间）按照协议要求插入一个间隙。调用方必须提供一个有效的、可修改的字符串引用作为目标，并确保 std::string_view 所引用的字符序列在调用期间保持有效。该函数不返回值，修改直接作用于目标字符串，用于构建请求 JSON 中需要保留特定间隔格式的文本块序列。'
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

函数 `clore::net::anthropic::protocol::detail::append_text_with_gap` 接受一个 `std::string &` 和一个 `std::string_view`。它将后者包含的文本内容追加到前者，并在追加的文本前后（或之间）按照协议要求插入一个间隙。调用方必须提供一个有效的、可修改的字符串引用作为目标，并确保 `std::string_view` 所引用的字符序列在调用期间保持有效。该函数不返回值，修改直接作用于目标字符串，用于构建请求 JSON 中需要保留特定间隔格式的文本块序列。

## Usage Patterns

- called in `build_request_json` to accumulate text blocks with gaps

## Called By

- function `clore::net::anthropic::protocol::build_request_json`

