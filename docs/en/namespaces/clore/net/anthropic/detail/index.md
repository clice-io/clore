---
title: 'Namespace clore::net::anthropic::detail'
description: 'The clore::net::anthropic::detail namespace serves as the internal implementation layer for the Anthropic API client, encapsulating protocol-specific logic and configuration details. It defines the Protocol struct, which provides methods for building request JSON, constructing HTTP headers, composing the API URL, parsing responses, and reading environment variables that configure the client’s behaviour. Notable constants in this namespace include kAnthropicApiKeyEnv, kAnthropicBaseUrlEnv, and kAnthropicVersion, which store the environment variable names and the default API version string.'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::detail`

## Summary

The `clore::net::anthropic::detail` namespace serves as the internal implementation layer for the Anthropic API client, encapsulating protocol-specific logic and configuration details. It defines the `Protocol` struct, which provides methods for building request JSON, constructing HTTP headers, composing the API URL, parsing responses, and reading environment variables that configure the client’s behaviour. Notable constants in this namespace include `kAnthropicApiKeyEnv`, `kAnthropicBaseUrlEnv`, and `kAnthropicVersion`, which store the environment variable names and the default API version string.

Architecturally, this namespace hides the low‑level Anthropic protocol details from the public API, allowing the rest of the `clore::net::anthropic` module to interact with a clean, abstract interface. By centralising request/response handling and environment configuration in `detail`, the codebase maintains a clear separation between internal mechanics and the user‑facing client surface.

## Types

### `clore::net::anthropic::detail::Protocol`

Declaration: `network/anthropic.cppm:654`

Definition: `network/anthropic.cppm:654`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- All methods are static and stateless
- Environment config must be valid before calling `build_url`, `build_headers`, or `build_request_json`
- `parse_response` expects a non-empty body or handles empty body with an error
- Provider name is always `"Anthropic"`

#### Key Members

- `read_environment`
- `build_url`
- `build_headers`
- `build_request_json`
- `parse_response`
- `provider_name`

#### Usage Patterns

- Used as a policy or adapter for the Anthropic provider in generic request workflows
- Methods are called sequentially: `read_environment`, then `build_url`, `build_headers`, `build_request_json`, and finally `parse_response`
- Likely substituable with other provider-specific `Protocol` structs via template or duck typing

#### Member Functions

##### `clore::net::anthropic::detail::Protocol::build_headers`

Declaration: `network/anthropic.cppm:667`

Definition: `network/anthropic.cppm:667`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

###### Declaration

```cpp
auto (const int &) -> int;
```

##### `clore::net::anthropic::detail::Protocol::build_request_json`

Declaration: `network/anthropic.cppm:685`

Definition: `network/anthropic.cppm:685`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

###### Declaration

```cpp
auto (const int &) -> int;
```

##### `clore::net::anthropic::detail::Protocol::build_url`

Declaration: `network/anthropic.cppm:663`

Definition: `network/anthropic.cppm:663`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

###### Declaration

```cpp
auto (const int &) -> std::string;
```

##### `clore::net::anthropic::detail::Protocol::parse_response`

Declaration: `network/anthropic.cppm:690`

Definition: `network/anthropic.cppm:690`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

###### Declaration

```cpp
auto (const int &) -> int;
```

##### `clore::net::anthropic::detail::Protocol::provider_name`

Declaration: `network/anthropic.cppm:713`

Definition: `network/anthropic.cppm:713`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

###### Declaration

```cpp
auto () -> std::string_view;
```

##### `clore::net::anthropic::detail::Protocol::read_environment`

Declaration: `network/anthropic.cppm:655`

Definition: `network/anthropic.cppm:655`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

###### Declaration

```cpp
auto () -> int;
```

## Variables

### `clore::net::anthropic::detail::kAnthropicApiKeyEnv`

Declaration: `network/anthropic.cppm:651`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

A `constexpr std::string_view` constant that holds the string `"ANTHROPIC_API_KEY"`, representing the name of the environment variable used to configure the Anthropic API key.

#### Usage Patterns

- used as environment variable name to retrieve Anthropic API key

### `clore::net::anthropic::detail::kAnthropicBaseUrlEnv`

Declaration: `network/anthropic.cppm:650`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

A compile-time constant `std::string_view` holding the environment variable name for the Anthropic base URL.

### `clore::net::anthropic::detail::kAnthropicVersion`

Declaration: `network/anthropic.cppm:652`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The constant `kAnthropicVersion` is a `constexpr std::string_view` initialized to "2023-06-01". It is declared in the `clore::net::anthropic::detail` namespace.

## Related Pages

- [Namespace clore::net::anthropic](../index.md)

