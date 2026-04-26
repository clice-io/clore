---
title: 'Module config:schema'
description: 'The module config:schema defines the core data structures that represent the configuration schema for the clore system. It owns the public types TaskConfig, LLMConfig, and FilterRule, which hold settings such as project paths, LLM parameters, and include/exclude filters. These structs are designed to be the canonical representation of user-provided configuration after validation and normalization.'
layout: doc
template: doc
---

# Module `config:schema`

## Summary

The module `config:schema` defines the core data structures that represent the configuration schema for the clore system. It owns the public types `TaskConfig`, `LLMConfig`, and `FilterRule`, which hold settings such as project paths, LLM parameters, and include/exclude filters. These structs are designed to be the canonical representation of user-provided configuration after validation and normalization.

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

The `clore::config::FilterRule` struct is a plain aggregate that holds two disjoint collections of pattern strings: `include` and `exclude`, each stored in a `std::vector<std::string>`. No special initialization or invariant enforcement is present; the two vectors are independent and expected to be populated by the caller according to the filtering semantics required. The struct serves solely as a container for these two pattern lists, with no member functions beyond the implicitly generated ones.

#### Invariants

- The `include` and `exclude` vectors can be empty.
- No constraints exist on the content of the strings beyond being valid pattern representations.

#### Key Members

- `include`: list of patterns to include
- `exclude`: list of patterns to exclude

#### Usage Patterns

- Used as a configuration parameter to specify which items should be included or excluded in some processing.
- Typically populated from a configuration file or user input.
- Accessed by other code to filter collections based on the include/exclude lists.

### `clore::config::LLMConfig`

Declaration: `config/schema.cppm:12`

Definition: `config/schema.cppm:12`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

`clore::config::LLMConfig` is a plain data structure that aggregates two configuration parameters: `system_prompt`, a `std::string` holding the system-level instruction text, and `retry_limit`, a `std::uint32_t` that defaults to `0` (interpreted as unlimited retries). The struct imposes no additional invariants beyond the natural type constraints of its members; the default value for `retry_limit` is the only implementation‑level choice, establishing a fallback semantics when the field is not explicitly set. All member initialization is trivial.

### `clore::config::TaskConfig`

Declaration: `config/schema.cppm:17`

Definition: `config/schema.cppm:17`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

The struct `clore::config::TaskConfig` is implemented as a plain aggregate that holds the configuration for a single task. Its fields are stored by value: `compile_commands_path`, `project_root`, `output_root`, and `workspace_root` are `std::string` path entries, while `filter` and `llm` are embedded `FilterRule` and `LLMConfig` objects respectively. The struct imposes no internal invariants; all validation, normalization, and semantic checks on the field values are delegated to external parsing or application logic. No custom constructors, assignment `operator`s, or other member functions are defined, so the type relies entirely on compiler-generated special member functions for construction, copy, and move.

#### Invariants

- All string fields are expected to contain valid filesystem paths
- `FilterRule` and `LLMConfig` are expected to be default-constructible

#### Key Members

- `compile_commands_path`
- `project_root`
- `output_root`
- `workspace_root`
- `filter`
- `llm`

#### Usage Patterns

- Loaded or populated by configuration parsing code
- Consumed by task execution logic to determine paths and behavior

## Internal Structure

The `config:schema` module provides the core data types that define the structure of the configuration system. It is decomposed into three simple, publicly-exported structs: `FilterRule`, `LLMConfig`, and `TaskConfig`, which together form a hierarchical schema. `FilterRule` encapsulates include/exclude path patterns, `LLMConfig` holds model‑specific parameters (such as `system_prompt` and `retry_limit`), and `TaskConfig` aggregates a filter rule, an LLM configuration, and several filesystem path fields (`project_root`, `workspace_root`, `output_root`, `compile_commands_path`). The module depends solely on the C++ standard library (`std`) and contains no additional logic or internal layering; it serves as a pure data definition layer upon which parsers and validators can build.

