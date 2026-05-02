---
title: 'Module http'
description: 'HTTP 模块封装了与 LLM（大语言模型）API 交互所需的网络层基础设施。它负责管理 HTTP 请求的生命周期，包括执行同步和异步请求、解析环境配置以获取 API 端点与凭据、实施并发速率限制，并提供统一的错误处理机制。模块内部维护了 TCP 超时、DNS 缓存、连接池等底层参数的常量配置，确保网络操作的稳定性和可预测性。'
layout: doc
template: doc
---

# Module `http`

## Summary

HTTP 模块封装了与 LLM（大语言模型）API 交互所需的网络层基础设施。它负责管理 HTTP 请求的生命周期，包括执行同步和异步请求、解析环境配置以获取 API 端点与凭据、实施并发速率限制，并提供统一的错误处理机制。模块内部维护了 TCP 超时、DNS 缓存、连接池等底层参数的常量配置，确保网络操作的稳定性和可预测性。

模块对外公开了速率限制的初始化和关闭函数（`initialize_llm_rate_limit` 与 `shutdown_llm_rate_limit`），以及用于表示 LLM 特定错误的 `LLMError` 类型。此外，模块通过 `detail` 命名空间暴露了环境配置读取（`read_environment`、`read_required_env`）、请求配置（`configure_request`）、同步/异步请求执行（`perform_http_request`、`perform_http_request_async`）以及结果转换（`unwrap_caught_result`）等内部实现。这些组件的共同职责是构成一个安全、可配置的 HTTP 通信层，供上层 LLM 调用逻辑使用。

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

`clore::net::LLMError` 在内部维护一个 `std::string message` 成员，作为错误描述的唯一存储位置。所有构造函数都保证这一不变量：默认构造函数将 `message` 保持为空字符串；从 `std::string` 构造的显式构造函数通过 `std::move(msg)` 直接将参数赋值给 `message`；从 `kota::error` 构造的显式构造函数调用 `err.message()` 获取描述并存入 `message`。这使 `message` 始终包含可显示的字符串，无论构造途径如何。

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

`clore::net::detail::EnvironmentConfig` 是一个仅包含两个 `std::string` 成员 `api_base` 与 `api_key` 的聚合体，用于在内部组件间传递 API 网络配置。该结构体不定义任何构造函数、析构函数或成员方法，其所有字段均直接暴露为公有数据成员，因此依赖方必须自行确保 `api_base` 和 `api_key` 在语义上有效（例如非空）。作为 `clore::net::detail` 命名空间下的实现细节，它被设计为一种轻量级的数据传输对象，不承担校验或不变量维护的职责，仅负责承载由上层调用方提供的原始配置字符串。

### `clore::net::detail::RawHttpResponse`

Declaration: `network/http.cppm:42`

Definition: `network/http.cppm:42`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

结构体 `clore::net::detail::RawHttpResponse` 是内部原始 HTTP 响应容器，仅由两个公有数据成员构成：`http_status` 以 `long` 类型存储状态码，默认初始化为 `0`；`body` 以 `std::string` 存储响应体，默认构造为空字符串。该类型不维护任何特殊不变式，所有成员均可自由修改，使其作为低层次值类型使用。`http_status` 的 `0` 值仅表示未显式赋值，而非有效 HTTP 状态码；`body` 为空字符串表示无响应体内容。其简单设计旨在作为中转数据载体，在被上层的 `HttpResponse` 解析和封装前保存原始 HTTP 响应信息。

#### Invariants

- `http_status` is an arbitrary `long` value; no validation that it corresponds to a valid HTTP status code
- `body` is an arbitrary `std::string`; no encoding or length constraints

#### Key Members

- `http_status`
- `body`

#### Usage Patterns

- Serves as a straightforward container for a parsed HTTP response before further processing
- Likely populated by lower‑level network code and consumed by higher‑level request/response abstractions

## Variables

### `clore::net::detail::g_llm_request_counter`

Declaration: `network/http.cppm:97`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The counter is read and incremented (via `fetch_add` or pre-increment) inside `clore::net::detail::perform_http_request_async` to produce a unique `request_number` for each outgoing request. This number is used to tag or correlate the request, facilitating tracing or logging.

#### Mutation Sources

- `clore::net::detail::perform_http_request_async` incrementing the counter

#### Usage Patterns

- read to obtain a unique request number
- assigned to the local variable `request_number`

### `clore::net::detail::g_llm_semaphore`

Declaration: `network/http.cppm:48`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The semaphore is initialized in `clore::net::initialize_llm_rate_limit` and released or reset in `clore::net::shutdown_llm_rate_limit`. It is read at the start of request processing to limit concurrency; code paths that issue LLM requests acquire the semaphore before proceeding and release it after completion, though the acquire/release logic itself is not shown in the provided evidence.

#### Mutation Sources

- initialization in `clore::net::initialize_llm_rate_limit`
- reset or release in `clore::net::shutdown_llm_rate_limit`

#### Usage Patterns

- acquired before LLM request to enforce concurrency limit
- released after LLM request completes

### `clore::net::detail::g_llm_semaphore_mutex`

Declaration: `network/http.cppm:47`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This mutex is used by the functions `clore::net::initialize_llm_rate_limit`, `clore::net::detail::(anonymous namespace)::current_llm_semaphore`, and `clore::net::shutdown_llm_rate_limit` to protect the global semaphore object (`clore::net::detail::g_llm_semaphore`) and related counter (`clore::net::detail::g_llm_request_counter`). It ensures exclusive access when initializing, querying, or shutting down the rate limiter.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- locked in `initialize_llm_rate_limit` to set up the semaphore
- locked in `current_llm_semaphore` to safely retrieve the shared semaphore pointer
- locked in `shutdown_llm_rate_limit` to destroy the semaphore

### `clore::net::detail::kConnMaxAgeSec`

Declaration: `network/http.cppm:102`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

It is read by `clore::net::detail::configure_request` to set the maximum age of a connection in seconds. As a compile-time constant, it is not mutated after initialization.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Read in `clore::net::detail::configure_request`

### `clore::net::detail::kDnsCacheTimeoutSec`

Declaration: `network/http.cppm:101`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This constant defines the DNS cache timeout in seconds and is read by the function `clore::net::detail::configure_request` to set the respective timeout on HTTP requests. It is never mutated after initialization.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Read by `clore::net::detail::configure_request` to configure DNS cache timeout on HTTP requests

### `clore::net::detail::kHttpConnectTimeoutMs`

Declaration: `network/http.cppm:99`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This constant is used by `clore::net::detail::configure_request` to set the connect timeout on outgoing HTTP requests, ensuring that connection attempts are bounded by this interval.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- used in `configure_request`

### `clore::net::detail::kHttpRequestTimeout`

Declaration: `network/http.cppm:100`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This constant is used as a timeout parameter in HTTP client operations to limit the waiting time for a request to finish. It is read but never modified, ensuring a consistent timeout across all requests that reference it.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- used as a timeout value for HTTP request operations
- read in request processing logic to enforce a deadline

### `clore::net::detail::kTcpKeepIdleSec`

Declaration: `network/http.cppm:103`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

Used in function `configure_request` to specify the idle time before TCP keep-alive probes begin.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Referenced in `configure_request` for socket option configuration

### `clore::net::detail::kTcpKeepIntvlSec`

Declaration: `network/http.cppm:104`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This constant is used within `clore::net::detail::configure_request` to set the TCP keepalive interval on an HTTP request object, ensuring that idle connections are probed periodically to detect dead peers.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- used in `clore::net::detail::configure_request` to configure TCP keepalive

## Functions

### `clore::net::detail::configure_request`

Declaration: `network/http.cppm:150`

Definition: `network/http.cppm:150`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::configure_request` 接受一个 `kota::http::request` 引用、一个 `std::span<const kota::http::header>` 头列表和一个 `std::string` 请求体 JSON 字符串，并填充该请求对象。它遍历头列表，对每个 `kota::http::header` 调用 `request.header` 设置对应的名称和值，然后将 `request_json` 通过 `request.body` 设置为请求体。接着，函数设置一系列与 curl 相关的选项：通过 `request.curl_option` 依次配置 `CURLOPT_CONNECTTIMEOUT_MS` 为 `kHttpConnectTimeoutMs`、`CURLOPT_NOSIGNAL` 为 `1L`、`CURLOPT_TCP_KEEPALIVE` 为 `1L`、`CURLOPT_TCP_KEEPIDLE` 为 `kTcpKeepIdleSec`、`CURLOPT_TCP_KEEPINTVL` 为 `kTcpKeepIntvlSec`、`CURLOPT_DNS_CACHE_TIMEOUT` 为 `kDnsCacheTimeoutSec` 以及 `CURLOPT_MAXAGE_CONN` 为 `kConnMaxAgeSec`。该过程无分支或错误处理，所有常量均定义于 `clore::net::detail` 命名空间内，共同为即将执行的 HTTP 请求建立标准的超时、保活和缓存策略。

#### Side Effects

- Mutates the provided `kota::http::request` object by adding HTTP headers
- Sets the request body via move assignment on `request.body()`
- Configures multiple curl options on the request affecting connection behavior (timeout, keepalive, DNS cache)

#### Reads From

- `headers` span containing header name-value pairs
- `request_json` string that becomes the body
- Internal constants: `kHttpConnectTimeoutMs`, `kTcpKeepIdleSec`, `kTcpKeepIntvlSec`, `kDnsCacheTimeoutSec`, `kConnMaxAgeSec`

#### Writes To

- `request` object's header list
- `request` object's body string
- `request` object's curl option store

#### Usage Patterns

- Called in HTTP request construction pipelines before dispatching the request
- Used internally by `clore::net::detail::perform_http_request` or related async variants

### `clore::net::detail::perform_http_request`

Declaration: `network/http.cppm:53`

Definition: `network/http.cppm:167`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数通过同步包装异步操作来实现 HTTP 请求。它首先创建一个本地的 `async::event_loop`，并将传入的 `headers` 区间复制到 `headers_vec` 中。然后调用 `perform_http_request_async`，传入构造后的 `url`、`headers_vec`、`request_json` 字符串以及事件循环，获得一个 `operation`（此操作被立即调用 `.catch_cancel()` 以捕获取消）。接着将 `operation` 调度到 `loop` 并运行事件循环，等待异步操作完成。在事件循环结束后，从 `operation.result()` 中获取结果：若结果被取消，则返回一个包含取消消息的 `LLMError`；若存在错误，则移动返回该错误；否则，移动出内部的 `RawHttpResponse` 作为成功值返回。该函数依赖 `perform_http_request_async` 执行实际网络 I/O，并依赖 `async::event_loop` 驱动异步任务。

#### Side Effects

- Performs an HTTP request over the network (I/O)
- May cause observable side effects on the target server

#### Reads From

- url parameter
- headers parameter
- `request_json` parameter

#### Writes To

- the returned `std::expected<RawHttpResponse, LLMError>` object

#### Usage Patterns

- Used as a synchronous wrapper around the async HTTP request machinery
- Called when a blocking HTTP request is needed in synchronous code paths

### `clore::net::detail::perform_http_request_async`

Declaration: `network/http.cppm:58`

Definition: `network/http.cppm:195`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数是一个异步协程，使用 `clore::net::detail::(anonymous namespace)::current_llm_semaphore` 获取全局信号量，若存在则 `co_await` 其 `acquire` 以遵守速率限制，并立即构造局部 `SemaphoreGuard` 确保无论成功、取消或错误都调用 `release` 释放信号量。它从 `clore::net::detail::g_llm_request_counter` 自增获取请求编号用于日志，然后通过 `clore::net::detail::(anonymous namespace)::get_thread_http_client` 获得线程局部 HTTP 客户端，生成 POST 请求并设置 `kHttpRequestTimeout` 超时，调用 `clore::net::detail::configure_request` 填入头信息和请求体。发送后检查响应：若被取消则先释放信号量再以 `LLMError` 失败；若有错误则同样处理；成功时构造 `RawHttpResponse`（包含 `http_status` 和 `body`），记录完成日志，释放信号量，最后 `co_return` 该响应。

内部控制流完全围绕信号量保护、请求编号跟踪和异步网络调用展开，依赖 `SemaphoreGuard::release` 进行资源清理，以及 `get_thread_http_client` 提供线程安全的客户端实例。

#### Side Effects

- 递增全局计数器 `g_llm_request_counter`
- 获取并释放全局信号量（可能阻塞）
- 记录开始、失败、完成日志（`logging::info`、`logging::warn`）
- 执行 HTTP POST 网络请求（I/O）
- 通过 `async::fail` 终止协程（抛出异常等效行为）

#### Reads From

- 参数 `url`、`headers`、`request_json`、`loop`
- 全局变量 `g_llm_request_counter`
- 函数 `current_llm_semaphore()` 返回的信号量
- 线程本地客户端 `get_thread_http_client()`

#### Writes To

- 全局计数器 `g_llm_request_counter`
- 信号量（获取和释放）
- 日志输出流
- 网络发送缓冲区（HTTP 请求）

#### Usage Patterns

- 以异步方式发起受信号量限制的 HTTP POST 请求
- 用于 LLM API 调用（日志中包含 'LLM' 标记）
- 在事件循环上下文中通过 `co_await` 等待完成
- 与 `SemaphoreGuard` 配合确保信号量释放

### `clore::net::detail::read_environment`

Declaration: `network/http.cppm:50`

Definition: `network/http.cppm:132`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数依次调用 `clore::net::detail::read_required_env` 两次：先读取由 `base_env` 指定的环境变量值赋给 `api_base`，若失败则立即返回 `std::unexpected` 包装的 `LLMError`；再读取 `key_env` 指定的环境变量值赋给 `api_key`，同样在失败时提前返回。两次读取均成功后，通过移动语义构造 `EnvironmentConfig` 对象并成功返回，其 `api_base` 和 `api_key` 字段分别持有上述结果。

内部控制流完全遵循“先失败先返回”的线性顺序，无分支或循环。核心依赖是 `read_required_env`（负责实际的环境变量查找和错误转换）以及 `EnvironmentConfig` 与 `LLMError` 类型（定义于同一模块）。整个函数的复杂度与这两个调用的正确性绑定，不涉及网络、并发或日志记录等其他基础设施。

#### Side Effects

- Reads environment variables via `read_required_env`

#### Reads From

- Environment variables specified by parameters `base_env` and `key_env`

#### Usage Patterns

- Used to fetch environment variables for API base URL and API key configuration

### `clore::net::detail::read_required_env`

Declaration: `network/http.cppm:123`

Definition: `network/http.cppm:123`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数通过 `std::getenv` 读取环境变量 `name`，首先将 `name` 转换为 `std::string` 以调用 C 标准库接口。若返回值为 `nullptr` 或首字符为空终止符，则构造一个 `LLMError` 对象，其 `message` 由 `std::format` 生成，并通过 `std::unexpected` 返回错误结果。否则，将获取的 C 字符串复制为 `std::string` 正常返回。整个过程不涉及任何外部依赖，仅依赖标准库设施和 `LLMError` 类型。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- The environment variable named by `name` via `std::getenv`

#### Usage Patterns

- retrieving mandatory configuration values like API keys or endpoints
- fallible environment variable lookup with error handling
- ensuring required environment is present before proceeding

### `clore::net::detail::unwrap_caught_result`

Declaration: `network/http.cppm:64`

Definition: `network/http.cppm:64`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数接受一个通用结果对象 `result` 和取消消息字符串 `cancel_message`，返回一个基于协程的任务类型 `kota::task<typename R::value_type, LLMError>`。内部首先检查 `result.is_cancelled()`，若为真则通过 `kota::fail` 构造一个包含 `cancel_message` 的 `LLMError` 对象并立即终止；否则检查 `result.has_error()`，若存在错误则将错误从结果中移出（`std::move(result).error()`）同样通过 `kota::fail` 产生 `LLMError`。若无取消或错误，则提取成功值（`std::move(*result)`）并返回。该函数依赖协程框架的 `kota::fail` 和 `kota::task`，以及 `LLMError` 的字符串和错误移动构造函数；结果类型 `R` 需提供 `is_cancelled()`、`has_error()` 和 `operator*` 接口，并定义 `value_type` 成员。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `result` parameter (via `is_cancelled()`, `has_error()`, `operator*`)
- `cancel_message` parameter

#### Usage Patterns

- Unwrapping asynchronous HTTP results with cancellation handling
- Converting a caught result to a task that propagates errors

### `clore::net::initialize_llm_rate_limit`

Declaration: `network/http.cppm:19`

Definition: `network/http.cppm:79`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::initialize_llm_rate_limit` 通过更新全局信号量 `detail::g_llm_semaphore` 来设置 LLM 请求的并发限制。它首先获取 `detail::g_llm_semaphore_mutex` 的锁，以确保线程安全。如果参数 `rate_limit` 为零，则将信号量重置为空指针（`detail::g_llm_semaphore.reset()`），从而禁用速率限制；否则，创建一个新的 `kota::semaphore` 实例，其初始计数等于 `rate_limit`。该函数仅操纵共享信号量本身，不涉及网络请求或环境读取，其效果会间接影响后续 `perform_http_request_async` 等函数中的 `SemaphoreGuard` 行为。

#### Side Effects

- Acquires mutex `detail::g_llm_semaphore_mutex`
- Modifies global shared pointer `detail::g_llm_semaphore`
- Creates or destroys a `kota::semaphore` object

#### Reads From

- Parameter `rate_limit`
- Mutex `detail::g_llm_semaphore_mutex`
- Global shared pointer `detail::g_llm_semaphore`

#### Writes To

- Global shared pointer `detail::g_llm_semaphore`

#### Usage Patterns

- Called during initialization to configure LLM concurrency limit
- Called with a non-zero value to enable rate limiting
- Called with zero to disable rate limiting

### `clore::net::shutdown_llm_rate_limit`

Declaration: `network/http.cppm:21`

Definition: `network/http.cppm:263`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

函数 `clore::net::shutdown_llm_rate_limit` 的实现围绕一个关键操作：通过 `std::lock_guard` 锁定全局互斥量 `detail::g_llm_semaphore_mutex`，然后调用 `reset()` 于全局共享指针 `detail::g_llm_semaphore`。这销毁了之前由 `clore::net::initialize_llm_rate_limit` 创建的 `kota::semaphore` 实例，从而停止所有对该信号量的等待操作并释放相关资源。该函数不抛出异常，其控制流极为简单：互斥保护后立即释放底层限流信号量，不涉及任何条件分支或额外状态检查。所有依赖都限定在 `detail` 命名空间内，包括互斥量和信号量指针，无外部 I/O 或异步操作。

#### Side Effects

- Acquires and releases `detail::g_llm_semaphore_mutex`
- Resets `detail::g_llm_semaphore` to its cleared state
- Potentially releases threads waiting on the semaphore

#### Reads From

- `detail::g_llm_semaphore_mutex` (read for locking)
- `detail::g_llm_semaphore` (read for reset operation)

#### Writes To

- `detail::g_llm_semaphore_mutex` (lock state changed)
- `detail::g_llm_semaphore` (reset to cleared state)

#### Usage Patterns

- Called during shutdown to disable rate limiting
- Used to reinitialize or clear rate limit state

## Internal Structure

模块 `http` 位于 `network/http.cppm`，对外暴露 `clore::net` 命名空间中的速率限制初始化/关闭函数 (`initialize_llm_rate_limit`, `shutdown_llm_rate_limit`) 和错误类型 (`LLMError`)，而请求执行细节封装在 `clore::net::detail` 内。这一分解将公共 API 与内部实现分离：公共接口用于启动/停止限流并报告错误，内部则包含同步 (`perform_http_request`) 与异步 (`perform_http_request_async`) 两种执行路径，以及环境配置解析 (`read_environment`, `read_required_env`)、请求配置 (`configure_request`)、结果解包 (`unwrap_caught_result`) 等工具函数。内部层次由多个匿名作用域中的常量和变量支撑，例如 TCP keep‑alive 参数、DNS 缓存超时、连接复用时长、HTTP 连接和请求超时，以及用于限流的全局信号量 (`g_llm_semaphore`) 和原子计数器 (`g_llm_request_counter`)。该模块仅依赖 `std` 和 `support` 两个导入，后者提供文本处理、文件 I/O 和缓存键工具等基础能力。实现结构上采用了守卫 (`SemaphoreGuard`) 确保限流信号的正确释放，并利用 `std::expected` 模式统一错误处理，从而保持了内部代码的健壮性和可维护性。

## Related Pages

- [Module support](../support/index.md)

