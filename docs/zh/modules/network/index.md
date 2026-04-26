---
title: 'Module network'
description: 'network 模块是 clore 库中负责 LLM（大语言模型）网络通信的核心。它封装了与 OpenAI 和 Anthropic 等提供者的异步交互，对外提供三个主要接口：validate_llm_provider_environment 用于校验运行时环境（如 API 密钥和基础 URL 等必要配置是否完备）；call_llm_async 用于发起针对 LLM 服务的异步请求；call_completion_async 用于发起异步完成调用。所有异步操作均基于 kota::event_loop 驱动，调用方需保证事件循环在操作期间持续有效。'
layout: doc
template: doc
---

# Module `network`

## Summary

`network` 模块是 `clore` 库中负责 LLM（大语言模型）网络通信的核心。它封装了与 `OpenAI` 和 Anthropic 等提供者的异步交互，对外提供三个主要接口：`validate_llm_provider_environment` 用于校验运行时环境（如 API 密钥和基础 URL 等必要配置是否完备）；`call_llm_async` 用于发起针对 LLM 服务的异步请求；`call_completion_async` 用于发起异步完成调用。所有异步操作均基于 `kota::event_loop` 驱动，调用方需保证事件循环在操作期间持续有效。

模块内部通过匿名命名空间中的辅助函数（如 `detect_provider_from_environment`、`dispatch_completion` 等）实现提供者自动检测、环境变量解析以及完成回调的分发。它维护了 `Provider` 枚举类型和相关的环境变量常量，确保在调用外部 LLM 服务之前能够正确识别和配置目标提供者。整体上，该模块抽象了不同 LLM 提供者的网络调用差异，为上层代码提供了一个简洁、一致的异步网络交互层。

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

函数 `clore::net::call_completion_async` 的实现采用两步委托模式：首先通过 `detect_provider_from_environment` 从环境变量（如 `kAnthropicApiKeyEnv`、`kOpenAIApiKeyEnv` 等）推断出 `Provider` 枚举值；若提供商检测失败，则通过 `kota::fail` 直接返回错误。成功后，将 `provider`、原始 `request` 和 `loop` 转发给 `dispatch_completion`，该函数内部根据 `Provider` 类型（如 `Anthropic` 或 `OpenAI`）调用对应的 `request_provider_text_async` 或 `call_llm_async` 等具体实现。外层使用 `unwrap_caught_result` 包装 `dispatch_completion` 的 `kota::task`，将取消事件（`cancel`）统一转换为 `LLMError` 类型的失败结果。整个流程依赖 `kota::event_loop` 进行异步编排，并利用模板元函数 `detail::unwrap_caught_result` 捕获协程异常。

#### Side Effects

- Performs I/O to detect the LLM provider from environment variables
- Makes network calls via `dispatch_completion` to communicate with the LLM API
- Potentially handles cancellation, which may affect scheduling or resource cleanup

#### Reads From

- Environment variables (through `detect_provider_from_environment`)
- The `CompletionRequest` parameter
- The `kota::event_loop&` parameter (for scheduling callbacks)

#### Writes To

- The coroutine return value (`kota::task::promise_type` state) via `co_return`

#### Usage Patterns

- Called to perform an asynchronous LLM completion with automatic provider detection from environment
- Typically used in async contexts that provide a `kota::event_loop`

### `clore::net::call_llm_async`

Declaration: `network/network.cppm:18`

Definition: `network/network.cppm:126`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

函数 `clore::net::call_llm_async` 首先调用 `clore::net::(anonymous namespace)::detect_provider_from_environment` 来检测当前环境支持的 LLM 提供程序。如果检测失败，则通过 `kota::fail` 立即终止协程；否则，它将获取提供程序对应的标签，并将该标签、`model`、`system_prompt`、`request` 和 `loop` 传递给 `clore::net::(anonymous namespace)::request_provider_text_async`。实际的完成请求调度委托给一个 lambda 表达式，该表达式调用 `clore::net::(anonymous namespace)::dispatch_completion`，根据检测到的提供程序执行底层网络交互。整个流程依赖于 `RequestProviderTextAsync` 和 `dispatch_completion` 的异步协作，以及 `kota::event_loop` 提供的事件驱动调度。

#### Side Effects

- 执行网络 I/O 调用 LLM API
- 读取环境变量以确定提供商

#### Reads From

- `model`
- `system_prompt`
- `request`
- `loop`
- 环境变量（通过 `detect_provider_from_environment`）

#### Writes To

- 返回的 `kota::task` 结果

#### Usage Patterns

- 异步调用 LLM 生成文本
- 配合事件循环使用
- 错误处理通过 `LLMError`

### `clore::net::validate_llm_provider_environment`

Declaration: `network/network.cppm:28`

Definition: `network/network.cppm:118`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::validate_llm_provider_environment` 的实现完全委托给 `detect_provider_from_environment`：它首先调用该函数获取一个 `std::expected` 结果，该结果表示环境中的 LLM 提供商配置是否有效。如果 `provider_result` 不持有值（即检测失败），函数立即返回一个 `std::unexpected` 包装的 `LLMError`；否则返回一个空的 `std::expected<void, LLMError>` 表示成功。整个控制流仅包含此单一分支，没有重试或附加验证逻辑，完全依赖 `detect_provider_from_environment` 内部对 `Provider` 枚举（如 `Provider::Anthropic` 和 `Provider::OpenAI`）以及对应环境变量（例如 `kAnthropicApiKeyEnv`、`kOpenAIApiKeyEnv`）的检查。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- environment variables (via `detect_provider_from_environment`)

#### Usage Patterns

- Called before making LLM API calls to ensure provider is configured

## Internal Structure

`network` 模块以匿名命名空间隐藏内部实现，仅公开三个异步 LLM 调用接口。内部通过枚举 `Provider` 和一组环境变量常量（如 `kAnthropicApiKeyEnv`）进行提供者检测，`detect_provider_from_environment` 和 `has_nonempty_env` 等辅助函数负责解析运行环境；`dispatch_completion` 和模板函数 `request_provider_text_async` 封装了异步请求的分发与完成逻辑。公共接口 `validate_llm_provider_environment` 作为前置校验，`call_llm_async` 与 `call_completion_async` 向上层提供统一的异步调用入口，底层依赖 `kota::event_loop` 驱动事件循环，实现与控制流解耦。模块仅导入 `std`，无其他外部依赖。

