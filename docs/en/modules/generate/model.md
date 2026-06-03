---
title: 'Module generate:model'
description: 'The generate:model module defines the core data types and functions that underpin the documentation generation pipeline. Its responsibility is to represent the intermediate and final artifacts of the generation process, including page identities and types (e.g., PageType, PageIdentity, PagePlan, GeneratedPage), symbol-level analyses (FunctionAnalysis, TypeAnalysis, VariableAnalysis), prompt classification (PromptKind, PromptRequest), and error handling (PathError, GenerateError, RenderError). The module also provides the LinkResolver for mapping entity names to page-relative paths and a SymbolAnalysisStore to aggregate analysis results. Together, these elements form the shared vocabulary used by planning, rendering, and output stages.'
layout: doc
template: doc
---

# Module `generate:model`

## Summary

The `generate:model` module defines the core data types and functions that underpin the documentation generation pipeline. Its responsibility is to represent the intermediate and final artifacts of the generation process, including page identities and types (e.g., `PageType`, `PageIdentity`, `PagePlan`, `GeneratedPage`), symbol-level analyses (`FunctionAnalysis`, `TypeAnalysis`, `VariableAnalysis`), prompt classification (`PromptKind`, `PromptRequest`), and error handling (`PathError`, `GenerateError`, `RenderError`). The module also provides the `LinkResolver` for mapping entity names to page-relative paths and a `SymbolAnalysisStore` to aggregate analysis results. Together, these elements form the shared vocabulary used by planning, rendering, and output stages.

The module’s public interface encompasses enumerations for page types and prompt kinds, data structures for page plans and generated pages, analysis stores and symbol-specific analysis structs, the link resolver, error types, and a set of utility functions. These utilities include key construction (`make_symbol_target_key`, `prompt_request_key`), analysis lookups (`find_function_analysis`, `find_type_analysis`, `find_variable_analysis`), path computation (`compute_page_path`, `make_source_relative`), conflict validation, and predicates for symbol kind classification or prompt type detection. Any consumer of this module can rely on these types and functions to build, query, or manipulate the generation model without accessing internal implementation details.

## Imports

- [`extract`](../extract/index.md)
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

Declaration: `src/generate/model.cppm:97`

Definition: `src/generate/model.cppm:97`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::FunctionAnalysis` is an internal aggregation type that holds all derived information about a single function after analysis. Its fields are populated by the analysis pass and consumed by generation. The boolean `has_side_effects` acts as a quick predicate; when `true`, the `side_effects` vector stores human-readable descriptions of each observed effect. The `reads_from` and `writes_to` vectors record which resources the function accesses, enabling the generator to produce accurate documentation about data flow. The `usage_patterns` list captures typical invocation examples or idiomatic uses extracted from the source. Finally, `overview_markdown` and `details_markdown` contain pre-formatted Markdown text for final documentation, keeping presentation logic separate from analysis. An invariant is that the length of `side_effects` must be non‑zero when `has_side_effects` is `true`.

#### Invariants

- Default value of `has_side_effects` is false
- All vector fields may be empty
- Strings contain plain markdown fragments

#### Key Members

- `overview_markdown`
- `details_markdown`
- `has_side_effects`
- `side_effects`
- `reads_from`
- `writes_to`
- `usage_patterns`

#### Usage Patterns

- Cached and reused across namespace, module, file, and symbol documentation pages
- Populated by code analysis passes
- Consumed by documentation generators

### `clore::generate::GenerateError`

Declaration: `src/generate/model.cppm:85`

Definition: `src/generate/model.cppm:85`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::GenerateError` is a lightweight error type that stores a human-readable description as a `std::string` in the single data member `message`. The struct relies on the compiler‑generated special member functions—default constructor, copy constructor, move constructor, copy assignment, move assignment, and destructor—all of which are implicitly defined because `message` is a well‑behaved `std::string`. No custom constructors, assignment `operator`s, or resource management logic are provided; the invariant is that `message` always contains a valid, movable string that describes the error that occurred. The simplicity of the structure makes it cheap to copy or move error objects, and no additional state or invariants beyond the contained text need to be enforced.

#### Invariants

- The `message` field is intended to be non-empty when the struct is used to represent an actual error.

#### Key Members

- `message`

#### Usage Patterns

- Returned or thrown from generation functions to indicate failure.
- Inspected by callers to obtain error details.

### `clore::generate::GeneratedPage`

Declaration: `src/generate/model.cppm:71`

Definition: `src/generate/model.cppm:71`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::GeneratedPage` serves as a trivial data aggregate that holds the output of a single page generation step. It contains three `std::string` fields: `title`, `relative_path`, and `content`. Each field is default-initialized to an empty string, and the struct itself imposes no invariants beyond those inherent to the string types; any sequence of characters is valid for each member. The `relative_path` is intended to represent the file path of the generated page relative to the output root, while `title` and `content` store the page's heading and full markup respectively. The struct is trivially copyable and movable, making it efficient to return from generation functions or to store in containers.

#### Invariants

- All fields are default-initialized to empty strings.

#### Key Members

- `title`
- `relative_path`
- `content`

#### Usage Patterns

- Returned from page generation functions.
- Passed to serialization or file writing routines.

### `clore::generate::GenerationSummary`

Declaration: `src/generate/model.cppm:77`

Definition: `src/generate/model.cppm:77`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The aggregate `clore::generate::GenerationSummary` consists of five `std::size_t` fields, each initialized to zero by default member initializers. These counters track the number of generated outputs (`written_output_count`) and the performance of two distinct caching layers: one for symbol analysis results (`symbol_analysis_cache_hits`, `symbol_analysis_cache_misses`) and one for page prompts (`page_prompt_cache_hits`, `page_prompt_cache_misses`). The primary invariant is that each counter is non‑negative and monotonically increases during a single generation session; no field is expected to be decremented or reset by external callers. The implementation is limited to the default construction and aggregate initialization, relying on the trivial initialization guarantees of `std::size_t` to provide zero‑starting values without explicit constructor code. No member functions or access restrictions are defined, making the struct a plain data aggregate intended for direct field accumulation by the generation pipeline.

#### Invariants

- All fields are initialized to zero by default
- Counters are non-negative integers

#### Key Members

- `written_output_count`
- `symbol_analysis_cache_hits`
- `symbol_analysis_cache_misses`
- `page_prompt_cache_hits`
- `page_prompt_cache_misses`

#### Usage Patterns

- Collected and read by generation logic to report performance statistics
- Used as a return or output parameter from generation functions

### `clore::generate::LinkResolver`

Declaration: `src/generate/model.cppm:190`

Definition: `src/generate/model.cppm:190`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Internally, `clore::generate::LinkResolver` maintains four unordered maps that serve as lookup tables: `name_to_path` for general entity names, `namespace_to_path` for qualified namespace and type names, `module_to_path` for module names, and `page_id_to_title` for mapping page identifiers to their display titles. Each map associates a `std::string` key with a `std::string` value representing a page-relative path or title. The resolver imposes no ordering or uniqueness constraints beyond those of `std::unordered_map`; the maps must be populated by the caller before any resolution is performed.

The four `resolve*` member functions (`resolve`, `resolve_namespace`, `resolve_module`, `resolve_page_title`) each accept a key, perform a find in the corresponding map, and return a `const std::string*` pointing to the stored value if found, or `nullptr` if the key is absent. This design avoids copying strings and directly exposes internal storage, so callers must not write through or extend the lifetime of the returned pointer beyond that of the map entry. The use of `[[nodiscard]]` encourages callers to check the pointer for validity, reinforcing the invariant that a missing name yields a null pointer rather than a default value.

#### Invariants

- All maps are fully populated before any resolve call.
- Keys in each map are unique.
- Returned pointers remain valid as long as the map is not modified.

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

- Called during documentation generation to resolve cross-reference links.
- Used to convert entity names to output-relative paths.
- Supports separate lookups for names, namespaces, modules, and page titles.

#### Member Functions

##### `clore::generate::LinkResolver::resolve`

Declaration: `src/generate/model.cppm:196`

Definition: `src/generate/model.cppm:196`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
[[nodiscard]] auto resolve(const std::string& name) const -> const std::string* {
        auto it = name_to_path.find(name);
        return it != name_to_path.end() ? &it->second : nullptr;
    }
```

##### `clore::generate::LinkResolver::resolve_module`

Declaration: `src/generate/model.cppm:206`

Definition: `src/generate/model.cppm:206`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
[[nodiscard]] auto resolve_module(const std::string& name) const -> const std::string* {
        auto it = module_to_path.find(name);
        return it != module_to_path.end() ? &it->second : nullptr;
    }
```

##### `clore::generate::LinkResolver::resolve_namespace`

Declaration: `src/generate/model.cppm:201`

Definition: `src/generate/model.cppm:201`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
[[nodiscard]] auto resolve_namespace(const std::string& name) const -> const std::string* {
        auto it = namespace_to_path.find(name);
        return it != namespace_to_path.end() ? &it->second : nullptr;
    }
```

##### `clore::generate::LinkResolver::resolve_page_title`

Declaration: `src/generate/model.cppm:211`

Definition: `src/generate/model.cppm:211`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
[[nodiscard]] auto resolve_page_title(const std::string& page_id) const -> const std::string* {
        auto it = page_id_to_title.find(page_id);
        return it != page_id_to_title.end() ? &it->second : nullptr;
    }
```

### `clore::generate::MarkdownFragmentResponse`

Declaration: `src/generate/model.cppm:93`

Definition: `src/generate/model.cppm:93`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::MarkdownFragmentResponse` is a simple aggregate type with a single data member, `std::string markdown`. Its implementation relies entirely on compiler-generated special member functions (default constructor, copy and move constructors, copy and move assignment `operator`s, and destructor), as no user-defined implementations are provided. The struct acts as a lightweight value carrier for a markdown text fragment, with no internal methods or validation; all operations on the contained string must be performed externally.

The only implicit invariant is that the `markdown` member is a valid `std::string` object, but no additional constraints (such as non-emptiness or well-formed markdown) are enforced by the type itself. This design keeps the struct minimal and focused on data transfer, with no state management or behavioral logic beyond the default operations inherited from its member.

### `clore::generate::PageIdentity`

Declaration: `src/generate/model.cppm:223`

Definition: `src/generate/model.cppm:223`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::PageIdentity` aggregates four fields that collectively capture the full identity of a generated documentation page. Its internal structure consists of a `PageType` enumerator (defaulting to `PageType::File`) to classify the kind of page, a `normalized_owner_key` string that likely designates the owning entity (e.g., a module or namespace) in a normalized form, a `qualified_name` string holding the complete fully qualified name of the documented entity, and a `source_relative_path` string storing the path of the original source relative to the project root. All three string fields default to empty, ensuring that every instance is safely constructed without uninitialized data. The struct serves purely as a passive aggregate without member functions beyond the compiler‑generated ones, so its invariants are maintained solely by consistent assignment of these four fields at creation time.

#### Invariants

- Fields are default-initialized.
- No explicit invariants documented.

#### Key Members

- `page_type`
- `normalized_owner_key`
- `qualified_name`
- `source_relative_path`

#### Usage Patterns

- Not provided in evidence.

### `clore::generate::PagePlan`

Declaration: `src/generate/model.cppm:55`

Definition: `src/generate/model.cppm:55`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `PagePlan` struct aggregates metadata and configuration for a single page in a generated documentation set. Its fields partition planning information into identification (`page_id`), classification (`page_type`), content descriptors (`title`, `relative_path`), ownership and dependency tracking (`owner_keys`, `depends_on_pages`, `linked_pages`), and generation inputs (`prompt_requests`). All fields have default values, ensuring that a default-constructed `PagePlan` is valid but incomplete. Key invariants include that `page_id` should be unique within a generation run, `relative_path` must correspond to a valid filesystem location when resolved, and the vectors `depends_on_pages` and `linked_pages` refer to existing `page_id` values elsewhere in the plan to form a consistent dependency graph. The `prompt_requests` vector holds `PromptRequest` objects that drive the LLM‑based content generation for this page.

#### Invariants

- Fields are initialized to default values (empty strings, File type, empty vectors).
- No internal invariants are enforced; valid values depend on the caller.

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

- Constructed and populated by code that determines page generation requirements.
- Passed to a generator function that processes the plan to produce final page output.

### `clore::generate::PagePlanSet`

Declaration: `src/generate/model.cppm:66`

Definition: `src/generate/model.cppm:66`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::PagePlanSet` is an aggregate that bundles two parallel vectors: `plans`, which holds the `PagePlan` objects, and `generation_order`, which stores the string identifiers (typically page names) in the order they were generated. Its purpose is to maintain a strict correspondence between each generated page plan and its position in the sequence. An invariant of the class is that `plans.size() == generation_order.size()`, ensuring that every plan has an associated identifier and that the index of a plan in `plans` matches the index of its name in `generation_order`. No member functions are defined; the struct relies on default initialization and direct field access.

#### Invariants

- Both `plans` and `generation_order` are default-constructed as empty
- No further invariants are specified

#### Key Members

- `plans`: a vector of `PagePlan` objects
- `generation_order`: a vector of strings indicating generation order

#### Usage Patterns

- Used to represent a set of page generation plans with an associated ordering
- Likely consumed by code generation logic that processes pages in the specified order

### `clore::generate::PageType`

Declaration: `src/generate/model.cppm:25`

Definition: `src/generate/model.cppm:25`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `clore::generate::PageType` enum is implemented as a scoped enumeration with an underlying `std::uint8_t` storage, reflecting its use as a lightweight category tag for generated documentation pages. The enumerators `Index`, `Module`, `Namespace`, and `File` appear in that declaration order, which defines their integer values implicitly from 0 to 3. No additional member functions or custom values are provided; the internal structure is purely a fixed set of distinct identifiers. The choice of `std::uint8_t` ensures the enum occupies exactly one byte, suiting it for use in packed data structures or as a discriminator in variant-like designs without wasting space. No invariants are enforced beyond the standard guarantee that each enumerator is a distinct constant of the enum type.

#### Member Variables

##### `clore::generate::PageType::File`

Declaration: `src/generate/model.cppm:29`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
File
```

##### `clore::generate::PageType::Index`

Declaration: `src/generate/model.cppm:26`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Index
```

##### `clore::generate::PageType::Module`

Declaration: `src/generate/model.cppm:27`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Module
```

##### `clore::generate::PageType::Namespace`

Declaration: `src/generate/model.cppm:28`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Namespace
```

### `clore::generate::PathError`

Declaration: `src/generate/model.cppm:219`

Definition: `src/generate/model.cppm:219`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::PathError` stores a single `std::string message` field, which holds a human‑readable description of an error. Its internal structure is minimal: no virtual functions, no user‑defined constructors, destructors, or assignment `operator`s, so all special member functions are compiler‑generated. The only invariant is that `message` contains a non‑empty string when the object is used to report an actual error; otherwise the default‑constructed empty string is valid as a sentinel. The absence of additional state keeps the type trivially copyable and suitable for use in exception‑handling or return‑value paths without external indirection.

#### Key Members

- message

#### Usage Patterns

- Returned or thrown by path generation functions to indicate failure
- Inspected by callers to retrieve the error description

### `clore::generate::PromptKind`

Declaration: `src/generate/model.cppm:34`

Definition: `src/generate/model.cppm:34`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `clore::generate::PromptKind` enum is an implementation-internal discriminator that classifies the kind of analysis or summary a generator prompt represents. It is defined as an `enum class` with an underlying `std::uint8_t` type, ensuring compact storage and value range safety. The enumerators — `NamespaceSummary`, `ModuleSummary`, `ModuleArchitecture`, `IndexOverview`, `FunctionAnalysis`, `TypeAnalysis`, `VariableAnalysis`, `FunctionDeclarationSummary`, `FunctionImplementationSummary`, `TypeDeclarationSummary`, and `TypeImplementationSummary` — collectively cover the distinct prompt categories required by the generation pipeline. As a closed enumeration, it maintains the invariant that every valid prompt must correspond to exactly one of these values, enabling switch‑based dispatch and exhaustive pattern matching without a fallback.

#### Invariants

- All enumerator values are distinct within the enum.
- The enum is scoped (`enum class`), preventing implicit conversion to integers.
- The set of enumerators is fixed at compile time.

#### Key Members

- `clore::generate::PromptKind::NamespaceSummary`
- `clore::generate::PromptKind::ModuleSummary`
- `clore::generate::PromptKind::ModuleArchitecture`
- `clore::generate::PromptKind::IndexOverview`
- `clore::generate::PromptKind::FunctionAnalysis`
- `clore::generate::PromptKind::TypeAnalysis`
- `clore::generate::PromptKind::VariableAnalysis`
- `clore::generate::PromptKind::FunctionDeclarationSummary`
- `clore::generate::PromptKind::FunctionImplementationSummary`
- `clore::generate::PromptKind::TypeDeclarationSummary`
- `clore::generate::PromptKind::TypeImplementationSummary`

#### Usage Patterns

- Used to select the appropriate prompt template or generation function.
- Passed as an argument to query a prompt registry or dispatcher.
- May be stored to indicate the kind of analysis requested.

#### Member Variables

##### `clore::generate::PromptKind::FunctionAnalysis`

Declaration: `src/generate/model.cppm:39`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
FunctionAnalysis
```

##### `clore::generate::PromptKind::FunctionDeclarationSummary`

Declaration: `src/generate/model.cppm:42`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
FunctionDeclarationSummary
```

##### `clore::generate::PromptKind::FunctionImplementationSummary`

Declaration: `src/generate/model.cppm:43`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
FunctionImplementationSummary
```

##### `clore::generate::PromptKind::IndexOverview`

Declaration: `src/generate/model.cppm:38`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
IndexOverview
```

##### `clore::generate::PromptKind::ModuleArchitecture`

Declaration: `src/generate/model.cppm:37`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
ModuleArchitecture
```

##### `clore::generate::PromptKind::ModuleSummary`

Declaration: `src/generate/model.cppm:36`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
ModuleSummary
```

##### `clore::generate::PromptKind::NamespaceSummary`

Declaration: `src/generate/model.cppm:35`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
NamespaceSummary
```

##### `clore::generate::PromptKind::TypeAnalysis`

Declaration: `src/generate/model.cppm:40`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
TypeAnalysis
```

##### `clore::generate::PromptKind::TypeDeclarationSummary`

Declaration: `src/generate/model.cppm:44`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
TypeDeclarationSummary
```

##### `clore::generate::PromptKind::TypeImplementationSummary`

Declaration: `src/generate/model.cppm:45`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
TypeImplementationSummary
```

##### `clore::generate::PromptKind::VariableAnalysis`

Declaration: `src/generate/model.cppm:41`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
VariableAnalysis
```

### `clore::generate::PromptRequest`

Declaration: `src/generate/model.cppm:50`

Definition: `src/generate/model.cppm:50`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::PromptRequest` is a plain data aggregation that pairs a `PromptKind` value with a target identifier stored as `std::string`. Both members are initialized with defaults: `kind` defaults to `PromptKind::NamespaceSummary` and `target_key` defaults to an empty string. There are no explicit invariants enforced by the struct itself; the relationship between `kind` and `target_key` is defined by the callers that populate and consume the request.

#### Invariants

- `kind` defaults to `PromptKind::NamespaceSummary`
- `target_key` defaults to an empty string
- No user-defined constructors or destructors; trivial type

#### Key Members

- `kind`
- `target_key`

#### Usage Patterns

- Instantiated and passed to prompt generation functions
- Used to specify prompt type and associated identifier

### `clore::generate::RenderError`

Declaration: `src/generate/model.cppm:89`

Definition: `src/generate/model.cppm:89`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct stores an error message as a `std::string` member named `message`. No other state or invariants are maintained; the struct acts as a lightweight error carrier, relying on the implicit default and copy/move operations provided by the compiler. Its simplicity ensures minimal overhead when passed by value or stored inside error-handling machinery such as `std::expected`.

#### Invariants

- The `message` field holds a descriptive error string

#### Key Members

- message

#### Usage Patterns

- Returned or thrown from generation functions to indicate errors
- Used as a lightweight error carrier

### `clore::generate::SymbolAnalysisStore`

Declaration: `src/generate/model.cppm:141`

Definition: `src/generate/model.cppm:141`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::SymbolAnalysisStore` is an implementation-level aggregate that bundles three analysis caches: `types` of type `TypeAnalysisCache`, `variables` of type `VariableAnalysisCache`, and `functions` of type `FunctionAnalysisCache`. Each cache stores the results of symbol analysis for its respective category, and the struct serves as a single container to hold all per‑symbol analysis data after a pass completes. No invariants beyond the validity of the individual caches are maintained; the struct is a plain data holder with all fields publicly accessible.

#### Invariants

- The three cache fields are distinct and likely initialized together.

#### Key Members

- `functions`
- `types`
- `variables`

#### Usage Patterns

- The struct is used to hold the analysis results for symbols, likely populated during analysis phase and queried during generation.

### `clore::generate::SymbolTargetKeyView`

Declaration: `src/generate/model.cppm:152`

Definition: `src/generate/model.cppm:152`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::SymbolTargetKeyView` is an aggregate view type that holds two non-owning `std::string_view` members: `qualified_name` and `signature`. It serves as a lightweight, copyable key for symbol-target lookups without duplicating string storage. As a trivial aggregate, it supports default construction, aggregate initialization, and implicit copy/move operations. The key invariant is that the character buffers referenced by both string views must outlive any instance of the view; callers are responsible for ensuring the underlying strings remain valid. No custom constructors, assignment `operator`s, or comparison `operator`s are declared; all behavior is compiler-generated, making the type suitable for use in associative containers with a user-provided comparator or hasher.

#### Invariants

- The referenced strings must outlive the view
- `qualified_name` and `signature` refer to valid, stable data

#### Key Members

- `qualified_name`
- `signature`

#### Usage Patterns

- Used as a key for symbol target lookup or storage
- Passed as a parameter to functions requiring symbol identity without copying

### `clore::generate::TypeAnalysis`

Declaration: `src/generate/model.cppm:107`

Definition: `src/generate/model.cppm:107`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::TypeAnalysis` is a plain aggregate that stores the complete output of a type analysis. It holds two markdown strings, `overview_markdown` and `details_markdown`, that contain the prose description of the type, and three `std::vector<std::string>` fields — `invariants`, `key_members`, and `usage_patterns` — that capture structured, itemized findings. The design separates free‑form explanatory content from enumerable facts; the vectors are expected to list individual insights, while the markdown fields provide context and narrative.

There are no explicit invariants enforced by the struct itself beyond the typical requirement that the contained data be logically consistent. In practice, the fields are populated by the analysis step so that `overview_markdown` and `details_markdown` together form a coherent textual summary, and each element in `invariants`, `key_members`, and `usage_patterns` represents a distinct, deduplicated observation about the analyzed type. This layout allows downstream consumers to either render the full analysis as a markdown document or extract specific categories programmatically.

#### Invariants

- Fields are populated consistently for a given type analysis
- No field is null or undefined after initialization

#### Key Members

- `overview_markdown`
- `details_markdown`
- `invariants`
- `key_members`
- `usage_patterns`

#### Usage Patterns

- Created once per type and reused across documentation pages
- Populated by analysis logic and consumed by documentation generators

### `clore::generate::VariableAnalysis`

Declaration: `src/generate/model.cppm:115`

Definition: `src/generate/model.cppm:115`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct stores the results of analyzing a single variable declaration or reference site. The boolean `is_mutated` defaults to `false` and is set to `true` only when the analysis encounters an assignment, increment, or other mutating operation on the variable; the `mutation_sources` vector then holds strings describing where and how that mutation occurs (e.g., a location or operation name). `usage_patterns` collects categories such as “read”, “written”, “passed to function”, while `overview_markdown` and `details_markdown` hold pre‑formatted prose that is later embedded into generated documentation. An invariant is that `mutation_sources` should be empty when `is_mutated` is `false`, and non‑empty otherwise; the struct does not enforce this, so callers must maintain consistency.

#### Invariants

- `is_mutated` is initialized to false
- `mutation_sources` and `usage_patterns` are initially empty

#### Key Members

- overview and detail documentation strings
- mutation flag and source list
- usage pattern list

#### Usage Patterns

- Used to store results of variable analysis, likely populated by analysis functions
- Consumed by documentation generation to produce variable pages

## Functions

### `clore::generate::analysis_details_markdown`

Declaration: `src/generate/model.cppm:173`

Definition: `src/generate/model.cppm:389`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::analysis_details_markdown` is a thin delegating wrapper that retrieves the *details* markdown fragment for a given symbol. It accepts a `SymbolAnalysisStore` and a `SymbolInfo`, and immediately forwards the call to `analysis_markdown`, passing a `FieldAccessor` lambda that returns the address of `analysis.details_markdown` from whichever analysis type is active — `TypeAnalysis`, `FunctionAnalysis`, or `VariableAnalysis`. This decouples the markdown selection logic from the specific field name; the caller only needs to provide the store and the symbol identity.

Internally, `analysis_markdown` handles the cache lookup (via the store’s per‑symbol map), resolves the correct symbol analysis object by calling helpers such as `find_type_analysis`, `find_function_analysis`, or `find_variable_analysis`, and then invokes the accessor to obtain the requested markdown string. The result pointer is either a cached string from the store or `nullptr` if the analysis does not exist. This function depends on `analysis_markdown` and the store’s analysis tables, but does not itself implement any caching or resolution logic.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses` of type `const SymbolAnalysisStore &`
- `symbol` of type `const extract::SymbolInfo &`
- `analysis.details_markdown` field

#### Usage Patterns

- Used during documentation generation to retrieve the detailed analysis section for a symbol.

### `clore::generate::analysis_markdown`

Declaration: `src/generate/model.cppm:358`

Definition: `src/generate/model.cppm:358`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::analysis_markdown` routes a symbol-level analysis request to the correct typed analysis structure and extracts a specific markdown field via a generic `field_accessor` callable. It first computes a symbolic target key by calling `make_symbol_target_key` on the provided symbol. The control flow then branches on the symbol’s kind using the predicates `is_function_kind`, `is_type_kind`, and `is_variable_kind`. For each kind, the corresponding find helper—`find_function_analysis`, `find_type_analysis`, or `find_variable_analysis`—looks up the analysis in the `SymbolAnalysisStore` using the target key. If the analysis is found, the `field_accessor` is invoked with that analysis object to retrieve the desired string field (e.g., `overview_markdown`, `details_markdown`, or `usage_patterns`). The result is returned as a pointer to the field, or `nullptr` if no matching analysis exists or the symbol kind is unrecognized. The function depends on the internal find functions, the kind predicate helpers, and the `SymbolAnalysisStore` data structure for dispatching to the concrete `FunctionAnalysis`, `TypeAnalysis`, or `VariableAnalysis` types.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses` parameter (const `SymbolAnalysisStore&`)
- `symbol` parameter (const `extract::SymbolInfo&`)
- `field_accessor` parameter (functor applied to analysis objects)
- results of `find_function_analysis`, `find_type_analysis`, `find_variable_analysis`

#### Usage Patterns

- Retrieving overview markdown for a symbol analysis
- Retrieving details markdown for a symbol analysis
- Extracting specific analysis fields via an accessor

### `clore::generate::analysis_overview_markdown`

Declaration: `src/generate/model.cppm:170`

Definition: `src/generate/model.cppm:382`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `analysis_overview_markdown` serves as a thin delegation point that extracts the overview markdown from a symbol’s analysis result. It invokes `analysis_markdown` with a field accessor lambda that returns a pointer to the `overview_markdown` member of the appropriate analysis struct – one of `TypeAnalysis`, `FunctionAnalysis`, or `VariableAnalysis`. The dispatching logic inside `analysis_markdown` uses the symbol’s kind to look up the corresponding analysis via `find_type_analysis`, `find_function_analysis`, or `find_variable_analysis`. The resolved analysis object is then passed to the accessor, and the resulting `const std::string*` (with the overview markdown content) is returned directly.

All internal control flow is encapsulated within `analysis_markdown`, which also handles caching and thread safety via a mutex‑protected cache. Dependencies include the `SymbolAnalysisStore` (containing `types`, `functions`, `variables`), the symbol’s identity derived from the provided integer identifier, and the field accessor mechanism defined by `FieldAccessor`. No validation or error handling is performed at this layer; failures are propagated from the underlying lookup and accessor functions.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses` (const `SymbolAnalysisStore&`)
- `symbol` (const `extract::SymbolInfo&`)
- the `overview_markdown` field of the resolved `SymbolAnalysis`

#### Usage Patterns

- Used to obtain overview markdown for a symbol analysis
- Likely called by documentation rendering functions such as `render_page_markdown` or `build_symbol_analysis_prompt`

### `clore::generate::build_link_resolver`

Declaration: `src/generate/model.cppm:217`

Definition: `src/generate/model.cppm:487`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::build_link_resolver` constructs a `LinkResolver` by iterating over each `PagePlan` in the input `plan_set`. For every plan, it records the page title into `resolver.page_id_to_title` and inserts the plan’s owner keys into the generic `resolver.name_to_path` map using `emplace`, so that the first registration prevails in case of duplicate keys. Additionally, if the plan’s `page_type` is `PageType::Namespace` or `PageType::Module`, the same key is also stored in the disambiguated maps `resolver.namespace_to_path` or `resolver.module_to_path` respectively. After handling the owner keys, the function extracts any suffix after the colon in `plan.page_id` and repeats the same insertion logic for that suffix, enabling lookup by a simplified name. The algorithm relies on the `PagePlanSet`’s sorted `generation_order` to determine registration priority and uses no external dependencies beyond the `LinkResolver`’s internal maps.

#### Side Effects

- Allocates and populates internal maps of a new `LinkResolver` instance
- Transfers ownership of the created `LinkResolver` to the caller

#### Reads From

- `plan_set.plans`
- `plan.page_id`
- `plan.title`
- `plan.owner_keys`
- `plan.relative_path`
- `plan.page_type`

#### Writes To

- `resolver.page_id_to_title`
- `resolver.name_to_path`
- `resolver.namespace_to_path`
- `resolver.module_to_path`

#### Usage Patterns

- Called to build a `LinkResolver` for later use in resolving page titles, namespaces, and modules
- Used by other functions that need to map symbol keys or page `IDs` to relative paths

### `clore::generate::compute_page_path`

Declaration: `src/generate/model.cppm:230`

Definition: `src/generate/model.cppm:592`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function switches on `identity.page_type` to construct a file path from the given `PageIdentity`. For `PageType::Index` it uses the constant `kIndexPath` directly. For `PageType::Module` it splits `identity.qualified_name` by `.` via `split_qualified`, checks for a partition (delimited by `:`), normalizes each component with `normalize_name`, prepends `kModulePrefix`, and either appends `index.md` or a partition-based filename. For `PageType::Namespace` it splits by `::`, normalizes parts, prepends `kNamespacePrefix`, and appends `index.md`. For `PageType::File` it strips the extension from `identity.source_relative_path`, prepends `kFilePrefix`, and appends `.md`. The assembled path string is then cleaned by `sanitize_path_chars` and validated by `validate_path_component`; if validation fails the function returns `std::unexpected` with a `PathError`. Internal helpers `normalize_name`, `split_qualified`, `sanitize_path_chars`, and `validate_path_component` are called, and the path aggregation uses a local `join_path` utility.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- const `PageIdentity&` parameter `identity` fields `page_type`, `qualified_name`, `source_relative_path`
- constants `kIndexPath`, `kModulePrefix`, `kNamespacePrefix`, `kFilePrefix`

#### Usage Patterns

- called during page generation to determine output file path for each page

### `clore::generate::find_function_analysis`

Declaration: `src/generate/model.cppm:161`

Definition: `src/generate/model.cppm:339`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function performs a direct lookup within the `SymbolAnalysisStore::functions` associative container, keyed by `symbol_target_key`. It calls `find` on the map and returns a pointer to the matched `FunctionAnalysis` if found, or `nullptr` otherwise. The only nontrivial dependency is the map structure itself; no additional algorithm or branching is involved beyond the standard container lookup.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses.functions`
- `symbol_target_key`

#### Usage Patterns

- Lookup function analysis by symbol key for further processing
- Check existence of analysis before accessing detailed fields

### `clore::generate::find_type_analysis`

Declaration: `src/generate/model.cppm:164`

Definition: `src/generate/model.cppm:345`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::find_type_analysis` performs a direct lookup in the `types` member of the provided `SymbolAnalysisStore`. It uses `analyses.types.find(symbol_target_key)` to locate a `TypeAnalysis` by its symbol target key string, returning a pointer to the found entry or `nullptr` if no match exists. This simple map lookup is the entire internal control flow; the function relies on the `SymbolAnalysisStore` aggregate which holds `std::unordered_map`-like containers for symbol analyses. No additional dependencies on other subsystems are used within this function itself.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses.types` (the map of type analysis records)
- `symbol_target_key` (the lookup key)

#### Usage Patterns

- Retrieve type analysis for a symbol during documentation generation
- Used by higher-level analysis formatting functions like `analysis_details_markdown` or `analysis_overview_markdown`

### `clore::generate::find_variable_analysis`

Declaration: `src/generate/model.cppm:167`

Definition: `src/generate/model.cppm:351`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::find_variable_analysis` implements a direct lookup inside the `SymbolAnalysisStore` container. It accepts a `std::string_view` representing the `symbol_target_key` and searches the `analyses.variables` map. If the key is present, a pointer to the corresponding `VariableAnalysis` object is returned; otherwise, `nullptr` is returned. This implementation has no branching beyond the map lookup and no additional control flow or external dependencies beyond the storage layout of `SymbolAnalysisStore`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- analyses`.variables` (the map from symbol target keys to `VariableAnalysis` objects)
- `symbol_target_key` (the lookup key)

#### Usage Patterns

- retrieve a `VariableAnalysis`* for a given symbol target key
- check if variable analysis exists

### `clore::generate::is_function_kind`

Declaration: `src/generate/model.cppm:178`

Definition: `src/generate/model.cppm:409`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::is_function_kind` implements a simple dispatch over `extract::SymbolKind`. It uses a `switch` statement to match against `extract::SymbolKind::Function` and `extract::SymbolKind::Method`, returning `true` for either, and `false` for the default case. This logic directly encodes the classification rule that both functions and methods are considered function‑like kinds within the generate subsystem. The only dependency is the enumeration `extract::SymbolKind`, which is expected to be defined in the `extract` module.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `kind` parameter of type `extract::SymbolKind`

#### Usage Patterns

- filtering symbol kinds
- guard conditions when deciding how to process a symbol

### `clore::generate::is_page_level_symbol`

Declaration: `src/generate/model.cppm:182`

Definition: `src/generate/model.cppm:421`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::is_page_level_symbol` determines whether a given `extract::SymbolInfo` represents a symbol that should be treated as a top-level page for code generation. It first rejects symbols whose `lexical_parent_kind` is neither `extract::SymbolKind::Unknown` nor `extract::SymbolKind::Namespace`, ensuring only symbols directly declared in a namespace or at global scope are considered. It then excludes any symbol whose `enclosing_namespace` or `lexical_parent_name` contains the string `"(anonymous namespace)"`, because anonymous‑namespace symbols are implementation details, not public API. When the symbol has a `parent`, the function uses `extract::lookup_symbol` to retrieve the parent; if the parent is a type or function (checked via `is_type_kind` or `is_function_kind`), the symbol is rejected because it is nested inside a larger construct. Only symbols that pass all these filters are considered page‑level and return `true`. The function depends on the extraction model’s symbol hierarchy and several kind‑classification helpers from the `clore::generate` namespace.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model` (const `ProjectModel` &)
- `sym` (const `SymbolInfo` &)

#### Usage Patterns

- Called during page generation to filter symbols that warrant a dedicated documentation page

### `clore::generate::is_page_summary_prompt`

Declaration: `src/generate/model.cppm:149`

Definition: `src/generate/model.cppm:313`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function performs a simple equality check against two enum members of `PromptKind`: `NamespaceSummary` and `ModuleSummary`. It has no branching beyond the return of the logical OR result and no dependencies on external state or complex data structures. The entire implementation consists of a single expression evaluating whether the passed `kind` matches either of these two summary‑oriented prompt kinds.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `PromptKind` parameter `kind`

#### Usage Patterns

- Called to classify whether a prompt kind corresponds to a page summary
- Used in conditional logic for prompt generation or caching

### `clore::generate::is_symbol_analysis_prompt`

Declaration: `src/generate/model.cppm:150`

Definition: `src/generate/model.cppm:317`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::is_symbol_analysis_prompt` implements a straightforward classification of a `PromptKind` value. It returns `true` if the input `kind` matches any of three analysis‑related enumerators: `PromptKind::FunctionAnalysis`, `PromptKind::TypeAnalysis`, or `PromptKind::VariableAnalysis`; otherwise it returns `false`. The logic is a simple equality check combined with logical OR disjunction, with no loops, recursion, or branching beyond the direct comparison. The only dependency is the `PromptKind` enum itself and the specific enumerators used in the check.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `kind` of type `PromptKind`

#### Usage Patterns

- Used to classify prompt kinds as symbol analysis prompts
- Likely invoked when deciding whether a prompt belongs to symbol analysis category

### `clore::generate::is_type_kind`

Declaration: `src/generate/model.cppm:176`

Definition: `src/generate/model.cppm:396`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::is_type_kind` implements a switch-based dispatch on the `extract::SymbolKind` enumeration. It returns `true` for any of the seven type‑related symbol kinds: `extract::SymbolKind::Class`, `extract::SymbolKind::Struct`, `extract::SymbolKind::Enum`, `extract::SymbolKind::Union`, `extract::SymbolKind::Concept`, `extract::SymbolKind::Template`, and `extract::SymbolKind::TypeAlias`. All other kinds cause the default branch to return `false`. The function has no external dependencies beyond the definition of the `extract::SymbolKind` enum, and its control flow is purely a single level of pattern matching with no loops or recursion.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `kind` of type `extract::SymbolKind`

#### Usage Patterns

- called to classify symbol kinds in generation logic
- used as a condition in control flow for type-specific processing

### `clore::generate::is_variable_kind`

Declaration: `src/generate/model.cppm:180`

Definition: `src/generate/model.cppm:417`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::is_variable_kind` is a predicate that determines whether a given `extract::SymbolKind` corresponds to a variable-like symbol. Its implementation performs a direct equality check against two enumerators: it returns `true` if the input `kind` equals either `extract::SymbolKind::Variable` or `extract::SymbolKind::EnumMember`, and `false` otherwise. This classification is used elsewhere in the generation pipeline to separate variable and enum-member symbols from other symbol kinds (e.g., functions, types) when constructing page plans or resolving analysis store lookups. The function has no dependencies beyond the `extract::SymbolKind` enum and involves no branching beyond the two comparisons, making it a lightweight, inline‑eligible test.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- kind parameter

#### Usage Patterns

- Used in symbol classification
- Called during analysis or page generation

### `clore::generate::make_source_relative`

Declaration: `src/generate/model.cppm:185`

Definition: `src/generate/model.cppm:448`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function constructs a composite key from `project_root` and `path` separated by a newline, then consults a thread‑safe `SourceRelativeCache` under a shared lock. If a hit is found, it returns the cached relative path immediately, avoiding any filesystem work. Otherwise, it uses `std::filesystem::path::lexically_normal` to normalize both inputs and `lexically_relative` to compute the relative path from `project_root` to `path`. If the resulting relative string is empty or begins with `".."` (indicating the path is outside the project root), it falls back to the original `path`; otherwise it converts the result to a generic string. The computed result is then inserted into the cache under a unique lock before being returned. The entire algorithm relies on the static `source_relative_cache()` function to provide the shared mutex and map, and on `std::filesystem` for path normalization and relativization.

#### Side Effects

- Updates the global `SourceRelativeCache` with a new entry for the computed relative path.

#### Reads From

- parameter `path`
- parameter `project_root`
- `source_relative_cache().relative_by_key` under shared lock

#### Writes To

- `source_relative_cache().relative_by_key` under exclusive lock

#### Usage Patterns

- Used to normalize file paths for consistent relative references in generated documentation.

### `clore::generate::make_symbol_target_key`

Declaration: `src/generate/model.cppm:157`

Definition: `src/generate/model.cppm:322`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function constructs a unique string key for a given symbol by first embedding the length of the qualified name as a prefix, then appending the qualified name itself, followed by the symbol’s signature. This length‑prefixed design enables the corresponding parser (`parse_symbol_target_key`) to efficiently extract the qualified name without scanning for delimiters. The implementation uses `std::format` to produce the initial `"size:name"` segment and then simply concatenates the signature via `operator+=`. No branching or look‑up is performed; the key is derived purely from the `extract::SymbolInfo` fields `qualified_name` and `signature`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `symbol.qualified_name`
- `symbol.qualified_name.size()`
- `symbol.signature`

#### Usage Patterns

- Called to generate a lookup key for a symbol analysis.
- Used in `parse_symbol_target_key` to reverse the operation.
- Employed as part of key construction for caching symbol data.

### `clore::generate::page_type_name`

Declaration: `src/generate/model.cppm:32`

Definition: `src/generate/model.cppm:279`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::page_type_name` implements a trivial mapping from the `PageType` enum to human-readable string names. Internally, it uses a `switch` statement over the four enumerators (`Index`, `Module`, `Namespace`, `File`), each returning a corresponding string literal (e.g., `"index"`, `"module"`). A default case returns `"unknown"` as a safety net for any unrecognized value. The sole dependency is the `PageType` enum defined in the same translation unit; the function itself involves no state, memory allocation, or external calls.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `type` parameter of type `PageType`

#### Usage Patterns

- Get human-readable name of a page type for documentation or logging
- Label page generation targets
- Map page types to output file names or identifiers

### `clore::generate::parse_symbol_target_key`

Declaration: `src/generate/model.cppm:159`

Definition: `src/generate/model.cppm:328`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::parse_symbol_target_key` first delegates to the internal helper `clore::generate::(anonymous namespace)::parse_length_prefixed_symbol_target_key`. If that optional result has a value, the parsed `SymbolTargetKeyView` is returned directly. Otherwise, it constructs a `SymbolTargetKeyView` where the entire input `target_key` is set as the `qualified_name` field and the `signature` field is left as an empty view. This fallback path accommodates symbol keys that do not follow the length‑prefixed encoding, allowing arbitrary qualified name strings to be handled uniformly.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `target_key` parameter

#### Usage Patterns

- Used to parse symbol target keys for lookups or formatting.

### `clore::generate::prompt_kind_name`

Declaration: `src/generate/model.cppm:48`

Definition: `src/generate/model.cppm:289`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Implementation: [Implementation](functions/prompt-kind-name.md)

The function `clore::generate::prompt_kind_name` maps each enumerator of `clore::generate::PromptKind` to a corresponding `std::string_view` string literal. It uses a simple `switch` statement over the `kind` parameter, returning a descriptive, underscore-separated name such as `"namespace_summary"`, `"module_architecture"`, `"function_analysis"`, or `"type_implementation_summary"`. If the `kind` does not match any known enumerator (which should not occur in normal usage), the fallback return is `"unknown_prompt"`. No external dependencies or sub-calls are involved; the function is a pure, constant‑time lookup from enum to string.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `kind` parameter

#### Usage Patterns

- Called by `clore::generate::prompt_request_key` to derive a string key for a given prompt request.

### `clore::generate::prompt_request_key`

Declaration: `src/generate/model.cppm:147`

Definition: `src/generate/model.cppm:306`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::prompt_request_key` constructs a string key that uniquely identifies a prompt request by combining the prompt kind name with an optional target key. The control flow first checks whether `request.target_key` is empty; if so, the key is simply the result of `prompt_kind_name(request.kind)`. Otherwise, the function concatenates `prompt_kind_name(request.kind)` with a `:` delimiter followed by `request.target_key`. This relies on the helper function `clore::generate::prompt_kind_name` to produce a human‑readable representation of the `PromptKind` enumeration, and the resulting key is used as a cache or lookup identifier in subsequent generation steps.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- request`.kind`
- request`.target_key`

#### Usage Patterns

- caching prompt requests
- identifying prompt requests

### `clore::generate::validate_no_path_conflicts`

Declaration: `src/generate/model.cppm:232`

Definition: `src/generate/model.cppm:660`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::validate_no_path_conflicts` performs a single-pass conflict detection over a collection of `(path, id)` pairs. It initializes a local `std::unordered_map<std::string, std::string>` named `seen` and iterates through the supplied `path_to_id` vector. For each pair, it calls `seen.emplace(path, id)`. If the insertion fails (i.e., `inserted` is `false`), the path has already been mapped to a different identifier, so the function immediately returns `std::unexpected(PathError{.message = ...})`. The error message is constructed via `std::format` and includes the previously recorded identifier, the new identifier, and the duplicate path. If all insertions succeed, the function returns a default `std::expected<void, PathError>` representing success. The algorithm relies solely on the standard library’s unordered associative container semantics and the locally defined `PathError` struct; no other external state or complex data flow is involved.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `path_to_id` of type `const std::vector<std::pair<std::string, std::string>>&`

#### Usage Patterns

- called before inserting path mappings to ensure uniqueness
- used in page path computation to avoid collisions

## Internal Structure

The `generate:model` module defines the central data structures and value types that underpin the documentation generation pipeline. It provides the representation of page identity (`PageIdentity`), page plans (`PagePlan`, `PagePlanSet`), generated pages (`GeneratedPage`), symbol analyses (`FunctionAnalysis`, `TypeAnalysis`, `VariableAnalysis`), and supporting infrastructure such as the `LinkResolver`, error types (`PathError`, `GenerateError`, `RenderError`), and enumerations (`PageType`, `PromptKind`). These types are used to capture the extraction results from the `extract` module and to serve as the foundation for planning, rendering, and output.

Internally, the module relies on helper utilities in an anonymous namespace—such as `normalize_name`, `split_qualified`, `source_relative_cache`, and `sanitize_path_chars`—to preprocess and transform identifiers and paths. It imports the `extract` module (for symbol and AST data) and the `support` module (for string handling, caching, and I/O). The public API consists of functions like `compute_page_path`, `parse_symbol_target_key`, `analysis_markdown`, `build_link_resolver`, and predicates (`is_type_kind`, `is_page_summary_prompt`), which collectively bridge the gap between extracted facts and the generation of final documentation pages.

## Related Pages

- [Module extract](../extract/index.md)
- [Module support](../support/index.md)

