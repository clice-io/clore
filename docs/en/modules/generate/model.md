---
title: 'Module generate:model'
description: 'The generate:model module defines the core data structures and analysis models that drive the documentation generation pipeline. It owns the types representing page plans (PagePlan, PagePlanSet, GeneratedPage), symbol analyses (FunctionAnalysis, TypeAnalysis, VariableAnalysis), error conditions (GenerateError, RenderError, PathError), and the stateful link resolver (LinkResolver) used to produce cross‑references in generated output. Enumerations such as PageType and PromptKind categorize pages and AI‑generated content requests, while utility functions like compute_page_path, make_symbol_target_key, and the find_*_analysis family provide the public API for building, resolving, and inspecting the document‑generation model.'
layout: doc
template: doc
---

# Module `generate:model`

## Summary

The `generate:model` module defines the core data structures and analysis models that drive the documentation generation pipeline. It owns the types representing page plans (`PagePlan`, `PagePlanSet`, `GeneratedPage`), symbol analyses (`FunctionAnalysis`, `TypeAnalysis`, `VariableAnalysis`), error conditions (`GenerateError`, `RenderError`, `PathError`), and the stateful link resolver (`LinkResolver`) used to produce cross‑references in generated output. Enumerations such as `PageType` and `PromptKind` categorize pages and AI‑generated content requests, while utility functions like `compute_page_path`, `make_symbol_target_key`, and the `find_*_analysis` family provide the public API for building, resolving, and inspecting the document‑generation model.

The module’s public implementation scope covers all symbol analysis storage (`SymbolAnalysisStore`), page identity and planning logic (`PageIdentity`), markdown fragment generation helpers (`analysis_overview_markdown`, `analysis_details_markdown`, `analysis_markdown`), and the key‑based identification and caching infrastructure (`SymbolTargetKeyView`, `SourceRelativeCache`). It serves as the authoritative representation of the generation state, bridging the extraction phase and the rendering phase by supplying a rich set of types that describe what pages to produce and how symbols should be documented.

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

The struct `clore::generate::FunctionAnalysis` is a plain data aggregate that holds the results of analyzing a single function. Its fields capture both high‑level textual summaries (`overview_markdown`, `details_markdown`) and a fine‑grained breakdown of side‑effect behaviour. The boolean `has_side_effects` acts as a pre‑computed flag, and the vectors `side_effects`, `reads_from`, `writes_to`, and `usage_patterns` enumerate the specific resources or patterns discovered. An important invariant is that `has_side_effects` is true if and only if `side_effects` is non‑empty; similarly, entries in `reads_from`, `writes_to`, and `usage_patterns` must correspond to identifiers recorded during the analysis pass. Because the struct is a simple aggregate with user‑provided default member initializers (e.g. `has_side_effects = false`), it is typically list‑initialized or aggregate‑initialised directly, with no custom constructors or member functions.

#### Invariants

- `overview_markdown` and `details_markdown` are expected to contain markdown-formatted text.
- `has_side_effects` is `true` whenever `side_effects` is non-empty, but this relationship is not enforced by the type.

#### Key Members

- `overview_markdown`
- `details_markdown`
- `has_side_effects`
- `side_effects`
- `reads_from`
- `writes_to`
- `usage_patterns`

#### Usage Patterns

- Populated by analysis passes that examine function behavior.
- Consumed by documentation generators to produce human-readable function descriptions.
- Used to determine if a function has side effects or which resources it accesses.

### `clore::generate::GenerateError`

Declaration: `generate/model.cppm:69`

Definition: `generate/model.cppm:69`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::GenerateError` is an implementation detail consisting of a single public data member `message` of type `std::string`. It serves as a lightweight error representation, likely used to convey a textual error description. No additional constructors, assignment `operator`s, or methods are defined beyond the implicitly generated ones, making the struct a simple aggregate. Its invariants are limited to whatever invariants the `std::string` member maintains.

### `clore::generate::GeneratedPage`

Declaration: `generate/model.cppm:55`

Definition: `generate/model.cppm:55`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::GeneratedPage` is a simple aggregate containing three `std::string` members: `title`, `relative_path`, and `content`. Each member is default-initialized to an empty string, ensuring that every instance starts in a well-defined state with no pointer or memory hazards. The struct imposes no invariants beyond those of its string members; all fields are publicly accessible and can be modified independently. Because it provides no custom constructors, assignment `operator`s, or member functions, its implementation is trivial and relies entirely on the compiler-generated special member functions.

#### Invariants

- All string members are default-initialized to empty.
- No invariants are enforced beyond standard string validity.

#### Key Members

- `title` – the display title of the page
- `relative_path` – the target file path relative to output root
- `content` – the rendered HTML or text content

#### Usage Patterns

- Created and filled by page generators.
- Consumed by output writers that serialize to disk.
- Passed by value or const reference in generation pipelines.

### `clore::generate::GenerationSummary`

Declaration: `generate/model.cppm:61`

Definition: `generate/model.cppm:61`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `clore::generate::GenerationSummary` struct aggregates counters that track key performance metrics during a generation pass. Every data member is a `std::size_t` initialized to zero, ensuring that a default‑constructed instance represents a clean, baseline state. The fields `written_output_count` records the number of output files or documents produced; `symbol_analysis_cache_hits` and `symbol_analysis_cache_misses` measure cache efficiency for symbol analysis lookups; and `page_prompt_cache_hits` and `page_prompt_cache_misses` capture analogous statistics for page prompt caching. The invariant that all counters start at zero and are only incremented over the lifetime of a generation run provides a simple, unidirectional accumulation model. No explicit constructors or mutators are needed beyond the default member initializers and the natural increment operations external to the struct.

#### Invariants

- All fields are non-negative integers of type `std::size_t`.
- Fields are default-initialized to zero.

#### Key Members

- `written_output_count`
- `symbol_analysis_cache_hits`
- `symbol_analysis_cache_misses`
- `page_prompt_cache_hits`
- `page_prompt_cache_misses`

#### Usage Patterns

- Used to accumulate generation and caching statistics.
- Likely populated by generation logic and inspected for performance or debugging purposes.

### `clore::generate::LinkResolver`

Declaration: `generate/model.cppm:174`

Definition: `generate/model.cppm:174`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `clore::generate::LinkResolver` holds four `std::unordered_map<std::string, std::string>` lookup tables: `name_to_path` for general entity names, `namespace_to_path` for qualified namespace names, `module_to_path` for module names, and `page_id_to_title` for page identifiers. Each table maps a string key to a relative output path or title string, and is populated before resolution. All `resolve`, `resolve_namespace`, `resolve_module`, and `resolve_page_title` member functions are `const`-qualified and `[[nodiscard]]`; they perform a `find` in the corresponding map and return a `const std::string*`, either pointing to the stored value or `nullptr` if the key is absent. This design avoids copying strings while clearly indicating lookup failure through the nullable pointer.

#### Invariants

- Maps are populated before any resolve call.
- Resolve methods are const and do not modify maps.
- Each key maps to at most one path or title.
- Returned pointer is valid as long as the map is not modified.

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

- Populated by other parts of the codebase with entity-to-path mappings.
- Called during markdown generation to resolve links for entities.
- Used to look up paths for namespaces, modules, and page identifiers.

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

The struct `clore::generate::MarkdownFragmentResponse` is implemented as a plain aggregate type with a single public data member `markdown` of type `std::string`. This design imposes no invariants beyond those inherent to the string itself, allowing direct aggregate initialization and assignment. The `markdown` field is intended to hold the generated Markdown content, and the struct serves as a lightweight, transparent container that can be easily constructed, moved, or copied without additional logic. No special member functions are defined, so the compiler‑generated defaults handle all object lifecycle operations.

#### Invariants

- The `markdown` string may be empty or contain valid markdown.
- No other constraints are implied by the evidence.

#### Key Members

- `markdown`

#### Usage Patterns

- Used as the return type for functions that generate Markdown fragments.
- Other code can directly access the `markdown` member to obtain the generated text.

### `clore::generate::PageIdentity`

Declaration: `generate/model.cppm:207`

Definition: `generate/model.cppm:207`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::PageIdentity` is a flat aggregate of four data members that together identify a generated documentation page. Its internal structure consists of a `PageType` enumerator `page_type` (defaulting to `PageType::File`), a `normalized_owner_key` string for recording the owning entity's key, a `qualified_name` string holding the fully qualified name, and a `source_relative_path` string storing the path relative to the source root. No invariants are enforced beyond those provided by the default member initializers, making it a plain data carrier. The `normalized_owner_key` and `qualified_name` fields are expected to be non-empty for most page identities, while `source_relative_path` may be empty for synthetic pages.

#### Invariants

- Fields are default-initialized; `page_type` defaults to `PageType::File`, string fields to empty.

#### Key Members

- `page_type`
- `normalized_owner_key`
- `qualified_name`
- `source_relative_path`

#### Usage Patterns

- Used as a data transfer object to carry page identity information during documentation generation.
- Likely constructed and passed to other components that generate or index pages.

### `clore::generate::PagePlan`

Declaration: `generate/model.cppm:39`

Definition: `generate/model.cppm:39`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `clore::generate::PagePlan` struct is an internal aggregate that holds all metadata and configuration for a single page to be generated. Its fields are all default-initialized: `page_id` and `title` are empty strings, `page_type` defaults to `PageType::File`, `relative_path` is an empty string, and the four vector fields (`owner_keys`, `depends_on_pages`, `linked_pages`, `prompt_requests`) are initially empty. The `depends_on_pages` and `linked_pages` vectors encode the page’s position in the dependency and cross‑reference graph, while `prompt_requests` stores the collection of `PromptRequest` objects that will drive content generation. `owner_keys` records authorship or ownership identifiers, and `relative_path` holds the intended output path. The struct itself imposes no invariants beyond the default values; validation and well‑formedness constraints (such as uniqueness of `relative_path` or acyclic dependency relationships) are enforced externally during the planning phase. All members are directly settable, making `PagePlan` a straightforward data carrier between the planning and generation stages.

#### Invariants

- `page_id` uniquely identifies the page
- `page_type` defaults to `PageType::File`
- vectors are initially empty

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

- populated by generation frontend
- consumed by page generator
- stores inter-page dependencies and prompt specifications

### `clore::generate::PagePlanSet`

Declaration: `generate/model.cppm:50`

Definition: `generate/model.cppm:50`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::PagePlanSet` consists of two public data members: `plans`, a `std::vector<PagePlan>`, and `generation_order`, a `std::vector<std::string>`. These two vectors are maintained as parallel sequences: the element at index `i` in `generation_order` records the identifier (typically a page name) that corresponds to the plan at the same index in `plans`. This design couples the ordered sequence of generated pages with their associated plan objects, allowing callers to iterate or index them together. The implicit default constructor initializes both vectors to empty, which is the only valid starting state; no invariants are enforced beyond the implicit correspondence of indices, which must be preserved by any code that adds or removes elements in tandem.

#### Invariants

- plans and `generation_order` may be empty

#### Key Members

- plans
- `generation_order`

#### Usage Patterns

- used to store page plans and their generation order

### `clore::generate::PageType`

Declaration: `generate/model.cppm:9`

Definition: `generate/model.cppm:9`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `clore::generate::PageType` enum is defined with an underlying type of `std::uint8_t` to reduce memory footprint in the generation pipeline. The four enumerators (`Index`, `Module`, `Namespace`, `File`) form a closed set of page categories, and each value uniquely identifies the kind of documentation page being produced. No implicit ordering invariants are imposed beyond the exclusivity of each enumerator; the internal logic relies on these values to dispatch to appropriate page‑building routines or to influence template selection during code generation.

#### Invariants

- Each enumerator represents a distinct page type.
- The enum is scoped (`enum class`), so values must be qualified with `PageType::`.

#### Key Members

- `PageType::Index`
- `PageType::Module`
- `PageType::Namespace`
- `PageType::File`

#### Usage Patterns

- Used to select or identify the type of a page during documentation generation.
- May be used in switch statements or as a parameter to generation functions.

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

The struct `clore::generate::PathError` is a simple wrapper around a `std::string` member named `message`. No special invariants are enforced by the type itself; the `message` is expected to be populated with a human-readable description of the error that occurred during path generation. The implementation is trivial, as the struct has no user‑defined constructors, destructors, or member functions – it relies on the default compiler‑generated special members. There are no additional data members or base classes, making `PathError` a straightforward aggregate that serves only to convey error information.

#### Invariants

- `message` should contain a human-readable description of the error.
- `message` is expected to be non-empty for meaningful errors.

#### Key Members

- `message` of type `std::string`

#### Usage Patterns

- Returned or thrown by generation functions to indicate failure.
- Checked by callers to obtain error details.

### `clore::generate::PromptKind`

Declaration: `generate/model.cppm:18`

Definition: `generate/model.cppm:18`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The enum `clore::generate::PromptKind` is implemented as a scoped enumeration with an underlying type of `std::uint8_t`. It defines eleven distinct enumerators that represent categories of prompts used during code generation. The enumerators cover summaries (e.g., `NamespaceSummary`, `ModuleSummary`), analyses (e.g., `TypeAnalysis`, `FunctionAnalysis`, `VariableAnalysis`), declarations (e.g., `TypeDeclarationSummary`, `FunctionDeclarationSummary`), implementations (e.g., `TypeImplementationSummary`, `FunctionImplementationSummary`), and high‑level overviews (`ModuleArchitecture`, `IndexOverview`). While the enumeration does not enforce any particular invariant on the values themselves, the ordering of the enumerators is fixed and used by internal dispatch logic to select appropriate prompt templates for different code elements. The small underlying type and finite set of values allow the enum to be compactly stored and efficiently compared in switch statements and lookup structures.

#### Invariants

- All enumerator values are distinct
- Enum fits in `uint8_t` storage

#### Key Members

- `NamespaceSummary`
- `ModuleSummary`
- `ModuleArchitecture`
- `IndexOverview`
- `FunctionAnalysis`
- `TypeAnalysis`
- `VariableAnalysis`
- `FunctionDeclarationSummary`
- `FunctionImplementationSummary`
- `TypeDeclarationSummary`
- `TypeImplementationSummary`

#### Usage Patterns

- Used as a tag to select the appropriate prompt template or generation logic
- Passed to functions that construct or format prompts for LLM queries
- Enables `switch` statements to handle each kind distinctly

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

The struct `clore::generate::PromptRequest` is a simple aggregate used internally within the `clore::generate` module. It holds two data members: `kind` of type `PromptKind` (defaulting to `PromptKind::NamespaceSummary`) and `target_key` of type `std::string` (defaulting to an empty string). There are no invariants enforced beyond the language‑provided defaults; the struct relies on the caller to set the fields appropriately before use. The default member initialisers ensure that a `PromptRequest` constructed without explicit arguments represents a request for a namespace summary with no specific target.

#### Invariants

- Default values provide stable initial state for both fields.
- `target_key` may be empty; no constraints on its content.

#### Key Members

- `clore::generate::PromptRequest::kind`
- `clore::generate::PromptRequest::target_key`

#### Usage Patterns

- Passed to generation functions to specify what to document.
- Default-constructed for requests without a specific target.

### `clore::generate::RenderError`

Declaration: `generate/model.cppm:73`

Definition: `generate/model.cppm:73`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::RenderError` is a simple error type that stores a human-readable description of a rendering failure in its single data member `message`. No invariants are enforced beyond those of `std::string` itself—the string may be empty if no specific error text is provided, but in practice callers supply a meaningful message. The struct is publicly aggregate-initializable and implicitly defines all special member functions, so no constructors, assignment `operator`s, or destructors are explicitly implemented; the compiler-generated defaults suffice.

#### Invariants

- The `message` field is expected to be non-empty when an error is present, but this is not enforced.

#### Key Members

- `message` (`std::string`)

#### Usage Patterns

- Returned or thrown to indicate a failure in rendering; typically caught or handled by checking the message contents.

### `clore::generate::SymbolAnalysisStore`

Declaration: `generate/model.cppm:125`

Definition: `generate/model.cppm:125`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::SymbolAnalysisStore` is a simple aggregate serving as the central container for all analysis results generated during a symbol-processing pass. Its three public fields—`functions` (`FunctionAnalysisCache`), `types` (`TypeAnalysisCache`), and `variables` (`VariableAnalysisCache`)—are each a dedicated cache that holds the extracted metadata for the corresponding symbol kind. The struct imposes no specific invariant beyond the consistency guaranteed by the code that populates these caches; the fields are always filled together by the analysis pipeline and are only read after that pipeline completes. No explicit constructors or member functions are defined, so the object relies on default member initialization and aggregate initialization.

#### Invariants

- All three cache members are always initialized
- Each cache corresponds exclusively to a specific symbol category

#### Key Members

- `functions`
- `types`
- `variables`

#### Usage Patterns

- Used as a container for symbol analysis results in the generate module
- Accessed by other components to retrieve cached analysis data for functions, types, and variables

### `clore::generate::SymbolTargetKeyView`

Declaration: `generate/model.cppm:136`

Definition: `generate/model.cppm:136`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::SymbolTargetKeyView` is an aggregate struct that serves as a lightweight, non-owning key for identifying a symbol target. It holds two public `std::string_view` members: `qualified_name` and `signature`. The intended invariant is that both string views remain valid for the lifetime of the view instance, as they are assumed to point to externally managed strings (e.g., internal registries or computed values). No special member functions, constructors, or assignment `operator`s are defined; the struct relies on the compiler-generated defaults. Because it is a simple aggregate, initialization is performed via brace-enclosed list or designated initializers, and comparison or hashing behavior must be provided externally if needed. The implementation is purely a composition of two string views with no additional logic or runtime checks.

#### Invariants

- The struct is an aggregate and can be initialized with brace initialization.
- Both `qualified_name` and `signature` are non-owning views; the caller must ensure the underlying character data remains valid for the lifetime of the view.

#### Key Members

- `std::string_view qualified_name`
- `std::string_view signature`

#### Usage Patterns

- Used as a key or identifier for symbol targets, likely in lookup or comparison operations.
- Expected to be used in contexts where both the qualified name and signature are needed without copying strings.

### `clore::generate::TypeAnalysis`

Declaration: `generate/model.cppm:91`

Definition: `generate/model.cppm:91`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `clore::generate::TypeAnalysis` struct is a plain aggregate data container that captures the results of analyzing a C++ type. Its internal structure consists of two `std::string` fields, `overview_markdown` and `details_markdown`, which hold human-readable prose summaries at different levels of granularity, and three `std::vector<std::string>` fields: `invariants` documents constraints that must hold for the type, `key_members` lists noteworthy members, and `usage_patterns` records typical ways the type is used. There are no custom constructors, destructors, or member functions—the members are default-initialized and can be populated directly after instantiation. The invariants of the struct itself are minimal: each vector is expected to contain logically grouped, non‑duplicate entries that correspond to the category it represents, though no runtime checks enforce this contract. All important member implementations are implicit, relying on the compiler‑generated special member functions.

#### Invariants

- `overview_markdown` and `details_markdown` are Markdown fragments without headings or code fences
- invariants, `key_members`, and `usage_patterns` contain short phrases

#### Key Members

- `overview_markdown`
- `details_markdown`
- invariants
- `key_members`
- `usage_patterns`

#### Usage Patterns

- Cached and reused across documentation pages for namespace, module, file, and symbol contexts
- Populated by an analysis process that extracts information from source code

### `clore::generate::VariableAnalysis`

Declaration: `generate/model.cppm:99`

Definition: `generate/model.cppm:99`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::VariableAnalysis` is a plain data record that bundles all information produced during variable analysis into a single object. Its boolean `is_mutated` flags whether the variable undergoes any mutation, while `mutation_sources` lists the locations or reasons for those mutations; an invariant is that when `is_mutated` is false, `mutation_sources` must be empty, and when true the vector should contain at least one entry. The `usage_patterns` vector enumerates how the variable is used (e.g., reads, writes, or passing to functions), and the two markdown fields, `overview_markdown` and `details_markdown`, hold pre‑formatted textual summaries for different levels of presentation detail. No member functions modify these fields beyond construction or direct assignment, so the struct functions as a simple aggregator that the analysis pass fills and later stages consume unchanged.

#### Invariants

- `overview_markdown` and `details_markdown` hold Markdown text
- `is_mutated` reflects mutation state
- `mutation_sources` lists sources of mutation
- `usage_patterns` lists usage patterns

#### Key Members

- `overview_markdown` field
- `details_markdown` field
- `is_mutated` flag
- `mutation_sources` vector
- `usage_patterns` vector

#### Usage Patterns

- Populated by variable analysis routines
- Cached across module and file documentation
- Queried for generated documentation content

## Functions

### `clore::generate::analysis_details_markdown`

Declaration: `generate/model.cppm:157`

Definition: `generate/model.cppm:373`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::analysis_details_markdown` delegates its implementation entirely to the generic helper `analysis_markdown`, passing a field-accessor lambda that extracts the `details_markdown` member from the concrete analysis struct (one of `FunctionAnalysis`, `TypeAnalysis`, or `VariableAnalysis`). This mirrors the pattern of `analysis_overview_markdown`, which retrieves `overview_markdown` instead. The internal control flow is thus determined by `analysis_markdown`: it uses the `extract::SymbolInfo` to look up the appropriate analysis record from the `SymbolAnalysisStore`, then returns a pointer to the string obtained via the provided accessor. The only dependency beyond the store and symbol is the existence of `details_markdown` fields on the three analysis types and the correct dispatch in `analysis_markdown`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses` (const `SymbolAnalysisStore`&)
- `symbol` (const `extract::SymbolInfo`&)

#### Usage Patterns

- Used to fetch the detailed analysis markdown for symbol documentation pages
- Called during page generation to include details in analysis sections

### `clore::generate::analysis_markdown`

Declaration: `generate/model.cppm:342`

Definition: `generate/model.cppm:342`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::analysis_markdown` first constructs a `target_key` by calling `make_symbol_target_key` on the provided `symbol`. It then examines `symbol.kind` through three kind‑predicate checks in order: `is_function_kind`, `is_type_kind`, and `is_variable_kind`. For each matching kind, it performs a lookup into the `analyses` store using the corresponding `find_function_analysis`, `find_type_analysis`, or `find_variable_analysis` helper. If a valid `analysis` pointer is obtained, the function applies the `field_accessor` functor to that analysis object and returns the resulting `const std::string*`. On any lookup failure or if the symbol kind does not fall into one of the three categories, the function returns `nullptr`. This dispatching relies on the `make_symbol_target_key` function, the kind‑testing predicates (`is_function_kind`, `is_type_kind`, `is_variable_kind`), and the respective `find_*_analysis` functions defined in the same module.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- analyses
- symbol
- `field_accessor`

#### Usage Patterns

- Used to extract overview or details markdown by passing a corresponding member pointer or lambda.

### `clore::generate::analysis_overview_markdown`

Declaration: `generate/model.cppm:154`

Definition: `generate/model.cppm:366`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::analysis_overview_markdown` is a thin delegation to the generic template `clore::generate::analysis_markdown`. It passes a field accessor lambda that, given a symbol analysis object (of any supported kind), returns a pointer to that object’s `overview_markdown` member. The internal control flow is entirely governed by `analysis_markdown`, which uses the provided `extract::SymbolInfo` and `SymbolAnalysisStore` to dispatch to the correct analysis struct—`TypeAnalysis`, `FunctionAnalysis`, or `VariableAnalysis`—based on the symbol’s kind, and then invokes the lambda on that struct. If the symbol is not found or the analysis lacks an overview, the lambda yields `nullptr` and the function returns `nullptr`. Dependencies include the `analysis_markdown` template, the analysis store, and the `overview_markdown` field present on each of the three analysis struct types.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `clore::generate::SymbolAnalysisStore&`
- `clore::extract::SymbolInfo&`

#### Usage Patterns

- Called by documentation generation code to obtain the overview section for a symbol.

### `clore::generate::build_link_resolver`

Declaration: `generate/model.cppm:201`

Definition: `generate/model.cppm:471`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function iterates over each `plan` in `plan_set.plans` and populates a `LinkResolver` by recording the `plan.title` under `plan.page_id` in `resolver.page_id_to_title`. For every key in `plan.owner_keys`, it uses `emplace` on `resolver.name_to_path` so that the first mapping for any given key is preserved, preventing later duplicates from overwriting. If `plan.page_type` equals `PageType::Namespace`, it additionally registers the same path in `resolver.namespace_to_path`; if it equals `PageType::Module`, it does so in `resolver.module_to_path`. After processing the owner keys, if `plan.page_id` contains a colon, the function extracts the substring after the colon and repeats the same insertion logic—including the type‑specific conditional maps—for that suffix. This two‑phase registration ensures that abbreviated names (e.g., a symbol name without its enclosing namespace) are also resolvable.

The function relies on the `emplace` semantics of the internal maps to implement a first‑mapping‑wins policy, which avoids overwriting when a module and namespace share the same qualified name. No validation or conflict detection is performed beyond this implicit deduplication, and the returned `LinkResolver` is built directly from the provided `PagePlanSet` without further transformation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan_set` (parameter)
- `plan_set.plans`
- each plan's `page_id`, `title`, `owner_keys`, `relative_path`, `page_type`

#### Writes To

- `resolver.page_id_to_title`
- `resolver.name_to_path`
- `resolver.namespace_to_path`
- `resolver.module_to_path`

#### Usage Patterns

- called to create a `LinkResolver` from a `PagePlanSet` before link resolution
- used in page generation pipeline to enable ID-to-path and key-to-path lookups
- callers should use `resolve_module`/`resolve_namespace` for disambiguation when generic `name_to_path` may conflict

### `clore::generate::compute_page_path`

Declaration: `generate/model.cppm:214`

Definition: `generate/model.cppm:576`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function dispatches on `identity.page_type` to construct the output path. For `PageType::Index`, it directly assigns the constant `kIndexPath`. For `PageType::Module`, it splits `identity.qualified_name` on `"."` using `split_qualified`, optionally extracts a partition after a colon from the last segment, then prepends `kModulePrefix` and normalizes each part via `normalize_name`, finally appending either `"index.md"` or a partition-based filename. The `PageType::Namespace` branch follows a similar pattern: split on `"::"`, prepend `kNamespacePrefix`, normalize each component, and append `"index.md"`. For `PageType::File`, it strips the extension from `identity.source_relative_path` (using the last dot) and prepends `kFilePrefix` plus a slash and appends `".md"`.

After path construction, `sanitize_path_chars` modifies the path in place, and `validate_path_component` performs final validation, returning `std::unexpected(PathError)` if invalid. The function depends on helpers in the anonymous namespace: `split_qualified`, `normalize_name`, `join_path`, and the constants `kIndexPath`, `kModulePrefix`, `kNamespacePrefix`, and `kFilePrefix`. It also relies on `sanitize_path_chars` and `validate_path_component` for post-processing.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `identity` of type `const PageIdentity &`
- global constant `kIndexPath`
- global constant `kModulePrefix`
- global constant `kNamespacePrefix`
- global constant `kFilePrefix`
- helper function `split_qualified`
- helper function `normalize_name`
- helper function `join_path`
- helper function `sanitize_path_chars`
- helper function `validate_path_component`

#### Usage Patterns

- called during page generation to determine output file paths
- used by `write_page` and related functions

### `clore::generate::find_function_analysis`

Declaration: `generate/model.cppm:145`

Definition: `generate/model.cppm:323`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function performs a single map lookup on `analyses.functions` using the provided `symbol_target_key`. If the key is present, it returns a pointer to the associated `FunctionAnalysis`; otherwise it returns `nullptr`. The only dependency is the `SymbolAnalysisStore` structure and its member `functions`, which is assumed to be an associative container keyed by symbol target key strings.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses.functions` map

#### Usage Patterns

- retrieve cached function analysis
- check if analysis exists for a function symbol target key

### `clore::generate::find_type_analysis`

Declaration: `generate/model.cppm:148`

Definition: `generate/model.cppm:329`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Inside, `clore::generate::find_type_analysis` directly invokes `analyses.types.find(symbol_target_key)` to locate a previously stored `TypeAnalysis` record. If the iterator `it` does not equal `analyses.types.end()`, the function returns a pointer to `it->second`; otherwise it returns `nullptr`. No other container lookups or branching occur. This implementation mirrors its counterparts `find_function_analysis` and `find_variable_analysis`, relying entirely on the associative container’s lookup performance and the correctness of the `symbol_target_key` identifier.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- analyses`.types`

#### Usage Patterns

- Retrieving type analysis from store
- Looking up type analysis by key

### `clore::generate::find_variable_analysis`

Declaration: `generate/model.cppm:151`

Definition: `generate/model.cppm:335`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function performs a single map lookup on the `analyses.variables` member, which is a container keyed by symbol target key (a `std::string_view`). It returns a pointer to the corresponding `VariableAnalysis` if found, or `nullptr` otherwise. The control flow is linear: a direct call to `find` on the map, followed by a ternary condition on the iterator equality with `end`. Dependencies are limited to the `SymbolAnalysisStore` data structure and its `variables` field; no other external functions or complex branching is involved.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses.variables` map
- `symbol_target_key` parameter

#### Usage Patterns

- Look up variable analysis by symbol target key

### `clore::generate::is_function_kind`

Declaration: `generate/model.cppm:162`

Definition: `generate/model.cppm:393`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::is_function_kind` implements a simple classification switch over an `extract::SymbolKind` value. The switch explicitly matches `extract::SymbolKind::Function` and `extract::SymbolKind::Method`, returning `true` for those two cases; all other `extract::SymbolKind` enumerators fall through to the `default` branch, which returns `false`. This internal control flow is a straightforward discriminative check with no additional dependencies or side effects, making the function suitable for use in filter predicates or as part of a dispatch chain that distinguishes function-like symbols from other symbol kinds.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `kind`

#### Usage Patterns

- Called to determine if a symbol kind corresponds to a function or method.

### `clore::generate::is_page_level_symbol`

Declaration: `generate/model.cppm:166`

Definition: `generate/model.cppm:405`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function applies a series of rejection checks to decide whether a given `extract::SymbolInfo` should be treated as a page-level symbol. It first examines `sym.lexical_parent_kind`: if the value is not `extract::SymbolKind::Unknown` and not `extract::SymbolKind::Namespace`, the symbol is considered nested inside a function or type and the function returns `false`. Next it searches both `sym.enclosing_namespace` and `sym.lexical_parent_name` for the substring `"(anonymous namespace)"`; if found, the symbol is treated as an implementation detail and rejected. Finally, if `sym.parent` is present, it uses `extract::lookup_symbol` to retrieve the parent symbol; if the parent’s `kind` is classified as a type or function via `is_type_kind` or `is_function_kind`, the function returns `false`. Only when all these conditions are satisfied does it return `true`, indicating that the symbol is a top‑level public element suitable for its own generated page.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model`
- `sym.lexical_parent_kind`
- `sym.enclosing_namespace`
- `sym.lexical_parent_name`
- `sym.parent`
- `extract::lookup_symbol(model, *sym.parent)`

#### Usage Patterns

- Used during page planning to decide which symbols should have dedicated documentation pages.

### `clore::generate::is_page_summary_prompt`

Declaration: `generate/model.cppm:133`

Definition: `generate/model.cppm:297`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::is_page_summary_prompt` implements a simple predicate that returns `true` only when the input `kind` equals either `PromptKind::NamespaceSummary` or `PromptKind::ModuleSummary`. Its internal control flow consists of a single logical disjunction of two equality comparisons against the `PromptKind` enum members. The function depends solely on the `PromptKind` enumeration and serves as a discriminator to identify prompts that correspond to top‑level page summaries for namespaces or modules, as opposed to symbol‑level analysis prompts.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `kind` parameter (`PromptKind`)

#### Usage Patterns

- Used to identify page summary prompts in generation logic
- Called to branch behavior for namespace or module summary prompts

### `clore::generate::is_symbol_analysis_prompt`

Declaration: `generate/model.cppm:134`

Definition: `generate/model.cppm:301`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::is_symbol_analysis_prompt` implements a simple membership test against the `clore::generate::PromptKind` enumeration. Its algorithm consists of a single logical disjunction: it returns `true` if the input `kind` is equal to either `clore::generate::PromptKind::FunctionAnalysis`, `clore::generate::PromptKind::TypeAnalysis`, or `clore::generate::PromptKind::VariableAnalysis`; otherwise it returns `false`. The control flow is direct and uses no branching beyond the short-circuit evaluation of the `||` `operator`s. The only dependency is the `clore::generate::PromptKind` enum and its three relevant enumerators, making the function a lightweight compile‑time constant check.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `kind` of type `PromptKind`

#### Usage Patterns

- used to categorize prompt kinds for symbol analysis
- used in conditional logic to dispatch analysis generation

### `clore::generate::is_type_kind`

Declaration: `generate/model.cppm:160`

Definition: `generate/model.cppm:380`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::is_type_kind` implements a mapping from a given `extract::SymbolKind` to a boolean value by means of a switch statement. The control flow enumerates the specific symbol kinds that represent type-level entities—`extract::SymbolKind::Class`, `extract::SymbolKind::Struct`, `extract::SymbolKind::Enum`, `extract::SymbolKind::Union`, `extract::SymbolKind::Concept`, `extract::SymbolKind::Template`, and `extract::SymbolKind::TypeAlias`—each of which yields a return of `true`. Any other value of `extract::SymbolKind` falls through to the default case and returns `false`. The function has no external dependencies beyond the underlying symbol-kind enumeration and serves as a simple classification dispatch for later branching in page planning or prompt generation logic.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `kind` parameter of type `extract::SymbolKind`

#### Usage Patterns

- Used to classify symbol kinds as type definitions

### `clore::generate::is_variable_kind`

Declaration: `generate/model.cppm:164`

Definition: `generate/model.cppm:401`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::is_variable_kind` performs a simple classification by comparing its `extract::SymbolKind` parameter against two specific enumeration members. It returns `true` only when the given `kind` is either `extract::SymbolKind::Variable` or `extract::SymbolKind::EnumMember`; otherwise it returns `false`. The logic uses a direct equality check and a logical OR, with no branching or additional state. Its sole dependency is the `extract::SymbolKind` enumeration, which is used to define the two recognized symbol kinds. This function is used internally by generation logic to differentiate variable‑like symbols from other symbol categories (e.g., types or functions) without requiring knowledge of the broader symbol analysis structures.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `extract::SymbolKind kind` parameter

#### Usage Patterns

- categorizing symbols
- filtering symbol kinds
- checking if a symbol is variable-like

### `clore::generate::make_source_relative`

Declaration: `generate/model.cppm:169`

Definition: `generate/model.cppm:432`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::make_source_relative` computes a filesystem‑relative path from an absolute path to a project root directory. It first constructs a composite key from the two input strings and checks an internal LRU‑style cache (`source_relative_cache`) under a shared lock; if a cached result exists, it is returned immediately. Otherwise it normalises both paths via `std::filesystem::path::lexically_normal` and invokes `lexically_relative` on the root. If the produced relative path is empty or begins with `..`, the original absolute path is retained as a fallback; otherwise the relative path in generic form is used. The computed result is then stored in the cache under a unique lock before being returned. The function depends solely on the module‑local `source_relative_cache` singleton, which provides a thread‑safe `SourceRelativeCache` with a `mutex` and a `relative_by_key` map.

#### Side Effects

- Updates `source_relative_cache` by inserting a mapping from the constructed key to the resolved relative path.

#### Reads From

- `path` parameter
- `project_root` parameter
- `source_relative_cache` cache store

#### Writes To

- `source_relative_cache` cache store (key-value pair added)

#### Usage Patterns

- Used to convert absolute source paths to project-relative paths for documentation
- Called during page generation to produce link targets or paths

### `clore::generate::make_symbol_target_key`

Declaration: `generate/model.cppm:141`

Definition: `generate/model.cppm:306`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::make_symbol_target_key` produces a length-prefixed compound key from an `extract::SymbolInfo` object. It first uses `std::format` to write the size of `symbol.qualified_name` followed by a colon and the qualified name itself into a `std::string`. The formatted string is then extended by appending `symbol.signature` directly. The resulting key packs both the symbol’s fully qualified name (with its length for unambiguous parsing) and its signature into a single contiguous string, enabling later decomposition via `parse_symbol_target_key` into a `SymbolTargetKeyView`. The function has no control flow branches; its entire logic is a linear sequence of two concatenation operations, relying on `std::format` for the name portion and `std::string::operator+=` for the signature. It depends on the `extract::SymbolInfo` data type and the C++20 standard library formatting facility.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- symbol`.qualified_name`
- symbol`.signature`

#### Usage Patterns

- generating lookup keys for symbol caches
- creating unique identifiers for symbols
- building keys for prompt requests

### `clore::generate::page_type_name`

Declaration: `generate/model.cppm:16`

Definition: `generate/model.cppm:263`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function implements a direct mapping from the `PageType` enumerator to its string representation. It uses a `switch` statement over the given `type` and returns a `std::string_view` literal for each known enumerator: `PageType::Index` maps to `"index"`, `PageType::Module` to `"module"`, `PageType::Namespace` to `"namespace"`, and `PageType::File` to `"file"`. If an unrecognized value is passed, the fallback `"unknown"` is returned. The control flow is a straightforward single-level dispatch with no branching or external dependencies; the function is essentially a static lookup table.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the input parameter `type` of type `PageType`

#### Usage Patterns

- Used to convert a `PageType` to a string for documentation page naming or diagnostics.

### `clore::generate::parse_symbol_target_key`

Declaration: `generate/model.cppm:143`

Definition: `generate/model.cppm:312`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function attempts to decode a length‑prefixed encoding first by calling `parse_length_prefixed_symbol_target_key`. If that helper returns a value, the result is forwarded directly as the `SymbolTargetKeyView`. Otherwise, the fallback path constructs a `SymbolTargetKeyView` by assigning the entire `target_key` to the `qualified_name` member and leaving the `signature` member empty. This two‑branch strategy enables the function to handle both compact encoded keys (where qualified name and signature are separated and length‑prefixed) and plain qualified names that lack an explicit signature component. The fallback relies solely on the default member initialization of `SymbolTargetKeyView`, and the only external call is to `parse_length_prefixed_symbol_target_key`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `target_key` parameter of type `std::string_view`

#### Usage Patterns

- Parsing symbol target key strings into structured views
- Used during symbol identification from input

### `clore::generate::prompt_kind_name`

Declaration: `generate/model.cppm:32`

Definition: `generate/model.cppm:273`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::prompt_kind_name` implements a straightforward mapping from the `PromptKind` enum to a human-readable `std::string_view`. Its internal control flow consists of a single `switch` statement that covers every known enumerator — from `PromptKind::NamespaceSummary` through `PromptKind::TypeImplementationSummary` — each returning a corresponding `snake_case` literal (e.g., `"function_analysis"`, `"module_summary"`). After the switch, a default fallback returns `"unknown_prompt"`, handling any unrecognized or future `PromptKind` values gracefully. The only dependency is the `PromptKind` enum itself, and the function contains no branching, loops, or external calls; it is a pure, O(1) lookup suitable for serialization, logging, or prompt key generation throughout the generation pipeline.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `kind` parameter

#### Usage Patterns

- called to obtain string key for prompt kind in caching or evidence-building logic
- used in context where prompt kind needs to be logged or stored as text

### `clore::generate::prompt_request_key`

Declaration: `generate/model.cppm:131`

Definition: `generate/model.cppm:290`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::prompt_request_key` computes a unique string key for a given `PromptRequest` by evaluating its `target_key` field. If `request.target_key` is empty, the key is simply the result of `prompt_kind_name(request.kind)`. Otherwise, it returns the concatenation `prompt_kind_name(request.kind) + ":" + request.target_key`. This branch-then-concatenate pattern ensures that requests without a specific target (e.g., index or module overviews) are identified solely by their `PromptKind`, while symbol‑specific requests include the target‑key suffix for precise caching and lookup. The function depends on the `prompt_kind_name` helper to convert the `PromptKind` enumerator to a string and on the `PromptRequest` structure’s `kind` and `target_key` fields.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `request.kind`
- `request.target_key`
- the `prompt_kind_name` function (called internally)

#### Usage Patterns

- generating a cache key or identifier for a prompt request
- used when a unique string representation of a `PromptRequest` is needed

### `clore::generate::validate_no_path_conflicts`

Declaration: `generate/model.cppm:216`

Definition: `generate/model.cppm:644`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function iterates over a collection of `(path, id)` pairs stored in `path_to_id` and uses an `std::unordered_map<std::string, std::string> seen` to detect collisions. For each pair, it calls `seen.emplace(path, id)`; if the returned `inserted` flag is `false`, a duplicate path has been encountered. In that case, it returns `std::unexpected(PathError{.message = ...})` containing a formatted string that identifies both the existing ID (`it->second`) and the conflicting ID along with the duplicate path. If all paths are unique, the function returns an empty `std::expected<void, PathError>`. The algorithm is a straightforward linear scan with unordered‑map lookup, relying on `std::format` for error message construction and on the `PathError` struct for conveying the conflict.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `path_to_id` parameter (vector of path-id pairs)

#### Usage Patterns

- Called before page generation to ensure unique output paths
- Used to validate configuration or mapping inputs

## Internal Structure

The `generate:model` module defines the core data types and interfaces that represent the state and structure of the documentation generation pipeline. It is decomposed into several logical groups: page identity and page type enumerations (`PageIdentity`, `PageType`), prompt request and prompt kind descriptions (`PromptRequest`, `PromptKind`), symbol analysis records for functions, types, and variables (`FunctionAnalysis`, `TypeAnalysis`, `VariableAnalysis`), planning structures that describe which pages to produce (`PagePlan`, `PagePlanSet`), error types for distinct failure modes (`GenerateError`, `PathError`, `RenderError`), and the link resolver that maps entity names to output paths (`LinkResolver`). The module imports `extract` for analysis data, `std` for standard library facilities, and `support` for utilities. Internally, it employs anonymous namespaces to encapsulate helper types and functions (e.g., `SourceRelativeCache` for path computations) and provides a set of free functions (`find_*_analysis`, `analysis_markdown`, `compute_page_path`, etc.) that operate on these model types, forming a well‑separated layer between extraction and rendering.

## Related Pages

- [Module extract](../extract/index.md)
- [Module support](../support/index.md)

