---
title: 'clore::net::anthropic::protocol::detail::appendtextwithgap'
description: '函数 clore::net::anthropic::protocol::detail::append_text_with_gap 负责在逐步构建协议消息内容时，将一段新文本追加到目标字符串 target 中，并在需要时插入一个空行作为分隔。算法首先检查输入 text 是否为空，若为空则立即返回，避免无效操作。若 text 非空，则检查 target 当前是否非空——只有非空时才会在尾部追加两个换行符 "\n\n"，从而在已有内容与新增内容之间形成一个显式的间隙。最后将 text 直接追加到 target 末尾。整个函数仅依赖标准库的字符串拼接操作，无其它外部依赖，其内部控制流为简单的条件判断与字符串连接。'
layout: doc
template: doc
---

# `clore::net::anthropic::protocol::detail::append_text_with_gap`

Owner: [Module anthropic](../index.md)

Declaration: `src/network/anthropic.cppm:34`

Definition: `src/network/anthropic.cppm:34`

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

函数 `clore::net::anthropic::protocol::detail::append_text_with_gap` 负责在逐步构建协议消息内容时，将一段新文本追加到目标字符串 `target` 中，并在需要时插入一个空行作为分隔。算法首先检查输入 `text` 是否为空，若为空则立即返回，避免无效操作。若 `text` 非空，则检查 `target` 当前是否非空——只有非空时才会在尾部追加两个换行符 `"\n\n"`，从而在已有内容与新增内容之间形成一个显式的间隙。最后将 `text` 直接追加到 `target` 末尾。整个函数仅依赖标准库的字符串拼接操作，无其它外部依赖，其内部控制流为简单的条件判断与字符串连接。

## Side Effects

- Appends the content of `text` to the string referenced by `target`, potentially reallocating memory.

## Reads From

- `text` (input `string_view`)
- `target` (string content for checking emptiness)

## Writes To

- `target` (modified by appending `text` and optional newlines)

## Usage Patterns

- Called by `clore::net::anthropic::protocol::build_request_json` to add text blocks.

## Called By

- function `clore::net::anthropic::protocol::build_request_json`

