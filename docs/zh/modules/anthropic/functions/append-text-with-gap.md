---
title: 'clore::net::anthropic::protocol::detail::appendtextwithgap'
description: '该函数在目标字符串 target 尾部追加给定的 text，并在 target 已有内容时插入两个换行符 \n\n 作为分隔符，以确保不同文本块之间保持视觉间隔。内部控制流首先检查 text 是否为空，若为空则直接返回，避免无意义操作；若 text 非空，则判断 target 是否非空，若非空则在追加前先写入 "\n\n"，最后执行实际的字符串拼接。函数没有外部依赖，仅依赖 std::string 和 std::string_view 的基本操作，实现紧凑的文本块拼接逻辑。'
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

该函数在目标字符串 `target` 尾部追加给定的 `text`，并在 `target` 已有内容时插入两个换行符 `\n\n` 作为分隔符，以确保不同文本块之间保持视觉间隔。内部控制流首先检查 `text` 是否为空，若为空则直接返回，避免无意义操作；若 `text` 非空，则判断 `target` 是否非空，若非空则在追加前先写入 `"\n\n"`，最后执行实际的字符串拼接。函数没有外部依赖，仅依赖 `std::string` 和 `std::string_view` 的基本操作，实现紧凑的文本块拼接逻辑。

## Side Effects

- modifies `target` string by appending newlines and text

## Reads From

- parameter `target` (reads its empty state)
- parameter `text` (reads entire content)

## Writes To

- parameter `target` (appends to it)

## Usage Patterns

- called in `build_request_json` to accumulate text blocks with gaps

## Called By

- function `clore::net::anthropic::protocol::build_request_json`

