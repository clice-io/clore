---
title: 'clore::support::ensureutf8'
description: '该函数的核心实现是一个逐字节扫描的修正循环。它从输入 text 的起始偏移量开始，每次调用 clore::support::(anonymous namespace)::valid_utf8_sequence_length 判断当前偏移处能否解析为一个合法的 UTF‑8 序列。若返回非零长度，则通过 std::string::append 将对应的子串原样拷贝至结果 normalized 中，并将偏移量前进该长度；否则（返回 0）表示该字节无效，此时向结果中追加一个替换字符（通过常量 clore::support::(anonymous namespace)::kUtf8Replacement 获取），并将偏移量仅向前移动一个字节，继续扫描。'
layout: doc
template: doc
---

# `clore::support::ensure_utf8`

Owner: [Module support](../index.md)

Declaration: `support/logging.cppm:75`

Definition: `support/logging.cppm:405`

Declaration: [`Namespace clore::support`](../../../namespaces/clore/support/index.md)

## Implementation

```cpp
auto ensure_utf8(std::string_view text) -> std::string {
    std::string normalized;
    normalized.reserve(text.size());

    for(std::size_t offset = 0; offset < text.size();) {
        auto sequence_length = valid_utf8_sequence_length(text, offset);
        if(sequence_length == 0) {
            normalized += kUtf8Replacement;
            ++offset;
            continue;
        }

        normalized.append(text.substr(offset, sequence_length));
        offset += sequence_length;
    }

    return normalized;
}
```

该函数的核心实现是一个逐字节扫描的修正循环。它从输入 `text` 的起始偏移量开始，每次调用 `clore::support::(anonymous namespace)::valid_utf8_sequence_length` 判断当前偏移处能否解析为一个合法的 UTF‑8 序列。若返回非零长度，则通过 `std::string::append` 将对应的子串原样拷贝至结果 `normalized` 中，并将偏移量前进该长度；否则（返回 0）表示该字节无效，此时向结果中追加一个替换字符（通过常量 `clore::support::(anonymous namespace)::kUtf8Replacement` 获取），并将偏移量仅向前移动一个字节，继续扫描。

该算法的控制流仅依赖循环与条件分支，无递归或外部 I/O。所有 UTF‑8 合法性判断委托给 `valid_utf8_sequence_length`，该函数负责处理单字节、多字节序列的边界检测以及过长的编码、孤立续字节等非法情况。结果字符串通过 `reserve` 预先分配与输入等长的容量，避免重复扩容。

## Side Effects

- 分配并构造一个新的 `std::string`

## Reads From

- `text` 参数（`std::string_view`）
- 内部常量 `kUtf8Replacement`

## Usage Patterns

- 在输出或进一步处理前清理字符串
- 被 `write_utf8_text_file` 和 `truncate_utf8` 调用

## Calls

- function `clore::support::(anonymous namespace)::valid_utf8_sequence_length`

## Called By

- function `clore::support::truncate_utf8`
- function `clore::support::write_utf8_text_file`

