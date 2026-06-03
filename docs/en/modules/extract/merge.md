---
title: 'Module extract:merge'
description: 'The extract:merge module is responsible for combining symbol information extracted from multiple translation units into a single, coherent project model. Its public interface provides functions such as deduplicate, append_unique, append_unique_range, merge_symbol_info, rebuild_lookup_maps, and rebuild_model_indexes. These operations ensure efficient and correct merging of symbol metadata, including deduplication and index reconstruction, while supporting parallelism via hardware thread count and per‑thread local data structures like LocalIndexData. The module depends on the configuration, filtering, model, and support modules, and its implementation owns the merging logic that updates the shared projected model with incoming symbol data, namespace hierarchies, and file‑symbol associations.'
layout: doc
template: doc
---

# Module `extract:merge`

## Summary

The `extract:merge` module is responsible for combining symbol information extracted from multiple translation units into a single, coherent project model. Its public interface provides functions such as `deduplicate`, `append_unique`, `append_unique_range`, `merge_symbol_info`, `rebuild_lookup_maps`, and `rebuild_model_indexes`. These operations ensure efficient and correct merging of symbol metadata, including deduplication and index reconstruction, while supporting parallelism via hardware thread count and per‑thread local data structures like `LocalIndexData`. The module depends on the configuration, filtering, model, and support modules, and its implementation owns the merging logic that updates the shared projected model with incoming symbol data, namespace hierarchies, and file‑symbol associations.

## Imports

- [`config`](../config/index.md)
- [`extract:filter`](filter.md)
- [`extract:model`](model.md)
- [`support`](../support/index.md)

## Functions

### `clore::extract::append_unique`

Declaration: `src/extract/merge.cppm:26`

Definition: `src/extract/merge.cppm:26`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::append_unique` implements a simple linear deduplication algorithm. It takes a reference to a `std::vector<T>` called `values` and a const reference to a `T` value. Internally, it calls `std::find` on the range `[values.begin(), values.end())` to locate the element. If the iterator returned by `std::find` is equal to `values.end()` — that is, the value is not already present in the container — it appends the value via `values.push_back(value)`. This ensures that the vector maintains the order of first insertion while avoiding duplicate entries.

The function has no dependencies on other parts of the `clore::extract` library; it relies solely on the C++ standard library for the linear search and push operation. Because the search is linear in the current size of `values`, repeated calls on a growing collection have O(n²) worst‑case complexity. The implementation is used as a building block in higher‑level merge routines such as `clore::extract::merge_symbol_info_impl`, typically for accumulating unique symbol or namespace identifiers without duplicates.

#### Side Effects

- Modifies the vector by appending the value if it is not already present.

#### Reads From

- `values` vector (to check existing elements)
- `value` (the element to potentially insert)

#### Writes To

- `values` vector (via `push_back`)

#### Usage Patterns

- Insert unique elements into a container
- Avoid duplicates in merge or accumulation operations

### `clore::extract::append_unique_range`

Declaration: `src/extract/merge.cppm:33`

Definition: `src/extract/merge.cppm:33`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::append_unique_range` appends elements from the `incoming` vector to the `values` vector while preserving uniqueness. The algorithm selects one of two strategies based on the relative sizes of the two vectors to balance time and space. When `incoming` is smaller, it constructs an `std::unordered_set` from `incoming` and erases any values already present in `values`, then iterates over `incoming` again to push only those elements that remain in the set (and removes them to avoid duplicates within `incoming`). This minimizes the space overhead of the set relative to the potentially large `values` container. When `incoming` is larger or equal, the function builds an `std::unordered_set` from `values` and then inserts each element of `incoming`; if the insertion is successful (i.e., the element was not already in the set), it is appended to `values`. An early return occurs if `incoming` is empty. The implementation depends on `std::unordered_set` for O(1) average‑case lookups and on the `std::vector` interface for mutation.

#### Side Effects

- Mutates the `values` vector by appending new unique elements.

#### Reads From

- `values` parameter (elements read for duplicate checking)
- `incoming` parameter (elements read for potential appending)

#### Writes To

- `values` parameter (elements appended)

#### Usage Patterns

- Used to merge collections while avoiding duplicates
- Called in repeated merge operations where `values` grows over time

### `clore::extract::deduplicate`

Declaration: `src/extract/merge.cppm:63`

Definition: `src/extract/merge.cppm:63`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::deduplicate` sorts the elements of the input `std::vector<T>` via `std::sort` and then uses `std::unique` together with `std::vector::erase` to remove consecutive duplicates, producing a sorted, duplicate-free vector in‑place. The algorithm relies solely on the standard library components `std::sort`, `std::unique`, and `std::vector::erase`; no custom comparators or conditional logic are employed. The internal control flow is linear: sorting first ensures that duplicates are adjacent, then `std::unique` shifts non‑duplicates to the front and returns an iterator to the new logical end, and finally `erase` deletes the trailing duplicate elements.

#### Side Effects

- Modifies the vector in place: sorts elements and removes duplicate entries.

#### Reads From

- The vector elements are read for comparison during sort and unique.

#### Writes To

- The same vector is written: reordered and resized.

#### Usage Patterns

- Called to remove duplicates from a sorted or unsorted vector before further processing.

### `clore::extract::merge_symbol_info`

Declaration: `src/extract/merge.cppm:69`

Definition: `src/extract/merge.cppm:229`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation delegates directly to the internal helper `merge_symbol_info_impl`, which performs the actual merging. Inside that helper, the function first extracts symbol data from the `incoming` source and applies `deduplicate` to the current `model` to remove any duplicate entries. It then iterates over all incoming symbols, using `append_unique` to insert each new symbol while checking against a local `seen` set. For each symbol, the function determines the enclosing namespace via `find_enclosing_namespace`, ensures the namespace hierarchy exists by calling `ensure_namespace_hierarchy`, and updates the relevant `parent_children`, `namespace_symbols`, and `explicit_namespaces` structures in a thread-local `LocalIndexData` object. When `hardware_threads` and the symbol count allow, the work is partitioned into chunks processed in parallel by `run_parallel_chunks`; otherwise, a single thread processes the entire list. After all symbols are merged, `rebuild_model_indexes` and `rebuild_lookup_maps` are called to reconstruct the index structures, including file‑symbol associations and namespace lookups. Key dependencies include the `deduplicate`, `append_unique`, `append_unique_range`, `ensure_namespace_hierarchy`, and `find_enclosing_namespace` utilities, along with the `LocalIndexData` aggregate that holds per‑thread symbol tables.

#### Side Effects

- Modifies the `current` `SymbolInfo` object by merging data from `incoming`.

#### Reads From

- `incoming` parameter (const `SymbolInfo`&)

#### Writes To

- `current` parameter (`SymbolInfo`&)

#### Usage Patterns

- Called to merge symbol information from one `SymbolInfo` into another.

### `clore::extract::merge_symbol_info`

Declaration: `src/extract/merge.cppm:68`

Definition: `src/extract/merge.cppm:225`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The target overload of `clore::extract::merge_symbol_info` forwards to `clore::extract::(anonymous namespace)::merge_symbol_info_impl`, which performs the actual merge by processing `incoming` entries while updating the `model`. Under the hood, the implementation parallelizes the work using a chunking strategy: it computes `num_threads` from `hardware_threads` and the configured concurrency level, then partitions the incoming symbols into `parts`. Each thread processes a chunk and stores results in a `LocalIndexData` structure that holds per-thread collections such as `file_symbols`, `namespace_symbols`, `parent_children`, and `explicit_namespaces`. Within each chunk, symbols are inserted into `current` using `append_unique` (and `append_unique_range` for contiguous ranges), deduplicated via `deduplicate`, and missing namespace ancestors are created by `ensure_namespace_hierarchy`. After all threads finish, a post‑merge phase calls `rebuild_model_indexes` and `rebuild_lookup_maps` to update the global lookup structures, finalizing the merge.

#### Side Effects

- Modifies `current` by merging data from `incoming`
- Moves from `incoming`, leaving it in a valid but unspecified state

#### Reads From

- `current` (existing data)
- `incoming` (data to merge)

#### Writes To

- `current` (updated symbol info)

#### Usage Patterns

- Called when combining symbol info from incremental extraction passes
- Used in batch processing pipelines to accumulate symbol metadata

### `clore::extract::rebuild_lookup_maps`

Declaration: `src/extract/merge.cppm:73`

Definition: `src/extract/merge.cppm:442`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::rebuild_lookup_maps` first clears the two lookup maps (`symbol_ids_by_qualified_name` and `module_name_to_sources`) in the `model`. It then repopulates `symbol_ids_by_qualified_name` by iterating over all entries in `model.symbols` and pushing each `symbol_id` into the vector keyed by the symbol's `qualified_name`. For each such vector, it sorts by `signature`, then `declaration_location.file`, then `declaration_location.line`, and finally by the `symbol_id` itself as a tie‑breaker; after sorting, it applies `std::unique` to remove duplicates. If a vector retains more than one element, a `logging::info` message is emitted indicating the qualified name and candidate count. The same pattern is applied to `module_name_to_sources`: it iterates over `model.modules`, appending each source file to the vector for its module name, then sorts and deduplicates. It additionally counts how many sources are marked as `is_interface`; if more than one interface source exists for the same module name, a warning is logged with the two file paths.

#### Side Effects

- Modifies `model.symbol_ids_by_qualified_name` by clearing and repopulating it
- Modifies `model.module_name_to_sources` by clearing and repopulating it
- Sorts vectors within `model.symbol_ids_by_qualified_name` and `model.module_name_to_sources`
- Erases duplicate entries from those vectors
- Logs info messages about overload candidates
- Logs warning messages about duplicate module interfaces

#### Reads From

- `model.symbols`
- `model.modules`
- `model.modules[source].name`
- `model.modules[source].is_interface`
- `sym.qualified_name`
- `sym.signature`
- `sym.declaration_location.file`
- `sym.declaration_location.line`
- `symbol_id`

#### Writes To

- `model.symbol_ids_by_qualified_name`
- `model.module_name_to_sources`

#### Usage Patterns

- Called after merging symbol information to update lookup indices
- Used to maintain consistency of reverse maps in `ProjectModel`
- Prepares the model for fast symbol and module queries

### `clore::extract::rebuild_model_indexes`

Declaration: `src/extract/merge.cppm:71`

Definition: `src/extract/merge.cppm:233`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function begins by clearing existing index structures: it clears each file’s symbol list, deduplicates includes, clears the namespace map, and resets per‑symbol relationship containers (`calls`, `references`, `children`, `derived`, `called_by`, `referenced_by`). It then collects all symbol identifiers and builds a set of known namespace qualified names for later inference. The core reconstruction is split into two parallel phases using a `run_parallel_chunks` lambda that partitions work over hardware threads. The first parallel pass iterates over every symbol: for each symbol it computes a normalized file path, optionally stores a file‑symbol association (if the path passes a filter), records explicit namespace symbols, infers an enclosing namespace for non‑namespace symbols via `find_enclosing_namespace`, and captures parent‑child relationships. Per‑thread `LocalIndexData` buffers are merged into global maps (`file_symbols`, `namespace_symbols`, `parent_children`, `explicit_namespaces`). After merging, the function sequentially updates the `ProjectModel`: it populates `model.files` with file‑symbol mappings, calls `ensure_namespace_hierarchy` for each explicit or inferred namespace to create canonical entries, fills `model.namespaces` with symbol lists, and appends child identifiers to each parent’s `children` list. The second parallel phase deduplicates the just‑built index arrays: file symbol lists, namespace symbol lists and children, and per‑symbol `children` and `derived` vectors, each processed in parallel over the respective containers. The complete flow relies on internal helpers (`deduplicate`, `filter_root_path`, `matches_filter`, `find_enclosing_namespace`, `ensure_namespace_hierarchy`) and uses the task configuration for filtering and path resolution.

#### Side Effects

- Clears `model.files[].symbols` and deduplicates `model.files[].includes`
- Clears `model.namespaces` map
- Deduplicates `sym.calls`, `sym.references`, `sym.children`, `sym.derived`, `sym.called_by`, `sym.referenced_by` for each symbol
- Populates `model.files[].symbols` with newly computed symbol lists
- Creates or ensures entries in `model.namespaces` via `ensure_namespace_hierarchy`
- Populates `model.namespaces[].symbols` and `model.namespaces[].name`
- Adds child `IDs` to `parent_it->second.children`
- Deduplicates `model.files[].symbols`, `model.namespaces[].symbols`, `model.namespaces[].children`, `model.symbols[].children`, and `model.symbols[].derived`
- Creates and joins multiple threads for parallel processing

#### Reads From

- `config.filter`
- `filter_root_path(config)` return value
- `model.files`
- `model.symbols`
- `model.namespaces`
- `symbol.declaration_location.file`
- `sym.qualified_name`
- `sym.kind`
- `sym.parent`
- `sym.calls`
- `sym.references`
- `sym.children`
- `sym.derived`
- `sym.called_by`
- `sym.referenced_by`
- `known_namespace_names` set

#### Writes To

- `model.files[].symbols`
- `model.files[].path`
- `model.files[].includes` (deduplication)
- `model.namespaces` (clear and insert)
- `model.namespaces[].name`
- `model.namespaces[].symbols`
- `model.namespaces[].children`
- `model.symbols[].children`
- `model.symbols[].derived`
- `model.symbols[].calls` (deduplication)
- `model.symbols[].references` (deduplication)
- `model.symbols[].called_by` (deduplication)
- `model.symbols[].referenced_by` (deduplication)

#### Usage Patterns

- Called after symbol extraction to rebuild lookup indexes
- Used during project model construction or update
- Invoked to ensure file, namespace, and parent–child associations are current

## Internal Structure

The `extract:merge` module reconciles multiple extractions into a single project model. It imports `extract:model` for the core data structures (`SymbolInfo`, `FileInfo`, etc.), `extract:filter` for path-based operations, `config` for threading settings, and `support` for string and caching utilities. The module decomposes into a public API (`append_unique`, `append_unique_range`, `deduplicate`, `merge_symbol_info` overloads, `rebuild_lookup_maps`, `rebuild_model_indexes`) and a set of internal helpers (`merge_symbol_info_impl`, `ensure_namespace_hierarchy`, `find_enclosing_namespace`).

Internally, the module layers merge logic atop a parallel chunking strategy: it splits symbol data across `hardware_threads` workers, each processing a partition of the incoming symbols. The `LocalIndexData` struct (with fields `file_symbols`, `namespace_symbols`, `parent_children`, `explicit_namespaces`) serves as per‑thread temporary storage during index rebuilding, later aggregated into the final model. The implementation structure thus separates deduplication and merging (which operate on individual symbols or ranges) from index reconstruction (which populates lookup maps and namespace hierarchies).

## Related Pages

- [Module config](../config/index.md)
- [Module extract:filter](filter.md)
- [Module extract:model](model.md)
- [Module support](../support/index.md)

