---
title: 'Namespace clore::net::anthropic::detail'
description: '该命名空间封装了与 Anthropic API 通信的低级实现细节。核心是 Protocol 结构体，它提供构建请求 JSON（build_request_json）、解析响应（parse_response）、生成请求头（build_headers）、构建 URL（build_url）以及读取环境变量（read_environment）等方法。此外还定义了三个关键常量：kAnthropicApiKeyEnv（环境变量名称“ANTHROPIC_API_KEY”）、kAnthropicBaseUrlEnv（环境变量名称“ANTHROPIC_BASE_URL”）以及 kAnthropicVersion（API 版本“2023-06-01”）。这些元素共同构成了与 Anthropic 服务交互的底层支持，作为内部实现细节，不直接暴露给库的用户，起到隔离与复用底层逻辑的架构作用。'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::detail`

## Summary

该命名空间封装了与 Anthropic API 通信的低级实现细节。核心是 `Protocol` 结构体，它提供构建请求 JSON（`build_request_json`）、解析响应（`parse_response`）、生成请求头（`build_headers`）、构建 URL（`build_url`）以及读取环境变量（`read_environment`）等方法。此外还定义了三个关键常量：`kAnthropicApiKeyEnv`（环境变量名称“`ANTHROPIC_API_KEY`”）、`kAnthropicBaseUrlEnv`（环境变量名称“`ANTHROPIC_BASE_URL`”）以及 `kAnthropicVersion`（API 版本“2023-06-01”）。这些元素共同构成了与 Anthropic 服务交互的底层支持，作为内部实现细节，不直接暴露给库的用户，起到隔离与复用底层逻辑的架构作用。

## Types

### `clore::net::anthropic::detail::Protocol`

Declaration: `src/network/anthropic.cppm:663`

Definition: `src/network/anthropic.cppm:663`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- 所有成员函数都是静态的，不依赖任何实例状态。
- `parse_response` 在 HTTP 状态码 >= 400 时会生成 `LLMError`，即使主体解析成功。
- `read_environment` 使用硬编码的环境变量名（来自常量 `kAnthropicBaseUrlEnv` 和 `kAnthropicApiKeyEnv`）。
- `build_headers` 始终设置 `Content-Type`、`x-api-key` 和 `anthropic-version` 三个标头。

#### Key Members

- `build_request_json`
- `parse_response`
- `build_headers`
- `build_url`
- `read_environment`
- `provider_name`
- `capability_probe_key`

#### Usage Patterns

- 作为协议适配器，直接调用静态方法来执行完整的 API 请求生命周期。
- 在高层抽象（如 `clore::net` 中的通用客户端）中通过模板参数传递，以支持不同的 AI 提供者。
- `capability_probe_key` 用于缓存或探测特定模型的能力，结合提供者名称、API 基地址和模型名。

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

A `constexpr std::string_view` constant holding the environment variable name `"ANTHROPIC_API_KEY"` used to locate the Anthropic API key at runtime.

#### Usage Patterns

- Used as environment variable name to retrieve the Anthropic API key

### `clore::net::anthropic::detail::kAnthropicBaseUrlEnv`

Declaration: `src/network/anthropic.cppm:659`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

A `constexpr std::string_view` constant named `clore::net::anthropic::detail::kAnthropicBaseUrlEnv` initialized to `"ANTHROPIC_BASE_URL"`. It serves as a compile-time string literal representing the name of an environment variable.

#### Usage Patterns

- Used as a key to read the `ANTHROPIC_BASE_URL` environment variable at runtime

### `clore::net::anthropic::detail::kAnthropicVersion`

Declaration: `src/network/anthropic.cppm:661`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

A constant `std::string_view` representing the Anthropic API version `"2023-06-01"`.

## Related Pages

- [Namespace clore::net::anthropic](../index.md)

