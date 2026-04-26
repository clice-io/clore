---
title: 'clore::support::ensureutf8'
description: 'The function clore::support::ensure_utf8 iterates over the input text character‑by‑character using an offset index. At each position it invokes valid_utf8_sequence_length to determine the byte length of a valid UTF‑8 sequence starting there. If that length is zero — indicating an invalid leading byte or misplaced continuation byte — the algorithm appends the constant kUtf8Replacement (the Unicode replacement character U+FFFD encoded in UTF‑8) to normalized and advances offset by one. Otherwise it appends the identified valid sequence via text.substr(offset, sequence_length) and increments offset by that sequence_length. The result is a std::string that is guaranteed to contain only well‑formed UTF‑8, with any malformed bytes replaced by the standard replacement character.'
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

The function `clore::support::ensure_utf8` iterates over the input `text` character‑by‑character using an `offset` index. At each position it invokes `valid_utf8_sequence_length` to determine the byte length of a valid UTF‑8 sequence starting there. If that length is zero — indicating an invalid leading byte or misplaced continuation byte — the algorithm appends the constant `kUtf8Replacement` (the Unicode replacement character U+FFFD encoded in UTF‑8) to `normalized` and advances `offset` by one. Otherwise it appends the identified valid sequence via `text.substr(offset, sequence_length)` and increments `offset` by that `sequence_length`. The result is a `std::string` that is guaranteed to contain only well‑formed UTF‑8, with any malformed bytes replaced by the standard replacement character.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `text` parameter

## Writes To

- local `normalized` string (returned by value)

## Usage Patterns

- called by `write_utf8_text_file` to ensure output is valid UTF-8
- called by `truncate_utf8` to sanitize input before truncation

## Calls

- function `clore::support::(anonymous namespace)::valid_utf8_sequence_length`

## Called By

- function `clore::support::truncate_utf8`
- function `clore::support::write_utf8_text_file`

