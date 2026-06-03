---
title: 'Module config:load'
description: 'The config:load module is responsible for reading and parsing TOML configuration data, validating its structure, and exposing the result as an integer handle for use by other configuration utilities. It provides two public entry points: clore::config::load_config, which accepts a file path, and clore::config::load_config_from_string, which processes a configuration string directly. Both functions return an int that acts as a configuration identifier; a non‑negative value indicates success, while a negative value signals an error, which the caller should check before using the handle. Internally, the module parses the TOML content into a RawTaskConfig struct (containing fields like llm and filter), rejects any unknown top‑level keys, and then converts the raw config into the returned handle. It relies on the config:schema module for the configuration data structures and on the support module for foundational utilities. The ConfigError struct, with its message field, is used to report validation or parsing failures.'
layout: doc
template: doc
---

# Module `config:load`

## Summary

The `config:load` module is responsible for reading and parsing TOML configuration data, validating its structure, and exposing the result as an integer handle for use by other configuration utilities. It provides two public entry points: `clore::config::load_config`, which accepts a file path, and `clore::config::load_config_from_string`, which processes a configuration string directly. Both functions return an `int` that acts as a configuration identifier; a non‑negative value indicates success, while a negative value signals an error, which the caller should check before using the handle. Internally, the module parses the TOML content into a `RawTaskConfig` struct (containing fields like `llm` and `filter`), rejects any unknown top‑level keys, and then converts the raw config into the returned handle. It relies on the `config:schema` module for the configuration data structures and on the `support` module for foundational utilities. The `ConfigError` struct, with its `message` field, is used to report validation or parsing failures.

## Imports

- [`config:schema`](schema.md)
- [`support`](../support/index.md)

## Types

### `clore::config::ConfigError`

Declaration: `src/config/load.cppm:30`

Definition: `src/config/load.cppm:30`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

`clore::config::ConfigError` is an aggregate struct containing a single `std::string` member `message`. No special member functions are user‑defined; the compiler generates default construction, destruction, copy, and move operations. The struct does not enforce any internal invariants; `message` is a free‑form description. The implementation’s simplicity reflects its role as a lightweight container for error text, with no additional logic or data validation.

#### Key Members

- `message`

#### Usage Patterns

- Returned as an error type from config loading functions
- Caught or handled in code that processes configuration

## Functions

### `clore::config::load_config`

Declaration: `src/config/load.cppm:34`

Definition: `src/config/load.cppm:96`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The function begins by converting the incoming `path` to an `std::filesystem::path` (`config_path`), resolving it to an absolute path via `fs::absolute` if relative, then normalising with `lexically_normal`. It checks existence of the resolved file; if missing, it returns an error wrapped in `std::unexpected` with a `ConfigError` containing a formatted message. Otherwise, it delegates file I/O to `clore::support::read_utf8_text_file`, which returns an `std::optional` of the file content. A read failure also yields a `ConfigError`.

After successfully obtaining the file content, the function calls `load_config_from_string` on the content to parse and validate the TOML configuration. If parsing fails, the error from `load_config_from_string` is forwarded directly. On success, it sets the `workspace_root` field of the resulting `TaskConfig` to the parent directory of the resolved configuration file, thereby anchoring relative paths that may appear inside the config.

#### Side Effects

- reads a file from the filesystem
- allocates memory for the configuration object and read content
- sets the `workspace_root` field of the config

#### Reads From

- parameter `path`
- filesystem (via `fs::exists` and `read_utf8_text_file`)
- error message from `content.error()`

#### Writes To

- local variable `config_path`
- local variable `content`
- local variable `config`
- the `workspace_root` field of the returned config

#### Usage Patterns

- load a task configuration file for an application
- retrieve configuration from a user-specified path

### `clore::config::load_config_from_string`

Declaration: `src/config/load.cppm:36`

Definition: `src/config/load.cppm:125`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The implementation begins by normalizing the input string via `clore::support::strip_utf8_bom` to remove any byte‑order mark, then parses the normalized TOML text into a `::toml::table` using `::toml::parse`. If parsing fails, the caught `::toml::parse_error` is converted into a `clore::config::ConfigError` with its description, and the function returns `std::unexpected`. After successful parsing, `reject_unknown_top_level_keys` validates that only recognized keys appear in the table; if unknown keys are present, the validation returns an error which is forwarded immediately.

The refined table is then deserialized into a temporary `clore::config::(anonymous namespace)::RawTaskConfig` via `toml_codec::from_toml`. If the deserialization fails, the error message from the codec is wrapped in a `ConfigError` and returned. Finally, the raw config is moved into `to_config`, which converts it into the final `TaskConfig` and returns the expected value. Error propagation throughout uses `std::expected` with `std::unexpected` for all failure paths, giving a consistent, monadic control flow.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `toml_content` parameter
- global `toml::parse` library state (internal)

#### Usage Patterns

- Called to load a configuration from a TOML string
- Used in config loading pipelines

## Internal Structure

The `config:load` module serves as the public entry point for parsing project configuration from either a file path or a raw string. It depends on the `config:schema` module for the canonical `TaskConfig` data model and on the `support` module for foundational utilities such as file I/O and string normalization. Internally, the module is decomposed into two layers: a TOML parsing and validation layer and a conversion layer. The parsing layer first reads or accepts the configuration text, normalizes it, and parses it into a `toml::table`. It then validates the table by using `reject_unknown_top_level_keys` against an allowed set, ensuring no accidental or unsupported keys are present. The validated table is mapped onto an intermediate `RawTaskConfig` struct (which holds the `llm` and `filter` fields) before being transformed by `to_config` into the schema‑defined `TaskConfig`. The final result is returned as an `int` handle that subsequent configuration operations can reference. This two‑stage design separates concerns: the anonymous namespace encapsulates all parsing and validation logic while the public interface exposes only the two convenience functions. Error conditions are signalled via a `ConfigError` struct (carrying a descriptive `message`) embedded in a `std::expected` return type, keeping the API lean and error‑aware.

## Related Pages

- [Module config:schema](schema.md)
- [Module support](../support/index.md)

