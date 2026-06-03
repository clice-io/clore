---
title: 'clore::net::anthropic::protocol::detail::appendtextwithgap'
description: 'Appends a text segment, provided as a std::string_view, to the given std::string, ensuring a gap (typically a newline) is inserted between any existing content and the newly appended text. By managing this separation, the function helps structure the output string so that logically distinct textual parts remain visually separated, which is useful when constructing multi‑line message bodies or JSON content. Callers supply the destination string and the text to append; after the call, the destination string contains the original content, a gap, and the appended text.'
layout: doc
template: doc
---

# `clore::net::anthropic::protocol::detail::append_text_with_gap`

Owner: [Namespace clore::net::anthropic::protocol::detail](../index.md)

Declaration: `src/network/anthropic.cppm:34`

Definition: `src/network/anthropic.cppm:34`

Implementation: [`Module anthropic`](../../../../../../../modules/anthropic/index.md)

## Declaration

```cpp
auto (std::string &, std::string_view) -> void;
```

Appends a text segment, provided as a `std::string_view`, to the given `std::string`, ensuring a gap (typically a newline) is inserted between any existing content and the newly appended text. By managing this separation, the function helps structure the output string so that logically distinct textual parts remain visually separated, which is useful when constructing multi‑line message bodies or JSON content. Callers supply the destination string and the text to append; after the call, the destination string contains the original content, a gap, and the appended text.

## Usage Patterns

- called by `build_request_json` to concatenate text segments with separation

## Called By

- function `clore::net::anthropic::protocol::build_request_json`

