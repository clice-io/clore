---
title: 'Module network'
description: '该模块负责管理调用外部大型语言模型（LLM）的网络交互。它提供了三个公共函数：validate_llm_provider_environment 用于检查运行环境（如 API 密钥和端点 URL）是否正确配置；call_llm_async 用于发起对指定模型的异步文本生成请求，并返回一个可用于跟踪结果的标识符；以及 call_completion_async 用于驱动异步操作的完成（基于之前返回的标识符和事件循环）。模块内部封装了与 OpenAI 和 Anthropic 等提供者通信的细节，包括从环境变量中检测提供者、构建请求、调度回调和处理响应。'
layout: doc
template: doc
---

# Module `network`

## Summary

该模块负责管理调用外部大型语言模型（LLM）的网络交互。它提供了三个公共函数：`validate_llm_provider_environment` 用于检查运行环境（如 API 密钥和端点 URL）是否正确配置；`call_llm_async` 用于发起对指定模型的异步文本生成请求，并返回一个可用于跟踪结果的标识符；以及 `call_completion_async` 用于驱动异步操作的完成（基于之前返回的标识符和事件循环）。模块内部封装了与 `OpenAI` 和 Anthropic 等提供者通信的细节，包括从环境变量中检测提供者、构建请求、调度回调和处理响应。

## Imported By

- [`agent`](../agent/index.md)
- [`generate:scheduler`](../generate/scheduler.md)

## Functions

### `clore::net::call_completion_async`

Declaration: `src/network/network.cppm:31`

Definition: `src/network/network.cppm:157`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

函数 `clore::net::call_completion_async` 首先通过调用 `detect_provider_from_environment()` 从环境变量中解析出当前可用的 LLM 提供商，该函数返回一个包含 `Provider` 枚举值的期望结果。若结果无效，则立即通过 `kota::fail` 将错误抛出并终结协程；否则提取出 `provider` 值。随后，函数将 `provider`、请求对象 `request` 以及事件循环 `loop` 转发至 `dispatch_completion`，后者负责按提供商类型路由到具体的异步请求实现（例如 `request_provider_text_async`）。`dispatch_completion` 返回一个可被取消的 `kota::task`，调用链通过 `.catch_cancel()` 将取消信号转换为错误，最后由 `detail::unwrap_caught_result` 将内部错误类型统一为 `LLMError` 并 `co_return` 给调用方。整个过程依赖环境检查、派发逻辑以及 Kota 库的协程和取消机制，未显式区分同步或异步边界。

#### Side Effects

- reads environment variables via `detect_provider_from_environment`
- performs network I/O or LLM API calls via `dispatch_completion`
- may write to logs or error states internally

#### Reads From

- the `request` parameter of type `CompletionRequest`
- the `loop` parameter of type `kota::event_loop&`
- environment variables (indirectly through `detect_provider_from_environment`)
- state managed by `dispatch_completion` and `detail::unwrap_caught_result`

#### Writes To

- the returned `kota::task` result (asynchronous completion response or error)

#### Usage Patterns

- called by higher-level completion `APIs` to initiate an LLM request asynchronously
- used in contexts where an event loop drives the asynchronous workflow

### `clore::net::call_llm_async`

Declaration: `src/network/network.cppm:25`

Definition: `src/network/network.cppm:133`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::call_llm_async` 的实现首先调用 `detect_provider_from_environment` 来检测当前环境配置的 LLM 提供商（如 `Provider::OpenAI` 或 `Provider::Anthropic`），返回一个 `kota::expected`；若失败则通过 `kota::fail` 传播错误。成功获取 `provider` 后，利用 `provider_label` 获得可读名称，然后构造一个 lambda 闭包，该 lambda 接受 `CompletionRequest` 和 `kota::event_loop`，内部调用 `dispatch_completion` 执行实际的 HTTP 或协议级别的请求并返回 `CompletionResponse`。最后将 lambda、`model`、`system_prompt`、`request` 和 `loop` 转发给 `request_provider_text_async`，该函数负责组装请求、处理流式响应并返回最终结果。整个控制流的核心依赖是匿名命名空间中的环境检测 (`detect_provider_from_environment`) 和请求分发 (`dispatch_completion`)，以及 `request_provider_text_async` 对异步文本补全的封装。

#### Side Effects

- reads environment variables to detect the LLM provider
- makes asynchronous network requests to an LLM provider
- may modify internal provider state or cached capabilities

#### Reads From

- function parameters `model`, `system_prompt`, `request`, `loop`
- environment variables via `detect_provider_from_environment`
- LLM provider configuration and capabilities

#### Usage Patterns

- call LLM asynchronously with a structured prompt request
- used in event-loop-based async workflows for text generation
- integrated with error handling via `LLMError`

### `clore::net::validate_llm_provider_environment`

Declaration: `src/network/network.cppm:35`

Definition: `src/network/network.cppm:125`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

该函数通过调用`detect_provider_from_environment`完成环境验证，该内部函数负责检查`kOpenAIApiKeyEnv`、`kAnthropicApiKeyEnv`等必需的环境变量是否存在。若检测返回`std::expected`无值（即环境不完整），则立即用`std::unexpected`包装其`LLMError`并返回，否则返回`{}`表示成功。整个过程不涉及异步请求或事件循环，完全依赖`has_nonempty_env`和`has_provider_env`等静态辅助函数完成纯环境检查。

#### Side Effects

- Reads environment variables via `detect_provider_from_environment`
- May allocate and move an `LLMError` object if provider detection fails

#### Reads From

- Environment variables
- Result of `detect_provider_from_environment`

#### Usage Patterns

- Called to verify LLM provider configuration before making API requests
- Used to pre-check environment setup for LLM operations

## Internal Structure

模块 `network` 采用分层架构：顶层暴露 `call_llm_async`、`call_completion_async` 和 `validate_llm_provider_environment` 三个公共函数，它们均接受 `kota::event_loop` 引用以支持异步回调。内部通过匿名命名空间将提供者识别、环境检测和请求分发隔离为独立层——`detect_provider_from_environment` 根据环境变量（如 `kOpenAIApiKeyEnv`、`kAnthropicApiKeyEnv`）选择 `Provider` 枚举成员，`request_provider_text_async` 模板函数则针对不同提供者（`OpenAI`、Anthropic）执行具体的网络调用逻辑。实现上依赖 `kota` 事件循环库，并通过 `dispatch_completion` 将完成结果回传给公共接口。所有内部变量（如 `system_prompt`、`provider_result`）和辅助函数均位于匿名命名空间中，避免模块外部可见，从而保持接口简洁并便于扩展新的 LLM 提供者。

