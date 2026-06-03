---
title: 'clore::generate::formatevidencetextbounded'
description: 'clore::generate::format_evidence_text_bounded 接受一个 const EvidencePack & 和一个 std::size_t 参数，返回一个 std::string。该函数将提供的证据包格式化为一段文本，同时保证输出字符串的长度不超过传入的 std::size_t 界限。如果格式化后的证据超出该界限，函数会将其截断，确保结果始终满足长度约束。调用者应负责传递一个合法的证据包引用，并指定一个期望的最大字符数；返回值可直接用于后续的提示构建或文本展示，无需调用者自行处理截断逻辑。'
layout: doc
template: doc
---

# `clore::generate::format_evidence_text_bounded`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/evidence.cppm:100`

Definition: `src/generate/evidence.cppm:596`

Implementation: [`Module generate:evidence`](../../../../modules/generate/evidence.md)

## Declaration

```cpp
auto (const EvidencePack &, std::size_t) -> std::string;
```

`clore::generate::format_evidence_text_bounded` 接受一个 `const EvidencePack &` 和一个 `std::size_t` 参数，返回一个 `std::string`。该函数将提供的证据包格式化为一段文本，同时保证输出字符串的长度不超过传入的 `std::size_t` 界限。如果格式化后的证据超出该界限，函数会将其截断，确保结果始终满足长度约束。调用者应负责传递一个合法的证据包引用，并指定一个期望的最大字符数；返回值可直接用于后续的提示构建或文本展示，无需调用者自行处理截断逻辑。

## Usage Patterns

- Called by `clore::generate::format_evidence_text`

## Calls

- function `clore::generate::(anonymous namespace)::append_if_fits`

## Called By

- function `clore::generate::format_evidence_text`

