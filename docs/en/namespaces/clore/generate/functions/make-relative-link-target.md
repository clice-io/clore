---
title: 'clore::generate::makerelativelinktarget'
description: 'The function clore::generate::make_relative_link_target computes a relative filesystem path between two paths, both provided as std::string_view, and returns the resulting relative path as a std::string. It is a utility used internally when constructing link targets for generated documentation pages. The caller is responsible for supplying a valid source path and a valid target path; the function resolves the relative relationship between them without any side effects. The returned string is intended to be used as the path component of a LinkTarget, as seen in the caller clore::generate::make_link_target, which combines this relative path with a label and an optional code-style flag into a complete LinkTarget object.'
layout: doc
template: doc
---

# `clore::generate::make_relative_link_target`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/render/common.cppm:68`

Definition: `src/generate/render/common.cppm:68`

Implementation: [`Module generate:common`](../../../../modules/generate/common.md)

## Declaration

```cpp
auto (std::string_view, std::string_view) -> std::string;
```

The function `clore::generate::make_relative_link_target` computes a relative filesystem path between two paths, both provided as `std::string_view`, and returns the resulting relative path as a `std::string`. It is a utility used internally when constructing link targets for generated documentation pages. The caller is responsible for supplying a valid source path and a valid target path; the function resolves the relative relationship between them without any side effects. The returned string is intended to be used as the path component of a `LinkTarget`, as seen in the caller `clore::generate::make_link_target`, which combines this relative path with a label and an optional code-style flag into a complete `LinkTarget` object.

## Usage Patterns

- Called by `clore::generate::make_link_target` to produce a relative link target for documentation pages.

## Called By

- function `clore::generate::make_link_target`

