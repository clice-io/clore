---
title: 'clore::support::striputf8bom'
description: 'The function checks whether the input std::string_view text begins with the UTF‑8 byte‑order mark (BOM) stored in the constant kUtf8Bom. It first verifies that text.size() is at least std::size(kUtf8Bom) (three bytes) and then compares each of the first three bytes using static_cast<unsigned char> to avoid sign extension. If all three match, it returns the substring starting after the BOM; otherwise it returns the original text unchanged. No additional helpers or external dependencies are required; the only dependency is the constant kUtf8Bom defined in the same anonymous namespace.'
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

The function checks whether the input `std::string_view text` begins with the UTF‑8 byte‑order mark (BOM) stored in the constant `kUtf8Bom`. It first verifies that `text.size()` is at least `std::size(kUtf8Bom)` (three bytes) and then compares each of the first three bytes using `static_cast<unsigned char>` to avoid sign extension. If all three match, it returns the substring starting after the BOM; otherwise it returns the original `text` unchanged. No additional helpers or external dependencies are required; the only dependency is the constant `kUtf8Bom` defined in the same anonymous namespace.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- parameter `text`
- constant `kUtf8Bom`

## Usage Patterns

- called by `clore::support::read_utf8_text_file` to strip BOM from file contents

## Called By

- function `clore::support::read_utf8_text_file`

