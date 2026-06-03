---
title: 'clore::generate::trimascii'
description: 'The implementation of clore::generate::trim_ascii uses two sequential while loops to strip leading and trailing ASCII whitespace from a std::string_view. The first loop repeatedly inspects the first character via text.front(), checks it with std::isspace (applied to an unsigned char to avoid undefined behavior for negative values), and calls text.remove_prefix(1) until a non‑space character is encountered. The second loop performs the same logic from the end using text.back() and text.remove_suffix(1). The function returns the trimmed view after both loops complete. This approach requires only the standard library facilities <cctype> for std::isspace and <string_view> for std::string_view, and performs O(n) comparisons in the worst case where almost the entire string is whitespace.'
layout: doc
template: doc
---

# `clore::generate::trim_ascii`

Owner: [Module generate:common](../common.md)

Declaration: `src/generate/render/common.cppm:34`

Definition: `src/generate/render/common.cppm:34`

Declaration: [`Namespace clore::generate`](../../../namespaces/clore/generate/index.md)

## Implementation

```cpp
auto trim_ascii(std::string_view text) -> std::string_view {
    while(!text.empty() && std::isspace(static_cast<unsigned char>(text.front())) != 0) {
        text.remove_prefix(1);
    }
    while(!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
        text.remove_suffix(1);
    }
    return text;
}
```

The implementation of `clore::generate::trim_ascii` uses two sequential while loops to strip leading and trailing ASCII whitespace from a `std::string_view`. The first loop repeatedly inspects the first character via `text.front()`, checks it with `std::isspace` (applied to an `unsigned char` to avoid undefined behavior for negative values), and calls `text.remove_prefix(1)` until a non‑space character is encountered. The second loop performs the same logic from the end using `text.back()` and `text.remove_suffix(1)`. The function returns the trimmed view after both loops complete. This approach requires only the standard library facilities `<cctype>` for `std::isspace` and `<string_view>` for `std::string_view`, and performs O(n) comparisons in the worst case where almost the entire string is whitespace.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- text parameter

## Usage Patterns

- Used to trim whitespace from prompt texts
- Used in stripping inline markdown
- Used when adding prompt output

## Called By

- function `clore::generate::add_prompt_output`
- function `clore::generate::build_prompt_section`
- function `clore::generate::strip_inline_markdown`

