---
title: 'Module config:normalize'
description: 'The config:normalize module provides a single public function, clore::config::normalize, which transforms an integer configuration value into a canonical, internally consistent form. It accepts a mutable reference to the value and returns a std::expected<void, NormalizeError>, where failure surfaces a NormalizeError struct carrying a human‑readable message. This normalization step is designed to run after a configuration has been loaded (e.g., by clore::config::load_config) and before validation via clore::config::validate, ensuring that subsequent validation can assume a well‑formed input. The module depends on the config:schema module for the underlying configuration data model.'
layout: doc
template: doc
---

# Module `config:normalize`

## Summary

The `config:normalize` module provides a single public function, `clore::config::normalize`, which transforms an integer configuration value into a canonical, internally consistent form. It accepts a mutable reference to the value and returns a `std::expected<void, NormalizeError>`, where failure surfaces a `NormalizeError` struct carrying a human‑readable `message`. This normalization step is designed to run after a configuration has been loaded (e.g., by `clore::config::load_config`) and before validation via `clore::config::validate`, ensuring that subsequent validation can assume a well‑formed input. The module depends on the `config:schema` module for the underlying configuration data model.

## Imports

- [`config:schema`](schema.md)

## Types

### `clore::config::NormalizeError`

Declaration: `src/config/normalize.cppm:17`

Definition: `src/config/normalize.cppm:17`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The struct `clore::config::NormalizeError` is a simple error type that serves as a lightweight carrier for a descriptive error message. Internally, it contains a single public data member `message` of type `std::string`, which holds the human‑readable explanation of the normalization failure. No invariants beyond those inherent to `std::string` are required; the member is directly accessible and is expected to contain a valid, non‑empty string when the error is used. The trivial structure means that the default constructor, copy, and move operations are compiler‑generated, and no special member implementations are needed.

#### Invariants

- The `message` member stores a human-readable description of the error.

#### Key Members

- `std::string message`

#### Usage Patterns

- Used to represent a normalization error, typically returned or thrown from normalization functions.

## Functions

### `clore::config::normalize`

Declaration: `src/config/normalize.cppm:21`

Definition: `src/config/normalize.cppm:29`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The implementation of `clore::config::normalize` first ensures that `config.workspace_root` is non-empty by defaulting it to the current working directory. It then employs a local lambda `make_absolute` to convert several key path fields—`compile_commands_path`, `project_root`, `output_root`, and `workspace_root`—to absolute, lexically‑normalized forms. For each field, `make_absolute` returns a `std::expected<void, NormalizeError>`, aborting the entire function with an appropriate error if the input path is empty or if the conversion fails. Finally, a second lambda `normalize_separators` replaces any backslash characters with forward slashes, applied to all previously resolved absolute paths as well as every element in `config.filter.include` and `config.filter.exclude`. The function returns success only after all paths are both absolute and canonical in separator style.

#### Side Effects

- Modifies the `config.workspace_root` field
- Modifies the `config.compile_commands_path` field
- Modifies the `config.project_root` field
- Modifies the `config.output_root` field
- Modifies each string in `config.filter.include`
- Modifies each string in `config.filter.exclude`
- Calls `std::filesystem::absolute` (may query current working directory)
- Calls `std::filesystem::path::lexically_normal`
- Calls `std::filesystem::current_path` when `workspace_root` is empty

#### Reads From

- `config.workspace_root`
- `config.compile_commands_path`
- `config.project_root`
- `config.output_root`
- `config.filter.include`
- `config.filter.exclude`
- `std::filesystem::current_path()` (implicitly via `fs::absolute` and when `workspace_root.empty()`)

#### Writes To

- `config.workspace_root`
- `config.compile_commands_path`
- `config.project_root`
- `config.output_root`
- Elements of `config.filter.include`
- Elements of `config.filter.exclude`

#### Usage Patterns

- Called after loading configuration via `load_config` or `load_config_from_string`
- Called before `validate` to ensure paths are in canonical form
- Used to sanitize and complete user‑provided configuration

## Internal Structure

The `config:normalize` module depends on the `config:schema` module and provides a single public entry point—the `normalize` function—which accepts a mutable reference to an integer configuration value and returns an `std::expected<void, NormalizeError>`. Internally, the implementation is decomposed into several local variables and helpers that handle distinct normalization concerns: `workspace_root` and `base` guide path resolution; `make_absolute` and `normalize_separators` enforce absolute paths and consistent separator conventions; and `field`, `path`, and `config` track the current value being processed. The `NormalizeError` struct carries a public `message` string to report why normalization failed. This layered structure isolates path conversion, separator canonicalization, and error reporting, allowing the function to prepare a configuration for subsequent validation without performing full semantic checks.

## Related Pages

- [Module config:schema](schema.md)

