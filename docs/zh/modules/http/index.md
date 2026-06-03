---
title: 'Module http'
description: 'http 模块是 C++ 代码库中面向 LLM（大语言模型）网络通信的核心抽象层，位于命名空间 clore::net 内。它负责管理 HTTP 请求的整个生命周期：从环境变量中读取 API 密钥和基础地址、构造符合 LLM 接口要求的请求、支持同步（perform_http_request）与基于事件循环的异步（perform_http_request_async）调用方式，并公开了专用的结果类型 LLMError 和 RawHttpResponse 用于错误传递与响应解析。该模块内建了一套全局速率限制机制，通过初始化函数 initialize_llm_rate_limit 和配套的关闭函数 shutdown_llm_rate_limit 控制对 LLM API 的并发请求数量，确保网络资源的使用受到约束。此外，模块内部还定义了一系列与 TCP 连接、DNS 缓存及请求超时相关的编译期常量，为底层网络操作提供可配置的默认行为。'
layout: doc
template: doc
---

# Module `http`

## Summary

`http` 模块是 C++ 代码库中面向 LLM（大语言模型）网络通信的核心抽象层，位于命名空间 `clore::net` 内。它负责管理 HTTP 请求的整个生命周期：从环境变量中读取 API 密钥和基础地址、构造符合 LLM 接口要求的请求、支持同步（`perform_http_request`）与基于事件循环的异步（`perform_http_request_async`）调用方式，并公开了专用的结果类型 `LLMError` 和 `RawHttpResponse` 用于错误传递与响应解析。该模块内建了一套全局速率限制机制，通过初始化函数 `initialize_llm_rate_limit` 和配套的关闭函数 `shutdown_llm_rate_limit` 控制对 LLM API 的并发请求数量，确保网络资源的使用受到约束。此外，模块内部还定义了一系列与 TCP 连接、DNS 缓存及请求超时相关的编译期常量，为底层网络操作提供可配置的默认行为。

## Imports

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

Declaration: `src/network/http.cppm:37`

Definition: `src/network/http.cppm:37`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::LLMError` 的内部结构仅由一个 `std::string` 类型的数据成员 `message` 构成，所有构造函数均围绕其初始化。默认构造函数 `LLMError() = default` 使 `message` 保持空字符串状态，这是一个有效的不变量。接受 `std::string` 的构造函数通过 `std::move` 转移参数所有权，避免拷贝；接受 `kota::error` 的构造函数则调用 `err.message()` 提取底层错误描述字符串并存储。两个接受参数的构造函数均被标记为 `explicit`，防止隐式转换，从而保持构造意图明确。整个结构不依赖额外资源或复杂状态，其不变量始终保证 `message` 包含一个有效的 `std::string` 对象（可能为空）。

#### Invariants

- member `message` always contains a valid `std::string` (possibly empty)
- construction from `kota::error` ensures `message` reflects the content of `err.message()`
- default construction leaves `message` empty

#### Key Members

- `message`: the stored error description
- `LLMError()`: default constructor
- `LLMError(std::string msg)`: constructs from a string
- `LLMError(kota::error err)`: constructs from a `kota::error`

#### Usage Patterns

- returned as an error type from LLM‑related network operations
- interoperates with `kota::error` by converting its error information
- default‑constructed to represent an unset or placeholder error

#### Member Functions

##### `clore::net::LLMError::LLMError`

Declaration: `src/network/http.cppm:42`

Definition: `src/network/http.cppm:42`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

###### Implementation

```cpp
explicit LLMError(std::string msg) : message(std::move(msg)) {}
```

##### `clore::net::LLMError::LLMError`

Declaration: `src/network/http.cppm:44`

Definition: `src/network/http.cppm:44`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

###### Implementation

```cpp
explicit LLMError(kota::error err) : message(err.message()) {}
```

##### `clore::net::LLMError::LLMError`

Declaration: `src/network/http.cppm:40`

Definition: `src/network/http.cppm:40`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

###### Implementation

```cpp
LLMError() = default
```

### `clore::net::detail::EnvironmentConfig`

Declaration: `src/network/http.cppm:51`

Definition: `src/network/http.cppm:51`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

`clore::net::detail::EnvironmentConfig` 是一个仅有数据成员的聚合体，用于集中存储网络环境配置。其内部仅包含两个 `std::string` 类型字段：`api_base` 保存 API 基础 URL，`api_key` 保存认证密钥。该结构体不定义任何构造函数、析构函数或成员函数，完全依赖编译器生成的默认操作。其设计意图是作为内部传递的纯数据载体——字段值由调用方在构造时通过聚合初始化直接赋予，或通过后续成员赋值修改；调用链中任何对这些字段的读取都假定外部代码已确保 `api_base` 格式有效、`api_key` 非空，结构体自身并不执行运行时校验。生命周期由使用方直接管理，通常作为局部变量或传入的引用/指针出现，不承担资源所有权责任。

#### Key Members

- `api_base`
- `api_key`

### `clore::net::detail::RawHttpResponse`

Declaration: `src/network/http.cppm:56`

Definition: `src/network/http.cppm:56`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

`clore::net::detail::RawHttpResponse` 是一个简单的聚合类型，包含两个公共数据成员：`long http_status`（默认初始化为 `0`）和 `std::string body`（默认初始化为空字符串）。作为 `detail` 命名空间的一部分，它用于在内部组件间传递原始 HTTP 响应数据，未提供自定义构造函数或特殊成员函数，因此保持布局兼容性并避免运行时开销。该结构体的不变量在于调用方应自行设置恰当的 HTTP 状态码和响应正文，且默认值表示尚未收到有效响应。

#### Invariants

- `http_status` defaults to `0`, which is not a standard HTTP status code and indicates the field may not have been set.
- `body` defaults to an empty string, implying no response body has been assigned.

#### Key Members

- `clore::net::detail::RawHttpResponse::http_status`
- `clore::net::detail::RawHttpResponse::body`

#### Usage Patterns

- Used as a return type or intermediate representation when parsing HTTP responses.
- Other code in the network layer populates the fields after reading raw data.

## Variables

### `clore::net::detail::g_llm_request_counter`

Declaration: `src/network/http.cppm:111`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The counter is intended to provide a unique, thread-safe identifier or metric for each LLM request. Its atomic type ensures safe concurrent access, but the evidence does not describe how it is read, incremented, or used in surrounding logic beyond its declaration.

#### Mutation

No mutation is evident from the extracted code.

### `clore::net::detail::g_llm_semaphore`

Declaration: `src/network/http.cppm:62`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This semaphore controls concurrency for LLM requests, likely acquired via RAII guards such as `sem_guard` to limit simultaneous operations. It is initialized and shut down through dedicated functions, and access is synchronized with a companion mutex `g_llm_semaphore_mutex`.

#### Mutation Sources

- `clore::net::initialize_llm_rate_limit`
- `clore::net::shutdown_llm_rate_limit`

#### Usage Patterns

- Used with RAII guard objects for acquire/release
- Guarded by `clore::net::detail::g_llm_semaphore_mutex`
- Referenced in network request code paths

### `clore::net::detail::g_llm_semaphore_mutex`

Declaration: `src/network/http.cppm:61`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This mutex guards access to `clore::net::detail::g_llm_semaphore`, ensuring thread-safe operations on the semaphore object. It is locked by functions such as `clore::net::initialize_llm_rate_limit`, `clore::net::detail::current_llm_semaphore`, and `clore::net::shutdown_llm_rate_limit` before reading or modifying the semaphore. The mutex itself is not reassigned after initialization.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- guards access to `clore::net::detail::g_llm_semaphore`
- locked in `clore::net::initialize_llm_rate_limit`
- locked in `clore::net::detail::current_llm_semaphore`
- locked in `clore::net::shutdown_llm_rate_limit`

### `clore::net::detail::kConnMaxAgeSec`

Declaration: `src/network/http.cppm:116`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This constant is read in the function `clore::net::detail::configure_request` to set a connection age limit, likely preventing reuse of connections that exceed this duration. It participates in the HTTP request configuration logic to enforce connection freshness.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- used in `clore::net::detail::configure_request` to configure the maximum connection age

### `clore::net::detail::kDnsCacheTimeoutSec`

Declaration: `src/network/http.cppm:115`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This constant is consumed by `clore::net::detail::configure_request` to set the DNS cache timeout for outgoing HTTP requests, ensuring that resolved addresses are reused for a limited duration before a fresh lookup is performed.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- passed to `configure_request` as a DNS cache timeout value

### `clore::net::detail::kHttpConnectTimeoutMs`

Declaration: `src/network/http.cppm:113`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This constant is used by `clore::net::detail::configure_request` to set the connect timeout on `kota::http::request` objects. It is read as a compile-time value and passed as an argument to configure the underlying HTTP client's connection establishment timeout.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- passed as argument to `configure_request`

### `clore::net::detail::kHttpRequestTimeout`

Declaration: `src/network/http.cppm:114`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

As a `constexpr` constant, it is evaluated at compile time and cannot be mutated. It is intended to be used as a timeout value for HTTP request operations, ensuring that requests either complete or fail within the specified duration.

#### Mutation

No mutation is evident from the extracted code.

### `clore::net::detail::kTcpKeepIdleSec`

Declaration: `src/network/http.cppm:117`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This constant is consumed by `clore::net::detail::configure_request`, which likely applies it as the idle timeout for TCP keep-alive probes on outgoing HTTP connections, ensuring sockets remain alive during idle periods.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- passed to `clore::net::detail::configure_request`
- used as TCP keep-alive idle seconds

### `clore::net::detail::kTcpKeepIntvlSec`

Declaration: `src/network/http.cppm:118`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This constant is used inside `clore::net::detail::configure_request` to set the TCP keepalive interval for outgoing HTTP requests. It is read directly and passed to underlying socket configuration logic.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- used as a configuration parameter for TCP keepalive interval

## Functions

### `clore::net::detail::configure_request`

Declaration: `src/network/http.cppm:164`

Definition: `src/network/http.cppm:164`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数首先遍历 `headers` 参数中的每个 `kota::http::header` 元素，依次调用 `request.header(header.name, header.value)` 将 HTTP 头部设置到请求对象上。接着，它将 `request_json` 通过移动语义传递给 `request.body()` 以设置 HTTP 请求体。随后，通过一系列 `request.curl_option()` 调用配置底层 libcurl 的连接参数：设置连接超时时间为 `kHttpConnectTimeoutMs` 毫秒，禁用信号处理（`CURLOPT_NOSIGNAL`），启用 TCP 保活（`CURLOPT_TCP_KEEPALIVE`），并指定保活空闲时长 `kTcpKeepIdleSec` 和探测间隔 `kTcpKeepIntvlSec`，同时设置 DNS 缓存超时 `kDnsCacheTimeoutSec` 以及连接最大存活时间 `kConnMaxAgeSec`。这些常量均定义于 `clore::net::detail` 命名空间内，构成了函数对底层网络配置的全部依赖。

#### Side Effects

- 向 `request` 添加 HTTP 头部
- 设置 `request` 的请求体
- 配置 `request` 的 curl 选项（超时、keepalive、DNS缓存等）

#### Reads From

- 参数 `headers`（`std::span<const kota::http::header>`）
- 参数 `request_json`（`std::string`）
- 常量 `kHttpConnectTimeoutMs`
- 常量 `kTcpKeepIdleSec`
- 常量 `kTcpKeepIntvlSec`
- 常量 `kDnsCacheTimeoutSec`
- 常量 `kConnMaxAgeSec`

#### Writes To

- 参数 `request`（`kota::http::request &`）

#### Usage Patterns

- 在发起 HTTP 请求前调用以配置请求对象
- 用于设置统一超时和 keepalive 策略
- 作为 `perform_http_request` 或 `perform_http_request_async` 的准备工作

### `clore::net::detail::perform_http_request`

Declaration: `src/network/http.cppm:67`

Definition: `src/network/http.cppm:181`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

`clore::net::detail::perform_http_request` 创建一个本地 `async::event_loop` 实例，并将输入的 `headers` 复制到一个 `std::vector` 中。然后调用 `perform_http_request_async`，传入 URL、请求体 JSON 和循环，获取一个可取消的操作。该操作被调度到循环中，然后循环运行直到完成。最后，检查操作的结果：如果被取消，则返回一个包含取消消息的 `std::unexpected<LLMError>`；如果操作包含错误，则转发该错误；否则返回成功的结果，即 `RawHttpResponse`。

此实现依赖于 `perform_http_request_async` 来实际执行 HTTP 请求，并使用 `async::event_loop` 进行同步等待。整个流程在单个线程内完成，确保请求的同步语义。

#### Side Effects

- Performs HTTP I/O via the underlying async mechanism
- Allocates memory for the header vector copy
- Creates and runs an `async::event_loop` instance

#### Reads From

- parameter `url`
- parameter `headers` (span)
- parameter `request_json`

#### Writes To

- Returns a `RawHttpResponse` via `std::expected`
- Modifies the state of the local `async::event_loop` during execution

#### Usage Patterns

- Used as a blocking convenience function by callers that want a simple synchronous HTTP request
- Often called when the async infrastructure should be hidden from the caller

### `clore::net::detail::perform_http_request_async`

Declaration: `src/network/http.cppm:72`

Definition: `src/network/http.cppm:209`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数实现一个异步 HTTP POST 请求，用于调用 LLM API，返回一个 `async::task<RawHttpResponse, LLMError>`。内部首先通过 `current_llm_semaphore` 获取全局信号量，若存在则 `co_await` 等待，以控制并发量；随后创建一个 `SemaphoreGuard` 确保信号量最终被释放。接着原子递增 `g_llm_request_counter` 并记录日志，然后获取线程本地的 HTTP 客户端 `get_thread_http_client()`，在其上构造一个 `post` 请求，指定 `loop` 并设置 `kHttpRequestTimeout` 超时，再调用 `configure_request` 添加请求头 `headers` 和请求体 `request_json`。发送请求后通过 `catch_cancel` 处理取消情况：若取消则立即释放信号量并 `co_await async::fail` 返回 `LLMError`；若出现其他错误则记录警告、释放信号量并返回错误。请求成功时，从响应中提取 HTTP 状态码和文本体构造 `RawHttpResponse`，记录完成日志，然后释放信号量并 `co_return` 该结果。

控制流依赖以下关键组件：`current_llm_semaphore`（获取信号量共享指针）、`get_thread_http_client`（提供线程安全的客户端实例）、`configure_request`（填充请求头与体）、`kHttpRequestTimeout`（超时常量）、`LLMError` 和 `RawHttpResponse` 类型、`async::fail`（产生异常任务）、以及 `logging::info/warn`（日志）。信号量通过 `SemaphoreGuard` 在成功、取消或错误路径上手动释放，确保在协程返回前立即唤醒等待者。

#### Side Effects

- acquires and releases a shared semaphore via `current_llm_semaphore`
- increments the global atomic counter `g_llm_request_counter`
- sends an HTTP POST request over the network
- logs informational messages for request start and completion
- logs a warning message on request failure

#### Reads From

- `url` parameter
- `headers` parameter
- `request_json` parameter
- `loop` parameter
- global semaphore from `current_llm_semaphore()`
- global counter `g_llm_request_counter`
- thread-local HTTP client from `get_thread_http_client()`
- constant `kHttpRequestTimeout`

#### Writes To

- global counter `g_llm_request_counter` via `fetch_add`
- semaphore state via `acquire()` and `release()`
- network output via HTTP request
- log output via `logging::info` and `logging::warn`

#### Usage Patterns

- called to asynchronously invoke an LLM API endpoint
- used with an event loop for non-blocking I/O
- provides rate limiting through semaphore acquisition
- returns a task that can be awaited in a coroutine context

### `clore::net::detail::read_environment`

Declaration: `src/network/http.cppm:64`

Definition: `src/network/http.cppm:146`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

实现上，`clore::net::detail::read_environment` 完全依赖于 `clore::net::detail::read_required_env` 提供的环境变量查询能力。控制流分为三个步骤：首先以 `base_env` 为键调用 `read_required_env`，若结果不包含值则立即返回 `std::unexpected` 包装的错误；然后以 `key_env` 调用 `read_required_env`，同样在失败时提前返回错误；最后利用两次成功获得的值直接聚合构造 `EnvironmentConfig` 对象返回。该函数不涉及底层网络操作或并发控制，其核心职责是将两个独立的环境变量读取结果组合为一个配置结构，并通过 `std::expected` 的传播机制统一处理错误路径。

#### Side Effects

- reads environment variables via `read_required_env`
- allocates memory for `std::string` values

#### Reads From

- environment variables named by `base_env` and `key_env`

#### Usage Patterns

- called to obtain `EnvironmentConfig` from environment variable names
- typical invocation: `read_environment("API_BASE", "API_KEY")`

### `clore::net::detail::read_required_env`

Declaration: `src/network/http.cppm:137`

Definition: `src/network/http.cppm:137`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::read_required_env` 的核心算法非常简单：它接受一个环境变量名称 `name`，调用 `std::getenv` 获取其值。内部控制流根据返回值进行分支——若 `std::getenv` 返回空指针或返回的 C 字符串首个字符为空字符（即变量未设置或值为空串），则立即返回 `std::unexpected` 包装的 `LLMError`，其中错误消息通过 `std::format` 生成并包含变量名；否则，将 C 字符串拷贝为 `std::string` 并正常返回。该函数不涉及外部网络调用或复杂状态管理，依赖于标准库的 `std::getenv` 和字符串格式化工具。

该函数的实现依赖 `LLMError` 类型的统一构造方式（此处使用 `std::string` 参数的构造函数）来封装环境缺失错误，是整个读环境变量流程中负责强制检查的底层辅助。它没有访问其他模块的全局状态或锁，仅通过标准库 API 完成字符串检索与转换，错误处理路径干净且无资源泄漏风险。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the environment variable named by parameter `name` via `std::getenv`

#### Usage Patterns

- used to obtain required configuration values from the environment
- likely called by `clore::net::detail::read_environment` or similar initialization functions

### `clore::net::detail::unwrap_caught_result`

Declaration: `src/network/http.cppm:78`

Definition: `src/network/http.cppm:78`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

`clore::net::detail::unwrap_caught_result` 是一个模板函数，它接收一个结果对象 `R` 和一个取消消息字符串。其内部流程按顺序处理三种情况：首先检查 `result.is_cancelled()`，若成立则通过 `kota::fail` 协程等待机制投递一个以 `cancel_message` 构造的 `LLMError` 并立即结束；接着检查 `result.has_error()`，若存在错误则通过 `kota::fail` 投递 `std::move(result).error()` 取出的底层错误；若以上均不满足，则通过 `co_return` 返回 `std::move(*result)` 中的成功值。该函数依赖 `R` 提供 `is_cancelled`、`has_error`、`error` 以及解引用操作，并依赖 `kota::task` 与 `kota::fail` 作为协程框架，以及 `LLMError` 的统一错误类型构造。整个控制流确保所有异常路径都被转换为 `LLMError` 并推迟到调用方处理，成功路径则安全地提取底层值。

#### Side Effects

- Construction of `LLMError` with copied cancel message string
- Moving of value from `result` (ownership transfer)
- Potential allocation in `LLMError` constructor

#### Reads From

- `result` parameter
- `cancel_message` parameter
- `result.is_cancelled()`
- `result.has_error()`
- `*result`

#### Writes To

- Returned `kota::task` object
- `LLMError` objects created by `kota::fail`

#### Usage Patterns

- Used to unwrap a caught result from async operations
- Provides a custom cancellation message
- Converts to a task with `LLMError` error type

### `clore::net::initialize_llm_rate_limit`

Declaration: `src/network/http.cppm:33`

Definition: `src/network/http.cppm:93`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

该函数通过一个互斥锁 `detail::g_llm_semaphore_mutex` 保护对全局信号量 `detail::g_llm_semaphore` 的更新。如果传入的 `rate_limit` 值为 `0`，则调用 `detail::g_llm_semaphore.reset()` 释放已有信号量，从而禁用速率限制；否则，以 `rate_limit` 作为初始计数创建一个新的 `kota::semaphore` 对象并赋值给 `detail::g_llm_semaphore`。整个操作由 `std::lock_guard` 确保线程安全，且不涉及任何异步或网络调用，仅依赖标准库的互斥锁和 `kota::semaphore` 类型。

#### Side Effects

- acquires a mutex
- modifies global semaphore pointer
- potentially destroys old semaphore if `rate_limit` is 0

#### Reads From

- parameter `rate_limit`
- global mutex `detail::g_llm_semaphore_mutex`
- global shared pointer `detail::g_llm_semaphore`

#### Writes To

- global shared pointer `detail::g_llm_semaphore`
- global mutex `detail::g_llm_semaphore_mutex` (lock/unlock)
- semaphore object if `rate_limit` is 0 (destruction)

#### Usage Patterns

- called to configure the maximum concurrent LLM requests
- used in initialization code

### `clore::net::shutdown_llm_rate_limit`

Declaration: `src/network/http.cppm:35`

Definition: `src/network/http.cppm:277`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

函数 `clore::net::shutdown_llm_rate_limit` 的实现通过一个互斥锁操作来安全地销毁全局 LLM 信号量实例。它首先获取 `clore::net::detail::g_llm_semaphore_mutex` 的独占锁，然后调用 `clore::net::detail::g_llm_semaphore.reset()`，将 `std::unique_ptr` 管理的信号量对象置为空。该实现依赖于 `clore::net::detail` 命名空间下的全局互斥锁和信号量变量，不涉及其他网络或异步调用。

#### Side Effects

- Acquires a lock on `detail::g_llm_semaphore_mutex`
- Resets `detail::g_llm_semaphore`

#### Reads From

- `detail::g_llm_semaphore_mutex`
- `detail::g_llm_semaphore`

#### Writes To

- `detail::g_llm_semaphore_mutex`
- `detail::g_llm_semaphore`

#### Usage Patterns

- Called during shutdown to disable LLM rate limiting
- Used to reset the rate limiter state when reinitializing

## Internal Structure

`clore::net` 的 HTTP 模块分为公共接口和内部实现两层。公共命名空间下暴露了 LLM 相关错误类型 `LLMError` 以及速率限制生命周期管理函数 `initialize_llm_rate_limit` 与 `shutdown_llm_rate_limit`；所有具体的 HTTP 通信、配置解析和资源管理细节均封装在 `clore::net::detail` 命名空间中。模块导入 `support` 以利用通用工具和日志基础设施。

内部实现围绕两个核心请求函数组织：同步的 `perform_http_request` 与异步的 `perform_http_request_async`，后者依赖 `async::event_loop` 并借助全局信号量 (`g_llm_semaphore`) 和原子计数器 (`g_llm_request_counter`) 实现速率控制。环境配置通过 `read_environment` 与 `read_required_env` 读取，转换为 `EnvironmentConfig` 结构体；`configure_request` 负责组装 `kota::http::request`。此外，模块定义了多种 TCP 和 DNS 超时常量（如 `kHttpRequestTimeout`、`kDnsCacheTimeoutSec` 等）以及线程本地的 HTTP 客户端获取函数 `get_thread_http_client`，构成了清晰的内部层次：运行时配置 → 请求构建 → 速率限制 → 底层发送与错误包装（`RawHttpResponse`、`unwrap_caught_result`）。

## Related Pages

- [Module support](../support/index.md)

