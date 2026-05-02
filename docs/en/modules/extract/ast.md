---
title: 'Module extract:ast'
description: 'The extract:ast module is responsible for parsing C++ translation units via the Clang AST and extracting structured symbol, relation, and dependency information. It owns the public-facing result types ASTResult and ASTError, which convey either successfully extracted data (symbols, relations, dependencies) or an error condition. The primary entry point is the extract_symbols function, which accepts a source identifier and initiates the extraction pipeline. Internally, the module implements a set of Clang AST visitors and consumers that traverse declarations, references, calls, and member accesses to record symbol definitions, reference edges, call edges, and inheritance relationships, filtering results to the main file and its dependencies.'
layout: doc
template: doc
---

# Module `extract:ast`

## Summary

The `extract:ast` module is responsible for parsing C++ translation units via the Clang AST and extracting structured symbol, relation, and dependency information. It owns the public-facing result types `ASTResult` and `ASTError`, which convey either successfully extracted data (symbols, relations, dependencies) or an error condition. The primary entry point is the `extract_symbols` function, which accepts a source identifier and initiates the extraction pipeline. Internally, the module implements a set of Clang AST visitors and consumers that traverse declarations, references, calls, and member accesses to record symbol definitions, reference edges, call edges, and inheritance relationships, filtering results to the main file and its dependencies.

## Imports

- [`extract:compiler`](compiler.md)
- [`extract:model`](model.md)
- `std`
- [`support`](../support/index.md)

## Imported By

- [`extract:cache`](cache.md)

## Types

### `clore::extract::ASTError`

Declaration: `extract/ast.cppm:26`

Definition: `extract/ast.cppm:26`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The `clore::extract::ASTError` struct serves as a lightweight error wrapper, containing a single `std::string` field named `message`. Its design is intentionally minimal, with no additional state or invariants beyond the stored error description. The struct likely relies on the default special member functions provided by the compiler, as no user‑defined constructors, assignments, or destructor are declared. The `message` member holds the textual representation of the error, and the type as a whole acts as a plain container for propagating error information through the extraction process.

### `clore::extract::ASTResult`

Declaration: `extract/ast.cppm:37`

Definition: `extract/ast.cppm:37`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::ASTResult` is the aggregate container for the output of a single extraction pass. It holds three top‑level members: `symbols`, a `std::vector<SymbolInfo>` that records every extracted symbol; `relations`, a `std::vector<ExtractedRelation>` that captures connectivity among those symbols; and `dependencies`, a `std::vector<std::string>` listing the external module or header dependencies discovered during extraction. A fundamental invariant is that every `ExtractedRelation` in `relations` refers only to symbols present in `symbols` (by index or identifier), ensuring the result is self‑contained and can be used for downstream analysis without further lookups. No additional special members are defined, so the struct is trivially copyable and movable, relying on the default compiler‑generated operations.

### `clore::extract::ExtractedRelation`

Declaration: `extract/ast.cppm:30`

Definition: `extract/ast.cppm:30`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::ExtractedRelation` is a lightweight data container that pairs two symbol identifiers, stored in `from` and `to`, to form a directed relation between them. Two boolean members, `is_call` and `is_inheritance`, distinguish the relation kind. Neither flag is set by default, and the relation is considered untyped when both are `false`; at most one flag should normally be `true` to preserve the invariant that a single relation instance represents exactly one kind of edge. The struct holds no methods beyond those implicitly defined, and its members are directly accessible, serving as a plain transfer object within the extraction pipeline.

## Functions

### `clore::extract::extract_symbols`

Declaration: `extract/ast.cppm:43`

Definition: `extract/ast.cppm:669`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first validates the `CompileEntry` by checking for an empty argument list, returning an error if necessary. It then creates a compiler instance via `create_compiler_instance` and overrides the frontend options to force parse-only mode, preventing any object or module output. A `SymbolExtractorAction` is constructed, receiving references to `result.symbols` and a local vector of `RelationEdge` objects. The action is used to begin source file processing, execute the AST traversal, and end the source file; any execution failure is consumed and reported as an error. After extraction, dependency files are collected via `collect_dependency_files` and stored in `result.dependencies`. Finally, each `RelationEdge` in `raw_relations` is converted into an `ExtractedRelation` by mapping the `RelationKind` to the boolean fields `is_call` and `is_inheritance`, and the populated `ASTResult` is returned.

#### Side Effects

- Creates a compiler instance via `create_compiler_instance`, potentially invoking external toolchain or reading compilation database
- Runs clang frontend action `SymbolExtractorAction` which parses source files and performs semantic analysis
- Reads source files and collects dependency file paths via `collect_dependency_files`
- Mutates `result.symbols`, `result.dependencies`, and `result.relations`
- Consumes potential errors via `llvm::consumeError`, which may handle or log errors

#### Reads From

- `CompileEntry entry` (specifically `entry.arguments` and `entry.file`)
- Source files accessed through clang's `SourceManager`
- External toolchain or compilation database via `create_compiler_instance`

#### Writes To

- `ASTResult result` (fields `symbols`, `dependencies`, `relations`)
- Internal clang frontend state through `SymbolExtractorAction` methods

#### Usage Patterns

- Used to extract symbols and relations from a single compilation entry during project analysis
- Likely invoked in a loop over compile entries to build a complete project model

## Internal Structure

The module `extract:ast` implements the core C++ AST traversal and symbol extraction logic. Its internal decomposition centers on three key classes within an anonymous namespace: `SymbolExtractorVisitor`, which walks the Clang AST and records symbol definitions and relations such as calls, references, and inheritance; `SymbolExtractorConsumer`, which creates the visitor for a given translation unit; and `SymbolExtractorAction`, which wires the consumer into the Clang compiler frontend. Supporting free functions in the same namespace handle tasks like computing deterministic symbol identifiers, building function signatures, extracting documentation comments, and classifying declarations, keeping the visitor focused on traversal orchestration.

The module imports `std`, `support`, `extract:compiler`, and `extract:model`. This layering reflects a separation between lower-level compiler interaction (`extract:compiler` for compilation database support), shared data structures (`extract:model` for output types), and foundational utilities (`support` for hashing, paths, and logging). Internally, the anonymous namespace isolates implementation details; only the public entry point (the `extract_symbols` function) and the result types (`ASTResult`, `ASTError`) are exported. This structure ensures that the module’s core traversal logic remains self-contained while cooperating with the broader extraction pipeline through clearly defined data carriers.

## Related Pages

- [Module extract:compiler](compiler.md)
- [Module extract:model](model.md)
- [Module support](../support/index.md)

