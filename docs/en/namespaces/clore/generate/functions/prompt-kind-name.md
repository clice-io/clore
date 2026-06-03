---
title: 'clore::generate::promptkindname'
description: 'Converts a given PromptKind value to its corresponding human-readable string name. The function returns a std::string_view that serves as a stable text identifier for the kind, suitable for use in key construction, logging, or diagnostic output. It is called by prompt_request_key to obtain the kind portion of a unique request key.'
layout: doc
template: doc
---

# `clore::generate::prompt_kind_name`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/model.cppm:48`

Definition: `src/generate/model.cppm:289`

Implementation: [`Module generate:model`](../../../../modules/generate/model.md)

## Declaration

```cpp
auto (PromptKind) -> std::string_view;
```

Converts a given `PromptKind` value to its corresponding human-readable string name. The function returns a `std::string_view` that serves as a stable text identifier for the kind, suitable for use in key construction, logging, or diagnostic output. It is called by `prompt_request_key` to obtain the kind portion of a unique request key.

## Usage Patterns

- Called by `clore::generate::prompt_request_key` to derive a string key for a given prompt request.

## Called By

- function `clore::generate::prompt_request_key`

