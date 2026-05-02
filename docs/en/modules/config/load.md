---
title: 'Module config:load'
description: 'The config:load module is responsible for deserializing configuration data from external sources—either a file path or an inline TOML string—and converting it into the internal configuration structures defined by the config:schema module. It provides two primary public entry points: load_config (which accepts a file path) and load_config_from_string (which accepts a raw TOML string). Both return an int status code, where zero indicates success and non‑zero signals an error. The module also defines the ConfigError struct to represent error information when validation fails, and uses internal helpers such as reject_unknown_top_level_keys and RawTaskConfig to parse and validate the incoming TOML content against the expected schema.'
layout: doc
template: doc
---

# Module `config:load`

## Summary

The `config:load` module is responsible for deserializing configuration data from external sources—either a file path or an inline TOML string—and converting it into the internal configuration structures defined by the `config:schema` module. It provides two primary public entry points: `load_config` (which accepts a file path) and `load_config_from_string` (which accepts a raw TOML string). Both return an `int` status code, where zero indicates success and non‑zero signals an error. The module also defines the `ConfigError` struct to represent error information when validation fails, and uses internal helpers such as `reject_unknown_top_level_keys` and `RawTaskConfig` to parse and validate the incoming TOML content against the expected schema.

## Imports

- [`config:schema`](schema.md)
- `std`
- [`support`](../support/index.md)

## Types

### `clore::config::ConfigError`

Declaration: `config/load.cppm:15`

Definition: `config/load.cppm:15`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The struct `clore::config::ConfigError` is implemented as a lightweight value type containing a single `std::string` member `message`. This field stores a human-readable description of the error condition, with no additional internal invariants beyond those of `std::string`. No custom constructors, assignment `operator`s, or other special member functions are provided; the compiler-generated defaults suffice. The representation is intentionally minimal to serve as a simple error carrier within the configuration loading subsystem.

#### Invariants

- The `message` member can be any string, including an empty string.
- No other members or base classes exist.

#### Key Members

- `message`

#### Usage Patterns

- Thrown as an exception to signal configuration errors.
- Returned as an error value from configuration parsing functions.
- The `message` member is read by error-handling code to display or log the error.

## Functions

### `clore::config::load_config`

Declaration: `config/load.cppm:19`

Definition: `config/load.cppm:81`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The implementation of `clore::config::load_config` begins by normalizing the input `path` through `std::filesystem`. A relative `path` is resolved to an absolute path via `fs::absolute`, and the result is lexically normalized. If the file does not exist at `config_path`, an error `ConfigError` with a descriptive message is returned immediately. Otherwise, the file contents are read using `clore::support::read_utf8_text_file`; a read failure also produces an error.

On successful file reading, the obtained `content` string is forwarded to `clore::config::load_config_from_string`. The returned `config` (of type `std::expected<TaskConfig, ConfigError>`) is checked: if it holds a value, the `config->workspace_root` is set to the parent directory of `config_path` (as a string). The function then returns the fully populated `config` object. This design separates file I/O from TOML parsing, relying on `load_config_from_string` for the actual configuration interpretation.

#### Side Effects

- reads a configuration file from the filesystem
- allocates and constructs `TaskConfig` or `ConfigError` objects with ownership transfer to caller

#### Reads From

- parameter `path` (`std::string_view`)
- filesystem content via `clore::support::read_utf8_text_file`
- filesystem status via `std::filesystem::exists`
- result of `clore::config::load_config_from_string`

#### Writes To

- returned `std::expected<TaskConfig, ConfigError>` object (includes modifying `workspace_root` and constructing error values)
- temporary `ConfigError` objects via `std::format`

#### Usage Patterns

- load configuration at program startup
- parse a configuration file given its path

### `clore::config::load_config_from_string`

Declaration: `config/load.cppm:21`

Definition: `config/load.cppm:110`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The function first normalizes the input by stripping a UTF-8 BOM via `clore::support::strip_utf8_bom`, then parses the resulting string into a `::toml::table`. If parsing throws a `::toml::parse_error`, the function returns `std::unexpected` with a `ConfigError` capturing the error description. If parsing succeeds, `reject_unknown_top_level_keys` is called on the table; this helper, defined in the anonymous namespace, validates that every key in the table belongs to the set of `allowed_keys` and returns an error when an unknown key is encountered. After that check passes, `toml_codec::from_toml` decodes the table into a local `RawTaskConfig` object, again returning an error if decoding fails. Finally, the raw config is moved into `to_config`, which converts it to the public `TaskConfig` type, and the resulting value is returned wrapped in `std::expected`. The control flow is entirely linear with early‑exit error handling; key dependencies include the TOML parser, the `kota::codec::toml` deserializer, and the internal `RawTaskConfig` and `ConfigError` types.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `toml_content`

#### Usage Patterns

- called with TOML string from file contents or user input
- used to deserialize configuration in unit tests and programmatic config loading

## Internal Structure

The `config:load` module is decomposed into a public entry-point layer and an internal parsing and validation layer. The public API consists of two functions—`load_config` and `load_config_from_string`—both accepting a `std::string_view` and returning an `int` status code. Internally, the module uses an anonymous namespace to host a raw intermediate representation (`RawTaskConfig` with fields `llm` and `filter`) and a private conversion function `to_config` that transforms this raw form into the final configuration structures defined by the imported `config:schema` module. A `ConfigError` struct carries error messages, while helper functions like `reject_unknown_top_level_keys` validate the parsed TOML table against an allowed set of keys.  

The module imports three dependencies: `std` for standard library facilities, `config:schema` for the canonical configuration data types, and `support` for foundational utilities (e.g., logging, path handling). Layering proceeds from I/O (reading TOML content) to normalization (`normalized_toml`), then to syntactic validation against allowed keys, and finally to semantic conversion into the schema types. The implementation structure separates the public API surface from the private parsing logic, ensuring that callers interact only through the simple load functions while complex parsing and validation remain encapsulated.

## Related Pages

- [Module config:schema](schema.md)
- [Module support](../support/index.md)

