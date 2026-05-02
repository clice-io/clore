---
title: 'Module generate:model'
description: 'The generate:model module defines the core data types and supporting functions for the documentation generation pipeline. It owns the intermediate representation used to plan, analyze, and produce output pages, including enums like PageType and PromptKind, data structures such as PagePlan, PageIdentity, GeneratedPage, and per‑symbol analysis containers (FunctionAnalysis, TypeAnalysis, VariableAnalysis, and SymbolAnalysisStore), and infrastructure types for cross‑reference resolution (LinkResolver), error handling (GenerateError, RenderError, PathError), and prompt requests (PromptRequest, MarkdownFragmentResponse). The module also provides public utility functions to query, compute, and transform these models, such as retrieving specific analyses, computing page paths, building link resolvers, classifying prompts, and validating output paths. This set of types and functions forms the contract between earlier extraction phases and later rendering phases within the generation system.'
layout: doc
template: doc
---

# Module `generate:model`

## Summary

The `generate:model` module defines the core data types and supporting functions for the documentation generation pipeline. It owns the intermediate representation used to plan, analyze, and produce output pages, including enums like `PageType` and `PromptKind`, data structures such as `PagePlan`, `PageIdentity`, `GeneratedPage`, and per‑symbol analysis containers (`FunctionAnalysis`, `TypeAnalysis`, `VariableAnalysis`, and `SymbolAnalysisStore`), and infrastructure types for cross‑reference resolution (`LinkResolver`), error handling (`GenerateError`, `RenderError`, `PathError`), and prompt requests (`PromptRequest`, `MarkdownFragmentResponse`). The module also provides public utility functions to query, compute, and transform these models, such as retrieving specific analyses, computing page paths, building link resolvers, classifying prompts, and validating output paths. This set of types and functions forms the contract between earlier extraction phases and later rendering phases within the generation system.

## Imports

- [`extract`](../extract/index.md)
- `std`
- [`support`](../support/index.md)

## Imported By

- [`generate:analysis`](analysis.md)
- [`generate:common`](common.md)
- [`generate:diagram`](diagram.md)
- [`generate:dryrun`](dryrun.md)
- [`generate:evidence`](evidence.md)
- [`generate:evidence_builder`](index.md)
- [`generate:page`](page.md)
- [`generate:planner`](planner.md)
- [`generate:scheduler`](scheduler.md)
- [`generate:symbol`](symbol.md)

## Types

### `clore::generate::FunctionAnalysis`

Declaration: `generate/model.cppm:81`

Definition: `generate/model.cppm:81`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct aggregates analysis results for a function, with a set of fields that capture distinct aspects. The boolean member `has_side_effects` defaults to `false`, and an invariant requires that when it is `false` the vector `side_effects` remains empty, though the struct does not enforce this automatically. The string members `overview_markdown` and `details_markdown` hold descriptive text, while the string vectors `reads_from`, `writes_to`, and `usage_patterns` enumerate external resources, modified state, and observed invocation patterns. The implementation is a plain aggregate without custom constructors, destructors, or member functions, so all fields are default-initialized as per their types and the struct supports trivial copy, move, and aggregate initialization.

#### Invariants

- All members are public and mutable.
- `has_side_effects` defaults to `false` and is independent of the contents of `side_effects`.

#### Key Members

- `overview_markdown`
- `details_markdown`
- `has_side_effects`
- `side_effects`
- `reads_from`
- `writes_to`
- `usage_patterns`

#### Usage Patterns

- The struct is used as a cacheable result container for per-function analysis, populated by analysis passes and consumed by documentation generation.

### `clore::generate::GenerateError`

Declaration: `generate/model.cppm:69`

Definition: `generate/model.cppm:69`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The structure is minimal, consisting solely of a public `std::string` member named `message`. No constructors, assignment `operator`s, or other special members are explicitly declared, so the compiler implicitly provides them (default constructor, copy/move constructors, copy/move assignment, destructor). The sole invariant is that `message` holds a valid `std::string` at all times; it is the responsibility of the code that constructs or modifies a `clore::generate::GenerateError` to ensure the string content accurately describes the error condition. There are no additional constraints, allocator customization, or non‑trivial member functions to manage.

#### Invariants

- Message contains a descriptive error string

#### Key Members

- `message` - a `std::string` holding the error description

#### Usage Patterns

- Constructed with an error description when a generation fails
- Likely thrown as an exception or returned from a function indicating an error

### `clore::generate::GeneratedPage`

Declaration: `generate/model.cppm:55`

Definition: `generate/model.cppm:55`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::GeneratedPage` is a plain data aggregate that bundles the results of page generation. It holds three `std::string` fields: `title`, `relative_path`, and `content`, all default‑initialized to empty strings. As a lightweight value type, it transfers generated page data between pipeline stages. The intended invariant is that `relative_path` is a valid relative file path and `content` contains the rendered page output, though no runtime checks enforce this contract. Its simplicity supports straightforward composition and manipulation of generated pages without additional overhead.

#### Invariants

- all fields are `std::string` values, possibly empty
- the struct has no other invariants beyond the default string invariants

#### Key Members

- `title` – the page title
- `relative_path` – the relative file path for the generated page
- `content` – the full page content

#### Usage Patterns

- constructed using aggregate initialization `GeneratedPage{...}`
- fields are read or modified directly to configure a generated page
- passes completed page data from generation logic to output or serialization

### `clore::generate::GenerationSummary`

Declaration: `generate/model.cppm:61`

Definition: `generate/model.cppm:61`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::GenerationSummary` is an aggregate data container that accumulates counters tracking the performance and output of the generation process. Its five public fields — `written_output_count`, `symbol_analysis_cache_hits`, `symbol_analysis_cache_misses`, `page_prompt_cache_hits`, and `page_prompt_cache_misses` — are all of type `std::size_t` and are default‑initialized to zero. This guarantees that every new instance starts with all metrics at a neutral baseline, and the use of unsigned integer types enforces the non‑negative invariant that these counters must never hold negative values.

Because the struct has no explicit constructors, assignment `operator`s, or non‑static member functions, its behavior is entirely defined by the compiler‑generated special members. The sole internal invariant is that each counter may only increase through external mutation; no member implementation provides higher‑level logic such as resetting or summing. This design makes `GenerationSummary` a lightweight, trivially copyable record that can be safely aggregated across multiple generation runs, typically by copying or adding the counters of separate summary instances.

#### Invariants

- All counter members are non-negative integers.
- Every counter begins at zero on default construction.
- Cache hit and miss counts for a given category are independent (no enforced relationship).

#### Key Members

- `written_output_count`
- `symbol_analysis_cache_hits`
- `symbol_analysis_cache_misses`
- `page_prompt_cache_hits`
- `page_prompt_cache_misses`

#### Usage Patterns

- Instances are populated during generation to record performance metrics.
- Consumers read these values to report or log generation statistics.

### `clore::generate::LinkResolver`

Declaration: `generate/model.cppm:174`

Definition: `generate/model.cppm:174`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct stores four separate `std::unordered_map<std::string, std::string>` fields: `name_to_path`, `namespace_to_path`, `module_to_path`, and `page_id_to_title`. Each map holds a distinct category of entity name (e.g., a type name, a namespace, a module, or a page identifier) as key and its corresponding relative output path or page title as value. The lookup methods—`resolve`, `resolve_namespace`, `resolve_module`, and `resolve_page_title`—each perform a hash lookup on the appropriate map and return a `const std::string*` pointing to the stored string if found, or `nullptr` otherwise. This design keeps all lookups O(1) average-case and prevents accidental cross-category mapping. The maps are typically populated externally before resolution begins; the struct does not modify them during its lifetime.

#### Invariants

- Each `unordered_map` is keyed by a string representing an entity name, page ID, or similar identifier.
- All lookup methods return `nullptr` when the key is not present in the respective map.
- The maps are read-only after construction; no mutating methods are provided.

#### Key Members

- `name_to_path`
- `namespace_to_path`
- `module_to_path`
- `page_id_to_title`
- `resolve`
- `resolve_namespace`
- `resolve_module`
- `resolve_page_title`

#### Usage Patterns

- Used by link generation code to resolve entity names to relative paths for markdown cross-reference links.
- Typically populated by a builder component that collects namespace, module, and type information.
- Queried via the four `resolve*` methods during documentation page generation.

#### Member Functions

##### `clore::generate::LinkResolver::resolve`

Declaration: `generate/model.cppm:180`

Definition: `generate/model.cppm:180`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
[[nodiscard]] auto resolve(const std::string& name) const -> const std::string* {
        auto it = name_to_path.find(name);
        return it != name_to_path.end() ? &it->second : nullptr;
    }
```

##### `clore::generate::LinkResolver::resolve_module`

Declaration: `generate/model.cppm:190`

Definition: `generate/model.cppm:190`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
[[nodiscard]] auto resolve_module(const std::string& name) const -> const std::string* {
        auto it = module_to_path.find(name);
        return it != module_to_path.end() ? &it->second : nullptr;
    }
```

##### `clore::generate::LinkResolver::resolve_namespace`

Declaration: `generate/model.cppm:185`

Definition: `generate/model.cppm:185`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
[[nodiscard]] auto resolve_namespace(const std::string& name) const -> const std::string* {
        auto it = namespace_to_path.find(name);
        return it != namespace_to_path.end() ? &it->second : nullptr;
    }
```

##### `clore::generate::LinkResolver::resolve_page_title`

Declaration: `generate/model.cppm:195`

Definition: `generate/model.cppm:195`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
[[nodiscard]] auto resolve_page_title(const std::string& page_id) const -> const std::string* {
        auto it = page_id_to_title.find(page_id);
        return it != page_id_to_title.end() ? &it->second : nullptr;
    }
```

### `clore::generate::MarkdownFragmentResponse`

Declaration: `generate/model.cppm:77`

Definition: `generate/model.cppm:77`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::MarkdownFragmentResponse` is an aggregate type consisting of a single public data member `markdown` of type `std::string`. It serves as a lightweight container for the generated markdown content. The struct imposes no structural invariants beyond the standard validity of the underlying string; any content that constitutes valid markup or plain text is acceptable. As an aggregate, it supports direct member initialization and structured bindings, enabling straightforward construction and deconstruction without additional runtime overhead or validation.

#### Invariants

- No documented invariants; the struct is trivially copyable and movable.
- The `markdown` member holds any valid `std::string` value.

#### Key Members

- `std::string markdown`

#### Usage Patterns

- Used as a return type for functions that produce markdown fragments.
- Can be aggregate-initialized with a string literal or `std::string`.

### `clore::generate::PageIdentity`

Declaration: `generate/model.cppm:207`

Definition: `generate/model.cppm:207`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::PageIdentity` is a plain aggregate that bundles four fields to represent the identity of a generated documentation page. Its internal structure consists of `page_type` (a `PageType` enumerator defaulting to `PageType::File`), `normalized_owner_key`, `qualified_name`, and `source_relative_path`, all of which are `std::string` defaulting to empty. The key invariant is that `qualified_name` and `source_relative_path` together uniquely identify a page within a module; `normalized_owner_key` may be empty for pages that are not nested under an owner. No custom constructors, destructors, or member functions are defined, so the struct relies entirely on default member initializers and aggregate initialization. This design allows callers to construct a `PageIdentity` using brace initiation or designated initializers without exposing any internal logic beyond the field definitions.

#### Invariants

- `page_type` should be a valid member of the `PageType` enumeration
- All string members may be empty unless set externally

#### Key Members

- `page_type`
- `normalized_owner_key`
- `qualified_name`
- `source_relative_path`

#### Usage Patterns

- Not evident from provided context; used as a data container for page identification

### `clore::generate::PagePlan`

Declaration: `generate/model.cppm:39`

Definition: `generate/model.cppm:39`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::PagePlan` is a data‑carrying aggregate that captures all metadata and configuration needed to generate a single page. The core fields fall into three categories: identification and navigation (e.g. `page_id`, `title`, `relative_path`); dependency and linkage lists (`depends_on_pages` and `linked_pages`); and generation‑specific data (`prompt_requests` and `owner_keys`). The `page_id` is expected to be unique across plans, and the `relative_path` must be consistent with the output directory structure. `depends_on_pages` encodes a topological ordering constraint, while `prompt_requests` holds the sequence of LLM or template calls required to produce the page content. All vector fields are empty by default, and `page_type` defaults to `PageType::File`, making a minimal plan valid only when `page_id`, `title`, and `relative_path` are subsequently set before use.

#### Invariants

- `page_type` defaults to `PageType::File` if not otherwise set.
- All string and vector fields are default-initialized to empty values.
- The struct itself does not enforce inter-field consistency; it is a plain aggregate.

#### Key Members

- `page_id`
- `page_type`
- `title`
- `relative_path`
- `owner_keys`
- `depends_on_pages`
- `linked_pages`
- `prompt_requests`

#### Usage Patterns

- Used as a data container to specify all attributes needed to generate a page.
- Consumed by page generation logic to determine the page's identity, dependencies, and content requests.
- Likely populated by other components (e.g., parsing, planning) before being passed to generation.

### `clore::generate::PagePlanSet`

Declaration: `generate/model.cppm:50`

Definition: `generate/model.cppm:50`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The internal structure of `clore::generate::PagePlanSet` consists of two parallel containers: `plans`, a vector of `PagePlan` objects, and `generation_order`, a vector of strings. The `plans` member stores the actual page plan data, while `generation_order` holds the sequence of plan identifiers—typically page names or keys—that defines the order in which the plans should be generated.

A key invariant is that the two vectors are maintained in corresponding order: the string at index `i` in `generation_order` identifies the plan at the same index `i` in `plans`. This alignment ensures that iterating through `generation_order` yields a deterministic ordering of plans without requiring a separate key lookup. No additional state or synchronization logic is present; the struct acts as a simple aggregate that bundles the generation sequence with its associated plan data.

#### Invariants

- No explicit invariants are provided in the evidence.

#### Key Members

- `plans` – the container of `PagePlan` instances
- `generation_order` – a sequence of strings tracking generation order

#### Usage Patterns

- No usage patterns are shown in the evidence.

### `clore::generate::PageType`

Declaration: `generate/model.cppm:9`

Definition: `generate/model.cppm:9`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The enumeration `clore::generate::PageType` is defined as a scoped enumeration with underlying type `std::uint8_t`, ensuring a compact storage footprint. Its four enumerators—`Index`, `Module`, `Namespace`, and `File`—are declared in that specific order. This ordering encodes an implicit priority or processing sequence used internally: `Index` appears first, followed by `Module`, `Namespace`, and finally `File`. The integer values assigned by the compiler therefore increase in that order, which may be exploited for ordered iteration, comparison, or dispatch in page-generation logic. No other members or operations are defined, so the enumeration serves purely as a fixed set of discriminants for categorizing generated pages.

#### Invariants

- Only four distinct page types exist
- Each enumerator maps to a unique `std::uint8_t` value
- The enum is stored in `uint8_t` for space efficiency

#### Key Members

- `clore::generate::PageType::Index`
- `clore::generate::PageType::Module`
- `clore::generate::PageType::Namespace`
- `clore::generate::PageType::File`

#### Usage Patterns

- Used as a parameter or field to indicate the kind of page being generated
- Switched on to produce different layout or content logic
- Passed to page creation functions to specialize output

#### Member Variables

##### `clore::generate::PageType::File`

Declaration: `generate/model.cppm:13`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
File
```

##### `clore::generate::PageType::Index`

Declaration: `generate/model.cppm:10`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Index
```

##### `clore::generate::PageType::Module`

Declaration: `generate/model.cppm:11`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Module
```

##### `clore::generate::PageType::Namespace`

Declaration: `generate/model.cppm:12`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Namespace
```

### `clore::generate::PathError`

Declaration: `generate/model.cppm:203`

Definition: `generate/model.cppm:203`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::PathError` is a minimal error type implemented as a simple value object. Its only data member is `message`, a `std::string` that stores a human-readable description of the error. No special member functions are user-declared; the compiler-generated default constructor, destructor, copy, and move operations are relied upon. Consequently, the invariants are those of `std::string`: the `message` member always holds a valid string object, and the struct itself can be copied, moved, and destroyed trivially. The type serves exclusively as a lightweight wrapper for an error string within the generation subsystem.

#### Invariants

- `message` contains a human-readable error description
- the struct is default-constructible and copyable via implicit compiler-generated special members

#### Key Members

- `message`

#### Usage Patterns

- returned or thrown to indicate a path-related error
- used in contexts where a descriptive string error is sufficient

### `clore::generate::PromptKind`

Declaration: `generate/model.cppm:18`

Definition: `generate/model.cppm:18`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The enum `clore::generate::PromptKind` is defined with an underlying type of `std::uint8_t`, ensuring a compact representation suitable for storage or serialization. Its enumerators—`NamespaceSummary`, `ModuleSummary`, `ModuleArchitecture`, `IndexOverview`, `FunctionAnalysis`, `TypeAnalysis`, `VariableAnalysis`, `FunctionDeclarationSummary`, `FunctionImplementationSummary`, `TypeDeclarationSummary`, and `TypeImplementationSummary`—are listed sequentially and cover the complete set of distinct prompt categories used within the generation system. The order is fixed and does not rely on any bit‑flag or combinatorial semantics; each value uniquely identifies a specific kind of analysis or summary prompt. No additional member functions or overloads are provided, as the enum serves solely as a discriminated type for dispatch logic rather than a stateful object. No invariants beyond the defined values exist; the set is intentionally exhaustive for its purpose.

#### Invariants

- Each enumerator value corresponds to a specific kind of prompt.
- The underlying type is `std::uint8_t`, ensuring compact storage.
- All enumerators are mutually exclusive and distinct.

#### Key Members

- `clore::generate::PromptKind::NamespaceSummary`
- `clore::generate::PromptKind::ModuleSummary`
- `clore::generate::PromptKind::ModuleArchitecture`
- `clore::generate::PromptKind::FunctionAnalysis`
- `clore::generate::PromptKind::TypeAnalysis`
- `clore::generate::PromptKind::VariableAnalysis`
- `clore::generate::PromptKind::FunctionDeclarationSummary`
- `clore::generate::PromptKind::FunctionImplementationSummary`
- `clore::generate::PromptKind::TypeDeclarationSummary`
- `clore::generate::PromptKind::TypeImplementationSummary`
- `clore::generate::PromptKind::IndexOverview`

#### Usage Patterns

- Used to select generation behavior in switch or if-else chains.
- Passed as a parameter to indicate which type of prompt to generate.
- May be stored in configuration or state to drive the generation process.

#### Member Variables

##### `clore::generate::PromptKind::FunctionAnalysis`

Declaration: `generate/model.cppm:23`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
FunctionAnalysis
```

##### `clore::generate::PromptKind::FunctionDeclarationSummary`

Declaration: `generate/model.cppm:26`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
FunctionDeclarationSummary
```

##### `clore::generate::PromptKind::FunctionImplementationSummary`

Declaration: `generate/model.cppm:27`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
FunctionImplementationSummary
```

##### `clore::generate::PromptKind::IndexOverview`

Declaration: `generate/model.cppm:22`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
IndexOverview
```

##### `clore::generate::PromptKind::ModuleArchitecture`

Declaration: `generate/model.cppm:21`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
ModuleArchitecture
```

##### `clore::generate::PromptKind::ModuleSummary`

Declaration: `generate/model.cppm:20`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
ModuleSummary
```

##### `clore::generate::PromptKind::NamespaceSummary`

Declaration: `generate/model.cppm:19`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
NamespaceSummary
```

##### `clore::generate::PromptKind::TypeAnalysis`

Declaration: `generate/model.cppm:24`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
TypeAnalysis
```

##### `clore::generate::PromptKind::TypeDeclarationSummary`

Declaration: `generate/model.cppm:28`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
TypeDeclarationSummary
```

##### `clore::generate::PromptKind::TypeImplementationSummary`

Declaration: `generate/model.cppm:29`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
TypeImplementationSummary
```

##### `clore::generate::PromptKind::VariableAnalysis`

Declaration: `generate/model.cppm:25`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
VariableAnalysis
```

### `clore::generate::PromptRequest`

Declaration: `generate/model.cppm:34`

Definition: `generate/model.cppm:34`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::PromptRequest` is a plain data class that aggregates the parameters for a code-generation prompt. Its internal structure consists of exactly two public fields: `kind` (type `PromptKind`) and `target_key` (type `std::string`). The design enforces no invariants beyond the default initialization: `kind` defaults to `PromptKind::NamespaceSummary` and `target_key` defaults to an empty string, ensuring that a default-constructed object is always in a well‑defined state ready for use. Because both members are directly accessible, callers are free to mutate them without going through accessor functions, making the struct lightweight and suitable as a value‑type message payload.

#### Invariants

- `kind` is always a valid `PromptKind` value
- `target_key` is a string, possibly empty
- Default initialization sets `kind` to `PromptKind::NamespaceSummary` and `target_key` to an empty string
- Both fields are publicly accessible for direct manipulation

#### Key Members

- `kind` field
- `target_key` field

#### Usage Patterns

- Instances are constructed with specific `kind` and `target_key` values to request a prompt for a particular entity
- The struct is passed to functions that generate prompts based on its contents
- Default-constructed instances represent a request for a namespace summary with an unspecified target

### `clore::generate::RenderError`

Declaration: `generate/model.cppm:73`

Definition: `generate/model.cppm:73`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::RenderError` is a simple error type that encapsulates a diagnostic string via its single public member `message` of type `std::string`. It has no invariants beyond the validity of the underlying string; any non-null string (including the empty string) is a valid state. The implementation relies entirely on `std::string` for storage and management, so the struct is implicitly default-constructible, copyable, and movable with no special member functions. Its purpose is to serve as a lightweight, self-contained error representation for rendering operations, carrying a human-readable description of what went wrong.

### `clore::generate::SymbolAnalysisStore`

Declaration: `generate/model.cppm:125`

Definition: `generate/model.cppm:125`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `clore::generate::SymbolAnalysisStore` aggregates three distinct analysis caches: `FunctionAnalysisCache`, `TypeAnalysisCache`, and `VariableAnalysisCache`. Each of these caches stores precomputed analysis results for the corresponding symbol category, allowing efficient lookup during code generation. The struct itself serves as a homogeneous owner of these independent caches; it does not impose dependencies between them. The contained caches are expected to be fully populated before generation begins, and their contents are not modified after construction, ensuring a stable snapshot of the analysis state.

#### Invariants

- Each cache is expected to be fully populated before use.
- The struct is intended to be shared across multiple documentation generation contexts.

#### Key Members

- `functions`
- `types`
- `variables`

#### Usage Patterns

- Accessed as a shared cache by documentation generators.
- Populated once and then reused for multiple pages.

### `clore::generate::SymbolTargetKeyView`

Declaration: `generate/model.cppm:136`

Definition: `generate/model.cppm:136`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::SymbolTargetKeyView` is a lightweight, aggregate view type that holds two `std::string_view` members: `qualified_name` and `signature`. As a simple aggregate, it relies on compiler‑generated default construction, copy, and move operations; no custom constructors, assignment `operator`s, or other member functions are defined. The key invariant of this type is that the character data referred to by each `string_view` must remain valid and unmodified for the entire lifetime of the view instance. This design avoids copying the underlying string content and is intended for use as a lookup key or map inserter where the actual string storage is managed elsewhere. The struct itself does not enforce any relationship between the two views—they are treated as independent, opaque references.

#### Invariants

- The underlying character data for both `qualified_name` and `signature` must outlive the `SymbolTargetKeyView` instance.
- The struct has no owning or allocating behavior; it is a passive view into externally managed strings.

#### Key Members

- `std::string_view qualified_name`
- `std::string_view signature`

#### Usage Patterns

- Used as a key type for symbol identification in maps or sets without copying qualified names or signatures.
- Likely constructed by passing pointers or string views from persistent symbol tables or string storage.
- Expected to be compared or hashed for efficient lookup of symbol targets.

### `clore::generate::TypeAnalysis`

Declaration: `generate/model.cppm:91`

Definition: `generate/model.cppm:91`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::TypeAnalysis` is a plain data aggregate that stores the decomposition of a type's documentation output. Its five public fields hold distinct categories of generated content: `overview_markdown` and `details_markdown` contain Markdown prose at two levels of granularity, while the three `std::vector<std::string>` fields — `invariants`, `key_members`, and `usage_patterns` — collect lists of textual items that describe the type’s logical guarantees, prominent interface elements, and common use cases, respectively. No invariants link the fields; each vector is independently populated during analysis, and the struct imposes no constraints beyond the types of its members. It serves exclusively as a container for the results of the generation pipeline.

#### Invariants

- Fields are of standard library types (`std::string` and `std::vector<std::string>`).
- Each field holds independently maintained documentation text.

#### Key Members

- `overview_markdown`
- `details_markdown`
- `invariants`
- `key_members`
- `usage_patterns`

#### Usage Patterns

- Cached and reused across namespace, module, file, and symbol documentation pages.
- Stores analysis output for later retrieval by documentation generation.

### `clore::generate::VariableAnalysis`

Declaration: `generate/model.cppm:99`

Definition: `generate/model.cppm:99`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `clore::generate::VariableAnalysis` struct is an aggregate data carrier that bundles the results of analysing a single variable. Its internal state consists of two textual summaries (`overview_markdown` and `details_markdown`), a boolean flag `is_mutated` (initialised to `false`), and two vectors of strings: `mutation_sources` and `usage_patterns`. The invariant of `is_mutated` implies that when `true`, the `mutation_sources` vector is expected to contain at least one entry describing how the variable is mutated; conversely, when `false`, `mutation_sources` should be empty. The `usage_patterns` vector collects descriptions of how the variable is referenced, and may be empty even when `is_mutated` is `true`. No member functions are provided; all fields are publicly accessible and default-initialised, making the struct a plain data object with no custom copy, move, or comparison logic.

#### Invariants

- `is_mutated` is initialized to `false`
- `mutation_sources` and `usage_patterns` start as empty vectors
- All fields are expected to be populated by an analysis pass before use

#### Key Members

- `overview_markdown`
- `details_markdown`
- `is_mutated`
- `mutation_sources`
- `usage_patterns`

#### Usage Patterns

- Created and populated by variable analysis routines within the `clore::generate` library
- Consumed by documentation generation to produce structured content for variable symbols

## Functions

### `clore::generate::analysis_details_markdown`

Declaration: `generate/model.cppm:157`

Definition: `generate/model.cppm:373`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::analysis_details_markdown` is a thin wrapper around `analysis_markdown` specialized to extract the `details_markdown` field from the appropriate analysis struct. It accepts a `SymbolAnalysisStore` and a `extract::SymbolInfo` (referred to as `symbol` in the implementation) and returns a pointer to a string. Internally, it calls `analysis_markdown` with a lambda that returns `&analysis.details_markdown` for the resolved analysis object. The control flow delegates to `analysis_markdown`, which dispatches based on the symbol kind (function, type, or variable) to locate the corresponding `FunctionAnalysis`, `TypeAnalysis`, or `VariableAnalysis` within the store; if the analysis is found, the lambda retrieves the `details_markdown` member, otherwise `nullptr` is returned. This function depends on `analysis_markdown`, the `details_markdown` field present in each analysis struct, and the `SymbolAnalysisStore` that maps symbol identifiers to their analysis data.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const SymbolAnalysisStore &` `analyses`
- `const extract::SymbolInfo &` `symbol`

#### Usage Patterns

- Called to retrieve the details markdown for rendering in documentation pages

### `clore::generate::analysis_markdown`

Declaration: `generate/model.cppm:342`

Definition: `generate/model.cppm:342`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::analysis_markdown` first constructs a `SymbolTargetKeyView` from the supplied symbol using `make_symbol_target_key`. It then dispatches on the symbol’s kind: for function symbols it calls `find_function_analysis`; for type symbols `find_type_analysis`; for variable symbols `find_variable_analysis`. Each lookup uses the computed target key against the `analyses` store. If a matching analysis object is found, the templated `field_accessor` functor is invoked on that object (for example to extract `overview_markdown` or `details_markdown`) and a pointer to the resulting string is returned. When no analysis is available for the given kind, the function returns `nullptr`. The control flow is a straightforward chain of kind tests with early returns, relying on the helper functions `is_function_kind`, `is_type_kind`, and `is_variable_kind` to classify the symbol.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `SymbolAnalysisStore` `analyses`
- `extract::SymbolInfo` `symbol`
- `FieldAccessor` `field_accessor`

#### Usage Patterns

- accessing overview or details markdown for function, type, or variable analysis
- template used with field accessors like `&FunctionAnalysis::overview` or `&TypeAnalysis::details`
- lookup by symbol key in analysis store

### `clore::generate::analysis_overview_markdown`

Declaration: `generate/model.cppm:154`

Definition: `generate/model.cppm:366`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::analysis_overview_markdown` delegates to `clore::generate::analysis_markdown`, passing a field‑accessor lambda that returns the address of the `overview_markdown` member from the appropriate analysis type. Internally, `analysis_markdown` dispatches on the symbol kind by calling one of `clore::generate::find_function_analysis`, `clore::generate::find_type_analysis`, or `clore::generate::find_variable_analysis` to locate the corresponding `FunctionAnalysis`, `TypeAnalysis`, or `VariableAnalysis` object, then applies the given accessor to retrieve the overview text. This design unifies the retrieval of overview content across all symbol categories while keeping the per‑type field access logic within the caller.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses` (const `SymbolAnalysisStore`&)
- `symbol` (const `extract::SymbolInfo`&)

#### Usage Patterns

- Used as a convenience accessor for the overview markdown of a symbol's analysis.

### `clore::generate::build_link_resolver`

Declaration: `generate/model.cppm:201`

Definition: `generate/model.cppm:471`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function iterates over each plan in `plan_set.plans`. For every plan, it records a title mapping in `resolver.page_id_to_title` by associating `plan.page_id` with `plan.title`. Then, for each key in `plan.owner_keys`, it uses `emplace` on `resolver.name_to_path` so that the first registration wins; if the plan’s `page_type` is `PageType::Namespace` it also inserts into `resolver.namespace_to_path`, and if `PageType::Module` into `resolver.module_to_path`. After processing all owner keys, it checks `plan.page_id` for a colon delimiter. If found, the substring following the colon (the suffix) is treated as an additional symbolic name and inserted into the same three maps with the same type‑specific disambiguation logic. This suffix extraction supports paths like `module:some_component` where only `some_component` is stored as an extra key.

The algorithm depends on `PagePlanSet` and `PagePlan`, and uses `PageType` enumerators (`PageType::Namespace`, `PageType::Module`) to decide which specialized map to populate. No source‑relative path computation or analysis lookups occur inside the loop; all path and identity information is assumed to be precomputed in the `plan` objects. The returned `LinkResolver` provides both a generic `name_to_path` map and type‑keyed maps (`namespace_to_path`, `module_to_path`) to allow callers to disambiguate identical names from different scopes.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan_set` parameter, specifically `plan_set.plans`, and each plan's `page_id`, `title`, `owner_keys`, `relative_path`, `page_type`

#### Writes To

- Local `resolver` object's maps: `page_id_to_title`, `name_to_path`, `namespace_to_path`, `module_to_path` (these become part of the return value)

#### Usage Patterns

- Building a `LinkResolver` from a `PagePlanSet`
- Used by page generation to provide a mapping from `IDs` to titles and paths

### `clore::generate::compute_page_path`

Declaration: `generate/model.cppm:214`

Definition: `generate/model.cppm:576`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::compute_page_path` implements a switch on `identity.page_type` to construct a filesystem path string from a `PageIdentity`. For each page type, it decomposes the identity's `qualified_name` or `source_relative_path` into components, normalizes them via `normalize_name`, and assembles a path using helper constants such as `kIndexPath`, `kModulePrefix`, `kNamespacePrefix`, and `kFilePrefix`. Internal control flow handles partition extraction for module pages (splitting on `:`), extension stripping for file pages, and inserting an `"index.md"` suffix for namespace and module pages without a partition. After path construction, the function applies `sanitize_path_chars` to replace disallowed characters and calls `validate_path_component` to check the result; if validation fails, it returns a `PathError` via `std::unexpected`. The implementation relies on `split_qualified`, `join_path`, and the anonymous namespace helpers, and depends on the `PageType` enum to dispatch the correct logic for index, module, namespace, and file identities.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `identity.page_type`
- `identity.qualified_name`
- `identity.source_relative_path`

#### Usage Patterns

- Called during page generation to determine the output file path for a given `PageIdentity`.
- Used by page building functions such as `build_page_root` and `write_page` to produce the final file location.

### `clore::generate::find_function_analysis`

Declaration: `generate/model.cppm:145`

Definition: `generate/model.cppm:323`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function performs a lookup in the `analyses.functions` associative container using the provided `symbol_target_key` as the key. If a matching entry exists, it returns a pointer to the corresponding `FunctionAnalysis` object; otherwise it returns `nullptr`. This is a straightforward map lookup — typically a hash‑based or tree‑based search — with no additional control flow or side effects beyond the single `find` operation. The implementation depends on the `SymbolAnalysisStore` type (specifically its `functions` member, which is expected to be a mapping from key strings to `FunctionAnalysis` values) and the `FunctionAnalysis` type definition.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses.functions` via `std::map::find` using `symbol_target_key`

#### Usage Patterns

- Retrieve existing `FunctionAnalysis` for a symbol key
- Check if a function analysis has been cached
- Used by other generation functions to access analysis data

### `clore::generate::find_type_analysis`

Declaration: `generate/model.cppm:148`

Definition: `generate/model.cppm:329`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function performs a direct lookup in the `analyses.types` associative container using the provided `symbol_target_key`. If the key is present, it returns a pointer to the corresponding `TypeAnalysis` object; otherwise it returns `nullptr`. The implementation has no additional branching or side effects—it is a simple accessor that depends only on the structure of `SymbolAnalysisStore`, specifically that its `types` member supports `find` and stores `TypeAnalysis` values keyed by string view. This lookup is used downstream to retrieve precomputed type analysis data during documentation generation, and the returned pointer is treated as non‑owning (the store maintains ownership).

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses.types`
- `symbol_target_key`

#### Usage Patterns

- Look up an existing type analysis for rendering
- Called from other analysis retrieval functions

### `clore::generate::find_variable_analysis`

Declaration: `generate/model.cppm:151`

Definition: `generate/model.cppm:335`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function performs a straightforward lookup into the `variables` member of the provided `SymbolAnalysisStore`. It uses the `symbol_target_key` parameter—expected to be a `std::string_view`—as the key for a map search via `analyses.variables.find(symbol_target_key)`. If the key exists in the container, the function returns a pointer to the corresponding `VariableAnalysis`; otherwise it returns `nullptr`. No iteration, validation, or fallback logic is involved; the implementation depends entirely on the map’s associative lookup and the invariant that the caller supplies a correctly formatted target key. This minimal accessor is leveraged by higher-level generation routines to retrieve variable-specific analysis data without repeating the map query.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses.variables`
- `symbol_target_key`

#### Usage Patterns

- Retrieve variable analysis for a given symbol target key
- Used by functions that build evidence or markdown for variable symbols

### `clore::generate::is_function_kind`

Declaration: `generate/model.cppm:162`

Definition: `generate/model.cppm:393`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::is_function_kind` implements a simple classification over the `extract::SymbolKind` enumeration. Internally, it uses a `switch` statement that yields `true` for the `Function` and `Method` enumerators, and `false` for any other value (the `default` branch). The only dependency is the `extract::SymbolKind` enum, which must define at least those two members. No additional algorithms, data structures, or control flow beyond the single switch are present.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `kind` (integer representing an `extract::SymbolKind`)

#### Usage Patterns

- Checking if a symbol kind corresponds to a function (including method)
- Filtering symbols in meta-programming or generation logic
- Branching on symbol classification in `SymbolAnalysisStore` processing

### `clore::generate::is_page_level_symbol`

Declaration: `generate/model.cppm:166`

Definition: `generate/model.cppm:405`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function first checks the lexical parent kind: if it is neither `extract::SymbolKind::Unknown` nor `extract::SymbolKind::Namespace`, it immediately returns `false`, filtering out symbols nested inside functions, types, or other non-namespace scopes. It then searches for the literal `"(anonymous namespace)"` in both the enclosing namespace string and the lexical parent name; if found, the symbol is considered an implementation detail and is excluded. If the symbol has a parent (obtained via `extract::lookup_symbol`), the function checks whether that parent’s kind is either a type or a function (using `is_type_kind` and `is_function_kind`); if so, it returns `false` because the symbol is nested inside a non–page-level declaration. Only symbols that pass all these filters are classified as page-level.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const extract::ProjectModel& model` (via `extract::lookup_symbol`)
- `const extract::SymbolInfo& sym` (fields `lexical_parent_kind`, `enclosing_namespace`, `lexical_parent_name`, `parent`)
- `extract::lookup_symbol`
- `is_type_kind`
- `is_function_kind`

#### Usage Patterns

- Filtering symbols for page-level documentation generation
- Used in functions like `build_page_plan_set`, `collect_namespace_symbols`, and `collect_implementation_symbols`

### `clore::generate::is_page_summary_prompt`

Declaration: `generate/model.cppm:133`

Definition: `generate/model.cppm:297`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::is_page_summary_prompt` performs a straightforward categorical check. It evaluates the incoming `PromptKind` value and returns `true` exactly when `kind` matches either `PromptKind::NamespaceSummary` or `PromptKind::ModuleSummary`. The control flow consists of a single `return` statement using a logical OR, with no branching beyond the two equality comparisons. The function depends only on the `PromptKind` enumeration and its two named enumerators, serving as a quick predicate to distinguish page‑level summary prompts from other prompt kinds in the generation pipeline.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter "kind" of type "`PromptKind`"
- enum values "`PromptKind::NamespaceSummary`" and "`PromptKind::ModuleSummary`"

#### Usage Patterns

- Used as a predicate to distinguish page-level summary prompts from other prompt kinds.
- Likely called when building prompts for namespace or module summary pages.

### `clore::generate::is_symbol_analysis_prompt`

Declaration: `generate/model.cppm:134`

Definition: `generate/model.cppm:301`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::is_symbol_analysis_prompt` implements a simple classification query. It accepts a `PromptKind` enumerator and returns `true` if the value matches any of the three symbol‑focused analysis kinds: `PromptKind::FunctionAnalysis`, `PromptKind::TypeAnalysis`, or `PromptKind::VariableAnalysis`. The implementation is a direct equality comparison against each of these constants, short‑circuiting with `||`. No additional state, branching, or dependencies beyond the `PromptKind` enumeration are involved. This predicate is used to distinguish prompts that request per‑symbol analysis content (e.g., overviews, details, usage patterns) from other prompt categories such as summary or overview prompts.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `kind` of type `PromptKind`

#### Usage Patterns

- branching on prompt kind in build or dispatch logic
- filtering symbol analysis prompts from other prompt kinds

### `clore::generate::is_type_kind`

Declaration: `generate/model.cppm:160`

Definition: `generate/model.cppm:380`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function uses a switch statement over the `extract::SymbolKind` enumerator. It explicitly lists seven cases—Class, Struct, Enum, Union, Concept, Template, and `TypeAlias`—each returning true. All other symbol kinds fall through to the default branch, returning false. This logic mirrors the pattern used by sibling functions such as `is_function_kind` and `is_variable_kind`, and relies on the `extract::SymbolKind` enumeration defined in the extraction layer. No external state or complex branching is required; the function is purely deterministic and branch‑predictor friendly for the supported type‑like kinds.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `kind` (the `extract::SymbolKind` parameter)

#### Usage Patterns

- Used as a predicate to filter or classify symbol kinds as type-like
- Likely called in type analysis or evidence building functions

### `clore::generate::is_variable_kind`

Declaration: `generate/model.cppm:164`

Definition: `generate/model.cppm:401`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::is_variable_kind` performs a straightforward equality check against two enumerators of `extract::SymbolKind`. It accepts a single `extract::SymbolKind` parameter and returns `true` when the argument equals either `extract::SymbolKind::Variable` or `extract::SymbolKind::EnumMember`; otherwise it returns `false`. The function has no side effects, no control flow beyond the boolean expression, and depends solely on the definition of `extract::SymbolKind` and its two referenced enumerators. This predicate is used elsewhere in the generation pipeline to classify symbol kinds that should be treated as variable-like entities, enabling consistent branching for variable‑specific logic (e.g., selecting the appropriate `PromptKind::VariableAnalysis` or looking up `VariableAnalysis` records).

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `kind`

#### Usage Patterns

- used as a predicate to classify symbol kinds
- likely called in `clore::generate` symbol processing or filtering

### `clore::generate::make_source_relative`

Declaration: `generate/model.cppm:169`

Definition: `generate/model.cppm:432`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::make_source_relative` implements a caching path-relativizer that transforms an absolute or already-relative `path` into a normalized form relative to `project_root`. It first constructs a composite `key` by concatenating `project_root`, a newline delimiter, and `path`. It then checks a thread‑safe module‑level cache obtained via `source_relative_cache()`: under a `shared_lock` it looks up `key` in `SourceRelativeCache::relative_by_key` and returns the cached result on a hit. On a miss, it performs the core path computation using `std::filesystem`. Both `path` and `project_root` are normalized via `lexically_normal`, then `lexically_relative` derives the relative path from the normalized root. If the relative result is empty or begins with `".."` (indicating no valid relative transformation), the original `path` is used as the resolved value; otherwise the relative path is converted to generic format. Finally, under a `unique_lock`, the resolved string is inserted into the cache with `insert_or_assign` and returned. The only dependency is the anonymous‑namespace helper `source_relative_cache`, which provides the shared `SourceRelativeCache` instance.

#### Side Effects

- caches the computed relative path in a static thread-safe cache (`source_relative_cache`)
- mutates the cache by inserting or assigning a new entry

#### Reads From

- parameter `path`
- parameter `project_root`
- static cache `source_relative_cache`
- filesystem via `std::filesystem::path` normalization

#### Writes To

- static cache `source_relative_cache`

#### Usage Patterns

- computing relative source file paths for documentation generation
- used by other generate functions to obtain relative paths from project root

### `clore::generate::make_symbol_target_key`

Declaration: `generate/model.cppm:141`

Definition: `generate/model.cppm:306`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function constructs a string key from an `extract::SymbolInfo` object by first formatting the length of the symbol’s `qualified_name` followed by the name itself using `std::format`, then appending the `signature` member directly. The resulting key is returned as a `std::string`. The algorithm is purely linear and deterministic: it composes the key in two steps, relying on the `qualified_name` and `signature` fields of the input structure. No internal branching or iteration occurs, and the function has no dependencies beyond the standard library’s `std::format` and the definition of `extract::SymbolInfo`. This key is later used for lookup and caching in the symbol analysis pipeline.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- symbol`.qualified_name`
- symbol`.qualified_name``.size()`
- symbol`.signature`

#### Usage Patterns

- Generating unique keys for symbol targets in page plan or link resolution
- Creating cache keys for symbol analysis requests
- Mapping symbol identifiers to stable strings for output generation

### `clore::generate::page_type_name`

Declaration: `generate/model.cppm:16`

Definition: `generate/model.cppm:263`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::page_type_name` maps each enumerator of the `PageType` enum to a corresponding string literal via a `switch` statement. For `PageType::Index`, `PageType::Module`, `PageType::Namespace`, and `PageType::File`, it returns `"index"`, `"module"`, `"namespace"`, and `"file"` respectively. If none of those cases match—for example, if an unknown enumerator value is passed—it falls back to returning `"unknown"`. Its only dependency is the `PageType` enumeration defined in the same module.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `type` of type `PageType`

#### Usage Patterns

- used to obtain a string label for a page type during page generation
- likely called in functions such as `build_page_root` or `compute_page_path` to derive path components or metadata

### `clore::generate::parse_symbol_target_key`

Declaration: `generate/model.cppm:143`

Definition: `generate/model.cppm:312`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function delegates to the private helper `parse_length_prefixed_symbol_target_key` to attempt a structured parse of the incoming `target_key`. If that helper returns a populated `std::optional<SymbolTargetKeyView>`, the parsed view—containing separate `qualified_name` and `signature` fields—is returned directly.

When the length‑prefixed parse fails (i.e., the key is not in the prefixed form), the function constructs a fallback `SymbolTargetKeyView` that assigns the entire `target_key` to the `qualified_name` member and leaves the `signature` empty. This two‑path control flow accommodates both a compact encoded representation for known symbol keys and a simple string‑based fallback for arbitrary qualified names.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `target_key`

#### Usage Patterns

- Converting a raw target key string into a `SymbolTargetKeyView` for subsequent processing

### `clore::generate::prompt_kind_name`

Declaration: `generate/model.cppm:32`

Definition: `generate/model.cppm:273`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::prompt_kind_name` maps each enumerator of `clore::generate::PromptKind` to a corresponding string literal. It uses a flat `switch` statement over `kind`, returning a hard‑coded `std::string_view` for every known prompt category (e.g., `"namespace_summary"`, `"function_analysis"`, `"type_implementation_summary"`). If `kind` doesn’t match any case, the function falls through to a `default` path that returns `"unknown_prompt"`. This translation is used for serialisation, logging, or identification purposes; it has no dependencies beyond the `PromptKind` enumeration and the C++ standard library.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `kind` (the `PromptKind` parameter)

#### Usage Patterns

- obtain a string name for a `PromptKind`
- map enum value to corresponding string constant

### `clore::generate::prompt_request_key`

Declaration: `generate/model.cppm:131`

Definition: `generate/model.cppm:290`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::prompt_request_key` constructs a unique string key for a given `PromptRequest` by combining its `kind` and `target_key`. If `request.target_key` is empty, it returns just the result of `clore::generate::prompt_kind_name(request.kind)`; otherwise it returns that name concatenated with a colon separator and the `request.target_key` value. This key is used to identify and cache prompt responses, with the `target_key` portion typically representing a fully qualified symbol name or other page identifier.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `request.kind`
- `request.target_key`
- `prompt_kind_name(request.kind)`

#### Usage Patterns

- caching prompt responses
- creating lookup keys for prompt requests
- generating identifiers for page key computation

### `clore::generate::validate_no_path_conflicts`

Declaration: `generate/model.cppm:216`

Definition: `generate/model.cppm:644`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::validate_no_path_conflicts` performs a single linear scan over the input `path_to_id` vector, using a local `std::unordered_map<std::string, std::string> seen` to record each encountered path. For each `(path, id)` pair, it attempts to insert the mapping; if the insertion fails (i.e., the path already exists in `seen`), the function immediately returns `std::unexpected` containing a `PathError` whose `message` is constructed via `std::format` to report the conflicting identifiers and the shared path. On successful completion of the loop with no duplicates, it returns a default-constructed `std::expected<void, PathError>` representing success. The only dependencies are `std::unordered_map`, `std::unexpected`, `std::format`, and the `PathError` struct with its `message` field.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `path_to_id` parameter

#### Usage Patterns

- validate path uniqueness before building page plans
- ensure no duplicate keys in symbol-to-page mapping

## Internal Structure

The `generate:model` module provides the data‑model layer for the documentation generation pipeline. It defines the core structs and enumerations — such as `PageType`, `PromptKind`, `PagePlan`, `GeneratedPage`, and a family of analysis types (`FunctionAnalysis`, `TypeAnalysis`, `VariableAnalysis`, `SymbolAnalysisStore`) — that represent extracted symbol information, page plans, and generation results. These types are consumed by higher‑level generation logic to produce output. The module imports `extract` and `support` for foundational metadata and utilities, and relies on the standard library.

Internally, the module is split into a public interface and an anonymous namespace. The public section contains the main data types, along with utility functions for analysis lookups, markdown generation, and path resolution (e.g., `LinkResolver`, `analysis_markdown`, `compute_page_path`, `make_symbol_target_key`). The anonymous namespace encapsulates caching details, prefix constants, and helpers for string manipulation and path sanitisation (`SourceRelativeCache`, `normalize_name`, `split_qualified`, `sanitize_path_chars`, `join_path`). This layering keeps the internal caching and metadata‑specific logic private, while exposing a clean set of types and functions for the rest of the generation infrastructure.

## Related Pages

- [Module extract](../extract/index.md)
- [Module support](../support/index.md)

