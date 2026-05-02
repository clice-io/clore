---
title: 'clore::net::anthropic::protocol::detail::appendtextwithgap'
description: '该函数实现向目标字符串追加文本，并在存在已有内容时插入一个双换行分隔符。算法首先检查输入 text 是否为空，若为空则直接返回；否则判断 target 是否非空，若非空则在 target 末尾追加 "\n\n" 作为间隙，最后将 text 内容追加到 target 中。整个过程仅依赖标准字符串操作，不涉及外部库或复杂依赖。'
layout: doc
template: doc
---

# `clore::net::anthropic::protocol::detail::append_text_with_gap`

Owner: [Module anthropic](../index.md)

Declaration: `network/anthropic.cppm:25`

Definition: `network/anthropic.cppm:25`

Declaration: [`Namespace clore::net::anthropic::protocol::detail`](../../../namespaces/clore/net/anthropic/protocol/detail/index.md)

## Implementation

```cpp
auto append_text_with_gap(std::string& target, std::string_view text) -> void {
    if(text.empty()) {
        return;
    }
    if(!target.empty()) {
        target += "\n\n";
    }
    target += text;
}
```

该函数实现向目标字符串追加文本，并在存在已有内容时插入一个双换行分隔符。算法首先检查输入 `text` 是否为空，若为空则直接返回；否则判断 `target` 是否非空，若非空则在 `target` 末尾追加 `"\n\n"` 作为间隙，最后将 `text` 内容追加到 `target` 中。整个过程仅依赖标准字符串操作，不涉及外部库或复杂依赖。

## Side Effects

- mutates the `target` string by appending `text` and possibly a separator

## Reads From

- text parameter
- target parameter (for emptiness check)

## Writes To

- target parameter

## Usage Patterns

- called by `build_request_json` to assemble request body
- used for appending text blocks with a separating gap

## Called By

- function `clore::net::anthropic::protocol::build_request_json`

