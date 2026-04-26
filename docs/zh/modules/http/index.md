---
title: 'Module http'
description: '该模块提供了与 LLM（大语言模型）HTTP 通信相关的核心基础设施。它负责管理环境配置的读取（如 API 密钥和基础 URL）、构建和发起同步与异步 HTTP 请求、处理返回的原始响应或错误（通过 LLMError 类型），并支持可选的请求速率限制机制（通过初始化/关闭函数以及内部信号量）。模块的公开接口主要位于 clore::net 命名空间下，包含 initialize_llm_rate_limit、shutdown_llm_rate_limit 以及 LLMError 等；同时公开了 detail 命名空间内的若干实现辅助函数和结构体（如 perform_http_request、EnvironmentConfig 等），供库内部或紧密协作的模块使用。这些设计使上层调用者能够以统一的方式执行 LLM HTTP 操作，而无需直接处理底层网络细节。'
layout: doc
template: doc
---

# Module `http`

## Summary

该模块提供了与 LLM（大语言模型）HTTP 通信相关的核心基础设施。它负责管理环境配置的读取（如 API 密钥和基础 URL）、构建和发起同步与异步 HTTP 请求、处理返回的原始响应或错误（通过 `LLMError` 类型），并支持可选的请求速率限制机制（通过初始化/关闭函数以及内部信号量）。模块的公开接口主要位于 `clore::net` 命名空间下，包含 `initialize_llm_rate_limit`、`shutdown_llm_rate_limit` 以及 `LLMError` 等；同时公开了 `detail` 命名空间内的若干实现辅助函数和结构体（如 `perform_http_request`、`EnvironmentConfig` 等），供库内部或紧密协作的模块使用。这些设计使上层调用者能够以统一的方式执行 LLM HTTP 操作，而无需直接处理底层网络细节。

## Imports

- `std`
- [`support`](../support/index.md)

## Imported By

- [`anthropic`](../anthropic/index.md)
- [`client`](../client/index.md)
- [`generate:scheduler`](../generate/scheduler.md)
- [`openai`](../openai/index.md)
- [`protocol`](../protocol/index.md)
- [`provider`](../provider/index.md)
- [`schema`](../schema/index.md)

## Types

### `clore::net::LLMError`

Declaration: `network/http.cppm:23`

Definition: `network/http.cppm:23`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

内部结构仅包含一个 `std::string` 类型的 `message` 成员，这是该错误类型保存错误描述的唯一数据载体。所有构造函数通过直接赋值或隐式转换保证 `message` 始终处于有效状态：默认构造函数利用 `= default` 使 `message` 被默认初始化（空字符串）；接受 `std::string` 的显式构造函数使用 `std::move` 将参数移入 `message`，避免拷贝开销；接受 `kota::error` 的显式构造函数则通过调用 `err.message()` 获取底层错误字符串后直接赋值给 `message`。整个类型依赖 `message` 存储错误信息，无其他状态，本质上是一个轻量级的错误描述包装。

#### Invariants

- `message` 始终包含有效的错误描述字符串
- 默认构造的 `message` 为空字符串

#### Key Members

- `message`
- 默认构造函数 `LLMError()`
- `explicit LLMError(std::string msg)`
- `explicit LLMError(kota::error err)`

#### Usage Patterns

- 作为函数返回值表示 LLM 操作失败
- 从 `kota::error` 或字符串直接构造
- 通过 `message` 成员获取错误详情

#### Member Functions

##### `clore::net::LLMError::LLMError`

Declaration: `network/http.cppm:30`

Definition: `network/http.cppm:30`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

###### Implementation

```cpp
explicit LLMError(kota::error err) : message(err.message()) {}
```

##### `clore::net::LLMError::LLMError`

Declaration: `network/http.cppm:28`

Definition: `network/http.cppm:28`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

###### Implementation

```cpp
explicit LLMError(std::string msg) : message(std::move(msg)) {}
```

##### `clore::net::LLMError::LLMError`

Declaration: `network/http.cppm:26`

Definition: `network/http.cppm:26`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

###### Implementation

```cpp
LLMError() = default
```

### `clore::net::detail::EnvironmentConfig`

Declaration: `network/http.cppm:37`

Definition: `network/http.cppm:37`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该结构体是用于聚合API连接参数的内部配置容器。它包含两个 `std::string` 成员：`api_key` 用于存储认证凭据，`api_base` 用于存储HTTP请求的目标基础URL。整个类型没有自定义构造函数，因此其成员的默认初始化为空字符串，调用方需在构造后直接赋值或通过其他机制填充。作为实现细节，它不附带任何不变式断言或有效性验证。

#### Invariants

- None documented.

#### Key Members

- `api_base`
- `api_key`

#### Usage Patterns

- Used as a parameter or member in HTTP client configuration within the `clore::net::detail` namespace.

### `clore::net::detail::RawHttpResponse`

Declaration: `network/http.cppm:42`

Definition: `network/http.cppm:42`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该结构体是一个轻量聚合，仅由 `http_status` 和 `body` 两个公有字段构成。内部结构保持最小化，不提供构造函数或成员函数，因此创建与访问均通过直接初始化或赋值完成。不变式方面，`http_status` 预期被调用方设置为合法的 HTTP 状态码（如 200、404），但该值未经校验，零值表示未初始化；`body` 作为 `std::string` 具备自包含的内存管理，无需额外约定。

重要成员实现体现在字段本身的默认初始化：`http_status` 初始化为 `0`，`body` 为空字符串，使得默认构造的实例处于已知的“未填充”状态。实际使用中，该结构通常由更低层网络解析过程填充，作为从原始字节流到更高层响应对象的中间缓冲。由于其聚合性质，复制与移动均依赖编译器生成的版本，保持平凡且高效。

#### Invariants

- `http_status` is set to a valid HTTP status code after a response is received
- `body` contains the full response body as received from the server

#### Key Members

- `http_status`
- `body`

#### Usage Patterns

- Used internally by the HTTP client to hold raw response data before parsing
- Constructed by the network layer and then used to build a higher-level response object

## Variables

### `clore::net::detail::g_llm_request_counter`

Declaration: `network/http.cppm:94`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

`clore::net::detail::g_llm_request_counter` is a global atomic counter that provides monotonically increasing identifiers for outgoing LLM requests, ensuring thread-safe uniqueness. It is associated with the `perform_http_request_async` function, which likely increments it when initiating a request.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- assigned to local request number variables
- accessed in HTTP request context

### `clore::net::detail::g_llm_semaphore`

Declaration: `network/http.cppm:47`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The variable controls concurrent access to LLM API calls. It is acquired before sending a request (in `perform_http_request_async`) and released after completion. It is initialized by `initialize_llm_rate_limit` with a given count and cleaned up by `shutdown_llm_rate_limit`.

#### Mutation Sources

- `initialize_llm_rate_limit` assigns a new semaphore
- `shutdown_llm_rate_limit` resets or destroys the semaphore

#### Usage Patterns

- acquired/released in `perform_http_request_async` for rate limiting
- passed to initialization and shutdown functions

### `clore::net::detail::kHttpConnectTimeoutMs`

Declaration: `network/http.cppm:96`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

Used within `configure_request` to set the HTTP connect timeout duration. The value is passed to the HTTP request configuration to limit how long the client waits to establish a connection.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- passed to HTTP request configuration in `configure_request`

### `clore::net::detail::kHttpRequestTimeout`

Declaration: `network/http.cppm:97`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

Defined in the `clore::net::detail` namespace, this constant provides a standard timeout value for HTTP request operations. It is typically passed to request-building functions to limit the maximum wait time for a response, and its value of 120 seconds is a common default for network calls.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- used as timeout argument in HTTP request functions

## Functions

### `clore::net::detail::configure_request`

Declaration: `network/http.cppm:126`

Definition: `network/http.cppm:126`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数依次执行三次操作：首先遍历传入的 `headers` 范围，对每一个 `kota::http::header` 调用 `request.header(header.name, header.value)` 将其附加到请求对象中；然后将 `request_json` 的所有权通过 `request.body(std::move(request_json))` 转移给请求体；最后设置三个 curl 选项：连接超时时间 `CURLOPT_CONNECTTIMEOUT_MS` 使用常量 `kHttpConnectTimeoutMs`，静默信号 `CURLOPT_NOSIGNAL` 为 `1L`，以及 `CURLOPT_TCP_KEEPALIVE` 为 `1L`。内部不含分支或循环，所有步骤线性执行，仅依赖 `kota::http::request` 的成员函数与 `kHttpConnectTimeoutMs` 常量。

#### Side Effects

- Modifies the `kota::http::request` object pointed to by `request` by setting its headers, body, and curl options.

#### Reads From

- `headers` span of HTTP headers
- `request_json` string

#### Writes To

- `request` object's header list
- `request` object's body string
- `request` object's curl option settings

#### Usage Patterns

- Called before performing an HTTP request to set up headers and body.
- Used to apply curl options for timeout and keepalive.

### `clore::net::detail::perform_http_request`

Declaration: `network/http.cppm:52`

Definition: `network/http.cppm:139`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数作为同步包装器，通过一个局部 `async::event_loop` 驱动异步 HTTP 请求。它首先将参数转换为异步变体 `perform_http_request_async` 期望的形式，然后安排该操作并运行事件循环直到完成。操作完成后的结果通过 `operation.result()` 获取：如果操作被取消（例如通过 `catch_cancel()`），则构造一个带有格式化取消消息的 `LLMError` 并返回 `std::unexpected`；如果异步阶段报告了错误，则转发该错误；否则成功返回 `std::move(*result)`，即一个 `RawHttpResponse`。核心依赖包括 `perform_http_request_async`（后者内部会处理速率限制、环境配置和连接管理）以及 `async::event_loop` 提供的协程调度能力。整个函数的控制流完全围绕这一本地循环展开，将异步操作同步化以简化调用方使用。

#### Side Effects

- Makes an HTTP request to the given URL, causing network I/O
- Allocates memory for copies of the URL string, headers vector, and request body string
- Runs an event loop synchronously, blocking the calling thread until the async operation completes

#### Reads From

- parameter `url`
- parameter `headers` (as `std::span`)
- parameter `request_json`

#### Writes To

- returns an `std::expected` with either `RawHttpResponse` or `LLMError`

#### Usage Patterns

- Callers use this function to perform a blocking HTTP request
- Used to synchronously invoke asynchronous HTTP operations
- Typically called from non-async contexts where a blocking interface is required

### `clore::net::detail::perform_http_request_async`

Declaration: `network/http.cppm:57`

Definition: `network/http.cppm:165`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::perform_http_request_async` 是一个协程，它首先检查全局信号量 `g_llm_semaphore`，若存在则 `co_await` 获取许可，然后创建一个 `SemaphoreGuard` 对象，其析构函数会在协程结束时释放该信号量。接着通过原子递增 `g_llm_request_counter` 分配请求编号并记录日志。核心流程是构造 `kota::http::client` 并设置超时 `kHttpRequestTimeout`，在给定的 `async::event_loop` 上发起 POST 请求，调用 `configure_request` 填充头信息和请求体 `request_json`。`co_await` 发送请求时使用 `catch_cancel` 处理取消，若被取消则 `co_await async::fail` 产生一个包含请求编号和URL的 `LLMError`；若存在传输错误则同样 `co_await async::fail`，并记录警告。成功后，从响应中提取状态码和正文文本构造 `RawHttpResponse`，记录完成日志并 `co_return` 该对象。

该函数依赖全局锁 `g_llm_semaphore` 来限制并发，依赖 `logging` 模块输出调试信息，并通过 `kota::http` 库完成网络 I/O。内部控制流完全遵循协程的异步模式，所有异常或取消均通过 `co_await async::fail` 传播为 `async::task` 的失败结果。

#### Side Effects

- 获取和释放全局信号量 `g_llm_semaphore`
- 原子增加全局计数器 `g_llm_request_counter`
- 调用 `logging::info` 和 `logging::warn` 记录日志
- 执行异步 HTTP 网络 I/O 请求

#### Reads From

- 全局变量 `g_llm_semaphore`
- 全局计数器 `g_llm_request_counter`
- 参数 `url`
- 参数 `headers`
- 参数 `request_json`
- 参数 `loop`

#### Writes To

- 全局信号量 `g_llm_semaphore`（状态变更）
- 全局计数器 `g_llm_request_counter`（增加）
- 日志输出（记录请求与响应信息）

#### Usage Patterns

- 作为 LLM API 异步调用的核心实现
- 被高层请求调度函数调用
- 处理并发限流和错误恢复

### `clore::net::detail::read_environment`

Declaration: `network/http.cppm:49`

Definition: `network/http.cppm:108`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

`clore::net::detail::read_environment` 的实现核心是两次调用 `read_required_env`，分别用于解析 `base_env` 和 `key_env` 参数对应的环境变量。如果任一调用失败（返回无值），该函数立即返回一个包装了错误 `LLMError` 的 `std::unexpected`，并通过移动操作传递错误。仅当两次都成功时，函数才使用移动后的值构造并返回 `EnvironmentConfig` 结构体，分别填充 `api_base` 和 `api_key` 字段。该流程完全依赖 `read_required_env` 来执行实际的环境变量查找和错误构造，自身不涉及网络或并发控制。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- environment variables specified by `base_env` and `key_env` (via `read_required_env`)

#### Usage Patterns

- obtaining LLM API endpoint configuration from environment
- initializing `EnvironmentConfig` for network requests

### `clore::net::detail::read_required_env`

Declaration: `network/http.cppm:99`

Definition: `network/http.cppm:99`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `read_required_env` 首先将参数 `name` 转换为 `std::string` 并调用 `std::getenv` 获取环境变量的原始 C 字符串指针。若指针为 `nullptr` 或者第一个字符是空字符，则构造一个 `LLMError` 对象，其中包含一条说明该必需环境变量未设置的格式化消息，并通过 `std::unexpected` 返回左值错误。成功时，将获取到的 C 字符串拷贝为 `std::string` 并返回。

整个实现完全依赖标准库函数 `std::getenv` 和自定义错误类型 `LLMError`，没有使用其他内部工具或外部依赖。错误路径仅返回一个携带描述性消息的 `LLMError`，不包含额外的错误码或嵌套原因。返回类型为 `std::expected<std::string, LLMError>`，符合常见的预期/错误模式。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- 环境变量（通过 `std::getenv` 读取）
- 参数 `name`

#### Usage Patterns

- 作为配置初始化的一部分，用于读取必需的环境变量
- 被 `read_environment` 等其他函数调用

### `clore::net::detail::unwrap_caught_result`

Declaration: `network/http.cppm:63`

Definition: `network/http.cppm:63`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::unwrap_caught_result` 是一个模板辅助函数，用于将低层异步操作的结果 `R`（预期为 `kota::result` 类型）转换为协程任务 `kota::task` 的返回值或失败状态。内部控制流首先检查 `result.is_cancelled()`，若为真则通过 `co_await kota::fail` 抛出一个以 `cancel_message` 构造的 `LLMError`；接着检查 `result.has_error()`，若为真则移动其中的错误并同样通过 `co_await kota::fail` 抛出该错误；若均未触发，则将 `result` 的解引用值移动返回。该函数依赖 `kota::task`、`kota::fail` 及 `LLMError` 的类型接口，并假定 `R` 类型支持 `is_cancelled()`、`has_error()`、移动析构及解引用操作。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `result` parameter (via `is_cancelled()`, `has_error()`, `error()`, `*result`)
- `cancel_message` parameter

#### Usage Patterns

- Used to handle results from async operations that may be cancelled or contain errors, converting them into a coroutine task that either yields the value or fails with `LLMError`.

### `clore::net::initialize_llm_rate_limit`

Declaration: `network/http.cppm:19`

Definition: `network/http.cppm:78`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

函数 `clore::net::initialize_llm_rate_limit` 通过检查参数 `rate_limit` 的值来决定全局信号量 `clore::net::detail::g_llm_semaphore` 的状态。当 `rate_limit` 为零时，调用 `reset` 方法释放当前的信号量资源，从而禁用速率限制；否则，使用传入的 `rate_limit` 值构造一个新的 `kota::semaphore` 实例，并将其赋给 `g_llm_semaphore`，以限制后续请求的并发数。该实现直接依赖于 `kota::semaphore` 类型和全局对象，是并发控制的核心初始化点。

#### Side Effects

- Resets or replaces the global LLM semaphore pointer `detail::g_llm_semaphore`
- Allocates a new `kota::semaphore` object when `rate_limit` is non-zero

#### Reads From

- Parameter `rate_limit`

#### Writes To

- Global variable `detail::g_llm_semaphore`

#### Usage Patterns

- Called during initialization to set the concurrency limit for LLM requests
- Used to enable or disable rate limiting by passing zero

### `clore::net::shutdown_llm_rate_limit`

Declaration: `network/http.cppm:21`

Definition: `network/http.cppm:223`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::shutdown_llm_rate_limit` 的实现直接调用 `detail::g_llm_semaphore.reset()`。该操作会强制释放全局信号量，清除所有等待状态，并允许后续请求不再受速率限制约束。函数被标记为 `noexcept`，不执行任何网络 I/O、环境变量读取或异步循环操作。其唯一依赖是 `detail::g_llm_semaphore`——一个由 `clore::net::initialize_llm_rate_limit` 初始化的计数器信号量。重置后，任何正在等待信号量的请求（通过 `detail::perform_http_request_async` 中的 `SemaphoreGuard` 控制）将立即被解除阻塞。

#### Side Effects

- 重置全局信号量 `detail::g_llm_semaphore`，可能改变速率限制状态。

#### Writes To

- `detail::g_llm_semaphore`

#### Usage Patterns

- 应用关闭时调用
- 重置速率限制状态时调用

## Internal Structure

`http` 模块实现了 LLM（大语言模型）HTTP 请求的完整通信管理。它对外公开了 `initialize_llm_rate_limit` / `shutdown_llm_rate_limit` 用于限流控制，以及 `LLMError` 错误类型；内部实现则通过 `detail` 命名空间分解为同步请求（`perform_http_request`）、异步请求（`perform_http_request_async`）、环境变量读取（`read_environment`、`read_required_env`）、请求配置（`configure_request`）和错误解包（`unwrap_caught_result`）等独立职责。模块依赖 `std` 和 `support` 基础库，内部采用基于 `kota::semaphore` 的全局信号量 `g_llm_semaphore` 以及原子计数器 `g_llm_request_counter` 来实现跨请求的速率隔离和唯一标识。异步路径围绕 `async::event_loop` 构建，并通过局部的 `SemaphoreGuard` 管理信号量生命周期，将网络调用与事件循环深度绑定。

## Related Pages

- [Module support](../support/index.md)

