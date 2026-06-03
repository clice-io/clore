---
title: 'clore::support::ensureutf8'
description: '该函数遍历输入的 text 字符串，在每个偏移位置调用 valid_utf8_sequence_length 确定当前字节序列的 UTF-8 编码长度。若返回零，说明该字节不是一个有效序列的起始字节，此时向结果 normalized 追加替换字符 kUtf8Replacement 并将偏移量前进一个字节；否则将有效序列直接追加到 normalized 中，偏移量前进相应的序列长度。整个过程仅依赖于辅助函数 valid_utf8_sequence_length，通过逐字节检查来替换无效字节，从而保证输出始终是合法的 UTF-8 字符串。'
layout: doc
template: doc
---

# `clore::support::ensure_utf8`

Owner: [Module support](../index.md)

Declaration: `src/support/logging.cppm:98`

Definition: `src/support/logging.cppm:428`

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

该函数遍历输入的 `text` 字符串，在每个偏移位置调用 `valid_utf8_sequence_length` 确定当前字节序列的 UTF-8 编码长度。若返回零，说明该字节不是一个有效序列的起始字节，此时向结果 `normalized` 追加替换字符 `kUtf8Replacement` 并将偏移量前进一个字节；否则将有效序列直接追加到 `normalized` 中，偏移量前进相应的序列长度。整个过程仅依赖于辅助函数 `valid_utf8_sequence_length`，通过逐字节检查来替换无效字节，从而保证输出始终是合法的 UTF-8 字符串。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `text` parameter
- `kUtf8Replacement` constant

## Writes To

- local variable `normalized` (returned)

## Usage Patterns

- Normalizing input strings to ensure valid UTF-8 encoding
- Used by `write_utf8_text_file` to guarantee valid UTF-8 before writing
- Used by `truncate_utf8` to ensure truncated string ends at a valid boundary

## Calls

- function `clore::support::(anonymous namespace)::valid_utf8_sequence_length`

## Called By

- function `clore::support::truncate_utf8`
- function `clore::support::write_utf8_text_file`

