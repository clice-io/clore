---
title: 'Module config:normalize'
description: 'The config:normalize module is responsible for transforming configuration values into a standardized, canonical form. It exposes the clore::config::normalize function, which accepts a mutable integer reference and modifies it in place to conform to expected constraints, returning an integer indicating success or error status. The normalization process likely handles tasks such as resolving relative paths to absolute ones and normalizing separators, as suggested by public variables like workspace_root, make_absolute, and normalize_separators.'
layout: doc
template: doc
---

# Module `config:normalize`

## Summary

The `config:normalize` module is responsible for transforming configuration values into a standardized, canonical form. It exposes the `clore::config::normalize` function, which accepts a mutable integer reference and modifies it in place to conform to expected constraints, returning an integer indicating success or error status. The normalization process likely handles tasks such as resolving relative paths to absolute ones and normalizing separators, as suggested by public variables like `workspace_root`, `make_absolute`, and `normalize_separators`.

Publicly, the module owns the `clore::config::NormalizeError` struct, which carries a `message` field to communicate failures during normalization. These types complement other configuration error and schema types from the `config:schema` dependency. Overall, the module encapsulates the logic and error reporting for standardizing configuration inputs.

## Imports

- [`config:schema`](schema.md)
- `std`

## Types

### `clore::config::NormalizeError`

Declaration: `config/normalize.cppm:10`

Definition: `config/normalize.cppm:10`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The struct `clore::config::NormalizeError` is a trivial error type that carries a single data member, `message` of type `std::string`. It serves as a dedicated error representation for normalization operations within the config subsystem. There are no invariants beyond the string being well-formed; the struct is intended to be constructed and passed by value, relying on standard string semantics for ownership and copying. No custom constructors, assignment `operator`s, or member functions are provided, so all special member functions are implicitly defined. This simplicity makes `NormalizeError` a lightweight and unambiguous result type for functions that may fail during configuration normalization.

#### Invariants

- The `message` member contains a textual description of the error that occurred.

#### Key Members

- `message`: a `std::string` that holds the error description.

#### Usage Patterns

- Used as an exception type or error result to convey normalization failures.
- Its `message` member is accessed to retrieve error details.

## Functions

### `clore::config::normalize`

Declaration: `config/normalize.cppm:14`

Definition: `config/normalize.cppm:22`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The function `clore::config::normalize` performs a two‑phase path cleanup on a `TaskConfig` object. First, if `config.workspace_root` is empty, it is set to the current working directory via `std::filesystem::current_path`. Then a local lambda `make_absolute` is applied to `config.workspace_root`, `compile_commands_path`, `project_root`, and `output_root`. This lambda uses `std::filesystem::path` to detect relative paths; if a path is relative and a base is provided (defaulting to `std::nullopt`, which triggers `fs::absolute`), it resolves the path against that base. Every resolved path is then run through `lexically_normal()` and converted back to a string. The lambda returns a `std::expected<void, NormalizeError>`, and each call is immediately checked; any failure (e.g., an empty path) short‑circuits the function with an error containing a formatted `message`.

After all paths are made absolute, the second phase normalizes separators using the lambda `normalize_separators`, which replaces every backslash (`\\`) with a forward slash (`/`). This is applied to the same four fields as well as to every element of `config.filter.include` and `config.filter.exclude`. The function ultimately returns an empty success `std::expected` if all steps pass. Dependencies include `std::filesystem` for path manipulation, `std::format` for error messages, `std::expected` for error handling, and `std::optional` for the optional base in `make_absolute`.

#### Side Effects

- Mutates the `config` parameter's path fields (`workspace_root`, `compile_commands_path`, `project_root`, `output_root`, filter inclusion/exclusion lists)
- Calls `std::filesystem::current_path()` which queries the operating system

#### Reads From

- `config.workspace_root`
- `config.compile_commands_path`
- `config.project_root`
- `config.output_root`
- `config.filter.include`
- `config.filter.exclude`
- `std::filesystem::current_path()`

#### Writes To

- `config.workspace_root`
- `config.compile_commands_path`
- `config.project_root`
- `config.output_root`
- `config.filter.include` (each element)
- `config.filter.exclude` (each element)

#### Usage Patterns

- Called after loading a `TaskConfig` to ensure paths are absolute and use consistent separators
- Part of the configuration normalization pipeline before validation

## Internal Structure

The module `config:normalize` is a small, focused module that implements normalization of configuration values. It imports only `config:schema` (for the configuration schema types) and the standard library, indicating a clean dependency boundary. The module exposes a single public function, `normalize`, which accepts a mutable reference to an integer and returns an integer status, along with a dedicated error struct `NormalizeError` to signal failures. Internally, the module uses local variables such as `config`, `path`, `workspace_root`, `make_absolute`, and `normalize_separators` to perform transformations like making paths absolute and normalizing separators. The implementation is structured as a straightforward pipeline: it first gathers necessary context (e.g., workspace root), then applies normalization steps in sequence, modifying the configuration value in place and returning an appropriate status code. This decomposition keeps the normalization logic encapsulated and testable.

## Related Pages

- [Module config:schema](schema.md)

