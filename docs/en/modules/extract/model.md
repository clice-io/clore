---
title: 'Module extract:model'
description: 'The extract:model module defines the core data structures that represent a fully extracted C++ project. It owns the public types ProjectModel, SymbolInfo, SymbolID, SymbolKind, SourceLocation, SourceRange, FileInfo, NamespaceInfo, and ModuleUnit, along with the query functions (lookup_symbol, find_symbol, find_symbols, find_module_by_name, find_module_by_source, find_modules_by_name) used to navigate the model. It also provides utility functions (split_top_level_qualified_name, join_qualified_name_parts, namespace_prefix_from_qualified_name, symbol_kind_name, resolve_source_snippet) that support both internal processing and external callers. By encapsulating all symbol metadata and project‑level indexing, this module serves as the authoritative data layer for the extraction pipeline.'
layout: doc
template: doc
---

# Module `extract:model`

## Summary

The `extract:model` module defines the core data structures that represent a fully extracted C++ project. It owns the public types `ProjectModel`, `SymbolInfo`, `SymbolID`, `SymbolKind`, `SourceLocation`, `SourceRange`, `FileInfo`, `NamespaceInfo`, and `ModuleUnit`, along with the query functions (`lookup_symbol`, `find_symbol`, `find_symbols`, `find_module_by_name`, `find_module_by_source`, `find_modules_by_name`) used to navigate the model. It also provides utility functions (`split_top_level_qualified_name`, `join_qualified_name_parts`, `namespace_prefix_from_qualified_name`, `symbol_kind_name`, `resolve_source_snippet`) that support both internal processing and external callers. By encapsulating all symbol metadata and project‑level indexing, this module serves as the authoritative data layer for the extraction pipeline.

## Imports

- [`support`](../support/index.md)

## Imported By

- [`extract:ast`](ast.md)
- [`extract:merge`](merge.md)

## Types

### `clore::extract::FileInfo`

Declaration: `src/extract/model.cppm:139`

Definition: `src/extract/model.cppm:139`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::FileInfo` is a plain aggregate that bundles the three core pieces of information extracted from a single translation unit: the `path` (a `std::string`) identifying the source file, a `symbols` vector of `SymbolID` values representing the symbols defined or declared in that file, and an `includes` vector of `std::string` listing the file paths included by that source. No invariants are enforced beyond the default constructors and member initializers, so every field is freely mutable after construction. The struct serves solely as a transport container; no member functions other than the implicitly defined ones are provided.

### `clore::extract::ModuleUnit`

Declaration: `src/extract/model.cppm:152`

Definition: `src/extract/model.cppm:152`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::ModuleUnit` is a simple aggregate holding the parsed data for a single C++20 module unit. The invariant is that `name` stores the fully qualified module name (e.g., `"foo"` or `"foo:bar"`), and `is_interface` distinguishes an exported interface (`true` for `export module`) from a partition or implementation unit (`false` for `module`). The `source_file` member holds the normalized filesystem path to the source file, and `imports` collects the names of modules imported by this unit. The `symbols` vector is populated with `SymbolID` values representing the declarations introduced in the unit; its contents are the primary output of the extraction process. All fields have straightforward defaults (`is_interface` defaults to `false`, string members are empty by default) and are populated during the parsing phase.

#### Invariants

- `is_interface` is `true` for `export module` and `false` for `module`.
- `name` is the full module name as written in the source (e.g., `"foo"` or `"foo:bar"`).
- `source_file` is a normalized path (absolute or relative) to the source file.
- `imports` and `symbols` contain the parsed imports and symbols from the module unit.

#### Key Members

- `name`
- `is_interface`
- `source_file`
- `imports`
- `symbols`

#### Usage Patterns

- Stores the parsed result of a C++20 module unit in the extraction pipeline.
- Used as a data source for building module dependency graphs and symbol tables.
- Referenced by other structures in the `clore::extract` model to represent collection of module units.

### `clore::extract::NamespaceInfo`

Declaration: `src/extract/model.cppm:145`

Definition: `src/extract/model.cppm:145`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::NamespaceInfo` serves as a plain data container for representing a C++ namespace during extraction. Its internal structure consists of three fields: `name` holds the fully qualified namespace name as a `std::string`, `symbols` stores a `std::vector<SymbolID>` of the symbol identifiers defined directly within that namespace (not in nested scopes), and `children` lists the names of immediate child namespaces as a `std::vector<std::string>`. There are no invariants enforced beyond the natural constraints of the types; the fields are populated externally by the extraction logic and are not modified after construction. The struct has no member functions other than the implicitly defined default constructor and destructor, making it a pure aggregate that simplifies serialization and iteration.

#### Invariants

- No explicit invariants are documented from the provided evidence.

#### Key Members

- `name`: the namespace name
- `symbols`: list of symbol identifiers belonging to this namespace
- `children`: names of child namespaces

#### Usage Patterns

- `NamespaceInfo` is used to model namespace hierarchy and symbol membership within the extraction framework.

### `clore::extract::ProjectModel`

Declaration: `src/extract/model.cppm:160`

Definition: `src/extract/model.cppm:160`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The internal structure of `clore::extract::ProjectModel` consists of several hash maps and a vector that together capture every extracted entity from a single project. The core containers are `symbols` (keyed by `SymbolID`), `files` (keyed by normalized file path), `namespaces` (keyed by name), and `modules` (keyed by normalized source path). Two additional maps enable efficient lookup: `symbol_ids_by_qualified_name` maps a fully qualified name to a vector of `SymbolID` values (to handle overloads), and `module_name_to_sources` maps a module name to the list of source file paths that contribute to it. The `file_order` vector preserves the order in which files were processed, and the boolean `uses_modules` is set to `true` if at least one C++20 module declaration was encountered.

Key invariants include consistent key formats across maps: file paths are normalized before insertion, and qualified names are stored exactly as they appear. The transparent string hashing (`clore::support::TransparentStringHash`) and equality (`clore::support::TransparentStringEqual`) used for `modules`, `symbol_ids_by_qualified_name`, and `module_name_to_sources` allow heterogeneous lookup without requiring `std::string` temporaries. The integrity of the model relies on the fact that every `FileInfo` in `files` corresponds to an entry in `file_order`, and every `SymbolID` referenced in `symbol_ids_by_qualified_name` exists in `symbols`. The `modules` map is a subset of `files`, and `module_name_to_sources` collects the normalized paths from those entries.

#### Invariants

- `uses_modules` is `true` if and only if at least one module declaration was found during extraction.
- Keys in `modules` are normalized source file paths.
- `symbol_ids_by_qualified_name` may contain multiple `SymbolID` entries per qualified name when overloads exist.
- All maps are populated only after extraction completes.

#### Key Members

- `symbols`
- `files`
- `namespaces`
- `modules`
- `symbol_ids_by_qualified_name`
- `module_name_to_sources`
- `file_order`
- `uses_modules`

#### Usage Patterns

- Used during code generation and evidence building to look up symbols by qualified name and modules by name.
- Accessed after extraction to retrieve the project model for downstream processing.

### `clore::extract::SourceLocation`

Declaration: `src/extract/model.cppm:81`

Definition: `src/extract/model.cppm:81`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct holds location information in three public fields: a `std::string file`, a `std::uint32_t line`, and a `std::uint32_t column`. The key invariant is that a `line` value of `0` marks an unknown location; valid declarations use 1-based line numbers. This invariant is exposed by the `is_known()` member, which simply returns `line != 0`. The default initialization of `line` to `0` ensures that a default-constructed `clore::extract::SourceLocation` is considered unknown, and the `is_known()` query is efficient, consisting of a single comparison. No other invariants are enforced; the fields are freely writable and represent no implicit relationships among themselves beyond the documented meaning of `line == 0`.

#### Invariants

- `line == 0` indicates an unknown location
- Valid source lines are positive integers (>=1)
- Default construction yields an unknown location

#### Key Members

- `file`
- `line`
- `column`
- `is_known()`

#### Usage Patterns

- Stored as a member in other extraction-related types
- Checked via `is_known()` to determine if location is valid
- Passed around as a value type to convey source provenance

#### Member Functions

##### `clore::extract::SourceLocation::is_known`

Declaration: `src/extract/model.cppm:87`

Definition: `src/extract/model.cppm:87`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
[[nodiscard]] bool is_known() const noexcept {
        return line != 0;
    }
```

### `clore::extract::SourceRange`

Declaration: `src/extract/model.cppm:92`

Definition: `src/extract/model.cppm:92`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct is a simple aggregate type that bundles two `SourceLocation` values, `begin` and `end`, to represent an interval in source text. No invariants are enforced by the class itself; callers are responsible for ensuring that `begin` logically precedes `end` when constructing or assigning a `SourceRange`. Because the struct declares no user‑defined special member functions, the compiler provides default constructor, copy, move, assignment, and destructor, giving the object plain value semantics.

#### Invariants

- No documented invariants; the relationship between begin and end is not enforced by the type.

#### Key Members

- `begin`: the starting location of the range
- `end`: the ending location of the range

#### Usage Patterns

- Likely used to represent source regions for diagnostics or highlighting in `clore::extract` contexts.

### `clore::extract::SymbolID`

Declaration: `src/extract/model.cppm:45`

Definition: `src/extract/model.cppm:45`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::SymbolID` stores a compound identifier composed of a primary 64-bit `hash` and a secondary 32-bit `signature`. The critical invariant is that a valid `SymbolID` must have a non-zero `hash`; a zero `hash` represents the invalid or null sentinel. The `signature` field is derived from the same USR source and serves to disambiguate the extremely improbable scenario of a 64-bit hash collision, effectively widening the unique identifier space.

The member functions enforce and leverage this invariant. The `is_valid()` method simply checks whether `hash` is non-zero. Both `operator==` and `operator<=>` are defaulted, performing lexicographic comparison first by `hash` and then by `signature`. This ordering ensures that invalid `IDs` (with zero `hash`) consistently compare as less than any valid ID, and that valid `IDs` are strictly ordered by their combined `hash` and `signature` values.

#### Invariants

- `hash == 0` denotes an invalid/null identifier.
- Valid `SymbolID` objects always have a non-zero `hash`.
- The `signature` field disambiguates the astronomically unlikely case of a 64-bit hash collision.

#### Key Members

- `hash`
- `signature`
- `is_valid()`

#### Usage Patterns

- Used as a unique, comparable identifier for symbols extracted from source code.
- Comparison `operator`s (`==`, `<=>`) allow sorting and hashing in containers.
- `is_valid()` is used to test whether a `SymbolID` represents an actual symbol.

#### Member Functions

##### `clore::extract::SymbolID::is_valid`

Declaration: `src/extract/model.cppm:52`

Definition: `src/extract/model.cppm:52`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
[[nodiscard]] bool is_valid() const noexcept {
        return hash != 0;
    }
```

##### `clore::extract::SymbolID::operator<=>`

Declaration: `src/extract/model.cppm:57`

Definition: `src/extract/model.cppm:57`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
auto operator<=>(const SymbolID&) const = default
```

##### `clore::extract::SymbolID::operator==`

Declaration: `src/extract/model.cppm:56`

Definition: `src/extract/model.cppm:56`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
bool operator==(const SymbolID&) const = default
```

### `clore::extract::SymbolInfo`

Declaration: `src/extract/model.cppm:97`

Definition: `src/extract/model.cppm:97`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::SymbolInfo` aggregates all extracted metadata about a single symbol. It stores core identity through `id`, `name`, `qualified_name`, `enclosing_namespace`, and `signature`. Location information is captured by `declaration_location` and an optional `definition_location`. Documentation and source text are retained in `doc_comment` and `source_snippet`; when the snippet is empty, the four companion fields (`source_snippet_offset`, `source_snippet_length`, `source_snippet_file_size`, `source_snippet_hash`) encode a lazy reference into the raw file content of `declaration_location.file`, minimizing memory overhead. The hierarchy is represented by an optional `parent`, explicit `lexical_parent_name` and `lexical_parent_kind`, and a `children` vector. Relationship tracking includes `bases`/`derived` for inheritance, `calls`/`called_by` for call graph edges, and `references`/`referenced_by` for usage links. Additional metadata fields are `access` (e.g., public/private), `is_template`, and `template_params`. All identifiers for related symbols use `SymbolID` values, ensuring a consistent, referential structure throughout the extraction model.

#### Invariants

- `id` is unique per symbol
- `name` is the unqualified name
- `source_snippet_offset`/`length`/`file_size`/`hash` are used only when `source_snippet` is empty
- `parent` and `children` are consistent for tree structure
- `bases` and `derived` are consistent for inheritance
- `calls` and `called_by` are consistent for call graph
- `references` and `referenced_by` are consistent for reference graph

#### Key Members

- `id`
- `kind`
- `name`
- `qualified_name`
- `declaration_location`
- `definition_location`
- `parent`
- `children`
- `bases`
- `derived`
- `calls`
- `called_by`
- `references`
- `referenced_by`
- `is_template`

#### Usage Patterns

- Used as the output of extraction
- Clients read fields to build documentation or analysis
- `parent`/`children` used for hierarchy
- `bases`/`derived` for inheritance
- `calls`/`called_by` for call graph
- `references`/`referenced_by` for usage tracking
- Lazy source snippet fields used for on-demand loading

### `clore::extract::SymbolKind`

Declaration: `src/extract/model.cppm:25`

Definition: `src/extract/model.cppm:25`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The `clore::extract::SymbolKind` enumeration is defined as a scoped `enum class` with an underlying type of `std::uint8_t`, ensuring compact storage and compatibility with serialization or bit‑level operations. Its enumerators directly represent the distinct categories of symbols recognized during extraction, such as `Namespace`, `Class`, `Struct`, `Union`, `Enum`, `EnumMember`, `Function`, `Method`, `Variable`, `Field`, `TypeAlias`, `Macro`, `Template`, `Concept`, and the sentinel `Unknown`. The inclusion of `Unknown` guarantees a catch‑all value for handling unsupported or unrecognized symbols, allowing fallback logic without undefined behavior. No custom member functions are declared; the enumeration relies on default compiler‑generated operations, maintaining a trivial, value‑type invariant where every valid symbol kind maps to a unique integral code in the range `0`–`14`.

#### Invariants

- Each enumerator represents a unique symbol category.
- The underlying type `std::uint8_t` ensures compact storage.
- The set of members is fixed and defined at compile time.

#### Key Members

- `Namespace`
- `Class`
- `Function`
- `Method`
- `Variable`
- `Unknown`

#### Usage Patterns

- Used to tag extracted symbols with their kind for downstream processing.
- Employed in switch statements or lookup tables to dispatch behavior per symbol type.
- Referenced in serialization or storage of symbol metadata.

#### Member Variables

##### `clore::extract::SymbolKind::Class`

Declaration: `src/extract/model.cppm:27`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Class
```

##### `clore::extract::SymbolKind::Concept`

Declaration: `src/extract/model.cppm:39`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Concept
```

##### `clore::extract::SymbolKind::Enum`

Declaration: `src/extract/model.cppm:30`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Enum
```

##### `clore::extract::SymbolKind::EnumMember`

Declaration: `src/extract/model.cppm:31`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
EnumMember
```

##### `clore::extract::SymbolKind::Field`

Declaration: `src/extract/model.cppm:35`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Field
```

##### `clore::extract::SymbolKind::Function`

Declaration: `src/extract/model.cppm:32`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Function
```

##### `clore::extract::SymbolKind::Macro`

Declaration: `src/extract/model.cppm:37`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Macro
```

##### `clore::extract::SymbolKind::Method`

Declaration: `src/extract/model.cppm:33`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Method
```

##### `clore::extract::SymbolKind::Namespace`

Declaration: `src/extract/model.cppm:26`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Namespace
```

##### `clore::extract::SymbolKind::Struct`

Declaration: `src/extract/model.cppm:28`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Struct
```

##### `clore::extract::SymbolKind::Template`

Declaration: `src/extract/model.cppm:38`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Template
```

##### `clore::extract::SymbolKind::TypeAlias`

Declaration: `src/extract/model.cppm:36`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
TypeAlias
```

##### `clore::extract::SymbolKind::Union`

Declaration: `src/extract/model.cppm:29`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Union
```

##### `clore::extract::SymbolKind::Unknown`

Declaration: `src/extract/model.cppm:40`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Unknown
```

##### `clore::extract::SymbolKind::Variable`

Declaration: `src/extract/model.cppm:34`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Variable
```

## Functions

### `clore::extract::find_module_by_name`

Declaration: `src/extract/model.cppm:205`

Definition: `src/extract/model.cppm:433`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::find_module_by_name` resolves a module name to a single `ModuleUnit` pointer by first delegating to `find_modules_by_name` to collect all matching modules from the `ProjectModel`. The control flow then branches based on the size of the returned collection: an empty result yields `nullptr`, a single result is returned directly, and multiple results trigger a scan to count interface units (using `ModuleUnit::is_interface`). If exactly one interface unit exists, it is returned; if none exist, a warning is logged and the first module is returned as a fallback; if multiple interfaces are found, an ambiguity warning is logged and `nullptr` is returned. The algorithm depends on `find_modules_by_name`, the `ModuleUnit` and `ProjectModel` types, and the `logging::warn` facility.

#### Side Effects

- Logs warnings via `logging::warn` when there are multiple modules matching the name (either all non-interface or multiple interfaces)

#### Reads From

- `model` (the `ProjectModel`)
- `module_name` (a `std::string_view`)
- collection returned by `clore::extract::find_modules_by_name(model, module_name)`
- `module->is_interface` for each `ModuleUnit*` in the collection

#### Usage Patterns

- Resolve a module name to a single module unit, handling ambiguous definitions
- Used in contexts where a unique module is required for further analysis

### `clore::extract::find_module_by_source`

Declaration: `src/extract/model.cppm:211`

Definition: `src/extract/model.cppm:466`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function performs a direct lookup in the `model.modules` associative container using the provided `source_file` as a key. If the key exists, it returns a pointer to the corresponding `ModuleUnit`; otherwise, it returns `nullptr`. There is no iteration or fallback logic, making the implementation a simple constant-time (or logarithmic, depending on the underlying container) search. The only external dependency is the `ProjectModel` structure, specifically its `modules` member, which is expected to be a map from source file paths to `ModuleUnit` objects. No other model fields or helper functions are consulted.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model.modules`
- `source_file` parameter

#### Usage Patterns

- used to obtain a module unit by its source path
- often called during project traversal or resolution

### `clore::extract::find_modules_by_name`

Declaration: `src/extract/model.cppm:208`

Definition: `src/extract/model.cppm:412`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function iterates over the result of a map lookup in `model.module_name_to_sources` for the given `module_name`. If the lookup fails or the associated list is empty, an empty vector is returned immediately. Otherwise, it reserves capacity for the expected number of results and calls `find_module_by_source` for each source file entry, appending any valid `ModuleUnit` pointer. Finally, the collected pointers are sorted by their `source_file` field to produce a deterministic order, and the vector is returned.

Internally, the function relies on the `ProjectModel`'s mapping from module names to source file paths (`module_name_to_sources`) and on the helper `find_module_by_source` to resolve a source path to a `ModuleUnit`. It does not perform any in-depth symbol analysis; its purpose is to locate all module units that match a given logical module name, handling the case where a module spans multiple source files or translation units.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model.module_name_to_sources`
- indirectly reads from `model` via `find_module_by_source`

#### Usage Patterns

- resolving module references by name
- enumerating module units with a specific name
- preparing data for module-related queries

### `clore::extract::find_symbol`

Declaration: `src/extract/model.cppm:198`

Definition: `src/extract/model.cppm:396`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/find-symbol.md)

The function first checks whether the `signature` argument is empty; if so, it delegates to the single‑parameter overload of `find_symbol`, which performs a direct lookup of `qualified_name` in the `ProjectModel`’s `symbol_ids_by_qualified_name` mapping. Otherwise, it calls `find_symbols` to retrieve all symbols whose qualified name matches, then linearly scans the returned list, comparing each symbol’s `SymbolInfo::signature` field against the provided `signature`. The first match is returned; if no match is found, `nullptr` is returned. This disambiguation step handles overloaded or otherwise ambiguous names by requiring exact signature equality.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model` (the `ProjectModel`)
- `qualified_name`
- `signature`
- the `SymbolInfo` objects returned by `find_symbols`
- the `signature` field of each `SymbolInfo`

#### Usage Patterns

- Lookup a symbol by name and signature
- Fallback to name-only lookup when signature is empty
- Iterate over matching symbols to find exact signature match

### `clore::extract::find_symbol`

Declaration: `src/extract/model.cppm:196`

Definition: `src/extract/model.cppm:388`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/find-symbol.md)

The implementation of `clore::extract::find_symbol` delegates to `clore::extract::find_symbols` to collect all symbols matching the given `qualified_name` within the `ProjectModel`. If the returned container of matches contains exactly one element, that element (a pointer to `SymbolInfo`) is returned; otherwise, `nullptr` is returned to indicate either no match or an ambiguous result. The function serves as a restricted query that requires a unique resolution, relying entirely on the internal logic of `find_symbols` (which traverses the model’s symbol tables, namespace hierarchies, and module storage) for candidate collection. No additional caching or state management is performed at this level.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model` (the project model's symbol index)
- `qualified_name` (the lookup string)

#### Usage Patterns

- Used to resolve a uniquely identifiable symbol by qualified name.
- Called by the three-parameter overload to refine lookup by module.

### `clore::extract::find_symbols`

Declaration: `src/extract/model.cppm:202`

Definition: `src/extract/model.cppm:371`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::find_symbols` performs a direct lookup in the `ProjectModel::symbol_ids_by_qualified_name` map using the provided `qualified_name` string. If the key is absent, it immediately returns an empty vector. Otherwise, it reserves capacity for the associated collection of `SymbolID` values and iterates over each element, calling `clore::extract::lookup_symbol` with the model and the current `SymbolID` to obtain a pointer to a `const SymbolInfo`. Only non‑null pointers are appended to the result vector, which is finally returned. The algorithm depends on the integrity of the `symbol_ids_by_qualified_name` index and on `lookup_symbol` being able to resolve every stored `SymbolID` within the same `ProjectModel`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- model`.symbol_ids_by_qualified_name`
- `lookup_symbol`(model, `symbol_id`)

#### Usage Patterns

- Resolving all symbols with a given fully qualified name within a project model
- Displaying symbol information for autocompletion or navigation
- Finding overloads or multiple declarations of the same name

### `clore::extract::join_qualified_name_parts`

Declaration: `src/extract/model.cppm:76`

Definition: `src/extract/model.cppm:345`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first determines `safe_count` as the minimum of `count` and `parts.size()`, ensuring it never exceeds the actual number of elements. It then iterates over the first `safe_count` elements of the `parts` vector, concatenating each element to a local `joined` string. Before appending any element beyond the first, it inserts the scope resolution `operator` `"::"`. The loop index `index` is used both for bounds checking and to conditionally add the separator. The resulting `joined` string is returned. The algorithm relies only on standard library facilities (`std::vector` and `std::string`) and performs no external lookups or model traversals.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `const std::vector<std::string>& parts`
- parameter `std::size_t count`

#### Usage Patterns

- Reconstructing a qualified name from its individual components
- Used when combining namespace or type name parts

### `clore::extract::lookup_symbol`

Declaration: `src/extract/model.cppm:194`

Definition: `src/extract/model.cppm:366`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::lookup_symbol` performs a direct lookup in the `ProjectModel::symbols` map using the provided `SymbolID` as the key. It calls `std::map::find` on the `model.symbols` container, relying on the ordering defined by `SymbolID::operator<=>` and hashing via `std::hash<SymbolID>`. If the key is found, the function returns a pointer to the corresponding `SymbolInfo`; otherwise, it returns `nullptr`. No additional validation, fallback logic, or cross‑reference resolution is performed—the function is a straightforward accessor that delegates entirely to the associative container’s lookup mechanism. Its dependencies are limited to the `ProjectModel` type (specifically the `symbols` field) and the comparability of `SymbolID`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model.symbols` (member of `ProjectModel`)
- `id` parameter

#### Usage Patterns

- Look up a symbol by its unique identifier
- Used in symbol resolution and merging operations

### `clore::extract::namespace_prefix_from_qualified_name`

Declaration: `src/extract/model.cppm:79`

Definition: `src/extract/model.cppm:358`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function splits the input `qualified_name` into its top-level components by calling `split_top_level_qualified_name`. If the resulting number of parts is one or fewer, an empty string is returned immediately—there is no namespace qualifier. Otherwise, it invokes `join_qualified_name_parts` with all parts except the last one (the simple name) to reconstruct the namespace prefix as a `std::string`. The algorithm relies on two helper functions: `split_top_level_qualified_name` to decompose the qualified name and `join_qualified_name_parts` to recombine the leading components. Internal control flow is minimal: a single conditional branch followed by either an early `return {}` or a call to the joiner.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `qualified_name`

#### Usage Patterns

- Extract namespace prefix from qualified name
- Used in symbol processing to determine containing namespace

### `clore::extract::resolve_source_snippet`

Declaration: `src/extract/model.cppm:217`

Definition: `src/extract/model.cppm:472`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::resolve_source_snippet` begins by acquiring a per-symbol mutex via `source_snippet_mutex_for`, ensuring thread-safe access to the `SymbolInfo` fields. It short-circuits if `sym.source_snippet` is already populated, returning `true`. Otherwise, it validates that `sym.source_snippet_length` is non‑zero and that `sym.declaration_location.file` is non‑empty; failure here returns `false`. The function then checks the on‑disk file size against the cached `sym.source_snippet_file_size` (if nonzero) and verifies that the stored `source_snippet_offset` and `source_snippet_length` are within bounds of the current file. It opens the file in binary mode, seeks to the recorded offset, and reads exactly `length` bytes into a local buffer. If `sym.source_snippet_hash` is nonzero, the function computes a hash of the buffer via `hash_source_snippet_bytes` and compares it; a mismatch causes early return `false`. Finally, it normalizes any `\r\n` sequences to `\n` and moves the result into `sym.source_snippet`, returning `true` to indicate successful resolution.

#### Side Effects

- Acquires a mutex (`source_snippet_mutex_for(sym)`)
- Reads from the filesystem (file size via `std::filesystem::file_size`, file content via `std::ifstream`)
- Writes to `sym.source_snippet` (mutates the `SymbolInfo` object)

#### Reads From

- `sym.source_snippet`
- `sym.source_snippet_length`
- `sym.declaration_location.file`
- `sym.source_snippet_offset`
- `sym.source_snippet_file_size`
- `sym.source_snippet_hash`
- File system state (file size and contents)
- Return value of `hash_source_snippet_bytes(result)`

#### Writes To

- `sym.source_snippet` (the normalized snippet string)
- Mutex state (acquired and released)

#### Usage Patterns

- Called to lazily resolve the source snippet for a `SymbolInfo`
- Used after symbol extraction to provide the source text
- May be invoked from display or export code paths

### `clore::extract::split_top_level_qualified_name`

Declaration: `src/extract/model.cppm:74`

Definition: `src/extract/model.cppm:282`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first performs an early return with an empty vector if the input `qualified_name` is empty. To avoid redundant work, it attempts a shared-lock lookup in the global cache obtained from `split_qualified_name_cache()`. If a match for `qualified_name` is found, the cached result is returned immediately. Otherwise, the algorithm parses the string character by character. It maintains a `template_depth` counter to ignore `::` separators that appear inside template angle brackets (e.g., `A<B::C>::D`). When a `::` is encountered at depth zero, the accumulated `current` token is appended to the `parts` vector and cleared. Angle brackets increment or decrement the depth as appropriate, and all other characters are appended to `current`. After the loop, any remaining `current` content is moved into `parts`, and a leading empty token (caused by a leading `::`) is removed. Finally, a unique lock is acquired on the same cache; if the cache has reached `kSplitQualifiedNameCacheMaxEntries` entries, it is cleared before inserting the new mapping. The computed `parts` vector is then returned.

#### Side Effects

- mutates the global cache `split_qualified_name_cache` by inserting or updating entries
- acquires and releases shared and unique locks on the cache mutex

#### Reads From

- parameter `qualified_name`
- global cache returned by `split_qualified_name_cache()` (mutex, map)

#### Writes To

- global cache map `parts_by_qualified_name` inside `split_qualified_name_cache()`

#### Usage Patterns

- decomposing qualified names for symbol lookup or matching
- cached parsing of qualified names to avoid repeated work

### `clore::extract::symbol_kind_name`

Declaration: `src/extract/model.cppm:43`

Definition: `src/extract/model.cppm:261`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::symbol_kind_name` implements a direct mapping from the `SymbolKind` enumeration to a human-readable string label. Its control flow consists of a single `switch` statement that exhaustively covers every `SymbolKind` enumerator — `Namespace`, `Class`, `Struct`, `Union`, `Enum`, `EnumMember`, `Function`, `Method`, `Variable`, `Field`, `TypeAlias`, `Macro`, `Template`, `Concept`, and `Unknown` — returning a corresponding string literal such as `"namespace"` or `"enum_member"`. A fallback `return "unknown"` clause ensures a safe default even if the enumeration is extended, though all known values are explicitly handled.

The function depends solely on the `SymbolKind` type, which is defined in the same module. No additional data structures, external lookups, or allocation are required; the result is a compile-time constant `std::string_view` for each case.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `kind` parameter of type `SymbolKind`

#### Usage Patterns

- convert `SymbolKind` to string for logging or UI
- used in error messages
- used in debugging output

## Internal Structure

The `extract:model` module defines the core data structures and query interface for representing an extracted C++ project. It imports the `support` module for string handling and utility functions. Internally, the module is decomposed into public entity types—`SymbolKind`, `SymbolID`, `SourceLocation`, `SourceRange`, `SymbolInfo`, `FileInfo`, `NamespaceInfo`, `ModuleUnit`, and `ProjectModel`—alongside public lookup functions (`lookup_symbol`, `find_symbol`, `find_symbols`, `find_module_*`) and name decomposition helpers (`split_top_level_qualified_name`, `join_qualified_name_parts`). An anonymous namespace provides internal caching (`SplitQualifiedNameCache`) and thread-safe mechanisms for on‑demand source snippet resolution, using a stripe‑based mutex strategy to limit contention. This layering separates the raw, immutable model data from the higher‑level query logic, while the internal helpers improve performance for repeated operations during indexing and cross‑referencing.

## Related Pages

- [Module support](../support/index.md)

