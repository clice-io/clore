---
title: 'Module generate:markdown'
description: 'The module generate:markdown provides the core data structures and functions for constructing and rendering Markdown documentation from internal semantic representations. It defines a hierarchy of node types (e.g., TextFragment, CodeFragment, CodeFence, BlockQuote, BulletList, SemanticSection) and a MarkdownDocument container with optional Frontmatter. Public functions allow creating individual Markdown elements (make_text, make_code, make_link, etc.) and assembling them into a complete document, as well as rendering the final output. The module encapsulates the logic for formatting inline code, blockquotes, sections with semantic kinds, and other Markdown-specific constructs, decoupling the generation pipeline from the output format.'
layout: doc
template: doc
---

# Module `generate:markdown`

## Summary

The module `generate:markdown` provides the core data structures and functions for constructing and rendering Markdown documentation from internal semantic representations. It defines a hierarchy of node types (e.g., `TextFragment`, `CodeFragment`, `CodeFence`, `BlockQuote`, `BulletList`, `SemanticSection`) and a `MarkdownDocument` container with optional Frontmatter. Public functions allow creating individual Markdown elements (`make_text`, `make_code`, `make_link`, etc.) and assembling them into a complete document, as well as rendering the final output. The module encapsulates the logic for formatting inline code, blockquotes, sections with semantic kinds, and other Markdown-specific constructs, decoupling the generation pipeline from the output format.

## Imports

- `std`

## Imported By

- [`generate:analysis`](analysis.md)
- [`generate:common`](common.md)
- [`generate:page`](page.md)
- [`generate:scheduler`](scheduler.md)
- [`generate:symbol`](symbol.md)

## Types

### `clore::generate::BlockQuote`

Declaration: `generate/markdown.cppm:62`

Definition: `generate/markdown.cppm:62`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::BlockQuote` stores its content in a single data member `fragments`, which is a `std::vector<InlineFragment>`. This vector holds the inline‑level markup elements that constitute the quote’s interior. The implementation imposes no invariants beyond those inherent to `std::vector`: the container may be empty, and the iteration order of `fragments` defines the sequential arrangement of the rendered inline content. No custom constructors, assignment `operator`s, or member functions are defined; `BlockQuote` is a plain aggregate that relies on default member initialization and compiler‑generated special member functions.

#### Invariants

- The `fragments` vector holds the sequence of inline elements within the block quote.
- An empty `fragments` vector represents an empty block quote.

#### Key Members

- `fragments`

#### Usage Patterns

- Constructed with a list of `InlineFragment` objects to define the quote content.
- Iterated over or accessed during markdown output generation to render the block quote.

### `clore::generate::BulletList`

Declaration: `generate/markdown.cppm:49`

Definition: `generate/markdown.cppm:49`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::BulletList` is a simple aggregate that holds a single data member: a `std::vector<ListItem>` named `items`. Its implementation relies entirely on the standard vector container, meaning the internal invariant is that the vector manages a contiguous sequence of `ListItem` objects. No additional member functions are defined; the struct is used as a plain data carrier whose `items` vector is populated externally. The `items` member provides the only storage for the bullet list entries, and any invariants (such as non-emptiness or ordering) must be enforced by the code that constructs or modifies the struct.

### `clore::generate::CodeFence`

Declaration: `generate/markdown.cppm:53`

Definition: `generate/markdown.cppm:53`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `clore::generate::CodeFence` struct serves as a plain data aggregate that bundles a code snippet with an optional language identifier. It contains two `std::string` members: `language` holds the programming language label (e.g., “cpp” or “python”), and `code` stores the actual source text. The struct has no constructors, member functions, or invariants beyond the default guarantees of its embedded strings; all fields are publicly accessible and can be set or read directly. Its primary role is to represent a complete code block for output generation, where the `language` string may be empty to indicate no syntax highlighting. No additional implementation details—such as validation, formatting, or ownership logic—are introduced, making `CodeFence` a minimal, transparent value type.

### `clore::generate::CodeFragment`

Declaration: `generate/markdown.cppm:29`

Definition: `generate/markdown.cppm:29`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::CodeFragment` is a straightforward data-holder consisting of a single public data member `code` of type `std::string`. It serves solely to store the textual content of a generated code fragment. No explicit constructors, destructors, or member functions are defined; the class relies entirely on compiler‑generated default operations for construction, assignment, and destruction. The only invariant is that the `code` member contains a valid `std::string` (which may be empty). The struct performs no validation, transformation, or ownership management on its content, making it a lightweight, pass‑through wrapper whose intended use is to bundle generated text for further composition or emission in the pipeline.

#### Invariants

- The `code` member is a valid `std::string` object.
- No constraints on the content or length of the string are enforced.

#### Key Members

- `code` of type `std::string` stores the code fragment content.

#### Usage Patterns

- Instantiated directly with a string literal or variable containing code.
- Collected into larger objects or sequences for later assembly into complete generated output.

### `clore::generate::Frontmatter`

Declaration: `generate/markdown.cppm:18`

Definition: `generate/markdown.cppm:18`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `clore::generate::Frontmatter` struct serves as a plain data container for YAML frontmatter metadata. Its four string members–`title`, `description`, `layout`, and `page_template`–are intended to capture the document title, a short description, the layout name, and the template name, respectively. `layout` and `page_template` are default-initialised to `"doc"`, ensuring a consistent fallback when no explicit layout or template is provided. No invariants, methods, or custom constructors exist beyond the compiler‑generated defaults; the struct is a simple aggregate that relies on the caller to populate or overwrite fields as needed.

#### Invariants

- `layout` defaults to `"doc"`
- `page_template` defaults to `"doc"`
- all members are `std::string`

#### Key Members

- `title`
- `description`
- `layout`
- `page_template`

#### Usage Patterns

- Populated with frontmatter data before generating markdown pages
- Consumed by functions that produce YAML header blocks in documentation output

### `clore::generate::LinkFragment`

Declaration: `generate/markdown.cppm:33`

Definition: `generate/markdown.cppm:33`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::LinkFragment` is a plain aggregate holding three public fields: `label`, `target`, and `code_style`. The `label` stores the display text of the link, `target` stores the destination URL, and `code_style` is a boolean defaulting to `false` that controls whether the label should be rendered in monospaced code formatting. No invariants are enforced beyond the default member initializer; the struct is intended as a lightweight value type used internally during Markdown generation, allowing callers to specify link content without exposing construction or validation logic.

#### Invariants

- `code_style` defaults to `false`
- No invariants enforced; all members are mutable public strings and bool

#### Key Members

- `label`
- `target`
- `code_style`

#### Usage Patterns

- No explicit usage is described in the evidence; the struct likely serves as input to markdown generation functions where a link fragment with optional code styling is needed.

### `clore::generate::ListItem`

Declaration: `generate/markdown.cppm:45`

Definition: `generate/markdown.cppm:45`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::ListItem` is a simple aggregate that contains a single public member: a `std::vector<InlineFragment>` named `fragments`. This vector is the only data member, and the struct imposes no additional invariants beyond those inherited from `std::vector`; it is typically expected to be non‑empty when representing a meaningful list item, but the type itself enforces no such constraint. No custom constructors, assignment `operator`s, or member functions are provided, so the struct relies entirely on compiler‑generated defaults, making it a lightweight container for the list item’s inline content.

#### Invariants

- The `fragments` vector may be empty or non-empty; no constraint is imposed.
- `ListItem` is a simple aggregate with no special constructors or invariants beyond those of its member types.

#### Key Members

- `fragments` – stores the list item's content as a vector of inline fragments

#### Usage Patterns

- Defined in the `clore::generate` module for markdown generation.
- Likely used as part of a larger list structure (e.g., `ListBlock` or similar) but no evidence of such usage is provided.

### `clore::generate::MarkdownDocument`

Declaration: `generate/markdown.cppm:94`

Definition: `generate/markdown.cppm:94`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::MarkdownDocument` aggregates the entire content of a generated markdown document into two fields. The `frontmatter` member is a `std::optional<Frontmatter>`, capturing optional YAML-style metadata that appears at the top of the document; when it has a value, that metadata logically precedes all body content. The `children` member is a `std::vector<MarkdownNode>` that holds the sequence of content nodes forming the document body. There are no explicitly enforced invariants beyond those inherent to the container types: the order of elements in `children` reflects the rendering order, and `frontmatter`, if present, must be a well-formed `Frontmatter` object. Because `MarkdownDocument` is a plain aggregate with no custom constructors, assignment `operator`s, or member functions, its internal state is directly exposed for construction and manipulation.

#### Invariants

- The `frontmatter` may be absent (`std::nullopt`).
- The `children` vector may be empty, and its elements are stored in document order.

#### Key Members

- `frontmatter`
- `children`

#### Usage Patterns

- Other code populates the fields and then traverses or serializes the structure.

### `clore::generate::MarkdownNode`

Declaration: `generate/markdown.cppm:73`

Definition: `generate/markdown.cppm:73`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::MarkdownNode` is a thin wrapper around a `std::variant` that serves as a tagged union for markdown content. The single data member `value` holds one of seven alternatives: `Paragraph`, `BulletList`, `CodeFence`, `MermaidDiagram`, `BlockQuote`, `RawMarkdown`, or `SemanticSectionPtr`. The key invariant is that exactly one of these types is active at any time, enforced by `std::variant`’s type‑safe discrimination. No custom constructors, assignment `operator`s, or destructors are defined, so the struct relies entirely on the default compiler‑generated special member functions, providing trivial copy, move, and value semantics for the variant. The implementation is minimal: the entire internal structure is just the variant field, and correct usage depends on compile‑time access via `std::visit` or explicit index‑based retrieval.

### `clore::generate::MermaidDiagram`

Declaration: `generate/markdown.cppm:58`

Definition: `generate/markdown.cppm:58`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::MermaidDiagram` is a simple aggregate type defined in the `clore::generate` namespace. It contains a single public data member `code` of type `std::string` that stores the textual representation of a Mermaid diagram. The absence of user‑defined constructors, destructors, or assignment `operator`s means the compiler synthesizes default implementations, making the struct a plain data container. The invariant is that `code` holds a valid Mermaid diagram source, subject to the usual string invariants. This structure is designed for lightweight transfer of diagram text within the generation pipeline.

#### Invariants

- The `code` member is a free-form string; no validity of Mermaid syntax is enforced.

#### Key Members

- `code` of type `std::string`

#### Usage Patterns

- Instances are created with a diagram string and passed to functions that generate output.
- Acts as a straightforward value type for representing Mermaid diagram content.

### `clore::generate::Paragraph`

Declaration: `generate/markdown.cppm:41`

Definition: `generate/markdown.cppm:41`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `Paragraph` struct is a simple aggregate type that holds a single public member, `fragments`, which is a `std::vector<InlineFragment>`. This vector serves as the sole internal storage, maintaining an ordered sequence of inline fragments that together constitute the paragraph’s content. No additional invariants are enforced beyond those inherent to the vector; the fragments are stored contiguously in insertion order, and the struct provides no custom constructors, destructors, or member functions. As a result, `Paragraph` acts as a lightweight, value-semantic wrapper around its underlying fragment list.

#### Invariants

- The fragments are stored in a `std::vector` in the order they were added.
- The vector may be empty, indicating an empty paragraph.
- Each element in the vector is an `InlineFragment`.

#### Key Members

- `fragments` — the vector of inline fragments composing the paragraph.

#### Usage Patterns

- Constructed with a list of `InlineFragment` objects to form a paragraph.
- Iterated over to render or process the paragraph content.
- Likely part of a larger markup or document generation system where paragraphs are assembled from inline elements.

### `clore::generate::RawMarkdown`

Declaration: `generate/markdown.cppm:66`

Definition: `generate/markdown.cppm:66`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::RawMarkdown` is a simple wrapper struct containing a single `std::string` member `markdown`. Its primary purpose is to provide type-level distinction for a raw markdown string, separating it from plain text or other string types within the generation pipeline. The struct imposes no invariants beyond those of `std::string` itself; the `markdown` member may be any valid string, including empty. No constructors, assignment `operator`s, or member functions are defined, so the class relies on default compiler-generated implementations. This minimal design ensures that `RawMarkdown` acts solely as a typed container, enforcing semantic clarity without adding runtime overhead.

#### Invariants

- The `markdown` string may be empty or contain any valid Markdown text.
- No validation is performed on the content of `markdown`.

#### Key Members

- `markdown`: the `std::string` holding the Markdown content.

#### Usage Patterns

- Used as an input or output type in functions that handle Markdown generation.
- Constructed directly from a string literal or variable.
- The member `markdown` is accessed directly for reading or writing.

### `clore::generate::SemanticKind`

Declaration: `generate/markdown.cppm:7`

Definition: `generate/markdown.cppm:7`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The enum `clore::generate::SemanticKind` is defined with an underlying type of `std::uint8_t` to provide a compact, ordered classification of semantic declaration categories during documentation generation. The enumerators — `Index`, `Namespace`, `Module`, `Type`, `Function`, `Variable`, `File`, `Section` — are arranged in a logical hierarchy that mirrors a typical documentation page structure, from broad index and grouping elements down to concrete definitions. This ordering is an internal invariant that supports consistent sorting and grouping of generated content, while the small underlying type ensures minimal storage overhead when `SemanticKind` values are stored in arrays or associative containers.

#### Invariants

- Each enumerator corresponds to a unique semantic category.
- The underlying type is `std::uint8_t`.
- All possible values of `SemanticKind` are explicitly listed as enumerators.

#### Key Members

- `clore::generate::SemanticKind::Type`
- `clore::generate::SemanticKind::Index`
- `clore::generate::SemanticKind::Function`
- `clore::generate::SemanticKind::File`
- `clore::generate::SemanticKind::Module`
- `clore::generate::SemanticKind::Namespace`
- `clore::generate::SemanticKind::Variable`
- `clore::generate::SemanticKind::Section`

#### Usage Patterns

- Used to categorize documentation symbols in the `clore::generate` module.
- Likely used as a discriminator in a variant or as a tag in a switch statement.

#### Member Variables

##### `clore::generate::SemanticKind::File`

Declaration: `generate/markdown.cppm:14`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
File
```

##### `clore::generate::SemanticKind::Function`

Declaration: `generate/markdown.cppm:12`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Function
```

##### `clore::generate::SemanticKind::Index`

Declaration: `generate/markdown.cppm:8`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Index
```

##### `clore::generate::SemanticKind::Module`

Declaration: `generate/markdown.cppm:10`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Module
```

##### `clore::generate::SemanticKind::Namespace`

Declaration: `generate/markdown.cppm:9`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Namespace
```

##### `clore::generate::SemanticKind::Section`

Declaration: `generate/markdown.cppm:15`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Section
```

##### `clore::generate::SemanticKind::Type`

Declaration: `generate/markdown.cppm:11`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Type
```

##### `clore::generate::SemanticKind::Variable`

Declaration: `generate/markdown.cppm:13`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Variable
```

### `clore::generate::SemanticSection`

Declaration: `generate/markdown.cppm:70`

Definition: `generate/markdown.cppm:84`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct aggregates the parameters needed to render a markdown section within a generated document. All fields have well‑defined defaults: `kind` defaults to `SemanticKind::Section`, `level` to `2`, `omit_if_empty` to `true`, and `code_style_heading` to `false`. The `subject_key` holds a lookup key for the documented entity, `heading` provides the section title text, and `children` stores the sequence of `MarkdownNode` elements that form the section’s content. The `omit_if_empty` flag controls whether the section is emitted when `children` is empty; this prevents spurious headers from appearing in the output. No special invariants or dynamic checks are required—the struct serves purely as a data carrier whose fields are set during the generation pipeline before being consumed by markdown serialization logic.

#### Invariants

- Default `level` is 2
- Default `omit_if_empty` is true
- Default `code_style_heading` is false
- `children` vector may be empty
- Default `kind` is `SemanticKind::Section`

#### Key Members

- `kind`
- `heading`
- `level`
- `children`
- `omit_if_empty`
- `subject_key`
- `code_style_heading`

#### Usage Patterns

- Used to represent sections in markdown generation
- Likely aggregated into a hierarchy via the `children` vector
- Defaults allow trivial creation of simple sections without explicit configuration

### `clore::generate::TextFragment`

Declaration: `generate/markdown.cppm:25`

Definition: `generate/markdown.cppm:25`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::TextFragment` is a simple data carrier that holds a single `std::string` member `text`. Its internal structure imposes no additional invariants beyond those inherent to `std::string`; the class is trivially copyable and movable. As a pure aggregate, it relies on default memberwise construction and assignment, serving solely to encapsulate a textual segment for use within the generation infrastructure. No special member implementations are required, and the type is intended to be passed and stored by value.

#### Invariants

- The `text` member is a valid `std::string` object.
- The struct has no user-defined constructors, destructors, or assignment `operator`s.
- All members are public and directly accessible.

#### Key Members

- `text`

#### Usage Patterns

- Instances are created as aggregate initializers or default-constructed.
- Other code populates the `text` member and reads it to obtain the textual content.
- Serves as a building block within the generation system for passing string data.

## Functions

### `clore::generate::code_spanned_fragments`

Declaration: `generate/markdown.cppm:124`

Definition: `generate/markdown.cppm:693`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function delegates to the internal helper `append_code_spanned_fragments`, which populates a `std::vector<InlineFragment>` by scanning the input text for code‑aware inline elements. The helper iterates character‑by‑character, identifying backtick‑delimited code spans via `read_backtick_span` and fenced code blocks through `is_fence_line`. It also recognizes identifier boundaries using `is_identifier_start` and `is_identifier_continue` to conditionally render identifiers in code style. When a code span or fence is detected, the corresponding raw text is appended as a `CodeFragment`; otherwise, text is grouped into `TextFragment` objects via `append_text_fragment`. The helper further handles link spans (`read_link_span`), `operator` tokens (`read_operator_token`), and call/angle suffixes, ensuring that code style runs are correctly isolated and padded. Finally, the public function returns the completed vector of `InlineFragment` instances.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- text parameter

#### Usage Patterns

- Parse markdown text into inline fragments based on code spans

### `clore::generate::code_spanned_markdown`

Declaration: `generate/markdown.cppm:126`

Definition: `generate/markdown.cppm:699`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function iterates over the input `markdown` line by line, maintaining a `in_fence` flag toggled by `is_fence_line`. Inside a fenced block, lines are appended verbatim; outside, the line is passed to `append_code_spanned_line`, which performs code‑span rewriting (e.g., transforming inline code fragments into styled spans). The loop continues until the last line (when no trailing newline exists), building the result in the pre‑reserved `rendered` string.

Dependencies include the free functions `is_fence_line` and `append_code_spanned_line`, both in the `clore::generate::(anonymous namespace)`. The function uses `std::string_view` for the input and `std::string` for the output, with no other external state.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `markdown` parameter

#### Usage Patterns

- used in generation pipeline to format documentation markdown
- called when rendering page content with code span handling

### `clore::generate::make_blockquote`

Declaration: `generate/markdown.cppm:113`

Definition: `generate/markdown.cppm:169`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function constructs a `MarkdownNode` representing a block quote by creating a `BlockQuote` object that contains a single `TextFragment`. It delegates the text content to `make_text`, which converts the provided `std::string` into a `TextFragment` suitable for inline rendering. The resulting `BlockQuote` is then wrapped in a `MarkdownNode` via its `value` field.

No further control flow is required; the implementation is a direct compositional expression. The dependency on `make_text` ensures consistent text handling (e.g., escaping or fragment construction), while the `BlockQuote` struct provides the structural container for the quoted content.

#### Side Effects

- allocates a `MarkdownNode` object

#### Reads From

- parameter `text`

#### Usage Patterns

- Used to create a blockquote Markdown element

### `clore::generate::make_code`

Declaration: `generate/markdown.cppm:101`

Definition: `generate/markdown.cppm:136`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Creates a `CodeFragment` by moving the given `code` string into the `.code` member and returning the resulting `InlineFragment`. This is a direct construction with no branching or iteration. The sole dependency is the `CodeFragment` struct, which must be complete at this call site.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- Parameter `code` of type `std::string`

#### Writes To

- Constructs a `CodeFragment` object (returned by value)

#### Usage Patterns

- Used to create inline code fragments for markdown pages
- Called by other generation functions to produce code snippets

### `clore::generate::make_code_fence`

Declaration: `generate/markdown.cppm:109`

Definition: `generate/markdown.cppm:156`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_code_fence` constructs a `MarkdownNode` representing a fenced code block. It takes two `std::string` parameters, `language` and `code`, and initializes a `CodeFence` struct with both strings moved into the respective fields (`.language` and `.code`). The resulting `CodeFence` is then placed as the `.value` of a `MarkdownNode` using aggregate initialization. The function relies on the `MarkdownNode` variant being able to hold a `CodeFence` instance, and it performs no additional validation or transformation; its sole algorithmic step is the construction of the `CodeFence` and the wrapping `MarkdownNode`. The use of `std::move` ensures efficient transfer of string ownership, avoiding copies.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `language` parameter
- `code` parameter

#### Writes To

- returned `MarkdownNode` object

#### Usage Patterns

- building code fence nodes for Markdown generation
- used in page rendering functions like `render_page_markdown`
- paired with other `MarkdownNode` factories such as `make_blockquote` or `make_mermaid`

### `clore::generate::make_link`

Declaration: `generate/markdown.cppm:103`

Definition: `generate/markdown.cppm:140`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_link` constructs a `LinkFragment` value by moving the provided `label` and `target` strings into the corresponding fields and storing the `code_style` flag. No further processing, branching, or external function calls are involved; the implementation is a direct aggregate initialization. It depends only on the `LinkFragment` struct definition and the types of its three parameters.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `label` parameter
- `target` parameter
- `code_style` parameter

#### Writes To

- returned `InlineFragment`

#### Usage Patterns

- Create link fragments for Markdown rendering
- Used to generate hyperlinks in documentation pages

### `clore::generate::make_mermaid`

Declaration: `generate/markdown.cppm:111`

Definition: `generate/markdown.cppm:165`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_mermaid` is a factory that constructs a `MarkdownNode` representing a Mermaid diagram. It accepts a `std::string` parameter `code` and moves it into the `code` member of a newly created `MermaidDiagram` struct, then wraps that struct in a `MarkdownNode` via aggregate initialization. There is no branching or looping; the sole dependency is the `MermaidDiagram` type, which stores the raw diagram source, and the `MarkdownNode` variant that holds it. The returned node can later be rendered by the `render_markdown` machinery.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `code`

#### Writes To

- returned `MarkdownNode`

#### Usage Patterns

- Wrapping Mermaid diagram code for inclusion in generated Markdown documents

### `clore::generate::make_paragraph`

Declaration: `generate/markdown.cppm:105`

Definition: `generate/markdown.cppm:148`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::make_paragraph` is a direct factory function. It accepts a `std::string` `text`, invokes `make_text` to produce a `TextFragment`, then wraps that fragment inside a `Paragraph` and finally a `MarkdownNode`. No additional processing, validation, or control flow occurs; the function serves as a thin convenience wrapper that composes the lower-level `make_text` with the `Paragraph` and `MarkdownNode` data structures.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `text` parameter (moved from)

#### Usage Patterns

- used to create paragraph nodes when building markdown page content
- likely called by higher-level page generators like `render_page_markdown` or `build_page_root`

### `clore::generate::make_raw_markdown`

Declaration: `generate/markdown.cppm:107`

Definition: `generate/markdown.cppm:152`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::make_raw_markdown` is straightforward: it accepts a `std::string` parameter `markdown`, moves it into a `RawMarkdown` struct's `markdown` field, and wraps that struct in a `MarkdownNode` via aggregate initialization. No further processing, validation, or transformation occurs. The function serves as a direct passthrough, converting an arbitrary Markdown string into a `MarkdownNode` that can later be rendered verbatim without any structural interpretation by the generation pipeline.

Dependencies are limited to the `RawMarkdown` type (which holds the raw text) and the `MarkdownNode` variant type. The function has no conditional logic, loops, or external calls; its entire control flow is a single return statement. This makes it the simplest node‑construction helper in the `clore::generate` API, intended for embedding pre‑formatted or non‑semantic Markdown content.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `markdown` parameter (moved into the node)

#### Usage Patterns

- Called by generation functions to create a `MarkdownNode` holding raw markdown text.
- Used when the markdown content does not require further structure like paragraphs or code blocks.

### `clore::generate::make_section`

Declaration: `generate/markdown.cppm:115`

Definition: `generate/markdown.cppm:173`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_section` is a factory that constructs a `SemanticSectionPtr` (a `std::unique_ptr<SemanticSection>`) by aggregate-initializing a `SemanticSection` with the supplied arguments. Each parameter is directly assigned to the corresponding field of the struct: `kind`, `subject_key`, `heading`, `level`, `omit_if_empty`, `code_style_heading`, and an empty `children` vector. The string parameters `subject_key` and `heading` are moved to avoid unnecessary copies. No additional validation, transformation, or conditional logic is performed; the function simply forwards the input to the newly allocated object and returns it.

#### Side Effects

- Allocates a `SemanticSection` object on the heap
- Transfers ownership of the allocated object via returned `std::unique_ptr`

#### Reads From

- `kind`
- `subject_key`
- `heading`
- `level`
- `omit_if_empty`
- `code_style_heading`

#### Writes To

- Heap memory for a `SemanticSection` object

#### Usage Patterns

- Used to construct a `SemanticSection` for semantic analysis output
- Commonly called by page building functions to create structured sections

### `clore::generate::make_text`

Declaration: `generate/markdown.cppm:99`

Definition: `generate/markdown.cppm:132`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_text` is a factory that constructs an `InlineFragment` from a plain `std::string`. Its implementation is trivial: it moves the input `text` into a `TextFragment` struct and returns it as an `InlineFragment` (likely a variant or base class). No algorithmic logic or branching is involved; the sole purpose is to wrap a raw string into the inline fragment system so it can be composed with other fragments such as code spans or links. The function depends only on the `TextFragment` and `InlineFragment` types, and it serves as a leaf node builder in the fragment hierarchy.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `text` of type `std::string`

#### Usage Patterns

- Called to wrap a plain string into a markdown text node.
- Used in constructing markdown document content from strings.

### `clore::generate::render_markdown`

Declaration: `generate/markdown.cppm:122`

Definition: `generate/markdown.cppm:730`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function begins by checking whether `document.frontmatter` contains a value. If present, it writes a YAML front matter block: it appends `"---\n"`, then a `title:` line using `yaml_quote` on `fm.title`, a `description:` line using `yaml_quote` on `fm.description`, a `layout:` line with `fm.layout`, a `template:` line with `fm.page_template`, and finally a closing `"---\n\n"`. After the front matter, it iterates over each child in `document.children`, calls `render_node` on that child, and appends the returned string to `rendered`. The final concatenated string is returned. The overall flow is linear: conditional front matter generation followed by a loop over document children, relying on `yaml_quote` for safe quoting and on `render_node` to convert each `MarkdownNode` into its text representation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `document.frontmatter` (optional `Frontmatter`)
- `document.frontmatter->title`
- `document.frontmatter->description`
- `document.frontmatter->layout`
- `document.frontmatter->page_template`
- `document.children` (iterable of `MarkdownNode`)
- `render_node(child)` for each child

#### Writes To

- the returned `std::string` (constructed and returned)

#### Usage Patterns

- called to produce the final Markdown output for a document
- used by page rendering or file writing functions

## Internal Structure

The module `generate:markdown` is a self-contained Markdown generation library implemented as a single C++20 module. It imports only the standard library and is decomposed into two layers: a public API and a set of internal helper functions. The public API provides factory functions (e.g., `make_paragraph`, `make_code_fence`, `make_section`, `make_blockquote`) that construct a document tree composed of struct types such as `MarkdownDocument`, `MarkdownNode`, `SemanticSection`, `Paragraph`, `CodeFence`, and fragment types (`TextFragment`, `LinkFragment`, `CodeFragment`). The central rendering entry point is `render_markdown`, which walks the tree and produces final Markdown text.

Internally, the module relies on an anonymous namespace containing numerous helper functions that implement text scanning, inline parsing, code‑span detection, blockquote line prefixing, and whitespace normalization. These helpers operate on integer indices and pointers derived from a character buffer, forming a tightly coupled pipeline that transforms raw source data into the structured document representation. The separation between public constructors and private rendering logic keeps the module’s interface clean while allowing complex text manipulation to be hidden from callers.

