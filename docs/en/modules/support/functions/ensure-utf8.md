---
title: 'clore::support::ensureutf8'
description: 'The implementation of clore::support::ensure_utf8 iterates over the input text byte by byte, using the helper function valid_utf8_sequence_length (defined in an anonymous namespace) to determine the length of the UTF-8 sequence starting at each offset. If the helper returns 0, the current byte is not a valid UTF-8 lead byte; in that case a replacement character (kUtf8Replacement) is appended to the output, and the iteration advances by one. When a valid sequence length is found, the corresponding substring is appended and offset is incremented by that length. The result accumulates into a std::string preallocated to the input size, and is returned as a fully valid UTF-8 string. The only explicit dependency is the sequence-length validator, which encapsulates the UTF-8 encoding rules.'
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

The implementation of `clore::support::ensure_utf8` iterates over the input `text` byte by byte, using the helper function `valid_utf8_sequence_length` (defined in an anonymous namespace) to determine the length of the UTF-8 sequence starting at each `offset`. If the helper returns `0`, the current byte is not a valid UTF-8 lead byte; in that case a replacement character (`kUtf8Replacement`) is appended to the output, and the iteration advances by one. When a valid sequence length is found, the corresponding substring is appended and `offset` is incremented by that length. The result accumulates into a `std::string` preallocated to the input size, and is returned as a fully valid UTF-8 string. The only explicit dependency is the sequence-length validator, which encapsulates the UTF-8 encoding rules.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- parameter `text`
- constant `kUtf8Replacement`

## Writes To

- returned `std::string`

## Usage Patterns

- Ensuring text is valid UTF-8 before passing to `write_utf8_text_file`
- Sanitizing input before truncation in `truncate_utf8`

## Calls

- function `clore::support::(anonymous namespace)::valid_utf8_sequence_length`

## Called By

- function `clore::support::truncate_utf8`
- function `clore::support::write_utf8_text_file`

