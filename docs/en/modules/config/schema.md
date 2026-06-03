---
title: 'Module config:schema'
description: 'The config:schema module defines the core data structures that represent the project''s configuration model. It includes the TaskConfig struct, which holds runtime settings such as project and workspace root paths, output directory, compile commands location, file filtering rules, and LLM configuration. Supporting types like LLMConfig and FilterRule specify language model parameters and include/exclude patterns for file processing. These types collectively form the schema that external configuration files (e.g., YAML or JSON) are deserialized into, and are used throughout the codebase to access validated configuration values.'
layout: doc
template: doc
---

# Module `config:schema`

## Summary

The `config:schema` module defines the core data structures that represent the project's configuration model. It includes the `TaskConfig` struct, which holds runtime settings such as project and workspace root paths, output directory, compile commands location, file filtering rules, and LLM configuration. Supporting types like `LLMConfig` and `FilterRule` specify language model parameters and include/exclude patterns for file processing. These types collectively form the schema that external configuration files (e.g., YAML or JSON) are deserialized into, and are used throughout the codebase to access validated configuration values.

## Imported By

- [`config:load`](load.md)
- [`config:normalize`](normalize.md)
- [`config:validate`](validate.md)

## Types

### `clore::config::FilterRule`

Declaration: `src/config/schema.cppm:13`

Definition: `src/config/schema.cppm:13`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The struct `clore::config::FilterRule` is an aggregate type with two public data members: `include` and `exclude`, both of type `std::vector<std::string>`. There are no custom constructors, destructors, or member functions; the struct relies on compiler-generated defaults. The only internal invariant is that each vector holds its own independent sequence of strings; no cross-constraints between the two lists are enforced by the type itself. This flat design makes `FilterRule` a simple container for storing filter patterns, leaving the interpretation or ordering of include‑then‑exclude logic to the code that consumes the struct.

#### Invariants

- Each vector holds pattern strings
- Intended to be used together for include/exclude logic

#### Key Members

- include
- exclude

#### Usage Patterns

- Retrieved or populated from configuration sources
- Used to filter sets of items based on pattern matching

### `clore::config::LLMConfig`

Declaration: `src/config/schema.cppm:18`

Definition: `src/config/schema.cppm:18`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The struct `clore::config::LLMConfig` is implemented as a plain aggregate data type, consisting of two public member variables: `system_prompt` of type `std::string` and `retry_limit` of type `std::uint32_t` with a default member initializer of `0`. No invariants are enforced beyond the intrinsic properties of these types—the unsigned integer type guarantees a non-negative value, and the string may be empty. The struct does not declare any special member functions, so it relies on implicitly defined default construction, copy, move, and assignment operations, behaving as a trivial configuration bundle that can be initialized via aggregate initialization.

#### Invariants

- `retry_limit` is a non-negative integer (unsigned 32-bit)
- `system_prompt` is a string that can be empty or contain arbitrary text

#### Key Members

- `system_prompt`
- `retry_limit`

#### Usage Patterns

- Passed to functions that interact with an LLM to influence generation behavior
- Defaults to zero retries, meaning no retry is attempted unless explicitly configured

### `clore::config::TaskConfig`

Declaration: `src/config/schema.cppm:23`

Definition: `src/config/schema.cppm:23`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The struct `clore::config::TaskConfig` is a plain aggregate that bundles all configuration data required for a single analysis task. Its public members comprise four file‑system path strings—`project_root`, `workspace_root`, `output_root`, and `compile_commands_path`—plus two sub‑configuration objects: `filter` of type `FilterRule` and `llm` of type `LLMConfig`. No invariants are enforced by the struct itself; fields are default‑constructed (strings become empty, nested objects take their defaults) and consistency among paths (e.g., that `compile_commands_path` lies inside `project_root`) is the responsibility of the surrounding code. The implementation is purely declarative, acting as a container for deserialized task settings, and offers no methods beyond the implicitly generated ones.

#### Invariants

- Paths are expected to be valid filesystem paths but no validation is present.
- `filter` and `llm` are fully owned sub-configurations.

#### Key Members

- `compile_commands_path`: path to `compile_commands``.json`
- `project_root`: root directory of the project
- `output_root`: output directory for results
- `workspace_root`: workspace directory
- `filter`: filtering rules for the task
- `llm`: LLM configuration for the task

#### Usage Patterns

- Constructed by config parsing logic and passed to task execution.
- Accessed by task runners to read paths and sub-configurations.

## Internal Structure

The `config:schema` module defines the core data structures that represent the user-facing configuration model. It is decomposed into three plain structs—`FilterRule`, `LLMConfig`, and `TaskConfig`—each with only public fields, making the module a pure data contract with no encapsulated behavior. `FilterRule` groups include and exclude path patterns, `LLMConfig` holds the system prompt and retry limit, and `TaskConfig` aggregates all settings: compile commands path, project, workspace and output roots, along with the LLM configuration and filter rule. This module has minimal external imports (likely only standard types) and serves as the foundational layer that other config modules (e.g., parsing, validation) depend on, ensuring a stable interface between configuration sources and consumers.

