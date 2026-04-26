---
title: 'Module config:normalize'
description: 'The config:normalize module is responsible for normalizing configuration values within the clore system. Its public interface consists of the clore::config::normalize function, which accepts a mutable reference to an int representing a configuration value, modifies it according to normalization rules, and returns an int status code. The module also exposes the clore::config::NormalizeError struct containing a message field to communicate error details. Additional publicly accessible variables—such as make_absolute, workspace_root, normalize_separators, and configuration-related variables like config and path—define the normalization context and behavior. The implementation relies on the config:schema module for its core configuration types and on the standard library.'
layout: doc
template: doc
---

# Module `config:normalize`

## Summary

The `config:normalize` module is responsible for normalizing configuration values within the clore system. Its public interface consists of the `clore::config::normalize` function, which accepts a mutable reference to an `int` representing a configuration value, modifies it according to normalization rules, and returns an `int` status code. The module also exposes the `clore::config::NormalizeError` struct containing a `message` field to communicate error details. Additional publicly accessible variables—such as `make_absolute`, `workspace_root`, `normalize_separators`, and configuration-related variables like `config` and `path`—define the normalization context and behavior. The implementation relies on the `config:schema` module for its core configuration types and on the standard library.

## Imports

- [`config:schema`](schema.md)
- `std`

## Types

### `clore::config::NormalizeError`

Declaration: `config/normalize.cppm:10`

Definition: `config/normalize.cppm:10`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The struct `clore::config::NormalizeError` is implemented as a trivially-defined error type holding a single `std::string` member `message`. There are no user-declared constructors, destructors, or assignment `operator`s; the compiler‑generated special members suffice. No invariants are enforced beyond the default guarantees of `std::string`; the `message` field is expected to contain a human‑readable description of the normalization failure. Because the struct provides no further member functions or data, its role is limited to carrying an error string through the conventional error‑reporting path. The absence of any validation or formatting logic makes it an intentionally minimal error representation.

#### Invariants

- No explicit invariants are documented.

#### Key Members

- `message`: a `std::string` describing the error.

#### Usage Patterns

- Used to report errors during configuration normalization.

## Functions

### `clore::config::normalize`

Declaration: `config/normalize.cppm:14`

Definition: `config/normalize.cppm:22`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The function `clore::config::normalize` processes a `TaskConfig` object by converting all path fields to absolute, normalized forms and ensuring consistent forward-slash separators. It first checks `config.workspace_root`; if empty, it defaults to `std::filesystem::current_path()`. The internal lambda `make_absolute` then converts each relevant path to an absolute path: for `workspace_root` it uses `std::filesystem::absolute`, while for `compile_commands_path`, `project_root`, and `output_root` it resolves relative paths against the now-absolute `workspace_root` (via `base / p`). If any path is empty, `make_absolute` returns a `std::unexpected` with a `NormalizeError` containing a descriptive `message` via `std::format`. After all absolute conversions succeed, a second lambda `normalize_separators` iterates over every character in the path string, replacing backslash `'\\'` with forward slash `'/'`. This normalization is applied not only to the four primary path fields but also to every element in `config.filter.include` and `config.filter.exclude`.

#### Side Effects

- modifies the input `TaskConfig` object in-place
- normalizes path separators from backslash to forward slash
- resolves relative paths to absolute paths

#### Reads From

- config`.workspace_root`
- config`.compile_commands_path`
- config`.project_root`
- config`.output_root`
- config`.filter``.include`
- config`.filter``.exclude`
- `std::filesystem::current_path()`

#### Writes To

- config`.workspace_root`
- config`.compile_commands_path`
- config`.project_root`
- config`.output_root`
- config`.filter``.include`
- config`.filter``.exclude`

#### Usage Patterns

- called to normalize a `TaskConfig` after loading
- ensures all path fields are absolute and use forward slashes

## Internal Structure

The `config:normalize` module is a thin, focused layer that processes a single configuration value at a time. It imports the `config:schema` module to understand the structure of configuration data and the `std` module for fundamental types and utilities like `std::string`. Internally, it defines a `NormalizeError` struct (with a public `message` field) to carry error information and exposes a single public function, `clore::config::normalize`, which accepts a mutable reference to an integer configuration value and returns an integer status code. The module’s implementation is built around several file‑local variables—such as `normalize_separators` for separator‑replacement logic, `workspace_root` and `make_absolute` for resolving relative paths, and multiple `r` variables that likely act as temporary result holders. This decomposition keeps the main function’s logic modular without exposing internal helpers, while the error type allows callers to distinguish success from specific issues during normalization.

## Related Pages

- [Module config:schema](schema.md)

