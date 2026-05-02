---
title: 'Module http'
description: 'The http module provides the foundational HTTP networking layer for LLM (Large Language Model) API communication within the codebase. Its primary responsibility is managing the full lifecycle of HTTP requests – from reading environment variables (API keys, base URLs) and configuring request objects, to performing both synchronous (perform_http_request) and asynchronous (perform_http_request_async) HTTP operations, and interpreting responses and errors via dedicated types such as RawHttpResponse and LLMError. The module also integrates a rate-limiting subsystem (initialize_llm_rate_limit / shutdown_llm_rate_limit) that controls concurrency for LLM requests, using a semaphore and associated counters.'
layout: doc
template: doc
---

# Module `http`

## Summary

The `http` module provides the foundational HTTP networking layer for LLM (Large Language Model) API communication within the codebase. Its primary responsibility is managing the full lifecycle of HTTP requests – from reading environment variables (API keys, base `URLs`) and configuring request objects, to performing both synchronous (`perform_http_request`) and asynchronous (`perform_http_request_async`) HTTP operations, and interpreting responses and errors via dedicated types such as `RawHttpResponse` and `LLMError`. The module also integrates a rate-limiting subsystem (`initialize_llm_rate_limit` / `shutdown_llm_rate_limit`) that controls concurrency for LLM requests, using a semaphore and associated counters.

Public-facing implementation scope owned by this module includes the rate limit initialization and shutdown functions, the `LLMError` error type, the `detail` sub-namespace containing request execution (`perform_http_request`, `perform_http_request_async`), environment configuration readers (`read_environment`, `read_required_env`), request configuration helper (`configure_request`), error unwrapping utility (`unwrap_caught_result`), and various compile-time networking constants (connection timeouts, DNS cache timeout, TCP keepalive intervals). Internally, it manages a global request identifier counter, a shared semaphore for rate limiting, and per-thread HTTP clients to ensure safe concurrent access.

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

`clore::net::LLMError` is a minimal error carrier holding a single `std::string message` member that captures a human-readable description of a failure encountered during LLM network interactions. The default constructor is defaulted, leaving `message` empty, which serves as the "no error set" state for default-constructed instances.

The two value constructors are marked `explicit` to prevent implicit conversions: the `std::string` overload moves its argument into `message` via `std::move(msg)`, while the `kota::error` overload bridges from the `kota` error model by storing the result of `err.message()`. There are no additional invariants beyond those of `std::string` itself.

#### Invariants

- `message` always holds the error description as a `std::string`
- constructors taking arguments are `explicit` to prevent implicit conversions
- default-constructed instances have an empty `message`

#### Key Members

- `message` field of type `std::string`
- default constructor `LLMError()`
- `explicit LLMError(std::string msg)` constructor
- `explicit LLMError(kota::error err)` constructor adapting from `kota::error`

#### Usage Patterns

- constructed from a raw message string
- constructed by adapting a `kota::error` into an LLM-specific error
- used as an error representation within `clore::net` HTTP/LLM code paths

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

The struct `clore::net::detail::EnvironmentConfig` is a plain data aggregate that bundles two `std::string` members: `api_base` and `api_key`. Its role is to hold environment‑specific HTTP configuration parameters—the base URL for API calls and the authentication key—as a single, movable value. No invariants are enforced beyond the default behavior of `std::string`; the caller is responsible for providing suitable non‑empty strings when required. The struct has no user‑defined constructors, assignment `operator`s, or destructor, relying on compiler‑generated implementations that correctly copy or move the contained strings.

#### Invariants

- Both members are always of type `std::string`
- No guarantee of non-empty or valid content

#### Key Members

- `api_base`
- `api_key`

#### Usage Patterns

- Constructed with environment-specific values before initializing higher-level network objects
- Passed by value or const reference to setup HTTP clients or service wrappers

### `clore::net::detail::RawHttpResponse`

Declaration: `network/http.cppm:42`

Definition: `network/http.cppm:42`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The struct `clore::net::detail::RawHttpResponse` is a simple internal data holder that stores the raw result of an HTTP response before higher‑level processing. It consists of two fields: `http_status`, a `long` initialised to `0`, and `body`, a `std::string`. The type has an implicit invariant that an `http_status` of `0` should be interpreted as “status not yet assigned” or an uninitialised response, while a non‑zero value represents an actual HTTP status code. The `body` field holds the complete response payload as received over the wire, and may be empty if no body was sent. No custom constructors, assignment `operator`s, or destructor are declared; the compiler‑generated versions are sufficient because the struct is trivially copyable and movable. The sole purpose of `RawHttpResponse` is to act as a transient vessel within the HTTP networking internals, decoupling the raw byte acquisition from the subsequent parsing and application‑facing response objects.

#### Invariants

- `http_status` may be zero or any valid HTTP status code
- `body` may be empty or contain response content

#### Key Members

- `http_status`
- `body`

#### Usage Patterns

- Used as a return type or intermediate data holder for HTTP responses
- Likely populated by HTTP parsing or networking code

## Variables

### `clore::net::detail::g_llm_request_counter`

Declaration: `network/http.cppm:97`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This counter is incremented each time a new HTTP request is initiated by `perform_http_request_async`. Its value is read atomically to assign a unique numeric identifier to each request, aiding in logging and tracking.

#### Mutation Sources

- incremented inside `perform_http_request_async`

#### Usage Patterns

- read to produce a unique request number
- used in HTTP request lifecycle of `perform_http_request_async`

### `clore::net::detail::g_llm_semaphore`

Declaration: `network/http.cppm:48`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The semaphore is managed by `clore::net::initialize_llm_rate_limit` which initializes it with a maximum count, and by `clore::net::shutdown_llm_rate_limit` which resets or releases it. It synchronizes access to LLM request operations, enforcing a limit on simultaneous requests.

#### Mutation Sources

- `clore::net::initialize_llm_rate_limit` assigns a new semaphore
- `clore::net::shutdown_llm_rate_limit` resets or destroys the semaphore

#### Usage Patterns

- referenced in rate‑limiting setup and teardown functions
- used to enforce a maximum concurrency of LLM requests

### `clore::net::detail::g_llm_semaphore_mutex`

Declaration: `network/http.cppm:47`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This mutex guards the shared LLM semaphore state, such as `clore::net::detail::g_llm_semaphore`. It is used by the functions `clore::net::initialize_llm_rate_limit`, `clore::net::detail::(anonymous namespace)::current_llm_semaphore`, and `clore::net::shutdown_llm_rate_limit` to ensure thread-safe access when modifying or querying the semaphore.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- locked/unlocked in `clore::net::initialize_llm_rate_limit`
- locked/unlocked in `clore::net::detail::(anonymous namespace)::current_llm_semaphore`
- locked/unlocked in `clore::net::shutdown_llm_rate_limit`

### `clore::net::detail::kConnMaxAgeSec`

Declaration: `network/http.cppm:102`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

It defines the maximum age of a connection in seconds, used as a timeout parameter when configuring HTTP requests via `clore::net::detail::configure_request`.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Read by `clore::net::detail::configure_request` to set connection max age

### `clore::net::detail::kDnsCacheTimeoutSec`

Declaration: `network/http.cppm:101`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This variable is used within `clore::net::detail::configure_request` to configure the DNS cache timeout for HTTP requests. As a `constexpr` value, it is evaluated at compile time and cannot be modified at runtime.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Used in `clore::net::detail::configure_request` to set DNS cache timeout

### `clore::net::detail::kHttpConnectTimeoutMs`

Declaration: `network/http.cppm:99`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This constant is consumed by the function `clore::net::detail::configure_request` to set the connection timeout on HTTP requests. It is used as a timeout parameter to limit how long the system waits for a TCP connection to be established, ensuring that requests do not hang indefinitely.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- passed to `configure_request` to set connection timeout

### `clore::net::detail::kHttpRequestTimeout`

Declaration: `network/http.cppm:100`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

Because it is `constexpr`, this value is evaluated at compile time and cannot be modified at runtime. It provides a standardized timeout limit for HTTP request operations, ensuring requests do not hang indefinitely. Its exact consumption site is not shown in the provided evidence, but by naming and context it is likely used to set a timeout on the HTTP client's request handling.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Referenced as a constant timeout value in HTTP request logic (inferred from name and module context).

### `clore::net::detail::kTcpKeepIdleSec`

Declaration: `network/http.cppm:103`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This constant is consumed by the function `clore::net::detail::configure_request`, which uses it to set the TCP keep-alive idle timeout on outgoing HTTP requests. Its value is passed directly to underlying socket options, influencing network connection lifecycle.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- reads in `clore::net::detail::configure_request` to set socket keep-alive idle timeout

### `clore::net::detail::kTcpKeepIntvlSec`

Declaration: `network/http.cppm:104`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This constant defines the number of seconds between TCP keepalive probes. It is consumed by the `clore::net::detail::configure_request` function to configure the keepalive interval on HTTP requests, ensuring idle connections are probed and potentially closed if unresponsive.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- consumed as a constant in `clore::net::detail::configure_request` to set the TCP keepalive interval

## Functions

### `clore::net::detail::configure_request`

Declaration: `network/http.cppm:150`

Definition: `network/http.cppm:150`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::configure_request` performs a linear sequence of operations to prepare a `kota::http::request` instance for an outgoing LLM API call. It iterates over the provided `std::span<const kota::http::header>`, calling `request.header(name, value)` for each element to attach HTTP headers. After all headers are set, the function moves the `request_json` string into the request body via `request.body(std::move(request_json))`.

The remaining steps configure low‑level `cURL` settings through a series of `request.curl_option` calls. These set the connection timeout (`CURLOPT_CONNECTTIMEOUT_MS` using `kHttpConnectTimeoutMs`), disable signal handling (`CURLOPT_NOSIGNAL`), enable TCP keep‑alive (`CURLOPT_TCP_KEEPALIVE`), and configure keep‑alive idle time (`CURLOPT_TCP_KEEPIDLE` from `kTcpKeepIdleSec`) and interval (`CURLOPT_TCP_KEEPINTVL` from `kTcpKeepIntvlSec`). DNS cache lifetime is set with `CURLOPT_DNS_CACHE_TIMEOUT` from `kDnsCacheTimeoutSec`, and the maximum connection age is set with `CURLOPT_MAXAGE_CONN` from `kConnMaxAgeSec`. The function has no internal branches or error handling; all constants are defined elsewhere in the detail namespace, and the caller must ensure the request object is valid.

#### Side Effects

- Modifies the provided `kota::http::request` object by setting HTTP headers, the request body, and curl options for timeout, keepalive, DNS caching, and connection reuse.

#### Reads From

- `headers` span of `kota::http::header`
- `request_json` string
- constants `kHttpConnectTimeoutMs`, `kTcpKeepIdleSec`, `kTcpKeepIntvlSec`, `kDnsCacheTimeoutSec`, `kConnMaxAgeSec`

#### Writes To

- The `kota::http::request` object: its headers, body, and curl options (`CURLOPT_CONNECTTIMEOUT_MS`, `CURLOPT_NOSIGNAL`, `CURLOPT_TCP_KEEPALIVE`, `CURLOPT_TCP_KEEPIDLE`, `CURLOPT_TCP_KEEPINTVL`, `CURLOPT_DNS_CACHE_TIMEOUT`, `CURLOPT_MAXAGE_CONN`)

#### Usage Patterns

- Called during HTTP request preparation to apply standard configuration before sending the request
- Used in the HTTP client flow to centralize setup of headers, body, and performance-related options

### `clore::net::detail::perform_http_request`

Declaration: `network/http.cppm:53`

Definition: `network/http.cppm:167`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::perform_http_request` acts as a synchronous wrapper around the asynchronous `clore::net::detail::perform_http_request_async`. It creates a local `async::event_loop`, copies the input `headers` into a `headers_vec`, constructs an async operation from `perform_http_request_async`, and attaches a `.catch_cancel()` handler to it. The operation is scheduled on the loop, which is then run to completion, blocking the caller until the HTTP request finishes.

After the loop terminates, the function inspects the `operation.result()`. If the result was cancelled (e.g., due to `clore::net::shutdown_llm_rate_limit`), it returns a `std::unexpected` containing an `LLMError` with a cancellation message. If the result contains an error, that error is moved into the unexpected return. Otherwise, the contained value (a `RawHttpResponse`) is returned as the expected success. Key dependencies are the asynchronous request internal logic (which handles rate limiting, DNS caching, TLS, and environment-based configuration) and the event loop infrastructure provided by `async::event_loop`.

#### Side Effects

- Performs network I/O via HTTP request
- Allocates memory for header copy and string copies
- Runs an event loop synchronously

#### Reads From

- `url` parameter
- `headers` parameter
- `request_json` parameter

#### Usage Patterns

- Wraps asynchronous HTTP request into synchronous call
- Used when a blocking HTTP request is needed

### `clore::net::detail::perform_http_request_async`

Declaration: `network/http.cppm:58`

Definition: `network/http.cppm:195`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::perform_http_request_async` is a coroutine that executes an HTTP POST request to a language model endpoint with rate-limiting support. It first acquires a shared semaphore from `current_llm_semaphore()` using a `co_await`; the semaphore is stored in a RAII `SemaphoreGuard` to ensure release on all exit paths (including cancellation and errors). The request is logged with an incrementing counter from `g_llm_request_counter`. The implementation retrieves a thread‑local HTTP client via `get_thread_http_client()`, constructs a POST on the provided `loop`, sets a timeout using `kHttpRequestTimeout`, and delegates header and body configuration to `configure_request`. The request is sent with `.send().catch_cancel()`; if cancelled or erroneous, the guard releases the semaphore and the coroutine fails with an `LLMError`. On success, a `RawHttpResponse` is constructed from the HTTP status and body text, a completion log is emitted, the semaphore is released, and the response is returned.

#### Side Effects

- acquires and releases a semaphore (`kota::semaphore` via `SemaphoreGuard`)
- increments the global atomic counter `g_llm_request_counter`
- calls `logging::info` to log request start and completion
- calls `logging::warn` on request failure
- performs an asynchronous HTTP POST request via `request.send()`
- cancels the asynchronous operation on cancellation via `co_await async::fail`

#### Reads From

- parameter `url` (`std::string`)
- parameter `headers` (`std::vector<kota::http::header>`)
- parameter `request_json` (`std::string`)
- parameter `loop` (`async::event_loop`&)
- global semaphore returned by `current_llm_semaphore()`
- global atomic counter `g_llm_request_counter`
- constant `kHttpRequestTimeout` (presumed integer or duration)
- thread-local HTTP client from `get_thread_http_client()`

#### Writes To

- global counter `g_llm_request_counter` (via `fetch_add`)
- semaphore (release via `SemaphoreGuard::release` or destructor)
- logs via `logging::info` and `logging::warn`
- local `RawHttpResponse` object returned via `co_return`

#### Usage Patterns

- called within an asynchronous coroutine context using `co_await`
- used to send LLM HTTP requests with concurrency limiting via semaphore
- paired with an `async::event_loop` for non-blocking I/O
- handles cancellation and error propagation for robust callers

### `clore::net::detail::read_environment`

Declaration: `network/http.cppm:50`

Definition: `network/http.cppm:132`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function sequentially calls `clore::net::detail::read_required_env` with the `base_env` and `key_env` arguments. Each call returns a `std::expected<std::string, LLMError>`. If either call does not contain a value, the function immediately propagates the failure by returning `std::unexpected` containing the moved error from the failed call. Only when both environment lookups succeed does it construct a `clore::net::detail::EnvironmentConfig` with the two resolved values (`api_base` and `api_key`). The algorithm is purely sequential and relies entirely on `read_required_env` for error handling; there is no retry or fallback logic.

#### Side Effects

- reads environment variables via `clore::net::detail::read_required_env`

#### Reads From

- `base_env` parameter
- `key_env` parameter
- environment variables via `clore::net::detail::read_required_env`

#### Writes To

- local variable `api_base`
- local variable `api_key`
- return value of type `clore::net::detail::EnvironmentConfig`

#### Usage Patterns

- reading API configuration from environment variables at startup
- initializing an `EnvironmentConfig` from two named environment variables

### `clore::net::detail::read_required_env`

Declaration: `network/http.cppm:123`

Definition: `network/http.cppm:123`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::read_required_env` retrieves the value of a mandatory environment variable identified by the parameter `name`. It first converts the `std::string_view` `name` to a C‑string via an intermediate `std::string`, then calls `std::getenv` to obtain the raw pointer. If the pointer is `nullptr` (variable not set) or the first character is the null terminator (empty string), the function returns `std::unexpected` containing an `LLMError` whose message is constructed using `std::format` to indicate the missing variable. Otherwise, it returns a `std::expected<std::string, LLMError>` holding a copy of the value as a `std::string`. The only external dependencies are the standard library functions `std::getenv` and `std::format`, plus the custom error type `LLMError`.

#### Side Effects

- reads from the process environment
- allocates heap memory for the returned `std::string` and the temporary `std::string` argument to getenv

#### Reads From

- environment variable named by the parameter `name`

#### Usage Patterns

- required configuration variable retrieval
- validate existence and non-emptiness of an environment variable

### `clore::net::detail::unwrap_caught_result`

Declaration: `network/http.cppm:64`

Definition: `network/http.cppm:64`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function first evaluates the cancellation state of the incoming `result` by calling `result.is_cancelled()`. If true, it immediately `co_awaits` `kota::fail` with an `LLMError` constructed from the provided `cancel_message`, converting the message string into the error type. Otherwise, it checks `result.has_error()`; if an error is present, it `co_awaits` `kota::fail` forwarding the error via `std::move(result).error()`. If neither condition holds, the function proceeds to `co_return` the value extracted from the result (`std::move(*result)`), unwrapping the successful payload. This control flow ensures that cancellation and error propagation are handled before returning the inner value, relying on the `LLMError` type for error representation and on `kota::task`’s coroutine primitives for asynchronous failure injection.

#### Side Effects

- Invokes `kota::fail`, which records or propagates an error through the coroutine's failure mechanism when the result is cancelled or has an error.

#### Reads From

- `result` (via `is_cancelled()`, `has_error()`, `operator*()`)
- `cancel_message` (used to create an `LLMError` on cancellation)

#### Writes To

- Sets the coroutine's error state via `co_await kota::fail(LLMError(...))` or `co_await kota::fail(std::move(result).error())`

#### Usage Patterns

- Used in asynchronous result handling to unwrap a `R` type that may indicate cancellation or error into a `kota::task`.
- Typically called at the end of an async operation to convert a caught result from `perform_http_request_async` or similar into a task result.

### `clore::net::initialize_llm_rate_limit`

Declaration: `network/http.cppm:19`

Definition: `network/http.cppm:79`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function acquires a mutex lock on `detail::g_llm_semaphore_mutex` to safely modify the global semaphore instance stored in `detail::g_llm_semaphore`. If the provided `rate_limit` is zero, the semaphore is reset to a null pointer, effectively disabling rate limiting. Otherwise, a new `kota::semaphore` is allocated with an initial count equal to the integer `rate_limit` (cast to `std::ptrdiff_t`), which governs the maximum number of concurrent LLM requests allowed by downstream functions such as `clore::net::detail::perform_http_request_async`. This function serves as the initialization point for the rate‑limiting mechanism and is typically paired with `clore::net::shutdown_llm_rate_limit` for teardown.

#### Side Effects

- acquires mutex `detail::g_llm_semaphore_mutex`
- modifies global `detail::g_llm_semaphore`
- allocates a new `kota::semaphore` if `rate_limit` > 0
- resets the global semaphore if `rate_limit` == 0

#### Reads From

- parameter `rate_limit`

#### Writes To

- global `detail::g_llm_semaphore`

#### Usage Patterns

- called during startup to set LLM rate limit
- called with 0 to disable rate limiting

### `clore::net::shutdown_llm_rate_limit`

Declaration: `network/http.cppm:21`

Definition: `network/http.cppm:263`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The implementation of `clore::net::shutdown_llm_rate_limit` is straightforward: it acquires exclusive access via a `std::lock_guard` on `clore::net::detail::g_llm_semaphore_mutex`, then resets (`std::shared_ptr::reset`) the `clore::net::detail::g_llm_semaphore` shared pointer. This destruction of the underlying `kota::semaphore` object releases any waiting threads and disables the rate‑limiting mechanism. The function is `noexcept` and relies solely on the global mutex and semaphore variables defined in the `clore::net::detail` namespace. No other dependencies or control flow branching exist—the lock is the only synchronisation point, ensuring safe teardown even if concurrent calls to `perform_http_request_async` are still active.

#### Side Effects

- Acquires the global mutex `detail::g_llm_semaphore_mutex`.
- Resets the global semaphore `detail::g_llm_semaphore`, modifying its internal state.

#### Reads From

- `detail::g_llm_semaphore_mutex` (via locking)
- `detail::g_llm_semaphore` (via its `reset()` method)

#### Writes To

- `detail::g_llm_semaphore` (reset to a default/empty state)

#### Usage Patterns

- Called to disable or reinitialize the LLM rate limiter during shutdown
- Complement to `initialize_llm_rate_limit` for lifecycle management

## Internal Structure

The `http` module (defined in `network/http.cppm`) belongs to the `clore::net` namespace and provides HTTP communication capabilities for LLM (Large Language Model) API interactions. It imports the standard library and a `support` module for foundational utilities. The module is decomposed into a public interface and an internal `detail` namespace. The public API exposes `LLMError` for structured error reporting, along with initialization and shutdown functions for rate limiting (`initialize_llm_rate_limit`, `shutdown_llm_rate_limit`). The `detail` namespace encapsulates all implementation internals, including environment configuration (`EnvironmentConfig`, `read_environment`, `read_required_env`), synchronous and asynchronous HTTP request functions (`perform_http_request`, `perform_http_request_async`), request configuration (`configure_request`), and error conversion (`unwrap_caught_result`).

Internally, the module is layered around rate-limited HTTP access. A global semaphore (`g_llm_semaphore`) and its mutex (`g_llm_semaphore_mutex`) control concurrent LLM requests, while an atomic counter (`g_llm_request_counter`) assigns unique identifiers. Thread-local HTTP clients are managed via `get_thread_http_client`, and a local `SemaphoreGuard` ensures proper semaphore release during asynchronous operations. Compile-time constants (`kHttpConnectTimeoutMs`, `kHttpRequestTimeout`, `kDnsCacheTimeoutSec`, `kTcpKeepIdleSec`, `kTcpKeepIntvlSec`, `kConnMaxAgeSec`) define networking timeouts and keep-alive settings. The implementation separates synchronous blocking requests from event-loop-driven asynchronous requests, both relying on the same configuration and rate-limiting infrastructure.

## Related Pages

- [Module support](../support/index.md)

