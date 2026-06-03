---
title: 'Module http'
description: 'The http module is responsible for handling HTTP networking specifically for LLM (large language model) interactions. It provides both synchronous and asynchronous HTTP request functions, each configured with rate‑limiting, connection timeouts, and keep‑alive settings. The module also manages environment‑based configuration: reading required and optional settings (such as API keys and base URLs) into structured EnvironmentConfig objects, and exposing them for use in request construction.'
layout: doc
template: doc
---

# Module `http`

## Summary

The `http` module is responsible for handling HTTP networking specifically for LLM (large language model) interactions. It provides both synchronous and asynchronous HTTP request functions, each configured with rate‑limiting, connection timeouts, and keep‑alive settings. The module also manages environment‑based configuration: reading required and optional settings (such as API keys and base `URLs`) into structured `EnvironmentConfig` objects, and exposing them for use in request construction.

Its public‑facing implementation scope includes rate‑limiting control through `initialize_llm_rate_limit` and `shutdown_llm_rate_limit`, synchronous and asynchronous request functions (`perform_http_request` and `perform_http_request_async`), helper functions for reading environment variables (`read_environment`, `read_required_env`), and error representation via `LLMError`. All networking is built on top of the underlying `kota` library, and the module imports the `support` module for foundational utilities. The internal detail namespace holds thread‑local clients, semaphores for concurrency control, and constants for connection tuning.

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

The struct `clore::net::LLMError` is a simple error wrapper that stores a human-readable description in a single `std::string` member named `message`.  The default constructor is `= default`, which leaves `message` empty; this empty state is valid and represents the absence of an error.  Two explicit constructors populate the message: one takes a `std::string` directly (via move), and the other takes a `kota::error` and copies the error’s `.message()` into the member.  Both constructors are marked `explicit` to prevent accidental implicit conversions from strings or `kota::error` objects.  The internal invariant is that `message` always holds the description it was constructed with, with no additional validation or formatting logic.

#### Invariants

- `message` always contains a valid string (default-constructed to empty).
- Constructing from `kota::error` extracts the error message without losing information.

#### Key Members

- `std::string message`
- `LLMError(std::string msg)`
- `LLMError(kota::error err)`

#### Usage Patterns

- Returned from functions to indicate an LLM-related error.
- Captures errors from the `kota` library for uniform error handling.
- Instantiated with a descriptive string for custom error cases.

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

The struct `clore::net::detail::EnvironmentConfig` is an internal aggregation type that bundles two fundamental configuration parameters for network requests. It contains two `std::string` members: `api_base`, which stores the base URL endpoint, and `api_key`, which holds the authentication credential. The struct has no user‑defined constructors, destructors, or assignment `operator`s, relying on default compiler‑generated implementations. The invariants are minimal — both strings may be empty or non‑empty; no validation or trimming is performed within the struct itself. Its primary role is to serve as a lightweight, cohesive data holder that simplifies passing environment‑specific settings through the networking layer, enabling easy construction and copying of configuration sets.

#### Invariants

- No invariants enforced; callers must ensure `api_base` is a valid URL and `api_key` is non-empty if required.

#### Key Members

- `api_base`
- `api_key`

#### Usage Patterns

- Passed to constructors or initialization functions of network-related classes.

### `clore::net::detail::RawHttpResponse`

Declaration: `src/network/http.cppm:56`

Definition: `src/network/http.cppm:56`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The `clore::net::detail::RawHttpResponse` struct is a simple aggregate that stores the raw components of an HTTP response: an integer `http_status` (defaulting to `0`) and a `std::string body`. As a `detail` type, it serves as an internal data carrier for low-level HTTP processing, where `http_status` holds the parsed numeric status code and `body` holds the response payload as received from the network. No invariants are enforced by the struct itself; instead, its fields are populated by the surrounding parsing logic, which ensures that `http_status` is set to a valid HTTP status code (e.g., `200`, `404`, `500`) after a successful parse, and that `body` contains the raw response data. The default value of `0` for `http_status` acts as a sentinel, indicating that no response has been processed yet.

#### Invariants

- `http_status` defaults to 0, which may represent an uninitialized or error state
- `body` is an empty string by default

#### Key Members

- `http_status` (the HTTP status code)
- `body` (the response body)

#### Usage Patterns

- Used as a return type for functions that fetch raw HTTP responses
- Consumed by code within the `clore::net` namespace

## Variables

### `clore::net::detail::g_llm_request_counter`

Declaration: `src/network/http.cppm:111`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This variable serves as a monotonically increasing identifier for LLM requests. It is read to generate unique request numbers, likely within HTTP request processing logic.

#### Mutation Sources

- LLM request initialization

#### Usage Patterns

- read to assign request number

### `clore::net::detail::g_llm_semaphore`

Declaration: `src/network/http.cppm:62`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The semaphore is used in conjunction with `clore::net::detail::g_llm_semaphore_mutex` to synchronize access. Its lifetime is managed by `clore::net::initialize_llm_rate_limit` and `clore::net::shutdown_llm_rate_limit`, which respectively create and reset the semaphore. The variable is consumed by code that acquires and releases the semaphore to limit concurrent LLM operations.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- rate limiting LLM requests
- semaphore acquire/release via guard
- synchronized with mutex

### `clore::net::detail::g_llm_semaphore_mutex`

Declaration: `src/network/http.cppm:61`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This mutex guards the shared `clore::net::detail::g_llm_semaphore` object, ensuring thread-safe operations when acquiring, releasing, or reinitializing the semaphore in functions like `clore::net::initialize_llm_rate_limit`, `clore::net::shutdown_llm_rate_limit`, and `clore::net::detail::(anonymous namespace)::current_llm_semaphore`.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- locked and unlocked to protect access to `g_llm_semaphore`

### `clore::net::detail::kConnMaxAgeSec`

Declaration: `src/network/http.cppm:116`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

Used in `clore::net::detail::configure_request` to set the connection max age parameter, controlling how long a connection can be reused before being closed.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- used in `configure_request` to set connection lifetime

### `clore::net::detail::kDnsCacheTimeoutSec`

Declaration: `src/network/http.cppm:115`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This constant is used in the `clore::net::detail::configure_request` function, likely to set a timeout for DNS cache entries, controlling how long resolved DNS addresses are reused before fresh resolution is required.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- referenced in `clore::net::detail::configure_request`

### `clore::net::detail::kHttpConnectTimeoutMs`

Declaration: `src/network/http.cppm:113`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This constant specifies the timeout duration for establishing an HTTP connection. It is used in the function `clore::net::detail::configure_request` to set the connect timeout value on a `kota::http::request` object, ensuring that connection attempts are bounded by this limit.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- passed as timeout argument in `configure_request`

### `clore::net::detail::kHttpRequestTimeout`

Declaration: `src/network/http.cppm:114`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This constant establishes the maximum time allowed for an HTTP request to complete before it is considered failed. It is read when constructing HTTP request objects to set the timeout parameter, ensuring uniform timeout behavior across all requests. The 2-minute value is chosen to accommodate typical network latency and server response times.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Read as the default timeout for HTTP requests
- Used in network request configuration

### `clore::net::detail::kTcpKeepIdleSec`

Declaration: `src/network/http.cppm:117`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This constant specifies the idle time in seconds before a TCP keep-alive probe is sent. It is used in `clore::net::detail::configure_request` to configure the keep-alive idle interval for HTTP requests.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Referenced as `kTcpKeepIdleSec` in `configure_request` to set TCP keep-alive idle seconds.

### `clore::net::detail::kTcpKeepIntvlSec`

Declaration: `src/network/http.cppm:118`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This constant specifies the TCP keep-alive interval in seconds. It is used in the `configure_request` function to set the keep-alive interval on HTTP requests, ensuring connections remain open during idle periods.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- used as a timeout parameter in `clore::net::detail::configure_request`

## Functions

### `clore::net::detail::configure_request`

Declaration: `src/network/http.cppm:164`

Definition: `src/network/http.cppm:164`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function iterates over the provided `headers` span, calling `request.header(header.name, header.value)` for each entry. It then moves the `request_json` string into the request body via `request.body(std::move(request_json))`. Following that, it configures several libcurl (CURL) options on the request: it sets the connection timeout in milliseconds using `kHttpConnectTimeoutMs`, disables signal handling with `CURLOPT_NOSIGNAL`, enables TCP keepalive and sets the idle and interval seconds using `kTcpKeepIdleSec` and `kTcpKeepIntvlSec`, configures the DNS cache timeout with `kDnsCacheTimeoutSec`, and sets the maximum connection age with `kConnMaxAgeSec`. All these constants are defined elsewhere in the `clore::net::detail` namespace. The function thus centralizes the common HTTP request setup for headers, body, and low-level transport tuning, and is called from other detail functions before dispatching the request.

#### Side Effects

- Mutates the HTTP request by setting headers
- Mutates the HTTP request by setting the body
- Mutates the HTTP request by configuring `cURL` options

#### Reads From

- headers span
- `request_json` string
- header`.name` and header`.value`
- `kHttpConnectTimeoutMs`, `kTcpKeepIdleSec`, `kTcpKeepIntvlSec`, `kDnsCacheTimeoutSec`, `kConnMaxAgeSec` constants

#### Writes To

- `kota::http::request` passed as reference (header, body, `curl_option` methods)

#### Usage Patterns

- Called when preparing an HTTP request before transmission
- Used to uniformly apply headers and `cURL` options to a request

### `clore::net::detail::perform_http_request`

Declaration: `src/network/http.cppm:67`

Definition: `src/network/http.cppm:181`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::perform_http_request` acts as a synchronous wrapper around the asynchronous `clore::net::detail::perform_http_request_async`. It begins by creating a local `async::event_loop` and copying the provided `headers` span into a `std::vector<kota::http::header>` (named `headers_vec`). The two string parameters (`url` and `request_json`) are also copied into `std::string` objects to guarantee ownership. It then calls `perform_http_request_async` with these copies and the loop, obtaining an operation object whose cancellation is caught via `.catch_cancel()`. The operation is scheduled on the loop via `loop.schedule(operation)`, and the loop is run to completion with `loop.run()`, which blocks the calling thread.

After the loop finishes, the function inspects the operation’s result. If the result indicates cancellation, it returns `std::unexpected(LLMError(...))` with a message including the URL. If the result contains an error (a `kota::error`), it moves that error into a `LLMError` and returns it as an unexpected. Otherwise, it moves the contained `RawHttpResponse` out of the result and returns it as an expected value. This design cleanly bridges the asynchronous machinery to a synchronous interface, leveraging `std::expected` for error reporting and relying on the thread‑safe `async::event_loop` and the underlying HTTP client managed by `perform_http_request_async`.

#### Side Effects

- Performs an HTTP request (I/O) via the event loop and async operation
- Copies the input headers into a newly allocated vector
- Creates and runs a local event loop

#### Reads From

- `url` parameter
- `headers` parameter (span of headers)
- `request_json` parameter
- Result of the async operation (`operation.result()`)

#### Writes To

- Local `headers_vec` (vector copy of input headers)
- Local `loop` and `operation` state
- Returned `std::expected<RawHttpResponse, LLMError>` value

#### Usage Patterns

- Used to perform a synchronous HTTP request where the caller does not want to manage an event loop
- Likely called by higher-level synchronous wrappers or in contexts where async is not required
- Serves as a bridge between async internals and synchronous interfaces

### `clore::net::detail::perform_http_request_async`

Declaration: `src/network/http.cppm:72`

Definition: `src/network/http.cppm:209`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The implementation of `clore::net::detail::perform_http_request_async` is a C++20 coroutine that orchestrates an asynchronous HTTP POST request to an LLM endpoint while enforcing global rate limiting. It first acquires a shared semaphore via `current_llm_semaphore`; if a semaphore is active, it awaits acquisition to throttle requests. A local `SemaphoreGuard` is constructed to ensure the semaphore is released on any exit path—normal completion, cancellation, or error. The function then increments the global request counter `g_llm_request_counter` for logging correlation.  

Using the thread‑local HTTP client returned by `get_thread_http_client`, it creates a POST request on the provided `async::event_loop`, sets a timeout via `kHttpRequestTimeout`, and configures headers and body via `configure_request`. The request is sent and the response is awaited with `catch_cancel()` to intercept cancellation. On cancellation or transport error, the guard releases the semaphore and the coroutine fails with an `LLMError` containing a descriptive message. On success, a `RawHttpResponse` is constructed from the HTTP status and response body, the guard releases the semaphore, and the response is `co_returned`.

#### Side Effects

- Network I/O via HTTP client
- Semaphore acquisition and release (synchronization)
- Logging via `logging::info` and `logging::warn`
- Incrementing global request counter `g_llm_request_counter`

#### Reads From

- Global thread-local HTTP client (`get_thread_http_client`)
- Global semaphore (`current_llm_semaphore`)
- Global request counter `g_llm_request_counter`
- Parameters: `url`, `headers`, `request_json`, `loop`
- HTTP request configuration (`configure_request`)

#### Writes To

- Semaphore state (acquire/release)
- Global request counter `g_llm_request_counter`
- Log output
- Network socket (via HTTP client)
- Output task result (`RawHttpResponse`)

#### Usage Patterns

- Called to perform an LLM HTTP request with concurrency control
- Used in async workflows expecting a task with `RawHttpResponse`
- Usually invoked from other coroutines that handle the response or error

### `clore::net::detail::read_environment`

Declaration: `src/network/http.cppm:64`

Definition: `src/network/http.cppm:146`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The implementation of `clore::net::detail::read_environment` is a straightforward, synchronous factory that assembles an `EnvironmentConfig` from two required environment variable lookups. It first calls `read_required_env(base_env)` and checks the result; if that fails, the error (of type `LLMError`) is moved into a `std::unexpected` and returned immediately. Only after the first lookup succeeds does it invoke `read_required_env(key_env)` with the same error‑checking pattern. If both lookups succeed, it moves the inner values into the `api_base` and `api_key` fields of a newly constructed `EnvironmentConfig` and returns that object wrapped in a `std::expected`. The function performs no I/O itself and relies entirely on `read_required_env` to interpret environment variables and produce either a string or a `LLMError`. No asynchronous operations, rate limiting, or HTTP interactions occur within this function.

#### Side Effects

- reads environment variables via `read_required_env`

#### Reads From

- environment variables identified by `base_env` and `key_env`

#### Usage Patterns

- obtain API base URL and API key from environment variables

### `clore::net::detail::read_required_env`

Declaration: `src/network/http.cppm:137`

Definition: `src/network/http.cppm:137`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::read_required_env` converts its `std::string_view` parameter `name` to a null-terminated C-string via `std::string` and passes it to `std::getenv`. If the returned pointer is null or points to an empty string (first character is `'\0'`), it constructs an `LLMError` with a `std::format` message indicating that the required environment variable is not set, then returns that error wrapped in `std::unexpected`. Otherwise, the raw C-string is copied into a `std::string` and returned as a success value inside `std::expected<std::string, LLMError>`. The function relies solely on the C runtime `std::getenv`, the C++ standard library’s format and string utilities, and the custom `LLMError` type; no other network or concurrency components are involved.

#### Side Effects

- Reads environment variable via `std::getenv`
- Allocates memory for the returned string and error message

#### Reads From

- Environment variable named by `name`

#### Usage Patterns

- Used to retrieve mandatory environment variables for configuration
- Called by `read_environment` and other setup functions to obtain required settings

### `clore::net::detail::unwrap_caught_result`

Declaration: `src/network/http.cppm:78`

Definition: `src/network/http.cppm:78`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `unwrap_caught_result` first inspects the provided `result` for cancellation via `result.is_cancelled()`. If cancelled, it triggers a coroutine failure by invoking `kota::fail` with a `LLMError` constructed from the `cancel_message` string. Next, it checks `result.has_error()`. On error, it moves the underlying error out of `result` and passes it to `kota::fail`, propagating the original error type. If neither condition holds, the function unwraps the successful value using `co_return std::move(*result)`. Control flow is linear: each condition is tested sequentially, and the first match causes early exit via `co_await`. Dependencies include `LLMError` for error representation, `kota::fail` for suspending with failure, and the duck-typed interface of `R` (requiring `is_cancelled()`, `has_error()`, dereference, and move semantics).

#### Side Effects

- May allocate dynamic memory for constructing `LLMError` strings
- Moves the result value out of the parameter, potentially transferring ownership

#### Reads From

- `result` (parameter of type `R`)
- `cancel_message` (parameter of type `std::string_view`)

#### Usage Patterns

- Propagate cancellation or error from a cancellable async result
- Used internally in async HTTP request functions like `perform_http_request_async`

### `clore::net::initialize_llm_rate_limit`

Declaration: `src/network/http.cppm:33`

Definition: `src/network/http.cppm:93`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function acquires a mutex lock on `detail::g_llm_semaphore_mutex` via `std::lock_guard` to ensure thread‑safe access to the global shared pointer `detail::g_llm_semaphore`. It then checks the `rate_limit` parameter: if zero, the semaphore is reset to `nullptr`, effectively disabling rate limiting; otherwise, a new `kota::semaphore` is constructed with the specified limit (cast to `std::ptrdiff_t`) and assigned to the shared pointer. This implementation relies exclusively on the internal synchronization primitive and the `kota::semaphore` type; no network or asynchronous calls are involved.

#### Side Effects

- Acquires and releases `detail::g_llm_semaphore_mutex`
- Modifies `detail::g_llm_semaphore` global shared pointer

#### Reads From

- `rate_limit` parameter
- `detail::g_llm_semaphore_mutex` global mutex (lock acquisition)

#### Writes To

- `detail::g_llm_semaphore` global shared pointer

#### Usage Patterns

- Called to set or update the LLM rate limit
- Used during initialization or runtime reconfiguration
- Called before making LLM requests to control concurrency

### `clore::net::shutdown_llm_rate_limit`

Declaration: `src/network/http.cppm:35`

Definition: `src/network/http.cppm:277`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function acquires exclusive ownership of the global mutex `detail::g_llm_semaphore_mutex` via a `std::lock_guard`, ensuring thread safety. After the lock is held, it calls `reset()` on `detail::g_llm_semaphore`, which destroys the managed semaphore object (a `std::shared_ptr<kota::semaphore>`) and releases any associated resources. This simple two‑step control flow cleanly deinitializes the concurrency limiter used by LLM HTTP requests, preventing further rate‑limit enforcement after shut‑down.

#### Side Effects

- mutex lock and unlock via `std::lock_guard`
- reset of `detail::g_llm_semaphore`

#### Reads From

- `detail::g_llm_semaphore_mutex`

#### Writes To

- `detail::g_llm_semaphore`

#### Usage Patterns

- called during system shutdown to clean up rate limiter
- called before reinitializing rate limit parameters

## Internal Structure

The `http` module is organized as a C++ module file (`http.cppm`) that imports the `support` module for foundational utilities. Its public interface resides in the `clore::net` namespace, offering the `LLMError` type and two rate‑limiting lifecycle functions (`initialize_llm_rate_limit`, `shutdown_llm_rate_limit`). The bulk of the implementation is placed in the `clore::net::detail` namespace, which contains internal structures (`EnvironmentConfig`, `RawHttpResponse`, `SemaphoreGuard`) and core HTTP request primitives. The module provides both synchronous and asynchronous request paths: `perform_http_request` returns a `std::expected<RawHttpResponse, LLMError>`, while `perform_http_request_async` uses an `async::event_loop` and guards concurrency with a semaphore-based rate limiter. Global state (`g_llm_semaphore`, `g_llm_semaphore_mutex`, `g_llm_request_counter`) enforces a configurable rate limit across all LLM requests. A set of compile‑time constants (`kHttpConnectTimeoutMs`, `kHttpRequestTimeout`, `kConnMaxAgeSec`, `kDnsCacheTimeoutSec`, `kTcpKeepIdleSec`, `kTcpKeepIntvlSec`) control connection timeouts, DNS caching, and TCP keepalive behaviour. The module relies on the `kota` library for HTTP client and semaphore primitives, and on the `support` module for utility functions such as logging, caching, and string handling. This layering cleanly separates the public API from internal networking details, enabling independent evolution of the rate‑limiting logic, request configuration, and transport configuration.

## Related Pages

- [Module support](../support/index.md)

