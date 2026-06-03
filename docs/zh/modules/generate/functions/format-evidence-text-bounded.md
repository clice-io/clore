---
title: 'clore::generate::formatevidencetextbounded'
description: '函数 clore::generate::format_evidence_text_bounded 的核心算法是将证据数据按语义段组织并逐段累积文本，同时严格遵循长度限制。它首先声明两个内部结构 SymbolSection 和 TextSection 用于描述每个段的结构（标题、项目容器指针和渲染函数指针）。函数初始化一个预留 4096 字节的 std::string text，然后通过 append_if_fits 尝试追加 ## EVIDENCE\n\n 标题；若失败（标题本身已超出 max_length）则直接返回空字符串。成功后再依次处理四个符号段（"### Target\n"、"### Local Context\n"、"### Dependencies\n"、"### Used By\n"）和两个文本段（"### Source Snippets\n"、"### Related Page Summaries\n"）。每个段通过 append_section_bounded 进行追加，该函数会在内部迭代每个项目，使用对应的渲染函数（如 render_detailed_fact、render_context_fact、render_source_snippet、render_summary_item）将 SymbolFact 或 std::string 转换为文本块，并在每次追加前检查剩余可用空间，确保总长度不超过 max_length。'
layout: doc
template: doc
---

# `clore::generate::format_evidence_text_bounded`

Owner: [Module generate:evidence](../evidence.md)

Declaration: `src/generate/evidence.cppm:100`

Definition: `src/generate/evidence.cppm:596`

Declaration: [`Namespace clore::generate`](../../../namespaces/clore/generate/index.md)

## Implementation

```cpp
auto format_evidence_text_bounded(const EvidencePack& pack, std::size_t max_length) -> std::string {
    struct SymbolSection {
        std::string_view title;
        const std::vector<SymbolFact>* items = nullptr;
        std::string (*render)(const SymbolFact&) = nullptr;
    };

    struct TextSection {
        std::string_view title;
        const std::vector<std::string>* items = nullptr;
        std::string (*render)(const std::string&) = nullptr;
    };

    std::string text;
    text.reserve(4096);

    if(!append_if_fits(text, "## EVIDENCE\n\n", max_length)) {
        return {};
    }

    const std::array<SymbolSection, 4> symbol_sections{
        SymbolSection{
                      .title = "### Target\n",
                      .items = &pack.target_facts,
                      .render = &render_detailed_fact,
                      },
        SymbolSection{
                      .title = "### Local Context\n",
                      .items = &pack.local_context,
                      .render = &render_context_fact,
                      },
        SymbolSection{
                      .title = "### Dependencies\n",
                      .items = &pack.dependency_context,
                      .render = &render_context_fact,
                      },
        SymbolSection{
                      .title = "### Used By\n",
                      .items = &pack.reverse_usage_context,
                      .render = &render_context_fact,
                      },
    };

    for(const auto& section: symbol_sections) {
        append_section_bounded(text, section.title, *section.items, section.render, max_length);
    }

    const std::array<TextSection, 2> text_sections{
        TextSection{
                    .title = "### Source Snippets\n",
                    .items = &pack.source_snippets,
                    .render = &render_source_snippet,
                    },
        TextSection{
                    .title = "### Related Page Summaries\n",
                    .items = &pack.related_page_summaries,
                    .render = &render_summary_item,
                    },
    };

    for(const auto& section: text_sections) {
        append_section_bounded(text, section.title, *section.items, section.render, max_length);
    }

    return text;
}
```

函数 `clore::generate::format_evidence_text_bounded` 的核心算法是将证据数据按语义段组织并逐段累积文本，同时严格遵循长度限制。它首先声明两个内部结构 `SymbolSection` 和 `TextSection` 用于描述每个段的结构（标题、项目容器指针和渲染函数指针）。函数初始化一个预留 4096 字节的 `std::string text`，然后通过 `append_if_fits` 尝试追加 `## EVIDENCE\n\n` 标题；若失败（标题本身已超出 `max_length`）则直接返回空字符串。成功后再依次处理四个符号段（`"### Target\n"`、`"### Local Context\n"`、`"### Dependencies\n"`、`"### Used By\n"`）和两个文本段（`"### Source Snippets\n"`、`"### Related Page Summaries\n"`）。每个段通过 `append_section_bounded` 进行追加，该函数会在内部迭代每个项目，使用对应的渲染函数（如 `render_detailed_fact`、`render_context_fact`、`render_source_snippet`、`render_summary_item`）将 `SymbolFact` 或 `std::string` 转换为文本块，并在每次追加前检查剩余可用空间，确保总长度不超过 `max_length`。

整个控制流是线性的顺序追加，没有任何分支判断段的存在性——所有段都无条件尝试追加，但 `append_section_bounded` 可能会因为长度限制而跳过部分或全部项目。关键依赖是 `append_if_fits` 和 `append_section_bounded`（两者均在匿名命名空间中），它们共同实现了长度约束下的安全拼接；渲染函数则作为回调由调用方提供（已在匿名命名空间中定义）。内部结构 `SymbolSection` 和 `TextSection` 仅仅是简单的数据聚合器，没有额外逻辑。该函数本身不涉及任何外部输入的状态突变，所有数据均来自传入的 `EvidencePack` 常量引用，长度边界由 `max_length` 参数硬性限定。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `pack.target_facts`
- `pack.local_context`
- `pack.dependency_context`
- `pack.reverse_usage_context`
- `pack.source_snippets`
- `pack.related_page_summaries`
- `max_length`
- `pack` (the `EvidencePack` parameter)

## Usage Patterns

- Called by `clore::generate::format_evidence_text`

## Calls

- function `clore::generate::(anonymous namespace)::append_if_fits`

## Called By

- function `clore::generate::format_evidence_text`

