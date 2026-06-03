---
title: 'Module generate:evidence'
description: 'The generate:evidence module is responsible for collecting, formatting, and packaging evidence that feeds into documentation generation prompts. It owns the logic for transforming raw symbol and analysis data—gathered from the extract and generate:model modules—into structured EvidencePack objects that describe namespaces, modules, functions, types, variables, index overviews, and architectural summaries. The module exposes high-level entry points such as build_evidence_for_* functions (for each page kind), format_evidence_text (and a bounded variant), and build_prompt, which assembles the final LLM prompt from an evidence pack and a prompt template. Internally, it manages caching (via NamespaceFactsCacheEntry), detail-level rendering helpers, and the SymbolFact structure that captures per-symbol attributes like qualified names, signatures, documentation, and template parameters.'
layout: doc
template: doc
---

# Module `generate:evidence`

## Summary

The `generate:evidence` module is responsible for collecting, formatting, and packaging evidence that feeds into documentation generation prompts. It owns the logic for transforming raw symbol and analysis data—gathered from the `extract` and `generate:model` modules—into structured `EvidencePack` objects that describe namespaces, modules, functions, types, variables, index overviews, and architectural summaries. The module exposes high-level entry points such as `build_evidence_for_*` functions (for each page kind), `format_evidence_text` (and a bounded variant), and `build_prompt`, which assembles the final LLM prompt from an evidence pack and a prompt template. Internally, it manages caching (via `NamespaceFactsCacheEntry`), detail-level rendering helpers, and the `SymbolFact` structure that captures per-symbol attributes like qualified names, signatures, documentation, and template parameters.

## Imports

- [`extract`](../extract/index.md)
- [`generate:model`](model.md)

## Imported By

- [`generate:analysis`](analysis.md)
- [`generate:evidence_builder`](index.md)
- [`generate:scheduler`](scheduler.md)

## Types

### `clore::generate::EvidencePack`

Declaration: `src/generate/evidence.cppm:34`

Definition: `src/generate/evidence.cppm:34`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::EvidencePack` serves as a flat container that aggregates all evidence required by the documentation generator. Its fields are plain data members of standard types, with no custom constructors, destructors, or accessors, making it a simple aggregate. The invariants are implicit: each field is expected to be populated consistently before the pack is consumed; for example, `page_id` and `prompt_kind` should form a valid key pair, and the various vector fields (`target_facts`, `local_context`, `dependency_context`, `reverse_usage_context`, `source_snippets`, `related_page_summaries`) are expected to contain complete, non‑overlapping information for a single subject. No member functions or special member definitions are present, so the implementation relies entirely on the default compiler‑generated behavior for copy, move, and assignment.

#### Invariants

- All vector fields may be empty
- Scalar string fields may be empty

#### Key Members

- `target_facts`
- `local_context`
- `subject_name`
- `subject_kind`

#### Usage Patterns

- Assembled by evidence collectors and consumed by generation logic
- Each field is independently populated

### `clore::generate::PromptError`

Declaration: `src/generate/evidence.cppm:102`

Definition: `src/generate/evidence.cppm:102`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::PromptError` is a lightweight error type that holds a single `std::string` member `message` to store a human‑readable error description. No other data members or invariants exist beyond those inherent to the string itself; the struct is trivially constructible, copyable, and movable. Its internal structure directly mirrors its role as a simple carrier for error information, with no additional validation or state management in the implementation.

#### Invariants

- The `message` string is expected to be non-empty when representing an actual error.

#### Key Members

- `message`: a `std::string` that holds the error description.

#### Usage Patterns

- Used as the exception type or error result in prompt generation contexts, such as in `clore::generate::PromptGenerator` or related functions.

### `clore::generate::SymbolFact`

Declaration: `src/generate/evidence.cppm:21`

Definition: `src/generate/evidence.cppm:21`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::SymbolFact` serves as a flat, aggregate data holder for all evidence extracted about a single symbol from source code. All member fields are directly set during the extraction phase; no invariants are enforced by the struct itself beyond the default values provided in the member initializers. The boolean `is_template` defaults to `false`, and `declaration_line` defaults to `0`, meaning a zero line number is the expected sentinel for an unset location when `declaration_file` is empty. The fields `qualified_name`, `signature`, `kind_label`, `access`, `template_params`, and `doc_comment` are all plain `std::string` values, filled by the extractor. There is no hidden state or computed member—every field is a raw piece of data that the generation pipeline later consumes to produce documentation.

#### Invariants

- Fields are expected to be populated with consistent data from extraction
- `is_template` defaults to `false` if not set
- `declaration_line` defaults to `0` if not provided

#### Key Members

- `id`
- `qualified_name`
- `signature`
- `kind_label`
- `access`
- `is_template`
- `template_params`
- `declaration_file`
- `declaration_line`
- `doc_comment`

#### Usage Patterns

- Instantiated by extraction phases to hold symbol data
- Consumed by generation phases to produce documentation output
- Stored or passed between components as a value type

## Functions

### `clore::generate::__detail::collect_analysis_summaries`

Declaration: `src/generate/evidence.cppm:133`

Definition: `src/generate/evidence.cppm:253`

The function `clore::generate::__detail::collect_analysis_summaries` acts as a thin forwarding wrapper that delegates to `clore::generate::__detail::collect_analysis_summaries_impl`. It accepts a `SymbolAnalysisStore` (`analyses`), a `ProjectModel` (`model`), and a `std::vector<extract::SymbolID>` (`ids`). Internally, it passes these three arguments to the implementation function along with an identity callable that returns each `extract::SymbolID` unchanged. The returned value is a `std::vector<std::string>` containing the collected analysis summaries. This design separates the generic summarization logic from the specific ID mapping — in this case, the `IDs` are used directly without transformation, relying on the implementation to iterate over the provided `IDs`, query the analysis store for each symbol, and produce the corresponding summary text.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses`
- `model`
- `ids`

#### Usage Patterns

- Called with an analysis store, project model, and a list of symbol `IDs` to obtain a vector of summary strings.

### `clore::generate::__detail::collect_analysis_summaries_impl`

Declaration: `src/generate/evidence.cppm:128`

Definition: `src/generate/evidence.cppm:225`

The function `clore::generate::__detail::collect_analysis_summaries_impl` iterates over the given `Range` of `items`, deduplicating by symbol identity. For each item, it extracts a symbol ID using the forwarded `SymbolIDOf` callable and inserts it into a local `seen` set; if insertion fails the symbol has already been processed and the item is skipped. It then calls `extract::lookup_symbol` on the provided `model` to retrieve the corresponding symbol; if the lookup fails the item is also skipped. Finally it attempts to obtain a precomputed analysis summary via `analysis_overview_markdown` using the `analyses` store and the symbol; if the returned pointer is null or the string is empty the item is skipped. Otherwise the summary string is appended to the result vector. The function reserves capacity upfront and returns a collection of all successfully collected summary strings. Its core dependencies are the `extract::lookup_symbol` and `analysis_overview_markdown` functions, along with the external `analyses` and `model` objects that supply the analysis cache and symbol database.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses`
- `model`
- `items`
- `symbol_id_of` functor
- `extract::lookup_symbol(model, symbol_id)` result
- `analysis_overview_markdown(analyses, *symbol)` result

#### Usage Patterns

- Used internally to aggregate analysis summaries from a set of symbol `IDs`
- Supports `collect_analysis_summaries` and similar batch functions

### `clore::generate::__detail::collect_facts`

Declaration: `src/generate/evidence.cppm:115`

Definition: `src/generate/evidence.cppm:175`

The function `clore::generate::__detail::collect_facts` deduplicates a list of symbol identifiers and converts each unique identifier into a `SymbolFact` object. It iterates over the provided `ids` vector, using a `std::unordered_set<extract::SymbolID>` named `seen` to skip previously encountered identifiers. For each unseen identifier, it calls `extract::lookup_symbol(model, id)` to obtain a pointer to the corresponding symbol within the `extract::ProjectModel`. If the pointer is non‑null, the symbol is transformed via `to_symbol_fact(*sym, project_root)`, which extracts relevant attributes (such as `qualified_name`, `declaration_file`, `signature`, etc.) using the given `project_root` path. The resulting `SymbolFact` objects are appended to a local vector, which is returned to the caller. This function depends on the `extract` module for symbol lookup and on the internal helper `to_symbol_fact` for attribute extraction.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- const `extract::ProjectModel`& model
- const `std::vector<extract::SymbolID>`& ids
- const `std::string`& `project_root`
- `extract::lookup_symbol`

#### Usage Patterns

- Converts a list of symbol `IDs` into a deduplicated vector of `SymbolFact` objects
- Used in fact generation pipelines, likely called by other collect functions

### `clore::generate::__detail::collect_merged_facts`

Declaration: `src/generate/evidence.cppm:120`

Definition: `src/generate/evidence.cppm:191`

The function `clore::generate::__detail::collect_merged_facts` accepts a `extract::ProjectModel`, a `project_root` string, and a variadic pack of `Groups` (each expected to be a container of `extract::SymbolID`). It collects a deduplicated set of `SymbolFact` objects from the symbol `IDs` across all supplied groups. Internally, a `std::unordered_set<extract::SymbolID>` named `seen` records already‑processed symbols to avoid duplicates. A lambda `append_group` iterates over a group’s `IDs`; for each `id`, it attempts insertion into `seen`. If the insertion succeeds (i.e., the ID is new), it calls `extract::lookup_symbol(model, id)` to retrieve the symbol, and if the pointer is non‑null, converts it to a `SymbolFact` via `to_symbol_fact(*sym, project_root)` and appends the fact to the result vector. The groups are processed left‑to‑right using a fold expression `(append_group(groups), ...)`. The final vector of `SymbolFact` is returned, containing all unique symbols from the combined input groups, in the order they appear across the groups.

#### Side Effects

- allocates and returns a new vector of `SymbolFact`
- allocates and temporarily uses an `unordered_set` for deduplication

#### Reads From

- `model` parameter
- `project_root` parameter
- each `groups` parameter (containers of `extract::SymbolID`)

#### Usage Patterns

- collecting deduplicated facts from multiple ID groups for code generation

### `clore::generate::__detail::collect_namespace_facts_cached`

Declaration: `src/generate/evidence.cppm:145`

Definition: `src/generate/evidence.cppm:287`

The function employs a static cache indexed by a composite key of `namespace_name` and `project_root` to avoid redundant traversals of namespace symbols. Upon invocation, it first checks the cache; if no entry exists, it creates a `NamespaceFactsCacheEntry` and populates it by iterating over all symbols within the namespace (obtained via `extract::lookup_symbol` on the provided `model`). Each symbol is converted to a `SymbolFact` using `to_symbol_fact`, then classified by kind into separate vectors (`all_functions`, `all_types`, `all_variables`) using the `is_function_kind`, `is_type_kind`, and `is_variable_kind` predicates. This lazy population ensures that once cached, subsequent calls for the same namespace and project reuse the precomputed facts.

After cache lookup, the function selects the appropriate sub-vector based on the `target_kind` parameter and performs a final filter to exclude the symbol identified by `exclude_id`. The result is a copy of the matching facts (or an empty vector if the namespace string is empty or no symbols of the requested kind exist). Key dependencies include the global `namespace_facts_cache()` map, `to_symbol_fact` for constructing fact objects, and the kind‑checking utility functions.

#### Side Effects

- mutates the `namespace_facts_cache()` by inserting a new cache entry on cache miss

#### Reads From

- parameter `model` (const `extract::ProjectModel`&)
- parameter `namespace_name` (const `std::string`&)
- parameter `project_root` (const `std::string`&)
- parameter `target_kind` (`extract::SymbolKind`)
- parameter `exclude_id` (`extract::SymbolID`)
- static cache returned by `namespace_facts_cache()`
- symbol data via `extract::lookup_symbol(model, sym_id)`
- `sym` fields and `project_root` via `to_symbol_fact`
- predicate functions `is_function_kind`, `is_type_kind`, `is_variable_kind`

#### Writes To

- static cache (`namespace_facts_cache()`) via `cache.emplace`

#### Usage Patterns

- called when building evidence for multiple symbols in the same namespace to avoid redundant traversal
- likely invoked from sibling functions like `collect_facts` or `collect_summaries`

### `clore::generate::__detail::collect_summaries`

Declaration: `src/generate/evidence.cppm:124`

Definition: `src/generate/evidence.cppm:212`

The function `clore::generate::__detail::collect_summaries` accepts a `PageSummaryCache` (a map-like container keyed by strings) and a `std::vector<std::string>` of keys. It iterates over each `key` in the input vector, queries the cache for that key, and if the entry exists (`cache.find(key) != cache.end()`) and the stored string is non-empty (`!it->second.empty()`), appends `it->second` to a local `result` vector. The function returns the collected `result` vector, which may be shorter than the input key list because missing or empty cache entries are silently skipped.

The control flow is a simple linear scan over the keys, relying on the cache’s lookup operation and the empty‑string check to filter summaries. No external functions are called; dependencies are limited to the cache container’s `find` method and the `std::vector` push‑back. The function does not modify the cache or the input keys.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `cache` (of type `PageSummaryCache`)
- `keys` (of type `std::vector<std::string>`)

#### Usage Patterns

- filter summary entries from cache
- collect non-empty cached summaries

### `clore::generate::__detail::to_symbol_fact`

Declaration: `src/generate/evidence.cppm:113`

Definition: `src/generate/evidence.cppm:159`

`clore::generate::__detail::to_symbol_fact` performs a direct field‑by‑field mapping from an `extract::SymbolInfo` input to a `clore::generate::SymbolFact` value. It copies scalar or string fields such as `id`, `qualified_name`, `signature`, `access`, `is_template`, `template_params`, `declaration_line`, and `doc_comment` verbatim. The `kind_label` is produced by calling `extract::symbol_kind_name(sym.kind)`. The `declaration_file` is made relative to the project root via `clore::generate::make_source_relative(sym.declaration_location.file, project_root)`.  

The function has no branching or loops; its entire body is a single aggregate‑initialisation expression. It depends on the `extract::SymbolInfo` type and `extract::symbol_kind_name`, the `SymbolFact` structure, and the `make_source_relative` utility. As a pure conversion helper, it does not perform any I/O, caching, or validation beyond the implicit copying of data.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `extract::SymbolInfo` parameter `sym`
- `std::string` parameter `project_root`
- `extract::symbol_kind_name` (global state implicitly read by function)

#### Writes To

- Returned `SymbolFact` object (local copy)

#### Usage Patterns

- Convert extracted symbol info to symbol fact for further processing
- Used in fact-collection functions within `clore::generate::__detail`

### `clore::generate::build_evidence_for_function_analysis`

Declaration: `src/generate/evidence.cppm:52`

Definition: `src/generate/evidence_builder.cppm:61`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::build_evidence_for_function_analysis` constructs an evidence model for a specified function target by iterating over `page_summaries` and `imported_summaries`. It first resolves the `project_root` and initializes a `model` object, then processes each package (`pack`) to collate `analyses`, `member_ids`, `dependency_ids`, and `symbol_ids`. For each summary, it calls the internal helper `maybe_resolve_snippet` to conditionally resolve inline snippets, and aggregates relevant facts (e.g., `namespace_fact`, `local_ids`). The control flow centers on a nested traversal of `root` components, `mod` modules, and `file_it` entries, populating the evidence data structure that the function finally returns as an integer status code. Dependencies include the project model infrastructure and the snippet‑resolution utility within the anonymous namespace.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Usage Patterns

- called by other evidence-building functions
- used in analysis generation workflows

### `clore::generate::build_evidence_for_function_declaration_summary`

Declaration: `src/generate/evidence.cppm:79`

Definition: `src/generate/evidence_builder.cppm:246`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation begins by initializing the local model, pack, and root structures, then retrieving project-level configuration via `project_root` and `model`. It iterates over imported summaries and page summaries, using `maybe_resolve_snippet` to conditionally resolve any embedded code snippets for each declaration target. Dependencies are collected into `dependency_ids`, `symbol_ids`, and `member_ids` by traversing `analyses` and `pack` entries. The function assembles a hierarchical evidence tree rooted at `root`, linking `namespace_fact`, `local_ids`, and `mod` objects, and finally returns an integer status code representing the outcome of the evidence construction.

#### Side Effects

No observable side effects are evident from the extracted code.

### `clore::generate::build_evidence_for_function_implementation_summary`

Declaration: `src/generate/evidence.cppm:84`

Definition: `src/generate/evidence_builder.cppm:276`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::build_evidence_for_function_implementation_summary` constructs a collection of evidence entries for a given function by first resolving the target symbol via `clore::generate::(anonymous namespace)::maybe_resolve_snippet`. It then fetches `summary_keys`, `dependency_ids`, and `member_ids` from the `model` and iterates over `analyses` to assemble the evidence. For each candidate, the algorithm conditionally calls `maybe_resolve_snippet` to verify snippet availability, then combines data from `page_summaries`, `imported_summaries`, and the `pack`’s symbol information. The function also scans `namespace_fact` entries and `local_ids` to capture cross-references, and it handles file-level evidence via `file_it`. The overall flow depends on the `project_root` for path resolution, the `model` for symbol and snippet metadata, and the `analyses` structure to produce the final evidence set. The return value (an `int`) represents the count or status of the evidence built.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const int &` first parameter
- `const int &` second parameter
- `std::string_view` third parameter

#### Writes To

- returns an `int` handle (no concrete state identified)

#### Usage Patterns

- used to generate evidence for function implementation summaries during documentation generation

### `clore::generate::build_evidence_for_index_overview`

Declaration: `src/generate/evidence.cppm:76`

Definition: `src/generate/evidence_builder.cppm:212`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function begins by establishing the `project_root` and retrieving `page_summaries` and `imported_summaries`. It then initializes a `target` from one of the integer parameters and uses `maybe_resolve_snippet` to conditionally process snippet references. The core algorithm iterates over `page_summaries`, extracting `summary_keys` and `dependency_ids` to build an `analyses` collection. For each summary key, it resolves the corresponding `sym`, gathers `member_ids`, `local_ids`, and `namespace_fact` from the `model`, and constructs a `pack` that aggregates these pieces—including the `root`, `target`, `symbol_ids`, and `dependency_ids`—into a structured evidence object. The function returns a result of type `int`, typically indicating the number of successfully processed summaries or a status code. Dependencies include the `model` (which stores symbol and namespace hierarchies), the `maybe_resolve_snippet` helper, and the `pack` assembly logic that feeds into the index overview generation.

#### Side Effects

No observable side effects are evident from the extracted code.

### `clore::generate::build_evidence_for_module_architecture`

Declaration: `src/generate/evidence.cppm:70`

Definition: `src/generate/evidence_builder.cppm:181`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::build_evidence_for_module_architecture` begins by initializing containers for `project_root`, `page_summaries`, and `imported_summaries`. It then iterates over `summary_keys`; for each key it invokes the helper `clore::generate::(anonymous namespace)::maybe_resolve_snippet` to determine whether the corresponding snippet should be included. If the snippet is resolved, the algorithm populates a `model` with a `namespace_fact`, `local_ids`, `member_ids`, and a `root` structure, while also aggregating entries into `analyses`, `dependency_ids`, and `symbol_ids`. The control flow branches based on the presence of constraints such as `pack` and `target`, and uses `file_it` and `mod` to navigate the module hierarchy.

Throughout the process, `project_root` and `root` serve as base paths, `ns` and `pack` manage namespace and package scoping, and `sym` tracks symbol identifiers. The function returns an integer (likely the count of resolved summaries or a status code), relying on the external `maybe_resolve_snippet` callback and the internal accumulation of evidence into structured analysis records.

#### Side Effects

- Modifies the evidence storage for the module architecture
- May update internal caches or aggregated analysis data

#### Reads From

- The four integer parameters representing entity identifiers
- The `string_view` parameter representing the module name
- Global or context-level analysis stores (e.g., `SymbolAnalysisStore`)

#### Writes To

- Evidence storage (e.g., `EvidencePack` or similar) for the module architecture

#### Usage Patterns

- Called during page generation for module documentation
- Invoked as part of the evidence-building pipeline for module-level pages

### `clore::generate::build_evidence_for_module_summary`

Declaration: `src/generate/evidence.cppm:64`

Definition: `src/generate/evidence_builder.cppm:150`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function constructs evidence for a module summary by aggregating data from multiple analyses and summaries. It first resolves snippet references for the given `target` using `clore::generate::(anonymous namespace)::maybe_resolve_snippet`. Then it iterates over `page_summaries`, `imported_summaries`, and `analyses` to collect `summary_keys`, `dependency_ids`, `member_ids`, `symbol_ids`, `local_ids`, and `namespace_fact`. Key entities such as `pack`, `mod`, `root`, and `ns` are retrieved from the `model` and `project_root`. The function also processes `file_it` to gather file-level information and updates the `model` or `root` object with the collected evidence. Internal control flow involves multiple loops over analysis results, conditionally inserting `IDs` and facts into the evidence structure, and finally returning the assembled `model`. Dependencies include the `model`, `project_root`, and the internal snippet resolution helper.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameters: const int & (four instances) representing module, file, namespace, or depth identifiers
- parameter: `std::string_view` providing a module name or view key

#### Usage Patterns

- called when constructing module summary evidence for a code generation page
- used in the page building pipeline for module documentation

### `clore::generate::build_evidence_for_namespace_summary`

Declaration: `src/generate/evidence.cppm:47`

Definition: `src/generate/evidence_builder.cppm:29`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function constructs a `namespace_fact` for the given `target` namespace by resolving its `sym` and traversing the project's `model`. It begins by retrieving `page_summaries` and `imported_summaries` from `project_root`, then uses `model` to obtain the namespace's `root` node and iterates over associated `file_it`, `mod`, and `pack` elements. For each encountered child symbol, it collects `member_ids`, `symbol_ids`, and `local_ids`, applying `maybe_resolve_snippet` as a filter. The final `namespace_fact` is assembled from these collected identifiers, along with `dependency_ids` derived from `summary_keys` and `dependency_ids` computed from the imported summaries. The resulting evidence is stored in `analyses` and returned as an integer status code.

#### Side Effects

No observable side effects are evident from the extracted code.

### `clore::generate::build_evidence_for_type_analysis`

Declaration: `src/generate/evidence.cppm:56`

Definition: `src/generate/evidence_builder.cppm:90`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function begins by resolving the target entity from the provided parameters, establishing the `project_root` and the `model` that contain the type’s definition. It then collects `page_summaries` and `imported_summaries` for the entity, merging them while using `clore::generate::(anonymous namespace)::maybe_resolve_snippet` to filter out incomplete or unresolvable snippet references. A set of `dependency_ids` is built by traversing the `pack` and `analyses` associated with the target, and these `IDs` are later used to prune duplicate evidence from the final structure. The algorithm then iterates over the model’s symbol graph—processing `sym`, `ns`, `mod`, and `file_it`—to extract relevant `namespace_fact`, `member_ids`, and `local_ids`, which are aggregated into a single `analyses` container. Finally, it constructs a `root` evidence node that summarises the type’s context, combines the collected fact groups, and returns an integer (likely a count of successfully assembled evidence items or a status code).

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- const int & (likely store or symbol identifier)
- const int & (likely additional identifier)
- `std::string_view` (type name or analysis key)

#### Usage Patterns

- Called during page generation for type symbols
- Used to prepare evidence for type analysis prompts

### `clore::generate::build_evidence_for_type_declaration_summary`

Declaration: `src/generate/evidence.cppm:89`

Definition: `src/generate/evidence_builder.cppm:310`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function accepts four parameters—three integer handles and a string view—and returns an integer result. It constructs evidence for a type declaration summary by navigating the semantic model. First, it resolves the declaration target from the first integer parameter, then uses the second integer as a pack identifier to locate the relevant analysis group. The third integer serves as a project root handle, which is used to retrieve the directory tree (`root`) and a mutable document model (`model`). The function iterates over `page_summaries` and `imported_summaries` to collect `summary_keys` and `dependency_ids`, filtering entries that are reachable from the current `project_root`. It calls `maybe_resolve_snippet` for each candidate to verify resolvability. Then it builds a set of `member_ids` by inspecting `analyses` associated with the target `pack`, and assembles `namespace_fact` and `local_ids` from the symbol table. Finally, it constructs a proof object (evidence) inside the `model` by correlating `member_ids`, `dependency_ids`, and the resolved `symbol_ids`, returning a status code that indicates success or failure of the summary generation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- first `const int &` parameter (likely symbol analysis store or page plan set)
- second `const int &` parameter
- third `const int &` parameter
- `std::string_view` parameter (type name)

#### Writes To

- return value (int, indicating status)

#### Usage Patterns

- invoked during type declaration summary generation

### `clore::generate::build_evidence_for_type_implementation_summary`

Declaration: `src/generate/evidence.cppm:94`

Definition: `src/generate/evidence_builder.cppm:342`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function constructs evidence for a type implementation summary by iterating over the model's analyses and packs of identifiers. It traverses the model graph starting from `root`, processing `target`'s dependencies and using `project_root` to resolve paths. For each `pack` it collects `summary_keys`, `dependency_ids`, `member_ids`, and `symbol_ids` from the model, and utilizes `namespace_fact` to capture enclosing namespace context. The internal control flow relies on `maybe_resolve_snippet` to conditionally resolve snippet references, and it populates the `page_summaries` and `imported_summaries` structures by walking nodes such as `mod`, `file_it`, and `sym`. The algorithm depends on the `model` object for symbol and analysis lookups, and coordinates with `root` to ensure all relevant evidence is aggregated for the implementation summary.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const int &` first parameter (likely a symbol or analysis store identifier)
- `const int &` second parameter (likely a context or page plan identifier)
- `std::string_view` third parameter (likely a type name or key)

#### Usage Patterns

- called during generation of type implementation summary pages
- invoked by higher-level page building functions like `clore::generate::append_type_member_sections`

### `clore::generate::build_evidence_for_variable_analysis`

Declaration: `src/generate/evidence.cppm:60`

Definition: `src/generate/evidence_builder.cppm:121`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function builds evidence by first loading the project model from `project_root` and retrieving the `page_summaries` and `imported_summaries` collections. It then resolves the variable analysis `target` to a specific symbol `sym` within the model, and uses the `pack` to gather a set of `analyses` and their associated `dependency_ids`. For each analysis, the algorithm collects local facts via `namespace_fact`, `local_ids`, and member-level identifiers in `member_ids`, as well as broader `symbol_ids`. A key step involves calling `maybe_resolve_snippet` for each relevant entry to determine whether a snippet needs to be materialized, influencing which summary keys are ultimately included.

Control flow iterates through the analyses and their dependencies, populating `summary_keys` and potentially updating `root` and `mod` references to reflect the current context. The function consults the model’s hierarchy—traversing namespaces and modules through `ns` and `mod`—and synchronizes with `file_it` to align file-level evidence. Throughout, it relies on the cached `page_summaries` and `imported_summaries` to avoid redundant work, and all collected evidence is aggregated into a structure that captures the variable’s syntactic and semantic dependencies.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const int &` (first parameter)
- `const int &` (second parameter)
- `std::string_view` (third parameter)

#### Usage Patterns

- Called during documentation generation for variable symbols
- Part of the evidence building pipeline for analysis

### `clore::generate::build_prompt`

Declaration: `src/generate/evidence.cppm:106`

Definition: `src/generate/evidence.cppm:663`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function begins by calling `prompt_template_of(kind)` to retrieve the appropriate template string based on the prompt kind. If the returned view is empty, it returns a `PromptError` with an explanatory message. Otherwise, it constructs a `std::string` from the template and delegates to `instantiate_prompt_with_evidence`, passing the template string, the `EvidencePack`, and the result of `format_evidence_text(evidence)` as the evidence text. This internal helper merges the formatted evidence into the template, producing the final prompt string. The only external dependency is the pair of helpers `format_evidence_text` (which renders all evidence sections into a single string) and `instantiate_prompt_with_evidence` (which performs template substitution).

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `kind`
- parameter `evidence`
- internal prompt template store via `prompt_template_of(kind)`
- `format_evidence_text(evidence)`

#### Writes To

- returned `std::expected<std::string, PromptError>` object

#### Usage Patterns

- called by prompt-building infrastructure
- used to generate documentation prompts from evidence packs

### `clore::generate::format_evidence_text`

Declaration: `src/generate/evidence.cppm:98`

Definition: `src/generate/evidence.cppm:592`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Implementation: [Implementation](functions/format-evidence-text.md)

The function `clore::generate::format_evidence_text` acts as a thin delegation wrapper around `clore::generate::format_evidence_text_bounded`. It calls that bounded function with `std::numeric_limits<std::size_t>::max()` as the maximum length parameter, effectively disabling any truncation of the generated evidence text. This design centralises the actual evidence‑text construction logic in the bounded variant, while providing a convenient unbounded interface for callers that do not need to enforce a length cap. No additional algorithm or control flow is introduced; the entire behaviour is determined by `format_evidence_text_bounded`, which handles the accumulation of text sections, symbol facts, source snippets, and analysis results under a configurable length constraint.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- pack parameter of type `const EvidencePack &`

#### Usage Patterns

- called by `clore::generate::build_prompt` to obtain the full evidence text for a prompt

### `clore::generate::format_evidence_text_bounded`

Declaration: `src/generate/evidence.cppm:100`

Definition: `src/generate/evidence.cppm:596`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Implementation: [Implementation](functions/format-evidence-text-bounded.md)

The function `clore::generate::format_evidence_text_bounded` assembles a Markdown evidence block from an `EvidencePack` while respecting a caller-provided `max_length` budget. It preallocates a result string with `text.reserve(4096)`, then attempts to write the top-level heading `"## EVIDENCE\n\n"` via `append_if_fits`; if that fails, it returns an empty string immediately.  

Internally, it defines two local helper structs — `SymbolSection` (holding a title, a pointer to a `std::vector<SymbolFact>` and a render function pointer) and `TextSection` (analogous for `std::vector<std::string>` items). It creates an `std::array` of four `SymbolSection` instances covering target facts, local context, dependency context, and reverse usage context, in that order. A second array holds two `TextSection` instances for source snippets and related page summaries. The function iterates over each section array, calling `append_section_bounded` for each section to add its title and rendered items, always checking against `max_length` to avoid exceeding the budget. The sole explicitly listed dependency is `append_if_fits`, which is used for the initial heading and presumably also internally by `append_section_bounded` for individual lines or items. The final returned `text` contains the full (possibly truncated) evidence string.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `EvidencePack::target_facts`
- `EvidencePack::local_context`
- `EvidencePack::dependency_context`
- `EvidencePack::reverse_usage_context`
- `EvidencePack::source_snippets`
- `EvidencePack::related_page_summaries`
- `max_length` parameter

#### Usage Patterns

- called by `clore::generate::format_evidence_text`
- used to generate bounded evidence strings for prompts

## Internal Structure

The `generate:evidence` module, implemented in `src/generate/evidence.cppm`, is responsible for collecting, structuring, and formatting evidence required to produce LLM prompts for documentation generation. It imports the `extract` module for symbol and analysis data, and the `generate:model` module for core types such as `EvidencePack`, `SymbolFact`, and `PromptKind`. Internally, the module is decomposed into three layers: a public API surface (functions like `build_evidence_for_namespace_summary`, `build_evidence_for_function_analysis`, `format_evidence_text`, and `build_prompt`), an internal `__detail` namespace that handles fact collection with caching (e.g., `collect_namespace_facts_cached`, `collect_analysis_summaries_impl`), and anonymous‑namespace helpers that provide low‑level rendering, prompt template constants, and bounded formatting logic. The data structures `EvidencePack` and `SymbolFact` serve as the primary evidence containers, while `TextSection` and `SymbolSection` are used internally by the bounded formatter to organize output. A `NamespaceFactsCacheEntry` structure enables efficient reuse of namespace facts across multiple sibling symbol queries, avoiding redundant traversals.

## Related Pages

- [Module extract](../extract/index.md)
- [Module generate:model](model.md)

