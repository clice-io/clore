---
title: 'Module extract:ast'
description: 'The extract:ast module is responsible for extracting structured symbol and relation information from C++ source code by traversing the Clang AST. It implements internal visitor classes (SymbolExtractorVisitor, SymbolExtractorConsumer, SymbolExtractorAction) that walk declarations, expressions, and references to compute unique symbol identifiers, capture source snippet bounds, and record edges such as calls, inheritance, and references. Additional helpers handle lexical context description, doc comment extraction, function signature building, and dependency file collection.'
layout: doc
template: doc
---

# Module `extract:ast`

## Summary

The `extract:ast` module is responsible for extracting structured symbol and relation information from C++ source code by traversing the Clang AST. It implements internal visitor classes (`SymbolExtractorVisitor`, `SymbolExtractorConsumer`, `SymbolExtractorAction`) that walk declarations, expressions, and references to compute unique symbol identifiers, capture source snippet bounds, and record edges such as calls, inheritance, and references. Additional helpers handle lexical context description, doc comment extraction, function signature building, and dependency file collection.  

Publicly, the module exposes the `extract_symbols` function as the entry point for triggering extraction from a given resource, and two result types: `ASTResult` (which holds extracted symbols, relations, and dependencies) and `ASTError` (which encapsulates extraction failure details). Together these provide a complete interface for analyzing a single translation unit and producing a structured representation of its declarations and their interrelationships.

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

The struct `clore::extract::ASTError` is an aggregate error type with a single public data member `message` of type `std::string`. Internally, the struct imposes no invariants beyond those inherited from `std::string`; the `message` field may be empty or contain any valid character sequence. The struct has no user‑declared constructors, destructor, or assignment `operator`s, so the compiler implicitly generates a default constructor, copy/move constructors, and copy/move assignment `operator`s. No member functions or friend definitions are present, making `clore::extract::ASTError` a trivial wrapper used solely to convey an error string across the extraction boundary.

#### Key Members

- message

#### Usage Patterns

- Thrown as an exception when AST extraction encounters an error.
- Caught by callers to inspect the error message and handle failure.

### `clore::extract::ASTResult`

Declaration: `extract/ast.cppm:37`

Definition: `extract/ast.cppm:37`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::ASTResult` serves as a simple aggregate container for the output of AST extraction. Internally, it holds three public member vectors: `symbols` of type `std::vector<SymbolInfo>`, `relations` of type `std::vector<ExtractedRelation>`, and `dependencies` of type `std::vector<std::string>`. No invariants beyond those implied by the types are enforced; each vector may be empty or contain well-formed elements depending on the extraction context. The struct relies on the implicitly defined default constructor, destructor, and copy/move operations, making it trivially copyable and movable. There are no additional member functions or access controls, so all fields are directly modifiable. This design is intentionally lightweight, acting as a straightforward transfer object between the extraction logic and its consumers.

#### Invariants

- No explicit invariants documented beyond the type being an aggregate.

#### Key Members

- symbols
- relations
- dependencies

#### Usage Patterns

- Returned as the result type from AST extraction functions.
- Consumed by downstream processes that analyze or transform the extracted data.

### `clore::extract::ExtractedRelation`

Declaration: `extract/ast.cppm:30`

Definition: `extract/ast.cppm:30`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::ExtractedRelation` is an aggregate type that stores a single directed relation between two symbols. It holds two `SymbolID` members, `from` and `to`, which identify the source and target of the relation. Two boolean flags, `is_call` and `is_inheritance`, encode the relation kind; the implementation defaults both to `false` (via in-class initializers), meaning a default-constructed `ExtractedRelation` represents no specific edge type. The only implied invariant is that the flags can be used independently—when `is_inheritance` is `true`, `from` denotes the derived type and `to` the base type. There are no explicit constructors or member functions; the struct relies on default member initializers and aggregate initialization, making it a plain data carrier with trivial copy and move semantics.

#### Invariants

- `from` and `to` are valid `SymbolID` values.
- At most one of `is_call` or `is_inheritance` may be true? Not specified; both can be false or true.

#### Key Members

- `from`
- `to`
- `is_call`
- `is_inheritance`

#### Usage Patterns

- Used as part of the extraction output to record symbol relationships.
- Inspected to determine call or inheritance dependencies.

## Functions

### `clore::extract::extract_symbols`

Declaration: `extract/ast.cppm:43`

Definition: `extract/ast.cppm:669`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::extract_symbols` orchestrates a full Clang-based extraction pass over a single translation unit. After validating that the `CompileEntry` provides a non-empty argument list and successfully constructing a compiler instance, the function forces the frontend into `ParseSyntaxOnly` mode—ensuring no output files are generated—and then drives the `SymbolExtractorAction` through the standard `BeginSourceFile`, `Execute`, and `EndSourceFile` lifecycle. This action, initialized with references to `result.symbols` and a local `raw_relations` vector, populates those containers during traversal via the `SymbolExtractorVisitor`’s `VisitNamedDecl`, `VisitDeclRefExpr`, `VisitCallExpr`, `VisitMemberExpr`, and recursion on `TraverseFunctionDecl` / `TraverseCXXMethodDecl`.

After extraction completes, dependencies are collected using `collect_dependency_files` from the `SourceManager`. Finally, every `RelationEdge` in `raw_relations` is mapped to an `ExtractedRelation`, translating the `RelationKind::Call` and `RelationKind::Inheritance` enumerators into boolean flags. The resulting `ASTResult` is returned, containing `symbols`, `relations`, and `dependencies`; any errors during instance creation or action execution propagate as an `ASTError`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- const `CompileEntry` &entry
- entry`.arguments`
- entry`.file`

#### Usage Patterns

- called to extract AST symbols and relations for a single compile entry
- used in vectorized or async extraction flows

## Internal Structure

The `extract:ast` module forms the core AST analysis layer of the extraction pipeline. It depends on `extract:compiler` for compilation‑entry data and on `extract:model` for the output data structures (`ASTResult`, `ASTError`, `ExtractedRelation`). The module also imports the standard library and the `support` utility module. Its public entry point is `extract_symbols`, which accepts a resource identifier and triggers extraction; the result is an `int` status code.

Internally, the module is decomposed into Clang‑based AST traversal machinery, all contained in an anonymous namespace. A `SymbolExtractorVisitor` (a `RecursiveASTVisitor`) traverses the AST, recording symbol metadata via `VisitNamedDecl`, `TraverseFunctionDecl`, and other overrides, and building dependency edges (`RelationEdge`) for calls, references, and inheritance through dedicated methods such as `VisitCallExpr`, `VisitDeclRefExpr`, and `VisitMemberExpr`. The visitor is driven by a `SymbolExtractorConsumer` that receives the translation unit and orchestrates the traversal. A `SymbolExtractorAction` (an `ASTFrontendAction`) wraps the consumer and exposes the extracted symbol and relation vectors to the caller. Supporting utilities—`compute_symbol_id`, `classify_decl`, `describe_lexical_context`, `get_source_snippet_bounds`, `hash_source_snippet_bytes`, and dependency‑file collection—are implemented as free functions in the same namespace, providing a clean internal layering that isolates Clang‑specific logic from the public API.

## Related Pages

- [Module extract:compiler](compiler.md)
- [Module extract:model](model.md)
- [Module support](../support/index.md)

