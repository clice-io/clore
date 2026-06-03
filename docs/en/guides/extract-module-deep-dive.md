---
title: 'Extract Module Deep Dive'
description: 'Guide: Extract Module Deep Dive'
layout: doc
template: doc
---

# Extract Module Deep Dive

**Namespace**: `clore::extract`  
**Source files**: `src/extract/*.cppm`  
**Purpose**: Parse C++ source code using Clang tooling, build a semantic `ProjectModel` with all symbols, files, namespaces, modules, and their relationships.

## Architecture

The extract module is the first major pipeline stage after configuration. It takes a `TaskConfig`, loads the compilation database, scans and filters files, parses each translation unit's AST, merges results, and produces a `ProjectModel`.

```
TaskConfig
    ↓
┌──────────────────┐
│  compiler.cppm   │  Load and normalize compilation database
│  (load_compdb)   │
└──────────────────┘
    ↓  CompilationDatabase
┌──────────────────┐
│  filter.cppm     │  Apply include/exclude path filters
└──────────────────┘
    ↓  Filtered CompilationDatabase
┌──────────────────┐
│  scan.cppm       │  Dependency graph construction (include scanning)
│  (build_dependency_graph_async) │
└──────────────────┘
    ↓  DependencyGraph + ScanCache
┌──────────────────┐
│  ast.cppm        │  Parallel AST extraction via Clang tooling
│  (extract_symbols) │
└──────────────────┘
    ↓  ASTResult[] (symbols + relations per file)
┌──────────────────┐
│  merge.cppm      │  Merge symbols across files, build indices
│  (rebuild_model_indexes) │
└──────────────────┘
    ↓
┌──────────────────┐
│  model.cppm      │  Final ProjectModel assembly, module info,
│                  │  lookup maps, source snippet resolution
└──────────────────┘
    ↓  ProjectModel
```

## Key Components

### `compiler.cppm` — Compilation Database Handling

**Key types:**
- `CompileEntry` — A single compilation database entry: file, directory, arguments, normalized paths, cache key, source hash
- `CompilationDatabase` — Collection of `CompileEntry` with optional toolchain cache
- `CompDbError` — Error type for compilation database loading failures

**Key functions:**
- `load_compdb(path)` → Loads `compile_commands.json`, normalizes entries
- `normalize_entry_file(entry)` → Canonicalizes the source file path
- `build_compile_signature(entry)` → Computes a hash of compiler arguments for cache invalidation
- `ensure_cache_key(entry)` → Ensures each entry has a unique cache key
- `create_compiler_instance(entry)` → Creates a Clang compiler instance for AST parsing

### `filter.cppm` — Path Filtering

**Key types:**
- `PathResolveError` — Error type for path resolution
- `FilterRule` (from `clore::config`) — Include/exclude patterns

**Key functions:**
- `filter_root_path(config)` → Determines the root directory for filtering
- `matches_filter(path, filter, root)` → Checks if a path matches include/exclude rules
- `resolve_path_under_directory(path, dir)` → Resolves relative paths against a base directory
- `canonical_graph_path(path)` → Produces a canonical path string for the dependency graph

### `scan.cppm` — Dependency Scanning

**Key types:**
- `DependencyGraph` — Files and directed edges (includes, imports)
- `DependencyEdge` — Single edge with from/to file paths
- `ScanResult` — Module name, interface flag, module imports, includes
- `ScanCache` — Persistent cache of scan results keyed by cache key
- `ScanError` — Error type

**Key functions:**
- `build_dependency_graph_async(db, graph, cache, loop)` — Core async function that scans all files for includes and module dependencies
- `scan_file(entry)` — Scans a single file for preprocessor directives (includes) and module declarations
- `scan_module_decl(content, result)` — Fast text-based scan for `module` / `export module` / `import` declarations
- `topological_order(graph)` — Computes a valid file processing order

### `ast.cppm` — AST Extraction

**Key types:**
- `ASTResult` — Symbols, relations, and dependencies from a single AST parse
- `ExtractedRelation` — Relationship between two symbols (inheritance, call, reference)
- `ASTError` — Error type

**Key function:**
- `extract_symbols(entry)` → Runs Clang's libtooling on a single translation unit, producing all declared symbols and their inter-relationships. This is the most expensive operation.

### `merge.cppm` — Symbol Merging

**Key functions:**
- `merge_symbol_info(target, source)` — Merges two copies of the same symbol (from different translation units), combining relationship vectors
- `rebuild_model_indexes(config, model)` — Rebuilds file, namespace, and parent-child indices after all symbols are merged
- `rebuild_lookup_maps(model)` — Builds the `symbol_ids_by_qualified_name` and `module_name_to_sources` lookup maps

### `model.cppm` — Data Structures

**Key types:**
- `SymbolID` — Unique identifier for a symbol (64-bit hash + signature string)
- `SymbolInfo` — All metadata for a single symbol (see architecture overview)
- `SourceLocation` — File/line/column location
- `SourceRange` — Begin/end location pair
- `FileInfo` — File path and its includes
- `NamespaceInfo` — Namespace name, symbols, child namespaces
- `ModuleUnit` — Module name, source file, interface flag, imports, symbols
- `ProjectModel` — Top-level container (see architecture overview)
- `SymbolKind` — Enum: Unknown, Namespace, Class, Struct, Union, Enum, `EnumMember`, Function, Method, Variable, Field, `TypeAlias`, Macro, Template, Concept

**Key functions:**
- `find_symbol(model, qualified_name)` — Looks up a symbol by qualified name
- `find_symbols(model, qualified_name)` — Looks up all symbols matching a qualified name (supports overloads)
- `find_module_by_name/model(model, name)` — Module lookup
- `resolve_source_snippet(symbol)` — On-demand resolution of source code snippet from file

## Caching Strategy

The extract module uses a two-level cache:

1. **Scan cache** (`ScanCache`): Caches `ScanResult` (includes + module declarations) per file. Invalidated when compile signature or source hash changes.

2. **AST cache** (`CacheRecord`): Caches the full `ASTResult` (symbols + relations). Includes a `DependencySnapshot` to detect when transitive dependencies have changed, requiring re-extraction.

Cache records are persisted to disk between runs and loaded/stored by the main `extract_project_async` function.

## Performance Characteristics

- **Dependency scanning** is fast (text-based, no compilation)
- **AST extraction** is the bottleneck — runs in parallel across files using worker threads
- **Symbol merging** is O(symbols × relations) and uses concurrent hash tables
- Memory is managed via `reserve()` calls to pre-allocate space for estimated symbol counts

