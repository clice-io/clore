---
title: 'clore::support::ensureutf8'
description: '函数 clore::support::ensure_utf8 遍历输入字符串的每个字节偏移，核心控制流依赖于调用 valid_utf8_sequence_length 来判断从当前位置开始的字节序列是否构成合法的 UTF-8 编码。若该函数返回非零长度，则直接将对应子串追加到结果中；若返回零（表示无效起始字节或编码错误），则追加一个预定义的替换字符 kUtf8Replacement 并仅将偏移前进一个字节，以容忍非法序列。内部使用 reserve 预分配与输入等长的内存以避免多次重分配。该实现不依赖任何外部状态，完全基于对每个字节的局部解码决策。'
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

函数 `clore::support::ensure_utf8` 遍历输入字符串的每个字节偏移，核心控制流依赖于调用 `valid_utf8_sequence_length` 来判断从当前位置开始的字节序列是否构成合法的 UTF-8 编码。若该函数返回非零长度，则直接将对应子串追加到结果中；若返回零（表示无效起始字节或编码错误），则追加一个预定义的替换字符 `kUtf8Replacement` 并仅将偏移前进一个字节，以容忍非法序列。内部使用 `reserve` 预分配与输入等长的内存以避免多次重分配。该实现不依赖任何外部状态，完全基于对每个字节的局部解码决策。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `text` parameter
- `valid_utf8_sequence_length` function
- `kUtf8Replacement` constant

## Writes To

- returned `std::string`

## Usage Patterns

- called by `write_utf8_text_file` to ensure valid UTF-8 before writing
- called by `truncate_utf8` to sanitize input before truncation

## Calls

- function `clore::support::(anonymous namespace)::valid_utf8_sequence_length`

## Called By

- function `clore::support::truncate_utf8`
- function `clore::support::write_utf8_text_file`

