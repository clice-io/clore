---
title: 'clore::support::striputf8bom'
description: 'The implementation first verifies that the input text is at least as long as the kUtf8Bom byte sequence (three bytes). It then compares the first three bytes of text against kUtf8Bom, using static_cast<unsigned char> to avoid sign‑extension issues with char types. If all three bytes match, it returns a std::string_view starting after the BOM by calling text.substr(std::size(kUtf8Bom)). Otherwise, it returns the original text unchanged. The only dependency is the constant kUtf8Bom, which is defined in the anonymous namespace within the same translation unit.'
layout: doc
template: doc
---

# `clore::support::strip_utf8_bom`

Owner: [Module support](../index.md)

Declaration: `support/logging.cppm:83`

Definition: `support/logging.cppm:470`

Declaration: [`Namespace clore::support`](../../../namespaces/clore/support/index.md)

## Implementation

```cpp
auto strip_utf8_bom(std::string_view text) -> std::string_view {
    if(text.size() >= std::size(kUtf8Bom) && static_cast<unsigned char>(text[0]) == kUtf8Bom[0] &&
       static_cast<unsigned char>(text[1]) == kUtf8Bom[1] &&
       static_cast<unsigned char>(text[2]) == kUtf8Bom[2]) {
        return text.substr(std::size(kUtf8Bom));
    }

    return text;
}
```

The implementation first verifies that the input `text` is at least as long as the `kUtf8Bom` byte sequence (three bytes). It then compares the first three bytes of `text` against `kUtf8Bom`, using `static_cast<unsigned char>` to avoid sign‑extension issues with `char` types. If all three bytes match, it returns a `std::string_view` starting after the BOM by calling `text.substr(std::size(kUtf8Bom))`. Otherwise, it returns the original `text` unchanged. The only dependency is the constant `kUtf8Bom`, which is defined in the anonymous namespace within the same translation unit.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- parameter `text` of type `std::string_view`
- constant `kUtf8Bom` (likely a three‑byte array `{0xEF, 0xBB, 0xBF}`)

## Usage Patterns

- Stripping the UTF‑8 BOM from file contents before processing in `clore::support::read_utf8_text_file`

## Called By

- function `clore::support::read_utf8_text_file`

