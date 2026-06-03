---
title: 'Module extract:filter'
description: 'The extract:filter module is responsible for path‑based filtering and normalisation within the extraction pipeline. It provides public utilities to compute project‑relative paths, resolve paths safely under a given directory, canonicalise file paths to a stable string representation, and test whether a path prefix matches a filter pattern. It also exposes a filtering predicate that decides inclusion of compilation entities based on a filter string and a root directory, and a function to derive a filtered root path from an integer identifier. The module defines a PathResolveError struct to report failures when a path escapes its intended base directory.'
layout: doc
template: doc
---

# Module `extract:filter`

## Summary

The `extract:filter` module is responsible for path‑based filtering and normalisation within the extraction pipeline. It provides public utilities to compute project‑relative paths, resolve paths safely under a given directory, canonicalise file paths to a stable string representation, and test whether a path prefix matches a filter pattern. It also exposes a filtering predicate that decides inclusion of compilation entities based on a filter string and a root directory, and a function to derive a filtered root path from an integer identifier. The module defines a `PathResolveError` struct to report failures when a path escapes its intended base directory.

The public scope includes functions that operate on filesystem paths and strings: `project_relative_path`, `resolve_path_under_directory`, `canonical_graph_path`, `path_prefix_matches`, `matches_filter`, and `filter_root_path`. These are used throughout the extraction workflow to enforce boundary safety, enable consistent path lookup, and control which files or symbols are included based on dynamically‑configured filter criteria. The module depends on the `config` module for access to configuration parameters that may influence path resolution and filtering behaviour.

## Imports

- [`config`](../config/index.md)

## Imported By

- [`extract:merge`](merge.md)

## Types

### `clore::extract::PathResolveError`

Declaration: `src/extract/filter.cppm:17`

Definition: `src/extract/filter.cppm:17`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::PathResolveError` serves as a lightweight error type within the extraction subsystem, used to report failures during path resolution. Its sole data member is `message`, a `std::string` that holds a human-readable description of the error. The struct imposes no invariants beyond those inherent to `std::string`; the `message` may be empty if no specific error text is provided. As an aggregate, it supports direct member initialization and is trivially copyable and movable. No additional member functions or constructors are defined—the type relies on default compiler-generated operations, making it a minimal, self‑contained error representation that integrates naturally with error‑handling patterns in the codebase.

#### Invariants

- The `message` member is intended to provide explanatory text for the error.

#### Key Members

- `message` – a `std::string` that describes the error.

#### Usage Patterns

- No direct usage patterns are evident from the provided context.

## Functions

### `clore::extract::canonical_graph_path`

Declaration: `src/extract/filter.cppm:30`

Definition: `src/extract/filter.cppm:112`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/canonical-graph-path.md)

The function attempts to produce a canonical, platform‑generic string representation of a filesystem path. It first tries to resolve the input via `std::filesystem::absolute` and then applies `std::filesystem::weakly_canonical` on the lexically‑normalised absolute path. If either step fails (indicated by the `std::error_code` `ec`), it falls back to returning the generic string of the absolute, lexically‑normalised path. If `absolute` itself fails, the function repeats the same two‑step attempt using only `path.lexically_normal()` as the basis, eventually returning the generic string of the normalised path as a last resort. The algorithm depends entirely on `std::filesystem` operations and uses `ec` to detect errors at each stage without throwing exceptions.

#### Side Effects

- Allocates a `std::string` for the return value
- Performs filesystem reads to resolve symlinks and determine the current working directory via absolute and `weakly_canonical`

#### Reads From

- Input path parameter
- Filesystem state via `std::filesystem::absolute` and `std::filesystem::weakly_canonical`
- Current working directory implicitly via `std::filesystem::absolute`

#### Usage Patterns

- Used by `clore::extract::matches_filter` to obtain a canonical string representation of a path for comparison

### `clore::extract::filter_root_path`

Declaration: `src/extract/filter.cppm:36`

Definition: `src/extract/filter.cppm:170`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::filter_root_path` follows a straightforward conditional algorithm to resolve a canonical root directory from a given `config::TaskConfig` object. It first inspects the `config.workspace_root` field; if that string is non‑empty, the function immediately returns a `std::filesystem::path` constructed from that value and normalized via `lexically_normal()`. Otherwise, it falls back to the `config.project_root` field and returns the same kind of normalized path. No further validation or path resolution is performed inside this function—it relies on the caller to ensure that at least one of the two configuration fields holds a sensible filesystem location. The sole dependency is on the `std::filesystem` library and the `config::TaskConfig` type from which the two string members are read.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- config`.workspace_root`
- config`.project_root`

#### Usage Patterns

- Used to obtain a canonical root path for filtering operations.

### `clore::extract::matches_filter`

Declaration: `src/extract/filter.cppm:32`

Definition: `src/extract/filter.cppm:133`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::matches_filter` begins by normalising the input `file` path and the `filter_root` path through the dependency `clore::extract::canonical_graph_path`, converting each to a canonical representation via `std::filesystem::path`. It then invokes `clore::extract::project_relative_path` on these two canonical paths to obtain a relative path from `filter_root` to `file`. If no such relative path exists (i.e., `file` lies outside the directory tree rooted at `filter_root`), the function immediately returns `false`. Otherwise, the resulting relative path is converted to a generic string for pattern matching.

The second phase applies the include/exclude logic defined in the `config::FilterRule` parameter. If the `include` list is non‑empty, the function iterates over its patterns; at least one must match the relative string via `clore::extract::path_prefix_matches`—otherwise the function returns `false`. After verifying inclusion, every pattern in the `exclude` list is tested similarly; if any exclude pattern matches, the function returns `false`. Only when all inclusion constraints are satisfied and no exclusion applies does the function return `true`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `file` parameter
- the `filter` parameter's `include` and `exclude` patterns
- the `filter_root` parameter

#### Usage Patterns

- Called during extract processing to filter source files
- Used to check if a file path matches include/exclude filtering rules

### `clore::extract::path_prefix_matches`

Declaration: `src/extract/filter.cppm:21`

Definition: `src/extract/filter.cppm:42`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first strips trailing slashes from `pattern` and returns `false` if it becomes empty. It then distinguishes two cases based on whether `pattern` contains a slash. If it does, the function checks that `relative` starts with the entire `pattern` and that the character immediately after the prefix is either the end of the string or a slash, ensuring a whole-path-component boundary. If `pattern` contains no slash, the function performs a similar component-wise check: it returns `true` only if `relative` equals `pattern` exactly or if `relative` begins with `pattern` followed by a slash. No external dependencies are used beyond the standard `std::string_view` operations; the control flow is purely conditional with early returns.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `relative` parameter
- `pattern` parameter

#### Usage Patterns

- Path filtering in directory-based selection
- Matching file paths against prefix patterns

### `clore::extract::project_relative_path`

Declaration: `src/extract/filter.cppm:23`

Definition: `src/extract/filter.cppm:73`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation of `clore::extract::project_relative_path` computes a filesystem path relative to a given root, then validates that the result does not escape the root directory. The function first calls `std::filesystem::path::lexically_relative` on the `root_path` argument, obtaining a tentative relative path. If the result is empty (meaning no relative path exists), the function immediately returns `std::nullopt`. Otherwise, it iterates over each component of the obtained relative path; if any component equals `".."`, the path attempts to ascend above the root, so the function again returns `std::nullopt`. Only when all components are safe does it return the relative path as a `std::optional<std::filesystem::path>`. The algorithm relies solely on `std::filesystem::path` operations and uses no external filesystem calls, making it purely lexical and side-effect free.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `file` parameter
- `root_path` parameter

#### Usage Patterns

- Converting absolute file path to project-relative path
- Validating that a file is under a given root directory

### `clore::extract::resolve_path_under_directory`

Declaration: `src/extract/filter.cppm:27`

Definition: `src/extract/filter.cppm:88`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function first checks whether the input `path` is empty, returning an error with a descriptive message in `PathResolveError` if so. It then constructs a `std::filesystem::path` from the string. If the path is relative, it validates that the `directory` argument is non‑empty—otherwise it returns an error explaining that a relative path requires a directory. When both conditions are satisfied, it forms an absolute path by resolving the relative path against the given directory via `operator/`. Finally, the result is normalized with `lexically_normal()` and returned as the expected `std::filesystem::path`. The only external dependency beyond standard library types (`std::filesystem`, `std::expected`) is the custom error struct `PathResolveError`, which carries a `message` string.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the parameter `path`
- the parameter `directory`

#### Usage Patterns

- resolving file paths from `compile_commands``.json` entries
- used by other path normalization functions

## Internal Structure

The `extract:filter` module provides a cohesive set of path‑manipulation and filtering utilities for the extraction pipeline. It is decomposed into several logical layers: low‑level helpers such as `canonical_graph_path` and `path_prefix_matches` for normalising and comparing path strings; mid‑level resolution functions like `project_relative_path` and `resolve_path_under_directory` that compute safe, project‑rooted paths; and higher‑level predicates (`matches_filter`) and path derivation (`filter_root_path`) that incorporate filter logic. Error handling is encapsulated in the `PathResolveError` struct, used with `std::expected` in `resolve_path_under_directory`.

Internally, the module imports the `config` module, indicating that filter behaviour depends on configuration settings (such as root directories or filter patterns). The implementation relies on `std::filesystem` for path operations and avoids platform‑specific constructs by normalising paths early. The functions are arranged so that the lower‑level path utilities are self‑contained, while the filtering layer builds on them, maintaining a clear separation of concerns without circular dependencies.

## Related Pages

- [Module config](../config/index.md)

