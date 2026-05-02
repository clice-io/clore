---
title: 'Module generate:dryrun'
description: 'The generate:dryrun module is responsible for constructing and providing dry‑run documentation pages that preview the output of the generation pipeline without actually invoking AI models. It estimates the number and types of AI requests that would be produced (per symbol category such as variables, functions, types, modules, and namespaces), assembles textual summaries for each page, and builds dedicated pages for LLMs content and request‑estimate overviews. The module also supplies a cache key function to uniquely identify page summaries for reuse.'
layout: doc
template: doc
---

# Module `generate:dryrun`

## Summary

The `generate:dryrun` module is responsible for constructing and providing dry‑run documentation pages that preview the output of the generation pipeline without actually invoking AI models. It estimates the number and types of AI requests that would be produced (per symbol category such as variables, functions, types, modules, and namespaces), assembles textual summaries for each page, and builds dedicated pages for `LLMs` content and request‑estimate overviews. The module also supplies a cache key function to uniquely identify page summaries for reuse.

Publicly, the module owns four functions: `build_dry_run_page_summary_texts` which generates pre‑formatted summary fragments for a dry‑run page, `build_request_estimate_page` which constructs the page that details the estimated request counts, `page_summary_cache_key_for_request` which returns a deterministic integer key for caching page summaries, and `build_llms_page` which builds a page specifically for LLM‑related documentation. Internally, the module uses helper types like `RequestEstimate` and `LabeledPage` along with functions such as `estimate_request_count` and `project_name_from_config` to compute and structure the dry‑run output. It depends on configuration, extraction, analysis, and model modules to obtain the project data and prompt plans needed for the estimation.

## Imports

- [`config`](../config/index.md)
- [`extract`](../extract/index.md)
- [`generate:analysis`](analysis.md)
- [`generate:model`](model.md)
- `std`

## Imported By

- [`generate:scheduler`](scheduler.md)

## Functions

### `clore::generate::build_dry_run_page_summary_texts`

Declaration: `generate/dryrun.cppm:11`

Definition: `generate/dryrun.cppm:316`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function iterates over the provided `prompt_requests` and, for each request, checks whether `page_summary_cache_key_for_request` returns a value for the given `plan`. If no cache key exists, the request is skipped. For those that do yield a key, it calls `fallback_page_summary_for_request` to obtain a summary text; if that result is non-empty, the summary is inserted into the output map under a key produced by `prompt_request_key`. The final map contains only requests that have both a cacheable key and a non‑empty fallback summary.

The control flow is a linear scan with two filtering steps: absence of a cache key and an empty fallback both cause the request to be ignored. The function depends on three helpers from the same namespace—`page_summary_cache_key_for_request`, `fallback_page_summary_for_request`, and `prompt_request_key`—and relies on the `PagePlan` and `PromptRequest` types for the input parameters. No external side effects are performed; the result is returned as a new `std::unordered_map`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan`
- `prompt_requests`
- `page_summary_cache_key_for_request` result
- `fallback_page_summary_for_request` result

#### Usage Patterns

- called during dry-run page generation to produce summary key-value pairs

### `clore::generate::build_llms_page`

Declaration: `generate/dryrun.cppm:19`

Definition: `generate/dryrun.cppm:333`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::build_llms_page` constructs a `GeneratedPage` representing an `llms.txt` index file for the generated C++ reference. It first retrieves the project name via `project_name_from_config`, then builds a markdown string content beginning with a top-level heading, a description line, and a link to the API reference markdown file. If the `request_estimate_path` parameter is non‑empty, a link to a dry‑run request estimate page is appended. The remaining content is assembled by calling the helper `append_llms_section` three times with `PageType` values `Module`, `Namespace`, and `File`, each receiving the same `plan_set` and `config`. Finally, the function returns a `GeneratedPage` with the constructed `title`, the fixed `relative_path` `"llms.txt"`, and the assembled `content`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan_set`
- `config`
- `request_estimate_path`
- `project_name_from_config(config)`

#### Writes To

- local `content` string
- returned `GeneratedPage` object

#### Usage Patterns

- called as part of the page generation pipeline to produce the `LLMs` overview file

### `clore::generate::build_request_estimate_page`

Declaration: `generate/dryrun.cppm:15`

Definition: `generate/dryrun.cppm:230`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function retrieves an estimate by calling `estimate_request_count` with the provided `plan_set` and `model`, and obtains the project name via `project_name_from_config`. It then constructs the `content` string in a fixed order: first embedding a YAML front matter block with title, description, layout, and page template; then printing the project name and a summary section listing the total, page, and symbol prompt tasks from the returned `RequestEstimate`. The following sections break down page‑level tasks (namespace summaries, module summaries and architecture prompts, index overviews) and symbol‑level tasks, using the estimate’s fields such as `function_symbols`, `type_symbols`, `variable_symbols` and their associated request counters for analysis and summary generation. The function finally returns a `GeneratedPage` with a static title, a fixed relative path of `"request-estimate.md"`, and the assembled `content`. No branching or iteration occurs beyond the sequential formatting steps; the primary dependency is the accurate computation of the `RequestEstimate` struct by the helper function `estimate_request_count`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan_set` parameter
- `model` parameter
- `config` parameter
- result of `estimate_request_count(plan_set, model)`
- result of `project_name_from_config(config)`

#### Usage Patterns

- called during dry run generation to produce the estimate page
- likely invoked by `clore::generate::generate_dry_run`

### `clore::generate::page_summary_cache_key_for_request`

Declaration: `generate/dryrun.cppm:23`

Definition: `generate/dryrun.cppm:293`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function determines a cache key for a page summary based on the request kind. It checks the `request.kind` via a switch statement. For `PromptKind::NamespaceSummary` and `PromptKind::ModuleSummary`, it attempts to retrieve the first element from `plan.owner_keys` as the cache key; if empty, it returns `std::nullopt`. For all other request kinds—`ModuleArchitecture`, `IndexOverview`, `FunctionAnalysis`, `TypeAnalysis`, `VariableAnalysis`, `FunctionDeclarationSummary`, `FunctionImplementationSummary`, `TypeDeclarationSummary`, and `TypeImplementationSummary`—it unconditionally returns `std::nullopt`, indicating no cache key. A fallback return after the switch also yields `std::nullopt`. The logic depends on the `PagePlan` type (specifically its `owner_keys` member) and the `PromptRequest` type with its `PromptKind` enum, but performs no further computation or external calls.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.owner_keys`
- `request.kind`

#### Usage Patterns

- Used to derive a cache key for page summaries, particularly for namespace and module summaries.

## Internal Structure

The `generate:dryrun` module provides the dry‑run stage of the documentation generation pipeline, where it produces descriptive page content and cost estimates without performing the full asynchronous AI queries. It depends on `config` for project settings, `extract` for the parsed project model, `generate:analysis` for structured analysis of symbols, and `generate:model` for core data types like `PagePlan` and `PageType`. Internally, the module is layered around a `RequestEstimate` struct that accumulates counts for various request categories (module summaries, namespace pages, type/function analyses, etc.). Helper functions such as `estimate_request_count`, `project_name_from_config`, and `fallback_page_summary_for_request` compute these estimates and generate textual summaries based on the configuration and a `plan_set`. The public entry points – `build_dry_run_page_summary_texts`, `build_request_estimate_page`, `page_summary_cache_key_for_request`, and `build_llms_page` – use these internals to produce either a textual summary for a dry‑run page, a page‑estimate display, a cache key for repeated summaries, or a dedicated LLM page. This decomposition keeps request‑counting logic separate from page‑building logic and ensures that the module can be used both for interactive dry‑run feedback and for previewing the generated documentation structure.

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)
- [Module generate:analysis](analysis.md)
- [Module generate:model](model.md)

