---
title: 'Module extract:ast'
description: 'The extract:ast module is responsible for translating a C++ translation unit into a structured representation of symbols and their relationships. It owns the public entry point extract_symbols, which consumes a compilation entry identifier and returns either an ASTResult (containing discovered symbols, relations, and dependency files) or an ASTError on failure. The module drives Clang’s AST front‑end with custom visitors (SymbolExtractorVisitor, SymbolExtractorConsumer) that walk function, method, call, member, and declaration nodes to collect symbol metadata and edges (calls, inheritance, references). Internally, it computes source snippets, lexical context information, and symbol identifiers, and it deduplicates relations using a hash set. The public API surfaces only the extracted data structures and the extraction function, while all traversal and analysis logic remains hidden in an anonymous namespace.'
layout: doc
template: doc
---

# Module `extract:ast`

## Summary

The `extract:ast` module is responsible for translating a C++ translation unit into a structured representation of symbols and their relationships. It owns the public entry point `extract_symbols`, which consumes a compilation entry identifier and returns either an `ASTResult` (containing discovered symbols, relations, and dependency files) or an `ASTError` on failure. The module drives Clang’s AST front‑end with custom visitors (`SymbolExtractorVisitor`, `SymbolExtractorConsumer`) that walk function, method, call, member, and declaration nodes to collect symbol metadata and edges (calls, inheritance, references). Internally, it computes source snippets, lexical context information, and symbol identifiers, and it deduplicates relations using a hash set. The public API surfaces only the extracted data structures and the extraction function, while all traversal and analysis logic remains hidden in an anonymous namespace.

## Imports

- [`extract:compiler`](compiler.md)
- [`extract:model`](model.md)
- [`support`](../support/index.md)

## Imported By

- [`extract:cache`](cache.md)

## Types

### `clore::extract::ASTError`

Declaration: `src/extract/ast.cppm:49`

Definition: `src/extract/ast.cppm:49`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The single data member `message` stores an arbitrary error description and is the sole state of the type. No custom constructors, destructors, or assignment `operator`s are defined, so the compiler generates default versions that copy, move, or destroy `message` according to the usual rules for `std::string`. The struct carries no additional invariants beyond those of its member; a default-constructed `ASTError` contains an empty `message`, and after any move or copy the contents follow `std::string` semantics. Because the structure is trivial, its implementation consists entirely of the implicit special member functions and the inline representation of `std::string`.

#### Invariants

- `message` may be empty or non-empty; no further constraints are documented.

#### Key Members

- `message`

#### Usage Patterns

- Used as the value type in `std::expected` or similar error-handling patterns within extraction code.
- Likely returned or thrown to communicate extraction failures to callers.

### `clore::extract::ASTResult`

Declaration: `src/extract/ast.cppm:60`

Definition: `src/extract/ast.cppm:60`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::ASTResult` is an aggregate type that bundles the three principal output collections produced by the AST extraction phase. Its internal state consists solely of three `std::vector` members: `symbols` holds instances of `SymbolInfo`, `relations` holds `ExtractedRelation` objects, and `dependencies` stores `std::string` entries. No invariants are enforced beyond those inherent to the vector types; the struct serves purely as a flat container for the extracted data, with all fields left default‑initialised. Important member implementations are trivial – the compiler‑generated default constructor, destructor, and copy/move operations suffice, so no custom special members are provided.

#### Invariants

- Fields are initialized as empty vectors by default.
- All vectors are populated during extraction and are not modified afterward.

#### Key Members

- `symbols`
- `relations`
- `dependencies`

#### Usage Patterns

- Returned by extraction functions as the main output.
- Iterated over by consumers to process symbols, relations, and dependencies.

### `clore::extract::ExtractedRelation`

Declaration: `src/extract/ast.cppm:53`

Definition: `src/extract/ast.cppm:53`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::ExtractedRelation` models a directed edge between two symbols. Its internal structure consists of two `SymbolID` fields, `from` and `to`, that respectively identify the source and target of the relation, and two boolean flags, `is_call` and `is_inheritance`, that classify the relationship type. The default member initializers set both flags to `false`, making the relation’s kind unspecified until explicitly stated. An important invariant is that at most one flag should be set at a time: `is_call` indicates a call edge, while `is_inheritance` indicates an inheritance edge in which `from` is the derived type and `to` is the base type. As an aggregate type, `ExtractedRelation` supports direct memberwise initialization, and no user‑defined constructors or special member functions are provided, relying on the compiler‑generated defaults for construction and assignment.

#### Invariants

- The relation is directed from `from` to `to`.
- At most one of `is_call` or `is_inheritance` may be true at a time, but this is not enforced by the type.

#### Key Members

- `from`
- `to`
- `is_call`
- `is_inheritance`

#### Usage Patterns

- Used as an element in collections of extracted symbol relationships.
- Checked downstream to classify edges for graph construction or analysis.

## Functions

### `clore::extract::extract_symbols`

Declaration: `src/extract/ast.cppm:66`

Definition: `src/extract/ast.cppm:692`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::extract_symbols` first validates that the given `CompileEntry` contains non-empty arguments, returning an `ASTError` with a descriptive message if validation fails. It then creates a compiler instance via `create_compiler_instance`; if that fails, an error is returned. To ensure the compilation is treated purely as semantic analysis, the function forces extraction‑only mode by modifying the frontend options: setting `ProgramAction` to `clang::frontend::ParseSyntaxOnly`, and clearing `OutputFile` and `ModuleOutputPath`. A `SymbolExtractorAction` is constructed with references to the `ASTResult::symbols` container and a local vector of `RelationEdge` objects. The action drives the AST extraction through `BeginSourceFile`, `Execute`, and `EndSourceFile`; any execution error is consumed via `llvm::consumeError` and results in an `ASTError`. On success, `collect_dependency_files` populates `result.dependencies`. Finally, each `RelationEdge` in `raw_relations` is converted into an `ExtractedRelation`, mapping `RelationKind::Call` to `is_call` and `RelationKind::Inheritance` to `is_inheritance`, and stored in `result.relations`.

#### Side Effects

- Reads the source file and its transitive includes
- Creates a Clang compiler instance (may load toolchain data)
- Allocates memory for the returned `ASTResult`

#### Reads From

- `entry` parameter (`const CompileEntry&`)
- Source files and headers via the Clang compiler instance

#### Writes To

- Returned `ASTResult` object containing extracted symbols and relations

#### Usage Patterns

- Called after loading a compilation database and sanitizing arguments
- Used to extract per-translation-unit symbol information for a project

## Internal Structure

The `extract:ast` module implements the actual Clang AST traversal that produces the extracted symbol and relation data. Its top-level public entry point is `extract_symbols`, which accepts a compilation entry index. Underneath, the module is decomposed into several closely related internal components: `SymbolExtractorAction` creates a `SymbolExtractorConsumer` per translation unit; the consumer, in turn, instantiates a `SymbolExtractorVisitor` that recursively walks the AST. The visitor overrides Clang’s `RecursiveASTVisitor` hooks—`TraverseFunctionDecl`, `TraverseCXXMethodDecl`, `VisitNamedDecl`, `VisitCallExpr`, `VisitDeclRefExpr`, `VisitMemberExpr`—to capture symbol definitions, call edges, inheritance relations, and cross-references.

All extraction logic lives within an anonymous namespace, which enforces a strong encapsulation boundary. Supporting helper functions such as `compute_symbol_id`, `classify_decl`, `build_function_signature`, `print_template_parameters`, `describe_lexical_context`, `get_source_snippet_bounds`, and `edge_hash` handle ID generation, type classification, signature formatting, lexical scoping, source-snippet extraction, and relation deduplication. The module imports `extract:model` for its public output types (`ASTResult`, `ExtractedRelation`), `extract:compiler` for compilation context, and `support` for utilities at its internal use.

## Related Pages

- [Module extract:compiler](compiler.md)
- [Module extract:model](model.md)
- [Module support](../support/index.md)

