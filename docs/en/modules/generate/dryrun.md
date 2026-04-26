---
title: 'Module generate:dryrun'
description: 'The generate:dryrun module simulates a documentation generation pass without producing final output, enabling accurate estimation of the LLM request counts and resource costs required for full generation. It implements the dry‑run logic by analyzing a PagePlanSet and a configuration to compute a RequestEstimate that breaks down anticipated requests by category (module summaries, symbol analyses, implementation summaries, etc.), and then assembles summary texts and structured LLMs‑compatible pages that present the estimate to the user.'
layout: doc
template: doc
---

# Module `generate:dryrun`

## Summary

The `generate:dryrun` module simulates a documentation generation pass without producing final output, enabling accurate estimation of the LLM request counts and resource costs required for full generation. It implements the dry‑run logic by analyzing a `PagePlanSet` and a configuration to compute a `RequestEstimate` that breaks down anticipated requests by category (module summaries, symbol analyses, implementation summaries, etc.), and then assembles summary texts and structured `LLMs`‑compatible pages that present the estimate to the user.

The public‑facing implementation scope includes four entry points: `build_dry_run_page_summary_texts`, `build_request_estimate_page`, `build_llms_page`, and `page_summary_cache_key_for_request`. These functions accept page plans or request identifiers (passed as `const int &` references) and return integer status codes or cache keys. The module owns all logic for tallying prompt use, generating labeled sections for the `LLMs` output, deriving project names from configuration, and producing fallback summaries when primary results are unavailable.

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

The function iterates over each `PromptRequest` in `prompt_requests` and filters out those for which `page_summary_cache_key_for_request` does not yield a value (i.e., no previously computed cache key). For each remaining request, it invokes `fallback_page_summary_for_request` to obtain a dry‑run summary string; if the result is non‑empty, it stores the summary in the output map keyed by `prompt_request_key(request)`. The core logic relies on two dependency functions: `page_summary_cache_key_for_request` determines whether a request is eligible for a dry‑run summary, and `fallback_page_summary_for_request` produces the actual text. The function returns a mapping from request keys to their summary texts, skipping any request that lacks a cache key or produces an empty summary.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan` (const reference to `PagePlan`)
- `prompt_requests` (const reference to `std::vector<PromptRequest>`)
- `page_summary_cache_key_for_request(plan, request)` return value
- `fallback_page_summary_for_request(plan, request)` return value
- `prompt_request_key(request)` return value

#### Writes To

- local variable `summary_texts` (type `std::unordered_map<std::string, std::string>`) which is returned

#### Usage Patterns

- Called during dry-run page generation to pre-populate summary texts without real LLM inference
- Used to provide fallback summaries for prompt requests in a dry run context

### `clore::generate::build_llms_page`

Declaration: `generate/dryrun.cppm:19`

Definition: `generate/dryrun.cppm:333`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::build_llms_page` assembles a machine‑readable index page (`llms.txt`) for the generated C++ reference. It first retrieves the project name via `project_name_from_config(config)` and builds a Markdown string containing a top‑level heading, a brief description, and a link to the API reference index. If `request_estimate_path` is non‑empty, it also appends a link to the dry run request estimate page. The core algorithm then delegates to three calls of `append_llms_section`, each passing the shared `plan_set` and a distinct `PageType` (Module, Namespace, File) to produce labeled subsections with their entry lists. The final `GeneratedPage` is returned with the constructed `content`, the project name as the title, and the fixed relative path `"llms.txt"`.

Internally, the function depends on several local helpers and types: `project_name_from_config` for the project name, `append_llms_section` (defined in the anonymous namespace) to format each section, and `PagePlanSet`, `config::TaskConfig`, `PageType`, and `GeneratedPage` from the surrounding module. The control flow is purely sequential, reserving an initial capacity of 2048 bytes for the `content` string to reduce reallocations.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan_set`
- `config`
- `request_estimate_path`

#### Writes To

- the returned `GeneratedPage` content

#### Usage Patterns

- called during documentation generation to produce the llms`.txt` page
- used as part of the page generation pipeline

### `clore::generate::build_request_estimate_page`

Declaration: `generate/dryrun.cppm:15`

Definition: `generate/dryrun.cppm:230`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function first calls `estimate_request_count` with the `plan_set` and `model` parameters to obtain a `RequestEstimate` containing the breakdown of prompt tasks. It then retrieves the project name by invoking `project_name_from_config` on the `config`. A `std::string` named `content` is pre-allocated with a capacity of 2048 and built sequentially: frontmatter (title, description, layout, `page_template`) followed by Markdown body that uses `std::format` to embed every integer field from the estimate—such as `total_requests`, `page_requests`, `symbol_requests`, `namespace_page_requests`, `module_summary_requests`, subtotals for function/type/variable symbols and their associated analysis and summary request counts—into explanatory lines. No conditional branches or loops are present; the control flow is purely a linear sequence of appending formatted strings. Finally, the assembled content is moved into a `GeneratedPage` struct with a fixed title ("Dry Run Request Estimate") and relative path ("request-estimate`.md`") and returned.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const clore::generate::PagePlanSet& plan_set`
- `const clore::extract::ProjectModel& model`
- `const clore::config::TaskConfig& config`

#### Writes To

- Returns a `clore::generate::GeneratedPage` object

#### Usage Patterns

- Called during dry-run page generation to produce a request estimate page
- Used to inform users about the number of prompt tasks before actual execution

### `clore::generate::page_summary_cache_key_for_request`

Declaration: `generate/dryrun.cppm:23`

Definition: `generate/dryrun.cppm:293`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::page_summary_cache_key_for_request` computes an optional cache key string based on the kind of a `PromptRequest` and the associated `PagePlan`. Internally, it switches on `request.kind`. For `PromptKind::NamespaceSummary` and `PromptKind::ModuleSummary`, if `plan.owner_keys` is non-empty, the function returns the first element of that container; otherwise it yields `std::nullopt`. For all other enumerated kinds listed in the switch—`ModuleArchitecture`, `IndexOverview`, `FunctionAnalysis`, `TypeAnalysis`, `VariableAnalysis`, `FunctionDeclarationSummary`, `FunctionImplementationSummary`, `TypeDeclarationSummary`, and `TypeImplementationSummary`—the function immediately returns `std::nullopt`. A final fallback return outside the switch also produces `std::nullopt`, ensuring that any unlisted or default prompt kind also results in no cache key. The only dependencies are the `PagePlan` type (specifically its `owner_keys` member) and the `PromptRequest`'s `kind` field; no external state or additional queries are involved.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `request.kind`
- `plan.owner_keys`

#### Usage Patterns

- generates cache key for page summaries
- called when constructing cache entries for dry-run or generation

## Internal Structure

The `generate:dryrun` module is decomposed into a small set of public entry points and a collection of private helpers in an anonymous namespace. The public surface comprises `build_dry_run_page_summary_texts`, `build_request_estimate_page`, `build_llms_page`, and `page_summary_cache_key_for_request`, each returning an `int` result. Internally, the module relies on the helper struct `RequestEstimate` to aggregate counts of various request types (module, function, type, variable, index, namespace), and on functions such as `estimate_request_count`, `append_llms_section`, `llms_entry_label`, `fallback_page_summary_for_request`, and `project_name_from_config` to compute estimates, format LLM-specific pages, and derive project metadata. These private utilities operate over `plan`, `plan_set`, `config`, and `estimate` variables, and build summary content in `content` strings and `LabeledPage` structures.

The module imports `config`, `extract`, `generate:analysis`, and `generate:model` to obtain project configuration, extraction results, analysis data, and core page/model types. Its internal layering separates estimation logic from page-construction and summary-text generation, with the anonymous namespace handling all private computation while the exported functions provide a narrow, stable interface to the rest of the `generate` subsystem.

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)
- [Module generate:analysis](analysis.md)
- [Module generate:model](model.md)

