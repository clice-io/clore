---
title: 'clore::support::striputf8bom'
description: 'The function clore::support::strip_utf8_bom implements a simple prefix-stripping algorithm. It first checks whether the input std::string_view is long enough to contain the UTF-8 BOM sequence by comparing its size to std::size(kUtf8Bom). If the length requirement is met, it performs three bytewise comparisons using static_cast<unsigned char> against the constant kUtf8Bom (defined in the anonymous namespace). When all three bytes match, the function returns a view starting after the BOM via text.substr(std::size(kUtf8Bom)). Otherwise, it returns the original string view unchanged. The implementation has no loops, no allocation, and relies solely on the inline constant kUtf8Bom for the BOM byte values.'
layout: doc
template: doc
---

# `clore::support::strip_utf8_bom`

Owner: [Module support](../index.md)

Declaration: `src/support/logging.cppm:106`

Definition: `src/support/logging.cppm:493`

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

The function `clore::support::strip_utf8_bom` implements a simple prefix-stripping algorithm. It first checks whether the input `std::string_view` is long enough to contain the UTF-8 BOM sequence by comparing its size to `std::size(kUtf8Bom)`. If the length requirement is met, it performs three bytewise comparisons using `static_cast<unsigned char>` against the constant `kUtf8Bom` (defined in the anonymous namespace). When all three bytes match, the function returns a view starting after the BOM via `text.substr(std::size(kUtf8Bom))`. Otherwise, it returns the original string view unchanged. The implementation has no loops, no allocation, and relies solely on the inline constant `kUtf8Bom` for the BOM byte values.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- text

## Usage Patterns

- called by `read_utf8_text_file` to strip BOM

## Called By

- function `clore::support::read_utf8_text_file`

