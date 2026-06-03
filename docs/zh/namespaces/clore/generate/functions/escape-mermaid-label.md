---
title: 'clore::generate::escapemermaidlabel'
description: 'clore::generate::escape_mermaid_label 接受一个 std::string_view 输入，返回一个 std::string。该函数负责将任意字符串转义为在 Mermaid 图（如节点标签）中安全使用的形式。调用方应使用此函数准备所有可能包含 Mermaid 语法特殊字符的标签文本，确保生成的图能够正确渲染且不会因未转义的字符而断裂。'
layout: doc
template: doc
---

# `clore::generate::escape_mermaid_label`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/render/diagram.cppm:27`

Definition: `src/generate/render/diagram.cppm:123`

Implementation: [`Module generate:diagram`](../../../../modules/generate/diagram.md)

## Declaration

```cpp
auto (std::string_view) -> std::string;
```

`clore::generate::escape_mermaid_label` 接受一个 `std::string_view` 输入，返回一个 `std::string`。该函数负责将任意字符串转义为在 Mermaid 图（如节点标签）中安全使用的形式。调用方应使用此函数准备所有可能包含 Mermaid 语法特殊字符的标签文本，确保生成的图能够正确渲染且不会因未转义的字符而断裂。

## Usage Patterns

- Called by `clore::generate::render_namespace_diagram_code` to escape Mermaid node labels

## Called By

- function `clore::generate::render_namespace_diagram_code`

