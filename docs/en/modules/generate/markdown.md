---
title: 'Module generate:markdown'
description: 'The generate:markdown module provides the core infrastructure for constructing and rendering structured Markdown documents within the documentation generation pipeline. It defines a hierarchy of node types, including MarkdownDocument, MarkdownNode, and semantic containers such as SemanticSection, Paragraph, CodeFence, BulletList, BlockQuote, and inline fragment types (TextFragment, CodeFragment, LinkFragment). Public factory functions (make_text, make_code, make_link, make_paragraph, make_code_fence, make_blockquote, make_raw_markdown, make_mermaid, make_section) allow callers to assemble complete document models, while the render_markdown function serializes those models into valid Markdown text. The module also handles code span detection and preservation through code_spanned_fragments and code_spanned_markdown, ensuring that inline code in source text is faithfully reproduced or transformed into proper Markdown code spans. Overall, this module owns the public‑facing representation and rendering of generated Markdown content.'
layout: doc
template: doc
---

# Module `generate:markdown`

## Summary

The `generate:markdown` module provides the core infrastructure for constructing and rendering structured Markdown documents within the documentation generation pipeline. It defines a hierarchy of node types, including `MarkdownDocument`, `MarkdownNode`, and semantic containers such as `SemanticSection`, `Paragraph`, `CodeFence`, `BulletList`, `BlockQuote`, and inline fragment types (`TextFragment`, `CodeFragment`, `LinkFragment`). Public factory functions (`make_text`, `make_code`, `make_link`, `make_paragraph`, `make_code_fence`, `make_blockquote`, `make_raw_markdown`, `make_mermaid`, `make_section`) allow callers to assemble complete document models, while the `render_markdown` function serializes those models into valid Markdown text. The module also handles code span detection and preservation through `code_spanned_fragments` and `code_spanned_markdown`, ensuring that inline code in source text is faithfully reproduced or transformed into proper Markdown code spans. Overall, this module owns the public‑facing representation and rendering of generated Markdown content.

## Imported By

- [`generate:analysis`](analysis.md)
- [`generate:common`](common.md)
- [`generate:page`](page.md)
- [`generate:scheduler`](scheduler.md)
- [`generate:symbol`](symbol.md)

## Types

### `clore::generate::BlockQuote`

Declaration: `src/generate/markdown.cppm:73`

Definition: `src/generate/markdown.cppm:73`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The internal structure of `clore::generate::BlockQuote` consists solely of a single member: `fragments`, which is a `std::vector` of `InlineFragment` objects. This vector holds the ordered sequence of inline elements that constitute the content of a block‑quote node in the generated Markdown document. The type is an aggregate, with no user‑defined constructors, destructors, or assignment `operator`s; all members are initialized via default member initializers where applicable, and the vector is default‑constructed to an empty state.

The implementation implicitly relies on the invariants of `std::vector` and `InlineFragment`. There are no custom invariants enforced by the struct itself; the `fragments` vector may be empty, representing an empty block quote, or contain any number of `InlineFragment` elements. The struct’s triviality means that copy, move, and comparison operations are compiler‑generated and simply forward to the corresponding operations of the contained vector. No special member implementations are provided, so correctness depends entirely on the correct manipulation of `fragments` by external code.

#### Key Members

- fragments

#### Usage Patterns

- Used by markdown generation code to structure block quotes
- May be populated with inline fragments representing styled text or inline elements

### `clore::generate::BulletList`

Declaration: `src/generate/markdown.cppm:60`

Definition: `src/generate/markdown.cppm:60`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::BulletList` is a trivial aggregate consisting solely of a single data member: a `std::vector<ListItem>` named `items`. No invariants beyond those guaranteed by the vector itself are enforced; the struct serves as a homogeneous container for a sequence of list items. There are no custom constructors, assignment `operator`s, or member functions, so the type relies entirely on default compiler-generated special members and the standard library’s container semantics. This simplicity makes `BulletList` a transparent value type that can be directly manipulated, moved, and passed as part of larger generation structures.

#### Invariants

- Items are stored in sequence as they appear in the vector
- The vector may be empty, representing an empty bullet list

#### Key Members

- `items`: `std::vector<ListItem>` containing the list entries

#### Usage Patterns

- Populated with `ListItem` objects and passed to rendering functions for markdown output
- Likely constructed by builders or parsers during document generation

### `clore::generate::CodeFence`

Declaration: `src/generate/markdown.cppm:64`

Definition: `src/generate/markdown.cppm:64`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::CodeFence` is a simple data aggregate that stores the content and metadata of a Markdown code fence. It consists of two `std::string` fields: `language` indicates the language identifier (e.g., `"cpp"` or `"python"`) and `code` holds the raw code block text. There are no invariants enforced beyond those of the contained strings; either field may be empty, and the struct itself imposes no validity constraints. The type serves exclusively as a lightweight transfer object, used to carry generated code along with its language tag from generation stages to output formatting. No member functions or constructors beyond the implicitly defined defaults are present, so instantiation and field assignment rely on aggregate initialization.

#### Invariants

- Both `language` and `code` are arbitrary `std::string` objects; no content restrictions are enforced.
- The struct has no user-declared constructors, destructors, or assignment `operator`s, so it is an aggregate type.

#### Key Members

- `language`: the language identifier for the code fence (e.g., "cpp", "python").
- `code`: the actual source code content enclosed in the fence.

#### Usage Patterns

- Direct initialization and member assignment are used to set `language` and `code`.
- Other code accesses the members to read the stored values, likely for serialization into markdown text.

### `clore::generate::CodeFragment`

Declaration: `src/generate/markdown.cppm:40`

Definition: `src/generate/markdown.cppm:40`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::CodeFragment` is a thin aggregate wrapper around a single `std::string` member named `code`. Its purpose is to hold a piece of generated source code text; the struct itself imposes no invariants beyond those already guaranteed by `std::string`. The class is trivially copyable and movable, relying on the compiler‑generated default constructor, copy/move constructors, and copy/move assignment `operator`s. No custom constructors, destructors, or member functions are provided, so the type behaves as a simple value container that can be initialized with any string via aggregate initialization or direct assignment. The only invariant maintained by the struct is that `code` contains a valid UTF‑8 sequence (as required by the underlying `std::string`).

#### Invariants

- No invariants beyond those of `std::string`.

#### Key Members

- `code`

#### Usage Patterns

- Used as a data carrier for code fragments.

### `clore::generate::Frontmatter`

Declaration: `src/generate/markdown.cppm:29`

Definition: `src/generate/markdown.cppm:29`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `clore::generate::Frontmatter` struct is a plain aggregate data container that holds the metadata fields for a Markdown document's frontmatter. Its four `std::string` members—`title`, `description`, `layout`, and `page_template`—are direct copies of the corresponding frontmatter keys. The invariant is that `layout` and `page_template` default to the value `"doc"`, which ensures a sensible fallback if not explicitly overridden. There are no custom constructors, assignment `operator`s, or validation logic; the struct relies on aggregate initialization and member-wise defaults. The default member initializers for `layout` and `page_template` are the only “implementation” beyond trivial field storage, making the type a straightforward passive record.

#### Invariants

- No enforced invariants; all members are freely assignable strings.

#### Key Members

- `title`
- `description`
- `layout`
- `page_template`

#### Usage Patterns

- No usage patterns observed in the provided evidence.

### `clore::generate::InlineFragment`

Declaration: `src/generate/markdown.cppm:50`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The type alias `clore::generate::InlineFragment` is a `std::variant` over `TextFragment`, `CodeFragment`, and `LinkFragment`. This variant provides a type-safe sum type for representing inline elements within a generated document, where each alternative corresponds to a distinct markdown inline construct — plain text, inline code, or a hyperlink. The variant's structure ensures that exactly one fragment type is active at any time, and it is typically used in conjunction with `std::visit` or a similar visitation pattern to dispatch operations such as rendering or serialization based on the active alternative.

As an alias, `InlineFragment` does not introduce any new member implementations; all functionality is inherited from `std::variant`. The key invariant is that the variant is always in a valid state holding exactly one of the three fragment types. This design centralizes the handling of inline content and avoids the need for raw unions or manual type discrimination elsewhere in the generation pipeline.

#### Invariants

- Only one alternative is active at a time

#### Key Members

- `std::variant`
- `TextFragment`
- `CodeFragment`
- `LinkFragment`

#### Usage Patterns

- Used to store and process inline elements in Markdown generation

### `clore::generate::LinkFragment`

Declaration: `src/generate/markdown.cppm:44`

Definition: `src/generate/markdown.cppm:44`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::LinkFragment` is an aggregate data type that bundles the three components of a generated link fragment. Its internal structure consists of two `std::string` members, `label` and `target`, and a `bool` member `code_style` default‑initialized to `false`. As a plain aggregate it has no user‑defined constructors, assignment `operator`s, or destructors; all member initialization follows the default‑initializer for `code_style` and value‑initialization for the two strings. The only invariant is that `code_style` is always `false` unless explicitly set to `true`; the strings may be empty, and no validation or post‑condition is enforced on `label` or `target`. Because the struct has no member functions beyond the implicitly generated ones, its important “implementation” is the trivial aggregate layout that permits direct member access and aggregate initialization.

#### Invariants

- No explicit invariants are documented; the struct is a plain aggregate.
- `code_style` defaults to `false`.
- All members are public and can be directly accessed.

#### Key Members

- `label` - the display text of the link fragment
- `target` - the URL target of the link fragment
- `code_style` - whether the label should be rendered as inline code

#### Usage Patterns

- Not specified in evidence; the struct is only defined in the provided snippet.

### `clore::generate::ListItem`

Declaration: `src/generate/markdown.cppm:56`

Definition: `src/generate/markdown.cppm:56`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::ListItem` aggregates a flat sequence of `InlineFragment` objects stored in the single member `fragments`. The internal structure is a direct container; no additional metadata or linking is maintained. The primary invariant is that the order of elements in `fragments` reflects the linear inline content of the list item, with each fragment representing a contiguous piece of formatted text (e.g., plain, emphasized, or linked). The struct has no custom constructors, destructors, or member functions; all lifecycle and modification rely on `std::vector` operations. There is no invariant requiring a non-empty `fragments`; an empty vector denotes an empty list item. The simplicity of the design supports efficient construction by directly populating the vector during Markdown generation, avoiding additional abstraction layers.

#### Key Members

- fragments

### `clore::generate::MarkdownDocument`

Declaration: `src/generate/markdown.cppm:105`

Definition: `src/generate/markdown.cppm:105`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::MarkdownDocument` serves as a flat container for the generated Markdown content. It holds two data members: an `std::optional<Frontmatter> frontmatter` for an optional YAML front‑matter block, and an `std::vector<MarkdownNode> children` which stores the ordered sequence of top‑level document nodes. The struct has no custom constructors, destructors, or member functions; all invariants are maintained by the default semantics of its members. The `children` vector is expected to contain at least one node in a well‑formed document, but the struct itself does not enforce this—correctness is ensured by the generating algorithm that builds the `MarkdownDocument` instance.

#### Invariants

- `frontmatter` may be `std::nullopt`
- `children` may be empty

#### Key Members

- `frontmatter`
- `children`

#### Usage Patterns

- Used to represent the result of generating a Markdown document
- Can be constructed with or without frontmatter

### `clore::generate::MarkdownNode`

Declaration: `src/generate/markdown.cppm:84`

Definition: `src/generate/markdown.cppm:84`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The internal structure of `clore::generate::MarkdownNode` consists of a single data member, `value`, which is a `std::variant` over six concrete Markdown block types—`Paragraph`, `BulletList`, `CodeFence`, `MermaidDiagram`, `BlockQuote`, `RawMarkdown`—and a `SemanticSectionPtr`. This variant serves as a discriminated union: at any moment exactly one alternative is active, and the kind of node is determined by the variant’s index or through `std::visit`. The struct itself is a simple aggregate with compiler‑generated special member functions, so its object representation is that of the underlying variant. No additional invariants are enforced beyond those provided by the variant itself (no variant is valueless by exception unless an assignment throws). The member `value` is the sole state‑holder, making node construction and assignment a direct operation on the variant.

#### Invariants

- The `value` variant always holds exactly one of the listed types.
- The node does not enforce any additional structural constraints beyond the variant's type safety.

#### Key Members

- `value` of type `std::variant<...>`

#### Usage Patterns

- Used as a building block in a Markdown document model.
- Likely traversed or visited to generate output Markdown text.
- Can be combined with `SemanticSectionPtr` for hierarchical document structure.

### `clore::generate::MermaidDiagram`

Declaration: `src/generate/markdown.cppm:69`

Definition: `src/generate/markdown.cppm:69`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::MermaidDiagram` is a trivial aggregate consisting solely of a single public `std::string` member `code`. Its design purpose is to encapsulate the raw textual representation of a Mermaid diagram that has been generated. As an aggregate, it has no user‑defined constructors, destructors, or member functions, and the only invariant is that `code` may be any string value; the struct itself does not validate the syntax or structure of the diagram source. The simplicity of the implementation makes it a lightweight output container, easily movable or copyable, and directly usable in other contexts that expect a Mermaid diagram string.

#### Invariants

- The `code` string contains the Mermaid diagram source.

#### Key Members

- `code` (`std::string`) – the Mermaid diagram source code.

#### Usage Patterns

- Other code creates instances of `MermaidDiagram` and assigns to the `code` member, or uses it to pass diagram data between components.

### `clore::generate::Paragraph`

Declaration: `src/generate/markdown.cppm:52`

Definition: `src/generate/markdown.cppm:52`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::Paragraph` is implemented as a trivial aggregate containing a single member, `fragments`, of type `std::vector<InlineFragment>`. This vector holds the ordered sequence of inline‑level elements that constitute the paragraph’s content. No invariants are enforced beyond those of the underlying vector: elements appear in linear order, and the paragraph is considered empty when the vector is empty. The struct relies on default member‑wise initialization and compiler‑generated copy/move semantics, with no custom constructors, assignment `operator`s, or other member functions.

### `clore::generate::RawMarkdown`

Declaration: `src/generate/markdown.cppm:77`

Definition: `src/generate/markdown.cppm:77`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::RawMarkdown` is a simple value type that holds the generated Markdown output as a `std::string` in its only data member `markdown`. There are no explicit invariants beyond the normal validity of the underlying string; an empty string is permitted and represents no output. The type provides no special member implementations aside from the compiler-generated defaults, making it a lightweight wrapper intended to convey the result of the Markdown generation process.

#### Invariants

- The `markdown` string can be any valid UTF-8 or ASCII content.

#### Key Members

- `markdown`: the raw markdown string

#### Usage Patterns

- Used as a wrapper to pass or store raw markdown content.

### `clore::generate::SemanticKind`

Declaration: `src/generate/markdown.cppm:18`

Definition: `src/generate/markdown.cppm:18`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The enum `clore::generate::SemanticKind` is implemented as a scoped enumeration with an underlying type of `std::uint8_t` to ensure compact storage, particularly when used in look‑up tables or as keys in generated code. Each enumerator corresponds to a distinct semantic category (e.g., `Type`, `Function`, `Namespace`) and the values are implicitly assigned in declaration order starting from zero. This sequential, zero‑based layout forms an invariant that allows the enumerators to be used directly as array indices or in switch‑based dispatch without requiring explicit integer mappings. The ordering of the enumerators—`Index`, `Namespace`, `Module`, `Type`, `Function`, `Variable`, `File`, `Section`—reflects a natural progression from broad structural scopes (namespace, module) to more concrete elements (type, function, variable) and finally to file‑ and section‑level markers, which aids in consistent iteration and categorization during code generation.

#### Invariants

- Each enumerator value is unique and non-overlapping.
- The enum is used as a discriminator for documentation generation logic.

#### Key Members

- `Index`
- `Namespace`
- `Module`
- `Type`
- `Function`
- `Variable`
- `File`
- `Section`

#### Usage Patterns

- Used in tag dispatch or `switch` statements to select documentation generation behavior.
- Passed as a parameter to functions in the `clore::generate` module.

#### Member Variables

##### `clore::generate::SemanticKind::File`

Declaration: `src/generate/markdown.cppm:25`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
File
```

##### `clore::generate::SemanticKind::Function`

Declaration: `src/generate/markdown.cppm:23`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Function
```

##### `clore::generate::SemanticKind::Index`

Declaration: `src/generate/markdown.cppm:19`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Index
```

##### `clore::generate::SemanticKind::Module`

Declaration: `src/generate/markdown.cppm:21`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Module
```

##### `clore::generate::SemanticKind::Namespace`

Declaration: `src/generate/markdown.cppm:20`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Namespace
```

##### `clore::generate::SemanticKind::Section`

Declaration: `src/generate/markdown.cppm:26`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Section
```

##### `clore::generate::SemanticKind::Type`

Declaration: `src/generate/markdown.cppm:22`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Type
```

##### `clore::generate::SemanticKind::Variable`

Declaration: `src/generate/markdown.cppm:24`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Variable
```

### `clore::generate::SemanticSection`

Declaration: `src/generate/markdown.cppm:81`

Definition: `src/generate/markdown.cppm:95`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::SemanticSection` serves as an internal data container that models one logical section within generated documentation. Its fields capture the section’s hierarchic level (default `2`), a semantic kind tag, an optional subject key for cross‑reference resolution, a heading string, and a flag `code_style_heading` that requests monospaced rendering. The boolean `omit_if_empty` (default `true`) controls whether the section is emitted when its `children` vector is empty, allowing structural placeholders to be suppressed during output. The `children` vector holds the actual `MarkdownNode` content, and together with `level` it defines the section’s depth in the final document tree.

The struct’s invariants are minimal: `level` should be at least 1 to align with markdown heading semantics, and `kind` must be a valid `SemanticKind` enumerator. `subject_key` may be empty when no topic is associated. No explicit construction logic is provided beyond default member initializers, so consumers are responsible for populating fields consistently. The section’s role is purely that of a passive data record, read by downstream generation passes to produce the final markdown output.

#### Invariants

- All fields have default values that serve as sensible starting points.
- The `children` vector is initially empty unless populated.

#### Key Members

- `kind` – discriminates the semantic role of the section.
- `heading` – the section's headline text.
- `children` – nested child nodes for hierarchical structure.
- `level` – the heading level (e.g., `<h2>` for 2).
- `omit_if_empty` – controls whether an empty section is dropped.
- `code_style_heading` – indicates if the heading should be rendered as code.

#### Usage Patterns

- Created as a simple value type and populated with field-assignment syntax.
- Passed to markdown generation routines that iterate `children` and use `kind`, `heading`, `level`, etc., to emit formatted output.
- Used in conjunction with `MarkdownNode` to build document trees.

### `clore::generate::SemanticSectionPtr`

Declaration: `src/generate/markdown.cppm:82`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The type alias `clore::generate::SemanticSectionPtr` is defined as `std::unique_ptr<SemanticSection>`, establishing exclusive ownership and automatic lifetime management for `SemanticSection` objects within the `generate` module. This alias ensures that a `SemanticSectionPtr` cannot be copied—only moved—which enforces a single owner at any point and prevents accidental double-deletion. Internally, the alias relies on the default deleter of `std::unique_ptr`; no custom deleter or additional state is provided. The primary invariance introduced by this alias is the transfer of ownership via move semantics: functions or containers that accept or store `SemanticSectionPtr` values assume responsibility for the pointed-to object's lifetime. There is no invariant that the pointer is never null; null `SemanticSectionPtr` values are valid and may represent an absent or optional section.

#### Invariants

- Exclusive ownership of a single `SemanticSection` object
- Can be null if uninitialized or moved from

#### Key Members

- Underlying `std::unique_ptr` interface (e.g., `get`, `reset`, `operator*`, `operator->`)

#### Usage Patterns

- Used to manage the lifetime of `SemanticSection` objects
- Passed by value to transfer ownership
- Stored in containers or as class members

### `clore::generate::TextFragment`

Declaration: `src/generate/markdown.cppm:36`

Definition: `src/generate/markdown.cppm:36`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::TextFragment` holds a single data member, `text`, of type `std::string`. Its internal structure is trivial, consisting only of this string; no invariants are enforced beyond those inherent to `std::string`. The implementation is minimal and relies entirely on the default lifecycle and assignment semantics of `std::string` — the fragment does not introduce any additional constraints, validation, or derived state. As a result, constructing, copying, moving, or destroying a `TextFragment` amounts to the corresponding operations on its `text` member.

#### Key Members

- text

## Functions

### `clore::generate::code_spanned_fragments`

Declaration: `src/generate/markdown.cppm:135`

Definition: `src/generate/markdown.cppm:704`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Implementation: [Implementation](functions/code-spanned-fragments.md)

The function `clore::generate::code_spanned_fragments` is a thin wrapper that constructs a `std::vector<InlineFragment>` and passes the input string to the internal helper `append_code_spanned_fragments`. This helper, defined in an anonymous namespace, performs the actual line‑by‑line scanning and splitting of the markdown text into code‑spanned fragments by identifying backtick‑delimited spans, inline code candidates, and other syntactic patterns. By separating the allocation from the parsing logic, `code_spanned_fragments` provides a clean public interface while the core algorithm remains reusable across different call sites.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- input parameter `text` of type `std::string_view`

#### Writes To

- returns a newly allocated `std::vector<InlineFragment>`

#### Usage Patterns

- Used by `clore::generate::(anonymous namespace)::append_rendered_text` to break up text into fragments for rendering

### `clore::generate::code_spanned_markdown`

Declaration: `src/generate/markdown.cppm:137`

Definition: `src/generate/markdown.cppm:710`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function iterates line‑by‑line through the input markdown, maintaining an `in_fence` flag that is toggled whenever the helper `is_fence_line` identifies a fenced code block boundary. Inside a fence, each line is appended unchanged to the output; outside a fence, each line is processed by `append_code_spanned_line`, which marks up code‑like spans (such as identifiers and `operator`s) with the target formatting. The algorithm preserves original line breaks and stops when no further characters remain. Its sole dependencies are the two namespace‑internal helpers `is_fence_line` and `append_code_spanned_line`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the input `markdown` parameter

### `clore::generate::make_blockquote`

Declaration: `src/generate/markdown.cppm:124`

Definition: `src/generate/markdown.cppm:180`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_blockquote` constructs a `MarkdownNode` representing a blockquote. It takes a `std::string` parameter `text` and, in a single expression, moves that string into a call to `make_text` to produce an `InlineFragment`. That fragment is then directly placed into the `fragments` member of a newly created `BlockQuote` struct, which is in turn stored as the `value` of the returned `MarkdownNode`. The implementation relies solely on the `make_text` helper to transform the plain input into the required inline fragment representation; there is no additional parsing or conditional logic.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `text`

#### Usage Patterns

- Building blockquote elements for documentation.
- Creating markdown nodes for text quotations.

### `clore::generate::make_code`

Declaration: `src/generate/markdown.cppm:112`

Definition: `src/generate/markdown.cppm:147`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Implementation: [Implementation](functions/make-code.md)

The function `clore::generate::make_code` constructs and returns an `InlineFragment` representing a code span. Its implementation directly creates a `CodeFragment` by moving the input `code` string into the `CodeFragment::code` member, then returns that value as the polymorphic `InlineFragment` type. No additional parsing, validation, or transformation is performed; the algorithm is a straightforward constructor call. Dependencies are limited to the `CodeFragment` struct (defined in the same module) and the type alias `InlineFragment`, which is a `std::variant` over several fragment types. This function is invoked during the inline-level code span detection phase when the parser encounters backtick-enclosed code in the Markdown input, providing the foundational representation for code spans that later get rendered by `render_inline`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `code` (via move)

#### Writes To

- returned `InlineFragment` (constructed `CodeFragment`)

#### Usage Patterns

- creating inline code fragments in `append_existing_code_span`
- creating inline code fragments in `append_code_aware_text`
- wrapping a string into a code span fragment

### `clore::generate::make_code_fence`

Declaration: `src/generate/markdown.cppm:120`

Definition: `src/generate/markdown.cppm:167`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::make_code_fence` is a straightforward factory function. It constructs a `CodeFence` instance by moving the `language` and `code` arguments into the corresponding fields of the struct, then returns that `CodeFence` object wrapped as the `value` of a newly created `MarkdownNode`. No branching, iteration, or external helper calls occur; the entire operation reduces to a single struct initialization and a return statement.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `language` parameter
- `code` parameter

#### Writes To

- returned `MarkdownNode`

#### Usage Patterns

- building markdown code blocks
- generating fenced code regions for documentation output

### `clore::generate::make_link`

Declaration: `src/generate/markdown.cppm:114`

Definition: `src/generate/markdown.cppm:151`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_link` is a factory that constructs an `InlineFragment` representing a hyperlink. Internally, it directly initializes a `LinkFragment` by moving the `label` and `target` string parameters into the corresponding fields, and stores the boolean `code_style` flag. No validation, transformation, or conditional branching occurs—the implementation is a single aggregate initialization. The function depends only on the `LinkFragment` struct and its public fields `label`, `target`, and `code_style`; it is typically used as a building block for higher-level markdown generation routines that assemble paragraph or list fragments.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `label` parameter
- `target` parameter
- `code_style` parameter

#### Usage Patterns

- Used to create inline link fragments for markdown rendering

### `clore::generate::make_mermaid`

Declaration: `src/generate/markdown.cppm:122`

Definition: `src/generate/markdown.cppm:176`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_mermaid` constructs a `MarkdownNode` representing a Mermaid diagram. Internally, it moves the provided `code` string into a `MermaidDiagram` struct (filling its `code` field) and then returns a `MarkdownNode` that holds that struct as its value. No further processing or validation is performed; the function serves as a simple factory that packages the raw diagram source into the `MarkdownNode` variant type used elsewhere in the generator pipeline.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `std::string` code parameter

#### Writes To

- returned `MarkdownNode` containing `MermaidDiagram`

#### Usage Patterns

- creating Mermaid diagram nodes for markdown generation

### `clore::generate::make_paragraph`

Declaration: `src/generate/markdown.cppm:116`

Definition: `src/generate/markdown.cppm:159`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_paragraph` constructs a `MarkdownNode` containing a single `Paragraph` whose `fragments` vector holds exactly one `InlineFragment`. This fragment is produced by calling `clore::generate::make_text` on the input string after moving it. The implementation is a direct return of the aggregate-initialized `MarkdownNode`, relying solely on `make_text` to convert the raw text into an inline fragment. No branching, iteration, or additional preprocessing occurs; the entire text is treated as a single paragraph with no internal structure.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `text` parameter

#### Writes To

- the returned `MarkdownNode` object

#### Usage Patterns

- creating paragraph nodes from plain text
- building markdown paragraph content

### `clore::generate::make_raw_markdown`

Declaration: `src/generate/markdown.cppm:118`

Definition: `src/generate/markdown.cppm:163`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_raw_markdown` serves as a factory that wraps an arbitrary markdown string into a `MarkdownNode`. It constructs a `RawMarkdown` object by moving the input `std::string` into its `markdown` field, then returns a `MarkdownNode` that holds that `RawMarkdown`. The implementation involves no parsing or transformation; it is purely a type‑wrapping step. The dependency is limited to the `MarkdownNode` and `RawMarkdown` types, which are defined in the same module. This function is used when a caller already has verbatim markdown content that should be stored as‑is for later rendering (e.g., via `render_markdown`), bypassing any additional structural analysis.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `markdown`

#### Writes To

- returned `MarkdownNode`

#### Usage Patterns

- creating a markdown node from raw string
- as a building block for more complex markdown generation

### `clore::generate::make_section`

Declaration: `src/generate/markdown.cppm:126`

Definition: `src/generate/markdown.cppm:184`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::make_section` is a straightforward factory function. It constructs a `SemanticSection` object using a designated initializer list, directly assigning each parameter—`kind`, `subject_key`, `heading`, `level`, `omit_if_empty`, and `code_style_heading`—to its corresponding field. The `children` field is initialized as an empty vector. The function then wraps the constructed object in a `SemanticSectionPtr` via `std::make_unique` and returns it. There is no branching, iteration, or delegation to helper routines; the entire operation is a single allocation and memberwise copy or move of the provided arguments. The primary dependency is the `SemanticSection` struct definition, specifically its field layout, and the `SemanticSectionPtr` type alias.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameters `kind`, `subject_key`, `heading`, `level`, `omit_if_empty`, `code_style_heading`

#### Usage Patterns

- called to create section nodes for documentation page building
- used within page rendering logic to encapsulate semantic content

### `clore::generate::make_text`

Declaration: `src/generate/markdown.cppm:110`

Definition: `src/generate/markdown.cppm:143`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Implementation: [Implementation](functions/make-text.md)

The function `clore::generate::make_text` serves as a factory that wraps a plain string into the `InlineFragment` variant by constructing a `TextFragment` directly. It accepts a `std::string`, moves it into the `TextFragment::text` field, and returns the resulting `InlineFragment` object. The implementation is a single-expression return, with no branching, iteration, or conditional logic; it relies solely on the definitions of `TextFragment` and the `InlineFragment` type alias. No other functions or external dependencies are involved.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `text` parameter (moved from)

#### Writes To

- returned `InlineFragment` containing a `TextFragment`

#### Usage Patterns

- used to embed plain text in inline fragments
- called by `make_paragraph`, `make_blockquote`, and `append_text_fragment`

### `clore::generate::render_markdown`

Declaration: `src/generate/markdown.cppm:133`

Definition: `src/generate/markdown.cppm:741`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::render_markdown` builds a Markdown string by first handling an optional `Frontmatter` block: it writes the YAML delimiters and the fields `title`, `description`, `layout`, and `page_template`, using the helper `yaml_quote` to safely quote string values. After the frontmatter, it iterates over each `MarkdownNode` in `document.children` and passes it to the internal function `render_node`, which recurses through the node’s content to produce the final text. The overall control flow is a sequential transformation of the structured `MarkdownDocument` into a flat string, relying on a set of anonymous‑namespace helpers for inline rendering, code‑span detection, section handling, and node‑specific output.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- document`.frontmatter`
- document`.children`

#### Usage Patterns

- Used to convert a `MarkdownDocument` into a string for writing to a file
- Called during page generation to produce Markdown output

## Internal Structure

The `generate:markdown` module is decomposed into three internal layers: a set of value-type data structures representing document components (`MarkdownDocument`, `MarkdownNode`, `SemanticSection`, `InlineFragment` variants, and block-level containers such as `Paragraph`, `CodeFence`, `BulletList`, `BlockQuote`); a collection of factory functions (`make_text`, `make_code`, `make_link`, `make_paragraph`, `make_code_fence`, `make_mermaid`, `make_blockquote`, `make_section`) that construct these types; and a rendering layer (`render_markdown`, `render_node`, `render_section`, `render_inline`) that walks the composed tree and produces a Markdown string. The module imports only standard library types (e.g., `std::string`, `std::vector`, `std::variant`, `std::unique_ptr`) and a minimal internal `Frontmatter` structure, keeping dependencies narrow.

Beneath these layers, the implementation structure includes a set of private helper functions that handle low-level Markdown scanning and transformation. Utilities such as `code_spanned_fragments`, `read_backtick_span`, `read_link_span`, `read_code_candidate`, and `append_code_aware_text` operate directly on raw string views to decompose inline content into code spans, links, and plain text. These helpers are used by both the factory functions and the rendering layer, ensuring that code-span preservation and link detection are handled consistently. The overall architecture follows a clear separation of concerns: data types define the document model, factory functions provide a builder API, rendering functions serialize the model, and the low-level string helpers manage text-level parsing without depending on the higher document abstractions.

