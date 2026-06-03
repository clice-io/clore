---
title: 'Module generate:dryrun'
description: 'The generate:dryrun module provides a dry‑run preview of the documentation generation pipeline without writing output to disk. It constructs summary pages (e.g., LLM overview, request estimates) and generates summary texts for each page, enabling callers to inspect all derived content in memory. Public functions include build_llms_page, build_request_estimate_page, build_dry_run_page_summary_texts, and page_summary_cache_key_for_request. Internally, the module computes request estimates, derives project names from configuration, and assembles LLM entry labels for reporting. It depends on the config, extract, generate:analysis, and generate:model modules for configuration, symbol extraction, analysis prompts, and core data types.'
layout: doc
template: doc
---

# Module `generate:dryrun`

## Summary

The `generate:dryrun` module provides a dry‑run preview of the documentation generation pipeline without writing output to disk. It constructs summary pages (e.g., LLM overview, request estimates) and generates summary texts for each page, enabling callers to inspect all derived content in memory. Public functions include `build_llms_page`, `build_request_estimate_page`, `build_dry_run_page_summary_texts`, and `page_summary_cache_key_for_request`. Internally, the module computes request estimates, derives project names from configuration, and assembles LLM entry labels for reporting. It depends on the `config`, `extract`, `generate:analysis`, and `generate:model` modules for configuration, symbol extraction, analysis prompts, and core data types.

## Imports

- [`config`](../config/index.md)
- [`extract`](../extract/index.md)
- [`generate:analysis`](analysis.md)
- [`generate:model`](model.md)

## Imported By

- [`generate:scheduler`](scheduler.md)

## Functions

### `clore::generate::build_dry_run_page_summary_texts`

Declaration: `src/generate/dryrun.cppm:27`

Definition: `src/generate/dryrun.cppm:332`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

This function constructs a map from prompt‑request keys to their corresponding summary texts during a dry‑run simulation. It iterates over each element in the input `prompt_requests` vector, skipping any request for which `page_summary_cache_key_for_request` returns an empty optional (indicating no cached summary exists). For remaining requests, it computes a fallback summary via `fallback_page_summary_for_request`; if that summary is empty the request is also skipped. Otherwise, the summary is inserted into the result map under the key produced by `prompt_request_key`. The algorithm thus filters requests based on cache eligibility and non‑empty fallback content, assembling only the summaries that are guaranteed to be available for the dry‑run page.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const int& plan` (`PagePlan`)
- `const std::vector<PromptRequest>& prompt_requests`
- `page_summary_cache_key_for_request(plan, request)`
- `fallback_page_summary_for_request(plan, request)`
- `prompt_request_key(request)`

#### Writes To

- Local `std::unordered_map<std::string, std::string> summary_texts` (returned)

#### Usage Patterns

- Dry run generation to build a map of summary texts per prompt request
- Aggregates cached or fallback summaries for later use

### `clore::generate::build_llms_page`

Declaration: `src/generate/dryrun.cppm:35`

Definition: `src/generate/dryrun.cppm:349`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function constructs the `llms.txt` page content by first retrieving the project name via `project_name_from_config`. It writes a Markdown header with the project name, a brief description, and a link to the API reference index. If the `request_estimate_path` is non‑empty, it conditionally appends a link to a dry‑run request estimate page. The body is then built by three consecutive calls to `append_llms_section`, one for each page type (`PageType::Module`, `PageType::Namespace`, `PageType::File`), which enumerates entries from the `plan_set` and inserts the corresponding labels and links. The complete string is used to create and return a `GeneratedPage` with a fixed relative path of `"llms.txt"`. The function depends on `project_name_from_config` and the helper `append_llms_section`; it does not directly interact with the `RequestEstimate` type or perform any request counting.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan_set`
- config
- `request_estimate_path`

#### Writes To

- `GeneratedPage` object: title, `relative_path`, content

#### Usage Patterns

- Used in page generation pipeline to produce the LLMS index file

### `clore::generate::build_request_estimate_page`

Declaration: `src/generate/dryrun.cppm:31`

Definition: `src/generate/dryrun.cppm:246`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::build_request_estimate_page` computes and returns a `GeneratedPage` containing a static Markdown report of estimated prompt-task counts for the current documentation plan. It first calls `estimate_request_count(plan_set, model)` to obtain a `RequestEstimate` object, and `project_name_from_config(config)` to retrieve the project name from the task configuration. The algorithm then constructs the page content by concatenating a YAML front matter, a project header, a summary line showing `estimate.total_requests`, `estimate.page_requests`, and `estimate.symbol_requests`, and two detailed breakdown sections: one for page-level tasks (namespace summaries, module summaries, module architecture prompts, index overviews) and one for symbol-level tasks with per-category counts (functions, types, variables) and individual request-type fields. The construction is purely sequential and formatting-driven, using `std::format` to embed integer counts; there is no branching or iteration. The function depends on `estimate_request_count`, `project_name_from_config`, and the `RequestEstimate` struct, and its output is a `GeneratedPage` fixed at the relative path `"request-estimate.md"`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan_set`
- `model`
- `config`
- return value of `estimate_request_count`
- return value of `project_name_from_config`

#### Usage Patterns

- Used during dry run generation to produce the estimate page
- Called by the dry run orchestrator to generate the request estimate page

### `clore::generate::page_summary_cache_key_for_request`

Declaration: `src/generate/dryrun.cppm:39`

Definition: `src/generate/dryrun.cppm:309`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The implementation of `clore::generate::page_summary_cache_key_for_request` uses a `switch` statement on `request.kind` to determine the cache key for a given `PromptRequest` within the context of a `PagePlan`. For two `PromptKind` values—`NamespaceSummary` and `ModuleSummary`—it inspects the `plan.owner_keys` container; if the container is not empty, the function returns the first element (`plan.owner_keys.front()`); otherwise it returns `std::nullopt`. For all other handled `PromptKind` enumerators (`ModuleArchitecture`, `IndexOverview`, `FunctionAnalysis`, `TypeAnalysis`, `VariableAnalysis`, `FunctionDeclarationSummary`, `FunctionImplementationSummary`, `TypeDeclarationSummary`, `TypeImplementationSummary`), the function unconditionally returns `std::nullopt`. A trailing `return std::nullopt;` acts as a catch-all, ensuring a value is always returned even if an unlisted `PromptKind` is encountered. The function’s control flow is entirely determined by the `request.kind` discriminant and, for the two key-retrieval cases, by the emptiness or non-emptiness of the `plan.owner_keys` sequence. No external computations or further dependencies beyond the types `PagePlan` and `PromptRequest` are involved.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.owner_keys`
- `request.kind`

#### Usage Patterns

- used as a cache key computation for page summaries
- determines if a page summary can be cached based on prompt kind

## Internal Structure

The `generate::dryrun` module implements the dry-run mode for the documentation generation pipeline. It is structured as a set of public page-building and preview functions that rely on internal helpers and a dedicated `RequestEstimate` struct (declared in an anonymous namespace) to collect counts of symbol, analysis, and summary requests across different categories (e.g., function, type, variable, module, namespace). The public interface comprises `build_dry_run_page_summary_texts`, `build_llms_page`, `build_request_estimate_page`, and `page_summary_cache_key_for_request`; internally it also defines functions such as `estimate_request_count`, `project_name_from_config`, `llms_entry_label`, `fallback_page_summary_for_request`, and `append_llms_section` (which uses a local `LabeledPage` struct).

The module imports `config`, `extract`, `generate:analysis`, and `generate:model`, forming a clear layered dependency: configuration and extraction provide raw data, the analysis module processes that data into structured summaries, and the dry-run module then assembles preview pages and request estimates using the core types defined in `generate::model`. Internally, the implementation is decomposed into estimation logic (aggregated in `RequestEstimate`), page building (for `LLMs` overview and request estimate pages), and summary text generation, with each function operating on a shared set of global variables (`plan`, `plan_set`, `config`, `model`, `prompt_requests`, etc.) that represent the current generation context.

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)
- [Module generate:analysis](analysis.md)
- [Module generate:model](model.md)

