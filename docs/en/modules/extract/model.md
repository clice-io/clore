---
title: 'Module extract:model'
description: 'The extract:model module defines the core data model for representing the results of C++ source code extraction. It owns a collection of public data structures—ProjectModel, SymbolInfo, ModuleUnit, FileInfo, NamespaceInfo, SymbolID, SourceLocation, SourceRange, and the SymbolKind enumeration—that capture extracted symbols, their hierarchical relationships, source locations, C++20 module units, file metadata, and namespace structure. The module also provides a suite of query functions, such as lookup_symbol, find_symbol, find_module_by_name, find_modules_by_name, find_module_by_source, find_symbols, resolve_source_snippet, split_top_level_qualified_name, join_qualified_name_parts, namespace_prefix_from_qualified_name, and symbol_kind_name, enabling consumers to efficiently search and navigate the extracted data.'
layout: doc
template: doc
---

# Module `extract:model`

## Summary

The `extract:model` module defines the core data model for representing the results of C++ source code extraction. It owns a collection of public data structures—`ProjectModel`, `SymbolInfo`, `ModuleUnit`, `FileInfo`, `NamespaceInfo`, `SymbolID`, `SourceLocation`, `SourceRange`, and the `SymbolKind` enumeration—that capture extracted symbols, their hierarchical relationships, source locations, C++20 module units, file metadata, and namespace structure. The module also provides a suite of query functions, such as `lookup_symbol`, `find_symbol`, `find_module_by_name`, `find_modules_by_name`, `find_module_by_source`, `find_symbols`, `resolve_source_snippet`, `split_top_level_qualified_name`, `join_qualified_name_parts`, `namespace_prefix_from_qualified_name`, and `symbol_kind_name`, enabling consumers to efficiently search and navigate the extracted data.

## Imports

- `std`
- [`support`](../support/index.md)

## Imported By

- [`extract:ast`](ast.md)
- [`extract:merge`](merge.md)

## Types

### `clore::extract::FileInfo`

Declaration: `extract/model.cppm:122`

Definition: `extract/model.cppm:122`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::FileInfo` is a plain aggregate data holder that groups together the key pieces of information produced by the extraction process for a single source file. Its three public data members—`path`, `symbols`, and `includes`—are left directly accessible; there is no encapsulation or computed accessors. The invariant across all instances is that `path` must contain a valid, normalized file path string (relative or absolute), while `symbols` and `includes` are simply vectors that may be empty or filled with the corresponding extracted entities. Because the struct has no constructors beyond the implicitly generated ones and no member functions, its sole implementation concern is that its fields are default‑initialized correctly: `path` is an empty string, and both vectors are empty. This minimal design prioritises simplicity and direct field access over information hiding, making it suitable for use as a lightweight transport object within the extraction pipeline.

#### Invariants

- No invariants enforced beyond type safety of the fields
- All fields are public and mutable

#### Key Members

- `path`: the filesystem path of the source file
- `symbols`: a vector of `SymbolID` representing symbols defined or declared in the file
- `includes`: a vector of strings representing include directives encountered in the file

#### Usage Patterns

- Populated by extraction logic to record symbolic information per file
- Consumed by downstream analysis or serialization to retrieve file-level extraction results

### `clore::extract::ModuleUnit`

Declaration: `extract/model.cppm:135`

Definition: `extract/model.cppm:135`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::ModuleUnit` is a plain aggregate used internally to hold the result of parsing a single C++20 module unit (either an interface unit or a partition). Its fields directly capture the unit’s identity and content: `name` stores the fully qualified module name (e.g., `"foo"` or `"foo:bar"`), `is_interface` distinguishes `export module` units from non‑export ones, `source_file` records the normalized source path, `imports` lists the names of every module imported by this unit, and `symbols` collects the `SymbolID` values for all declarations that belong to the unit. No invariants are enforced by the struct itself; it serves as a straightforward data transfer object that the extraction pipeline populates and downstream consumers read.

#### Invariants

- `name` is a full module name in `"module"` or `"module:partition"` form.
- `is_interface` is `true` for `export module` units, `false` for internal partition units.
- `source_file` is a normalized filesystem path.
- `imports` contains only module names, not header units or other imports.
- `symbols` lists all symbols declared within that unit.

#### Key Members

- `name`
- `is_interface`
- `source_file`
- `imports`
- `symbols`

#### Usage Patterns

- Populated during module extraction and used as a data carrier for further analysis.
- Accessed by other parts of `clore::extract` to query module metadata.
- Stored in collections or containers for processing across multiple module units.

### `clore::extract::NamespaceInfo`

Declaration: `extract/model.cppm:128`

Definition: `extract/model.cppm:128`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The `clore::extract::NamespaceInfo` struct is a plain data container that captures the results of extracting a single C++ namespace. It stores the namespace’s unqualified `name` as a `std::string`, a vector of `SymbolID` values representing all symbols declared directly within that namespace (e.g., functions, classes, variables), and a `children` vector of `std::string` names listing the immediate nested namespaces. The design is intentionally minimal: no member functions, no invariants beyond the requirement that each `children` entry must correspond to a valid subordinate namespace (enforced externally by the extraction logic). The struct serves purely as a transfer object between the extraction phase and any analysis or serialization that follows, relying entirely on the caller to populate and interpret its fields consistently.

#### Invariants

- The `name` uniquely identifies the namespace within its parent scope
- `symbols` contains only `IDs` of symbols defined directly in this namespace, not inherited
- `children` contains names of direct child namespaces, not transitive

#### Key Members

- `name`
- `symbols`
- `children`

#### Usage Patterns

- Populated during the namespace extraction phase by iterating over declarations
- Later read by documentation generators to produce namespace pages and links to contained symbols and sub-namespaces

### `clore::extract::ProjectModel`

Declaration: `extract/model.cppm:143`

Definition: `extract/model.cppm:143`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::ProjectModel` serves as the central container holding all extracted information for a single parsed project. Its primary maps (`symbols`, `files`, `namespaces`, `modules`) store the core entities keyed by `SymbolID`, file path, namespace name, and normalized source file path respectively. The `files` and `modules` maps use `std::string` keys with transparent hash and equality comparators (`clore::support::TransparentStringHash` and `clore::support::TransparentStringEqual`) to enable efficient heterogenous lookups without constructing temporary string objects. The `file_order` vector preserves the stable order in which files were processed, which is essential for deterministic output generation.

To support fast generation and cross-linking, the model maintains two auxiliary lookup maps: `symbol_ids_by_qualified_name` maps a fully qualified name to a `std::vector<SymbolID>` (allowing multiple overloads to share the same qualified name), and `module_name_to_sources` maps a module name to the list of source file paths that define or declare that module. The boolean `uses_modules` is initially `false` and is set to `true` when at least one module declaration is encountered during extraction, indicating that the project uses C++20 modules. Key invariants include that every entry in `symbols`, `files`, and `namespaces` is fully populated after extraction, that `file_order` contains exactly the keys of `files`, and that all string keys in the transparent maps are normalized to a canonical form.

### `clore::extract::SourceLocation`

Declaration: `extract/model.cppm:64`

Definition: `extract/model.cppm:64`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::SourceLocation` aggregates three data members: a `std::string file` for the source file path, and two `std::uint32_t` members `line` and `column`. The core invariant is that `line == 0` denotes an unknown or invalid location; all valid source lines are numbered from 1 onward. The `column` member is only meaningful when `line` is known, though no additional constraints are imposed on its value. The member function `is_known` is implemented as a straightforward comparison `return line != 0;`, providing a canonical, efficient check for location validity. Default construction zero-initializes `line` and `column`, ensuring that every default‑created instance is initially in the unknown state.

#### Invariants

- Line 0 signifies an unknown location; valid lines start at 1.
- Column may be 0 even for known locations, indicating an unknown column.
- The `file` string is stored as-is without validation.

#### Key Members

- `file` field
- `line` field
- `column` field
- `is_known()` method

#### Usage Patterns

- Track source positions in extracted or generated code.
- Check with `is_known()` before relying on `line` or `column` values.
- Default-constructed locations are treated as unknown.

#### Member Functions

##### `clore::extract::SourceLocation::is_known`

Declaration: `extract/model.cppm:70`

Definition: `extract/model.cppm:70`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
[[nodiscard]] bool is_known() const noexcept {
        return line != 0;
    }
```

### `clore::extract::SourceRange`

Declaration: `extract/model.cppm:75`

Definition: `extract/model.cppm:75`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::SourceRange` is a plain aggregate data type that stores two `clore::extract::SourceLocation` fields, `begin` and `end`. Together these define a contiguous region in source code; no inherent invariant (such as ordering or half‑open intervals) is enforced by the struct itself—any such semantics are imposed externally by the code that constructs or consumes a `SourceRange`. Because the struct declares no user‑defined special member functions, it relies entirely on the compiler‑generated default constructor, destructor, and copy/move operations, making it a trivial value type for representing a pair of source positions.

#### Invariants

- Both `begin` and `end` are valid `SourceLocation` values.
- The struct is trivially copyable and movable as an aggregate.

#### Key Members

- `begin`
- `end`

#### Usage Patterns

- Used to represent a source code span, likely for diagnostics or code extraction.
- Can be passed by value or returned from functions that produce a location range.
- Expected to be compared or ordered, though no `operator`s are shown in evidence.

### `clore::extract::SymbolID`

Declaration: `extract/model.cppm:28`

Definition: `extract/model.cppm:28`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::SymbolID` stores a pair of fields that together uniquely identify a named entity: a 64‑bit `hash` and a 32‑bit `signature`. The core invariant is that a value of `hash == 0` denotes the invalid or null sentinel; all valid identifiers have a non‑zero `hash`. The `signature` provides an additional disambiguation token, derived from the same USR source, to guard against the astronomically unlikely event of a 64‑bit hash collision.  

The member function `is_valid` simply checks `hash != 0` and is marked `noexcept`. Both `operator==` and `operator<=>` are defaulted, giving the struct straightforward equality and total ordering semantics based on all fields.

#### Invariants

- A `SymbolID` with `hash == 0` is invalid/null; all valid `IDs` have non-zero hash.
- The `hash` and `signature` together form a unique identity for a symbol.

#### Key Members

- `std::uint64_t hash`
- `std::uint32_t signature`
- `bool is_valid() const noexcept`
- `bool operator==(const SymbolID&) const = default`
- `auto operator<=>(const SymbolID&) const = default`

#### Usage Patterns

- Used as a key or identifier for symbols in extraction pipelines.
- Comparison `operator`s enable use in sorted containers and equality checks.
- `is_valid()` guards against using default-constructed or sentinel values.

#### Member Functions

##### `clore::extract::SymbolID::is_valid`

Declaration: `extract/model.cppm:35`

Definition: `extract/model.cppm:35`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
[[nodiscard]] bool is_valid() const noexcept {
        return hash != 0;
    }
```

##### `clore::extract::SymbolID::operator<=>`

Declaration: `extract/model.cppm:40`

Definition: `extract/model.cppm:40`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
auto operator<=>(const SymbolID&) const = default
```

##### `clore::extract::SymbolID::operator==`

Declaration: `extract/model.cppm:39`

Definition: `extract/model.cppm:39`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
bool operator==(const SymbolID&) const = default
```

### `clore::extract::SymbolInfo`

Declaration: `extract/model.cppm:80`

Definition: `extract/model.cppm:80`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::SymbolInfo` aggregates all extracted metadata for a single symbol. Its design separates identity (`id`, `name`, `qualified_name`, `enclosing_namespace`, `signature`) from source provenance (`declaration_location`, optional `definition_location`, `doc_comment`, and the `source_snippet` fields). To reduce memory when the full snippet is not needed, the implementation supports a lazy‑resolution pattern: if `source_snippet` is empty, the raw text is implicitly referenced by the file path in `declaration_location.file` combined with `source_snippet_offset`, `source_snippet_length`, `source_snippet_file_size`, and `source_snippet_hash`; these fields are unused when the snippet is stored directly.  

The struct maintains two orthogonal hierarchies: a tree of `parent`/`children` relationships (with `lexical_parent_name` and `lexical_parent_kind` for quick display) and an inheritance graph via `bases`/`derived`. Call‑graph edges are stored in `calls`/`called_by`, and generic reference links in `references`/`referenced_by`. Template specialisation is flagged with `is_template` and its formal parameters are captured in `template_params`. Access specifier is stored as a string (`access`). All relationship fields are vectors of `SymbolID`, ensuring no strong ownership cycles and allowing the extractor to build these links after all symbols have been collected.

### `clore::extract::SymbolKind`

Declaration: `extract/model.cppm:8`

Definition: `extract/model.cppm:8`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The `clore::extract::SymbolKind` enum is a scoped enumeration backed by `std::uint8_t` that categorises all symbol types encountered during extraction. Each enumerator maps to a distinct kind, with `Unknown` serving as the default fallback value. The enumerators are implicitly assigned consecutive integer values starting from zero, and the underlying type ensures the representation remains compact and trivially copyable. No invariants beyond the ordering are enforced; the enum is intended for straightforward classification and switch‑based dispatch within the extraction internals.

#### Invariants

- Each enumerator is a distinct integer value within `std::uint8_t` range.
- All possible symbol kinds are represented by named enumerators; `Unknown` acts as a catch-all.
- The enum is not a bitmask; values are mutually exclusive.

#### Key Members

- `Namespace`
- `Class`
- `Struct`
- `Union`
- `Enum`
- `EnumMember`
- `Function`
- `Method`
- `Variable`
- `Field`
- `TypeAlias`
- `Macro`
- `Template`
- `Concept`
- `Unknown`

#### Usage Patterns

- Used as a member in symbol data structures to indicate the kind of symbol.
- Switched upon in extraction or serialization logic to handle each kind appropriately.
- Stored alongside symbol name and location to enable type-safe operations on symbols.

#### Member Variables

##### `clore::extract::SymbolKind::Class`

Declaration: `extract/model.cppm:10`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Class
```

##### `clore::extract::SymbolKind::Concept`

Declaration: `extract/model.cppm:22`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Concept
```

##### `clore::extract::SymbolKind::Enum`

Declaration: `extract/model.cppm:13`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Enum
```

##### `clore::extract::SymbolKind::EnumMember`

Declaration: `extract/model.cppm:14`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
EnumMember
```

##### `clore::extract::SymbolKind::Field`

Declaration: `extract/model.cppm:18`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Field
```

##### `clore::extract::SymbolKind::Function`

Declaration: `extract/model.cppm:15`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Function
```

##### `clore::extract::SymbolKind::Macro`

Declaration: `extract/model.cppm:20`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Macro
```

##### `clore::extract::SymbolKind::Method`

Declaration: `extract/model.cppm:16`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Method
```

##### `clore::extract::SymbolKind::Namespace`

Declaration: `extract/model.cppm:9`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Namespace
```

##### `clore::extract::SymbolKind::Struct`

Declaration: `extract/model.cppm:11`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Struct
```

##### `clore::extract::SymbolKind::Template`

Declaration: `extract/model.cppm:21`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Template
```

##### `clore::extract::SymbolKind::TypeAlias`

Declaration: `extract/model.cppm:19`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
TypeAlias
```

##### `clore::extract::SymbolKind::Union`

Declaration: `extract/model.cppm:12`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Union
```

##### `clore::extract::SymbolKind::Unknown`

Declaration: `extract/model.cppm:23`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Unknown
```

##### `clore::extract::SymbolKind::Variable`

Declaration: `extract/model.cppm:17`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Variable
```

## Functions

### `clore::extract::find_module_by_name`

Declaration: `extract/model.cppm:188`

Definition: `extract/model.cppm:416`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::find_module_by_name` first delegates to `find_modules_by_name` to obtain all `ModuleUnit` pointers in the `ProjectModel` whose name matches the given `module_name`. If the resulting collection is empty, it returns `nullptr`. For a single candidate, it returns that module directly. When multiple modules share the same name, the function applies a disambiguation strategy based on the `ModuleUnit::is_interface` flag: if exactly one of them is an interface unit, that one is returned; if none are interfaces, it logs a warning via `logging::warn` and falls back to the first module; if two or more interfaces exist, it logs an ambiguity warning and returns `nullptr`. This control flow relies solely on `find_modules_by_name` and the interface flag, with fallback and warning mechanisms for ambiguous or implementation‑only scenarios.

#### Side Effects

- Logs warnings via `logging::warn` when multiple modules with the same name exist or when there is no interface unit

#### Reads From

- `ProjectModel` object (via delegation to `find_modules_by_name`)
- `module_name` parameter
- `is_interface` field of each `ModuleUnit` returned by `find_modules_by_name`

#### Writes To

- logging subsystem

#### Usage Patterns

- Resolving a unique module from a project model by name
- Handling ambiguous module name lookups after extraction

### `clore::extract::find_module_by_source`

Declaration: `extract/model.cppm:194`

Definition: `extract/model.cppm:449`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::find_module_by_source` performs a single lookup in the `ProjectModel::modules` container, which is keyed by source file path (as `std::string_view`). It calls `model.modules.find(source_file)` and, if the iterator is not equal to `model.modules.end()`, returns a pointer to the corresponding `ModuleUnit`; otherwise it returns `nullptr`. The function is entirely dependent on the structure of `ProjectModel::modules`, which must already be populated during model construction. Unlike the companion `clore::extract::find_module_by_name`, which searches by module name across all modules, this function relies directly on the file‑based mapping and requires no additional traversal or indexing.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model.modules` (the map from source file paths to module units)
- `source_file` (the parameter used as the lookup key)

#### Usage Patterns

- Used to obtain the module unit associated with a given source file path
- Typical in module resolution or when accessing module metadata from a source location

### `clore::extract::find_modules_by_name`

Declaration: `extract/model.cppm:191`

Definition: `extract/model.cppm:395`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::find_modules_by_name` retrieves all module units with a given name. It first performs a lookup in the `model.module_name_to_sources` map using the provided `module_name`. If the key is absent or the associated vector of source identifiers is empty, it returns an empty `std::vector<const ModuleUnit*>`. Otherwise, it reserves capacity for the number of sources and iterates over each entry, calling `find_module_by_source` to resolve each source index to a pointer to a `ModuleUnit`; only non‑null results are appended to the result vector. Finally, the collected module pointers are sorted by their `source_file` field to guarantee a deterministic order, and the sorted vector is returned. The entire flow depends on the `ProjectModel`’s auxiliary index (`module_name_to_sources`) and the helper `find_module_by_source`, which translates a source file identifier into the corresponding `ModuleUnit`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model.module_name_to_sources` map
- `module_name` parameter
- indirectly reads `model` via `find_module_by_source`
- indirectly reads `ModuleUnit::source_file` during sort

#### Writes To

- local `modules` vector (returned by value)

#### Usage Patterns

- Lookup all modules with a given name in the project model

### `clore::extract::find_symbol`

Declaration: `extract/model.cppm:179`

Definition: `extract/model.cppm:371`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::find_symbol` delegates directly to `clore::extract::find_symbols` to obtain a collection of symbols that match the given `qualified_name`. It then inspects the size of the returned container; if the container contains exactly one element, that element is returned as a pointer to `const SymbolInfo`; otherwise `nullptr` is returned. This simple control‑flow ensures that the function only returns a symbol when the qualified name uniquely identifies a single symbol within the `ProjectModel`, avoiding ambiguous results. The implementation depends on `find_symbols` to perform the underlying matching logic, which may consider multiple symbol kinds, namespaces, and module scopes.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `model` of type `const ProjectModel &`
- parameter `qualified_name` of type `std::string_view`
- function call `clore::extract::find_symbols(model, qualified_name)` returns a collection of `SymbolInfo *`

#### Usage Patterns

- Callers use this function when they need to look up a single symbol by its fully qualified name and expect exactly one match.
- Typically employed in code that resolves symbol names from user input or configuration where uniqueness is guaranteed.

### `clore::extract::find_symbol`

Declaration: `extract/model.cppm:181`

Definition: `extract/model.cppm:379`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first inspects the signature parameter. If it is empty, control immediately delegates to the two‑argument overload `clore::extract::find_symbol(model, qualified_name)`, which performs a name‑based lookup — typically leveraging a cached split of the qualified name via `clore::extract::(anonymous namespace)::split_qualified_name_cache` and searching `ProjectModel::symbols`, `ProjectModel::namespaces`, and module units for a match. When a non‑empty signature is provided, the implementation calls `clore::extract::find_symbols(model, qualified_name)` to retrieve every symbol whose `qualified_name` matches the given string. It then iterates over that result set, comparing each symbol’s `signature` field against the requested signature. The first `SymbolInfo*` whose `signature` equals the parameter is returned; if no symbol satisfies both the name and the signature, the function returns `nullptr`. This two‑stage approach separates name resolution from overload disambiguation, reusing the existing name‑based search infrastructure.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model` (const `ProjectModel&`)
- first integer parameter (likely a qualified name identifier)
- second integer parameter (likely a signature identifier)

#### Usage Patterns

- Used internally for efficient symbol lookup by numeric `IDs`
- Complemented by `find_symbol` overloads using `std::string_view` parameters

### `clore::extract::find_symbols`

Declaration: `extract/model.cppm:185`

Definition: `extract/model.cppm:354`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::find_symbols` performs a direct lookup in `model.symbol_ids_by_qualified_name` using the provided `qualified_name` key. If the key is absent, an empty vector is returned immediately. Otherwise, the function reserves capacity and iterates over each `SymbolID` stored in the corresponding value, calling `lookup_symbol` to obtain the corresponding `const SymbolInfo*` pointer; only non-null pointers are appended to the result vector.

This approach efficiently resolves multiple symbols sharing the same fully qualified name (e.g., overloaded functions or declarations across modules) by relying on the precomputed hash map, while the individual symbol retrieval delegates to the global symbol table via `lookup_symbol`. The function is a pure query—it does not modify the model and uses no internal caching beyond the provided index.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- model
- model`.symbol_ids_by_qualified_name`
- `lookup_symbol`(model, `symbol_id`)

#### Usage Patterns

- retrieve all symbols that share a given qualified name
- query the model for multiple overloads or entities with the same name

### `clore::extract::join_qualified_name_parts`

Declaration: `extract/model.cppm:59`

Definition: `extract/model.cppm:328`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function iterates over the first `count` elements of the input `parts` vector, or all elements if `count` exceeds the vector size, and concatenates them with `::` separators. It first computes a `safe_count` by clamping `count` to `parts.size()`, then loops from 0 to `safe_count - 1`, appending `::` before each element after the first. The resulting `std::string` is returned. No external project dependencies are used; only `std::vector`, `std::string`, and `std::size_t` are required. The implementation is self-contained and focuses solely on string assembly, handling edge cases such as an empty `parts` or a `count` of zero gracefully.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `parts` parameter (a `const std::vector<std::string>&`)
- `count` parameter (a `std::size_t`)

#### Usage Patterns

- Used to reassemble qualified names from a split vector of components
- Likely called during symbol extraction or name resolution to construct fully qualified names

### `clore::extract::lookup_symbol`

Declaration: `extract/model.cppm:177`

Definition: `extract/model.cppm:349`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function performs a single hash-based lookup on the `ProjectModel::symbols` associative container using the supplied `SymbolID` as the key. The control flow is linear: it invokes the container's `find` method, then checks whether the returned iterator equals `model.symbols.end()`. If the symbol is present, it returns a pointer to the corresponding `SymbolInfo` object; otherwise it returns `nullptr`. No additional validation, name resolution, or traversal of other projection structures (e.g., `ModuleUnit`, `NamespaceInfo`, or `FileInfo`) occurs within this function. The implementation relies solely on the `SymbolID` ordering semantics (defined by `operator<=>` and `operator==`) and the hash or tree structure of the underlying container.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model.symbols` container
- `id` parameter

#### Usage Patterns

- Retrieve symbol information by ID
- Check if a symbol exists in the model
- Perform safe lookup with nullptr check

### `clore::extract::namespace_prefix_from_qualified_name`

Declaration: `extract/model.cppm:62`

Definition: `extract/model.cppm:341`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::namespace_prefix_from_qualified_name` splits the input `qualified_name` into its top-level components using `split_top_level_qualified_name`. If the resulting vector of parts contains fewer than two elements, the function returns an empty string, indicating no namespace prefix exists. Otherwise, it reconstructs the prefix by joining all parts except the last one via `join_qualified_name_parts`, producing the namespace portion of the original qualified name. The only internal dependencies are these two utility functions, which handle the actual splitting and joining logic.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- Input parameter `qualified_name` of type `std::string_view`

#### Usage Patterns

- Extract namespace prefix from a fully qualified symbol name before further processing
- Utility used in symbol and namespace extraction logic

### `clore::extract::resolve_source_snippet`

Declaration: `extract/model.cppm:200`

Definition: `extract/model.cppm:455`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first acquires a per-symbol mutex via `source_snippet_mutex_for(sym)` to ensure thread safety, then checks if `sym.source_snippet` has already been resolved (cached). If the snippet is empty and required fields (`source_snippet_length`, `declaration_location.file`) are valid, it uses `std::filesystem::file_size` to open the source file and performs bounds checking on `offset` and `length` relative to the actual file size. An `std::ifstream` reads exactly `source_snippet_length` bytes starting at `source_snippet_offset`. If a `source_snippet_hash` is present, the function verifies the original raw bytes by computing `hash_source_snippet_bytes` and comparing against the stored hash. Finally, it normalizes Windows-style line endings (`\r\n`) to Unix (`\n`) by scanning the buffer and copying characters while skipping `\r` before `\n`. The normalized string is moved into `sym.source_snippet`, and the function returns `true` on success.

Internally, the function relies on the fields of `SymbolInfo` (`source_snippet`, `source_snippet_length`, `source_snippet_offset`, `source_snippet_file_size`, `source_snippet_hash`, `declaration_location`), the file system API, and the helper `hash_source_snippet_bytes`. The mutex striping mechanism (`source_snippet_mutex_for`) reduces contention when multiple symbols in the same file are resolved concurrently.

#### Side Effects

- modifies the `source_snippet` member of the provided `SymbolInfo`
- acquires a mutex for thread safety
- reads from the on-disk file
- potentially allocates memory for the snippet string

#### Reads From

- sym`.declaration_location``.file`
- sym`.source_snippet_length`
- sym`.source_snippet_offset`
- sym`.source_snippet_file_size`
- sym`.source_snippet_hash`
- the file at the recorded path

#### Writes To

- sym`.source_snippet`

#### Usage Patterns

- Called to populate the `source_snippet` field of a `SymbolInfo` after extraction
- Used in symbol resolution pipelines to lazily load source text from disk

### `clore::extract::split_top_level_qualified_name`

Declaration: `extract/model.cppm:57`

Definition: `extract/model.cppm:265`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first performs an early return if the input `qualified_name` is empty. It then consults a thread‑safe cache obtained via `split_qualified_name_cache()`: under a shared lock it looks up the precomputed parts; a hit returns the cached `std::vector<std::string>` immediately. On a miss, the function parses the qualified name character by character. It maintains a `template_depth` counter to correctly handle `::` that appear inside template argument lists (`<…>`), so that only scope‑resolution `operator`s at depth zero split the string. Consecutive colons (`::`) trigger a push of the current segment (if non‑empty) and skip the second colon. After the loop, the trailing segment is added, and any leading empty segment (caused by a leading `::`) is removed. Finally, the result is inserted into the cache under a unique lock; if the cache has grown beyond `kSplitQualifiedNameCacheMaxEntries` it is cleared to bound memory usage.

#### Side Effects

- acquires shared lock and later unique lock on `split_qualified_name_cache().mutex`
- reads from `split_qualified_name_cache().parts_by_qualified_name` map
- writes to `split_qualified_name_cache().parts_by_qualified_name` map (insertion or assignment)
- may clear the entire cache map if its size reaches `kSplitQualifiedNameCacheMaxEntries`

#### Reads From

- parameter `qualified_name`
- global cache `split_qualified_name_cache().parts_by_qualified_name`
- global cache mutex `split_qualified_name_cache().mutex` (for shared lock)

#### Writes To

- global cache `split_qualified_name_cache().parts_by_qualified_name` map
- global cache mutex `split_qualified_name_cache().mutex` (for unique lock)

#### Usage Patterns

- called during symbol name resolution to obtain top-level name components
- used internally to cache repeated splitting of the same qualified name
- employed in preprocessing steps for module or symbol identification

### `clore::extract::symbol_kind_name`

Declaration: `extract/model.cppm:26`

Definition: `extract/model.cppm:244`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::symbol_kind_name` maps each enumerator of `clore::extract::SymbolKind` to a corresponding string literal and returns it as a `std::string_view`. The implementation consists of a single `switch` statement covering all enumeration cases, including `SymbolKind::Unknown` for unrecognized values. A fallback `return "unknown"` at the end ensures safety if the enumeration is extended without updating the switch. The function has no external dependencies beyond the definition of `SymbolKind` and performs no additional computation or branching.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `kind` parameter

#### Usage Patterns

- converting symbol kind to display name
- serialization of symbol kind
- debug output

## Internal Structure

The `extract:model` module defines the core data structures and query interface for representing C++20 code extraction results. It decomposes the extraction domain into a layered record hierarchy: base types (`SourceLocation`, `SourceRange`) and core identifiers (`SymbolID`, `SymbolKind`) underpin richer entities (`SymbolInfo`, `FileInfo`, `NamespaceInfo`, `ModuleUnit`), which are aggregated into the top‑level `ProjectModel`. Accessor functions (`find_symbol`, `find_module_by_name`, `find_modules_by_name`, `lookup_symbol`, `find_symbols`, `find_module_by_source`) provide lookup by symbol identity, qualified‑name parts, or source location, while `resolve_source_snippet` and `namespace_prefix_from_qualified_name` offer deferred‑loading and decomposition utilities.

Internally, the module imports `std` for containers and algorithms and `support` for foundational utilities. An anonymous namespace encapsulates mutable global caches (`SplitQualifiedNameCache`, a source‑snippet mutex pool) and hashing primitives (`kSourceSnippetHashPrime`, `kSourceSnippetHashOffsetBasis`), together with the helper functions `hash_source_snippet_bytes` and `source_snippet_mutex_for`. This layering keeps shared, mutable state private while exposing a flat public API that operates on the model‑level types.

## Related Pages

- [Module support](../support/index.md)

