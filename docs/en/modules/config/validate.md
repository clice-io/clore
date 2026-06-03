---
title: 'Module config:validate'
description: 'The module config:validate is responsible for validating configuration values against the project’s configuration schema. Its public interface consists of the clore::config::validate function, which verifies that a given integer configuration value is valid, returning either success or a ValidationError that describes the failure. The module also exposes the ValidationError struct, which contains a message field to communicate the reason for a validation failure.'
layout: doc
template: doc
---

# Module `config:validate`

## Summary

The module `config:validate` is responsible for validating configuration values against the project’s configuration schema. Its public interface consists of the `clore::config::validate` function, which verifies that a given integer configuration value is valid, returning either success or a `ValidationError` that describes the failure. The module also exposes the `ValidationError` struct, which contains a `message` field to communicate the reason for a validation failure.

## Imports

- [`config:schema`](schema.md)

## Types

### `clore::config::ValidationError`

Declaration: `src/config/validate.cppm:14`

Definition: `src/config/validate.cppm:14`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The internal structure of `clore::config::ValidationError` consists solely of a `std::string` member `message`, which stores a human‑readable description of the validation failure. The struct relies on the default member‑wise copy/move semantics and trivial destructor provided by the compiler, as no custom special member functions are declared. No invariants are enforced beyond those inherent to `std::string`; the `message` may be empty to represent a missing or non‑informative error, though in practice callers are expected to supply a meaningful value. The simplicity of the implementation makes `clore::config::ValidationError` a lightweight value type suitable for propagation in result types or exception‑like flows.

#### Invariants

- `message` contains a textual description of the validation error.
- The struct is trivially constructible and copyable.

#### Key Members

- `message`

#### Usage Patterns

- Returned from config validation functions to indicate a specific validation failure.
- Carried inside `std::expected` or similar error-handling types.

## Functions

### `clore::config::validate`

Declaration: `src/config/validate.cppm:18`

Definition: `src/config/validate.cppm:48`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The function `clore::config::validate` performs a sequential series of validation steps on the input `TaskConfig` and returns `std::expected<void, ValidationError>`. The algorithm checks filesystem paths first: it verifies that `compile_commands_path` is non‑empty, exists on disk, and is a regular file; then it checks that `project_root` is non‑empty, exists, and is a directory; finally it checks that `output_root` is non‑empty and, if it already exists, that it is a directory. For the LLM sub‑configuration it delegates to the anonymous‑namespace helpers `validate_nonzero` for `llm.retry_limit` and `validate_nonempty` for `llm.system_prompt`. On the first violation the function returns `std::unexpected` containing a `ValidationError` with a descriptive message; if all checks pass it returns a valid `std::expected`. Control flow is purely sequential with early exit on the first error, and all error messages are constructed using `std::format`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- config`.compile_commands_path`
- config`.project_root`
- config`.output_root`
- config`.llm``.system_prompt`
- config`.llm``.retry_limit`

#### Usage Patterns

- called after loading config from file or string
- used before proceeding with task execution

## Internal Structure

The `config:validate` module is responsible for validating configuration values against domain-specific constraints, building on the types defined in the `config:schema` module. Its public interface consists of the `validate` function, which accepts a `const int &` and returns a `std::expected<void, ValidationError>`, and the `ValidationError` struct that carries a descriptive `message` string when validation fails.

Internally, the module uses two anonymous‑namespace helper functions—`validate_nonzero` and `validate_nonempty`—to perform specific checks on numeric and string fields respectively, each returning the same result type. This decomposition keeps the validation logic modular and testable, with each helper focusing on a single invariant. The overall implementation structure separates the public validation entry point from the private, type‑specific validation routines, while relying on the schema module solely for its type definitions rather than any behavior.

## Related Pages

- [Module config:schema](schema.md)

