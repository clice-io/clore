---
title: 'Module config:validate'
description: 'The module config:validate is responsible for verifying that configuration values conform to the expected constraints defined by the application''s schema. It exposes the public function clore::config::validate, which accepts a reference to a constant integer (representing a configuration value) and returns an integer status code indicating success (zero) or a specific error condition (non-zero). The module also provides the public struct ValidationError that carries a descriptive message string. Within the module, internal helper functions such as validate_nonzero and validate_nonempty support the validation logic for common checks. This module depends on config:schema and the standard library.'
layout: doc
template: doc
---

# Module `config:validate`

## Summary

The module `config:validate` is responsible for verifying that configuration values conform to the expected constraints defined by the application's schema. It exposes the public function `clore::config::validate`, which accepts a reference to a constant integer (representing a configuration value) and returns an integer status code indicating success (zero) or a specific error condition (non-zero). The module also provides the public struct `ValidationError` that carries a descriptive `message` string. Within the module, internal helper functions such as `validate_nonzero` and `validate_nonempty` support the validation logic for common checks. This module depends on `config:schema` and the standard library.

## Imports

- [`config:schema`](schema.md)
- `std`

## Types

### `clore::config::ValidationError`

Declaration: `config/validate.cppm:8`

Definition: `config/validate.cppm:8`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The struct `clore::config::ValidationError` is implemented as a lightweight wrapper holding a single `std::string` member named `message`. There are no additional invariants or special member functions; the struct relies on the default compiler‑generated constructors, destructor, and assignment `operator`s. Its sole purpose is to carry a human‑readable description of a validation problem, making it suitable for use in error‑reporting flows within the configuration validation subsystem.

#### Invariants

- The `message` member is a `std::string` with no additional constraints imposed by the struct.

#### Key Members

- `message` stores the error description.

#### Usage Patterns

- Returned or thrown by validation functions to indicate a configuration error.
- Likely compared or logged by callers to understand the validation failure.

## Functions

### `clore::config::validate`

Declaration: `config/validate.cppm:12`

Definition: `config/validate.cppm:42`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The implementation of `clore::config::validate` performs field‑by‑field validation on the input `TaskConfig` and returns a `std::expected<void, ValidationError>` that signals either success (an empty expected) or the first detected error. The algorithm follows a sequential short‑circuit pattern: it checks each required field for emptiness using `std::string::empty`, then for filesystem existence (`std::filesystem::exists`), and finally for the correct file type (`std::filesystem::is_regular_file` or `std::filesystem::is_directory`). For `output_root`, a non‑empty value must either not exist or be a directory. After the core filesystem checks, the LLM sub‑configuration is validated by calling two internal helpers—`validate_nonempty` on `config.llm.system_prompt` and `validate_nonzero` on `config.llm.retry_limit`. Each helper returns a `std::expected`; if either fails, the function immediately propagates that result. The only dependencies are the `std::filesystem` and `std::format` libraries and the two anonymous‑namespace validation functions.

#### Side Effects

- Reads file system state to check existence and type of paths specified in the config.

#### Reads From

- const `TaskConfig`& config
- File system state for paths: `compile_commands_path`, `project_root`, `output_root`
- Fields `llm.system_prompt` and `llm.retry_limit` via helpers

#### Usage Patterns

- Called after constructing or loading a `TaskConfig` to ensure configuration validity before use.
- Returned expected is typically checked with error handling, e.g., logging or propagating the `ValidationError`.

## Internal Structure

The `config:validate` module is responsible for verifying that configuration values conform to the constraints defined by the `config:schema` module. It imports `config:schema` to access schema types such as `FilterRule`, `LLMConfig`, and `TaskConfig`, and uses standard library facilities via `std`. Internally, the module is decomposed into a public function `clore::config::validate` and a set of helper functions placed in an anonymous namespace to enforce internal linkage. These helpers, such as `validate_nonzero` and `validate_nonempty`, implement specific validation rules against individual configuration fields supplied as function parameters. The public `validate` function orchestrates these helpers, applying them to the configuration reference it receives and returning an integer status code that signals success (zero) or a specific error condition (non-zero). The module also defines a `ValidationError` struct with a public `message` field to communicate error details to callers. This layered structure separates low‑level validation logic from the high‑level entry point, keeping the implementation modular and testable.

## Related Pages

- [Module config:schema](schema.md)

