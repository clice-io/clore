---
title: 'Module network'
description: '该模块负责处理与 LLM 提供者（如 OpenAI 和 Anthropic）的网络通信，包括检测当前环境中的提供者类型、验证所需环境变量是否到位，以及发起异步 LLM 请求和完成请求。它通过 kota::event_loop 驱动异步操作，并提供 validate_llm_provider_environment、call_llm_async 和 call_completion_async 等公共接口，使调用方能够以非阻塞方式与 LLM 服务交互，同时确保运行时配置的正确性。'
layout: doc
template: doc
---

# Module `network`

## Summary

该模块负责处理与 LLM 提供者（如 `OpenAI` 和 Anthropic）的网络通信，包括检测当前环境中的提供者类型、验证所需环境变量是否到位，以及发起异步 LLM 请求和完成请求。它通过 `kota::event_loop` 驱动异步操作，并提供 `validate_llm_provider_environment`、`call_llm_async` 和 `call_completion_async` 等公共接口，使调用方能够以非阻塞方式与 LLM 服务交互，同时确保运行时配置的正确性。

## Imports

- `std`

## Imported By

- [`agent`](../agent/index.md)
- [`generate:scheduler`](../generate/scheduler.md)

## Functions

### `clore::net::call_completion_async`

Declaration: `network/network.cppm:24`

Definition: `network/network.cppm:150`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

函数 `clore::net::call_completion_async` 是 LLM 完成请求的顶层入口，其实现通过环境变量动态选择底层提供者。首先调用 `detect_provider_from_environment` 检测并验证提供者，若结果无值则通过 `kota::fail` 立即中止协程。随后将成功解包的 `Provider` 值传递给 `dispatch_completion`，该函数负责将 `CompletionRequest` 和 `kota::event_loop` 分派到特定提供者的执行路径。最终通过 `detail::unwrap_caught_result` 包装协程结果，若遇到取消则将其转换为 `co_await` 可捕获的取消错误，确保异常路径一致处理。

#### Side Effects

- reads environment variables via `detect_provider_from_environment`
- performs network I/O via `dispatch_completion`
- handles cancellation via `catch_cancel` on the inner task
- constructs `LLMError` instances on failure

#### Reads From

- `CompletionRequest` parameter (moved from)
- environment variables (via `detect_provider_from_environment`)
- `kota::event_loop` parameter

#### Writes To

- the `kota::task` result encapsulating `CompletionResponse` or `LLMError`

#### Usage Patterns

- called with a `CompletionRequest` and an event loop to obtain a task
- awaited to asynchronously retrieve the completion response
- used in higher‑level async completion workflows

### `clore::net::call_llm_async`

Declaration: `network/network.cppm:18`

Definition: `network/network.cppm:126`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::call_llm_async` 首先调用 `detect_provider_from_environment` 来识别可用的 LLM 提供者。如果提供者检测失败（例如，没有设置环境变量），则通过 `kota::fail` 立即返回错误。成功检测后，它使用 `provider_label` 获取提供者的描述标签，然后委托给 `request_provider_text_async`。这个内部函数接收一个回调 lambda，该 lambda 调用 `dispatch_completion` 来执行实际的完成请求，并返回一个 `kota::task`。整个流程在传入的 `kota::event_loop` 上异步执行，并使用 `.or_fail()` 将错误向上传播。

#### Side Effects

- Reads environment variables to detect the LLM provider
- Makes asynchronous HTTP requests to the LLM API

#### Reads From

- environment variables
- `model` parameter
- `system_prompt` parameter
- `request` parameter
- `loop` parameter

#### Writes To

- network output via HTTP request to LLM API
- return value (task that resolves to a string or `LLMError`)

#### Usage Patterns

- Called to send a prompt to an LLM asynchronously
- Used in higher-level functions that require provider detection and error handling
- Part of the async LLM request pipeline

### `clore::net::validate_llm_provider_environment`

Declaration: `network/network.cppm:28`

Definition: `network/network.cppm:118`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

函数 `clore::net::validate_llm_provider_environment` 的实现完全委托给内部辅助函数 `clore::net::(anonymous namespace)::detect_provider_from_environment`。该函数首先调用 `detect_provider_from_environment()`，若其返回值没有值（即包含 `std::unexpected`），则立即以同样的 `LLMError` 错误返回 `std::unexpected`；否则返回一个空的 `std::expected<void, LLMError>` 表示成功。整个实现没有额外的逻辑分支或副作用，是一个简单的转发封装，利用匿名命名空间中的环境检测算法完成校验。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- environment variables via `detect_provider_from_environment()`

#### Usage Patterns

- called before initiating LLM provider interactions to ensure environment is properly set up

## Internal Structure

`network` 模块围绕公共异步调用接口 `call_llm_async`、`call_completion_async` 和 `validate_llm_provider_environment` 进行外部暴露。其内部实现被清晰地分层在匿名命名空间中：通过 `Provider` 枚举（`Anthropic`、`OpenAI`）及对应的环境变量常量（如 `kAnthropicApiKeyEnv`）对 LLM 提供商进行抽象；辅助函数 `detect_provider_from_environment`、`has_provider_env` 和 `provider_label` 负责环境检测与状态映射；而 `dispatch_completion` 和模板函数 `request_provider_text_async` 则完成异步请求的分发与适配。模块依赖仅限于标准库（`std`），并通过 `kota::event_loop` 驱动非阻塞操作，整体采用“公共接口 + 内部辅助层”的结构，将提供商逻辑隔离在匿名作用域中，以降低耦合。

