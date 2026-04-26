---
title: 'Module generate:common'
description: 'The generate:common module provides fundamental rendering and linking utilities used across the documentation generation pipeline. It owns public functions for constructing link targets (make_link_target, make_relative_link_target, make_source_link_target), building structural components such as string lists (build_string_list), list sections (build_list_section), and prompt sections (build_prompt_section), and for manipulating Markdown fragments (strip_inline_markdown, trim_ascii). It also defines the LinkTarget struct and the SymbolDocView enumeration, which control how symbol information is presented in generated output.'
layout: doc
template: doc
---

# Module `generate:common`

## Summary

The `generate:common` module provides fundamental rendering and linking utilities used across the documentation generation pipeline. It owns public functions for constructing link targets (`make_link_target`, `make_relative_link_target`, `make_source_link_target`), building structural components such as string lists (`build_string_list`), list sections (`build_list_section`), and prompt sections (`build_prompt_section`), and for manipulating Markdown fragments (`strip_inline_markdown`, `trim_ascii`). It also defines the `LinkTarget` struct and the `SymbolDocView` enumeration, which control how symbol information is presented in generated output.

The module is responsible for resolving symbol- and page-related references, including collecting implementation and namespace symbols (`collect_implementation_symbols`, `collect_namespace_symbols`), finding declaration and implementation pages (`find_declaration_page`, `find_implementation_pages`), and associating files with their respective modules (`find_module_for_file`). Additional utilities support symbol analysis rendering (`symbol_analysis_markdown_for`, `doc_label`, `symbol_doc_view_for`), source location assembly (`build_symbol_source_locations`), and related‑page target generation (`build_related_page_targets`). Together, these functions form a public‑facing layer that other generation modules depend on to produce coherent, cross‑linked documentation pages.

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

The struct `clore::generate::LinkTarget` is a plain aggregate containing three public data members: `label` and `target` (both `std::string`), and `code_style` (`bool`, defaulting to `false`). There are no custom constructors, destructors, or member functions; the type is designed for simple bundling of link metadata. The `code_style` field indicates whether the link should be rendered in a code‑style context, and its default value ensures that non‑code links are the common case. No invariants are enforced beyond the implicit guarantees of `std::string` (e.g., non‑null content) and `bool`; the struct is trivially copyable and movable, relying on the compiler‑generated special member functions.

#### Invariants

- `label` and `target` are conventional strings with no additional constraints
- `code_style` defaults to `false` when not explicitly set

#### Key Members

- `label`: the visible link text
- `target`: the URL or destination
- `code_style`: if `true`, indicates link should be rendered in a code-style font

#### Usage Patterns

- Created via aggregate initialization, e.g., `LinkTarget{"text", "url", true}`
- Likely used in rendering contexts to produce anchor tags with optional inline code formatting
- May be stored in containers to represent multiple link targets for a document

### `clore::generate::SymbolDocView`

Declaration: `generate/render/common.cppm:17`

Definition: `generate/render/common.cppm:17`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The enum `clore::generate::SymbolDocView` is a scoped enum with an explicit underlying type of `std::uint8_t`, ensuring a compact representation suitable for storage in tight data structures or for use as a bit-field flag. Its three enumerators—`Declaration`, `Implementation`, and `Details`—form an ordered, exhaustive set that controls the granularity of rendered symbol documentation. No special member implementations or invariants exist beyond the standard enum semantics; the sequential values (0, 1, 2) are implicitly assigned, and the underlying type guarantees that all enumerators fit within a single byte. The enum is used internally by the generate step to decide which portions of a symbol’s documentation to emit, with `Details` representing the most inclusive view.

#### Invariants

- Each member represents a distinct view mode.
- Values are ordered by increasing level of detail (Declaration → Implementation → Details).
- Only the three defined enumerators are valid.

#### Key Members

- `Declaration`
- `Implementation`
- `Details`

#### Usage Patterns

- Used to parameterize documentation rendering functions.
- Controls which parts of a symbol's documentation are displayed.

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

Based on its name and context, `clore::generate::add_prompt_output` probably stores the outcome of constructing or appending an output for a prompt request, such as a parsed response or output handle. No further evidence of its specific initialization or role is available.

#### Mutation

No mutation is evident from the extracted code.

### `clore::generate::add_symbol_analysis_detail_sections`

Declaration: `generate/render/common.cppm:170`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

This entity likely processes an analysis result and appends formatted detail sections to a symbol's documentation page. The evidence does not indicate any mutation; it is invoked to generate content based on input parameters such as `analyses`, `config`, and `target`.

#### Mutation

No mutation is evident from the extracted code.

### `clore::generate::add_symbol_analysis_sections`

Declaration: `generate/render/common.cppm:176`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

No evidence describes how `add_symbol_analysis_sections` is used or its role in surrounding logic. The variable likely participates in the symbol analysis section generation process, but specific interactions are not documented in the provided snippets.

#### Mutation

No mutation is evident from the extracted code.

### `clore::generate::push_link_paragraph`

Declaration: `generate/render/common.cppm:92`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The role, initialization, and usage of `clore::generate::push_link_paragraph` are not documented in the provided evidence.

#### Mutation

No mutation is evident from the extracted code.

### `clore::generate::push_location_paragraph`

Declaration: `generate/render/common.cppm:399`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Its role is to push a location paragraph for a symbol's source location. It is consumed in the context of building symbol source location data.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Called in `build_symbol_source_locations`

### `clore::generate::push_optional_link_paragraph`

Declaration: `generate/render/common.cppm:111`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

No evidence describes how it is read or participates in logic beyond its declaration. It likely serves as a helper for conditionally adding a link paragraph to generated documentation pages.

#### Mutation

No mutation is evident from the extracted code.

## Functions

### `clore::generate::build_list_section`

Declaration: `generate/render/common.cppm:133`

Definition: `generate/render/common.cppm:133`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::build_list_section` constructs a `SemanticSectionPtr` by delegating to `make_section` with `SemanticKind::Section`, an empty attribute set, the provided `heading` string, and the `level`. After the section is created, it checks whether the incoming `BulletList` contains any items; if so, it appends a `MarkdownNode` wrapping the complete `list` to the section’s child vector. No further processing or manipulation of the list is performed—the function simply assembles a titled section that either contains the list (if non-empty) or remains childless. Its primary dependency is on `make_section` for section allocation and on the `BulletList` (via `items`) and `MarkdownNode` types for child storage.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- heading
- level
- list

#### Writes To

- local `section` variable
- returned `SemanticSectionPtr`

#### Usage Patterns

- called by page-building functions to wrap a bullet list under a heading

### `clore::generate::build_prompt_section`

Declaration: `generate/render/common.cppm:124`

Definition: `generate/render/common.cppm:124`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::build_prompt_section` constructs a semantic section by first calling `make_section` with `SemanticKind::Section` and an empty content block, using the provided `heading` and `level`. It then checks the optional `output` pointer: if `output` is not null and the trimmed result of `trim_ascii(*output)` is non‑empty, it appends a raw markdown node created by `make_raw_markdown(*output)` to the section’s children. Internally, the control flow is a simple conditional that avoids adding an empty section child. Dependencies include `make_section`, `trim_ascii`, and `make_raw_markdown`, which together ensure the generated section either contains a non‑empty markdown block or remains child‑less.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- heading
- level
- output (if non-null and its content is non-empty after trimming)

#### Usage Patterns

- Constructing a section for a prompt
- Encapsulating a heading with optional output text

### `clore::generate::build_related_page_targets`

Declaration: `generate/render/common.cppm:504`

Definition: `generate/render/common.cppm:504`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation first iterates over each entry in `plan.linked_pages`. For every linked page identifier, it splits on `:` to extract an optional prefix (such as `"module"` or `"namespace"`) and the entity name. Based on the prefix, it attempts to resolve a target path using `links.resolve_module`, `links.resolve_namespace`, or falls back to `links.resolve` if no prefix is present or the prefixed lookup fails. Deduplication is enforced by a `seen` set keyed on the resolved path. For each unique target, the label is derived from `links.resolve_page_title` if the returned title is non‑empty; otherwise the entity name is used as the label, and `strip_inline_markdown` is applied to both candidates. The final `LinkTarget` is constructed via `make_link_target`, using the `current_page_path`, the processed label, and the resolved target path. The collection of targets is returned, providing a compact, deduplicated list of pages that should appear as related links.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- plan`.linked_pages`
- links
- `current_page_path`
- `strip_inline_markdown`

#### Usage Patterns

- Generates navigation links for related pages during page rendering

### `clore::generate::build_string_list`

Declaration: `generate/render/common.cppm:148`

Definition: `generate/render/common.cppm:148`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function iterates over each string in the input parameter `items`. For each element, it calls `trim_ascii` on the string and skips the item if the result is empty. Otherwise, it constructs a `ListItem` whose `fragments` member is populated by `code_spanned_fragments(item_text)` — a helper that likely applies code‑style formatting to text fragments — and then appends the item to a local `BulletList` named `list`. The final list is returned.

Internally, the only dependency besides standard iteration is `trim_ascii` for whitespace detection and `code_spanned_fragments` for converting plain text into styled fragments. No external state or complex control flow is involved; the function simply filters out blank entries and builds a structured bullet list.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `items` parameter (const `std::vector<std::string>`&)

#### Usage Patterns

- Used within page generation to produce bullet lists from collections of strings

### `clore::generate::build_symbol_link_list`

Declaration: `generate/render/common.cppm:360`

Definition: `generate/render/common.cppm:360`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function iterates over each symbol pointer in the input `symbols` vector, constructing a `BulletList` of link items. For each symbol, it first appends a text fragment containing the symbol’s kind name, obtained via `extract::symbol_kind_name`. It then computes a display label: if `use_full_name` is true, it uses `sym->qualified_name`; otherwise it applies `short_name_of` to that qualified name. A lookup is performed through the `links` resolver using the qualified name: if a target path is found, it creates a hyperlink via `make_link` with the display label (falling back to the qualified name if empty) and a relative path computed by `make_relative_link_target` from `current_page_path` and the resolved target; if no target is found, it emits inline code via `make_code` using the same display label. Each constructed `ListItem` is then appended to the output list. The function depends on the `LinkResolver` for symbol-to-page resolution, `short_name_of` for abbreviated labels, and several helper functions for building text, links, and code fragments.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `symbols` vector
- `current_page_path`
- `links` resolver
- `sym->kind`
- `sym->qualified_name`
- `extract::symbol_kind_name`
- `short_name_of`
- `links.resolve`
- `make_link`
- `make_relative_link_target`
- `make_code`
- `make_text`

#### Usage Patterns

- Used during page generation to create symbol index lists
- Supports both full and short display names
- Generates relative links based on current page path

### `clore::generate::build_symbol_source_locations`

Declaration: `generate/render/common.cppm:412`

Definition: `generate/render/common.cppm:412`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::build_symbol_source_locations` constructs a list of Markdown nodes that describe the source locations of a given symbol. It receives a `extract::SymbolInfo` object, a `config::TaskConfig`, a `LinkResolver`, and the `current_page_path` string view. The algorithm consists of two sequential conditionals: if `sym.declaration_location` is known, it calls `push_location_paragraph` with a "Declaration: " label and a link target produced by `make_source_link_target`; if `sym.definition_location` has a value and is known, it repeats the same pattern with a "Definition: " label. The resulting nodes are collected into a vector and returned. Internally, no loops or recursive calls are used; the control flow is a simple linear series of guarded operations. The function depends on `push_location_paragraph` to format the output nodes and on `make_source_link_target` to resolve the file, line, and column information into a clickable `LinkTarget` using the provided `config`, `links`, and `current_page_path`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- sym`.declaration_location`
- sym`.definition_location`
- config
- links
- `current_page_path`

#### Usage Patterns

- Generates source location markdown nodes for symbol documentation
- Called during page rendering to show declaration and definition links

### `clore::generate::collect_implementation_symbols`

Declaration: `generate/render/common.cppm:314`

Definition: `generate/render/common.cppm:314`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function iterates over the keys in `plan.owner_keys`. For each key, it checks the page type: if `plan.page_type` is `PageType::Module`, it retrieves the matching modules via `extract::find_modules_by_name` and collects symbols directly from each module’s symbol list; otherwise, it looks up the corresponding file in `model.files` and collects symbols from that file’s symbol list. In both branches, it uses a `seen` set of `extract::SymbolID` values to avoid duplicates, then validates each candidate symbol with `is_page_level_symbol` and the supplied `predicate` before appending it to the result. After processing all keys, the accumulated `symbols` vector is sorted by `qualified_name` and returned. The algorithm depends on `extract::find_modules_by_name`, `extract::lookup_symbol`, and `is_page_level_symbol` to traverse the project model and filter relevant top‑level symbols.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan`
- `model`
- `predicate`
- `plan.owner_keys`
- `plan.page_type`
- `model.files`
- module symbols via `extract::find_modules_by_name`
- file symbols via `file_it->second.symbols`
- symbol data via `extract::lookup_symbol`

#### Writes To

- local variable `symbols`
- local variable `seen`

#### Usage Patterns

- Called during page generation to gather all implementation symbols for a given page plan.
- Used in conjunction with `is_page_level_symbol` and a custom predicate to filter symbols.

### `clore::generate::collect_namespace_symbols`

Declaration: `generate/render/common.cppm:289`

Definition: `generate/render/common.cppm:289`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The algorithm begins by looking up the given `namespace_name` in the `model.namespaces` map; if the namespace is absent, it returns an empty vector immediately. Otherwise, it iterates over each symbol identifier stored in the namespace entry, using `extract::lookup_symbol` to retrieve the corresponding `extract::SymbolInfo` pointer. For each valid symbol, it applies two filters: `is_page_level_symbol` and the user-supplied `predicate`. Surviving symbol pointers are appended to a local vector. After collection, the vector is sorted in ascending order by the `qualified_name` member of `extract::SymbolInfo` via `std::sort`. Dependencies include `extract::ProjectModel`, `extract::lookup_symbol`, and presumably `is_page_level_symbol` (a helper from the same module). The function ultimately returns a sorted vector of pointers to namespace-level symbols that meet the filtering criteria.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `extract::ProjectModel` parameter `model`
- `std::string_view` parameter `namespace_name`
- `Predicate&&` parameter `predicate`
- `model.namespaces` map
- `ns_it->second.symbols` (set of symbol `IDs`)
- `extract::lookup_symbol`
- `is_page_level_symbol`
- `lhs->qualified_name` and `rhs->qualified_name`

#### Writes To

- local variable `symbols` (`std::vector<const extract::SymbolInfo*>`)

#### Usage Patterns

- called during namespace page generation to gather symbols that should appear on the page
- used with predicates that filter by symbol kind or other criteria

### `clore::generate::doc_label`

Declaration: `generate/render/common.cppm:279`

Definition: `generate/render/common.cppm:279`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::doc_label` implements a straightforward mapping from a `SymbolDocView` enum value to its corresponding human-readable label. The control flow consists of a single `switch` statement that handles the three enumeration members: `SymbolDocView::Declaration` returns the string `"Declaration"`, `SymbolDocView::Implementation` returns `"Implementation"`, and `SymbolDocView::Details` returns `"Details"`. A default fallback also returns `"Details"` for safety. The function depends only on the `SymbolDocView` enum and returns a `std::string_view` literal, avoiding any dynamic allocation or external data lookups.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `SymbolDocView view` parameter

#### Usage Patterns

- used to generate label text for symbol documentation views
- called when rendering section headings

### `clore::generate::find_declaration_page`

Declaration: `generate/render/common.cppm:473`

Definition: `generate/render/common.cppm:473`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::find_declaration_page` attempts to locate a suitable declaration page for a given symbol by performing a two‑step resolution. First, it queries the `links` resolver with the symbol's fully qualified name via `links.resolve(sym.qualified_name)`. If a target path is found and it differs from `current_page_path`, it returns a `LinkTarget` whose label is either `"Declaration"` (when no enclosing namespace exists) or `"Namespace "` concatenated with the namespace name derived from `sym.enclosing_namespace` or `namespace_of`. The label and target are converted into a `LinkTarget` by calling `make_link_target`.

When the direct symbol resolution fails, the function attempts a namespace‑level fallback. It derives the namespace name (again preferring `sym.enclosing_namespace` if present, otherwise calling `namespace_of`). If the namespace is empty, it returns `std::nullopt`. If a namespace target path is obtained via `links.resolve_namespace(ns_name)`, a link to that namespace page is returned. Otherwise, the function returns `std::nullopt`, indicating no declaration page could be found. This algorithm ensures that symbols without dedicated pages still have a chance to be linked to their enclosing namespace page.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `sym.qualified_name`
- `sym.enclosing_namespace`
- `current_page_path`
- `link_resolver` (via calls to `resolve` and `resolve_namespace`)
- `namespace_of()`

#### Usage Patterns

- Creating navigation links to declaration pages
- Cross-referencing symbols in generated documentation

### `clore::generate::find_implementation_pages`

Declaration: `generate/render/common.cppm:433`

Definition: `generate/render/common.cppm:433`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::find_implementation_pages` collects deduplicated link targets that point to the pages where a symbol's implementation and declaration are documented. It accepts a symbol info object, the project model, a link resolver, the current page path, and the project root. Internally, it maintains a `std::unordered_set<std::string>` called `seen` to avoid duplicate entries. A local lambda `try_add` first checks the provided `file_path`; if it is non‑empty and the model contains a module associated with that source file via `extract::find_module_by_source`, the function resolves the module’s name through `links.resolve_module` and pushes a `LinkTarget` labeled `"Module "` plus the module name. If no module is found, it falls back to `links.resolve` using the raw file path, labeling the target with a source‑relative path computed by `make_source_relative`. The function then invokes `try_add` for `sym.definition_location->file` (if present) and for `sym.declaration_location.file`, ensuring that both the definition and declaration location contribute at most one link each. The results are returned as a `std::vector<LinkTarget>`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `sym.definition_location`
- `sym.declaration_location`
- `model`
- `links`
- `current_page_path`
- `project_root`
- `extract::find_module_by_source`

#### Usage Patterns

- Used during page generation to gather link targets for symbol implementation pages
- Provides deduplicated list of module or file links for a symbol's definition and declaration

### `clore::generate::find_module_for_file`

Declaration: `generate/render/common.cppm:496`

Definition: `generate/render/common.cppm:496`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::find_module_for_file` accepts a `const extract::ProjectModel&` and a `std::string_view` representing a file path. It delegates to `extract::find_module_by_source` to determine whether the file is associated with a C++ module. If the returned pointer is non-null, the function returns the value of `mod->name` as a `std::optional<std::string>`. Otherwise it returns `std::nullopt`, indicating no module association was found.

The implementation is a thin wrapper around the extraction layer. It relies solely on the `extract::find_module_by_source` query, which performs the underlying lookup logic. No additional processing, string manipulation, or fallback mechanisms are employed. The function serves as a bridge from the generation layer to the extraction infrastructure for module file resolution.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model` (const `extract::ProjectModel`&)
- `file_path` (`std::string_view`)
- internal state of `extract::find_module_by_source`

#### Usage Patterns

- Querying module association for a file
- Used in page generation to determine file-to-module mapping

### `clore::generate::make_link_target`

Declaration: `generate/render/common.cppm:81`

Definition: `generate/render/common.cppm:81`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::make_link_target` constructs a `LinkTarget` by directly initializing its three fields. It moves the caller-provided `label` into `LinkTarget::label`, computes a relative path from `current_page_path` to `target_page_path` by delegating to `make_relative_link_target` and storing the result in `LinkTarget::target`, and forwards the `code_style` flag to `LinkTarget::code_style`. The function is a straightforward factory that avoids any branching or iteration; its internal control flow consists solely of returning an aggregate-initialized `LinkTarget` using these components. The only external dependency is `make_relative_link_target`, which resolves the relative path between the two page paths.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `current_page_path`
- label
- `target_page_path`
- `code_style`

#### Usage Patterns

- Building navigation links
- Constructing link targets for page generation

### `clore::generate::make_relative_link_target`

Declaration: `generate/render/common.cppm:57`

Definition: `generate/render/common.cppm:57`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function normalizes both the `current_page_path` and `target_page_path` using `std::filesystem::path::lexically_normal`, then computes the relative path from the parent directory of the current path (or `"."` if none) to the target path via `lexically_relative`. If the resulting relative path is empty (i.e., target is the same as the parent), it falls back to returning the target path in generic format; otherwise, it returns the relative path in generic format. The algorithm relies solely on the `std::filesystem` library for path manipulation and does not involve any other internal dependencies.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `current_page_path`
- `target_page_path`

#### Usage Patterns

- computing relative links for page navigation

### `clore::generate::make_source_link_target`

Declaration: `generate/render/common.cppm:383`

Definition: `generate/render/common.cppm:383`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_source_link_target` constructs a `LinkTarget` that represents either a clickable reference to a source file or a plain code-styled label when the source file cannot be resolved within the documentation. It first builds a label by formatting the source location’s file path through `make_source_relative` (relative to the project root) and appending the line number. It then queries the `LinkResolver` object `links` to see if a documentation target path exists for that file. If a target path is found, it delegates to `make_link_target` to produce a full `LinkTarget` with the label, the resolved target path, and `code_style` enabled. Otherwise, it returns a `LinkTarget` with an empty `target` field and `code_style` set to `true`, causing the label to be displayed as plain monospaced text without a hyperlink. This internal control flow ensures consistent formatting for all source-location references while gracefully degrading when no documentation mapping is available.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `location.file`
- `location.line`
- `config.project_root`
- `links.resolve(location.file)`
- `current_page_path`

#### Usage Patterns

- Used to create source link targets for documentation pages
- Called during rendering of page markdown and building link lists

### `clore::generate::namespace_of`

Declaration: `generate/render/common.cppm:53`

Definition: `generate/render/common.cppm:53`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function delegates to `extract::namespace_prefix_from_qualified_name`, passing the input `qualified_name` unmodified. This single call constitutes the entire body; there is no branching, error handling, or string manipulation beyond forwarding. The only dependency is the `extract` module, which must provide a function that isolates the namespace portion of a qualified name according to the project’s name resolution rules. No local state or additional computation is introduced, making the function a thin adapter that centralizes namespace extraction for callers within the `clore::generate` module.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `qualified_name`

#### Usage Patterns

- Extracting namespace from a fully qualified symbol name

### `clore::generate::prompt_output_of`

Declaration: `generate/render/common.cppm:71`

Definition: `generate/render/common.cppm:71`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::prompt_output_of` performs a single map lookup to retrieve a precomputed prompt output string. Internally, it constructs a `PromptRequest` object using the provided `kind` and `target_key` (converted to a `std::string`), then invokes `prompt_request_key` to generate a deterministic key for that request. This key is used to query the `outputs` unordered map; if an entry exists, the function returns a pointer to the corresponding string value, otherwise it returns `nullptr`. The function depends on the `prompt_request_key` function for key formatting and on the `PromptRequest` structure for bundling the lookup parameters.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- outputs
- kind
- `target_key`

#### Usage Patterns

- retrieving cached LLM responses by prompt kind and symbol target key
- checking whether a specific prompt output exists before rendering
- obtaining previously stored prompt results for page generation

### `clore::generate::short_name_of`

Declaration: `generate/render/common.cppm:45`

Definition: `generate/render/common.cppm:45`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::short_name_of` extracts the unqualified name from a fully qualified identifier. It receives a `qualified_name` as a `std::string_view` and delegates to `extract::split_top_level_qualified_name` to decompose the input into its constituent parts. If the resulting `parts` vector is empty, the function returns an empty string. Otherwise, it returns the last element of `parts`, which is the short name. There is no explicit loop or recursion; the algorithm relies entirely on the split utility to handle segmentation and selection is performed by simple vector access. This function has no other internal dependencies within the generation module.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `qualified_name` parameter

#### Usage Patterns

- extracts short name from qualified name

### `clore::generate::strip_inline_markdown`

Declaration: `generate/render/common.cppm:33`

Definition: `generate/render/common.cppm:33`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function iterates over each character in the input `text` and discards any character that matches inline Markdown syntax: backtick, asterisk, underscore, opening or closing bracket, or hash. All other characters are copied into an output string. After the copy, it calls `trim_ascii` on the result to remove leading and trailing ASCII whitespace. The algorithm is linear in the length of the input and avoids allocations beyond the initial `reserve`. It depends only on the helper `trim_ascii` and the character classification logic.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `text`

#### Writes To

- returned `std::string`

#### Usage Patterns

- cleaning markdown for plain text display
- sanitizing evidence text bounds

### `clore::generate::symbol_analysis_markdown_for`

Declaration: `generate/render/common.cppm:161`

Definition: `generate/render/common.cppm:161`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::symbol_analysis_markdown_for` acts as a dispatcher that selects an internal analysis‑rendering function based on the `plan.page_type`. If `plan.page_type` equals `PageType::Namespace`, it delegates to `analysis_overview_markdown`; otherwise it calls `analysis_details_markdown`. Both delegates receive the same `analyses` store and the current symbol `sym`. The chosen sub‑routine is responsible for assembling the final markdown string using a wide range of helpers such as `clore::generate::build_symbol_link_list`, `clore::generate::add_symbol_analysis_sections`, `clore::generate::build_list_section`, and location‑targeting utilities. The function itself is a thin branch point that centralizes the decision between an overview (namespace‑level) and a detailed (per‑symbol) presentation of symbol analysis information.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses` (`SymbolAnalysisStore`)
- `plan` (`PagePlan`)
- `sym` (`extract::SymbolInfo`)

#### Usage Patterns

- used to generate symbol analysis markdown, selecting overview or details based on page type

### `clore::generate::symbol_doc_view_for`

Declaration: `generate/render/common.cppm:269`

Definition: `generate/render/common.cppm:269`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function dispatches on `plan.page_type` to select the appropriate `SymbolDocView`. For `Namespace` pages it returns `Declaration`; for `Module` pages it calls `is_variable_kind(sym.kind)` to distinguish variables (which yield `Details`) from other symbols (which yield `Implementation`); any other page type yields `Details` by default.

The control flow is a simple switch with three branches. Dependencies include the `PagePlan` and `SymbolDocView` enumeration as well as the helper `is_variable_kind` (presumably defined elsewhere in the generation module) that tests whether a symbol represents a variable. No further logic or external data is consulted.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.page_type`
- `sym.kind`

#### Usage Patterns

- Called when building documentation content to select how a symbol is rendered

### `clore::generate::trim_ascii`

Declaration: `generate/render/common.cppm:23`

Definition: `generate/render/common.cppm:23`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The algorithm of `clore::generate::trim_ascii` operates on a `std::string_view` by first stripping leading whitespace: it repeatedly inspects the first character via `text.front()`, casts it to `unsigned char` to avoid undefined behavior with negative `char` values, and checks whether `std::isspace` returns nonzero. If so, it advances the view using `text.remove_prefix(1)`. After the leading pass, it performs an analogous loop for trailing whitespace using `text.back()` and `text.remove_suffix(1)`. Both loops guard against an empty view before each access. The function finally returns the trimmed `std::string_view` by value. Internally, it depends solely on the C++ standard library facilities `std::isspace` (from `<cctype>`) and `std::string_view` (from `<string_view>`), relying on no project‑specific types or complex branching.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `text`

#### Usage Patterns

- used in text processing for markdown generation
- called to normalize whitespace in evidence or content strings

## Internal Structure

The `generate:common` module serves as a shared rendering layer that decomposes documentation generation into small, composable functions and callable objects. It provides a toolkit of reusable building blocks—such as `LinkTarget`, `SymbolDocView`, and utilities like `strip_inline_markdown`, `trim_ascii`, and link-target constructors—that encapsulate common transformation and cross‑reference logic for symbol documentation pages. These primitives are assembled into higher‑level routines like `symbol_analysis_markdown_for`, `build_symbol_source_locations`, and the `collect_*` templated functions, which together form a procedural vocabulary for rendering.

Internally, the module imports from `generate:model` and `generate:markdown`, and relies on `config` and `extract` for configuration and project data. Functions and callable variables (e.g., `push_link_paragraph`, `add_symbol_analysis_sections`) are layered so that lower‑level utilities (string trimming, name resolution) support intermediate list and section builders, which in turn feed the highest‑level page construction functions. This decomposition enforces separation of concerns: each step is independently testable and can be reused across different page‑type generators without duplicating rendering logic.

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)
- [Module generate:markdown](markdown.md)
- [Module generate:model](model.md)

