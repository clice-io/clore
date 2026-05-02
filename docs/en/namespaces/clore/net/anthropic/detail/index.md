---
title: 'Namespace clore::net::anthropic::detail'
description: 'The namespace clore::net::anthropic::detail encapsulates internal implementation details of the Anthropic API client within the clore networking library. Its primary architectural role is to provide a dedicated scope for protocol-specific logic and configuration constants that are not intended for direct use by client code. Notable declarations include the Protocol struct, which defines methods for building request JSON, constructing HTTP headers and URLs, parsing responses, reading environment variables, and providing the provider name and capability probe key. Supporting constants such as kAnthropicApiKeyEnv, kAnthropicBaseUrlEnv, and kAnthropicVersion specify the environment variable names and API version string used to configure the client. By isolating these low‑level networking and serialization details, the namespace helps maintain separation of concerns and shields higher‑level abstractions from Anthropic‑specific implementation details.'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::detail`

## Summary

The namespace `clore::net::anthropic::detail` encapsulates internal implementation details of the Anthropic API client within the clore networking library. Its primary architectural role is to provide a dedicated scope for protocol-specific logic and configuration constants that are not intended for direct use by client code. Notable declarations include the `Protocol` struct, which defines methods for building request JSON, constructing HTTP headers and `URLs`, parsing responses, reading environment variables, and providing the provider name and capability probe key. Supporting constants such as `kAnthropicApiKeyEnv`, `kAnthropicBaseUrlEnv`, and `kAnthropicVersion` specify the environment variable names and API version string used to configure the client. By isolating these low‑level networking and serialization details, the namespace helps maintain separation of concerns and shields higher‑level abstractions from Anthropic‑specific implementation details.

## Types

### `clore::net::anthropic::detail::Protocol`

Declaration: `network/anthropic.cppm:654`

Definition: `network/anthropic.cppm:654`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- All members are static; no instance state exists.
- Environment configuration must provide `api_base` and `api_key`.
- HTTP response parsing expects a non-empty body for success.
- Error mapping respects HTTP status codes >= 400.

#### Key Members

- `read_environment`
- `build_url`
- `build_headers`
- `build_request_json`
- `parse_response`
- `provider_name`
- `capability_probe_key`

#### Usage Patterns

- Used as a protocol policy for generic API client code that calls these static methods.
- Relied upon by higher-level networking to construct and send Anthropic API requests.
- Provides the provider identifier for capability probe key generation.

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

##### `clore::net::anthropic::detail::Protocol::capability_probe_key`

Declaration: `network/anthropic.cppm:717`

Definition: `network/anthropic.cppm:717`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

###### Declaration

```cpp
auto (const int &, const int &) -> std::string;
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

The variable `clore::net::anthropic::detail::kAnthropicApiKeyEnv` is a `constexpr std::string_view` constant initialized to `"ANTHROPIC_API_KEY"`. It represents the name of the environment variable used to obtain the Anthropic API key.

### `clore::net::anthropic::detail::kAnthropicBaseUrlEnv`

Declaration: `network/anthropic.cppm:650`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

A `constexpr std::string_view` constant that holds the environment variable name `"ANTHROPIC_BASE_URL"` for configuring the base URL of the Anthropic API client.

### `clore::net::anthropic::detail::kAnthropicVersion`

Declaration: `network/anthropic.cppm:652`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

A compile-time constant string view that specifies the version of the Anthropic API to use.

#### Usage Patterns

- intended to be used as the version string for Anthropic API requests

## Related Pages

- [Namespace clore::net::anthropic](../index.md)

