---
title: 'Module extract:merge'
description: 'The extract:merge module is responsible for combining symbol information gathered from multiple extraction passes or sources into a single, coherent model. It provides the core merging logic through functions such as merge_symbol_info, which updates a target symbol record with data from a source, and utility templates like append_unique and deduplicate to handle duplicate entries during assembly. The module also owns the post‑merge index rebuilding phase, exposed via rebuild_lookup_maps and rebuild_model_indexes, ensuring that name‑based and identifier‑based lookups reflect the fully merged state. By leveraging the extract:model data structures and relying on filtering and support utilities, the module maintains consistency across the extraction pipeline while allowing parallel or incremental updates to the model.'
layout: doc
template: doc
---

# Module `extract:merge`

## Summary

The `extract:merge` module is responsible for combining symbol information gathered from multiple extraction passes or sources into a single, coherent model. It provides the core merging logic through functions such as `merge_symbol_info`, which updates a target symbol record with data from a source, and utility templates like `append_unique` and `deduplicate` to handle duplicate entries during assembly. The module also owns the post‑merge index rebuilding phase, exposed via `rebuild_lookup_maps` and `rebuild_model_indexes`, ensuring that name‑based and identifier‑based lookups reflect the fully merged state. By leveraging the `extract:model` data structures and relying on filtering and support utilities, the module maintains consistency across the extraction pipeline while allowing parallel or incremental updates to the model.

## Imports

- [`config`](../config/index.md)
- [`extract:filter`](filter.md)
- [`extract:model`](model.md)
- `std`
- [`support`](../support/index.md)

## Variables

### `clore::extract::append_unique`

Declaration: `extract/merge.cppm:12`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The evidence does not provide any usage or mutation of `append_unique`. While a snippet shows `void append_unique`, suggesting a function template, the target is classified as a variable. Its role in the surrounding logic cannot be determined from the given context.

#### Mutation

No mutation is evident from the extracted code.

### `clore::extract::append_unique_range`

Declaration: `extract/merge.cppm:19`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

No evidence describes how `clore::extract::append_unique_range` is read, assigned, or participates in surrounding logic. Its initialization and any mutations are unknown from the provided context.

#### Mutation

No mutation is evident from the extracted code.

### `clore::extract::deduplicate`

Declaration: `extract/merge.cppm:49`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

No additional details can be inferred from the evidence. The variable does not appear in the local context list or any usage patterns beyond its declaration.

#### Mutation

No mutation is evident from the extracted code.

## Functions

### `clore::extract::merge_symbol_info`

Declaration: `extract/merge.cppm:55`

Definition: `extract/merge.cppm:215`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::merge_symbol_info` delegates directly to the internal template `merge_symbol_info_impl`, which accepts the current model and an incoming `SymbolInfo`. The merge algorithm iterates over the incoming symbol's scopes and names, using utility functions such as `append_unique` and `append_unique_range` to add missing entries and `deduplicate` to remove duplicates after assembly. Conflicts between the current model and the incoming data are resolved according to configuration flags `prefer_incoming_snippet` and `prefer_incoming_definition`. The namespace hierarchy is rebuilt with `ensure_namespace_hierarchy` and `find_enclosing_namespace` to maintain parent-child relationships. Finally, indexes are updated via `rebuild_model_indexes` and `rebuild_lookup_maps` to reflect the merged state.

#### Side Effects

- modifies the `current` `SymbolInfo` object through the call to `merge_symbol_info_impl`

#### Reads From

- `current` (parameter, read prior to modification)
- `incoming` (parameter, const reference)

#### Writes To

- `current` (parameter, modified via `merge_symbol_info_impl`)

#### Usage Patterns

- called during symbol extraction to merge symbol information from multiple sources
- used when multiple compile entries contribute data to the same symbol

### `clore::extract::merge_symbol_info`

Declaration: `extract/merge.cppm:54`

Definition: `extract/merge.cppm:211`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::merge_symbol_info` is a public entry point that delegates to the anonymous-namespace helper `merge_symbol_info_impl`. For the rvalue-reference overload it moves the incoming `SymbolInfo` into the implementation, while the const-lvalue overload creates a copy before moving. The core merging algorithm processes symbol entries in parallel using `run_parallel_chunks`, distributing work across `num_threads` hardware threads. Each thread operates on a `per_thread` local state holding `LocalIndexData` and a local copy of the model segment. The merging logic handles namespace hierarchy reconciliation through `ensure_namespace_hierarchy` and `find_enclosing_namespace`, ensuring symbols are placed under the correct owning namespace. Duplicate symbols are resolved using `append_unique` and `append_unique_range`, with preference controlled by `prefer_incoming_snippet` and `prefer_incoming_definition` from the `config` object.

After parallel chunk processing completes, the results are combined and the model is finalized. The function calls `rebuild_model_indexes` to reconstruct internal indexing structures and `rebuild_lookup_maps` to update symbol lookup tables. It also invokes `deduplicate` to remove any remaining duplicates introduced during merging. The entire process depends on `run_parallel_chunks` for thread synchronization and work distribution, and uses `ensure_namespace_hierarchy` to create missing parent namespace entries while `find_enclosing_namespace` determines the appropriate container for each symbol.

#### Side Effects

- Modifies `current` by merging data from `incoming`
- Invalidates `incoming` as its resources are moved

#### Reads From

- `current`
- `incoming`

#### Writes To

- `current`
- `incoming` (moved-from state)

### `clore::extract::rebuild_lookup_maps`

Declaration: `extract/merge.cppm:59`

Definition: `extract/merge.cppm:428`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::rebuild_lookup_maps` first clears two lookup maps in the `ProjectModel`: `model.symbol_ids_by_qualified_name` and `model.module_name_to_sources`. It then repopulates the symbol lookup by iterating over every entry in `model.symbols`. For each symbol whose `sym.qualified_name` is non‑empty, it appends the `SymbolID` to the list keyed by that qualified name. After populating, it processes each `qualified_name` entry: it sorts the vector of `symbol_ids` using a comparator that orders by `lhs_sym.signature`, then by `lhs_sym.declaration_location.file` and `lhs_sym.declaration_location.line`, and finally by the `SymbolID` itself. Duplicates are removed via `std::unique` followed by `symbol_ids.erase`, and if more than one candidate remains, `logging::info` reports the qualified name and overload count.

The second part rebuilds the module source lookup. It iterates over every file‑to‑module mapping in `model.modules`; for any `mod_unit` with a non‑empty `mod_unit.name`, the source file is appended to the list for that module name in `model.module_name_to_sources`. Each list of `sources` is then sorted and deduplicated with `std::unique`. The function then counts how many of those sources correspond to interface units (`mod_unit.is_interface`). If more than one interface source exists, it logs a warning via `logging::warn` identifying the duplicate interfaces by their file paths.

#### Side Effects

- Clears and repopulates `model.symbol_ids_by_qualified_name`
- Clears and repopulates `model.module_name_to_sources`
- Logs info message for each qualified name with multiple symbols
- Logs warning message for each module name with duplicate interfaces

#### Reads From

- `model.symbols`
- `model.modules`

#### Writes To

- `model.symbol_ids_by_qualified_name`
- `model.module_name_to_sources`

#### Usage Patterns

- Called after initial population of the `ProjectModel` to synchronize lookup indices
- May be invoked again after modifying symbols or modules to refresh cached maps

### `clore::extract::rebuild_model_indexes`

Declaration: `extract/merge.cppm:57`

Definition: `extract/merge.cppm:219`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first clears all per-file symbol lists, resets the namespace map, and deduplicates relation vectors (calls, references, etc.) on every symbol. It then collects all symbol `IDs` into a flat vector and builds a set of known namespace qualified names for strict validation during namespace inference. A parallel-chunking lambda (`run_parallel_chunks`) dispatches work across hardware threads: each thread builds a `LocalIndexData` structure containing file‑to‑symbol mappings (filtered via `matches_filter` and `filter_root_path`), namespace‑to‑symbol mappings (obtained by calling `find_enclosing_namespace` on non‑namespace symbols), and parent–child edges. After all threads join, the local vectors are merged into global `std::unordered_map` collections (`file_symbols`, `namespace_symbols`) and a single `parent_children` vector. The explicit namespace names from the local data are also aggregated.

The merged `file_symbols` are written into `model.files`, creating or updating `FileInfo` entries. For namespaces, `ensure_namespace_hierarchy` is first called for each explicit namespace name, then for each implicit namespace name from the aggregated map, ensuring the full hierarchy exists in `model.namespaces` before appending symbol `IDs`. The parent–child edges are applied by pushing child `IDs` into the `children` field of the parent symbol. Finally, three parallel‑chunk passes deduplicate the `symbols` field of every `FileInfo`, the `symbols` and `children` fields of every `NamespaceInfo`, and the `children` and `derived` fields of every `SymbolInfo`.

#### Side Effects

- clears `file_info.symbols` for every file in `model.files`
- deduplicates `file_info.includes` for every file
- clears `model.namespaces` map
- deduplicates `sym.calls`, `sym.references`, `sym.derived`, `sym.called_by`, `sym.referenced_by` for every symbol
- clears `sym.children` for every symbol
- repopulates `model.files` entries with new `.path` and `.symbols`
- inserts or updates namespace entries in `model.namespaces` with `.name` and `.symbols`
- appends child symbol `IDs` to parent symbol's `.children` vector
- final deduplication of file symbols, namespace symbols, and symbol `.children`/`.derived`

#### Reads From

- `config` parameter
- `model` parameter (all fields: `model.files`, `model.symbols`, `model.namespaces`)
- per‑symbol `sym.declaration_location.file`, `sym.kind`, `sym.qualified_name`, `sym.parent`
- `filter_root_path(config)` result
- `known_namespace_names` set built from symbol qualified names

#### Writes To

- `model.files` (file entries' `.path` and `.symbols`)
- `model.namespaces` (namespace entries' `.name` and `.symbols`)
- `model.symbols` (symbols' `.children`, `.derived`, `.calls`, `.references`, `.called_by`, `.referenced_by`)
- local `file_symbols`, `namespace_symbols`, `parent_children`, `explicit_namespaces` (merged into model)

#### Usage Patterns

- called after merging symbol information to rebuild efficient lookup structures
- used to prepare a `ProjectModel` for queries or display
- invokes heavy parallelization to improve performance on large models

## Internal Structure

The `extract:merge` module is responsible for combining symbol information from multiple extraction passes into a single coherent project model. It exposes three core public functions: `merge_symbol_info` (two overloads) which merges a source symbol info into a target, `rebuild_lookup_maps` which refreshes name‑ and identifier‑based lookups, and `rebuild_model_indexes` which reconstructs internal index structures for a given model. Internally, the module decomposes work into a set of anonymous‑namespace helpers—`ensure_namespace_hierarchy` and `find_enclosing_namespace`—that resolve namespace nesting, and template utilities `append_unique`, `append_unique_range`, and `deduplicate` for efficient data deduplication. The implementation uses a parallel chunk‑processing strategy driven by `hardware_threads` and per‑thread `LocalIndexData` to partition merging tasks, with explicit control over which source (incoming or current) is preferred for snippets and definitions. The module imports `extract:model` for its data types, `extract:filter` for path resolution, `support` for logging and UTF‑8 utilities, and `config` for project‑level settings, forming a clear layering where merge logic sits atop the model and filtering infrastructure.

## Related Pages

- [Module config](../config/index.md)
- [Module extract:filter](filter.md)
- [Module extract:model](model.md)
- [Module support](../support/index.md)

