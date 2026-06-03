---
title: 'clore::support::ensureutf8'
description: 'clore::support::ensure_utf8 接受一个 std::string_view 并返回一个 std::string，该结果保证是有效的 UTF‑8 编码。调用者可将此函数用作一个适配层，确保无论输入数据的编码状态如何，下游处理都能接收到一个符合 UTF‑8 规范的字符串。函数内部会处理任何无效的字节序列，生成语义上可用的输出。'
layout: doc
template: doc
---

# `clore::support::ensure_utf8`

Owner: [Namespace clore::support](../index.md)

Declaration: `src/support/logging.cppm:98`

Definition: `src/support/logging.cppm:428`

Implementation: [`Module support`](../../../../modules/support/index.md)

## Declaration

```cpp
auto (std::string_view) -> std::string;
```

`clore::support::ensure_utf8` 接受一个 `std::string_view` 并返回一个 `std::string`，该结果保证是有效的 UTF‑8 编码。调用者可将此函数用作一个适配层，确保无论输入数据的编码状态如何，下游处理都能接收到一个符合 UTF‑8 规范的字符串。函数内部会处理任何无效的字节序列，生成语义上可用的输出。

## Usage Patterns

- Normalizing input strings to ensure valid UTF-8 encoding
- Used by `write_utf8_text_file` to guarantee valid UTF-8 before writing
- Used by `truncate_utf8` to ensure truncated string ends at a valid boundary

## Calls

- function `clore::support::(anonymous namespace)::valid_utf8_sequence_length`

## Called By

- function `clore::support::truncate_utf8`
- function `clore::support::write_utf8_text_file`

