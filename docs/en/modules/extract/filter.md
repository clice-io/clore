---
title: 'Module extract:filter'
description: 'The extract:filter module is responsible for providing path resolution, normalization, and filtering utilities used throughout the extraction pipeline. It abstracts filesystem operations to ensure that paths are safely resolved under designated root directories, canonicalized for consistent comparison, and tested against inclusion or exclusion patterns. The module handles error conditions such as unresolvable or out‐of‑range paths through a dedicated PathResolveError struct.'
layout: doc
template: doc
---

# Module `extract:filter`

## Summary

The `extract:filter` module is responsible for providing path resolution, normalization, and filtering utilities used throughout the extraction pipeline. It abstracts filesystem operations to ensure that paths are safely resolved under designated root directories, canonicalized for consistent comparison, and tested against inclusion or exclusion patterns. The module handles error conditions such as unresolvable or out‐of‑range paths through a dedicated `PathResolveError` struct.

The public interface consists of functions that operate on integer path handles (rather than raw strings) to enforce a controlled, pipeline‑safe environment. `resolve_path_under_directory` validates that a target path lies strictly under a given root and returns the canonical absolute path. `canonical_graph_path` produces a stable, normalized identifier for a path for use in graph operations. `matches_filter` determines whether a path satisfies a two‑parameter filter criterion. `filter_root_path` transforms or normalizes a root path for pipeline consumption. `path_prefix_matches` checks whether a path starts with a given prefix, and `project_relative_path` computes a relative path from a project root to a source path. Together, these functions form the core path manipulation layer for the extraction framework.

## Imports

- [`config`](../config/index.md)
- `std`

## Imported By

- [`extract:merge`](merge.md)

## Types

### `clore::extract::PathResolveError`

Declaration: `extract/filter.cppm:8`

Definition: `extract/filter.cppm:8`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::PathResolveError` is implemented as a trivial aggregate containing a single `std::string message` field. This field is intended to hold a human-readable description of the error that occurred during path resolution. Its design places no additional invariants beyond those already enforced by `std::string`; however, callers are expected to populate `message` with a meaningful value for diagnostic purposes. As an aggregate, the struct supports brace-initialization and is implicitly default-constructible, copyable, and movable, with no special member implementations required.

#### Invariants

- The struct is an aggregate with one data member.
- The `message` member may be empty or contain a human-readable error description.

#### Key Members

- `message` - the error description string

#### Usage Patterns

- No specific usage patterns are provided in the evidence; it is assumed to be used for error reporting.

## Functions

### `clore::extract::canonical_graph_path`

Declaration: `extract/filter.cppm:21`

Definition: `extract/filter.cppm:103`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::canonical_graph_path` implements a multi‑stage normalization strategy using `std::filesystem`. It first attempts to resolve the input `path` to an absolute form via `fs::absolute`. If that succeeds, it applies `fs::weakly_canonical` to the normalized absolute path; if the canonicalization also succeeds, the result is returned as a generic string. When canonicalization fails but absolute resolution succeeded, it falls back to the normalized absolute path’s generic string. If the initial `absolute` call fails, the function performs a purely lexical normalization of the original `path` and then tries `fs::weakly_canonical` on that normalized string; if that attempt fails as well, it returns the normalized generic string directly. The entire control flow is guarded by `std::error_code` to avoid exceptions, ensuring a usable string is always returned regardless of filesystem errors or missing entries.

#### Side Effects

- Performs filesystem I/O to resolve absolute and canonical paths

#### Reads From

- filesystem state
- current working directory
- the input `path` parameter

#### Writes To

- return value of type `std::string`

#### Usage Patterns

- Used to obtain a stable, canonical path key for graph nodes
- Called when constructing or looking up dependency graph entries
- Relies on filesystem to normalize path representations

### `clore::extract::filter_root_path`

Declaration: `extract/filter.cppm:27`

Definition: `extract/filter.cppm:161`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first checks whether `config.workspace_root` is non‑empty. If so, it returns `config.workspace_root` normalized via `lexically_normal()`; otherwise it falls back to normalizing `config.project_root`. This conditional choice means the filter root is always derived from a single configuration field, preferring the workspace root over the project root when present. The only dependency is `config::TaskConfig` and the `std::filesystem` path normalization utility.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `config::TaskConfig::workspace_root`
- `config::TaskConfig::project_root`

#### Usage Patterns

- computing canonical root path for filtering operations
- providing a normalized base directory for path comparisons

### `clore::extract::matches_filter`

Declaration: `extract/filter.cppm:23`

Definition: `extract/filter.cppm:124`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation normalises both the target `file` and the `filter_root` by calling `clore::extract::canonical_graph_path` on each, producing `file_path` and `root_path`. It then computes a project‑relative path using `clore::extract::project_relative_path`; if no relative path exists, the function immediately returns `false`. After converting the relative path to a generic string (`relative_str`), control proceeds through two pattern‑matching stages driven by the `filter.include` and `filter.exclude` vectors. For includes, the function iterates over each `pattern` using `clore::extract::path_prefix_matches`; if none match, it returns `false`. For excludes, any matching pattern also causes an immediate `false` return. If both stages pass, the function returns `true`. All path comparisons rely on `path_prefix_matches` rather than full equality, and relative‑path resolution is a precondition for any further matching.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `file`
- parameter `filter`
- parameter `filter_root`
- internal calls to `clore::extract::canonical_graph_path`
- internal calls to `clore::extract::project_relative_path`
- internal calls to `clore::extract::path_prefix_matches`
- fields `filter.include` and `filter.exclude`

#### Usage Patterns

- used in extraction pipeline to filter source files based on include/exclude rules
- called after obtaining a normalized file path and before further processing

### `clore::extract::path_prefix_matches`

Declaration: `extract/filter.cppm:12`

Definition: `extract/filter.cppm:33`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::path_prefix_matches` determines whether a `relative` path matches a given `pattern` as a directory‑aware prefix. It first strips any trailing slashes from `pattern`, returning `false` if the resulting pattern is empty. If the trimmed pattern contains a slash, the function requires an exact prefix match via `relative.starts_with(pattern)` and then verifies that either the strings are equal or the next character in `relative` is a slash—this enforces whole‑segment matching for multi‑component prefixes. When the pattern has no slash (i.e., it is a single name), the function first checks for an exact equality; if that fails, it ensures `relative` is at least one character longer than `pattern`, confirms the prefix match, and then checks that the character immediately following the prefix is a slash. This logic ensures that a single‑name pattern only matches an actual path component, not a partial file name. The function relies solely on `std::string_view` operations and has no external dependencies beyond the C++ standard library; it is used internally by `clore::extract::matches_filter` to evaluate inclusion/exclusion rules during file extraction.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `relative` (`std::string_view`)
- `pattern` (`std::string_view`)

#### Usage Patterns

- used by path-filtering logic
- called by `matches_filter` in the extract module

### `clore::extract::project_relative_path`

Declaration: `extract/filter.cppm:14`

Definition: `extract/filter.cppm:64`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::project_relative_path` computes a relative path from `root_path` to `file` using `std::filesystem::path::lexically_relative` and stores the result in `rel`. It then validates the computed relative path: if `rel` is empty (meaning `file` is not under `root_path`) or if any component equals `".."` (indicating an escape above `root_path`), the function returns `std::nullopt`. Otherwise, it returns `rel` as the project-relative path. The algorithm is purely linear with a single loop over the components of the relative path, relying only on the standard library facilities `std::filesystem::path` and `std::optional`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `file`
- parameter `root_path`

#### Usage Patterns

- Used to derive a project-local path for source files during extraction.
- Likely employed when normalizing file paths relative to a project root for indexing or analysis.

### `clore::extract::resolve_path_under_directory`

Declaration: `extract/filter.cppm:18`

Definition: `extract/filter.cppm:79`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation first checks whether the input `path` is empty, and if so, returns an error of type `clore::extract::PathResolveError` with an appropriate message. Otherwise, it constructs a `std::filesystem::path` object from `path`. If that path is relative, the function validates that the `directory` argument is non‑empty; if `directory` is empty, it returns another `PathResolveError`. Provided that check passes, the relative path is resolved by prepending `directory` via the `operator/` of `std::filesystem::path`. Finally, the function calls `lexically_normal()` on the resulting path to produce a canonical representation. No further resolution against the actual file system is performed — the logic relies solely on lexical normalization and the caller‑provided directory for disambiguation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `path` parameter
- `directory` parameter

#### Usage Patterns

- Resolves file paths from `compile_commands``.json` entries
- Combines relative paths with the compilation directory
- Used as a helper for normalizing entry file paths

## Internal Structure

The `extract:filter` module is decomposed into a layered set of path handling utilities, filtering predicates, and error types. It imports only the `config` module and the standard library, keeping its dependencies minimal. At the lowest internal layer, core operations such as `path_prefix_matches`, `canonical_graph_path`, `resolve_path_under_directory`, and `project_relative_path` work on integer path handles, providing normalized, directory‑constrained, or relative representations. Above that, `matches_filter` and `filter_root_path` combine these primitives to implement configurable filtering logic, returning boolean or transformed handles. The module also defines a `PathResolveError` struct for error reporting during resolution. Internally, all public functions and variables are declared in a single partition (`filter.cppm`), with local variables for intermediate state (e.g., `relative`, `canonical`, `ec`) kept private to the implementation. This structure separates pure path manipulation from higher‑level filtering, enabling reuse and clear responsibility boundaries.

## Related Pages

- [Module config](../config/index.md)

