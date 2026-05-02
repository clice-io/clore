---
title: 'clore::support::ensureutf8'
description: 'The function clore::support::ensure_utf8 implements a validation and repair pass over a std::string_view input. It reserves storage for the result and then iterates through the input by calling the helper clore::support::(anonymous namespace)::valid_utf8_sequence_length at each offset. If the sequence length is zero—indicating an invalid lead byte—the single byte is replaced with the constant kUtf8Replacement (the Unicode replacement character U+FFFD encoded as UTF-8) and the offset advances by one. Otherwise it appends the complete valid sequence and jumps the offset forward by that length. The entire algorithm depends solely on valid_utf8_sequence_length and the replacement literal; no other local functions or global state are involved in the loop.'
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

The function `clore::support::ensure_utf8` implements a validation and repair pass over a `std::string_view` input. It reserves storage for the result and then iterates through the input by calling the helper `clore::support::(anonymous namespace)::valid_utf8_sequence_length` at each offset. If the sequence length is zero—indicating an invalid lead byte—the single byte is replaced with the constant `kUtf8Replacement` (the Unicode replacement character U+FFFD encoded as UTF-8) and the offset advances by one. Otherwise it appends the complete valid sequence and jumps the offset forward by that length. The entire algorithm depends solely on `valid_utf8_sequence_length` and the replacement literal; no other local functions or global state are involved in the loop.

## Side Effects

- Allocates memory for a new `std::string`
- Appends data to the newly allocated string

## Reads From

- `text` parameter
- `valid_utf8_sequence_length` function result
- `kUtf8Replacement` constant value
- Bytes of input `text`

## Writes To

- Output `std::string` that is returned

## Usage Patterns

- Used by `write_utf8_text_file` to sanitize input before writing
- Used by `truncate_utf8` to ensure truncated result is valid UTF-8

## Calls

- function `clore::support::(anonymous namespace)::valid_utf8_sequence_length`

## Called By

- function `clore::support::truncate_utf8`
- function `clore::support::write_utf8_text_file`

