---
title: 'Module config:validate'
description: 'The config:validate module is responsible for checking whether a configuration value satisfies the applicable validation rules. Its public interface consists of the function clore::config::validate, which accepts a const int & and returns an integer status code (zero for success, non‑zero for an error identifier), and the struct clore::config::ValidationError, which contains a message field to describe the failure. Internally, the module relies on anonymous‑namespace helpers validate_nonzero and validate_nonempty to perform specific constraint checks. It depends on the config:schema module for schema definitions and on the C++ standard library.'
layout: doc
template: doc
---

# Module `config:validate`

## Summary

The `config:validate` module is responsible for checking whether a configuration value satisfies the applicable validation rules. Its public interface consists of the function `clore::config::validate`, which accepts a `const int &` and returns an integer status code (zero for success, non‑zero for an error identifier), and the struct `clore::config::ValidationError`, which contains a `message` field to describe the failure. Internally, the module relies on anonymous‑namespace helpers `validate_nonzero` and `validate_nonempty` to perform specific constraint checks. It depends on the `config:schema` module for schema definitions and on the C++ standard library.

## Imports

- [`config:schema`](schema.md)
- `std`

## Types

### `clore::config::ValidationError`

Declaration: `config/validate.cppm:8`

Definition: `config/validate.cppm:8`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The struct `clore::config::ValidationError` is implemented as a trivial value type containing a single data member `message` of type `std::string`. No invariants beyond basic string validity are enforced; the default constructor and copy/move operations are compiler-generated, and the `message` member is directly accessible for both reading and writing. The struct serves solely as a lightweight carrier for a human-readable description of a validation failure, with no additional metadata, error codes, or nesting.

#### Invariants

- The `message` member always contains a non-empty string when used
- No other members or state exist

#### Key Members

- `clore::config::ValidationError::message`

#### Usage Patterns

- Returned from validation functions to indicate failure
- Used as the error type in `std::expected` or similar patterns

## Functions

### `clore::config::validate`

Declaration: `config/validate.cppm:12`

Definition: `config/validate.cppm:42`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The implementation of `clore::config::validate` performs a series of sequential validation checks against the provided `TaskConfig` object. Each required field (`compile_commands_path`, `project_root`, `output_root`) is first tested for emptiness; if empty, the function returns a `std::unexpected` containing a `ValidationError` with an appropriate message. For fields that are not empty, filesystem existence and type checks are performed using `std::filesystem::exists` and `std::filesystem::is_regular_file` or `std::filesystem::is_directory`, returning an error on failure. The `output_root` field is only validated if it already exists (a missing directory is permitted). After the filesystem checks, the LLM sub‑configuration is validated by calling the local helper functions `validate_nonempty` on `config.llm.system_prompt` and `validate_nonzero` on `config.llm.retry_limit`; if either returns an unexpected result, that result is propagated. On successful completion of all checks, the function returns a default‑constructed `std::expected` (empty value).

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `TaskConfig` parameter
- filesystem state (via `std::filesystem::exists`, `is_regular_file`, `is_directory`)

#### Usage Patterns

- Called after loading configuration to ensure validity before use
- May be called on both initial configuration and after modifications

## Internal Structure

The module `config:validate` depends externally on `std` for basic utilities and on `config:schema` for the core schema types (`TaskConfig`, `LLMConfig`, `FilterRule`). It imports these via module imports, establishing a clear dependency from validation logic to the schema definition layer without circular coupling.

Internally, the module is decomposed into a public entry point and private helper functions in an anonymous namespace. The public function `clore::config::validate` accepts a configuration value (represented as `const int &` for the scalar case) and returns an `int` status code: `0` for valid, non‑zero for an error identifier. The anonymous namespace contains specialized validation routines—`validate_nonzero` and `validate_nonempty`—each handling a particular constraint logic. These helpers are invoked iteratively as the validator walks the fields of the schema struct, using the variable names `config`, `field`, and `value` to track the current object, schema field definition, and runtime value respectively. The struct `ValidationError` is used to capture failure messages, likely attached to a returned error identifier. This design isolates constraint implementations from the traversal logic, keeping each validation rule focused and testable.

## Related Pages

- [Module config:schema](schema.md)

