---
title: 'Namespace clore::net::openai::detail'
description: 'The clore::net::openai::detail namespace encapsulates internal implementation details for the OpenAI network layer. It defines the Protocol struct, which provides a cohesive set of methods for constructing API requests (build_url, build_request_json, build_headers), parsing responses (parse_response), reading environment configuration (read_environment), and identifying the provider (provider_name). This struct centralizes OpenAI-specific protocol logic, separating it from higher-level abstractions.'
layout: doc
template: doc
---

# Namespace `clore::net::openai::detail`

## Summary

The `clore::net::openai::detail` namespace encapsulates internal implementation details for the `OpenAI` network layer. It defines the `Protocol` struct, which provides a cohesive set of methods for constructing API requests (`build_url`, `build_request_json`, `build_headers`), parsing responses (`parse_response`), reading environment configuration (`read_environment`), and identifying the provider (`provider_name`). This struct centralizes `OpenAI`-specific protocol logic, separating it from higher-level abstractions.

In addition to the core `Protocol` struct, the namespace holds mutable state variables (`request`, `environment`, `raw_response`) that track the current request, environment settings, and raw response data. The namespace thus serves as a private, non-exported module that manages all `OpenAI` protocol details, forming the foundation on which public-facing components rely.

## Types

### `clore::net::openai::detail::Protocol`

Declaration: `network/openai.cppm:692`

Definition: `network/openai.cppm:692`

Implementation: [`Module openai`](../../../../../modules/openai/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- All member functions are static and constexpr-compatible on compilers supporting constexpr `std::string`?
- No mutable state is held by the struct.
- Environment variables `OPENAI_BASE_URL` and `OPENAI_API_KEY` must be set for `read_environment` to succeed.
- `build_request_json` and `parse_response` rely on external protocol utilities.

#### Key Members

- `read_environment`
- `build_url`
- `build_headers`
- `build_request_json`
- `parse_response`
- `provider_name`

#### Usage Patterns

- Called by a client to obtain environment configuration for constructing HTTP requests.
- `build_url` and `build_headers` are used to prepare the HTTP request.
- `build_request_json` serializes a `CompletionRequest` to JSON.
- `parse_response` deserializes the HTTP response body into `CompletionResponse`.
- `provider_name` is used for logging or identification.

#### Member Functions

##### `clore::net::openai::detail::Protocol::build_headers`

Declaration: `network/openai.cppm:705`

Definition: `network/openai.cppm:705`

Implementation: [`Module openai`](../../../../../modules/openai/index.md)

###### Declaration

```cpp
auto (const int &) -> int;
```

##### `clore::net::openai::detail::Protocol::build_request_json`

Declaration: `network/openai.cppm:719`

Definition: `network/openai.cppm:719`

Implementation: [`Module openai`](../../../../../modules/openai/index.md)

###### Declaration

```cpp
auto (const int &) -> int;
```

##### `clore::net::openai::detail::Protocol::build_url`

Declaration: `network/openai.cppm:701`

Definition: `network/openai.cppm:701`

Implementation: [`Module openai`](../../../../../modules/openai/index.md)

###### Declaration

```cpp
auto (const int &) -> std::string;
```

##### `clore::net::openai::detail::Protocol::parse_response`

Declaration: `network/openai.cppm:724`

Definition: `network/openai.cppm:724`

Implementation: [`Module openai`](../../../../../modules/openai/index.md)

###### Declaration

```cpp
auto (const int &) -> int;
```

##### `clore::net::openai::detail::Protocol::provider_name`

Declaration: `network/openai.cppm:739`

Definition: `network/openai.cppm:739`

Implementation: [`Module openai`](../../../../../modules/openai/index.md)

###### Declaration

```cpp
auto () -> std::string_view;
```

##### `clore::net::openai::detail::Protocol::read_environment`

Declaration: `network/openai.cppm:693`

Definition: `network/openai.cppm:693`

Implementation: [`Module openai`](../../../../../modules/openai/index.md)

###### Declaration

```cpp
auto () -> int;
```

## Related Pages

- [Namespace clore::net::openai](../index.md)

