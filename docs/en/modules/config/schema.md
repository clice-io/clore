---
title: 'Module config:schema'
description: 'The module config:schema defines the core data structures that represent the configuration schema for the clore system. It exposes three public structs: FilterRule, LLMConfig, and TaskConfig. These types collectively specify how tasks are configured, including file inclusion/exclusion filters, language model parameters (system prompt and retry limit), and essential filesystem paths (compile commands, project root, workspace root, and output root). This module is the single source of truth for the shape and names of all user-facing configuration options, serving as the public interface for configuration parsing and validation.'
layout: doc
template: doc
---

# Module `config:schema`

## Summary

The module `config:schema` defines the core data structures that represent the configuration schema for the clore system. It exposes three public structs: `FilterRule`, `LLMConfig`, and `TaskConfig`. These types collectively specify how tasks are configured, including file inclusion/exclusion filters, language model parameters (system prompt and retry limit), and essential filesystem paths (compile commands, project root, workspace root, and output root). This module is the single source of truth for the shape and names of all user-facing configuration options, serving as the public interface for configuration parsing and validation.

## Imports

- `std`

## Imported By

- [`config:load`](load.md)
- [`config:normalize`](normalize.md)
- [`config:validate`](validate.md)

## Types

### `clore::config::FilterRule`

Declaration: `config/schema.cppm:7`

Definition: `config/schema.cppm:7`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The `clore::config::FilterRule` struct is a trivial aggregate that stores two `std::vector<std::string>` fields: `include` and `exclude`. These vectors hold the inclusion and exclusion patterns respectively; no invariants are enforced beyond the default state of an empty vector. The struct relies on compiler-generated default constructors, destructor, and assignment `operator`s, making its implementation minimal and purely data‑driven.

#### Invariants

- No explicit invariants are documented; the members are independent `std::vector<std::string>` with no specified constraints.

#### Key Members

- `clore::config::FilterRule::include`
- `clore::config::FilterRule::exclude`

#### Usage Patterns

- Used as a data-only configuration type to specify inclusion and exclusion patterns for filtering operations.

### `clore::config::LLMConfig`

Declaration: `config/schema.cppm:12`

Definition: `config/schema.cppm:12`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The struct `clore::config::LLMConfig` is a plain data aggregate with two public members. The field `retry_limit` is a `std::uint32_t` that explicitly defaults to `0`, establishing the invariant that by default no retries are configured. The field `system_prompt` is a `std::string` that is value‑initialized to an empty string, which is a natural default for an optional text directive. There are no custom constructors, assignment `operator`s, or validation logic; the struct relies entirely on member initializers and the language‑provided defaults to maintain a consistent starting state.

#### Invariants

- `retry_limit` defaults to `0`
- `system_prompt` is a default-constructed `std::string` (empty)

#### Key Members

- `system_prompt`
- `retry_limit`

#### Usage Patterns

- Other code creates, reads, or modifies `clore::config::LLMConfig` instances directly by assigning values to its members

### `clore::config::TaskConfig`

Declaration: `config/schema.cppm:17`

Definition: `config/schema.cppm:17`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The struct `clore::config::TaskConfig` aggregates all parameters required to configure a code analysis task. Its members fall into three categories: filesystem paths (`compile_commands_path`, `project_root`, `output_root`, `workspace_root`), a filtering specification (`filter` of type `FilterRule`), and an LLM configuration (`llm` of type `LLMConfig`). Internally, the struct imposes no invariants on its fields – all strings and sub‑objects are default‑constructible, and the task runner is expected to validate the presence and correctness of paths before execution. The `compile_commands_path` typically points to a `compile_commands.json` file, while the root directories define the scope and output layout for the analysis.

#### Invariants

- No invariants are enforced by the type.

#### Key Members

- `project_root`
- `workspace_root`
- `output_root`
- `compile_commands_path`
- `filter`
- `llm`

#### Usage Patterns

- Defined as a data structure within the configuration module; its fields are publicly accessible for direct assignment and reading.

## Internal Structure

The `config:schema` module defines the foundational data structures for representing Clore’s configuration, separating schema definitions from loading and validation logic. It imports only the C++ standard library, ensuring minimal dependencies. The decomposition follows a clear internal layering: `FilterRule` encapsulates include/exclude patterns for file filtering, `LLMConfig` holds the system prompt and retry limits for language model interaction, and `TaskConfig` aggregates these alongside key filesystem paths (`project_root`, `workspace_root`, `output_root`, `compile_commands_path`). This structure promotes type safety, enables independent evolution of each configuration concern, and provides a stable interface for downstream modules that consume or validate configuration data.

