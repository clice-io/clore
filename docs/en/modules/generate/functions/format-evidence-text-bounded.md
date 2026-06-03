---
title: 'clore::generate::formatevidencetextbounded'
description: 'The function clore::generate::format_evidence_text_bounded assembles a Markdown evidence block from an EvidencePack while respecting a caller-provided max_length budget. It preallocates a result string with text.reserve(4096), then attempts to write the top-level heading "## EVIDENCE\n\n" via append_if_fits; if that fails, it returns an empty string immediately.'
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

The function `clore::generate::format_evidence_text_bounded` assembles a Markdown evidence block from an `EvidencePack` while respecting a caller-provided `max_length` budget. It preallocates a result string with `text.reserve(4096)`, then attempts to write the top-level heading `"## EVIDENCE\n\n"` via `append_if_fits`; if that fails, it returns an empty string immediately.  

Internally, it defines two local helper structs — `SymbolSection` (holding a title, a pointer to a `std::vector<SymbolFact>` and a render function pointer) and `TextSection` (analogous for `std::vector<std::string>` items). It creates an `std::array` of four `SymbolSection` instances covering target facts, local context, dependency context, and reverse usage context, in that order. A second array holds two `TextSection` instances for source snippets and related page summaries. The function iterates over each section array, calling `append_section_bounded` for each section to add its title and rendered items, always checking against `max_length` to avoid exceeding the budget. The sole explicitly listed dependency is `append_if_fits`, which is used for the initial heading and presumably also internally by `append_section_bounded` for individual lines or items. The final returned `text` contains the full (possibly truncated) evidence string.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `EvidencePack::target_facts`
- `EvidencePack::local_context`
- `EvidencePack::dependency_context`
- `EvidencePack::reverse_usage_context`
- `EvidencePack::source_snippets`
- `EvidencePack::related_page_summaries`
- `max_length` parameter

## Usage Patterns

- called by `clore::generate::format_evidence_text`
- used to generate bounded evidence strings for prompts

## Calls

- function `clore::generate::(anonymous namespace)::append_if_fits`

## Called By

- function `clore::generate::format_evidence_text`

