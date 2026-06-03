---
title: 'Generate Module Deep Dive'
description: 'Guide: Generate Module Deep Dive'
layout: doc
template: doc
---

# Generate Module Deep Dive

**Namespace**: `clore::generate`  
**Source files**: `src/generate/*.cppm`, `src/generate/render/*.cppm`  
**Purpose**: Plan documentation pages, orchestrate LLM-based analysis, render results as Markdown.

## Architecture

The generate module takes a `ProjectModel` (from the extract phase) and produces a set of `GeneratedPage` markdown documents. It has three sub-phases: **Plan**, **Analyze**, and **Render**.

```
ProjectModel (from extract)
    ↓
┌────────────────────────────┐
│  PLANNER (planner.cppm)    │  Enumerate pages, compute dependencies,
│  build_page_plan_set()     │  topological sort, validate paths
└────────────────────────────┘
    ↓  PagePlanSet
┌────────────────────────────┐
│  SCHEDULER (scheduler.cppm)│  For each page plan (in order):
│  generate_pages_async()    │
│                            │
│  ┌──────────────────────┐  │
│  │  EVIDENCE BUILDER    │  │  Build EvidencePack from ProjectModel
│  │  evidence_builder    │  │  (symbol facts, snippets, context)
│  └──────────────────────┘  │
│         ↓                  │
│  ┌──────────────────────┐  │
│  │  LLM ANALYSIS        │  │  Call LLM via clore::net, parse response
│  │  analysis.cppm       │  │  Store in SymbolAnalysisStore
│  └──────────────────────┘  │
│         ↓                  │
│  ┌──────────────────────┐  │
│  │  RENDER              │  │  Build MarkdownDocument with frontmatter,
│  │  render/*.cppm       │  │  cross-references, diagrams, symbol sections
│  └──────────────────────┘  │
└────────────────────────────┘
    ↓  std::vector<GeneratedPage>
┌────────────────────────────┐
│  WRITE (generate.cppm)     │  Write pages to output directory
└────────────────────────────┘
```

## Sub-Modules

### `planner.cppm` — Page Planning

**Key types:**
- `PagePlan` — A single page to generate: type, title, path, owner keys, dependencies, prompt requests
- `PagePlanSet` — All plans plus a topological generation order
- `PlanError` — Error type
- `PageType` — Enum: `Index`, `Namespace`, `Module`, `File`
- `PromptRequest` — A single LLM analysis request: kind, target key, output contract
- `PromptKind` — Enum: `NamespaceSummary`, `ModuleSummary`, `ModuleArchitecture`, `IndexOverview`, `FunctionAnalysis`, `TypeAnalysis`, `FunctionDeclarationSummary`, `FunctionImplementationSummary`, `TypeDeclarationSummary`, `TypeImplementationSummary`, `VariableAnalysis`

**Key functions:**
- `build_page_plan_set(config, model)` → Main entry point. Enumerates pages in three steps:
  1. **Content pages**: Either per-file (header-based) or per-module (module-based)
  2. **Namespace pages**: One per namespace with child symbols
  3. **Index page**: Root page linking everything
- `validate_no_path_conflicts(entries)` → Ensures no two pages map to the same output path
- `compute_page_path(identity)` → Builds the relative output path for a page identity

### `scheduler.cppm` — Async Orchestration

**Key functions:**
- `generate_pages_async(config, model, llm_model, rate_limit, output_root, loop)` → Top-level coroutine that:
  1. Calls `build_page_plan_set()` to plan pages
  2. Calls `build_link_resolver()` to create cross-reference mappings
  3. Iterates through pages in topological order
  4. For each page: builds evidence, calls LLM, parses response, renders markdown
  5. Caches analysis results for reuse

### `evidence.cppm` + `evidence_builder.cppm` — Evidence Construction

**Key types:**
- `EvidencePack` — Complete context for a single LLM analysis request:
  - `page_id`, `prompt_kind`, `subject_name`, `subject_kind`
  - `target_facts` (symbol facts), `local_context`, `dependency_context`
  - `reverse_usage_context`, `source_snippets`
  - `related_page_summaries`
- `SymbolFact` — Key facts about a symbol: name, qualified name, kind, signature, access, template params, location, doc comment
- `PromptError` — Error type

**Key functions:**
- `build_evidence_for_function_analysis(model, target, project_root)` → Builds evidence for a function analysis prompt
- `build_evidence_for_type_analysis(model, target, project_root)` → Builds evidence for a type analysis prompt
- `build_evidence_for_variable_analysis(model, target, project_root)` → Builds evidence for a variable analysis prompt
- `build_evidence_for_namespace_summary(model, analyses, target, project_root)` → Builds evidence for a namespace summary
- `build_evidence_for_module_summary(...)` → Builds evidence for a module summary
- `build_evidence_for_index_overview(model, analyses)` → Builds evidence for the index page
- `format_evidence_text(pack)` → Renders an `EvidencePack` as structured text for the LLM prompt
- `build_prompt(kind, pack)` → Constructs the full LLM prompt string

### `analysis.cppm` — LLM Response Parsing

**Key functions:**
- `parse_structured_response(raw, target_key)` → Parses an LLM response into a structured analysis object
- `normalize_markdown_fragment(raw, target_key)` → Cleans and normalizes markdown output
- `parse_markdown_prompt_output(raw, target_key)` → Parses markdown sections from LLM output
- `build_symbol_analysis_prompt(model, prompt_kind, target_key, analyses, config)` → Builds the prompt for symbol-level analysis
- `apply_symbol_analysis_response(analyses, model, prompt_kind, target_key, response)` → Applies parsed analysis back to the store
- `store_fallback_analysis(analyses, model, target_key)` → Creates a basic analysis when LLM calls fail

### `markdown.cppm` — Markdown AST

**Key types:**
- `MarkdownNode` — Variant-based node: `TextFragment`, `CodeFragment`, `LinkFragment`, `Paragraph`, `ListItem`, `BulletList`, `CodeFence`, `MermaidDiagram`, `RawMarkdown`, `BlockQuote`, `SemanticSection`, `MarkdownDocument`
- `MarkdownDocument` — Top-level document with `Frontmatter` (title, description, layout, template) and children
- `SemanticSection` — Named section with heading, level, subject key, kind, and children
- `Frontmatter` — YAML frontmatter: title, description, layout, `page_template`
- `SemanticKind` — Enum: `Index`, `Namespace`, `Module`, `Type`, `Function`, `Variable`, `File`, `Section`

**Key functions:**
- `make_text(s)`, `make_code(s)`, `make_link(text, target, code_style)` → Inline fragment constructors
- `make_paragraph(s)`, `make_blockquote(s)`, `make_code_fence(code, lang)`, `make_mermaid(code)` → Block-level constructors
- `make_section(kind, heading, subject, level, omit_if_empty, code_style_heading)` → Section constructor
- `render_markdown(doc)` → Renders the Markdown AST to a string

### `render/` — Page and Symbol Rendering

**Key files:**
- **`common.cppm`** — Shared render utilities: link resolution, source location rendering, symbol analysis sections, implementation page discovery
- **`symbol.cppm`** — Symbol-level rendering: doc groups, type member sections, page layout
- **`page.cppm`** — Page-level rendering: namespace pages, module pages, file pages, index pages
- **`diagram.cppm`** — Mermaid diagram generation: import diagrams, module dependency diagrams, namespace diagrams, file dependency diagrams

**Key functions:**
- `build_page_root(...)` → Builds the root for a namespace page
- `build_module_page_root(...)` → Builds the root for a module page
- `build_file_page_root(...)` → Builds the root for a file page
- `build_index_page_root(...)` → Builds the root for the index page
- `render_page_markdown(...)` → Full page rendering pipeline
- `write_page(page, output_root)` → Writes a rendered page to disk

### `cache.cppm` — Analysis Cache

**Key types:**
- `AnalysisCacheEntry` — Cached analysis result: prompt kind, target key, response markdown, timestamp

**Key functions:**
- `lookup(request)` → Looks up a cached analysis result
- `store(request, response)` → Stores a new analysis result
- Cache invalidation is based on compile signature changes (from the extract phase)

### `dryrun.cppm` — Dry Run Mode

**Key functions:**
- `generate_dry_run(config, model)` → Simulates generation without calling the LLM, estimating request counts and costs
- `build_request_estimate_page(config, model, plan_set)` → Produces an estimate page listing all planned LLM requests
- `build_dry_run_page_summary_texts(config, plan_set)` → Generates approximate summary texts for cache key computation

### `generate.cppm` — Public API

**Key functions:**
- `generate_pages(config, model, llm_model, rate_limit, output_root)` → Synchronous wrapper
- `generate_pages_async(config, model, llm_model, rate_limit, output_root, loop)` → Async entry point
- `generate_dry_run(config, model)` → Dry run entry point
- `write_pages(pages, output_root)` → Writes all generated pages to disk

## Key Data Flow

```
1. ProjectModel → build_page_plan_set() → PagePlanSet
2. For each PagePlan:
   a. Determine PromptKind from page type
   b. build_evidence_for_*(model, target, project_root) → EvidencePack
   c. build_prompt(kind, pack) → prompt string
   d. call_llm_async(system_prompt, prompt, ...) → raw response
   e. parse_structured_response(raw_response) → analysis
   f. Store analysis in SymbolAnalysisStore
   g. Render page using analysis data → GeneratedPage
3. write_pages(pages, output_root) → disk
```

## Caching Strategy

- **Symbol analysis cache**: Stores LLM responses keyed by (prompt kind, target key). Avoids re-querying the LLM when the underlying source code hasn't changed.
- **Page summary cache**: Used during dry run to estimate costs without full generation.
- Cache keys derive from compile signatures in the extract cache, ensuring cache invalidation when source code changes.

