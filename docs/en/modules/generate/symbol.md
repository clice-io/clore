---
title: 'Module generate:symbol'
description: 'The generate:symbol module is responsible for generating individual documentation pages for C++ symbols. It transforms analysis data from the extract and generate:model modules, guided by the config module, into complete rendered pages using the Markdown infrastructure from generate:markdown. Its public interface includes functions to build and inspect page layouts (build_page_doc_layout, page_supports_symbol_subpages), add documentation links (add_symbol_doc_links, find_doc_index_path), append member and type sections (append_type_member_sections, append_symbol_doc_pages), iterate over documentation groups (for_each_symbol_doc_group), and normalize frontmatter titles (normalize_frontmatter_title). The module also exposes the SymbolDocPlan and PageDocLayout structs that define the internal plan and structural layout for a generated page. Overall, this module owns the per-symbol page generation logic, including rendering frontmatter, documentation content, code snippets, and cross-references.'
layout: doc
template: doc
---

# Module `generate:symbol`

## Summary

The `generate:symbol` module is responsible for generating individual documentation pages for C++ symbols. It transforms analysis data from the `extract` and `generate:model` modules, guided by the `config` module, into complete rendered pages using the Markdown infrastructure from `generate:markdown`. Its public interface includes functions to build and inspect page layouts (`build_page_doc_layout`, `page_supports_symbol_subpages`), add documentation links (`add_symbol_doc_links`, `find_doc_index_path`), append member and type sections (`append_type_member_sections`, `append_symbol_doc_pages`), iterate over documentation groups (`for_each_symbol_doc_group`), and normalize frontmatter titles (`normalize_frontmatter_title`). The module also exposes the `SymbolDocPlan` and `PageDocLayout` structs that define the internal plan and structural layout for a generated page. Overall, this module owns the per-symbol page generation logic, including rendering frontmatter, documentation content, code snippets, and cross-references.

## Imports

- [`config`](../config/index.md)
- [`extract`](../extract/index.md)
- [`generate:common`](common.md)
- [`generate:diagram`](diagram.md)
- [`generate:markdown`](markdown.md)
- [`generate:model`](model.md)

## Imported By

- [`generate:page`](page.md)
- [`generate:scheduler`](scheduler.md)

## Types

### `clore::generate::PageDocLayout`

Declaration: `src/generate/render/symbol.cppm:37`

Definition: `src/generate/render/symbol.cppm:37`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::PageDocLayout` aggregates the documentation plans for a single generated page. Internally, it stores three separate vectors of `SymbolDocPlan` objects—`type_docs`, `variable_docs`, and `function_docs`—which are partitioned by symbol kind to simplify rendering loops. An additional `index_paths` map records a mapping from logical index keys to their corresponding output file paths, enabling cross‑page references. This layout guarantees that each symbol belongs to exactly one kind‑specific vector and that every key in `index_paths` is unique; no other invariants are enforced by the struct itself, leaving validation to the generation logic that populates it.

#### Invariants

- No explicit invariants are documented in the evidence.

#### Key Members

- `type_docs`
- `variable_docs`
- `function_docs`
- `index_paths`

#### Usage Patterns

- Populated during documentation generation and used by rendering functions to produce the final page content.

### `clore::generate::SymbolDocPlan`

Declaration: `src/generate/render/symbol.cppm:31`

Definition: `src/generate/render/symbol.cppm:31`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::SymbolDocPlan` is a recursive data structure that serves as an intermediate planning node for documentation generation. Its three members encode a tree of planned documentation entries: `symbol` points to the source symbol metadata, `index_path` stores a uniquely identifying path string for that node, and `children` holds a list of subordinate `SymbolDocPlan` nodes. The invariant is that each node corresponds to exactly one symbol (via the non-owning pointer) and that the tree structure mirrors the nested or hierarchical relationship among the original code symbols. No additional member functions are provided; the struct acts purely as an aggregate for carrying plan state between generation phases. Default member initializers ensure the pointer is null and the container is empty upon default construction.

#### Invariants

- `symbol` may be nullptr for a container/grouper node
- `children` forms a tree with no cycles (expected but not enforced)
- `index_path` is unique among siblings in a well-formed plan

#### Key Members

- `symbol` — the symbol being documented
- `children` — child documentation plans
- `index_path` — deterministic path for indexing output

#### Usage Patterns

- Built by a planner that traverses the symbol hierarchy
- Traversed recursively by renderers to produce documentation pages
- Used as input to index generation via `index_path`

## Functions

### `clore::generate::add_symbol_doc_links`

Declaration: `src/generate/render/symbol.cppm:61`

Definition: `src/generate/render/symbol.cppm:828`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function first queries the layout's index paths via `find_doc_index_path(layout, sym.qualified_name)` to locate the documentation page associated with the given symbol. If a valid `index_path` is found and it is different from `current_page_path`, a single `LinkTarget` is built using `make_link_target` with the current page, the label derived from `doc_label(view)`, and the found path. Finally, `push_link_paragraph` inserts a new paragraph into `nodes` containing the label string and the list of `targets` (which will either contain one link or be empty). The entire operation depends on the `PageDocLayout` structure for path lookups, `extract::SymbolInfo` for the symbol's qualified name, and the helper functions `find_doc_index_path`, `doc_label`, `make_link_target`, and `push_link_paragraph`.

#### Side Effects

- Appends a paragraph node to the `nodes` vector via `push_link_paragraph`.

#### Reads From

- layout
- `current_page_path`
- sym`.qualified_name`
- `doc_label`(view)
- `find_doc_index_path` result

#### Writes To

- The `nodes` parameter (vector)

#### Usage Patterns

- Used during page generation to add cross-reference links for symbol documentation.

### `clore::generate::append_symbol_doc_pages`

Declaration: `src/generate/render/symbol.cppm:78`

Definition: `src/generate/render/symbol.cppm:975`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function iterates over a provided list of `SymbolDocPlan` objects (`doc_plans`). For each `doc_plan`, it calls `render_symbol_page` with the current plan and the shared context (`owner_plan`, `config`, `model`, `outputs`, `analyses`, `links`, `layout`). If this call succeeds, the resulting `GeneratedPage` is appended to the output `pages` vector. Immediately afterward, the function recursively invokes itself with `doc_plan.children` to process any nested symbol documentation plans in a depth‑first manner. If either the rendering or the recursive call returns an error (`std::unexpected`), that error is immediately forwarded up the call chain, causing the entire operation to fail early. The function depends on `render_symbol_page`, the `SymbolDocPlan` hierarchy, and the various configuration and analysis structures (`PagePlan`, `config::TaskConfig`, `extract::ProjectModel`, `SymbolAnalysisStore`, `LinkResolver`, `PageDocLayout`) to produce page content and manage nested documentation without manual stack management.

#### Side Effects

- Mutates the `pages` vector by appending generated `GeneratedPage` objects.

#### Reads From

- `pages` (passed by reference)
- `doc_plans`
- `owner_plan`
- `config`
- `model`
- `outputs`
- `analyses`
- `links`
- `layout`

#### Writes To

- `pages` vector

#### Usage Patterns

- Recursively processes a tree of `SymbolDocPlan` to generate symbol documentation pages.

### `clore::generate::append_type_member_sections`

Declaration: `src/generate/render/symbol.cppm:67`

Definition: `src/generate/render/symbol.cppm:842`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function begins with a guard clause that checks `is_type_kind(sym.kind)` and returns immediately if the symbol is not a type, ensuring that only type symbols proceed. The core logic then makes three sequential calls to `append_member_section`, each with a different heading and a filtered set of child symbols obtained via `collect_member_symbols`. The first call uses a predicate that selects only type children (`is_type_kind`), producing a “Member Types” section; the second selects variable children (`is_variable_kind`) for “Member Variables”; and the third selects function children (`is_function_kind`) for “Member Functions”. All calls pass through the provided parameters (`config`, `model`, `analyses`, `plan`, `links`, `layout`, `current_page_path`, and `level`) unchanged, relying on `append_member_section` to generate the actual section content. The overall control flow is a simple linear sequence without branching beyond the initial guard, and the function’s only dependency on the symbol’s kind is for early exit—the actual filtering is delegated to the lambdas within `collect_member_symbols`.

#### Side Effects

- Mutates the nodes vector by appending `MarkdownNode` sections for member types, variables, and functions.

#### Reads From

- sym
- config
- model
- analyses
- plan
- links
- layout
- `current_page_path`
- level

#### Writes To

- nodes (the `std::vector<clore::generate::MarkdownNode>`& parameter)

#### Usage Patterns

- Called during type documentation page generation to populate member sections.

### `clore::generate::build_page_doc_layout`

Declaration: `src/generate/render/symbol.cppm:55`

Definition: `src/generate/render/symbol.cppm:915`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function begins by short‑circuiting if the page does not support symbol subpages (checked via `page_supports_symbol_subpages(plan)`) or if the computed base directory from `page_directory_of(plan.relative_path)` is empty; in either case an empty `PageDocLayout` is returned. Otherwise, it collects the set of relevant symbols from the `ProjectModel` using `collect_namespace_symbols` for namespace pages or `collect_implementation_symbols` for other page types, then partitions these symbols into three vectors (`type_symbols`, `variable_symbols`, `function_symbols`) based on their kind predicates (`is_type_kind`, `is_variable_kind`, `is_function_kind`). For each category it calls `build_symbol_doc_plans` (passing the plan, model, symbol list, and base directory) and stores the resulting doc plans in the corresponding fields of the layout (`type_docs`, `variable_docs`, `function_docs`). Finally, the function iterates over every symbol‑doc group in the layout via `for_each_symbol_doc_group` and registers each plan entry into the layout’s internal index using `register_symbol_doc_plan`. The main dependencies are the subspace helpers that perform symbol collection, plan construction, and plan registration; the algorithm ensures that only pages that can host symbol documentation are processed, and that the three symbol categories are handled symmetrically.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan` (`PagePlan`) members: `plan.page_type`, `plan.owner_keys`, `plan.relative_path`
- `model` (`extract::ProjectModel`)

#### Usage Patterns

- Called during page generation to organize symbol documentation groups based on a page plan and project model.

### `clore::generate::find_doc_index_path`

Declaration: `src/generate/render/symbol.cppm:58`

Definition: `src/generate/render/symbol.cppm:822`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function looks up a fully qualified symbol name in the provided `PageDocLayout` to retrieve the corresponding documentation index path. It converts the `std::string_view` qualified name to a `std::string` and queries the `index_paths` map (likely a `std::unordered_map` or similar associative container). If the key is found, it returns a pointer to the stored path string; otherwise, it returns `nullptr`. The implementation is a straightforward single‑lookup with a conditional ternary, relying entirely on the `PageDocLayout` data structure and the map’s find operation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- layout`.index_paths`

#### Usage Patterns

- Used to resolve documentation index paths during page generation

### `clore::generate::for_each_symbol_doc_group`

Declaration: `src/generate/render/symbol.cppm:45`

Definition: `src/generate/render/symbol.cppm:45`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function is a templated algorithm that iterates over the three predefined symbol‑document groups stored inside a `PageDocLayout` instance. It invokes the supplied `Visitor` callable exactly three times, passing in order the values of the `type_docs`, `variable_docs`, and `function_docs` fields. The control flow is strictly linear with no branching, recursion, or early exit; each group is visited unconditionally in that fixed sequence. The only dependencies are the `PageDocLayout` structure (which owns those three fields) and the `Visitor` type, which must accept a single argument matching the field’s type. No additional state or side‑effect management is performed by this function itself.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `layout.type_docs`
- `layout.variable_docs`
- `layout.function_docs`

#### Usage Patterns

- Iterate over symbol doc groups in a layout to generate documentation sections
- Apply a transformation or collection function to each doc group

### `clore::generate::normalize_frontmatter_title`

Declaration: `src/generate/render/symbol.cppm:51`

Definition: `src/generate/render/symbol.cppm:903`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Implementation: [Implementation](functions/normalize-frontmatter-title.md)

The function first calls `strip_inline_markdown` on the input `page_title`. If the stripped result is non-empty, it is returned; otherwise, the function returns a string created by `trim_ascii` applied to the original `page_title`. The logic prefers a markdown-stripped version but falls back to a whitespace-trimmed version when stripping yields an empty string (e.g., if the title consisted solely of markdown formatting). Both helper functions (`strip_inline_markdown` and `trim_ascii`) are assumed to be in the same namespace and are the only dependencies.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `page_title`

#### Writes To

- returned `std::string`

#### Usage Patterns

- Used to normalize frontmatter titles for documentation pages

### `clore::generate::page_supports_symbol_subpages`

Declaration: `src/generate/render/symbol.cppm:53`

Definition: `src/generate/render/symbol.cppm:911`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::page_supports_symbol_subpages` serves as a predicate that determines whether a given `PagePlan` qualifies for the generation of symbol subpages. Its implementation performs a single comparison on the `page_type` field of the `PagePlan` instance, returning `true` if the type equals `PageType::Namespace` or `PageType::Module`, and `false` otherwise. This lightweight check relies only on the `PagePlan` type and the `PageType` enumeration; no additional control flow or external dependencies are involved. The function is used elsewhere in the generation pipeline to decide whether to proceed with rendering subpages for symbols within the context defined by the plan.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.page_type` (field of `const PagePlan&` parameter)

#### Usage Patterns

- Used to conditionally generate symbol subpages for namespace and module pages

## Internal Structure

The `generate:symbol` module is the central component responsible for rendering per‑symbol documentation pages (for types, functions, variables, and other entities). It imports from `extract` for symbol analysis and from `generate:common`, `generate:diagram`, `generate:markdown`, and `generate:model` for shared link utilities, diagram generation, Markdown construction, and core data types. Internally, the module is layered into three main phases: (1) **plan building** – functions such as `build_page_doc_layout`, `build_symbol_doc_plans`, and `collect_documentable_children` construct `SymbolDocPlan` and `PageDocLayout` structures that organise which symbols and sections belong on each page; (2) **content construction** – a suite of anonymous‑namespace helpers (`declaration_snippet`, `resolved_snippet`, `append_symbol_context_links`, `append_member_section`, `append_relation_section`, `append_embedded_symbol_content`, etc.) generate Markdown fragments for declaration snippets, member lists, relation diagrams, and fallback descriptions, often using the `generate:diagram` and `generate:markdown` modules; (3) **page assembly** – high‑level entry points like `append_symbol_doc_pages`, `render_symbol_page`, and `render_document_page` combine these fragments into full pages, including frontmatter (via `build_symbol_frontmatter`), slug computation (`sanitize_doc_slug`, `default_symbol_slug`), and relative‑path resolution. This decomposition separates planning from rendering, allowing each piece to be tested and evolved independently while the anonymous‑namespace helpers encapsulate implementation details.

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)
- [Module generate:common](common.md)
- [Module generate:diagram](diagram.md)
- [Module generate:markdown](markdown.md)
- [Module generate:model](model.md)

