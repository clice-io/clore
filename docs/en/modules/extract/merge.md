---
title: 'Module extract:merge'
description: 'The extract:merge module is responsible for consolidating extracted symbol information into the project model, handling deduplication, namespace hierarchy resolution, and index reconstruction. It provides the public entry points merge_symbol_info (with both const‑reference and rvalue‑reference overloads) to merge symbol data from an incoming source into a target, along with rebuild_model_indexes and rebuild_lookup_maps to regenerate lookup structures after model modifications. Additionally, it exposes utility templates append_unique and deduplicate for efficient collection management, and an internal LocalIndexData structure to support parallel processing of merge operations.'
layout: doc
template: doc
---

# Module `extract:merge`

## Summary

The `extract:merge` module is responsible for consolidating extracted symbol information into the project model, handling deduplication, namespace hierarchy resolution, and index reconstruction. It provides the public entry points `merge_symbol_info` (with both const‑reference and rvalue‑reference overloads) to merge symbol data from an incoming source into a target, along with `rebuild_model_indexes` and `rebuild_lookup_maps` to regenerate lookup structures after model modifications. Additionally, it exposes utility templates `append_unique` and `deduplicate` for efficient collection management, and an internal `LocalIndexData` structure to support parallel processing of merge operations.

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

Based on its name and signature, `append_unique` serves as a utility to add an element to a container while avoiding duplicates. The exact container type and comparison logic depend on template instantiation, but the purpose is to ensure uniqueness in the target collection.

#### Mutation

No mutation is evident from the extracted code.

### `clore::extract::append_unique_range`

Declaration: `extract/merge.cppm:19`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The variable's role and how it participates in surrounding logic are not described in the provided evidence. No reads or mutations are observable from the snippets.

#### Mutation

No mutation is evident from the extracted code.

### `clore::extract::deduplicate`

Declaration: `extract/merge.cppm:49`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

It appears to be a function template, but the evidence does not provide enough context to determine its role.

#### Mutation

No mutation is evident from the extracted code.

## Functions

### `clore::extract::merge_symbol_info`

Declaration: `extract/merge.cppm:55`

Definition: `extract/merge.cppm:215`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `merge_symbol_info` is a thin public entry point that immediately delegates to the internal helper `merge_symbol_info_impl`. This helper drives the entire merging algorithm, which processes the incoming symbol data into the existing model. Control flow begins by dividing work across hardware threads via `run_parallel_chunks`; each thread operates on its own per-thread data structure (`per_thread`) to store merged results. Within each chunk, the algorithm uses `ensure_namespace_hierarchy` and `find_enclosing_namespace` to maintain the namespace tree, and calls `append_unique` or `append_unique_range` to insert symbols while avoiding duplicates. Configuration flags such as `prefer_incoming_snippet` and `prefer_incoming_definition` guide attribute conflict resolution. After all chunks complete, the per-thread results are consolidated and `deduplicate` is applied on the final output. Finally, the function `rebuild_model_indexes` and `rebuild_lookup_maps` are invoked to reconstruct the model’s internal index structures, ensuring consistency for subsequent queries.

#### Side Effects

- Modifies `current` `SymbolInfo` instance

#### Reads From

- `current` parameter (in/out)
- `incoming` parameter (read-only)

#### Writes To

- `current` parameter (mutated)

#### Usage Patterns

- Called by code that needs to combine symbol data from multiple sources
- Used during symbol extraction or update phases

### `clore::extract::merge_symbol_info`

Declaration: `extract/merge.cppm:54`

Definition: `extract/merge.cppm:211`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The public overload `clore::extract::merge_symbol_info(SymbolInfo& current, SymbolInfo&& incoming)` is a thin wrapper that moves `incoming` into the internal helper `merge_symbol_info_impl`. The implementation of `merge_symbol_info_impl` begins by inspecting `incoming` to determine the number of hardware threads (`hardware_threads`) and reserving per‑thread work partitions via `run_parallel_chunks`. For each partition, a worker block reads the symbol range, identifies the `incoming` model and its `config` (which includes flags like `prefer_incoming_snippet` and `prefer_incoming_definition`), and then runs the core merge for that chunk.

Inside each worker, the algorithm calls `ensure_namespace_hierarchy` to guarantee that the namespace `ns` from `incoming` exists in `model`’s namespace tree (creating missing ancestors), then locates the enclosing namespace via `find_enclosing_namespace` using the `owner_path`. For each symbol in the chunk, `sym` is compared with existing symbols in `model` at the same `parent_id`; if no match is found, `append_unique` adds it together with its `explicit_namespaces` and any derived `current_name`. Overlapping symbol ranges are handled by `append_unique_range` and later `deduplicate` cleans up duplication. After all chunks complete, `rebuild_model_indexes` re‑establishes internal indexes (e.g., `known_namespace_names`) and `rebuild_lookup_maps` updates the global lookup structures, ensuring the merged `model` is fully consistent.

#### Side Effects

- modifies the `current` `SymbolInfo` by merging data from `incoming`
- moves resources from `incoming`, leaving it in a valid but unspecified state

#### Reads From

- `current` (existing `SymbolInfo` data)
- `incoming` (data to be merged)

#### Writes To

- `current` (modified `SymbolInfo`)
- `incoming` (moved-from state)

#### Usage Patterns

- called during symbol extraction to combine partial symbol information
- used to merge newly discovered symbol attributes into an existing record

### `clore::extract::rebuild_lookup_maps`

Declaration: `extract/merge.cppm:59`

Definition: `extract/merge.cppm:428`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::rebuild_lookup_maps` regenerates two internal lookup structures of a `ProjectModel` instance. It first clears `model.symbol_ids_by_qualified_name` and `model.module_name_to_sources`, then iterates over all entries in `model.symbols`. For each symbol that has a non‑empty `qualified_name`, the function appends its `symbol_id` to the corresponding vector in the lookup map. After populating, each vector is sorted using a custom comparator that orders by `signature`, then `declaration_location.file`, then `declaration_location.line`, and finally by `symbol_id` as a tiebreaker. The sorted vectors are deduplicated via `std::unique`; if a qualified name maps to more than one symbol ID, an informational log message records the count of overload candidates.

The module side follows a similar pattern. The function clears `model.module_name_to_sources` and rebuilds it by iterating over `model.modules`; for each module unit with a non‑empty `name`, it adds the source file to the corresponding vector. Each source list is then sorted and deduplicated. Finally, the function counts how many interface units exist per module name. If more than one interface is found, a warning is logged listing the first two conflicting source files. No external dependencies beyond the model’s maps and the logging utilities are required.

#### Side Effects

- Clears `model.symbol_ids_by_qualified_name` and `model.module_name_to_sources`
- Populates `model.symbol_ids_by_qualified_name` with sorted, deduplicated symbol `IDs`
- Populates `model.module_name_to_sources` with sorted, deduplicated source file paths
- Logs info messages about overload candidate counts
- Logs warning messages about duplicate module interfaces

#### Reads From

- `model.symbols`
- `model.modules`
- `SymbolInfo::qualified_name`
- `SymbolInfo::signature`
- `SymbolInfo::declaration_location.file`
- `SymbolInfo::declaration_location.line`
- `ModuleUnit::name`
- `ModuleUnit::is_interface`
- `SymbolID`

#### Writes To

- `model.symbol_ids_by_qualified_name`
- `model.module_name_to_sources`
- log output

#### Usage Patterns

- Called after merging symbol info into a `ProjectModel`
- Invoked to refresh lookup caches when symbol or module data changes

### `clore::extract::rebuild_model_indexes`

Declaration: `extract/merge.cppm:57`

Definition: `extract/merge.cppm:219`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first resets the existing model indexes by clearing per-file symbols, deduplicating includes, clearing all namespaces, and cleaning per-symbol relation lists such as `calls`, `references`, `children`, `derived`, `called_by`, and `referenced_by`. It then captures all symbol `IDs` and builds a set of known namespace qualified names for strict validation. The core indexing is performed in parallel via `run_parallel_chunks`: each thread processes a range of symbol `IDs`, checking each symbol’s declaration location against the filter via `matches_filter`, inferring its enclosing namespace through `find_enclosing_namespace`, and recording explicit namespace names and parent–child pairs. Results are accumulated into thread‑local `LocalIndexData` structures and later merged into global maps (`file_symbols`, `namespace_symbols`, `parent_children`, `explicit_namespaces`).

After merging, the function populates `model.files` entries with the owner‑path mappings, ensures the namespace hierarchy using `ensure_namespace_hierarchy` for both explicit and inferred namespace names, and injects symbol `IDs` into namespace info objects. Parent–child relationships are applied by appending child `IDs` to the parent symbol’s `children` list. Finally, three parallel passes deduplicate lists within file info objects, namespace info objects (both `symbols` and `children`), and symbol info objects (`children` and `derived`), ensuring index consistency after the rebuild.

#### Side Effects

- clears `model.files[].symbols` and `model.files[].includes`
- clears `model.namespaces`
- clears `model.symbols[].children`
- deduplicates `model.symbols[].calls`, `model.symbols[].references`, `model.symbols[].derived`, `model.symbols[].called_by`, `model.symbols[].referenced_by`
- rebuilds `model.files` with updated path and symbol lists
- rebuilds `model.namespaces` with namespace info and symbol lists
- adds parent-child relationships to `model.symbols`

#### Reads From

- reads `config.filter` and `config.root_path` via `filter_root_path(config)`
- reads `model.symbols` keys and values (including `declaration_location.file`, `kind`, `qualified_name`, `parent`)
- reads `model.files` keys
- reads `model.namespaces` for known namespace names

#### Writes To

- writes to `model.files[].symbols` and `model.files[].path`
- writes to `model.namespaces` entries and `model.namespaces[].symbols`, `model.namespaces[].name`, `model.namespaces[].children`
- writes to `model.symbols[].children`

#### Usage Patterns

- called after extraction or merging to refresh indexes
- part of the model finalization pipeline
- ensures consistency of file, namespace, and parent-child associations

## Internal Structure

The `extract:merge` module is the component responsible for combining extracted symbol data into a consistent project model. It imports the `extract:model` for core data structures, `extract:filter` for path resolution, the `config` module for settings, and the `support` module for utility types. The module is decomposed into a set of internal helper functions and public entry points. The helper layer includes template utilities for deduplication (`append_unique`, `append_unique_range`, `deduplicate`) and namespace hierarchy management (`ensure_namespace_hierarchy`, `find_enclosing_namespace`). A private `LocalIndexData` struct supports per-thread state during parallel index rebuilding. On top of this, the public API provides `merge_symbol_info` (with overloads for const and rvalue references) to merge individual symbol records, `rebuild_model_indexes` to reconstruct internal indices after modifications, and `rebuild_lookup_maps` to repopulate name‑ and location‑based lookup tables. The implementation structure separates these concerns: low‑level merging of symbol fields, namespace resolution, and index maintenance are each handled by dedicated functions, promoting clarity and reuse.

## Related Pages

- [Module config](../config/index.md)
- [Module extract:filter](filter.md)
- [Module extract:model](model.md)
- [Module support](../support/index.md)

