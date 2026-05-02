---
title: 'Module generate:common'
description: 'The module generate:common provides shared utilities and data structures for the documentation generation pipeline. It defines fundamental types such as LinkTarget (a struct with label, target, and code_style fields) and the SymbolDocView enum (with members Declaration, Implementation, and Details). The module exposes a collection of public functions for string manipulation (trim_ascii, strip_inline_markdown), name resolution (short_name_of, namespace_of), link target construction (make_link_target, make_relative_link_target, make_source_link_target), page lookup (find_declaration_page, find_implementation_pages, find_module_for_file), symbol collection (collect_namespace_symbols, collect_implementation_symbols), and content building (build_list_section, build_prompt_section, build_symbol_link_list, build_symbol_source_locations, symbol_doc_view_for, doc_label, symbol_analysis_markdown_for, build_string_list, build_related_page_targets). These routines are used by other generation modules to compose rendered documentation pages, cross-reference symbols, and format output consistently.'
layout: doc
template: doc
---

# Module `generate:common`

## Summary

The module `generate:common` provides shared utilities and data structures for the documentation generation pipeline. It defines fundamental types such as `LinkTarget` (a struct with `label`, `target`, and `code_style` fields) and the `SymbolDocView` enum (with members `Declaration`, `Implementation`, and `Details`). The module exposes a collection of public functions for string manipulation (`trim_ascii`, `strip_inline_markdown`), name resolution (`short_name_of`, `namespace_of`), link target construction (`make_link_target`, `make_relative_link_target`, `make_source_link_target`), page lookup (`find_declaration_page`, `find_implementation_pages`, `find_module_for_file`), symbol collection (`collect_namespace_symbols`, `collect_implementation_symbols`), and content building (`build_list_section`, `build_prompt_section`, `build_symbol_link_list`, `build_symbol_source_locations`, `symbol_doc_view_for`, `doc_label`, `symbol_analysis_markdown_for`, `build_string_list`, `build_related_page_targets`). These routines are used by other generation modules to compose rendered documentation pages, cross-reference symbols, and format output consistently.

## Imports

- [`config`](../config/index.md)
- [`extract`](../extract/index.md)
- [`generate:markdown`](markdown.md)
- [`generate:model`](model.md)
- `std`

## Imported By

- [`generate:page`](page.md)
- [`generate:symbol`](symbol.md)

## Types

### `clore::generate::LinkTarget`

Declaration: `generate/render/common.cppm:11`

Definition: `generate/render/common.cppm:11`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `clore::generate::LinkTarget` struct is a simple aggregate used to store the components of a generated link target. It holds three public data members: a `std::string` `label` for the display text, a `std::string` `target` for the destination URI or identifier, and a `bool` `code_style` that controls whether the link should be rendered with a monospace style. The `code_style` member is default‑initialized to `false`, establishing the invariant that links are not styled as code unless explicitly set. No constructors, assignment `operator`s, or validation logic are provided; the struct relies on default member initialisation and aggregate initialisation to maintain internal consistency.

### `clore::generate::SymbolDocView`

Declaration: `generate/render/common.cppm:17`

Definition: `generate/render/common.cppm:17`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The enumeration `clore::generate::SymbolDocView` uses `std::uint8_t` as its underlying type, ensuring a compact representation. Its enumerators—`Declaration`, `Implementation`, and `Details`—are implicitly assigned contiguous integer values starting from zero, which encodes the natural precedence order for view modes. This ordering and the choice of a fixed‑width unsigned integer type support efficient storage and direct comparison, while the enum class scoping prevents accidental mixing with plain integers. No additional member values or flags are defined; the three enumerators cover all intended display modes, establishing a simple closed‑set invariant for dispatches that select rendering behavior.

#### Invariants

- Each enumerator is a unique value of type `std::uint8_t`.
- Values are non-negative and ordered as declared.

#### Key Members

- `Declaration`
- `Implementation`
- `Details`

#### Usage Patterns

- Used to parameterize rendering logic for symbol documentation.
- Consumed by code that generates different output sections based on the selected view.

#### Member Variables

##### `clore::generate::SymbolDocView::Declaration`

Declaration: `generate/render/common.cppm:18`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Declaration
```

##### `clore::generate::SymbolDocView::Details`

Declaration: `generate/render/common.cppm:20`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Details
```

##### `clore::generate::SymbolDocView::Implementation`

Declaration: `generate/render/common.cppm:19`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Implementation
```

## Variables

### `clore::generate::add_prompt_output`

Declaration: `generate/render/common.cppm:142`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

No read or write references to `clore::generate::add_prompt_output` appear in the provided evidence. Its exact initialization, mutation, or consumption is not documented in the available snippets, leaving its precise role unclear.

#### Mutation

No mutation is evident from the extracted code.

### `clore::generate::add_symbol_analysis_detail_sections`

Declaration: `generate/render/common.cppm:170`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

This variable likely participates in the generation of documentation by appending or formatting detail sections based on analysis data. It is referenced alongside other rendering variables such as `analyses`, `config`, and `symbol` to produce structured output.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- likely invoked to add analysis detail sections

### `clore::generate::add_symbol_analysis_sections`

Declaration: `generate/render/common.cppm:176`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

This variable is used within the rendering pipeline to attach analysis sections to symbol documentation. It interacts with related variables such as `analyses`, `plan`, `sym`, `config`, and `target` to produce structured analysis content. The exact parameters and invocation details are not visible in the provided evidence.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Called to add analysis sections for a symbol

### `clore::generate::push_link_paragraph`

Declaration: `generate/render/common.cppm:92`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The variable's exact role is not described in the provided evidence; however, its name suggests it is a callable (likely a function or lambda) responsible for inserting a link paragraph into generated documentation. No concrete usage or mutation is observed in the evidence.

#### Mutation

No mutation is evident from the extracted code.

### `clore::generate::push_location_paragraph`

Declaration: `generate/render/common.cppm:399`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

This variable is used by the function `clore::generate::build_symbol_source_locations` to produce a formatted paragraph describing a symbol's location. It is a callable that accepts relevant parameters and returns a string or similar output. The evidence does not indicate any mutation after initialization.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- called by `build_symbol_source_locations`

### `clore::generate::push_optional_link_paragraph`

Declaration: `generate/render/common.cppm:111`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Based solely on the declaration, the variable holds a deduced type and appears intended to be used as part of the link paragraph generation logic. No usage or mutation is evident from the provided context.

#### Mutation

No mutation is evident from the extracted code.

## Functions

### `clore::generate::build_list_section`

Declaration: `generate/render/common.cppm:133`

Definition: `generate/render/common.cppm:133`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function constructs a `SemanticSectionPtr` by calling `make_section` with `SemanticKind::Section`, an empty span, the moved `heading`, and `level`. After creating the section, it checks whether the moved `list`’s `.items` container is non‑empty; if so, it appends a `MarkdownNode` wrapping the `BulletList` to `section->children`. The returned section represents a structurally complete list block that can later be embedded into the document hierarchy.

Internally, the function depends solely on the `make_section` factory and the `MarkdownNode` type, as well as the `BulletList` data structure. No additional analysis or linking logic is performed; it is a thin builder that converts a raw list plus heading metadata into a semantic node, making it a leaf‑level rendering utility for list‑oriented content such as symbol listings or navigation summaries.

#### Side Effects

- Allocates dynamic memory for the `SemanticSectionPtr` and any associated child `MarkdownNode`

#### Reads From

- parameters `heading`, `level`, and `list`

#### Writes To

- allocated memory for the returned `SemanticSectionPtr` and its `children` vector
- return value (owned pointer to the constructed section)

#### Usage Patterns

- Called by page builders to generate a list section with a heading
- Used when a documentation section requires a bullet list as content
- Likely invoked in the generation of overview, analysis, or summary pages

### `clore::generate::build_prompt_section`

Declaration: `generate/render/common.cppm:124`

Definition: `generate/render/common.cppm:124`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::build_prompt_section` constructs a `SemanticSectionPtr` representing a prompt section. It first calls `make_section` with `SemanticKind::Section`, an empty vector, the provided `heading`, and `level` to produce a base section node. If the optional `output` pointer is non‑null and its content is non‑empty after stripping ASCII whitespace via `trim_ascii` , the function appends a raw markdown child—created by `make_raw_markdown(*output)`—to the section’s children vector. The resulting section is returned, ready for further nesting or insertion into a larger document tree.

#### Side Effects

- Allocates a `SemanticSection` object from `make_section`
- Conditionally allocates a `MarkdownNode` from `make_raw_markdown`
- Performs write operations on the `children` vector of the section

#### Reads From

- Parameter `heading`
- Parameter `level`
- Parameter `output` (null check)
- String pointed to by `output` (if non-null, after trimming via `trim_ascii`)

#### Writes To

- The returned `SemanticSectionPtr` pointing to a newly allocated section
- The `children` member of that section, if output is added

#### Usage Patterns

- Building prompt sections with optional output text for documentation generation
- Used in combination with `make_section` and `make_raw_markdown` to structure prompt content

### `clore::generate::build_related_page_targets`

Declaration: `generate/render/common.cppm:504`

Definition: `generate/render/common.cppm:504`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::build_related_page_targets` iterates over each entry in `plan.linked_pages`. For every entry, it attempts to split the string at the first colon to separate an optional prefix (e.g., `"module"` or `"namespace"`) from the entity name. If a prefix exists, the function delegates resolution to either `links.resolve_module` or `links.resolve_namespace`; otherwise it uses `links.resolve`. If a valid target path is obtained and it has not already been added (tracked by an internal `seen` set), the function retrieves a page title via `links.resolve_page_title`. The title is cleaned with `clore::generate::strip_inline_markdown` to produce a label, falling back to the raw entity name if the cleaned title is empty. Finally, a `LinkTarget` is constructed via `clore::generate::make_link_target` using `current_page_path`, the label, and the resolved target path, and appended to the output vector.

The control flow is a straightforward single pass through the linked pages, with duplicate elimination performed by a `std::unordered_set<std::string>` keyed on resolved target paths. Dependencies include the `PagePlan` object for input, the `LinkResolver` object for all page and name resolution, and utility functions `strip_inline_markdown` and `make_link_target`. The result is a vector of `LinkTarget` entries representing deduplicated, fully resolved references to related pages.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.linked_pages`
- `links.resolve`
- `links.resolve_module`
- `links.resolve_namespace`
- `links.resolve_page_title`
- `current_page_path`
- results of `strip_inline_markdown`

#### Usage Patterns

- called during page generation to collect related page links
- used to populate a "related pages" section in a documentation page

### `clore::generate::build_string_list`

Declaration: `generate/render/common.cppm:148`

Definition: `generate/render/common.cppm:148`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::build_string_list` iterates over a vector of strings provided as `items`. For each element, it first calls `trim_ascii` to strip whitespace and skips the item if the result is empty. Non‑empty trimmed text is passed to `code_spanned_fragments` to produce styled fragments, which are then packed into a `ListItem` and appended to a `BulletList`. The algorithm returns the completed list. It depends on the helper `trim_ascii` for input sanitization and `code_spanned_fragments` for fragment generation; no other external facilities are involved.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- const `std::vector<std::string>`& items

#### Usage Patterns

- Constructs a markdown bullet list from a list of strings
- Used in generating symbol lists and evidence sections in documentation pages
- Relies on `code_spanned_fragments` for inline code formatting

### `clore::generate::build_symbol_link_list`

Declaration: `generate/render/common.cppm:360`

Definition: `generate/render/common.cppm:360`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function iterates over each element in the `symbols` vector and builds a `BulletList` by constructing one `ListItem` per symbol. For every symbol, it prepends the kind name via `extract::symbol_kind_name` as plain text, then determines the display label by checking `use_full_name`; if false, it uses `short_name_of` on the symbol’s qualified name. The core branching decision relies on `links.resolve(sym->qualified_name)`: if a target path is found, the function creates a hyperlink using `make_link` with a relative target computed by `make_relative_link_target`; otherwise it falls back to `make_code` with the same label. The resulting list items are appended to `list`, which is returned.

Control flow is a straightforward range-based `for` loop with a single conditional (the `if`‑`else` for link vs. code). Key dependencies include the `LinkResolver` for symbol‑to‑page mapping, the helper `short_name_of` for abbreviation, and the low‑level fragment constructors (`make_text`, `make_link`, `make_code`). The function does not perform any recursive or nested symbol traversal; it relies entirely on the caller to provide the processed `symbols` vector and a pre‑populated `links` object.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `symbols` (vector of pointers to `extract::SymbolInfo`)
- `current_page_path` (string view of current document path)
- `links` (instance of `LinkResolver`)
- `use_full_name` (boolean flag)
- `sym->kind` (symbol kind enumeration)
- `sym->qualified_name` (string of fully qualified symbol name)
- result of `links.resolve(sym->qualified_name)` (optional target path)
- result of `short_name_of(sym->qualified_name)` (shortened name if `use_full_name` is false)

#### Usage Patterns

- building navigable symbol lists for documentation pages
- creating bullet lists of related symbols with resolved hyperlinks
- used in page generation to render symbol cross-references

### `clore::generate::build_symbol_source_locations`

Declaration: `generate/render/common.cppm:412`

Definition: `generate/render/common.cppm:412`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function iterates over the declaration and optional definition locations stored in the input `extract::SymbolInfo`. For each location that is known (i.e., `declaration_location.is_known()` and, if `definition_location` has a value, its `is_known()` check passes), it calls `push_location_paragraph` with a descriptive label and a `LinkTarget` produced by `make_source_link_target`. The resulting `MarkdownNode` vector accumulates these location paragraphs in order. Internally, the control flow is a simple sequence of two conditional blocks; the only dependency on external helpers is `push_location_paragraph` for node construction and `make_source_link_target` for converting a source location into a navigable link relative to `current_page_path`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `sym` (parameter of type `const extract::SymbolInfo&`)
- `config` (parameter of type `const config::TaskConfig&`)
- `links` (parameter of type `const LinkResolver&`)
- `current_page_path` (parameter of type `std::string_view`)

#### Usage Patterns

- Used to build source location sections in generated documentation pages

### `clore::generate::collect_implementation_symbols`

Declaration: `generate/render/common.cppm:314`

Definition: `generate/render/common.cppm:314`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function begins by initializing an empty vector `symbols` and an unordered set `seen` to track already-collected symbol `IDs` for deduplication. It iterates over each key in `plan.owner_keys`. If the `plan.page_type` is `PageType::Module`, it calls `extract::find_modules_by_name` to retrieve each matching module, then iterates over the module’s `symbols`. For each symbol ID, it checks whether it has already been inserted into `seen`; if not, it attempts to look up the symbol via `extract::lookup_symbol`, verifies it is a page-level symbol via `is_page_level_symbol`, and applies the user-supplied `predicate`. If all conditions pass, the symbol pointer is appended to `symbols`. For non‑module keys, the function treats each key as a file identifier, locates the file in `model.files`, and processes that file’s `symbols` in the same manner.

After processing all owner keys, the collected symbol pointers are sorted by `qualified_name` and returned. Key dependencies include `extract::lookup_symbol`, `extract::find_modules_by_name`, `is_page_level_symbol`, and the `seen` set for deduplication, ensuring no symbol appears more than once even if referenced from multiple owner keys.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.owner_keys`
- `plan.page_type`
- `model.files`
- `module->symbols`
- `file_it->second.symbols`
- `sym->qualified_name`
- `predicate` invoked with each symbol

#### Writes To

- local `std::vector<const extract::SymbolInfo*> symbols`
- local `std::unordered_set<extract::SymbolID> seen`

#### Usage Patterns

- Called to gather all implementation symbols for a given page plan and project model
- Typically used to populate the symbol list for rendering a page, with a predicate filtering by symbol kind or other criteria

### `clore::generate::collect_namespace_symbols`

Declaration: `generate/render/common.cppm:289`

Definition: `generate/render/common.cppm:289`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function retrieves all page-level symbols belonging to a given namespace that also satisfy a caller-supplied predicate. It first searches for the namespace within the model's namespace map; if absent, an empty vector is returned. Otherwise, it iterates over the symbol `IDs` stored in that namespace, resolving each to a pointer via `extract::lookup_symbol`, filtering on `is_page_level_symbol` and the `predicate` before appending to a result vector. After collection, the symbols are sorted by `qualified_name` using `std::sort` with a comparator lambda. Internal flow therefore consists of a map lookup, a linear scan over symbol `IDs` with three validity checks, and a final sort. Dependencies include `extract::lookup_symbol` for symbol resolution, the model's `namespaces` associative container, and the local helper `is_page_level_symbol`; the template parameter `Predicate` allows arbitrary filtering on each symbol's attributes.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model.namespaces`
- namespace symbol `IDs`
- `extract::lookup_symbol`
- `is_page_level_symbol`
- `predicate`
- symbol `qualified_name` for sorting

#### Usage Patterns

- called by page‑building functions to gather namespace symbols for documentation generation

### `clore::generate::doc_label`

Declaration: `generate/render/common.cppm:279`

Definition: `generate/render/common.cppm:279`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::doc_label` maps a `SymbolDocView` enumerator to a human-readable label string. Its implementation consists of a `switch` statement over the three possible values: `SymbolDocView::Declaration` yields `"Declaration"`, `SymbolDocView::Implementation` yields `"Implementation"`, and `SymbolDocView::Details` yields `"Details"`. A default fallback also returns `"Details"`, ensuring only valid inputs produce the expected output. The function has no external dependencies beyond the `SymbolDocView` definition and uses no loops or conditional branching other than the `switch`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `view` parameter of type `SymbolDocView`

#### Usage Patterns

- Used to obtain a display label for a symbol documentation view
- Called in contexts where a human-readable heading or tag is needed for the declaration, implementation, or details section

### `clore::generate::find_declaration_page`

Declaration: `generate/render/common.cppm:473`

Definition: `generate/render/common.cppm:473`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function first attempts to resolve a direct declaration page for the given symbol by calling `links.resolve(sym.qualified_name)`. If a target path is found and it differs from the `current_page_path`, it constructs a label from the symbol’s enclosing namespace (or falls back to `namespace_of` on the qualified name) and returns a `LinkTarget` via `make_link_target` with the `code_style` flag set. If the direct resolution fails, the function checks whether the namespace name is non‑empty; if empty, it immediately returns `std::nullopt`. Otherwise, it queries `links.resolve_namespace(ns_name)`; on success, it generates a link target with the label “Namespace ” plus the namespace name, still using `make_link_target`. If neither resolution yields a valid target, the function returns `std::nullopt`. The implementation relies on `LinkResolver` to map qualified names and namespace names to page paths, and on `make_link_target` to produce the final `LinkTarget` result.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `sym.qualified_name`
- `sym.enclosing_namespace`
- `links` resolver object
- `current_page_path`
- `namespace_of` function (reads symbol name)

#### Usage Patterns

- Used during page layout construction to generate a 'Declaration' link for symbols
- Called when building symbol documentation pages to provide navigation to declaration site
- Invoked as part of rendering a page bundle to include declaration backlinks

### `clore::generate::find_implementation_pages`

Declaration: `generate/render/common.cppm:433`

Definition: `generate/render/common.cppm:433`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function collects all implementation‑page link targets for a given symbol by inspecting its definition and declaration source locations. It uses a `try_add` lambda that, for a non‑empty file path, first attempts to find a containing module via `extract::find_module_by_source`. If a module exists, it resolves the module’s name through `links.resolve_module` and adds a `LinkTarget` labeled with that module name. Otherwise, it falls back to a direct file‑path resolution via `links.resolve` and labels the target with a source‑relative path. A `seen` set of target strings deduplicates the results. The function then calls `try_add` on `sym.definition_location->file` (if present) and always on `sym.declaration_location.file`, returning the vector of collected `LinkTarget` values.

Internally, the control flow relies on optional values and a local lambda with a mutable capture set. The primary dependencies are the `extract::find_module_by_source` utility for module lookup, the `LinkResolver` interface for resolving module names and file paths, and the helper `make_link_target` which constructs the final `LinkTarget` given the current page path, a label, a target path, and the `code_style` flag.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `sym` parameter
- the `model` parameter
- the `links` parameter
- the `current_page_path` parameter
- the `project_root` parameter

#### Usage Patterns

- Collecting link targets for implementation pages of a symbol during documentation generation

### `clore::generate::find_module_for_file`

Declaration: `generate/render/common.cppm:496`

Definition: `generate/render/common.cppm:496`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function queries the extract layer by invoking `extract::find_module_by_source` with the given `model` and `file_path`. If a module is found, it returns the module’s `name` field as a `std::optional<std::string>`; otherwise it returns `std::nullopt`. The implementation depends on `extract::find_module_by_source` to perform the lookup against the project model, and its control flow is a straightforward conditional branch based on the presence of a module match.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `extract::ProjectModel` parameter `model`
- the `std::string_view` parameter `file_path`

#### Usage Patterns

- Mapping source file paths to module names for documentation generation
- Used as a helper in page-building functions such as `build_page_root` and `build_file_page_root`

### `clore::generate::make_link_target`

Declaration: `generate/render/common.cppm:81`

Definition: `generate/render/common.cppm:81`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_link_target` is a straightforward factory that constructs a `LinkTarget` value. It accepts a `current_page_path`, a `label`, a `target_page_path`, and an optional boolean `code_style` (defaulting to `false`). Internally, it directly initializes the returned `LinkTarget` by moving the provided `label` into the `.label` field, calling `make_relative_link_target(current_page_path, target_page_path)` to compute the `.target` relative path, and copying the `code_style` argument into the `.code_style` field. There is no branching or iteration; its sole purpose is to bundle these three components into a single `LinkTarget` object, relying on `make_relative_link_target` for the actual path resolution.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `current_page_path`
- `label`
- `target_page_path`
- `code_style`

#### Usage Patterns

- used to generate relative link targets for cross-referencing pages
- called by page rendering utilities that produce documentation hyperlinks

### `clore::generate::make_relative_link_target`

Declaration: `generate/render/common.cppm:57`

Definition: `generate/render/common.cppm:57`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function converts an absolute or relative target path into a filesystem-relative link from the current page’s directory. It normalizes both paths via `std::filesystem::path::lexically_normal`, then extracts the parent of `current_page_path` as the base directory. Using `std::filesystem::path::lexically_relative`, it computes a relative path from `base` to the target; if that relative path is empty (e.g., the target is on a different drive), it falls back to returning the target’s generic string unchanged. Otherwise it returns the relative path in generic (forward-slash) form. The function depends on `std::filesystem` for path manipulation and avoids string copying until the final return.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `current_page_path`
- `target_page_path`

#### Usage Patterns

- Generating relative links between documentation pages
- Computing href attributes in generated Markdown

### `clore::generate::make_source_link_target`

Declaration: `generate/render/common.cppm:383`

Definition: `generate/render/common.cppm:383`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_source_link_target` builds a `LinkTarget` for a given source location. It first constructs a human-readable label by calling `make_source_relative` on the file path from the `SourceLocation` using `config.project_root`, then appends a colon and the line number. It then attempts to resolve the file through `links.resolve(location.file)`. If a target page path is found, it delegates to `make_link_target` with the current page path, the label, the resolved target, and a boolean indicating that the link points to a source file (`true`). If resolution fails, it returns a `LinkTarget` with the label, an empty `target`, and `code_style` set to `true`, indicating the label should be rendered as code without a hyperlink.

This function depends on `make_source_relative` to compute the relative file path, the `LinkResolver` interface for mapping file paths to documentation pages, and the `make_link_target` helper to create a fully qualified `LinkTarget`. The `LinkTarget` struct is used to carry the display text, the destination URL, and a flag for code formatting. The `TaskConfig` provides the project root for path normalization.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `location.file`
- `location.line`
- `config.project_root`
- `links` resolver
- `current_page_path`

#### Writes To

- returned `LinkTarget` object (by value)

#### Usage Patterns

- Called when generating source-reference links in documentation pages
- Used to create clickable source file locations with line numbers

### `clore::generate::namespace_of`

Declaration: `generate/render/common.cppm:53`

Definition: `generate/render/common.cppm:53`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::namespace_of` delegates entirely to the helper `extract::namespace_prefix_from_qualified_name`, forwarding its `qualified_name` argument and returning the computed namespace prefix. No additional logic, branching, or error handling is present; the implementation acts as a thin wrapper that hides the extraction utility behind a consistent interface. This design keeps the call site decoupled from the extraction algorithm and centralizes any future changes to namespace parsing within the `extract` module.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `qualified_name` parameter

#### Usage Patterns

- Obtaining the namespace part of a symbol's qualified name for grouping or display
- Used internally within page-building functions to determine namespace context

### `clore::generate::prompt_output_of`

Declaration: `generate/render/common.cppm:71`

Definition: `generate/render/common.cppm:71`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::prompt_output_of` implements a deferred lookup pattern for precomputed prompt results. It constructs a lookup key by building a `PromptRequest` from the `kind` and `target_key` parameters, then passes that request to `prompt_request_key` to produce the actual map key. If the key exists in the `outputs` unordered map, the function returns a pointer to the associated string; otherwise it returns `nullptr`. The internal control flow is a single `std::unordered_map::find` call, with no iteration or branching beyond the existence check. Its primary dependencies are the `PromptKind` enum, the `PromptRequest` aggregate, and the `prompt_request_key` function that serializes the request into a string suitable for map indexing.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `outputs` parameter (const `std::unordered_map<std::string, std::string>&`)
- the `kind` parameter (`PromptKind`)
- the `target_key` parameter (`std::string_view`)
- the result of `prompt_request_key`

#### Usage Patterns

- Look up existing analysis output
- Retrieve prompt response for a given kind
- Check if a prompt has been generated for a symbol

### `clore::generate::short_name_of`

Declaration: `generate/render/common.cppm:45`

Definition: `generate/render/common.cppm:45`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function obtains the shortest name for a given qualified identifier by delegating the tokenization step to `extract::split_top_level_qualified_name`. After splitting, it checks whether the resulting component list is empty — if so, it returns an empty string; otherwise it returns the last element of the collection, which corresponds to the unqualified name of the symbol. The implementation relies solely on the external splitting utility and standard string handling; there is no recursion or additional parsing.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `qualified_name` parameter

#### Usage Patterns

- extract unqualified symbol names from fully qualified identifiers
- build short display names for namespaces, classes, and functions
- strip leading namespace or module qualifiers when presenting symbol names

### `clore::generate::strip_inline_markdown`

Declaration: `generate/render/common.cppm:33`

Definition: `generate/render/common.cppm:33`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::strip_inline_markdown` performs a single linear pass over the input `text`. For each character `ch`, it checks against a fixed set of Markdown formatting characters—backtick (` ` `), asterisk (`*`), underscore (`_`), square brackets (`[`, `]`), and hash (`#`)—and skips them using `continue`. All other characters are appended to a local ``std::string`` `out`. After the loop, the accumulated string is trimmed of leading and trailing whitespace by calling ``clore::generate::trim_ascii`` and returned.

No additional data structures or complex control flow are required. The sole dependency is the `clore::generate::trim_ascii` function, which handles whitespace removal. The algorithm is intentionally minimal, relying on character-level filtering rather than parsing nested Markdown constructs. This makes it suitable only for stripping simple inline markers, not for fully parsing Markdown syntax.

#### Side Effects

- Allocates a new `std::string` to hold the stripped result.
- Calls `trim_ascii` to trim whitespace from the result.

#### Reads From

- Parameter `text` of type `std::string_view`

#### Writes To

- The returned `std::string` containing the stripped text

### `clore::generate::symbol_analysis_markdown_for`

Declaration: `generate/render/common.cppm:161`

Definition: `generate/render/common.cppm:161`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::symbol_analysis_markdown_for` dispatches between two rendering paths based on the `page_type` field of the `plan` parameter. When `plan.page_type` equals `PageType::Namespace`, it delegates to `analysis_overview_markdown`, which generates a high‑level symbol analysis summary appropriate for a namespace overview page. Otherwise it calls `analysis_details_markdown`, which produces a detailed per‑symbol analysis report. Both helpers receive the full `analyses` store and the target `sym` symbol info, and each returns a pointer to a string. This internal control flow isolates the page‑type decision and allows the two sub‑functions to focus on their respective markdown structures without repeating the selection logic at each call site.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const SymbolAnalysisStore& analyses`
- `const PagePlan& plan` (specifically `plan.page_type`)
- `const extract::SymbolInfo& sym`

#### Usage Patterns

- Used during page generation to retrieve symbol analysis markdown for rendering
- Called by functions like `render_page_markdown` to obtain analysis content

### `clore::generate::symbol_doc_view_for`

Declaration: `generate/render/common.cppm:269`

Definition: `generate/render/common.cppm:269`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::symbol_doc_view_for` determines the appropriate `SymbolDocView` for a given `PagePlan` and `extract::SymbolInfo` by evaluating the page type and symbol kind. Its control flow is a single switch on `plan.page_type`. For `PageType::Namespace`, it unconditionally returns `SymbolDocView::Declaration`. For `PageType::Module`, it checks `is_variable_kind(sym.kind)`; if the symbol is a variable kind, it returns `SymbolDocView::Details`, otherwise `SymbolDocView::Implementation`. In all other page types (the default case), it returns `SymbolDocView::Details`. This selection drives downstream rendering decisions, such as whether to show declaration details, implementation notes, or full documentation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.page_type`
- `sym.kind`

#### Usage Patterns

- Called when building documentation views to determine the detail level of a symbol on a page
- Used in rendering logic to decide whether to show declaration, implementation, or full details

### `clore::generate::trim_ascii`

Declaration: `generate/render/common.cppm:23`

Definition: `generate/render/common.cppm:23`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::trim_ascii` performs an in‑place trimming of leading and trailing ASCII whitespace from a `std::string_view`. Its control flow consists of two sequential while loops. The first loop repeatedly checks `std::isspace` on the front character (cast to `unsigned char` to avoid undefined behavior) and advances the view via `remove_prefix(1)` until a non‑space character is found or the view becomes empty. The second loop similarly inspects the back character with `std::isspace` and calls `remove_suffix(1)` until the trailing whitespace is removed. The function returns the resulting `std::string_view` with its original data pointer and an adjusted size. No external dependencies beyond the C++ standard library are used; the algorithm relies solely on `std::isspace` and the `std::string_view` mutation primitives.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `text` parameter of type `std::string_view`

#### Usage Patterns

- preprocess text before analysis
- sanitize leading/trailing whitespace
- normalize markdown fragment content

## Internal Structure

The module `generate:common` serves as the shared rendering layer within the documentation generation pipeline. It imports the extracted project model (`extract`), the page‑plan and symbol representations (`generate:model`), the Markdown construction library (`generate:markdown`), and the configuration module (`config`). This layering positions `common` above the core model and below page‑specific renderers, providing reusable utilities that bridge the gap between the raw semantic information and the final Markdown output.

Internally, the module is decomposed into several functional groups: string and name utilities (`trim_ascii`, `strip_inline_markdown`, `short_name_of`, `namespace_of`, `build_string_list`); link‑target construction (`make_link_target`, `make_relative_link_target`, `make_source_link_target`); page discovery (`find_declaration_page`, `find_implementation_pages`, `find_module_for_file`); symbol collection (`collect_namespace_symbols`, `collect_implementation_symbols`); and rendering helpers that assemble structural document components (`build_list_section`, `build_prompt_section`, `build_symbol_link_list`, `build_symbol_source_locations`). Public callable variables complement these functions to orchestrate paragraph insertion (`push_link_paragraph`, `push_optional_link_paragraph`, `push_location_paragraph`) and symbol analysis sections (`add_symbol_analysis_sections`, `add_symbol_analysis_detail_sections`, `add_prompt_output`). This design centralizes common logic, avoids duplication across page builders, and encapsulates the resolution of names, locations, and cross‑references behind a stable interface.

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)
- [Module generate:markdown](markdown.md)
- [Module generate:model](model.md)

