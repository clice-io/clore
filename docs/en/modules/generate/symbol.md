---
title: 'Module generate:symbol'
description: 'The generate:symbol module is responsible for planning and rendering documentation pages for individual code symbols. It owns the core data structures SymbolDocPlan and PageDocLayout, which capture the documentation decisions and structural arrangement for a symbol’s page. The module provides public entry points for building page layouts (build_page_doc_layout), iterating over symbol documentation groups (for_each_symbol_doc_group), appending symbol doc pages to the generation pipeline (append_symbol_doc_pages), and querying page capabilities (page_supports_symbol_subpages). It also exposes utilities for title normalization (normalize_frontmatter_title), index path resolution (find_doc_index_path), and inserting cross-reference links and type‑member sections (add_symbol_doc_links, append_type_member_sections). These functions and types form the public API that drives the symbol‑specific page generation process.'
layout: doc
template: doc
---

# Module `generate:symbol`

## Summary

The `generate:symbol` module is responsible for planning and rendering documentation pages for individual code symbols. It owns the core data structures `SymbolDocPlan` and `PageDocLayout`, which capture the documentation decisions and structural arrangement for a symbol’s page. The module provides public entry points for building page layouts (`build_page_doc_layout`), iterating over symbol documentation groups (`for_each_symbol_doc_group`), appending symbol doc pages to the generation pipeline (`append_symbol_doc_pages`), and querying page capabilities (`page_supports_symbol_subpages`). It also exposes utilities for title normalization (`normalize_frontmatter_title`), index path resolution (`find_doc_index_path`), and inserting cross-reference links and type‑member sections (`add_symbol_doc_links`, `append_type_member_sections`). These functions and types form the public API that drives the symbol‑specific page generation process.

## Imports

- [`config`](../config/index.md)
- [`extract`](../extract/index.md)
- [`generate:common`](common.md)
- [`generate:diagram`](diagram.md)
- [`generate:markdown`](markdown.md)
- [`generate:model`](model.md)
- `std`

## Imported By

- [`generate:page`](page.md)
- [`generate:scheduler`](scheduler.md)

## Types

### `clore::generate::PageDocLayout`

Declaration: `generate/render/symbol.cppm:19`

Definition: `generate/render/symbol.cppm:19`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::PageDocLayout` serves as an internal container that partitions all documentation plans for a single rendered page by symbol category. It holds three separate vectors of `SymbolDocPlan` — `type_docs`, `variable_docs`, and `function_docs` — each storing the plans for its respective kind of symbol. This separation simplifies the rendering logic by allowing different layout or ordering rules per category while keeping the plans for all symbols of the page collected together. Additionally, the `index_paths` unordered map associates string keys (typically fully qualified symbol names or link labels) with file-system paths, enabling efficient generation of cross-reference links or an index of pages that contain those symbols. The struct imposes no invariants beyond those implied by the container types: each vector is a simple sequence of plans, and the map must contain no duplicate keys.

#### Invariants

- `index_paths` keys are unique as per `std::unordered_map`
- Each vector may be empty
- Elements in each vector are `SymbolDocPlan` instances

#### Key Members

- `type_docs`
- `variable_docs`
- `function_docs`
- `index_paths`

#### Usage Patterns

- Populated by documentation generation logic
- Consumed by page rendering code to produce final output

### `clore::generate::SymbolDocPlan`

Declaration: `generate/render/symbol.cppm:13`

Definition: `generate/render/symbol.cppm:13`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::SymbolDocPlan` implements a recursive node in a documentation plan tree. It holds three fields: an owning pointer `symbol` (to a `extract::SymbolInfo`, typically pointing into a symbol collection, may be null for the root), a `std::string index_path` for generating output file paths, and a `std::vector<SymbolDocPlan> children` that recursively establishes the tree hierarchy. The key invariant is that the tree formed by `children` mirrors the semantic nesting of the symbols being documented (e.g., namespace → class → member). The `symbol` pointer is non‑owning; the referenced object must outlive the plan. The `index_path` is used during rendering to determine file locations, and the recursive structure enables depth‑first traversal without maintaining explicit parent links.

#### Invariants

- `symbol` may be null if no symbol info is associated
- `children` can be empty if the symbol has no sub-symbols
- Each `SymbolDocPlan` represents a node in a hierarchical documentation plan

#### Key Members

- `symbol`
- `index_path`
- `children`

#### Usage Patterns

- Used to construct a tree of documentation plans for symbols and their nested children
- Likely processed by a renderer to generate documentation output

## Variables

### `clore::generate::add_symbol_doc_links`

Declaration: `generate/render/symbol.cppm:43`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

This variable is consumed by `clore::generate::(anonymous namespace)::render_symbol_page` to add documentation links to rendered symbol pages. It likely holds a callable or function reference that performs the link insertion.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- called by `render_symbol_page`

### `clore::generate::append_symbol_doc_pages`

Declaration: `generate/render/symbol.cppm:60`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Based on its name, `append_symbol_doc_pages` is likely used to append documentation pages for symbols during generation. No evidence of its specific role or consumption is provided.

#### Mutation

No mutation is evident from the extracted code.

### `clore::generate::append_type_member_sections`

Declaration: `generate/render/symbol.cppm:49`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The variable's role is not explicitly shown in the evidence; it appears only in a declaration snippet. It is probably a callable (e.g., a function pointer or lambda) that participates in rendering type member documentation sections, but no further usage or mutation is observable from the provided context.

#### Mutation

No mutation is evident from the extracted code.

## Functions

### `clore::generate::build_page_doc_layout`

Declaration: `generate/render/symbol.cppm:37`

Definition: `generate/render/symbol.cppm:897`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function first performs early‑exit checks: if `page_supports_symbol_subpages` returns false, or the base directory derived via `page_directory_of` is empty, it returns an empty `PageDocLayout`. Otherwise it collects all relevant symbols for the page—using `collect_namespace_symbols` for namespace pages and `collect_implementation_symbols` otherwise—and classifies them into three vectors based on kind predicates (`is_type_kind`, `is_variable_kind`, `is_function_kind`). Each category is then processed by `build_symbol_doc_plans` to populate the corresponding field of the layout: `type_docs`, `variable_docs`, and `function_docs`. Finally `for_each_symbol_doc_group` iterates over every group of plans and calls `register_symbol_doc_plan` to integrate them into the layout, which is then returned.

#### Side Effects

- Modifies the returned `PageDocLayout` object by populating its `type_docs`, `variable_docs`, `function_docs` members and registering symbol doc plans via `register_symbol_doc_plan`

#### Reads From

- plan (`PagePlan`)
- model (`ProjectModel`)
- `page_supports_symbol_subpages`
- `page_directory_of`
- `collect_namespace_symbols`
- `collect_implementation_symbols`
- `is_type_kind`
- `is_variable_kind`
- `is_function_kind`
- `build_symbol_doc_plans`
- `for_each_symbol_doc_group`
- `register_symbol_doc_plan`

#### Writes To

- Returned `PageDocLayout` object (fields: `type_docs`, `variable_docs`, `function_docs`)
- (via `register_symbol_doc_plan`) internal mapping within layout

#### Usage Patterns

- Called during page documentation generation to construct a layout of symbol documentation plans categorized by kind

### `clore::generate::find_doc_index_path`

Declaration: `generate/render/symbol.cppm:40`

Definition: `generate/render/symbol.cppm:804`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function performs a direct map lookup on `layout.index_paths`, constructing a temporary `std::string` from the `qualified_name` view to serve as the key. If the entry exists, it returns a pointer to the associated value without copying; otherwise it returns `nullptr`. This avoids unnecessary string duplication and provides a simple existence-check mechanism for the precomputed index paths. The implementation has no external dependencies beyond the `PageDocLayout` data structure and the standard library.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- layout`.index_paths`
- `qualified_name`

#### Usage Patterns

- lookup index path by qualified name
- used during page generation to resolve symbol paths

### `clore::generate::for_each_symbol_doc_group`

Declaration: `generate/render/symbol.cppm:27`

Definition: `generate/render/symbol.cppm:27`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::for_each_symbol_doc_group` implements a simple dispatch loop over the three documentation group containers stored in a `PageDocLayout`. It sequentially invokes the `Visitor` functor on `layout.type_docs`, `layout.variable_docs`, and `layout.function_docs`, in that fixed order. There is no branching or conditional logic; the control flow is strictly linear. The design relies on `PageDocLayout` exposing these three public fields as the canonical partitions of symbol documentation. The visitor is forwarded via a forwarding reference, allowing both lvalue and rvalue callables. No internal state is modified, and no return value is extracted beyond the void return type. The function serves as a central iterator for downstream processing of all symbol documentation groups within a layout.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `layout.type_docs`
- `layout.variable_docs`
- `layout.function_docs`

#### Usage Patterns

- Iterating over all symbol doc groups in a page layout
- Applying a visitor to each group for rendering or analysis

### `clore::generate::normalize_frontmatter_title`

Declaration: `generate/render/symbol.cppm:33`

Definition: `generate/render/symbol.cppm:885`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function first strips inline Markdown formatting from the input `page_title` via `strip_inline_markdown`. If the resulting plain text is non‑empty, it is returned directly. Otherwise, the function falls back to `trim_ascii` applied to the original `page_title`, which removes leading and trailing ASCII whitespace. This fallback ensures that a title consisting entirely of Markdown markup still yields a non‑empty result.

The implementation depends on two internal helper functions: `strip_inline_markdown` for removing lightweight formatting (e.g., bold, italic, inline code) from the title string, and `trim_ascii` for whitespace trimming. Both are presumed to be defined in the same translation unit or namespace. No external dependencies or complex control flow are involved; the logic is a straightforward conditional return based on the intermediate plain‑text result.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `page_title` parameter

#### Usage Patterns

- Used to normalize frontmatter titles before page rendering
- Called during documentation generation to clean titles

### `clore::generate::page_supports_symbol_subpages`

Declaration: `generate/render/symbol.cppm:35`

Definition: `generate/render/symbol.cppm:893`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function evaluates a single condition: it accesses the `page_type` field of the provided `PagePlan` object and returns `true` if that value equals either `PageType::Namespace` or `PageType::Module`; otherwise it returns `false`. The control flow is a direct equality check with no branching or external calls. The implementation depends only on the `PagePlan` type and the `PageType` enumeration; no other analysis or state is consulted.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.page_type`

#### Usage Patterns

- Used in page generation to decide if subpages should be created for a given page plan.

## Internal Structure

The `generate:symbol` module is responsible for planning and rendering individual symbol documentation pages. It imports support from `generate:common` (linking and Markdown utilities), `generate:markdown` (document AST construction), `generate:model` (page plans and symbol analyses), `config`, `extract`, `generate:diagram`, and the standard library. Internally, the module is decomposed into a public interface—comprising `SymbolDocPlan`, `PageDocLayout`, and functions such as `build_page_doc_layout`, `for_each_symbol_doc_group`, and `append_symbol_doc_pages`—and an anonymous-namespace implementation layer that handles plan construction (`build_symbol_doc_plans`, `collect_documentable_children`, `collect_member_symbols`), Markdown rendering (`render_symbol_page`, `render_document_page`, `declaration_snippet`, `implementation_snippet`), and structural helpers (`sanitize_doc_slug`, `join_relative_paths`, `normalize_frontmatter_title`). This layering separates the stable export contract from the internal rendering and planning logic, enabling the pipeline to evolve without affecting external callers.

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)
- [Module generate:common](common.md)
- [Module generate:diagram](diagram.md)
- [Module generate:markdown](markdown.md)
- [Module generate:model](model.md)

