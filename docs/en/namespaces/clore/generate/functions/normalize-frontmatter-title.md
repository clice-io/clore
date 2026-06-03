---
title: 'clore::generate::normalizefrontmattertitle'
description: 'Returns a normalized version of the provided frontmatter title string. The input is a raw title string, and the output is a processed, canonical form suitable for embedding in the YAML frontmatter of generated documentation pages. This function ensures consistent formatting of title metadata across all generated pages, typically by trimming whitespace, removing extraneous characters, and applying any standard transformations required by the generation pipeline.'
layout: doc
template: doc
---

# `clore::generate::normalize_frontmatter_title`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/render/symbol.cppm:51`

Definition: `src/generate/render/symbol.cppm:903`

Implementation: [`Module generate:symbol`](../../../../modules/generate/symbol.md)

## Declaration

```cpp
auto (std::string_view) -> std::string;
```

Returns a normalized version of the provided frontmatter title string. The input is a raw title string, and the output is a processed, canonical form suitable for embedding in the YAML frontmatter of generated documentation pages. This function ensures consistent formatting of title metadata across all generated pages, typically by trimming whitespace, removing extraneous characters, and applying any standard transformations required by the generation pipeline.

## Usage Patterns

- Used to normalize frontmatter titles for documentation pages

## Called By

- function `clore::generate::(anonymous namespace)::build_symbol_frontmatter`

