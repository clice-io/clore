---
title: 'Module config:load'
description: 'The config:load module is responsible for deserializing and validating TOML configuration data, providing two public entry points: load_config for loading from a file path, and load_config_from_string for loading from an in‑memory string. Both return an integer status code that the caller must check to determine success. Internally, the module parses the TOML input, rejects unknown top‑level keys, and populates a RawTaskConfig intermediate representation before converting it into the schema types defined by the sibling config:schema module. Errors encountered during loading are captured as ConfigError objects, which carry a descriptive message string.'
layout: doc
template: doc
---

# Module `config:load`

## Summary

The `config:load` module is responsible for deserializing and validating TOML configuration data, providing two public entry points: `load_config` for loading from a file path, and `load_config_from_string` for loading from an in‑memory string. Both return an integer status code that the caller must check to determine success. Internally, the module parses the TOML input, rejects unknown top‑level keys, and populates a `RawTaskConfig` intermediate representation before converting it into the schema types defined by the sibling `config:schema` module. Errors encountered during loading are captured as `ConfigError` objects, which carry a descriptive message string.

## Imports

- [`config:schema`](schema.md)
- `std`
- [`support`](../support/index.md)

## Types

### `clore::config::ConfigError`

Declaration: `config/load.cppm:15`

Definition: `config/load.cppm:15`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The type `clore::config::ConfigError` is implemented as a simple value type holding a single `std::string` member `message`. There are no invariants beyond the usual string validity; the struct exists solely to bundle an error description in a dedicated type, enabling idiomatic error handling (e.g., via `std::expected` or exceptions) without exposing raw strings. No constructors, assignment `operator`s, or member functions are defined, making it a transparent wrapper that relies on default compiler-generated operations for copy, move, and comparison.

#### Invariants

- message contains descriptive error text

#### Key Members

- message

#### Usage Patterns

- thrown or returned as an error from configuration operations
- message is retrieved for logging or display

## Functions

### `clore::config::load_config`

Declaration: `config/load.cppm:19`

Definition: `config/load.cppm:81`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The implementation of `clore::config::load_config` begins by resolving the input `path` into an absolute, lexically normalized `config_path` using `std::filesystem`. If the file does not exist, it immediately returns a `ConfigError` with a descriptive message. Otherwise, it reads the file contents by calling `clore::support::read_utf8_text_file`. If reading fails, it returns another `ConfigError` carrying the underlying error string.

After obtaining the file content, the function delegates to `load_config_from_string` to parse the TOML and produce a `TaskConfig` value. On success, it sets the `workspace_root` member of the returned config to the parent directory of `config_path` (as a string). The entire flow uses early returns with `std::unexpected` for error propagation, relying on `ConfigError` as the common error type. No further validation or transformation is performed at this level; the heavy lifting occurs in `load_config_from_string`.

#### Side Effects

- reads from filesystem
- allocates memory for file content
- mutates returned `TaskConfig` object's `workspace_root` field

#### Reads From

- file system at given `path`
- file content via `clore::support::read_utf8_text_file`
- result of `clore::config::load_config_from_string`

#### Writes To

- return value of type `std::expected<TaskConfig, ConfigError>`
- `.workspace_root` field of returned `TaskConfig`

#### Usage Patterns

- loading configuration from a file path
- error handling with `std::expected` return type
- setting workspace root automatically from config file location

### `clore::config::load_config_from_string`

Declaration: `config/load.cppm:21`

Definition: `config/load.cppm:110`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The implementation first normalizes the input by stripping a UTF-8 BOM via `clore::support::strip_utf8_bom`. It then parses the normalized string into a `::toml::table` using `::toml::parse`, catching `::toml::parse_error` and returning an unexpected `ConfigError` on failure. After parsing, it calls the local helper `reject_unknown_top_level_keys` to validate that no top-level keys outside the allowed set are present; if any are found, the function returns an error immediately.

Once the table passes validation, the function decodes its contents into a `RawTaskConfig` using `toml_codec::from_toml`. If decoding fails, it again produces an appropriate `ConfigError`. Finally, the raw configuration is transformed into the public `TaskConfig` type by invoking `to_config` on the moved `raw` object. The entire flow depends on the TOML parsing library, the custom validation and decoding helpers, and the conversion step that assembles the final configuration structure.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `toml_content` parameter
- global TOML parser state

#### Writes To

- returned `std::expected<TaskConfig, ConfigError>` object

#### Usage Patterns

- loading configuration from a string
- testing with inline TOML
- processing embedded configuration

## Internal Structure

The `config:load` module is decomposed into a public API layer and an internal implementation hidden in an anonymous namespace. Two public entry points are provided: `load_config` for file‑based loading and `load_config_from_string` for direct string input; both return an `int` status code. Internally, the module defines a `RawTaskConfig` struct (with fields for LLM and filter settings) and a battery of helper functions—`to_config`, `reject_unknown_top_level_keys`—that handle TOML parsing, key validation, and conversion from the raw intermediate shape to the final configuration types imported from `config:schema`. The implementation relies on the `support` module for foundational utilities (file I/O, logging) and on `std` for standard library facilities, maintaining a clear separation between parsing, validation, and conversion stages.

## Related Pages

- [Module config:schema](schema.md)
- [Module support](../support/index.md)

