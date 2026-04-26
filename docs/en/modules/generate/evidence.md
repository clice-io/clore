---
title: 'Module generate:evidence'
description: 'The generate:evidence module is responsible for assembling and formatting the evidence used to drive documentation generation. It provides a family of public build_evidence_for_* entry points (e.g., build_evidence_for_namespace_summary, build_evidence_for_function_analysis, build_evidence_for_type_implementation_summary) that collect relevant facts, analyses, and summaries from extraction and model data, packaging them into EvidencePack structs. These evidence packs are then consumed by formatting functions (format_evidence_text, format_evidence_text_bounded) and a prompt builder (build_prompt), which produce the final text or AI‑prompt inputs for each documentation page.'
layout: doc
template: doc
---

# Module `generate:evidence`

## Summary

The `generate:evidence` module is responsible for assembling and formatting the evidence used to drive documentation generation. It provides a family of public `build_evidence_for_*` entry points (e.g., `build_evidence_for_namespace_summary`, `build_evidence_for_function_analysis`, `build_evidence_for_type_implementation_summary`) that collect relevant facts, analyses, and summaries from extraction and model data, packaging them into `EvidencePack` structs. These evidence packs are then consumed by formatting functions (`format_evidence_text`, `format_evidence_text_bounded`) and a prompt builder (`build_prompt`), which produce the final text or AI‑prompt inputs for each documentation page.

The module owns the public types `EvidencePack` and `SymbolFact`, along with internal detail helpers for caching namespace‑level facts, merging fact collections, and rendering source snippets. Its public interface covers all major page categories in the generation pipeline—namespace, module, index, architecture, function (declaration, implementation, analysis), variable analysis, and type (analysis, declaration, implementation)—and provides the bounded text formatting needed for length‑constrained contexts.

## Imports

- [`extract`](../extract/index.md)
- [`generate:model`](model.md)
- `std`

## Imported By

- [`generate:analysis`](analysis.md)
- [`generate:evidence_builder`](index.md)
- [`generate:scheduler`](scheduler.md)

## Types

### `clore::generate::EvidencePack`

Declaration: `generate/evidence.cppm:22`

Definition: `generate/evidence.cppm:22`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The `clore::generate::EvidencePack` struct is a plain aggregate data type that consolidates all evidence collected for a single generation request. It holds the target `page_id` and `prompt_kind` to identify the generation context, along with `subject_name` and `subject_kind` that describe the symbol or element being documented. The vectors of `SymbolFact` — `target_facts`, `local_context`, `dependency_context`, and `reverse_usage_context` — organize facts by scoping category, while `source_snippets` stores relevant code excerpts and `related_page_summaries` holds brief descriptions of linked pages.

The struct does not enforce invariants internally; correctness depends on the caller populating each field consistently. For example, `target_facts` should contain facts directly about the subject, whereas `local_context` and `dependency_context` provide surrounding or transitive facts. The `page_id` and `prompt_kind` are expected to be non‑empty, and all vectors are typically filled with data from the preceding analysis passes. No member functions are provided — the struct serves purely as a container for downstream generation logic.

#### Invariants

- Fields are set externally and not validated internally.
- Vectors may be empty; no guarantee of non‑emptiness.
- `page_id` and `prompt_kind` are typically non‑empty strings.

#### Key Members

- `page_id`
- `prompt_kind`
- `subject_name`
- `subject_kind`
- `target_facts`
- `local_context`
- `dependency_context`
- `reverse_usage_context`
- `related_page_summaries`
- `source_snippets`

#### Usage Patterns

- Populated by evidence collectors and passed to generation pipelines.
- Consumed by prompt builders to construct input for LLM or template engines.
- Acts as the primary input for `clore::generate` functions.

### `clore::generate::PromptError`

Declaration: `generate/evidence.cppm:90`

Definition: `generate/evidence.cppm:90`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::PromptError` is a lightweight error type that stores a single `std::string` member `message`. Its implementation consists solely of this data member, with no additional invariants beyond the natural properties of `std::string`. The class is trivially constructible, copyable, and movable, serving as a simple container for an error description without any validation or resource management logic. All behavior is delegated to the `std::string` member, making the struct suitable for use in error‑handling mechanisms such as `std::expected` or exception wrappers where the only required information is a human‑readable explanation of the failure.

#### Invariants

- The `message` member may contain any string, including an empty string.

#### Key Members

- The `message` member of type `std::string`

#### Usage Patterns

- Constructed with an error description string to represent a prompt generation error.

### `clore::generate::SymbolFact`

Declaration: `generate/evidence.cppm:9`

Definition: `generate/evidence.cppm:9`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The struct `clore::generate::SymbolFact` is a plain aggregate that stores the extracted metadata of a single symbol for use during documentation generation. All members are public and directly accessible. The field `id` of type `extract::SymbolID` uniquely identifies the symbol, while the string members such as `qualified_name`, `signature`, `kind_label`, `access`, `template_params`, `declaration_file`, and `doc_comment` hold the textual attributes. The boolean `is_template` defaults to `false`, and the integral `declaration_line` defaults to `0`, forming the only implicit invariants. No additional structural invariants are enforced by the type; the struct is intended to be populated by the extraction phase and then consumed by the generation phase as a flat collection of facts.

#### Invariants

- No documented invariants beyond default member initializers
- Fields may be empty or zero if not populated

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

- Populated during extraction and used to represent a symbol's metadata
- Consumed by code generation stages

## Functions

### `clore::generate::__detail::collect_analysis_summaries`

Declaration: `generate/evidence.cppm:121`

Definition: `generate/evidence.cppm:241`

This function is a thin forwarding wrapper that delegates all work to `collect_analysis_summaries_impl`. It accepts the same three parameters (`analyses`, `model`, and `ids`) and passes them directly to the implementation, along with an identity lambda `[](extract::SymbolID id) { return id; }`. The lambda is used as the `SymbolIDOf` callback; by returning the id unchanged, it indicates that the provided `ids` already represent the exact symbol identifiers to be processed, without any remapping or filtering. Consequently, the function itself contains no additional algorithm or control flow—its sole purpose is to serve as a public entry point that invokes the shared implementation with a trivial projection.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses`
- `model`
- `ids`

#### Usage Patterns

- wrapping `collect_analysis_summaries_impl` with identity projection
- retrieving analysis summaries for a set of symbol `IDs`

### `clore::generate::__detail::collect_analysis_summaries_impl`

Declaration: `generate/evidence.cppm:116`

Definition: `generate/evidence.cppm:213`

The function iterates over each element in the `items` range, deduplicating by calling `symbol_id_of` to derive an `extract::SymbolID` and checking against a `seen` unordered set. If the ID is new, it looks up the corresponding symbol via `extract::lookup_symbol` on the `model`; if the symbol exists, it retrieves its analysis summary through `analysis_overview_markdown` from the `analyses` store. Only non-null, non-empty summary strings are appended to the result vector, which is returned after processing all items. This implementation relies on `extract::lookup_symbol` for symbol resolution and `analysis_overview_markdown` for retrieving pre-computed analysis text from the analysis store.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses` (const `SymbolAnalysisStore&`)
- `model` (const `extract::ProjectModel&`)
- `items` (const `Range&`)
- `symbol_id_of` callable
- symbol data pointed by `extract::lookup_symbol` result

#### Usage Patterns

- Used to collect summaries for a range of symbols or declarations
- Provides deduplication by symbol ID

### `clore::generate::__detail::collect_facts`

Declaration: `generate/evidence.cppm:103`

Definition: `generate/evidence.cppm:163`

`clore::generate::__detail::collect_facts` accepts an `extract::ProjectModel`, a vector of `extract::SymbolID`, and a project root string. It deduplicates the input `IDs` using an internal `std::unordered_set<extract::SymbolID>`, then for each unique ID it calls `extract::lookup_symbol` to retrieve the corresponding symbol node. If the lookup succeeds, the function converts the node into a `SymbolFact` via the helper `to_symbol_fact` and appends it to a local `std::vector<SymbolFact>`, which becomes the return value. The function relies on the `extract` library for symbol resolution and on the `to_symbol_fact` utility for mapping the raw extracted data into the higher‑level fact representation used by the evidence‑generation pipeline.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model` parameter
- `ids` parameter
- `project_root` parameter
- result of `extract::lookup_symbol`

#### Writes To

- local `std::vector<SymbolFact> facts`
- local `std::unordered_set<extract::SymbolID> seen`

#### Usage Patterns

- used to convert a list of symbol `IDs` into a deduplicated vector of facts
- likely called by higher‑level generation functions that need symbol facts

### `clore::generate::__detail::collect_merged_facts`

Declaration: `generate/evidence.cppm:108`

Definition: `generate/evidence.cppm:179`

The function `clore::generate::__detail::collect_merged_facts` merges the symbol facts from multiple heterogeneous groups of symbol `IDs` into a single deduplicated vector. It receives an `extract::ProjectModel`, a `project_root` path, and a variadic pack `Groups...` (each group is a container of `extract::SymbolID`). An `std::unordered_set<extract::SymbolID>` named `seen` tracks already processed identifiers, preventing duplicates across groups.

A local lambda `append_group` iterates over a single group’s `IDs`. For each ID, it attempts insertion into `seen`; if the ID is already present, the lambda skips it. Otherwise, it calls `extract::lookup_symbol` on the model to obtain the corresponding symbol object, and if found, appends the result of `to_symbol_fact` to the `facts` vector. The variadic expansion `(append_group(groups), ...)` applies `append_group` to each group in left‑to‑right order via a fold expression. After all groups are processed, the function returns `facts`—a `std::vector<SymbolFact>` containing the deduplicated, collected facts.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- model
- `project_root`
- groups (each being a container of `extract::SymbolID`)
- symbol data retrieved via `extract::lookup_symbol`

#### Usage Patterns

- Merging symbol facts from multiple analysis groups
- Deduplicating symbol facts by symbol ID
- Collecting facts for batch documentation generation

### `clore::generate::__detail::collect_namespace_facts_cached`

Declaration: `generate/evidence.cppm:133`

Definition: `generate/evidence.cppm:275`

The function `clore::generate::__detail::collect_namespace_facts_cached` implements a memoized retrieval of all `SymbolFact` entries belonging to a given namespace. It takes a `extract::ProjectModel`, a `namespace_name`, a `project_root`, a `target_kind` indicating the kind of symbol being collected for, and an `exclude_id` used to omit the requesting symbol itself. The function first constructs a `cache_key` from `namespace_name` and `project_root`, then looks it up in a static cache returned by `namespace_facts_cache`. On a cache miss, it locates the namespace in the model and iterates over its symbols, calling `to_symbol_fact` on each one. The resulting facts are partitioned into three vectors inside a `NamespaceFactsCacheEntry`: `all_functions`, `all_types`, and `all_variables`, based on the result of `is_function_kind`, `is_type_kind`, or `is_variable_kind`. The entry is then inserted into the cache.

On cache hit, the function selects the appropriate vector according to `target_kind` using the same set of kind predicates. If no matching vector exists (e.g., an unknown kind) or if `namespace_name` is empty, it returns an empty result. Otherwise, it copies all facts from the selected source vector, skipping any fact whose `id` equals `exclude_id` to avoid self‑reference. The returned vector contains the remaining namespace‑level facts, ready for downstream evidence construction. The caching design avoids re‑traversing the namespace symbols for every sibling symbol during a multi‑symbol generation pass.

#### Side Effects

- updates the static namespace facts cache by inserting new entries
- may cause dynamic memory allocation for cache entries and result vectors

#### Reads From

- parameter `model` (project model)
- parameter `namespace_name`
- parameter `project_root`
- parameter `target_kind`
- parameter `exclude_id`
- static `namespace_facts_cache()` cache map
- results from `extract::lookup_symbol`
- results from `to_symbol_fact`

#### Writes To

- static `namespace_facts_cache()` via insertion/emplace

#### Usage Patterns

- called when building evidence for multiple symbols in the same namespace to avoid repeated namespace traversals
- used by higher-level fact collection functions such as `collect_facts`

### `clore::generate::__detail::collect_summaries`

Declaration: `generate/evidence.cppm:112`

Definition: `generate/evidence.cppm:200`

The function iterates over each key in the input `keys` vector. For each key, it performs a lookup in the `cache` map via `cache.find(key)`. If the iterator `it` is found (i.e., not equal to `cache.end()`) and the associated value (`it->second`) is not empty, that value is appended to the `result` vector. The function depends on the `PageSummaryCache` type supporting standard map lookup semantics; the returned vector preserves the order of keys from the input that had a non‑empty entry.

Control flow is linear—no branching beyond the condition check, no special handling for duplicate keys or missing entries. The function is a pure retrieval utility, extracting pre‑computed summary strings from a cache by their identifiers. It relies implicitly on the caller ensuring that `keys` contains valid lookup identifiers and that `cache` has been populated beforehand.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `cache` parameter
- `keys` parameter

#### Usage Patterns

- Retrieve a subset of cached summaries by specified keys

### `clore::generate::__detail::to_symbol_fact`

Declaration: `generate/evidence.cppm:101`

Definition: `generate/evidence.cppm:147`

The function `clore::generate::__detail::to_symbol_fact` converts a `SymbolInfo` object extracted by the `extract` layer into a `SymbolFact` which is the canonical fact representation used in prompt construction. It performs a direct field‑wise copy from `sym` to the returned `SymbolFact`: copying `sym.id`, `sym.qualified_name`, `sym.signature`, `sym.access`, `sym.is_template`, `sym.template_params`, and `sym.doc_comment`. The `kind_label` is produced by calling `extract::symbol_kind_name` on `sym.kind` and converting the result to `std::string`. The `declaration_file` is made project‑relative by passing the absolute path from `sym.declaration_location.file` through `clore::generate::make_source_relative` with `project_root`. The `declaration_line` is taken directly from `sym.declaration_location.line`. The function is a trivial aggregator with no branching or iteration; its only dependencies are the `extract` module’s symbol‑info types and `make_source_relative` for path normalisation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `sym.id`
- `sym.qualified_name`
- `sym.signature`
- `sym.kind`
- `sym.access`
- `sym.is_template`
- `sym.template_params`
- `sym.declaration_location.file`
- `sym.declaration_location.line`
- `sym.doc_comment`
- `project_root`

#### Usage Patterns

- Called to produce a `SymbolFact` from an extracted symbol for further analysis or serialization.

### `clore::generate::build_evidence_for_function_analysis`

Declaration: `generate/evidence.cppm:40`

Definition: `generate/evidence_builder.cppm:53`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function constructs evidence for function analysis by orchestrating several sub‑processes. It first initializes a `root` data structure and a `project_root` context, then iterates over a `pack` of symbols (`sym`). For each symbol, it invokes `clore::generate::(anonymous namespace)::maybe_resolve_snippet` to conditionally resolve snippet references, and collects the results into `analyses` and `page_summaries`. The function then builds a `model` by aggregating data from `analyses`, `imported_summaries`, and `namespace_fact` entries, using `file_it` to traverse translation units and assemble per‑file evidence. Finally, it returns an `int` representing the count of successfully processed function analyses.

Control flow proceeds linearly: setup → symbol resolution loop → model construction → evidence packing. The function depends on `model` to store intermediate analysis results, `pack` for symbol iteration, `project_root` for file path resolution, and the helper `maybe_resolve_snippet` to handle snippet‑based declarations. The `target` parameter (third argument) governs which specific function analysis is built, while `analyses` and `page_summaries` are gradually populated and later merged into the final output `pack`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Usage Patterns

- used to construct evidence for function analysis documentation

### `clore::generate::build_evidence_for_function_declaration_summary`

Declaration: `generate/evidence.cppm:67`

Definition: `generate/evidence_builder.cppm:238`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function iterates over each `analyses` collection and its associated `pack` structures, using the `project_root` and `root` (the document or AST entry point) to navigate the declaration context. For each `pack`, it calls `maybe_resolve_snippet` to resolve inline code references, then collects resolved content into `page_summaries` and `imported_summaries`. These summaries are assembled into a `model` object, which also incorporates namespace context via `namespace_fact` and `ns`. The `model` is then aggregated with information from `mod` (module context) and combined with the original `target` symbol to produce the final evidence. The entire algorithm relies on the interplay between the `pack`-based analysis structure and the `maybe_resolve_snippet` helper to transform raw analysis data into a structured summary model.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameters: `const int &` (first, second, third, fourth) and `int` (fifth)

#### Usage Patterns

- Called during evidence construction for function declaration summary pages
- Used in the page generation pipeline for symbol documentation

### `clore::generate::build_evidence_for_function_implementation_summary`

Declaration: `generate/evidence.cppm:72`

Definition: `generate/evidence_builder.cppm:268`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::build_evidence_for_function_implementation_summary` constructs the evidence data for a given target function by traversing the project structure from a specified `project_root`. It iterates over file entries (`file_it`) and corresponding page summaries, and for each relevant location it calls `maybe_resolve_snippet` to verify whether a snippet matches. The evidence is built by assembling `analyses` and `model` objects, which are then stored in a `pack` along with the `target`. The internal control flow follows a linear path: it first resolves the root assembly, then iterates over the files, collecting evidence from each snippet, and finally populates the model with the gathered information before packaging it for output. Dependencies include the project root directory, the target identifier, and helper functions like `maybe_resolve_snippet` that handle snippet validation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Usage Patterns

- called during evidence construction for function pages
- part of the page generation pipeline

### `clore::generate::build_evidence_for_index_overview`

Declaration: `generate/evidence.cppm:64`

Definition: `generate/evidence_builder.cppm:204`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function begins by traversing the directory tree rooted at `root`, using `project_root` as the base path for resolution. For each file encountered via `file_it`, it invokes `clore::generate::(anonymous namespace)::maybe_resolve_snippet` to conditionally resolve snippet references and accumulate the results into `analyses`. It then builds a `model` by processing each resolved `sym` and `target` within the current `pack`, constructing `page_summaries` and `imported_summaries` that reflect the index’s structure.

After the traversal, the function consolidates the collected `analyses`, `pack`, and `model` into a `namespace_fact` representation, combining data from the current module `mod` and any imported summaries. Finally, it returns an integer status code indicating success or failure of the evidence generation process. The entire flow depends on the internal `maybe_resolve_snippet` utility and the iterative assembly of `page_summaries` from the filesystem contents under `project_root`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the two integer parameters

#### Usage Patterns

- called during page generation for index overview
- used in documentation generation pipeline

### `clore::generate::build_evidence_for_module_architecture`

Declaration: `generate/evidence.cppm:58`

Definition: `generate/evidence_builder.cppm:173`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function first sets up the `project_root` and `root` directories, then initializes a data structure `pack` to hold the module information. It iterates over files or packages using `file_it`, and for each, performs initial analysis via `analyses`. For each target symbol (`sym`) or module (`mod`), it calls `clore::generate::(anonymous namespace)::maybe_resolve_snippet` to conditionally resolve snippet references, collecting the results into `page_summaries`. A `model` is built from the resolved data, incorporating `namespace_fact` and `imported_summaries`, and the evidence is finally constructed by consolidating this model with the original `root` and `target` information. The algorithm relies on the `pack` container to aggregate per‑module data and the `analyses` variable to drive per‑source processing, with `maybe_resolve_snippet` serving as the key dependency for conditional snippet resolution.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- integer parameters representing module context (e.g., module ID, file ID, analysis store)

#### Usage Patterns

- called when building documentation for a module to generate architecture evidence
- used in module page generation pipelines

### `clore::generate::build_evidence_for_module_summary`

Declaration: `generate/evidence.cppm:52`

Definition: `generate/evidence_builder.cppm:142`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function begins by establishing a `project_root` and iterating over `pack` or `mod` entities. For each `target`, it invokes `clore::generate::(anonymous namespace)::maybe_resolve_snippet` to conditionally process snippet references, using `namespace_fact` and `analyses` to resolve context. It then populates `page_summaries` by aggregating data from `analyses` and `model` structures, and incorporates `imported_summaries` when cross-module references are detected. The control flow uses a `root` variable to track the current context and `file_it` to traverse source files, with `sym`, `ns`, and `target` guiding the resolution. The algorithm relies on the internal helper `maybe_resolve_snippet` to decide whether a snippet should be expanded or left as a placeholder, and ultimately returns an integer representing the number of successfully built summary entries or an error indicator.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameters (const int &) representing module identifiers and analysis data
- internal analysis store (e.g., `SymbolAnalysisStore`, implied by local context functions)

#### Usage Patterns

- called during generation of module summary pages
- used to produce the `EvidencePack` or markdown content for a module overview

### `clore::generate::build_evidence_for_namespace_summary`

Declaration: `generate/evidence.cppm:35`

Definition: `generate/evidence_builder.cppm:21`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function begins by deriving a `project_root` and then iterates over `file_it` entries within a `root` structure to accumulate `page_summaries`. For each page summary, it optionally invokes the helper `clore::generate::(anonymous namespace)::maybe_resolve_snippet` to conditionally resolve snippet references. After gathering all summaries, it processes a targeted `pack` and `sym` to extract relevant `analyses` and `model` data. The core loop builds a `namespace_fact` by traversing `ns` and `mod` elements, incorporating `imported_summaries` from external packs. Finally, the function constructs the evidence for the namespace summary by combining the resolved `target` with the collected `page_summaries` and `analyses`, returning an integer result that indicates the success or count of the operation. The entire flow depends on the helper function for snippet resolution and on the local data structures `root`, `pack`, and `project_root` to navigate the module hierarchy.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the three `const int &` parameters (likely representing namespace identifier, analysis store, and other context)
- the `int` parameter (possibly a page plan or other index)

#### Writes To

- the returned `int` (an identifier for the built evidence)

#### Usage Patterns

- called during namespace summary page generation
- used in `build_namespace_page_root` or similar page assembly functions

### `clore::generate::build_evidence_for_type_analysis`

Declaration: `generate/evidence.cppm:44`

Definition: `generate/evidence_builder.cppm:82`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::build_evidence_for_type_analysis` orchestrates the collection of type‑analysis evidence across a project. It first traverses the `page_summaries` rooted at `project_root`, applying the helper `maybe_resolve_snippet` to filter resolvable code snippets. For each valid snippet, it populates an `analyses` structure, which is then used to construct a `model` that incorporates `pack`‑level metadata (e.g., module and namespace facts). The control flow iterates over `root`, resolves `target` elements, and builds `imported_summaries` from external dependencies before finalizing the evidence model. The function depends on `maybe_resolve_snippet` for snippet resolution, `pack` for package context, and `project_root` for file‑system navigation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the first `const int &` parameter
- the second `const int &` parameter
- the third `int` parameter

#### Usage Patterns

- called during documentation generation to produce evidence for type analysis pages
- likely invoked by higher-level page building functions such as `build_page_root` or `build_page_plan_set`

### `clore::generate::build_evidence_for_type_declaration_summary`

Declaration: `generate/evidence.cppm:77`

Definition: `generate/evidence_builder.cppm:302`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function begins by establishing a working `root` directory and `project_root` path, then iterates over a range represented by `file_it`. For each element in the pack, it calls the helper `clore::generate::(anonymous namespace)::maybe_resolve_snippet` to conditionally resolve snippet references. The core logic then builds a collection of `page_summaries` and `imported_summaries` by inspecting `analyses`, `model`, and the target `sym` from the current `pack`. A `namespace_fact` is extracted and used together with `ns` and `mod` to populate the evidence. The function synthesises information from `analyses`, `model`, and `pack` to produce a final summary, returning an integer that likely represents a result code or count.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameters of type `const int &` and `int` (likely representing type ID, analysis store, or other identifiers)

#### Usage Patterns

- Called by other evidence-building functions or page generation routines to produce type declaration documentation

### `clore::generate::build_evidence_for_type_implementation_summary`

Declaration: `generate/evidence.cppm:82`

Definition: `generate/evidence_builder.cppm:334`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function builds an evidence summary for a given type implementation by iterating over a project root and collecting relevant facts. It first resolves the target type symbol, then traverses the module hierarchy using `root` and `project_root` to locate the associated source files. For each relevant file, it processes `page_summaries` and `imported_summaries` to gather documentation snippets, calling `clore::generate::(anonymous namespace)::maybe_resolve_snippet` to conditionally resolve inline references. The algorithm aggregates namespace and module facts (`ns`, `mod`, `namespace_fact`) and updates the `model` and `analyses` collections, finalizing the evidence payload into `pack`. Control flow depends on the presence of `sym`, `target`, and the availability of resolved snippets, with early exits if the type cannot be located.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `clore::generate::build_evidence_for_type_implementation_summary` parameters (analysis store and type identifier)

#### Usage Patterns

- Called during page building for type implementation summaries
- Part of the evidence generation step before rendering markdown

### `clore::generate::build_evidence_for_variable_analysis`

Declaration: `generate/evidence.cppm:48`

Definition: `generate/evidence_builder.cppm:113`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function begins by initializing a `root` variable and then iterates over a `pack` of analyses. For each `target` in the `analyses` range, it calls the internal helper `maybe_resolve_snippet` to conditionally resolve a code snippet; if that succeeds, it constructs a `namespace_fact` from the current `ns` and `mod` context. The function then builds a `model` by combining `root`, `project_root`, and `imported_summaries` together with the current `page_summaries`. For each analysis, it records evidence by associating the `target` with the derived `model` and an incrementing integer counter. The dependencies include the anonymous-namespace function `maybe_resolve_snippet` and the types or structures represented by `project_root`, `pack`, `analyses`, `ns`, `mod`, and `model`, all of which are assumed to be defined within the `clore::generate` module.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `SymbolAnalysisStore` (via an opaque parameter)
- variable identifiers passed as integer references

#### Writes To

- an evidence pack (opaque integer handle)

#### Usage Patterns

- called by page building functions to include variable analysis evidence
- used in `build_symbol_analysis_prompt` and `render_page_markdown`

### `clore::generate::build_prompt`

Declaration: `generate/evidence.cppm:94`

Definition: `generate/evidence.cppm:651`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::build_prompt` constructs a prompt string by first resolving the appropriate template via `prompt_template_of(kind)`. If the returned template is empty, it returns an error of type `PromptError` with a descriptive message. Otherwise, it delegates to `instantiate_prompt_with_evidence`, passing the template, the `evidence` pack, and a pre‑formatted evidence text obtained from `format_evidence_text(evidence)`.  

The internal flow is a straightforward retrieval‑and‑fill pattern: the template is selected based on the `kind` discriminator, and then the evidence pack is rendered into a textual representation by `format_evidence_text`, which internally uses multiple bounded formatting routines and helpers like `format_evidence_text_bounded` to collect symbol facts, namespaces, analysis summaries, and other context fields such as `local_context`, `dependency_context`, `reverse_usage_context`, `related_page_summaries`, `source_snippets`, etc. The final filled prompt is returned or propagated through the caller.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- kind
- evidence
- `prompt_template_of`(kind)

#### Usage Patterns

- construct prompt message for LLM
- generate prompt text from template and evidence

### `clore::generate::format_evidence_text`

Declaration: `generate/evidence.cppm:86`

Definition: `generate/evidence.cppm:580`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::format_evidence_text` serves as the unbounded public entry point for formatting evidence text. Its entire implementation delegates to `format_evidence_text_bounded`, passing `std::numeric_limits<std::size_t>::max()` as the maximum length, effectively disabling any truncation. This design separates the bounded and unbounded formatting paths, keeping the core logic in `format_evidence_text_bounded` while providing a convenience wrapper that avoids requiring callers to supply an explicit size limit. The function’s control flow is a single call with no additional validation or preprocessing; all structural decisions (section building, prompt instantiation, and fact collection) are inherited from the bounded implementation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- pack (const `EvidencePack`&)

#### Usage Patterns

- Used to format evidence text when no size bound is required.

### `clore::generate::format_evidence_text_bounded`

Declaration: `generate/evidence.cppm:88`

Definition: `generate/evidence.cppm:584`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function constructs a structured evidence string by iterating over two categories of sections: symbol sections and text sections. It first attempts to append a top-level `## EVIDENCE` header, returning an empty string if the header alone exceeds `max_length`. The four symbol sections—target, local context, dependencies, and used by—are represented by `SymbolSection` instances that hold a title, a pointer to a vector of `SymbolFact`, and a render function pointer; each section is passed to `append_section_bounded` along with the cumulative `text` and `max_length`. The same pattern is then applied to two text sections for source snippets and related page summaries, using `TextSection` with `std::string` items and corresponding render functions. The algorithm relies on `append_section_bounded` (from an internal namespace) to progressively fill the buffer, and uses `append_if_fits` to guard the header. All rendering delegates to function pointers (`render_detailed_fact`, `render_context_fact`, `render_source_snippet`, `render_summary_item`), which consume the `EvidencePack`’s target facts, context vectors, source snippets, and summary items.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- const `EvidencePack` &pack
- pack`.target_facts`
- pack`.local_context`
- pack`.dependency_context`
- pack`.reverse_usage_context`
- pack`.source_snippets`
- pack`.related_page_summaries`
- int `max_length`

#### Writes To

- local `std::string` text returned by value

#### Usage Patterns

- used to format evidence text with a bounded length for inclusion in prompts or documents
- called by higher-level generation functions to prepare evidence content
- likely used when truncation of evidence is necessary to fit size constraints

## Internal Structure

The module `generate:evidence` decomposes into a public collection of evidence‑building functions (e.g., `build_evidence_for_namespace_summary`, `build_evidence_for_function_analysis`, `format_evidence_text`) and an internal implementation layer nested in the `__detail` namespace. The public functions accept symbol identifiers, analysis stores, and context parameters to produce `EvidencePack` structs that aggregate facts, source snippets, and related page summaries. Internal helpers such as `collect_facts`, `collect_summaries`, and `collect_namespace_facts_cached` handle the gathering and caching of `SymbolFact` objects, with a dedicated `NamespaceFactsCacheEntry` avoiding redundant traversal when multiple symbols belong to the same namespace. The module imports from `std`, `extract`, and `generate:model`, relying on the `extract` module for symbol extraction results and on `generate:model` for analysis records (e.g., `FunctionAnalysis`, `TypeAnalysis`) and prompt‑kind enumeration. The implementation structure thus separates high‑level evidence composition from low‑level fact collection, using template‑based merging (`collect_merged_facts`) and length‑bounded formatting (`format_evidence_text_bounded`) to support flexible rendering within the generation pipeline.

## Related Pages

- [Module extract](../extract/index.md)
- [Module generate:model](model.md)

