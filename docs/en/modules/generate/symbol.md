---
title: 'Module generate:symbol'
description: 'The generate:symbol module is responsible for producing rendered documentation pages for individual symbols (e.g., types, functions, variables) extracted from the codebase. It owns the planning and layout logic that determines how each symbol’s documentation is structured, including its frontmatter, code snippets (declaration, implementation, resolved), member sections, relation links, and fallback content. Public interface elements include the data structures SymbolDocPlan and PageDocLayout, which capture per-symbol documentation plans and overall page layout groupings, respectively. Key public functions such as build_page_doc_layout, normalize_frontmatter_title, for_each_symbol_doc_group, add_symbol_doc_links, and append_symbol_doc_pages expose the module’s ability to construct, query, and render symbol documentation pages and their sub‑pages.'
layout: doc
template: doc
---

# Module `generate:symbol`

## Summary

The `generate:symbol` module is responsible for producing rendered documentation pages for individual symbols (e.g., types, functions, variables) extracted from the codebase. It owns the planning and layout logic that determines how each symbol’s documentation is structured, including its frontmatter, code snippets (declaration, implementation, resolved), member sections, relation links, and fallback content. Public interface elements include the data structures `SymbolDocPlan` and `PageDocLayout`, which capture per-symbol documentation plans and overall page layout groupings, respectively. Key public functions such as `build_page_doc_layout`, `normalize_frontmatter_title`, `for_each_symbol_doc_group`, `add_symbol_doc_links`, and `append_symbol_doc_pages` expose the module’s ability to construct, query, and render symbol documentation pages and their sub‑pages.

The module collaborates closely with `generate:model` for core data types, `generate:markdown` for Markdown node construction, and `generate:diagram` for visual diagram generation. Its internal implementation handles tasks like collecting documentable children, building documentation plans, rendering symbol pages with context links and snippet sections, and sanitizing slugs and titles. The module also provides predicates like `page_supports_symbol_subpages` to control hierarchical documentation generation and utilities such as `find_doc_index_path` for navigating within the layout.

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

The internal structure of `clore::generate::PageDocLayout` consists of four member fields that collectively partition the documentation content for a single rendered page. Three `std::vector<SymbolDocPlan>` members — `type_docs`, `variable_docs`, and `function_docs` — store symbol documentation plans grouped by kind. The fourth member, `index_paths`, is a `std::unordered_map<std::string, std::string>` that likely maps logical names or identifiers to file paths for cross‑referencing or link generation. An important invariant is that the categorization of plans into the three vectors is the responsibility of the caller; the struct itself performs no validation or enforcement of category correctness. Additionally, consistency between the entries in these vectors and the keys in `index_paths` is not guaranteed by the struct’s own methods. The design thus treats `PageDocLayout` as a passive data aggregate, relying on external orchestration (e.g., the generation pipeline) to maintain coherence among the four collections.

#### Invariants

- `type_docs`, `variable_docs`, and `function_docs` each contain only `SymbolDocPlan` objects relevant to their category
- All keys in `index_paths` are unique
- The struct is intended to be fully populated before use

#### Key Members

- `index_paths`
- `type_docs`
- `variable_docs`
- `function_docs`

#### Usage Patterns

- Populated by a layout builder during documentation generation
- Consumed by a renderer to produce the final page output
- Used to categorize symbol documentation by type, variable, and function

### `clore::generate::SymbolDocPlan`

Declaration: `generate/render/symbol.cppm:13`

Definition: `generate/render/symbol.cppm:13`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::SymbolDocPlan` implements a tree node for organizing documentation generation plans. Its fields include `symbol`, a pointer to a `extract::SymbolInfo` object defaulting to `nullptr`, `index_path`, a `std::string` that stores the hierarchical path used for indexing, and `children`, a `std::vector<SymbolDocPlan>` representing child nodes. The `symbol` pointer being `nullptr` signifies an intermediate or root node without a direct symbol association. The `children` vector enables recursive nesting, with each child plan potentially having its own subtree. Invariants include that the tree structure mirrors the symbol hierarchy extracted from source code. The `index_path` field typically corresponds to the full dotted or qualified name of the symbol (or path for intermediate nodes) and is expected to be unique within the plan for correct output generation.

#### Invariants

- No explicit invariants beyond default initialization
- `symbol` may be null if not set
- `index_path` may be empty
- `children` vector may be empty

#### Key Members

- `symbol` member
- `index_path` member
- `children` member

#### Usage Patterns

- Used by documentation generation infrastructure to represent a hierarchical plan for symbol documentation
- Other code likely populates instances of `SymbolDocPlan` by assigning the `symbol`, `index_path`, and `children` fields
- The recursive `children` vector allows building a tree structure of nested symbol documentation plans

## Variables

### `clore::generate::add_symbol_doc_links`

Declaration: `generate/render/symbol.cppm:43`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The variable `clore::generate::add_symbol_doc_links` is a callable object (likely a function or lambda) that participates in generating symbol documentation pages. It is referenced inside `render_symbol_page` to insert hyperlinks to related symbols or documentation sections, contributing to the page's link paragraphs.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- called or referenced inside `render_symbol_page` to add symbol documentation links

### `clore::generate::append_symbol_doc_pages`

Declaration: `generate/render/symbol.cppm:60`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

No evidence of mutation or reassignment after initialization is present. The variable is presumed to be consumed as part of the documentation page assembly pipeline, either as a function object or a function pointer, given its appearance among other rendering-related variables.

#### Mutation

No mutation is evident from the extracted code.

### `clore::generate::append_type_member_sections`

Declaration: `generate/render/symbol.cppm:49`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Based solely on the provided evidence, no assignment, mutation, or consumption of this variable is shown. The variable name suggests involvement in appending type member sections during documentation generation, but this is speculative. The evidence includes only its declaration and a partial source snippet.

#### Mutation

No mutation is evident from the extracted code.

## Functions

### `clore::generate::build_page_doc_layout`

Declaration: `generate/render/symbol.cppm:37`

Definition: `generate/render/symbol.cppm:897`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function begins by checking `page_supports_symbol_subpages`; if the plan does not permit subpages, it returns a default‑constructed `PageDocLayout`. It then derives a base directory via `page_directory_of` and, if empty, returns the empty layout. Symbols are collected according to the page type: for namespace pages, `collect_namespace_symbols` is used; otherwise `collect_implementation_symbols` is called with an unconditional predicate. The resulting set of `extract::SymbolInfo` pointers is partitioned into three vectors (`type_symbols`, `variable_symbols`, `function_symbols`) based on their kind. For each category, `build_symbol_doc_plans` is invoked with the `model`, `plan`, the relevant symbol list, and the computed `base_dir`, producing `SymbolDocPlan` sequences that are stored respectively in `layout.type_docs`, `layout.variable_docs`, and `layout.function_docs`. Finally, `for_each_symbol_doc_group` iterates over all groups in the layout and registers each plan into the layout via `register_symbol_doc_plan`, which populates the index‑path and other cross‑reference structures of the `PageDocLayout`.

#### Side Effects

- Allocates memory for symbol doc plans via `build_symbol_doc_plans`
- Populates the `type_docs`, `variable_docs`, and `function_docs` vectors of the `PageDocLayout`
- Registers each `SymbolDocPlan` into the `PageDocLayout` via `register_symbol_doc_plan`

#### Reads From

- `plan` (const `PagePlan&`) — reads `plan.relative_path`, `plan.page_type`, `plan.owner_keys`
- `model` (const `extract::ProjectModel&`) — provides symbol info
- Collected `SymbolInfo` objects via `collect_namespace_symbols` or `collect_implementation_symbols`
- `page_directory_of` — computes base directory from path
- `page_supports_symbol_subpages` — queries page capability

#### Writes To

- The returned `PageDocLayout` object (local variable `layout`)
- `layout.type_docs`, `layout.variable_docs`, `layout.function_docs` — populated by `build_symbol_doc_plans`
- Internal registration state of `layout` via `register_symbol_doc_plan`

#### Usage Patterns

- Called during page documentation generation to create subpage layouts for types, variables, and functions
- Used as part of building the overall page structure for module, namespace, or implementation pages

### `clore::generate::find_doc_index_path`

Declaration: `generate/render/symbol.cppm:40`

Definition: `generate/render/symbol.cppm:804`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function accepts a reference to `PageDocLayout` and a `std::string_view` qualified name, then searches the `index_paths` map for an entry with that name. It constructs a temporary `std::string` from the view to perform the lookup, which may incur a small allocation but simplifies the comparison against the map’s key type. The function returns a pointer to the associated path string if found, or `nullptr` if no match exists.

Internally, the control flow is a single map lookup via `layout.index_paths.find(...)`. The result is compared against `layout.index_paths.end()`, and based on the check either a pointer to the found element’s second member (the path) is returned, or a null pointer. The key dependency is the `PageDocLayout` type, which must provide an `index_paths` member that is a map from `std::string` to some string type. The function avoids copying the path by returning a raw pointer to the existing storage.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `layout` (specifically `layout.index_paths` member)
- `qualified_name` parameter

#### Usage Patterns

- Lookup of index page paths during documentation generation
- Retrieving a path for a symbol after constructing a `PageDocLayout`

### `clore::generate::for_each_symbol_doc_group`

Declaration: `generate/render/symbol.cppm:27`

Definition: `generate/render/symbol.cppm:27`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::for_each_symbol_doc_group` is a simple dispatch that, given a `PageDocLayout` and a `Visitor` callable, applies the visitor to each of the three categorized symbol documentation collections stored in the layout. It sequentially invokes `visitor(layout.type_docs)`, `visitor(layout.variable_docs)`, and `visitor(layout.function_docs)`. This provides a uniform way to iterate over all top-level symbol-document groups—type docs, variable docs, and function docs—in a predetermined order. The function depends solely on the `PageDocLayout` structure exposing these three fields and on the visitor being callable with the type of each field. No conditional branching, mutation of layout state, or additional control flow is performed.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `PageDocLayout` parameter `layout`
- `layout.type_docs`
- `layout.variable_docs`
- `layout.function_docs`
- the `Visitor` functor parameter `visitor`

#### Usage Patterns

- used to apply a visitor to each of the three symbol documentation groups (type, variable, function) from a `PageDocLayout`

### `clore::generate::normalize_frontmatter_title`

Declaration: `generate/render/symbol.cppm:33`

Definition: `generate/render/symbol.cppm:885`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function first strips inline Markdown formatting from the input `page_title` via `strip_inline_markdown`. If the resulting plain text is non‑empty, it is returned directly as the normalized title. Otherwise, the function falls back to trimming ASCII whitespace from the original `page_title` using `trim_ascii` and returns that. The control flow is a simple conditional: the Markdown‑free variant is preferred; the trimmed original serves as a safe default when stripping yields an empty string.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `page_title` (the input `std::string_view`)

#### Usage Patterns

- used to normalize titles for frontmatter in documentation page generation
- ensures a clean, non-empty title for metadata rendering

### `clore::generate::page_supports_symbol_subpages`

Declaration: `generate/render/symbol.cppm:35`

Definition: `generate/render/symbol.cppm:893`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::page_supports_symbol_subpages` determines whether a given page type supports subpages for symbols. It accepts a constant reference to a `PagePlan` object and returns a boolean. Internally, it performs a single comparison against the `plan.page_type` member: if the value equals `PageType::Namespace` or `PageType::Module`, the result is `true`; otherwise `false`. This produces a straightforward, branch-free check with no additional control flow. The function depends only on the `PagePlan` type and the `PageType` enumeration, making it a lightweight predicate that is likely called when evaluating page layout or navigation generation. There are no side effects or external dependencies beyond the input parameter.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan` (the `PagePlan` parameter)
- `plan.page_type`

#### Usage Patterns

- Called in page generation logic to decide whether to generate symbol subpages for a given page plan.
- Used by functions such as `clore::generate::build_page_root` and `clore::generate::build_namespace_page_root` to conditionally include subpage structures.

## Internal Structure

The `generate:symbol` module is responsible for rendering documentation pages for individual code symbols. It decomposes the task into two layers: a planning layer that builds `SymbolDocPlan` and `PageDocLayout` structures to define which symbols are documented and how pages are organized, and a rendering layer that transforms those plans into Markdown content. The module imports extraction results, configuration, and shared generation utilities (`generate:common`, `generate:model`, `generate:markdown`, `generate:diagram`), placing it at the top of the generation pipeline where symbol metadata is consumed and user‑facing pages are produced.

Internally, the module is implemented as a single module unit using an anonymous namespace for nearly all helper functions. These helpers handle specific rendering concerns such as building symbol frontmatter, creating declaration or implementation snippets, collecting related symbols, and appending member or relation sections. Public entry points like `build_page_doc_layout`, `for_each_symbol_doc_group`, and `append_symbol_doc_pages` expose a controlled interface for other parts of the generator to request symbol page plans, iterate over groups, or trigger full page output. This structure keeps the rendering logic cohesive while providing clean boundaries for integration with broader documentation workflows.

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)
- [Module generate:common](common.md)
- [Module generate:diagram](diagram.md)
- [Module generate:markdown](markdown.md)
- [Module generate:model](model.md)

