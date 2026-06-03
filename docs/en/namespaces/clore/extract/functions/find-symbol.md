---
title: 'clore::extract::findsymbol'
description: 'clore::extract::find_symbol looks up a symbol in the provided ProjectModel by its fully qualified name. It returns a pointer to the matching SymbolInfo if exactly one symbol with that name exists; otherwise it returns nullptr. This overload serves as the core qualified‑name lookup and is used by the three‑parameter overload that additionally filters by an optional signature.'
layout: doc
template: doc
---

# `clore::extract::find_symbol`

Owner: [Namespace clore::extract](../index.md)

Declaration: `src/extract/model.cppm:196`

Definition: `src/extract/model.cppm:388`

Implementation: [`Module extract:model`](../../../../modules/extract/model.md)

## Declaration

```cpp
auto (const ProjectModel &, std::string_view) -> const SymbolInfo *;
```

`clore::extract::find_symbol` looks up a symbol in the provided `ProjectModel` by its fully qualified name. It returns a pointer to the matching `SymbolInfo` if exactly one symbol with that name exists; otherwise it returns `nullptr`. This overload serves as the core qualified‑name lookup and is used by the three‑parameter overload that additionally filters by an optional `signature`.

## Usage Patterns

- Used to resolve a uniquely identifiable symbol by qualified name.
- Called by the three-parameter overload to refine lookup by module.

## Called By

- function `clore::extract::find_symbol`

