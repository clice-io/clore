---
title: 'Module generate:common'
description: 'The generate:common module provides shared utilities and building blocks used throughout the documentation generation pipeline. Its responsibilities include constructing hyperlink targets (LinkTarget, make_link_target, make_relative_link_target, make_source_link_target), building cross-reference paragraphs and symbol link lists (push_link_paragraph, push_location_paragraph, push_optional_link_paragraph, build_symbol_link_list, build_related_page_targets), and formatting analysis sections (add_symbol_analysis_sections, add_symbol_analysis_detail_sections). It also offers helpers for string manipulation (strip_inline_markdown, trim_ascii, namespace_of, short_name_of), enumeration of symbols by namespace or implementation (collect_namespace_symbols, collect_implementation_symbols), and generation of prompt-focused sections (build_prompt_section, add_prompt_output). The module depends on the config, extract, generate:markdown, and generate:model modules, and its public interface includes the LinkTarget struct, the SymbolDocView enum, and numerous free functions that orchestrate the creation of structured, navigable documentation content.'
layout: doc
template: doc
---

# Module `generate:common`

## Summary

The `generate:common` module provides shared utilities and building blocks used throughout the documentation generation pipeline. Its responsibilities include constructing hyperlink targets (`LinkTarget`, `make_link_target`, `make_relative_link_target`, `make_source_link_target`), building cross-reference paragraphs and symbol link lists (`push_link_paragraph`, `push_location_paragraph`, `push_optional_link_paragraph`, `build_symbol_link_list`, `build_related_page_targets`), and formatting analysis sections (`add_symbol_analysis_sections`, `add_symbol_analysis_detail_sections`). It also offers helpers for string manipulation (`strip_inline_markdown`, `trim_ascii`, `namespace_of`, `short_name_of`), enumeration of symbols by namespace or implementation (`collect_namespace_symbols`, `collect_implementation_symbols`), and generation of prompt-focused sections (`build_prompt_section`, `add_prompt_output`). The module depends on the `config`, `extract`, `generate:markdown`, and `generate:model` modules, and its public interface includes the `LinkTarget` struct, the `SymbolDocView` enum, and numerous free functions that orchestrate the creation of structured, navigable documentation content.

## Imports

- [`config`](../config/index.md)
- [`extract`](../extract/index.md)
- [`generate:markdown`](markdown.md)
- [`generate:model`](model.md)

## Imported By

- [`generate:page`](page.md)
- [`generate:symbol`](symbol.md)

## Types

### `clore::generate::LinkTarget`

Declaration: `src/generate/render/common.cppm:22`

Definition: `src/generate/render/common.cppm:22`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::LinkTarget` is a simple aggregate that holds the three pieces of data required to describe a hyperlink: the visible `label`, the URL or document reference `target`, and a boolean `code_style` flag that defaults to `false`. Its internal structure imposes no invariants beyond the usual validity of the string members—any empty label or target is permitted, though likely meaningless in practice. The implementation relies on default member initialisation and aggregate initialization; no custom constructors, assignment `operator`s, or member functions are defined. Each instance is therefore a plain data bundle that can be constructed inline or via designated initialisers, and `code_style` silently defaults to `false` when omitted, ensuring backward compatibility and concise usage.

#### Invariants

- No invariants beyond the default value of `code_style` being `false`.

#### Key Members

- `label`
- `target`
- `code_style`

#### Usage Patterns

- Used as a data holder for constructing links in generated documentation or reports.

### `clore::generate::SymbolDocView`

Declaration: `src/generate/render/common.cppm:28`

Definition: `src/generate/render/common.cppm:28`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The internal structure of `clore::generate::SymbolDocView` is a scoped enumeration backed by `std::uint8_t`, providing three distinct modes that control which portion of a symbol’s documentation is rendered: `Declaration`, `Implementation`, and `Details`. These enumerators are implicitly assigned the values 0, 1, and 2 respectively, forming an ordered set used to select a specific view during doc generation. No additional flags or sentinel values are present, so the enum is exhaustive over its three cases. This compact representation minimizes memory overhead while enabling simple switch‑based dispatch in rendering logic.

#### Invariants

- The three enumerators are the only valid values
- Each value corresponds to a mutually exclusive view
- Underlying type is `std::uint8_t`

#### Key Members

- `Declaration`
- `Implementation`
- `Details`

#### Usage Patterns

- Used to select which portion of a symbol’s documentation to generate or display
- Likely passed as a parameter to rendering functions

#### Member Variables

##### `clore::generate::SymbolDocView::Declaration`

Declaration: `src/generate/render/common.cppm:29`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Declaration
```

##### `clore::generate::SymbolDocView::Details`

Declaration: `src/generate/render/common.cppm:31`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Details
```

##### `clore::generate::SymbolDocView::Implementation`

Declaration: `src/generate/render/common.cppm:30`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Implementation
```

## Functions

### `clore::generate::add_prompt_output`

Declaration: `src/generate/render/common.cppm:153`

Definition: `src/generate/render/common.cppm:153`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::add_prompt_output` accepts a mutable reference to a node list and an optional pointer to a pre-rendered output string. Its internal control flow first checks whether the pointer is non-null and then applies `clore::generate::trim_ascii` to the pointed string to test for non‑empty content after stripping ASCII whitespace. Only when both conditions hold does it append a raw Markdown node constructed from the output string. This gate ensures that no empty or whitespace‑only prompt fragments are inserted into the document, relying solely on the utility `trim_ascii` to perform the whitespace check.

#### Side Effects

- Mutates the provided vector of `MarkdownNode` by appending a new node.

#### Reads From

- `output` pointer (string content)
- `trim_ascii` function
- `make_raw_markdown` function

#### Writes To

- `nodes` vector (via `push_back`)

#### Usage Patterns

- Used to add a prompt's output to a list of markdown nodes, typically when building a page's markdown content.

### `clore::generate::add_symbol_analysis_detail_sections`

Declaration: `src/generate/render/common.cppm:181`

Definition: `src/generate/render/common.cppm:196`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::add_symbol_analysis_detail_sections` appends detail subsections to the output `nodes` vector based on the extracted symbol analysis stored in `analyses`. It first computes a target key via `make_symbol_target_key` from `sym`, then attempts to locate a matching analysis entry: `find_function_analysis`, `find_type_analysis`, or `find_variable_analysis`. For each supported symbol category, the function conditionally builds list sections using the lambda `make_list_node`, which internally calls `build_string_list` and `build_list_section`, or falls back to a descriptive paragraph (for side‑effects or mutations) when the analysis list is empty. The `plan.page_type` check suppresses certain sections (e.g., side effects, reads/writes, mutation sources) for namespace‑level pages. Finally, usage patterns are appended for all categories. The control flow is a linear dispatch with early returns after processing a found analysis, ensuring that only one category’s detail sections are added per invocation. Dependencies include `MarkdownNode`, `SymbolAnalysisStore`, `PagePlan`, and several local helpers for string list construction and paragraph creation.

#### Side Effects

- Appends markdown nodes to the provided `nodes` vector.

#### Reads From

- `analyses` (const `SymbolAnalysisStore&`)
- `plan` (const `PagePlan&`)
- `sym` (const `extract::SymbolInfo&`)
- `nodes` (`std::vector<MarkdownNode>`&)

#### Writes To

- `nodes` (`std::vector<MarkdownNode>`&) by appending new `MarkdownNode` objects.

#### Usage Patterns

- Called from higher-level page generation functions to populate detail sections.
- Typically invoked per symbol during rendering of analysis pages.

### `clore::generate::add_symbol_analysis_sections`

Declaration: `src/generate/render/common.cppm:187`

Definition: `src/generate/render/common.cppm:187`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::add_symbol_analysis_sections` acts as a two‑step dispatcher that populates the given `nodes` vector with analysis content for a single symbol. It first calls `clore::generate::add_prompt_output` using the string pointer returned by `clore::generate::symbol_analysis_markdown_for` (which queries the `analyses` store with `plan` and `sym`), thereby inserting any pre‑computed prompt output block. Immediately afterward, it invokes `clore::generate::add_symbol_analysis_detail_sections` with the same `nodes`, `analyses`, `plan`, `sym`, and `level` arguments, which recursively adds more granular subsections (declaration, implementation, related pages, etc.). The control flow is purely sequential, relying entirely on those two helper functions and the input data structures `clore::generate::MarkdownNode`, `clore::generate::SymbolAnalysisStore`, `clore::generate::PagePlan`, and `clore::extract::SymbolInfo`.

#### Side Effects

- appends Markdown nodes to the `nodes` vector

#### Reads From

- `analyses` (the `SymbolAnalysisStore`)
- `plan` (the `PagePlan`)
- `sym` (the `SymbolInfo`)
- `level` (heading depth)
- return value of `symbol_analysis_markdown_for`

#### Writes To

- `nodes` (the vector of `MarkdownNode`)

#### Usage Patterns

- called during symbol page generation to include analysis sections
- used when building documentation for a symbol's analysis

### `clore::generate::build_list_section`

Declaration: `src/generate/render/common.cppm:144`

Definition: `src/generate/render/common.cppm:144`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function constructs a `SemanticSectionPtr` by calling `make_section` with `SemanticKind::Section`, an empty initializer list, the given `heading`, and `level`. After creating the section, it checks whether the `BulletList` object `list` contains any items; if `list.items` is non‑empty, it moves the whole `list` into a `MarkdownNode` and appends that node to the section’s `children` vector. The resulting section pointer is returned. Internally, the algorithm is a straightforward two‑step pipeline: build a bare section container, then conditionally attach the rendered list content. All necessary types (`SemanticSectionPtr`, `SemanticKind`, `MarkdownNode`, `BulletList`) and the helper `make_section` are used directly, with no further branching or iteration.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `heading` (`std::string`)
- parameter `level` (`std::uint8_t`)
- parameter `list` (`BulletList`), specifically `list.items`

#### Usage Patterns

- Used to generate a section with a bullet list for documentation pages (e.g., symbol lists, option lists).

### `clore::generate::build_prompt_section`

Declaration: `src/generate/render/common.cppm:135`

Definition: `src/generate/render/common.cppm:135`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `build_prompt_section` constructs a section node by first calling `make_section` with `SemanticKind::Section` and the given `heading` and `level`, producing a `SemanticSectionPtr`. It then checks whether the `output` pointer is non‑null and, after trimming whitespace with `trim_ascii`, the resulting view is non‑empty. Only when both conditions hold does it append a raw markdown child node via `make_raw_markdown` to the section’s `children` list. The returned section always contains the heading structure; the markdown content is added conditionally, ensuring that an empty or whitespace‑only output string does not produce a spurious empty paragraph.

#### Side Effects

- Allocates a `SemanticSection` object and transfers ownership to the caller
- Modifies the children vector of the created section

#### Reads From

- parameter `heading`
- parameter `level`
- parameter `output` (dereferenced if non-null)
- `trim_ascii` function call result

#### Writes To

- heap-allocated `SemanticSection` object
- children vector of that section

#### Usage Patterns

- Called to construct a section with optional raw markdown content within a prompt page

### `clore::generate::build_related_page_targets`

Declaration: `src/generate/render/common.cppm:515`

Definition: `src/generate/render/common.cppm:515`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::build_related_page_targets` iterates over each entry in `plan.linked_pages`. For each entry, it parses an optional prefix (before the first colon) and the entity name (everything after the colon). If the prefix is `"module"` or `"namespace"`, the function resolves the target path using `links.resolve_module` or `links.resolve_namespace` respectively; otherwise it falls back to `links.resolve`. If the resolved `target_path` is null or has already been recorded in the `seen` set, the entry is skipped. Otherwise, the function retrieves the page title via `links.resolve_page_title`, derives a label by stripping inline Markdown with `strip_inline_markdown`, and constructs a `LinkTarget` via `make_link_target`. All collected `LinkTarget` objects are returned as a vector.

The algorithm relies on `LinkResolver` to map entity names to file paths and page titles, and uses `make_link_target` to produce the final target struct. Deduplication via `seen` prevents duplicate entries, and the prefix-based resolution allows explicit routing to module or namespace pages.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.linked_pages`
- `links` (`LinkResolver` instance)
- `current_page_path`
- `links.resolve_module`
- `links.resolve_namespace`
- `links.resolve`
- `links.resolve_page_title`
- `strip_inline_markdown`
- `make_link_target`

#### Usage Patterns

- Used during page generation to collect cross-reference link targets for related pages
- Called when building navigation or 'related pages' sections in documentation output

### `clore::generate::build_string_list`

Declaration: `src/generate/render/common.cppm:159`

Definition: `src/generate/render/common.cppm:159`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function iterates over each string in its input `items` parameter. For each item, it applies `clore::generate::trim_ascii` to strip leading/trailing whitespace; if the trimmed result is empty, the item is skipped. Otherwise, it creates a `ListItem`, populates its `fragments` member by calling `code_spanned_fragments` on the original untrimmed `item_text`, and appends the item to the internal `list` of type `BulletList`. The function returns the fully populated `BulletList`. No external dependencies beyond `trim_ascii` and `code_spanned_fragments` are required; the control flow is a simple loop with an early‑continue guard for blank entries.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- items

#### Writes To

- `BulletList` return value

#### Usage Patterns

- building bullet lists from string collections
- generating markdown lists from code-annotated text

### `clore::generate::build_symbol_link_list`

Declaration: `src/generate/render/common.cppm:371`

Definition: `src/generate/render/common.cppm:371`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function iterates over every symbol pointer in the `symbols` collection. For each symbol, it builds a `ListItem` by first prepending a text fragment containing the kind name (obtained from `extract::symbol_kind_name`) followed by a space. It then determines the display label: if `use_full_name` is true, the label is the symbol’s `qualified_name`; otherwise it calls `short_name_of` to produce a shortened form. If `label` is empty, it falls back to the full `qualified_name`. Using `links.resolve(sym->qualified_name)`, the function attempts to look up a target path; if a target is found, it constructs a hyperlink via `make_link` with a relative path computed by `make_relative_link_target` relative to `current_page_path`. When no target exists, the label is rendered as inline code via `make_code`. Each completed item is appended to the internal `list` (of type `BulletList`), which is returned after processing all symbols. The algorithm depends on the `LinkResolver` for resolving symbol names to paths and on helper functions from the `extract` and local `generate` namespaces for name formatting and markup creation.

#### Side Effects

- Allocates and populates a `BulletList` object and its `ListItem` elements

#### Reads From

- `symbols` parameter (vector of pointers to `extract::SymbolInfo`)
- `current_page_path` parameter
- `links` parameter (`LinkResolver`)
- `use_full_name` parameter
- Each symbol's `kind` and `qualified_name` members
- `links.resolve()` results
- `short_name_of()` function output
- `make_relative_link_target()` function output

#### Writes To

- Local `BulletList` `list`
- Local `ListItem` `item`
- `list.items` vector

#### Usage Patterns

- Called during page rendering to build a list of links to related symbols
- Used in documentation generation pipelines to produce navigation or reference lists

### `clore::generate::build_symbol_source_locations`

Declaration: `src/generate/render/common.cppm:423`

Definition: `src/generate/render/common.cppm:423`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function first creates an empty vector of `MarkdownNode` to accumulate output. It then interrogates the incoming `extract::SymbolInfo` object: if `sym.declaration_location.is_known()` evaluates to true, it calls `push_location_paragraph` with the label `"Declaration: "` and a `LinkTarget` produced by `make_source_link_target` from the declaration location, configuration, link resolver, and current page path. After that, it performs a similar check for the definition location – only if `sym.definition_location.has_value()` returns true and the contained location’s `is_known()` also returns true, it pushes a `"Definition: "` paragraph with the corresponding source link target. No further nodes are added if either location is unknown, and the function returns the resulting vector.

Control flow is strictly linear with two independent conditionals; there are no loops or recursive calls. The function depends on `push_location_paragraph` and `make_source_link_target` (both from the same module) and on the `extract::SymbolInfo` data structure, the `config::TaskConfig`, and the `LinkResolver` type. The returned nodes are intended for inclusion in a larger document section, and the function’s logic mirrors the typical pattern of rendering location metadata in a documentation page.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `sym.declaration_location`
- `sym.definition_location`
- `config`
- `links`
- `current_page_path`

#### Usage Patterns

- used in page rendering to generate source location link sections

### `clore::generate::collect_implementation_symbols`

Declaration: `src/generate/render/common.cppm:325`

Definition: `src/generate/render/common.cppm:325`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::collect_implementation_symbols` collects a sorted, deduplicated vector of pointers to `extract::SymbolInfo` objects from the project model, guided by a `PagePlan` and filtered by a user-supplied `Predicate`. It first iterates over each key in `plan.owner_keys`. For a module page (`plan.page_type == PageType::Module`), it calls `extract::find_modules_by_name` to locate the module and then iterates over `module->symbols`. For all other page types, it looks up the file directly in `model.files` using the key and processes the file’s `symbols` list. In both paths, each `extract::SymbolID` is inserted into a `std::unordered_set<extract::SymbolID>` named `seen` to prevent duplicates; if the insertion succeeds, the symbol is resolved via `extract::lookup_symbol` and only included if `is_page_level_symbol` returns `true` and the `predicate` accepts the symbol. After the outer loop, the collected symbols are sorted by `qualified_name` using `std::sort` before being returned.

The algorithm has two main stages: collecting qualified symbols with memoisation, then sorting. It depends on the structure of `extract::ProjectModel` (specifically `files` and modules), the helper `extract::lookup_symbol` for resolution, and the `extract::find_modules_by_name` utility for module-typed nodes. The function itself is templated on `Predicate`, allowing the caller to apply arbitrary filtering beyond the page-level and deduplication logic. The final sort ensures deterministic ordering for downstream usage.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.owner_keys`
- `plan.page_type`
- `model` (passed to `extract::find_modules_by_name`, `extract::lookup_symbol`, and `is_page_level_symbol`)
- `predicate` (called for each symbol)

#### Usage Patterns

- called to gather symbols for implementation documentation pages
- used with a plan and model to filter symbols that are page-level and match a predicate

### `clore::generate::collect_namespace_symbols`

Declaration: `src/generate/render/common.cppm:300`

Definition: `src/generate/render/common.cppm:300`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function begins by attempting to locate the given `namespace_name` within the `model.namespaces` map; if the namespace is not present, an empty vector is returned immediately. Otherwise, it iterates over all symbol identifiers stored in that namespace entry. For each identifier, `extract::lookup_symbol` resolves the pointer, and only symbols that satisfy both `is_page_level_symbol` (presumably a helper that filters for top‑level documentation symbols) and the caller‑supplied `predicate` are appended to a result vector. After the collection loop, the vector is sorted by `qualified_name` to produce a deterministic ordering, and the sorted vector is returned. Key dependencies include the `extract::ProjectModel` data structure, the symbol resolution function `lookup_symbol`, and the custom `is_page_level_symbol` filter; the sorting step relies on `std::sort` with a lambda comparing `qualified_name` members.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `extract::ProjectModel` `model`
- `std::string_view namespace_name`
- `Predicate&& predicate`
- `extract::lookup_symbol(model, sym_id)`
- `is_page_level_symbol(model, *sym)`
- `ns_it->second.symbols`
- `lhs->qualified_name` and `rhs->qualified_name`

#### Writes To

- local `std::vector<const extract::SymbolInfo*> symbols`

#### Usage Patterns

- called from namespace page builders to gather symbols
- used with a predicate to filter symbols for analysis or rendering
- provides sorted list of namespace symbols for further processing

### `clore::generate::doc_label`

Declaration: `src/generate/render/common.cppm:290`

Definition: `src/generate/render/common.cppm:290`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::doc_label` maps a `SymbolDocView` enumerator to a human-readable section label. It uses a `switch` statement to return a `std::string_view`: for `SymbolDocView::Declaration` it returns `"Declaration"`, for `SymbolDocView::Implementation` it returns `"Implementation"`, and for `SymbolDocView::Details` it returns `"Details"`. A default fallback also yields `"Details"`. The implementation has no external dependencies beyond the `SymbolDocView` enum and serves as a simple lookup for rendering the appropriate heading or tab identifier in generated documentation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `view` parameter

#### Usage Patterns

- used to get display labels for documentation sections
- used in rendering markdown for symbol documentation
- provides human-readable string for each `SymbolDocView` value

### `clore::generate::find_declaration_page`

Declaration: `src/generate/render/common.cppm:484`

Definition: `src/generate/render/common.cppm:484`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::find_declaration_page` attempts to locate the page that declares a given symbol, returning an optional `LinkTarget`. It first queries the `LinkResolver` (`links`) for a page corresponding to the symbol’s qualified name. If a target path exists and differs from the current page path, it constructs a label—either `"Declaration"` (if the symbol has no enclosing namespace) or `"Namespace "` followed by the namespace name—and returns a `LinkTarget` via `make_link_target`. If that lookup fails, the function extracts or computes the namespace from the symbol, and if non‑empty, tries to resolve that namespace name via `links.resolve_namespace`. On success, it again builds a named link target with `"Namespace "` prefix. If both lookups return nothing, the function returns an empty `std::nullopt`. The logic thus prioritises a direct declaration page over a broader namespace landing page, and never duplicates the current page.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- sym`.qualified_name`
- sym`.enclosing_namespace`
- links`.resolve`(const `std::string`&)
- links`.resolve_namespace`(const `std::string`&)
- `current_page_path`
- `namespace_of`(`std::string_view`)

#### Usage Patterns

- Called during page rendering to add a link to the declaration page of a symbol

### `clore::generate::find_implementation_pages`

Declaration: `src/generate/render/common.cppm:444`

Definition: `src/generate/render/common.cppm:444`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::find_implementation_pages` locates documentation pages corresponding to the implementation of a given symbol. It uses two candidate file paths: first the symbol's definition location, then its declaration location. For each file path, a lambda `try_add` checks whether the file belongs to a module by calling `extract::find_module_by_source`. If it does, the function resolves the module's documentation page via `links.resolve_module` and creates a `LinkTarget` with a label of the form "Module <name>" and `code_style` set to true. If the file is not part of a module, it falls back to `links.resolve` on the raw file path and generates a label via `make_source_relative`. A `seen` set of target paths prevents duplicate entries. The resulting vector of `LinkTarget` values is returned, representing all unique implementation pages for the symbol's source locations.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `sym`
- `model`
- `links`
- `current_page_path`
- `project_root`

#### Usage Patterns

- called during document generation to collect implementation page links for symbols

### `clore::generate::find_module_for_file`

Declaration: `src/generate/render/common.cppm:507`

Definition: `src/generate/render/common.cppm:507`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::find_module_for_file` is a thin wrapper around the extraction subsystem. It accepts a reference to the `extract::ProjectModel` and a `std::string_view` representing the source file path. The function delegates directly to `extract::find_module_by_source`, passing the `model` and `file_path`. If that call returns a non-null pointer to an `extract::Module`, the function returns its `name` member as an `std::optional<std::string>`; otherwise it returns `std::nullopt`.

The control flow is minimal—a single conditional that tests the result of the dependency. No iteration, recursion, or complex branching occurs. The primary dependency is on the `extract` namespace, specifically `extract::find_module_by_source` and the `extract::ProjectModel` type. This design keeps the generation layer decoupled from the extraction details while providing a straightforward mapping from a source file to its containing module name for use in page generation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const extract::ProjectModel& model`
- `std::string_view file_path`

#### Usage Patterns

- Used to determine the module for a file during page generation or link resolution

### `clore::generate::make_link_target`

Declaration: `src/generate/render/common.cppm:92`

Definition: `src/generate/render/common.cppm:92`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::make_link_target` constructs a `LinkTarget` object from its four parameters. It begins by moving the `label` string into the `.label` member of the returned struct. The `.target` member is obtained by calling `clore::generate::make_relative_link_target` with `current_page_path` and `target_page_path`, which computes the relative file path between the two pages. Finally, the `code_style` parameter is assigned directly to the `.code_style` field. There is no conditional branching or looping; the function serves as a lightweight factory that delegates the path‑resolution logic to `make_relative_link_target`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `current_page_path`
- parameter `label`
- parameter `target_page_path`
- parameter `code_style`

#### Writes To

- returned `LinkTarget` object

#### Usage Patterns

- creating hyperlink targets for page navigation
- building link data for markdown rendering

### `clore::generate::make_relative_link_target`

Declaration: `src/generate/render/common.cppm:68`

Definition: `src/generate/render/common.cppm:68`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Implementation: [Implementation](functions/make-relative-link-target.md)

The function `clore::generate::make_relative_link_target` computes a relative filesystem path from one documentation page to another. It normalizes both `current_page_path` and `target_page_path` using `std::filesystem::path::lexically_normal`, then determines the base directory by taking `parent_path` of the current path (falling back to `.` if none exists). The relative path is calculated via `lexically_relative` from that base; if the resulting path is empty (indicating the target is already identical to the base), the function falls back to returning the target’s `generic_string` form. Otherwise it returns the relative path in generic format. The implementation relies entirely on `std::filesystem` operations and uses no external dependencies beyond the C++ standard library.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `current_page_path`
- `target_page_path`

#### Writes To

- return value

#### Usage Patterns

- Called by `clore::generate::make_link_target` to produce a relative link target for documentation pages.

### `clore::generate::make_source_link_target`

Declaration: `src/generate/render/common.cppm:394`

Definition: `src/generate/render/common.cppm:394`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function first constructs a human-readable `label` by combining the file path (rendered relative via `make_source_relative`) and the source line number. It then queries the `LinkResolver` to see whether the file has a known documentation target. If a target path is found, it delegates to `make_link_target` to build a fully resolved `LinkTarget` with the constructed label, the resolved path, and `code_style` set to `true`. If no target is available, it returns a `LinkTarget` containing only the label and `code_style = true`, leaving the `target` field empty — effectively a local anchor with no hyperlink. The function depends on `make_source_relative` for path shortening, `LinkResolver::resolve` for cross‑reference lookup, and `make_link_target` for final `LinkTarget` creation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `location.file`
- `location.line`
- `config.project_root`
- `links`
- `current_page_path`

#### Usage Patterns

- Used to generate a source link target for navigation within generated documentation pages.
- Called by rendering functions that need to link to source code locations.

### `clore::generate::namespace_of`

Declaration: `src/generate/render/common.cppm:64`

Definition: `src/generate/render/common.cppm:64`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::namespace_of` implements a straightforward delegation pattern. It accepts a `std::string_view qualified_name` and returns the namespace portion of that name by forwarding the input to `extract::namespace_prefix_from_qualified_name`. The returned `std::string` is the result of that extraction call. There is no additional control flow, error handling, or transformation; the entire implementation consists of a single expression that relies entirely on the helper function from the `extract` namespace.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `qualified_name`

#### Usage Patterns

- obtaining namespace portion of a fully qualified symbol name

### `clore::generate::prompt_output_of`

Declaration: `src/generate/render/common.cppm:82`

Definition: `src/generate/render/common.cppm:82`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function accepts a reference to an `std::unordered_map<std::string, std::string>` named `outputs`, a `PromptKind` value `kind`, and an optional `std::string_view` `target_key`. It constructs a lookup key by calling `prompt_request_key` with a `PromptRequest` object whose `kind` and `target_key` fields are initialized from the parameters. The resulting string is used as the key for a `find` operation on the `outputs` map. If the key exists, the function returns a pointer to the associated value (the `std::string` element); otherwise it returns `nullptr`. This function serves as a thin lookup helper that abstracts the key-generation logic behind `prompt_request_key`, making callers agnostic to the exact key format. No other side effects or control flow variations are present.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `outputs` map
- `kind` parameter
- `target_key` parameter
- `prompt_request_key` function call

#### Usage Patterns

- Used to look up prompt outputs by kind and target key from a collection of generated results

### `clore::generate::push_link_paragraph`

Declaration: `src/generate/render/common.cppm:103`

Definition: `src/generate/render/common.cppm:103`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::push_link_paragraph` receives a `nodes` vector, a `label` string, and a vector of `LinkTarget` objects. It performs an early return if `targets` is empty. Otherwise, it constructs a single `Paragraph` by first appending a text fragment (via `make_text`) with the provided `label`. It then iterates over the `targets` vector: for each element after the first, it inserts a separator fragment `" | "`; for every target it appends a link fragment created by `make_link` using the target’s `label`, `target`, and `code_style` fields. The completed paragraph is moved into `nodes` as a `MarkdownNode`.

The algorithm depends on the `LinkTarget` structure and utility functions `make_text` and `make_link` for generating `MarkdownFragment` instances. It is designed to produce a single inline paragraph that clusters a set of hyperlinks under a common heading, separated by vertical bars — a common pattern in documentation navigation sections.

#### Side Effects

- Modifies the `nodes` vector by appending a new `MarkdownNode` representing a link paragraph.

#### Reads From

- `nodes` (not read, but passed by ref for mutation)
- `label`
- `targets`
- `targets[i].label`
- `targets[i].target`
- `targets[i].code_style`

#### Writes To

- `nodes` (appends a `MarkdownNode` containing a `Paragraph`)

#### Usage Patterns

- Building link paragraphs in markdown generation pipelines
- Similar to `push_location_paragraph` and `push_optional_link_paragraph` for rendering symbol documentation

### `clore::generate::push_location_paragraph`

Declaration: `src/generate/render/common.cppm:410`

Definition: `src/generate/render/common.cppm:410`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::push_location_paragraph` constructs and appends a single `Paragraph` node to the `nodes` vector. It first creates a `Paragraph` and inserts a text fragment from the `label` argument. Then, based on the `target` parameter, the control flow branches: if `target.target` is empty, a plain code fragment using `target.label` is added via `make_code`; otherwise, a hyperlink fragment is built via `make_link` using `target.label` as display text, `target.target` as the URL, and `target.code_style` to optionally wrap the label in code formatting. The completed `Paragraph` is wrapped in a `MarkdownNode` and pushed onto `nodes`. This function depends on the internal helpers `make_text`, `make_code`, and `make_link`, and on the `Paragraph`, `LinkTarget`, and `MarkdownNode` types.

#### Side Effects

- Appends a new `MarkdownNode` to the `nodes` collection.

#### Reads From

- label parameter
- target parameter (target`.target`, target`.label`, target`.code_style`)

#### Writes To

- nodes parameter (modified via `push_back`)

#### Usage Patterns

- Constructs a location paragraph node and appends it to the markdown node list.

### `clore::generate::push_optional_link_paragraph`

Declaration: `src/generate/render/common.cppm:122`

Definition: `src/generate/render/common.cppm:122`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::push_optional_link_paragraph` implements a conditional appending pattern. It first inspects the `std::optional<LinkTarget>` parameter `target` via `has_value()`; if the optional is empty the function returns immediately without modifying the output. When a target is present, the function constructs a `Paragraph` object by sequentially appending two `MarkdownFragment`s: a text fragment built from the provided `label` string via `make_text`, and a link fragment assembled from the three fields of the `LinkTarget` (its `label`, `target`, and `code_style`) through `make_link`. The completed `Paragraph` is then moved into the `nodes` vector as a `MarkdownNode`. This function depends on the `Paragraph` and `MarkdownNode` types, as well as the helper functions `make_text` and `make_link`, all belonging to the same generation module. The control flow is strictly linear with an early exit when the optional link target is absent.

#### Side Effects

- modifies the `nodes` vector by appending a `MarkdownNode`

#### Reads From

- `label`
- `target` (optional)
- `target->label`
- `target->target`
- `target->code_style`

#### Writes To

- `nodes` (via `push_back`)

#### Usage Patterns

- used to conditionally add a paragraph containing a text label and a link to the nodes list

### `clore::generate::short_name_of`

Declaration: `src/generate/render/common.cppm:56`

Definition: `src/generate/render/common.cppm:56`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::short_name_of` derives the leaf (unqualified) name from a fully qualified C++ identifier. Internally, it delegates to `extract::split_top_level_qualified_name`, which tokenizes the input `std::string_view` on scope-resolution `operator`s. If the resulting list is empty (i.e., the input is an empty string), an empty `std::string` is returned. Otherwise, the last element of the list—the base name—is returned. The function has no external dependencies beyond the `extract` utility and the standard library, and its control flow is a simple early-return guard followed by direct list access.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `qualified_name` parameter (`std::string_view`)

#### Usage Patterns

- extracting short symbol names for display
- stripping namespace prefixes from qualified names

### `clore::generate::strip_inline_markdown`

Declaration: `src/generate/render/common.cppm:44`

Definition: `src/generate/render/common.cppm:44`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::strip_inline_markdown` implements a straightforward character‑filtering algorithm to remove common inline Markdown formatting delimiters from a `std::string_view`. It iterates over each character in the input, skipping any that match backtick (`), asterisk (`*`), underscore (`_`), opening or closing square bracket (`[`, `]`), or hash (`#`). All other characters are appended to a local ``std::string`` buffer. After the loop, the function returns the result of passing this buffer through ``clore::generate::trim_ascii``, which strips leading and trailing ASCII whitespace. The only external dependency is ``trim_ascii``; no other routines or data structures are used.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `text` parameter (the input `std::string_view`)

#### Usage Patterns

- sanitizing user-provided or generated markdown text before comparison or display in plain text contexts
- stripping inline formatting from markdown strings during rendering or evidence building

### `clore::generate::symbol_analysis_markdown_for`

Declaration: `src/generate/render/common.cppm:172`

Definition: `src/generate/render/common.cppm:172`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::symbol_analysis_markdown_for` acts as a dispatch point based on the `plan.page_type` field. Internally it evaluates the `PageType` of the `plan` parameter; if the type equals `PageType::Namespace`, it delegates to `analysis_overview_markdown` passing `analyses` and `sym`. Otherwise, it falls through to `analysis_details_markdown`, forwarding the same arguments. Both callees are private helper functions that presumably produce a `std::string` markdown summary, and the result pointer is returned directly.

The control flow is a simple two‑way branch with no additional processing after the delegation. Dependencies include the `PagePlan` and `SymbolAnalysisStore` types, the `extract::SymbolInfo` structure, and the two helper functions whose signatures match `auto (const SymbolAnalysisStore&, const extract::SymbolInfo&) -> const std::string*`. The function does not perform any data transformation or error handling of its own; all analysis logic resides in the delegates.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- plan`.page_type`
- analyses
- sym

#### Usage Patterns

- Used to select the appropriate analysis markdown for a symbol based on the page type
- Dispatches to overview or detail markdown generation

### `clore::generate::symbol_doc_view_for`

Declaration: `src/generate/render/common.cppm:280`

Definition: `src/generate/render/common.cppm:280`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function selects the documentation view mode for a symbol based on the current page plan. It performs a switch on `plan.page_type`: for `Namespace` pages it unconditionally returns `SymbolDocView::Declaration`; for `Module` pages it returns `SymbolDocView::Details` when the symbol is a variable (according to `is_variable_kind(sym.kind)`), otherwise `SymbolDocView::Implementation`; any other page type defaults to `SymbolDocView::Details`.

The implementation depends on the `PagePlan` and `extract::SymbolInfo` types, the `PageType` and `SymbolDocView` enumerations, and the helper predicate `is_variable_kind`. The control flow is a simple `switch` with one conditional branch, making no external calls beyond the enum member selection.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- plan`.page_type`
- sym`.kind`

#### Usage Patterns

- called during page generation to select the appropriate documentation view for a symbol
- used to distinguish between declaration, details, and implementation views per page type

### `clore::generate::trim_ascii`

Declaration: `src/generate/render/common.cppm:34`

Definition: `src/generate/render/common.cppm:34`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Implementation: [Implementation](functions/trim-ascii.md)

The implementation of `clore::generate::trim_ascii` uses two sequential while loops to strip leading and trailing ASCII whitespace from a `std::string_view`. The first loop repeatedly inspects the first character via `text.front()`, checks it with `std::isspace` (applied to an `unsigned char` to avoid undefined behavior for negative values), and calls `text.remove_prefix(1)` until a non‑space character is encountered. The second loop performs the same logic from the end using `text.back()` and `text.remove_suffix(1)`. The function returns the trimmed view after both loops complete. This approach requires only the standard library facilities `<cctype>` for `std::isspace` and `<string_view>` for `std::string_view`, and performs O(n) comparisons in the worst case where almost the entire string is whitespace.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- text parameter

#### Usage Patterns

- Used to trim whitespace from prompt texts
- Used in stripping inline markdown
- Used when adding prompt output

## Internal Structure

The `generate:common` module acts as a shared utility layer within the documentation generation pipeline, providing reusable building blocks for constructing rendered content. It imports the project’s configuration, extraction, model, and markdown modules, and decomposes into several logical groups: string utilities (e.g., `trim_ascii`, `strip_inline_markdown`, `short_name_of`, `namespace_of`), link and target construction (the `LinkTarget` struct, `make_link_target`, `make_source_link_target`, link resolution helpers), and section-building routines (`build_list_section`, `build_prompt_section`, `add_symbol_analysis_sections`, `push_location_paragraph`, etc.). Internally, these functions rely on a few shared context variables such as `current_page_path`, `project_root`, and various analysis and page-plan handles, which are passed by reference through the call chain. The module does not define its own persistent state; instead, it operates on mutable outputs (document nodes) and queryable stores provided by the caller, forming a stateless, compositional layer that higher-level rendering modules compose to produce final documentation pages.

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)
- [Module generate:markdown](markdown.md)
- [Module generate:model](model.md)

