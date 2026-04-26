---
title: 'Module extract:model'
description: 'The extract:model module defines the core data structures and query functions that represent the extracted structure of a C++ project. It provides a unified model encompassing symbols (SymbolInfo, SymbolID, SymbolKind), source locations and ranges (SourceLocation, SourceRange), module units (ModuleUnit), file metadata (FileInfo), namespace hierarchies (NamespaceInfo), and the top-level project container (ProjectModel). These types capture declaration/definition positions, symbol relationships (parent, children, references, calls), qualified name decomposition, and on‑demand source‑snippet resolution.'
layout: doc
template: doc
---

# Module `extract:model`

## Summary

The `extract:model` module defines the core data structures and query functions that represent the extracted structure of a C++ project. It provides a unified model encompassing symbols (`SymbolInfo`, `SymbolID`, `SymbolKind`), source locations and ranges (`SourceLocation`, `SourceRange`), module units (`ModuleUnit`), file metadata (`FileInfo`), namespace hierarchies (`NamespaceInfo`), and the top-level project container (`ProjectModel`). These types capture declaration/definition positions, symbol relationships (parent, children, references, calls), qualified name decomposition, and on‑demand source‑snippet resolution.

The public interface includes lookup functions to retrieve symbols by ID or qualified name, locate modules by name or source file, resolve source snippets from disk, decompose and join qualified names, and obtain a stable integer representation for symbol kind names. Helper utilities such as `SymbolID` comparison and validity checks, `SourceLocation::is_known`, and caching mechanisms for qualified‑name splitting are also exposed. Collectively, the module owns the abstractions needed to store, navigate, and query the complete extraction result.

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

The struct `clore::extract::FileInfo` is a plain aggregate type that bundles the core data produced by the extraction phase for a single source file. Its three public fields — `path`, `symbols`, and `includes` — hold the file’s location string, a vector of `SymbolID` values representing the symbols found in that file, and a vector of include path strings. Because `clore::extract::FileInfo` has no user‑defined constructors, assignment `operator`s, or access controls, it behaves as a simple value type that can be initialized using aggregate initialization or designated initializers.

No invariants are enforced by the struct beyond those inherited from its member types (e.g., `std::string` and `std::vector` are always in a valid state). The `path` field is expected to contain a valid filesystem path after extraction, and the `symbols` and `includes` vectors may be empty if no symbols or includes were encountered. All fields are directly writable, so downstream code can freely modify the data once the struct is populated.

#### Invariants

- `path` should be a valid filesystem path
- `symbols` may be empty
- `includes` may be empty

#### Key Members

- `path`
- `symbols`
- `includes`

#### Usage Patterns

- Used as output of extraction to represent a translation unit
- Consumed by downstream processing that expects symbol and include lists

### `clore::extract::ModuleUnit`

Declaration: `extract/model.cppm:135`

Definition: `extract/model.cppm:135`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::ModuleUnit` is an aggregate data type that holds the extracted information for a single C++20 module unit, whether an interface unit or a partition. Its fields record the module's fully qualified `name` (e.g., `"foo"` or `"foo:bar"`), a boolean `is_interface` that distinguishes exported (`export module`) from non‑exported (`module`) units, and a normalized `source_file` path. The `imports` vector lists the names of modules imported by this unit, while the `symbols` vector stores the identifiers of all symbols declared within it. The structure imposes no invariants beyond standard well‑formedness: `name` should be non‑empty, and `symbols` may be empty if no declarations were encountered.

#### Invariants

- `is_interface` is true for export module declarations
- `name` is the fully qualified module name
- `source_file` is a normalized file path
- `imports` lists all module imports
- `symbols` contains all symbol `IDs` declared in this unit

#### Key Members

- `name`
- `is_interface`
- `source_file`
- `imports`
- `symbols`

#### Usage Patterns

- Populated by a module parser or extractor
- Gathered into a collection to represent an entire module's translation units
- Iterated over to analyze or serialize module structure

### `clore::extract::NamespaceInfo`

Declaration: `extract/model.cppm:128`

Definition: `extract/model.cppm:128`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::NamespaceInfo` is an aggregate data structure that captures the extracted information for a single namespace scope. Its member `name` stores the unqualified namespace name, while `children` holds a list of direct child namespace names (as strings) that appear within this namespace. All symbol definitions directly belonging to this namespace are collected in `symbols`, a vector of `SymbolID` values. The order of elements in `symbols` and `children` reflects the order of declaration or discovery during extraction, but no invariant guarantees uniqueness or sorting. The struct’s simplicity allows it to be default‑constructed, copied, or moved using compiler‑generated operations, and it imposes no constraints on the content of its members beyond those inherent in the contained standard library types.

#### Invariants

- All fields are public and can be freely modified.
- No invariants are enforced by the type itself; correctness depends on external usage.

#### Key Members

- `name`
- `symbols`
- `children`

#### Usage Patterns

- Used as a data container to represent a parsed namespace hierarchy.
- Instances are aggregated to form the full namespace tree during extraction.

### `clore::extract::ProjectModel`

Declaration: `extract/model.cppm:143`

Definition: `extract/model.cppm:143`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The `ProjectModel` struct serves as the central aggregation of all extracted project information. Its core data members are `std::unordered_map` containers: `symbols` maps `SymbolID` to `SymbolInfo`; `files` maps normalized file paths to `FileInfo`; `namespaces` maps qualified namespace names to `NamespaceInfo`; `modules` maps normalized source file paths to `ModuleUnit`; `symbol_ids_by_qualified_name` maps a qualified name to a `std::vector<SymbolID>` (to handle overloads); and `module_name_to_sources` maps a module name to the list of source files that declare it. The `file_order` member stores file paths in the sequence they were processed, preserving the extraction traversal order. Several of these maps—`modules`, `symbol_ids_by_qualified_name`, and `module_name_to_sources`—use `clore::support::TransparentStringHash` and `clore::support::TransparentStringEqual`, enabling heterogeneous lookup with `std::string_view` to avoid unnecessary allocations during queries. The `uses_modules` boolean is set to `true` if any C++20 module declaration has been encountered, serving as a flag for downstream processing stages that need to handle module‑specific logic.

#### Invariants

- `uses_modules` is true iff at least one module declaration exists
- `symbol_ids_by_qualified_name` may contain multiple `IDs` for overloaded names
- `modules` keys are normalized source file paths
- `module_name_to_sources` keys are exact module names
- `file_order` matches the order in which files were processed

#### Key Members

- `symbols`
- `files`
- `namespaces`
- `modules`
- `symbol_ids_by_qualified_name`
- `uses_modules`

#### Usage Patterns

- Used by generation and evidence building passes for qualified name lookup
- Used for cross-linking via module name lookup
- Provides access to all extracted symbols, files, and namespaces

### `clore::extract::SourceLocation`

Declaration: `extract/model.cppm:64`

Definition: `extract/model.cppm:64`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct holds a source location as three fields: a `file` path string, a `line` number, and a `column` number. The invariant is that a `line` value of `0` represents an unknown or invalid location; valid source lines start at `1`. This is enforced by the default initialization of `line` to `0`, and the `is_known()` member simply checks `line != 0`. The `column` field has no such sentinel; its semantics depend on the caller.

#### Invariants

- `line == 0` indicates the location is unknown
- Valid source lines start at `1`
- `is_known()` returns `false` only when `line == 0`

#### Key Members

- `file` (string)
- `line` (`uint32_t`)
- `column` (`uint32_t`)
- `is_known()` method

#### Usage Patterns

- Used to capture source positions in extraction results
- Checked for validity via `is_known()`

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

The struct `clore::extract::SourceRange` is a simple aggregate type that holds two `SourceLocation` members, `begin` and `end`. Its design purpose is to represent a contiguous source region by capturing the start and end locations. The primary invariant is that `begin` denotes the first location of the range and `end` the one‑past‑the‑last location, so the range is valid only if `begin <= end` (according to the ordering defined by `SourceLocation`). No special member functions are declared or need to be defined; the compiler‑generated default operations suffice for its role as a lightweight value‑type pair.

#### Invariants

- The `begin` and `end` members define the start and end of the range.
- No ordering or validity guarantees are specified in the evidence.

#### Key Members

- `begin`: the starting `SourceLocation` of the range.
- `end`: the ending `SourceLocation` of the range.

#### Usage Patterns

- Used as a field in other parsing or extraction structures to store the source location of a parsed construct.
- Returned by functions that produce a range covering a parsed token or node.

### `clore::extract::SymbolID`

Declaration: `extract/model.cppm:28`

Definition: `extract/model.cppm:28`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The internal state of `clore::extract::SymbolID` is a pair of unsigned integers: `hash` (a `std::uint64_t`) and `signature` (a `std::uint32_t`). The design enforces the invariant that a `hash` value of zero represents the invalid or null sentinel; all valid identifiers have a nonzero `hash`. The `signature` field stores an additional digest derived from the same USR source, serving to disambiguate the astronomically unlikely event of a 64‑bit hash collision. The member `is_valid()` simply tests whether `hash` is nonzero. Both `operator==` and `operator<=>` are defaulted, meaning they perform member‑wise comparison following declaration order (first `hash`, then `signature`). This gives `SymbolID` a total order and ensures that equality and three‑way comparison account for both the primary hash and the collision‑detection signature.

#### Invariants

- Valid `IDs` have non-zero hash.
- Zero hash represents invalid/null sentinel.
- Equality and ordering are based on both hash and signature.

#### Key Members

- `hash` field
- `signature` field
- `is_valid()` method
- `operator==`
- `operator<=>`

#### Usage Patterns

- Used as a key or identifier for symbols in extraction logic.
- Defaulted comparisons enable use in associative containers.
- Collision disambiguation relies on the signature field.

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

The struct `clore::extract::SymbolInfo` aggregates all extracted metadata about a single symbol. Its fields capture the symbol’s identity (`id`, `name`, `qualified_name`, `enclosing_namespace`), kind (`kind`), signature, and access specifier (`access`), along with its declaration and optional definition `SourceLocation`. Human‑readable documentation (`doc_comment`) and a source snippet (`source_snippet`) are stored directly; when the snippet is empty, the triad `source_snippet_offset`, `source_snippet_length`, `source_snippet_file_size`, and `source_snippet_hash` reference the raw source text in the file indicated by `declaration_location`, enabling deferred loading to reduce memory pressure. The structure models the symbol tree via `parent`, `children`, `lexical_parent_name`, and `lexical_parent_kind`, and tracks inheritance through `bases` and `derived`, call‑graph relations through `calls` and `called_by`, and general references through `references` and `referenced_by`. Template‑related state is captured by `is_template` and `template_params`. All collection fields store `SymbolID` values to maintain a lightweight cross‑reference graph without duplicating symbol data.

#### Invariants

- `id` uniquely identifies the symbol across an extraction session
- `source_snippet_offset`, `source_snippet_length`, `source_snippet_file_size`, and `source_snippet_hash` are consistent if `source_snippet` is empty
- `declaration_location` is always present; `definition_location` is optional
- `parent`, `children`, `bases`, `derived`, `calls`, `called_by`, `references`, `referenced_by` store relationships as `SymbolID` values that refer to other `SymbolInfo` instances

#### Key Members

- `id`
- `kind`
- `name`
- `qualified_name`
- `declaration_location`
- `parent` and `children`
- `bases` and `derived`
- `calls` and `called_by`
- `references` and `referenced_by`
- `source_snippet` and related offset fields
- `doc_comment`

#### Usage Patterns

- Populated by extraction tools to describe each discovered symbol
- Traversed by code analysis utilities to build dependency graphs or inheritance hierarchies
- Linked via `SymbolID` fields to form a forest of symbol trees
- Examined to generate cross-references, call graphs, or documentation

### `clore::extract::SymbolKind`

Declaration: `extract/model.cppm:8`

Definition: `extract/model.cppm:8`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The enumeration `clore::extract::SymbolKind` is implemented as a scoped enum with an explicit underlying type of `std::uint8_t`, ensuring it occupies exactly one byte. This compaction is intentional: it reduces memory overhead when storing many symbol kind values, for example in arrays, bit fields, or when serializing symbol data. The enumerators are ordered roughly by conceptual category – type-like symbols (`Namespace`, `Class`, `Struct`, `Union`, `Enum`) come first, followed by member and function-like entries (`EnumMember`, `Function`, `Method`, `Variable`, `Field`), then declaration-level forms (`TypeAlias`, `Macro`, `Template`, `Concept`), and finally the sentinel `Unknown`. The placement of `Unknown` as the last enumerator is an internal invariant: it enables range checks such as `kind < SymbolKind::Unknown` to test whether a symbol kind is a known value, and ensures that `Unknown` is the highest value, simplifying exhaustive switch‑statements or look‑up tables without requiring an explicit count constant. No additional member functions are provided; the enum relies solely on implicit conversion to `std::uint8_t` for direct use in numeric contexts.

#### Invariants

- Each enumerator has a distinct integer value assigned by the compiler.
- The `Unknown` enumerator serves as a default or error indicator.
- The enum is intended to be used as a discriminator in variant-like structures or as a label in symbol metadata.

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

- Other code compares against these enumerators to determine the kind of a symbol.
- The enum is likely used in switch statements or lookup tables.
- It may be serialized or compared with equality.

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

The function first delegates to `clore::extract::find_modules_by_name` to obtain a vector of candidate `ModuleUnit*` pointers for the given `module_name`. If the result is empty, it returns `nullptr`; if it contains exactly one element, that element is returned directly. When multiple candidates exist, the function iterates over them, counting those where `ModuleUnit::is_interface` is true. If exactly one interface unit is found, it is returned. If none of the candidates are interface units, a warning is emitted and the first candidate is returned as a fallback. If more than one interface unit exists, a warning about ambiguity is issued and `nullptr` is returned. The algorithm relies on the structure of `ModuleUnit` and the helper `find_modules_by_name`, and uses `logging::warn` for diagnostic output.

#### Side Effects

- Logs warning messages via `logging::warn` when multiple modules match and disambiguation fails or when only implementation units exist.

#### Reads From

- `model` (const `ProjectModel`&)
- `module_name` (`std::string_view`)
- `modules` result of `find_modules_by_name`
- `ModuleUnit::is_interface` member

#### Writes To

- Logging subsystem (through `logging::warn`)

#### Usage Patterns

- Used by callers to resolve a module name to a single unambiguous module unit
- Typically invoked during project extraction or symbol lookup when a module context is needed

### `clore::extract::find_module_by_source`

Declaration: `extract/model.cppm:194`

Definition: `extract/model.cppm:449`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function performs a direct lookup in the `model.modules` map using the provided `source_file` as the key. If an entry is found, it returns a pointer to the corresponding `ModuleUnit`; otherwise it returns `nullptr`. The entire implementation is a single map find operation—no iteration, special handling, or fallback logic is involved.

Its only dependency is the `ProjectModel` type, which must expose the `modules` member as an associative container keyed by source file paths (e.g., `std::unordered_map<std::string, clore::extract::ModuleUnit>`). The return is a non‑owning pointer into that container, so the caller must ensure the `model` outlives the returned pointer.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- model`.modules`
- `source_file`

#### Usage Patterns

- used to retrieve a module unit by its source file path
- likely called when mapping a source location to its containing module

### `clore::extract::find_modules_by_name`

Declaration: `extract/model.cppm:191`

Definition: `extract/model.cppm:395`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::find_modules_by_name` performs a lookup in the `model.module_name_to_sources` map using the given `module_name`. If the key is absent or the associated source list is empty, an empty vector is returned immediately. Otherwise, it iterates over each source file in that list, calling `find_module_by_source(model, source)` to retrieve the corresponding `ModuleUnit` pointer; only non-null pointers are retained. The resulting modules are then sorted in ascending order by their `source_file` string to provide a deterministic ordering. Finally, the sorted vector is returned.

The algorithm depends on `clore::extract::ProjectModel` providing a pre‑built mapping from module names to their source file identifiers, and on `clore::extract::find_module_by_source` mapping a source file identifier to a `clore::extract::ModuleUnit`. Sorting uses the `source_file` field of `ModuleUnit`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- model`.module_name_to_sources`
- model`.module_name_to_sources`[`module_name`]
- `ModuleUnit::source_file` (via sort lambda)
- result of `find_module_by_source`

#### Usage Patterns

- Used to retrieve all `ModuleUnit` pointers with a specific module name.
- Called when multiple sources define the same module name.

### `clore::extract::find_symbol`

Declaration: `extract/model.cppm:179`

Definition: `extract/model.cppm:371`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::find_symbol` is a convenience wrapper around `find_symbols`. It delegates the actual lookup to `find_symbols`, which performs name resolution across the `ProjectModel`’s files, namespaces, and modules using the provided `qualified_name`. After obtaining the collection of matching `SymbolInfo*` pointers, the function checks whether the collection contains exactly one element. If the match count is not exactly one—either because no symbol matches or multiple symbols share the same qualified name—it returns `nullptr` to signal ambiguity or absence. Otherwise, it returns the single candidate from the front of the collection. This design centralizes ambiguity handling and ensures callers receive a definitive result or a clear failure indication without additional processing.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model`
- `qualified_name`

#### Usage Patterns

- Resolve a qualified name to a unique symbol
- Lookup symbol by fully qualified name

### `clore::extract::find_symbol`

Declaration: `extract/model.cppm:181`

Definition: `extract/model.cppm:379`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::find_symbol` handles the common case of resolving an overloaded or templated symbol by both its qualified name and its signature string. The function first checks whether the `signature` parameter is empty; if so, it delegates entirely to the single‑argument overload `find_symbol(model, qualified_name)`, which performs a straightforward name lookup.

When a non‑empty signature is provided, the function calls `find_symbols(model, qualified_name)` to obtain a flat container of candidate `const SymbolInfo*` pointers whose qualified name matches. It then performs a linear scan over these candidates, comparing each symbol’s `signature` field against the given signature using `operator==`. On the first exact match the corresponding pointer is returned; if no candidate satisfies the signature constraint, the function returns `nullptr`. This two‑stage approach separates name‑based filtering from signature‑based disambiguation, relying on `find_symbols` for the initial lookup and on the `SymbolInfo::signature` member for final identification.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model`
- `qualified_name`
- `signature`
- `model` symbol lookup structures

#### Usage Patterns

- Searching for a symbol by qualified name and signature
- Obtaining a symbol pointer for further inspection

### `clore::extract::find_symbols`

Declaration: `extract/model.cppm:185`

Definition: `extract/model.cppm:354`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::find_symbols` performs a direct lookup in the `model.symbol_ids_by_qualified_name` map (a member of `ProjectModel`) using the provided `qualified_name`. If the key is not found, an empty vector is returned immediately. Otherwise, the function reserves capacity for the expected number of matches and iterates over the stored `SymbolID` values, calling `lookup_symbol(model, symbol_id)` for each. Only valid `SymbolInfo` pointers (non-null results from the lookup) are appended to the result vector. This design decouples the name-to-ID mapping from the ID-to-info resolution, allowing multiple symbols to share the same qualified name (e.g., overloaded functions) while relying on `lookup_symbol` to translate the semantic identifier into the concrete metadata.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- model`.symbol_ids_by_qualified_name`
- model (via `lookup_symbol`)

#### Usage Patterns

- retrieve all symbols matching a qualified name

### `clore::extract::join_qualified_name_parts`

Declaration: `extract/model.cppm:59`

Definition: `extract/model.cppm:328`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::join_qualified_name_parts` iterates over a `std::vector<std::string>` of qualified name parts, joining the first `count` elements with the scope resolution `operator` `"::"`. Internally, it first clamps `count` to the vector’s actual size via a `safe_count` variable to avoid out‑of‑range access. A simple loop then appends each part to a result string, inserting the separator before every element after the first. The final assembled `std::string` is returned. The implementation depends only on standard library types (`std::vector`, `std::string`, `std::size_t`) and contains no calls to other `clore::extract` utilities.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `parts` (const `std::vector<std::string>`&)
- `count` (`std::size_t`)

#### Usage Patterns

- reconstructing fully qualified symbol names from component parts
- building qualified names for lookup or display

### `clore::extract::lookup_symbol`

Declaration: `extract/model.cppm:177`

Definition: `extract/model.cppm:349`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function performs a direct lookup in the `model.symbols` associative container using the given `SymbolID`. It calls `find` and checks the returned iterator against `end`; if a match is found, it returns a pointer to the corresponding `SymbolInfo` object, otherwise it returns `nullptr`. There is no additional fallback logic or nested search—the symbol map is the sole authority for symbol identity within the project model. The entire operation relies solely on the `find` method of the container and the equality semantics of `SymbolID`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model` parameter
- `model.symbols`
- `id` parameter

#### Usage Patterns

- retrieving symbol details for a specific ID
- checking existence of a symbol
- used by code generation or analysis passes

### `clore::extract::namespace_prefix_from_qualified_name`

Declaration: `extract/model.cppm:62`

Definition: `extract/model.cppm:341`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function delegates its work to two internal utilities: `split_top_level_qualified_name` and `join_qualified_name_parts`. It first uses `split_top_level_qualified_name` to decompose `qualified_name` into a sequence of name segments. If the resulting list contains one or fewer parts, the qualified name has no namespace component, so an empty string is returned immediately. Otherwise, it calls `join_qualified_name_parts` on all segments except the last one, effectively discarding the innermost (leaf) name and reconstructing the prefix. This prefix is returned as the namespace portion. The approach is linear and relies solely on the two helper functions; no additional model lookups or state are involved.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `qualified_name` parameter (`std::string_view`)

#### Usage Patterns

- Used to derive the namespace scope of a qualified name
- Called during symbol extraction to separate namespace from name

### `clore::extract::resolve_source_snippet`

Declaration: `extract/model.cppm:200`

Definition: `extract/model.cppm:455`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first acquires a per-symbol mutex via `source_snippet_mutex_for` to serialize reads for the same `SymbolInfo`. It returns early with `true` if `source_snippet` is already populated (cached). The core algorithm then validates that `source_snippet_length` is non‑zero and that `declaration_location.file` is not empty; failures cause a `false` return. After computing the file size via `std::filesystem::file_size`, it performs three integrity checks: the optional `source_snippet_file_size` must match the actual file size (if non‑zero), and the `source_snippet_offset` and `source_snippet_length` must lie within the file bounds. A binary `std::ifstream` then seeks to the stored offset and reads exactly `source_snippet_length` bytes. If the symbol’s `source_snippet_hash` is non‑zero, the read bytes are hashed via `hash_source_snippet_bytes` and compared; a mismatch also returns `false`. Finally, the raw bytes are normalized by stripping `\r` before `\n` (to handle Windows line endings) and the result is moved into `sym.source_snippet`, returning `true` to indicate success.

Key dependencies include `std::filesystem::file_size`, `std::ifstream` binary I/O, `hash_source_snippet_bytes` (an internal hashing utility), and `source_snippet_mutex_for` (which uses a striped mutex pool indexed by symbol pointer). The function tightly relies on `SymbolInfo` fields such as `source_snippet`, `source_snippet_offset`, `source_snippet_length`, `source_snippet_file_size`, `source_snippet_hash`, and `declaration_location.file`.

#### Side Effects

- Acquires a mutex (synchronization)
- Reads bytes from a file on disk (I/O)
- Modifies the <code>`source_snippet`</code> member of the <code>`SymbolInfo`</code> parameter (state mutation)
- Allocates dynamic memory for the snippet string

#### Reads From

- <code>sym`.source_snippet`</code>
- <code>sym`.source_snippet_offset`</code>
- <code>sym`.source_snippet_length`</code>
- <code>sym`.declaration_location``.file`</code>
- <code>sym`.source_snippet_file_size`</code>
- <code>sym`.source_snippet_hash`</code>
- the on-disk file at <code>sym`.declaration_location``.file`</code>

#### Writes To

- <code>sym`.source_snippet`</code>

#### Usage Patterns

- Called to lazily load a symbol's source text after extraction
- Invoked during symbol display or search result rendering
- Used to populate the snippet view in documentation or code navigation

### `clore::extract::split_top_level_qualified_name`

Declaration: `extract/model.cppm:57`

Definition: `extract/model.cppm:265`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::split_top_level_qualified_name` first handles the trivial case of an empty input by returning an empty vector. It then performs a thread‑safe lookup in a per‑process cache obtained via `clore::extract::(anonymous namespace)::split_qualified_name_cache()`, using a shared lock on the cache’s `mutex`; if the `qualified_name` is already present in the map `parts_by_qualified_name`, the cached result is returned immediately. On a cache miss, the function manually splits the qualified name on `::` delimiters while tracking template depth (incrementing on `<`, decrementing on `>`). Colons are only treated as separators when the template depth is zero, preventing false splits inside template arguments. After the loop, any leading empty segment (produced by a leading `::`) is removed. Finally, a unique lock is acquired to update the cache; if the cache size exceeds `kSplitQualifiedNameCacheMaxEntries`, the entire cache is cleared before inserting the new entry. The parsed vector of name parts is then returned.

#### Side Effects

- modifies the global cache of split qualified names returned by `split_qualified_name_cache()`
- acquires and releases shared and unique locks on the cache mutex
- clears the cache if it exceeds `kSplitQualifiedNameCacheMaxEntries` entries

#### Reads From

- parameter `qualified_name`
- global cache `split_qualified_name_cache().parts_by_qualified_name`
- `kSplitQualifiedNameCacheMaxEntries` constant

#### Writes To

- `split_qualified_name_cache().parts_by_qualified_name` map
- a local `parts` vector that is returned

#### Usage Patterns

- parse qualified names for scope decomposition
- used by other extract functions needing top-level name parts
- cached to optimize repeated calls with the same qualified name

### `clore::extract::symbol_kind_name`

Declaration: `extract/model.cppm:26`

Definition: `extract/model.cppm:244`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::symbol_kind_name` is a straightforward mapping function that converts a `SymbolKind` enumerator into a human‑readable `std::string_view`. The algorithm relies solely on a `switch` statement that exhaustively covers all members of the `SymbolKind` enum (including `Namespace`, `Class`, `Struct`, `Union`, `Enum`, `EnumMember`, `Function`, `Method`, `Variable`, `Field`, `TypeAlias`, `Macro`, `Template`, `Concept`, and `Unknown`). Each case returns a fixed string literal corresponding to the kind; the default branch returns the string `"unknown"` as a fallback. The function has no internal control flow beyond the switch, and its only dependency is the `SymbolKind` enum itself, defined within the same module. It returns `std::string_view` to avoid copying and to provide a lightweight reference to the constant string data.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `kind`

#### Usage Patterns

- Used for logging or display of symbol kinds
- Obtain a string representation of a `SymbolKind`

## Internal Structure

The `extract:model` module is the core data‑model layer of the extraction pipeline. It decomposes the result of static analysis into three principal aggregates: `SymbolInfo` (identity, kind, location, relationships), `ModuleUnit` (C++20 module metadata, imports, symbol membership), and `ProjectModel` (top‑level container aggregating files, symbols, namespaces, and module units). Supporting value types (`SourceLocation`, `SourceRange`, `SymbolID`, `FileInfo`, `NamespaceInfo`) and the `SymbolKind` enumeration complete the model. All public types and free functions reside in `clore::extract`; the module imports `std` for containers and `support` for foundational utilities such as path normalization and hashing.

Internally, the module is organized around a flat public interface – query functions (`lookup_symbol`, `find_module_by_name`, `find_symbols`, etc.) operate directly on `ProjectModel` – while a private, unnamed‑namespace section houses caches and helpers. Chief among these is `SplitQualifiedNameCache`, a thread‑safe LRU‑like structure backed by a mutex‑protected map, used to avoid re‑parsing qualified name strings during repeated queries. The `resolve_source_snippet` function illustrates the module’s lazy‑resolution pattern: it reads snippet text from disk on demand, using a stripe of mutexes (`kSourceSnippetMutexStripeCount`) to minimise contention. The implementation avoids deep inheritance or layered abstractions; instead, it provides a stable, directly‑queryable snapshot of the extracted project, optimised for read‑only access after the extraction pass completes.

## Related Pages

- [Module support](../support/index.md)

