---
title: 'clore::generate::formatevidencetext'
description: '函数 clore::generate::format_evidence_text 的实现是一个轻量级转发器。它直接调用 clore::generate::format_evidence_text_bounded，并将参数 pack 与 std::numeric_limits<std::size_t>::max() 一同传递。该有界函数（实际承担所有工作）随后会使用最大可能的长度限制构建证据文本，从而在效果上不施加任何长度截断。这种设计将格式化逻辑集中到有界变体，同时通过无界包装器提供便利入口，避免了重复实现。'
layout: doc
template: doc
---

# `clore::generate::format_evidence_text`

Owner: [Module generate:evidence](../evidence.md)

Declaration: `src/generate/evidence.cppm:98`

Definition: `src/generate/evidence.cppm:592`

Declaration: [`Namespace clore::generate`](../../../namespaces/clore/generate/index.md)

## Implementation

```cpp
auto format_evidence_text(const EvidencePack& pack) -> std::string {
    return format_evidence_text_bounded(pack, std::numeric_limits<std::size_t>::max());
}
```

函数 `clore::generate::format_evidence_text` 的实现是一个轻量级转发器。它直接调用 `clore::generate::format_evidence_text_bounded`，并将参数 `pack` 与 `std::numeric_limits<std::size_t>::max()` 一同传递。该有界函数（实际承担所有工作）随后会使用最大可能的长度限制构建证据文本，从而在效果上不施加任何长度截断。这种设计将格式化逻辑集中到有界变体，同时通过无界包装器提供便利入口，避免了重复实现。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `pack` parameter of type `const EvidencePack&`

## Writes To

- returned `std::string`

## Usage Patterns

- Used by `build_prompt` to format evidence for prompts

## Calls

- function `clore::generate::format_evidence_text_bounded`

## Called By

- function `clore::generate::build_prompt`

