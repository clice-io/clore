---
title: 'Clore Architecture Overview'
description: 'Guide: Clore Architecture Overview'
layout: doc
template: doc
---

# Clore Architecture Overview

## What is Clore?

**Clore** is a C++ code documentation generator that uses Large Language Models (`LLMs`) to produce AI-assisted documentation for C++ codebases. It parses C++ source code using Clang-based tooling, builds a semantic model of the project, organizes content into pages, sends contextual evidence to an LLM for analysis, and renders the results as structured Markdown documentation.

## High-Level Pipeline

```
[Source Code] 
     ↓
┌─────────────────────┐
│   1. CONFIG         │  TOML-based configuration, validation, normalization
└─────────────────────┘
     ↓
┌─────────────────────┐
│   2. EXTRACT        │  Compilation DB loading, dependency scanning, AST parsing,
│                     │  symbol merging, cache management
└─────────────────────┘
     ↓        ProjectModel (symbols, files, namespaces, modules)
┌─────────────────────┐
│   3. PLAN           │  Page enumeration (file/module, namespace, index pages),
│                     │  topological sorting, path conflict validation
└─────────────────────┘
     ↓        PagePlanSet (ordered page plans)
┌─────────────────────┐
│   4. ANALYZE        │  For each page, build evidence packs → LLM prompt →
│                     │  parse structured analysis → cache results
└─────────────────────┘
     ↓        SymbolAnalysisStore (function/type/variable analyses)
┌─────────────────────┐
│   5. RENDER         │  Markdown document generation with frontmatter,
│                     │  cross-reference links, diagrams (Mermaid)
└─────────────────────┘
     ↓        GeneratedPage[] (markdown content)
┌─────────────────────┐
│   6. WRITE          │  Write pages to output directory
└─────────────────────┘
```

## Core Modules (C++20)

| Module | Namespace | Purpose |
|--------|-----------|---------|
| `config` | `clore::config` | TOML-based task configuration (load, normalize, validate, schema) |
| `extract` | `clore::extract` | C++ code analysis via Clang; builds `ProjectModel` |
| `generate` | `clore::generate` | Page planning, LLM analysis, markdown rendering |
| `network` | `clore::net` | LLM provider abstraction (Anthropic, `OpenAI`), HTTP, protocol |
| `agent` | `clore::agent` | Agent mode for iterative tool-calling documentation |
| `support` | `clore::support` | Utilities: file I/O, UTF-8, transparent string hashing, topological sort |

## Key Data Structures

### `clore::extract::ProjectModel`
The central output of the extraction phase. Contains all symbols, files, namespaces, and module metadata from the analyzed C++ project.

- **`symbols`**: `unordered_map<SymbolID, SymbolInfo>` — all parsed symbols
- **`files`**: `unordered_map<string, FileInfo>` — file-level info with includes
- **`namespaces`**: `unordered_map<string, NamespaceInfo>` — namespace hierarchy
- **`modules`**: `unordered_map<string, ModuleUnit>` — C++20 module units
- **`file_order`**: topological order of files
- **`symbol_ids_by_qualified_name`** & **`module_name_to_sources`**: lookup indices

### `clore::extract::SymbolInfo`
Represents a single C++ symbol (function, class, variable, etc.) with:
- Identity (`SymbolID` with hash + signature)
- Location (declaration + optional definition)
- Relationships (parent, children, bases, derived, calls, `called_by`, references)
- Documentation (doc comment, source snippet)

### `clore::generate::PagePlanSet`
The output of the planning phase. Contains:
- **`plans`**: all `PagePlan`s for files/modules, namespaces, and index
- **`generation_order`**: topological order for sequential generation

Each `PagePlan` has a `page_type`, `title`, `relative_path`, `owner_keys`, `depends_on_pages`, and `prompt_requests`.

### `clore::generate::SymbolAnalysisStore`
Stores the results of LLM analysis per symbol category:
- **`functions`**: `FunctionAnalysis` (overview, details, side effects, reads/writes)
- **`types`**: `TypeAnalysis` (overview, details, invariants, key members)
- **`variables`**: `VariableAnalysis` (overview, details, mutation, usage)

## Key Design Patterns

### 1. Pipeline with Coroutines
The entire pipeline uses C++20 coroutines (`kota::task<T, E>`) for async operations. Each phase returns a typed task that can `co_await` on the event loop.

### 2. Extensive Caching
- **Extract cache**: Compilation signatures, AST results, dependency snapshots
- **Generate cache**: Symbol analysis responses, page summary texts
- Cache keys are derived from file paths + compile signatures + source hashes

### 3. LLM Provider Abstraction
The `clore::net` module defines a `Protocol` template parameter approach. Both `clore::net::anthropic` and `clore::net::openai` implement the same interface (build request JSON, parse responses, read environment). Capability probing auto-detects which features each provider supports.

### 4. Evidence-Based Prompting
Before sending an LLM request, the system builds an `EvidencePack` containing:
- Symbol facts (name, kind, signature, location, doc comments)
- Source code snippets
- Dependency context (callers/callees)
- Related page summaries
This ensures the LLM has sufficient context to generate accurate documentation.

### 5. Two Output Modes
- **Header-based**: One page per source/header file
- **Module-based**: One page per C++20 module unit (when `uses_modules` is true)

## Directory Layout

```
src/
├── main.cpp                  # Entry point
├── agent/                    # Agent mode (agent.cppm, tools.cppm)
├── config/                   # Configuration (load, normalize, schema, validate)
├── extract/                  # C++ code extraction
│   ├── extract.cppm          # Main extraction orchestrator
│   ├── ast.cppm              # Clang AST parsing
│   ├── cache.cppm            # Extract cache
│   ├── compiler.cppm         # Compilation database handling
│   ├── filter.cppm           # Path filtering
│   ├── merge.cppm            # Symbol merging
│   ├── model.cppm            # Data model (SymbolInfo, ProjectModel, etc.)
│   └── scan.cppm             # Dependency scanning
├── generate/                 # Documentation generation
│   ├── generate.cppm         # Top-level generate functions
│   ├── analysis.cppm         # LLM response parsing
│   ├── cache.cppm            # Analysis cache
│   ├── dryrun.cppm           # Dry run estimation
│   ├── evidence.cppm         # Evidence building declarations
│   ├── evidence_builder.cppm # Evidence building implementation
│   ├── markdown.cppm         # Markdown AST and rendering
│   ├── model.cppm            # Page plans, analysis store
│   ├── planner.cppm          # Page planning
│   ├── scheduler.cppm        # Async scheduling of LLM calls
│   └── render/
│       ├── common.cppm       # Shared render utilities
│       ├── diagram.cppm      # Mermaid diagram generation
│       ├── page.cppm         # Page-level rendering
│       └── symbol.cppm       # Symbol-level rendering
├── network/                  # LLM network layer
│   ├── network.cppm          # High-level API
│   ├── client.cppm           # Provider-agnostic client
│   ├── http.cppm             # HTTP transport + rate limiting
│   ├── protocol.cppm         # Request/response protocol
│   ├── provider.cppm         # Provider detection
│   ├── schema.cppm           # JSON schema validation
│   ├── anthropic.cppm        # Anthropic provider
│   └── openai.cppm           # OpenAI provider
└── support/
    └── logging.cppm          # Utilities + logging
```

