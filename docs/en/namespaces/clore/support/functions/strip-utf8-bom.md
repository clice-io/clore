---
title: 'clore::support::striputf8bom'
description: 'Strips the UTF-8 byte order mark (BOM) from the beginning of the given std::string_view and returns a view pointing to the remainder of the string. If the input does not start with a UTF-8 BOM, the returned view is identical to the input. The function does not modify the original string and operates purely by adjusting the view bounds.'
layout: doc
template: doc
---

# `clore::support::strip_utf8_bom`

Owner: [Namespace clore::support](../index.md)

Declaration: `src/support/logging.cppm:106`

Definition: `src/support/logging.cppm:493`

Implementation: [`Module support`](../../../../modules/support/index.md)

## Declaration

```cpp
auto (std::string_view) -> std::string_view;
```

Strips the UTF-8 byte order mark (BOM) from the beginning of the given `std::string_view` and returns a view pointing to the remainder of the string. If the input does not start with a UTF-8 BOM, the returned view is identical to the input. The function does not modify the original string and operates purely by adjusting the view bounds.

## Usage Patterns

- called by `read_utf8_text_file` to strip BOM

## Called By

- function `clore::support::read_utf8_text_file`

