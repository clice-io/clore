---
title: 'Namespace clore::net::anthropic::detail'
description: 'The clore::net::anthropic::detail namespace encapsulates internal implementation details for the Anthropic API integration within the networking module. It is not intended for direct use by external consumers. The namespace provides the Protocol struct, which centralizes API communication logic, including methods for building request JSON, constructing HTTP headers and URLs, parsing responses, reading environment configuration, and providing the provider name. Supporting constants define environment variable names for the API key (kAnthropicApiKeyEnv) and base URL (kAnthropicBaseUrlEnv), as well as the API version string (kAnthropicVersion). This namespace serves as the low‑level building block, isolating protocol‑specific concerns from higher‑level abstractions in the clore::net::anthropic namespace.'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::detail`

## Summary

The `clore::net::anthropic::detail` namespace encapsulates internal implementation details for the Anthropic API integration within the networking module. It is not intended for direct use by external consumers. The namespace provides the `Protocol` struct, which centralizes API communication logic, including methods for building request JSON, constructing HTTP headers and `URLs`, parsing responses, reading environment configuration, and providing the provider name. Supporting constants define environment variable names for the API key (`kAnthropicApiKeyEnv`) and base URL (`kAnthropicBaseUrlEnv`), as well as the API version string (`kAnthropicVersion`). This namespace serves as the low‑level building block, isolating protocol‑specific concerns from higher‑level abstractions in the `clore::net::anthropic` namespace.

## Types

### `clore::net::anthropic::detail::Protocol`

Declaration: `src/network/anthropic.cppm:663`

Definition: `src/network/anthropic.cppm:663`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- All methods are static; no instance state exists.
- `read_environment` expects specific environment variable names defined elsewhere.
- `build_headers` always includes Content-Type, x-api-key, and anthropic-version headers.
- `parse_response` returns error for empty body or when HTTP status >= 400.
- `capability_probe_key` combines provider name, API base, and model name.

#### Key Members

- `read_environment`
- `build_url`
- `build_headers`
- `build_request_json`
- `parse_response`
- `provider_name`
- `capability_probe_key`

#### Usage Patterns

- Called by higher-level client code to perform Anthropic-specific tasks.
- Used to construct requests and interpret responses consistently.
- `read_environment` is invoked during initialization to load credentials.
- `build_url`, `build_headers`, `build_request_json` are used together to form HTTP requests.
- `parse_response` is used after receiving an HTTP response.

#### Member Functions

##### `clore::net::anthropic::detail::Protocol::build_headers`

Declaration: `src/network/anthropic.cppm:676`

Definition: `src/network/anthropic.cppm:676`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

###### Declaration

```cpp
auto (const int &) -> int;
```

##### `clore::net::anthropic::detail::Protocol::build_request_json`

Declaration: `src/network/anthropic.cppm:694`

Definition: `src/network/anthropic.cppm:694`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

###### Declaration

```cpp
auto (const int &) -> int;
```

##### `clore::net::anthropic::detail::Protocol::build_url`

Declaration: `src/network/anthropic.cppm:672`

Definition: `src/network/anthropic.cppm:672`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

###### Declaration

```cpp
auto (const int &) -> std::string;
```

##### `clore::net::anthropic::detail::Protocol::capability_probe_key`

Declaration: `src/network/anthropic.cppm:726`

Definition: `src/network/anthropic.cppm:726`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

###### Declaration

```cpp
auto (const int &, const int &) -> std::string;
```

##### `clore::net::anthropic::detail::Protocol::parse_response`

Declaration: `src/network/anthropic.cppm:699`

Definition: `src/network/anthropic.cppm:699`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

###### Declaration

```cpp
auto (const int &) -> int;
```

##### `clore::net::anthropic::detail::Protocol::provider_name`

Declaration: `src/network/anthropic.cppm:722`

Definition: `src/network/anthropic.cppm:722`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

###### Declaration

```cpp
auto () -> std::string_view;
```

##### `clore::net::anthropic::detail::Protocol::read_environment`

Declaration: `src/network/anthropic.cppm:664`

Definition: `src/network/anthropic.cppm:664`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

###### Declaration

```cpp
auto () -> int;
```

## Variables

### `clore::net::anthropic::detail::kAnthropicApiKeyEnv`

Declaration: `src/network/anthropic.cppm:660`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

A constant string view representing the name of the environment variable used to configure the Anthropic API key. It is defined as a `constexpr` in the `clore::net::anthropic::detail` namespace.

### `clore::net::anthropic::detail::kAnthropicBaseUrlEnv`

Declaration: `src/network/anthropic.cppm:659`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

A `constexpr std::string_view` constant defining the environment variable name `ANTHROPIC_BASE_URL` used to configure the base URL for Anthropic API requests.

#### Usage Patterns

- Read as an environment variable name
- Used in HTTP request URL construction

### `clore::net::anthropic::detail::kAnthropicVersion`

Declaration: `src/network/anthropic.cppm:661`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The constant `clore::net::anthropic::detail::kAnthropicVersion` is a `constexpr std::string_view` initialized to `"2023-06-01"`, serving as the version identifier for the Anthropic API within the networking module.

#### Usage Patterns

- read as a `string_view`
- likely used in HTTP request headers as version identifier

## Related Pages

- [Namespace clore::net::anthropic](../index.md)

