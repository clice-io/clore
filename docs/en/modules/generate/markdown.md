---
title: 'Module generate:markdown'
description: 'The generate:markdown module is responsible for constructing and rendering structured Markdown documents. It provides a set of public data types—such as MarkdownDocument, MarkdownNode, Paragraph, CodeFence, BlockQuote, BulletList, MermaidDiagram, SemanticSection, and various fragment types (e.g., TextFragment, CodeFragment, LinkFragment)—that form an abstract syntax tree (AST) for generated documentation. The module owns the implementation of these node types and the functions that build them, including make_text, make_code, make_blockquote, make_paragraph, make_section, make_code_fence, make_link, make_mermaid, and make_raw_markdown.'
layout: doc
template: doc
---

# Module `generate:markdown`

## Summary

The `generate:markdown` module is responsible for constructing and rendering structured Markdown documents. It provides a set of public data types—such as `MarkdownDocument`, `MarkdownNode`, `Paragraph`, `CodeFence`, `BlockQuote`, `BulletList`, `MermaidDiagram`, `SemanticSection`, and various fragment types (e.g., `TextFragment`, `CodeFragment`, `LinkFragment`)—that form an abstract syntax tree (AST) for generated documentation. The module owns the implementation of these node types and the functions that build them, including `make_text`, `make_code`, `make_blockquote`, `make_paragraph`, `make_section`, `make_code_fence`, `make_link`, `make_mermaid`, and `make_raw_markdown`. 

The module also owns the rendering pipeline through functions like `render_markdown` and internal helpers that traverse the AST and produce final Markdown output, handling inline styling, code spans, fenced blocks, headings, blockquotes, lists, and YAML frontmatter. Its public API enables callers to programmatically assemble documentation content from source analysis data, with the module managing the conversion to Markdown syntax and the composition of document sections.

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

The struct `clore::generate::BlockQuote` is implemented as a plain aggregate with a single member, `fragments`, of type `std::vector<InlineFragment>`. This vector stores the ordered sequence of inline fragments that constitute the content of a block quote during the generation phase. There are no custom constructors, assignment `operator`s, or additional member functions; the struct relies entirely on default compiler‑generated special member functions for initialization, copying, and moving. The only internal invariant is that `fragments` holds the fragments in the exact order they are intended to appear in the rendered output, and the vector may be empty if the block quote has no content. This straightforward structure makes `clore::generate::BlockQuote` a lightweight data holder with no hidden state or complex lifecycle.

### `clore::generate::BulletList`

Declaration: `generate/markdown.cppm:49`

Definition: `generate/markdown.cppm:49`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::BulletList` is a simple aggregate that stores the content of a bullet list as a single `std::vector<ListItem>` member named `items`. This vector is the sole internal state and holds the individual list entries in order. As an aggregate, it has no user‑defined constructors, destructors, or member functions; initialization and modification rely on direct member access and standard vector operations. The only invariant is that `items` contains the sequence of `ListItem` objects that represent the list’s body, with no implicit constraints on its length or the validity of its elements beyond those enforced by `ListItem` itself.

#### Invariants

- `items` contains all list items in order.

#### Key Members

- `items` : `std::vector<ListItem>`

#### Usage Patterns

- Used as a data container for generating markdown bullet lists.

### `clore::generate::CodeFence`

Declaration: `generate/markdown.cppm:53`

Definition: `generate/markdown.cppm:53`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::CodeFence` is a plain data aggregate that holds the two string members `language` and `code`. Its sole design purpose is to represent the content and metadata of a Markdown fenced code block. As a struct with no custom constructors, special member functions, or invariants, the object is a transparent container whose lifetime and ownership of its `std::string` members follow standard value semantics. No member function implementations exist beyond the implicitly generated ones.

#### Invariants

- `language` and `code` are independent strings with no inherent constraints.

#### Key Members

- `language`
- `code`

#### Usage Patterns

- Created and populated with language and code content, then passed to other generation functions to produce Markdown fences.

### `clore::generate::CodeFragment`

Declaration: `generate/markdown.cppm:29`

Definition: `generate/markdown.cppm:29`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::CodeFragment` is a lightweight value type that holds a single `std::string` member `code`. It serves as an opaque container for a snippet of generated code, intended to be passed or combined within the generation pipeline. No special invariants are imposed on the stored string—it may be empty or contain arbitrary text—and the struct relies entirely on default compiler-generated constructors, assignment `operator`s, and destructor, making it trivially copyable and movable. This simplicity allows `CodeFragment` to be used as a fundamental building block in code generation algorithms where fragments are accumulated, transformed, and concatenated.

#### Invariants

- The struct is a simple container with no explicit invariants.

#### Key Members

- The member `code` is the sole data member.

#### Usage Patterns

- Used to store and pass code fragments within the code generation process.

### `clore::generate::Frontmatter`

Declaration: `generate/markdown.cppm:18`

Definition: `generate/markdown.cppm:18`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::Frontmatter` is a plain aggregate with four `std::string` members: `title`, `description`, `layout`, and `page_template`. The latter two are given default values of `"doc"`, establishing a common expectation that, unless overridden, the generated frontmatter will reference the `doc` layout and template. No custom constructors, assignment `operator`s, or invariants are enforced beyond those inherent to `std::string`; the struct simply bundles the data fields that are serialized into YAML frontmatter during Markdown generation.

#### Invariants

- Fields `layout` and `page_template` default to `"doc"` if not explicitly set.

#### Key Members

- `title`
- `description`
- `layout`
- `page_template`

#### Usage Patterns

- Populated with page metadata before generating frontmatter in markdown output.
- Defaults for `layout` and `page_template` are commonly used for standard documentation pages.

### `clore::generate::LinkFragment`

Declaration: `generate/markdown.cppm:33`

Definition: `generate/markdown.cppm:33`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::LinkFragment` is an aggregate with three public data members: `label`, `target`, and `code_style`. It serves as a lightweight container for the components of a generated Markdown link fragment. A default member initializer sets `code_style` to `false`. No invariants are imposed beyond the plain text requirements of its `std::string` fields; the struct is intended for direct field access during link construction and requires no special member functions.

#### Invariants

- `label` and `target` are arbitrary strings with no specified format constraints.
- `code_style` defaults to `false` and is used as a boolean flag.

#### Key Members

- `label`: the display text for the link.
- `target`: the URL or reference target.
- `code_style`: indicates whether the label should be rendered in code style.

#### Usage Patterns

- Constructed with designated initializers or aggregate initialization.
- Passed to markdown generation functions to produce link syntax.

### `clore::generate::ListItem`

Declaration: `generate/markdown.cppm:45`

Definition: `generate/markdown.cppm:45`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::ListItem` is implemented as a plain aggregate with a single `std::vector<InlineFragment>` member named `fragments`. This design directly reflects its role as a holder of inline content for a list item within the generation process. The implementation imposes no additional invariants; the vector is expected to contain the ordered sequence of `InlineFragment` objects that constitute the item's text. Because `ListItem` contains no custom constructors, assignment `operator`s, or destructors, it relies entirely on the compiler-generated defaults, making it a straightforward value type that can be efficiently moved and copied. No member functions are provided beyond those implicitly defined, so all interaction with the list item's content occurs through direct access to `fragments`.

#### Key Members

- fragments

#### Usage Patterns

- Used to model individual items in a markdown list, where each item is composed of inline fragments.

### `clore::generate::MarkdownDocument`

Declaration: `generate/markdown.cppm:94`

Definition: `generate/markdown.cppm:94`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::MarkdownDocument` aggregates the top-level parsed representation of a Markdown document. Its internal structure consists of two data members: an `std::optional<Frontmatter> frontmatter` that may hold document metadata, and an `std::vector<MarkdownNode> children` that stores the sequence of top-level nodes. No custom constructors, assignment `operator`s, or other member functions are defined, so initialization, copying, and moving rely on compiler-generated defaults. The only implicit invariant is that when `frontmatter` has a value, it logically precedes the `children` sequence; no enforcement of ordering or consistency is provided by the struct itself.

#### Invariants

- frontmatter may be absent
- children elements retain document order

#### Key Members

- frontmatter
- children

#### Usage Patterns

- Acts as the output data structure for document parsing
- Consumed by rendering or further processing

### `clore::generate::MarkdownNode`

Declaration: `generate/markdown.cppm:73`

Definition: `generate/markdown.cppm:73`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::MarkdownNode` is a thin wrapper around a single `std::variant` member `value`, which serves as a discriminated union of all concrete block-level Markdown element types used in the generation process. Each instance holds exactly one of the seven alternatives: `Paragraph`, `BulletList`, `CodeFence`, `MermaidDiagram`, `BlockQuote`, `RawMarkdown`, or `SemanticSectionPtr`. The `SemanticSectionPtr` type is a pointer to another semantic section, enabling recursive or hierarchical node composition. Because the struct has no other data members and no user‑defined special member functions (likely relying on defaults), the variant’s type‑safe visitation pattern is the primary implementation concern: consumers must use `std::visit` or similar to operate on the node’s content. No invariants beyond those enforced by the variant itself are present, making the struct a simple, efficient building block for the Markdown generation graph.

#### Invariants

- The variant `value` always holds exactly one of the listed alternatives.
- The type of the contained element is known at compile time via the variant index.

#### Key Members

- `value`

#### Usage Patterns

- Constructed with a specific markdown element type to initialize the variant.
- Visited using `std::visit` to process different markdown constructs uniformly.
- Stored in containers to build a markdown document tree.

### `clore::generate::MermaidDiagram`

Declaration: `generate/markdown.cppm:58`

Definition: `generate/markdown.cppm:58`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::MermaidDiagram` is a trivial aggregate type that serves as a simple container for the textual representation of a generated Mermaid diagram. Its only member, `code` of type `std::string`, holds the complete Mermaid source string. The struct imposes no invariants on the content of this string—syntactic or semantic validity of the diagram is not enforced at this level and is instead delegated to the generating logic and the downstream rendering toolchain. As a plain aggregate, `clore::generate::MermaidDiagram` is default-constructible, copyable, and movable, and it functions purely as a value carrier within the code generation flow.

#### Invariants

- The `code` member stores the diagram source text.

#### Key Members

- `code` member of type `std::string`

#### Usage Patterns

- Constructed with or assigned a Mermaid diagram string.
- Accessed to retrieve the diagram code for rendering or output.

### `clore::generate::Paragraph`

Declaration: `generate/markdown.cppm:41`

Definition: `generate/markdown.cppm:41`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::Paragraph` is a simple aggregate whose sole data member is `fragments`, a `std::vector<InlineFragment>`. There are no user‑defined constructors, assignment `operator`s, or destructors; all special member functions are implicitly generated. The invariant of the struct is that the sequence of `InlineFragment` objects stored in `fragments` represents the logical content of a paragraph in the order they appear. Because the struct is an aggregate, it can be initialized using brace‑initialization. No additional invariants are enforced by the struct itself beyond those already guaranteed by the vector (e.g., valid iterators, contiguous storage). The absence of custom member implementations means the struct relies entirely on the default copy, move, and destruction semantics of its vector member.

#### Invariants

- fragments are stored in order

#### Key Members

- fragments

#### Usage Patterns

- aggregated in lists or documents
- populated with inline content during generation

### `clore::generate::RawMarkdown`

Declaration: `generate/markdown.cppm:66`

Definition: `generate/markdown.cppm:66`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::RawMarkdown` is a trivial wrapper around a single public member `std::string markdown`. Its sole purpose is to type‑tag a plain string as raw Markdown content, distinguishing it from other string types within the generation pipeline. No invariants are enforced beyond those inherent to `std::string`; the struct contains no constructors, assignment `operator`s, or member functions. All operations on the stored text are performed externally by functions that accept or return a `RawMarkdown` value.

#### Invariants

- No invariants are enforced; the struct is a plain aggregate.

#### Key Members

- `markdown` of type `std::string`

#### Usage Patterns

- Used to represent and pass raw markdown content.

### `clore::generate::SemanticKind`

Declaration: `generate/markdown.cppm:7`

Definition: `generate/markdown.cppm:7`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `clore::generate::SemanticKind` enumeration is an internal discriminator used to identify the kind of semantic entity being processed during documentation generation. It is implemented as an `enum class` with an underlying `std::uint8_t` storage, ensuring a compact and type‑safe representation. Each enumerator — `Index`, `Namespace`, `Module`, `Type`, `Function`, `Variable`, `File`, and `Section` — corresponds to a distinct category of documented elements, such as types, functions, or file‑level groupings. The ordering of enumerators follows the declaration order and does not imply any inherent hierarchy or priority; the values are assigned automatically by the compiler starting from zero. The enum’s integral representation allows efficient mapping to other internal data structures (e.g., lookup tables or switch‑based dispatch) while preventing accidental mixing with plain integer or unrelated enumeration types.

#### Invariants

- Each enumerator has a distinct integral value.
- Underlying type is `std::uint8_t`.

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

- No direct usage patterns observed in the provided evidence.

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

`clore::generate::SemanticSection` is a plain aggregate that holds the internal state for a semantically-typed documentation section. Its data members include an enumeration `kind` (defaulting to `SemanticKind::Section`), two string fields `subject_key` and `heading`, a `level` of type `std::uint8_t` (defaulting to 2), two boolean flags `omit_if_empty` and `code_style_heading`, and a `std::vector<MarkdownNode>` named `children`. The invariants are established entirely by these defaults: a new instance represents a second-level section that will be omitted from output when its content is empty unless overridden. The `children` vector stores the concrete Markdown content; the `kind` discriminator and `subject_key` identify which semantic category or generated entity this section corresponds to, while `heading` and `code_style_heading` control the textual label used. No custom constructors or member functions exist, so all initialization relies on default member initializers and aggregate initialization.

#### Invariants

- `level` is a non-zero unsigned integer suggesting heading depth
- `omit_if_empty` determines if a section with no children is skipped
- `kind` identifies the semantic type of the section
- `subject_key` is a key to associate with an entity
- `code_style_heading` toggles monospaced heading formatting

#### Key Members

- `kind`
- `subject_key`
- `heading`
- `level`
- `omit_if_empty`
- `code_style_heading`
- `children`

#### Usage Patterns

- Instantiated as part of markdown generation from code
- Populated with headings and child nodes
- Passed to a renderer that outputs markdown

### `clore::generate::TextFragment`

Declaration: `generate/markdown.cppm:25`

Definition: `generate/markdown.cppm:25`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `clore::generate::TextFragment` struct is a plain aggregate data carrier consisting of a single public member `std::string text`. No user‑defined constructors, destructors, or assignment `operator`s are declared; the type relies entirely on compiler‑generated special member functions. It provides no invariants beyond those inherent to `std::string` itself – the `text` member may hold any valid string, including an empty one. Internally, the struct serves as a lightweight wrapper to represent a contiguous piece of generated text within the markdown generation pipeline.

#### Invariants

- No explicit invariants; the `text` member can be any string.

#### Key Members

- `text`: The only data member, holding the fragment content.

#### Usage Patterns

- No specific usage patterns evident from evidence.

## Functions

### `clore::generate::code_spanned_fragments`

Declaration: `generate/markdown.cppm:124`

Definition: `generate/markdown.cppm:693`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::code_spanned_fragments` serves as a thin public interface that delegates the core algorithm to an internal helper. It accepts a `std::string_view` parameter `text`, creates an empty `std::vector<InlineFragment>` named `fragments`, and then calls the file‑scope function `clore::generate::(anonymous namespace)::append_code_spanned_fragments` to populate the vector. Once the helper returns, it returns the fully assembled `fragments` vector.

The actual logic inside `append_code_spanned_fragments` drives the parsing and segmentation of input `text` into inline fragments that respect code‑span boundaries. It iterates through the characters, identifies backtick‑enclosed code spans, link fragments, `operator` tokens, camel‑case boundaries, and identifier separators, and dispatches to sub‑routines such as `append_code_spanned_line`, `append_existing_code_span`, `append_code_aware_text`, and various readers (`read_backtick_span`, `read_call_suffix`, `read_link_span`, etc.). The result is a sequence of `InlineFragment` objects representing code‑style text, plain text, and other markdown inline elements, ready for further rendering or transformation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `text` parameter

#### Usage Patterns

- Called when a vector of `InlineFragment` objects is needed from a `std::string_view`, likely as a building block for markdown rendering utilities like `code_spanned_markdown`.

### `clore::generate::code_spanned_markdown`

Declaration: `generate/markdown.cppm:126`

Definition: `generate/markdown.cppm:699`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function processes a Markdown string line by line, maintaining an `in_fence` state variable to track whether the current line is inside a fenced code block. At each line, it checks whether the line is a fence delimiter by calling the anonymous-namespace helper `is_fence_line`; if so, it toggles `in_fence` and appends the line unchanged. When `in_fence` is `true` and the line is not a fence line, the line is also appended verbatim. For all other lines (outside any fenced block), the function delegates to the anonymous-namespace helper `append_code_spanned_line`, which applies the code-span transformation to the line. The final output, allocated with an initial reservation equal to the input size, accumulates the processed lines with newlines preserved.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `markdown` parameter

#### Usage Patterns

- called during markdown rendering to apply code spans to non-fence lines

### `clore::generate::make_blockquote`

Declaration: `generate/markdown.cppm:113`

Definition: `generate/markdown.cppm:169`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_blockquote` constructs a `MarkdownNode` representing a block quote from a given `std::string` input. Its implementation is straightforward: it moves the input string into `make_text`, which produces a `TextFragment`, then wraps that fragment inside a newly created `BlockQuote` struct (which holds a vector of `TextFragment` objects), and finally packs the `BlockQuote` into a `MarkdownNode` via its `MarkdownNode` value constructor. No branching, iteration, or dependency on other generation helpers occurs beyond `make_text`; the algorithm is a single delegation step with direct struct initialization.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `text`

#### Usage Patterns

- wrap text in a blockquote Markdown node

### `clore::generate::make_code`

Declaration: `generate/markdown.cppm:101`

Definition: `generate/markdown.cppm:136`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_code` takes a single `std::string` parameter named `code` and returns an `InlineFragment`. Internally, it directly constructs a `CodeFragment` object using aggregate initialization, moving the input string into the `code` field of `CodeFragment`. No additional processing, validation, or dependencies on other functions are involved; it is a trivial factory function that produces a `CodeFragment` with the provided code text.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `code`

#### Usage Patterns

- wrapping a code string into a markdown inline fragment

### `clore::generate::make_code_fence`

Declaration: `generate/markdown.cppm:109`

Definition: `generate/markdown.cppm:156`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_code_fence` directly constructs a `MarkdownNode` containing a `CodeFence` value. It moves the `language` and `code` string parameters into the corresponding fields of the `CodeFence` struct, then wraps it in the `MarkdownNode` variant. No additional validation, transformation, or formatting is applied; the implementation is a trivial factory that simply aggregates the two string arguments into the target node representation. The only dependencies are the `MarkdownNode` and `CodeFence` types, and the function performs no conditional logic or error handling.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- language parameter
- code parameter

#### Writes To

- returned `MarkdownNode`

#### Usage Patterns

- Creating markdown code fences for code snippets in generated documentation

### `clore::generate::make_link`

Declaration: `generate/markdown.cppm:103`

Definition: `generate/markdown.cppm:140`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::make_link` is a trivial factory function that constructs a `LinkFragment` value. It accepts a `label` string, a `target` string, and a `code_style` boolean, then moves each argument directly into the corresponding field of the `LinkFragment` struct (`label`, `target`, `code_style`) using brace‑initialization. No validation, transformation, or branching occurs; the function serves solely as a convenient, type‑safe constructor for `LinkFragment` objects used elsewhere in the generation pipeline. Its only dependency is the shared `LinkFragment` definition, which is aggregated into the caller’s inline fragment stream.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `label`
- `target`
- `code_style`

#### Writes To

- returns a new `LinkFragment` instance

#### Usage Patterns

- used to generate inline markdown link fragments
- likely called by rendering functions that produce markdown nodes

### `clore::generate::make_mermaid`

Declaration: `generate/markdown.cppm:111`

Definition: `generate/markdown.cppm:165`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_mermaid` constructs a `MarkdownNode` representing a Mermaid diagram from a provided `std::string code`. It moves the input `code` into a `MermaidDiagram` struct’s `code` field, then returns a `MarkdownNode` whose `value` is initialized with that `MermaidDiagram` via direct construction. The implementation is straightforward: there is no additional parsing, validation, or transformation of the code string; the function solely wraps the raw diagram text in the appropriate node type so that downstream rendering (`clore::generate::render_markdown`) can handle it as a code block with Mermaid syntax. The dependency is limited to the `MermaidDiagram` and `MarkdownNode` types defined in the same module.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `code` parameter (`std::string`)

#### Usage Patterns

- Create Mermaid diagram nodes for markdown output
- Used in rendering page bundle markdown

### `clore::generate::make_paragraph`

Declaration: `generate/markdown.cppm:105`

Definition: `generate/markdown.cppm:148`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::make_paragraph` is a straightforward factory function. It accepts a single `std::string` parameter (`text`) and immediately delegates the actual content processing to `clore::generate::make_text`, which converts the raw string into a `TextFragment` (likely performing any necessary escaping or inline formatting). The resulting `TextFragment` is then placed into a `Paragraph` object’s `fragments` vector, and the `Paragraph` is wrapped in a `MarkdownNode` using aggregate (or brace) initialization. The entire operation is a thin wrapper: no branching, looping, or error handling occurs within this function; all semantic analysis is deferred to `make_text`. The only dependency is the `make_text` utility, which must already produce a valid `TextFragment`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- function parameter `text`

#### Writes To

- returned `MarkdownNode`

#### Usage Patterns

- Used to create a paragraph Markdown node from a plain text string, likely as a helper within larger markdown generation functions.

### `clore::generate::make_raw_markdown`

Declaration: `generate/markdown.cppm:107`

Definition: `generate/markdown.cppm:152`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_raw_markdown` constructs a `MarkdownNode` representing a raw, verbatim markdown block. It moves its `std::string` argument into a `RawMarkdown` struct and wraps that struct inside a `MarkdownNode` via aggregate initialization. No further processing, validation, or transformation is performed; the raw text is stored as-is. This node type is used downstream by `render_node` and related rendering functions, which may embed the content directly in the final document output.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `markdown` of type `std::string`

#### Usage Patterns

- create a markdown node from a raw markdown string
- used to embed non-parsed markdown into a `MarkdownNode`

### `clore::generate::make_section`

Declaration: `generate/markdown.cppm:115`

Definition: `generate/markdown.cppm:173`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_section` constructs a `SemanticSection` object by direct aggregate initialization of the struct's fields using the supplied parameters. It creates a `std::unique_ptr<SemanticSection>` via `std::make_unique`, moving the `subject_key` and `heading` strings into the allocated object and initializing the remaining scalar fields (`kind`, `level`, `omit_if_empty`, `code_style_heading`) directly. The `children` member is default-initialised as an empty vector. There is no branching, error handling, or external dependency beyond the `SemanticSection` type and standard memory allocation. The returned pointer is the sole outcome; no side effects on any global state occur.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `SemanticKind kind`
- `std::string subject_key`
- `std::string heading`
- `std::uint8_t level`
- `bool omit_if_empty`
- `bool code_style_heading`

#### Writes To

- creates a new `SemanticSection` object on the heap

#### Usage Patterns

- Used to create section nodes for markdown document generation
- Called within page building functions to structure content

### `clore::generate::make_text`

Declaration: `generate/markdown.cppm:99`

Definition: `generate/markdown.cppm:132`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::make_text` is a factory function that constructs an `InlineFragment` from a raw string. It simply moves the input `text` into a newly created `TextFragment` via aggregate initialization, setting the `TextFragment::text` field. The function serves as a uniform entry point for creating plain-text inline fragments, used throughout the generation pipeline where a literal string needs to be represented as a markdown-compatible `InlineFragment`. No branching or additional logic is involved; the function relies only on the `TextFragment` struct definition.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `std::string` text parameter

#### Writes To

- Return value of type `InlineFragment`

#### Usage Patterns

- Creating text fragments for markdown nodes
- Wrapping raw strings into `InlineFragment` objects

### `clore::generate::render_markdown`

Declaration: `generate/markdown.cppm:122`

Definition: `generate/markdown.cppm:730`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function first checks for an optional `Frontmatter` on the `MarkdownDocument`. If present, it serializes the `title`, `description`, `layout`, and `page_template` fields into YAML front matter, applying `yaml_quote` to the title and description strings to escape any special characters. After the front matter (or immediately if none exists), it iterates over each child node in `document.children` and concatenates the result of calling `render_node` on that child. The core rendering logic is thus delegated to the `render_node` dispatcher (and its recursively invoked helpers such as `render_section`, `render_inlines`, `render_code_span`, etc.), which collectively handle all supported `MarkdownNode` variants.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- document`.frontmatter`
- document`.children`
- fm`.title`
- fm`.description`
- fm`.layout`
- fm`.page_template`

#### Usage Patterns

- called by page-level rendering functions like `render_page_markdown` and `render_page_bundle` to produce the final Markdown output
- used to serialize a `MarkdownDocument` structure into a string

## Internal Structure

The `generate:markdown` module decomposes documentation generation into an in-memory AST of distinct node types (paragraphs, code fences, blockquotes, lists, mermaid diagrams, raw markdown, and structured semantic sections) along with metadata structures such as `Frontmatter` and `MarkdownDocument`. The public API consists of factory functions (`make_blockquote`, `make_code`, `make_link`, etc.) that construct these nodes, and a central `render_markdown` function that converts the tree to final Markdown output. The module imports only the C++ standard library and relies on an internal layer of anonymous namespace helpers for parsing and formatting inline content—including code-span detection, backtick spans, link spans, `operator` tokens, and identifier boundary checks—as well as for constructing block-level elements like block quotes and code fences. This separation of concerns keeps node construction, inline handling, and final rendering loosely coupled while maintaining a single translation unit for clarity.

