---
title: 'Module extract:filter'
description: 'The extract:filter module provides path resolution and filtering utilities used during the extraction process. It defines public functions such as resolve_path_under_directory, canonical_graph_path, and project_relative_path that normalize and compute relative paths from project roots. Filtering capabilities are exposed through matches_filter and path_prefix_matches, which check inclusion criteria and directory ancestry. The PathResolveError struct reports failures when a path cannot be resolved to a filesystem location. Together, these components manage scope restrictions and path transformations essential for extracting relevant source artifacts.'
layout: doc
template: doc
---

# Module `extract:filter`

## Summary

The `extract:filter` module provides path resolution and filtering utilities used during the extraction process. It defines public functions such as `resolve_path_under_directory`, `canonical_graph_path`, and `project_relative_path` that normalize and compute relative paths from project roots. Filtering capabilities are exposed through `matches_filter` and `path_prefix_matches`, which check inclusion criteria and directory ancestry. The `PathResolveError` struct reports failures when a path cannot be resolved to a filesystem location. Together, these components manage scope restrictions and path transformations essential for extracting relevant source artifacts.

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

The struct `clore::extract::PathResolveError` consists of a single public member, `std::string message`, which stores a human-readable description of the resolution failure. No additional invariants are enforced beyond those inherent to `std::string`; the member is intended to be populated at construction or assignment time. All special member functions (default constructor, copy/move constructors, copy/move assignment, destructor) are implicitly defined by the compiler, meaning the struct is trivially copyable and movable via the corresponding operations of `std::string`. This lightweight design allows the error type to be passed and stored efficiently without custom resource management.

#### Invariants

- The `message` member is always initialized (default-constructed or assigned).

#### Key Members

- `std::string message`

#### Usage Patterns

- Constructed with a descriptive error message when path resolution fails.
- Likely returned as an error from functions or stored in `std::expected` or similar error-handling mechanisms.

## Functions

### `clore::extract::canonical_graph_path`

Declaration: `extract/filter.cppm:21`

Definition: `extract/filter.cppm:103`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::canonical_graph_path` normalises a given `std::filesystem::path` into a uniform, platform-independent string suitable for graph contexts. It first attempts to compute an absolute path via `fs::absolute`; if successful, it applies `fs::weakly_canonical` on the lexically-normalised absolute form. Should that second step fail, it falls back to the absolute normalised path’s `generic_string`. If the initial absolute resolution itself fails (e.g., due to a non-existent component), the function computes a lexically-normalised version of the original path and tries `fs::weakly_canonical` on that; as a final fallback, it returns the normalised string directly. Throughout, `std::error_code` captures filesystem errors, ensuring that no exceptions propagate from the standard library calls, and the returned string always uses forward slashes via `generic_string`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the input `path` parameter
- filesystem state (to resolve absolute and canonical paths)

#### Usage Patterns

- normalizing paths for dependency graph nodes
- computing a unique key for a filesystem path

### `clore::extract::filter_root_path`

Declaration: `extract/filter.cppm:27`

Definition: `extract/filter.cppm:161`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function evaluates two candidate root paths from the provided `config` object. It first checks whether `config.workspace_root` is non‑empty; if so, it returns that path normalized via `lexically_normal()`. Otherwise, it falls back to normalizing `config.project_root`. This simple conditional ensures that when a workspace root is configured, it takes precedence as the filter root, while a project root serves as the default. No further transformation or validation is performed on the selected path.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- config`.workspace_root`
- config`.project_root`

#### Usage Patterns

- Obtain the normalized root path for filtering operations.

### `clore::extract::matches_filter`

Declaration: `extract/filter.cppm:23`

Definition: `extract/filter.cppm:124`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::matches_filter` determines whether a given `file` should be processed based on a `config::FilterRule` with include and exclude patterns, relative to a `filter_root` directory. It begins by normalising both the `file` path and the `filter_root` via `canonical_graph_path`, then computes a relative path from the root to the file using `project_relative_path`. If the relative path cannot be obtained (e.g. the file lies outside the root), the function immediately returns `false`.

Internally, the algorithm proceeds in two phases. If the `filter.include` list is non-empty, it iterates over each pattern and tests `path_prefix_matches` against the relative path string; if no pattern matches, the function returns `false`. Regardless of include filtering, it then iterates over the `filter.exclude` list; if any exclusion pattern matches, the function returns `false`. Only if the relative path satisfies both the include (or absence of include rules) and the exclude rule set does it return `true`. The core dependencies are `canonical_graph_path` for path normalisation, `project_relative_path` for relative path computation, and `path_prefix_matches` for pattern matching against the generic string representation of the relative path.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `file` (const `std::string`&)
- parameter `filter` (const `config::FilterRule`&)
- parameter `filter_root` (const `std::filesystem::path`&)

#### Usage Patterns

- filtering source files during symbol extraction
- applying user-defined include/exclude rules

### `clore::extract::path_prefix_matches`

Declaration: `extract/filter.cppm:12`

Definition: `extract/filter.cppm:33`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::path_prefix_matches` determines whether a given `relative` path string starts with a `pattern` such that the match occurs at a directory boundary. It first strips any trailing slashes from `pattern`; if the result is empty it immediately returns `false`. When the pattern contains a slash, it performs a prefix check using `relative.starts_with(pattern)` and then validates that the remainder of `relative` is either empty or begins with another slash, ensuring the match respects path component boundaries. For patterns without a slash (i.e., a single name component), the function checks whether `relative` exactly equals `pattern`, or, if `relative` is longer, whether it starts with `pattern` followed by a slash. The implementation relies solely on `std::string_view` operations such as `empty()`, `back()`, `remove_suffix()`, `find()`, `starts_with()`, `size()`, and indexing, with no external dependencies or side effects.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `relative`
- `pattern`

#### Usage Patterns

- Used to filter compilation entries by file path prefixes
- Applied when checking if a source file belongs to a given directory pattern

### `clore::extract::project_relative_path`

Declaration: `extract/filter.cppm:14`

Definition: `extract/filter.cppm:64`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The implementation computes a relative path from `root_path` to `file` by calling `std::filesystem::path::lexically_relative` on `file` with `root_path`. This operation produces a path whose first component would place the starting location inside `root_path`; the result is stored in `rel`. If `rel` is empty—meaning the two paths are identical or `lexically_relative` could not produce a meaningful relative form—the function immediately returns `std::nullopt`. Otherwise, it iterates over each component of `rel`; if any component equals `".."`, the path escapes `root_path`, and the function again returns `std::nullopt`. Only when all components are within `root_path` does it return `rel` wrapped in `std::optional`. The entire control flow relies solely on standard library path operations and involves no external project‑specific dependencies.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `file` (const ref)
- `root_path` (const ref)

#### Usage Patterns

- path validation before extraction
- ensuring file is within project root

### `clore::extract::resolve_path_under_directory`

Declaration: `extract/filter.cppm:18`

Definition: `extract/filter.cppm:79`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The function `clore::extract::resolve_path_under_directory` resolves a file path from a compilation database entry against an associated directory. It first checks if the input `path` is empty, returning a `PathResolveError` with an appropriate message if so. For a relative `path`, it verifies that the provided `directory` is non‑empty; if empty, it returns an error indicating that a directory is required. Otherwise, it forms an absolute path by combining `directory` and `path` using the `/` `operator` on `std::filesystem::path`. Finally, the result is normalized via `lexically_normal()` and returned as a valid `std::expected`. The function relies solely on the `std::filesystem` library and the custom `PathResolveError` struct; no other project utilities are invoked.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- const `std::string`& path
- const `std::string`& directory

#### Usage Patterns

- Resolve compilation database entry file path
- Normalize relative source paths

## Internal Structure

The `extract:filter` module is decomposed into two conceptual layers: path resolution utilities and filtering predicates. The resolution layer—comprising `resolve_path_under_directory`, `canonical_graph_path`, `project_relative_path`, and `path_prefix_matches—handles` normalization, prefix checking, and derivation of relative paths using opaque integer path handles, relying on `std` for underlying filesystem operations. The filtering layer (`matches_filter`, `filter_root_path`) applies inclusion or root-scoping rules, drawing configuration from the `config` module and using the resolution utilities to validate filtered paths. Error conditions from resolution are explicitly represented by the public `PathResolveError` struct, which carries an error message. Implementation structure separates public interface in the primary module (filter`.cppm`) from internal details, with all public symbols declared at module scope and no further internal submodules indicated.

## Related Pages

- [Module config](../config/index.md)

