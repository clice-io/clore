---
title: 'Module http'
description: 'The http module provides the core networking layer for interacting with LLM APIs. It owns responsibilities ranging from environment‑based configuration discovery to building, dispatching, and processing HTTP requests. Public‑facing entities include the perform_http_request function for synchronous calls, perform_http_request_async for asynchronous invocations driven by an event loop, and the read_environment and read_required_env helpers that parse API credentials and base URLs from system variables. Error handling is consolidated in the LLMError type, while rate‑limiting controls are exposed through initialize_llm_rate_limit and shutdown_llm_rate_limit, backed by a global semaphore and a request counter.'
layout: doc
template: doc
---

# Module `http`

## Summary

The `http` module provides the core networking layer for interacting with LLM `APIs`. It owns responsibilities ranging from environment‑based configuration discovery to building, dispatching, and processing HTTP requests. Public‑facing entities include the `perform_http_request` function for synchronous calls, `perform_http_request_async` for asynchronous invocations driven by an event loop, and the `read_environment` and `read_required_env` helpers that parse API credentials and base `URLs` from system variables. Error handling is consolidated in the `LLMError` type, while rate‑limiting controls are exposed through `initialize_llm_rate_limit` and `shutdown_llm_rate_limit`, backed by a global semaphore and a request counter.

Internal details such as the `EnvironmentConfig` and `RawHttpResponse` structs, timeout constants, the request configuration helper `configure_request`, and the semaphore guard are part of the implementation scope. Together, these elements form a complete, self‑contained HTTP client tailored for LLM API integration, with support for synchronous and asynchronous workflows, request formatting, and concurrency throttling.

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

The struct stores a single `std::string` member `message` as its internal payload. Invariants: `message` is always a valid `std::string` (default-constructed empty or move-constructed from the argument). The default constructor leaves `message` empty. The explicit constructor from `std::string` moves the argument into `message` to avoid copying. The explicit constructor from `kota::error` extracts the error description by calling `err.message()` and stores the result, effectively wrapping a `kota::error` into a string-based error representation. No additional state or validation logic is present; the struct serves solely as a lightweight wrapper around a single error message string.

#### Invariants

- `message` holds a human-readable error description
- Default-constructed `LLMError` has an empty `message`
- Construction from `kota::error` copies the error's message

#### Key Members

- `std::string message`
- `LLMError()` default constructor
- `explicit LLMError(std::string msg)`
- `explicit LLMError(kota::error err)`

#### Usage Patterns

- Returned or thrown as an error type for LLM-related failures
- Constructed from a `kota::error` or an explicit message string

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

The `clore::net::detail::EnvironmentConfig` struct is a Plain-Old-Data aggregate that serves as an internal container for storing two configuration strings: `api_base` (the base URL for API requests) and `api_key` (the authentication key). Both members are `std::string` objects with no special invariants beyond typical string validity; the struct itself carries no member functions or additional logic, acting solely as a lightweight data carrier for environment settings used deeper in the networking layer. Its primary role is to bundle these two parameters together for convenient, consistent passing through internal interfaces.

#### Invariants

- `api_base` and `api_key` may be empty or contain configuration values

#### Key Members

- `api_base`
- `api_key`

#### Usage Patterns

- Passed to network client constructors or initialization functions
- Populated from environment variables or configuration files

### `clore::net::detail::RawHttpResponse`

Declaration: `network/http.cppm:42`

Definition: `network/http.cppm:42`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The struct `clore::net::detail::RawHttpResponse` is a plain aggregate that serves as a lightweight holder for the two essential parts of a raw HTTP response: the status code and the body content. Its only data members are `http_status` (a `long` defaulting to `0`) and `body` (a `std::string` defaulting to empty). No user‑defined constructors, assignment `operator`s, or destructors are declared; the compiler‑generated special member functions perform shallow copies and trivial destruction, which is sufficient for this internal struct. The key invariant is that `http_status` is expected to hold an integral HTTP status code (e.g., 200, 404) after parsing, while `body` contains the raw response payload. Because the struct is defined in a `detail` namespace, it is not part of the public API; its purpose is to be used internally by HTTP parsing machinery to transport the parsed result without additional processing or validation logic.

#### Invariants

- `http_status` is expected to hold a valid HTTP status code, though no validation is performed.
- `body` may be empty, representing an absent response body.

#### Key Members

- `http_status`
- `body`

#### Usage Patterns

- Constructed after parsing an HTTP response to hold the raw status and body.
- Used internally within the `clore::net` library, likely passed to higher-level response wrappers.

## Variables

### `clore::net::detail::g_llm_request_counter`

Declaration: `network/http.cppm:94`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

It is referenced by the function `clore::net::detail::perform_http_request_async`, suggesting it tracks the number of HTTP requests made, possibly for rate limiting or logging.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Referenced in `clore::net::detail::perform_http_request_async`

### `clore::net::detail::g_llm_semaphore`

Declaration: `network/http.cppm:47`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This semaphore controls the maximum number of simultaneous LLM HTTP requests. It is initialized by `clore::net::initialize_llm_rate_limit` and shut down by `clore::net::shutdown_llm_rate_limit`. Inside `clore::net::detail::perform_http_request_async`, the semaphore is likely acquired before sending a request and released after completion.

#### Mutation Sources

- `clore::net::initialize_llm_rate_limit`
- `clore::net::shutdown_llm_rate_limit`

#### Usage Patterns

- Acquired before performing an LLM HTTP request in `perform_http_request_async`
- Released after the request completes

### `clore::net::detail::kHttpConnectTimeoutMs`

Declaration: `network/http.cppm:96`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

Used in `configure_request` to set the connect timeout on outgoing HTTP connections, ensuring requests are not left hanging if the server does not respond within the specified period.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Referenced in `configure_request` to set connect timeout on HTTP requests

### `clore::net::detail::kHttpRequestTimeout`

Declaration: `network/http.cppm:97`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

Used to set the timeout on HTTP requests made by the networking layer. Likely passed to underlying HTTP client functions to control how long to wait for a response.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- HTTP request timeout configuration
- passed to HTTP client functions

## Functions

### `clore::net::detail::configure_request`

Declaration: `network/http.cppm:126`

Definition: `network/http.cppm:126`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function accepts a `kota::http::request` and a span of `kota::http::header` objects along with the request body as a `std::string`. It iterates over each header in the provided span, calling `request.header(header.name, header.value)` to attach them. Afterwards, it assigns the serialised `request_json` to the request body via `request.body()`. Several curl options are configured: `CURLOPT_CONNECTTIMEOUT_MS` is set to the constant `kHttpConnectTimeoutMs`, `CURLOPT_NOSIGNAL` is set to `1L` to avoid signal-based timeouts, and `CURLOPT_TCP_KEEPALIVE` is enabled with `1L`. No branching or error handling occurs; the function is purely a mutating preparation step for the HTTP request object.

#### Side Effects

- mutates the `request` object by adding headers, setting its body, and modifying curl options

#### Reads From

- `headers` parameter (span of headers)
- `request_json` parameter (string moved into request)

#### Writes To

- `request` parameter (mutable reference to `kota::http::request`)

#### Usage Patterns

- called as part of HTTP request construction pipeline
- used in `perform_http_request` or similar functions to finalize request setup

### `clore::net::detail::perform_http_request`

Declaration: `network/http.cppm:52`

Definition: `network/http.cppm:139`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function acts as a synchronous wrapper around the internal `perform_http_request_async` coroutine. It creates a local `async::event_loop` instance, copies the caller's `url`, `headers`, and `request_json` into owning containers required by the async variant, and then schedules the resulting operation on the loop. After running the loop to completion, it inspects the operation's outcome: if the operation was cancelled, it returns an `LLMError` with a cancellation message; if an error occurred, it moves the error into an unexpected result; otherwise, it moves the successful `RawHttpResponse` into an expected value. This function depends on the async infrastructure (`async::event_loop`, `perform_http_request_async`) and on `LLMError` for error reporting.

#### Side Effects

- performs an HTTP request via `perform_http_request_async`
- creates and runs an event loop
- allocates and copies headers and request string
- returns a result or error

#### Reads From

- url parameter
- headers parameter (span of headers)
- `request_json` parameter
- result of async operation

#### Writes To

- event loop (schedule, run)
- result variable (moved from operation)
- returned expected value

#### Usage Patterns

- synchronous wrapper over async HTTP request
- used to perform HTTP requests with async internal implementation

### `clore::net::detail::perform_http_request_async`

Declaration: `network/http.cppm:57`

Definition: `network/http.cppm:165`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::perform_http_request_async` is a coroutine that returns `async::task<RawHttpResponse, LLMError>`, performing an asynchronous HTTP POST request to an LLM endpoint. Upon entry, it checks the global semaphore `g_llm_semaphore`; if active, it co-awaits an acquire before creating a `SemaphoreGuard` that will release the semaphore on scope exit. It then fetches and logs a monotonically increasing request number from `g_llm_request_counter` along with the target `url`.

A `kota::http::client` is configured with `kHttpRequestTimeout` and a POST request is built on the provided `async::event_loop`. The helper `configure_request` attaches the given `headers` and `request_json` body. After sending the request via `co_await request.send().catch_cancel()`, the function handles three outcomes: cancellation produces an `LLMError` with a cancellation message; a transport error produces an `LLMError` with the error's message; on success, a `RawHttpResponse` is constructed from the response status and body text, logged, and co-returned. The control flow relies on `async::task`, the global semaphore for rate limiting, and the `kota::http` networking primitives.

#### Side Effects

- Acquires global semaphore `g_llm_semaphore` if present
- Releases global semaphore `g_llm_semaphore` on return
- Atomically increments global counter `g_llm_request_counter`
- Sends an HTTP POST request over the network
- Logs request and response information via `logging::info` and `logging::warn`

#### Reads From

- Global semaphore `g_llm_semaphore`
- Global counter `g_llm_request_counter`
- Parameter `url`
- Parameter `headers`
- Parameter `request_json`
- Parameter `loop` (event loop reference)
- Timeout constant `kHttpRequestTimeout`

#### Writes To

- Global counter `g_llm_request_counter` (incremented via `fetch_add`)
- Local variables `client`, `request`, `response`, `raw_response`
- Logging output

#### Usage Patterns

- Called as part of an `async::task` coroutine in the LLM request pipeline
- Used by higher-level async LLM functions to perform the actual HTTP call

### `clore::net::detail::read_environment`

Declaration: `network/http.cppm:49`

Definition: `network/http.cppm:108`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The implementation of `clore::net::detail::read_environment` follows a straightforward two-step validation pattern. It first calls `read_required_env` with the `base_env` parameter to obtain the API base URL, and then calls `read_required_env` again with the `key_env` parameter to obtain the API key. Each call is checked immediately: if the result does not have a value, the function returns `std::unexpected` containing the moved `LLMError` from the failed result. Only when both environment variables are successfully retrieved does the function construct and return an `EnvironmentConfig` by moving the extracted `api_base` and `api_key` values into its designated initializer fields.

The function depends entirely on `read_required_env` for environment‑variable lookup and error handling, and on `EnvironmentConfig` as the output type. There is no branching beyond the two guard clauses, no asynchronous operations, and no direct interaction with HTTP requests or rate‑limiting constructs—those concerns are handled by callers or by deeper layers invoked later in the pipeline.

#### Side Effects

- Reads process environment variables

#### Reads From

- process environment variables named by `base_env` and `key_env`
- `read_required_env` function

#### Writes To

- returned `EnvironmentConfig` object (moved members)
- error state via `std::unexpected`

#### Usage Patterns

- Called during network stack initialization to retrieve API credentials from environment

### `clore::net::detail::read_required_env`

Declaration: `network/http.cppm:99`

Definition: `network/http.cppm:99`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::read_required_env` first converts the input `std::string_view name` to a null‑terminated C string via `std::string`. It then calls `std::getenv` to retrieve the environment variable value. If the returned pointer is `nullptr` or points to an empty string (first character `\0`), the function returns `std::unexpected` containing a `clore::net::LLMError` constructed with a formatted message indicating which variable is missing. Otherwise, it returns a `std::expected` success containing a `std::string` copy of the value. This function depends on the standard library environment query (`std::getenv`) and the custom error type (`clore::net::LLMError`) for error reporting. It does not perform any network calls or involve other module components beyond string formatting.

#### Side Effects

- reads environment variable via `std::getenv`
- allocates heap memory for the returned string and error message

#### Reads From

- parameter `name`
- process environment (via `std::getenv`)

#### Writes To

- return value (string or `LLMError`)
- heap memory for string and error message

#### Usage Patterns

- reading required configuration from environment variables
- validating presence of mandatory environment variables

### `clore::net::detail::unwrap_caught_result`

Declaration: `network/http.cppm:63`

Definition: `network/http.cppm:63`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function checks for cancellation first by calling `R::is_cancelled()` on the incoming `result`. If the result was cancelled, it `co_awaits` `kota::fail(LLMError(...))` using the provided `cancel_message` to construct the error. If the result is not cancelled, it then checks `R::has_error()`. When an error is present, the function `co_awaits` `kota::fail(...)` with the error moved from `result`. Finally, if neither condition holds, it `co_returns` the value extracted from the result via `std::move(*result)`. The function relies on `kota::task` and `kota::fail` from the coroutine library, and on `LLMError` as the error type for failure signals. The control flow is strictly sequential: cancellation takes precedence over error reporting, and a valid value is only produced when neither condition is met.

#### Side Effects

- May cancel or fail the coroutine task via `co_await` `kota::fail`

#### Reads From

- result parameter (via `is_cancelled()`, `has_error()`, and dereference)
- `cancel_message` parameter

#### Writes To

- coroutine task result (via `co_return` or failure)
- result (moves error out)

#### Usage Patterns

- Used to convert a caught result into a coroutine task that either yields the value or fails
- Called in async contexts where a result may be cancelled or erroneous

### `clore::net::initialize_llm_rate_limit`

Declaration: `network/http.cppm:19`

Definition: `network/http.cppm:78`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function `clore::net::initialize_llm_rate_limit` modifies the module‑global rate‑limiting semaphore `detail::g_llm_semaphore`.  If the provided `rate_limit` is zero, the semaphore is reset (set to `nullptr`), effectively disabling rate limiting for subsequent requests.  Otherwise, a new `kota::semaphore` is constructed with a capacity equal to the given `rate_limit` (cast to `std::ptrdiff_t`).  This semaphore is later acquired via a `SemaphoreGuard` inside `detail::perform_http_request_async` to control concurrency of LLM API calls.  No other dependencies are invoked, and no error handling is performed; the function simply replaces or clears the shared synchronization primitive.

#### Side Effects

- Resets or creates the global LLM semaphore `detail::g_llm_semaphore`.

#### Reads From

- `rate_limit` parameter

#### Writes To

- `detail::g_llm_semaphore` global variable

#### Usage Patterns

- Called during initialization to set the LLM rate limit.
- Can be called with zero to disable rate limiting.

### `clore::net::shutdown_llm_rate_limit`

Declaration: `network/http.cppm:21`

Definition: `network/http.cppm:223`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The implementation of `clore::net::shutdown_llm_rate_limit` is a single direct invocation of `detail::g_llm_semaphore.reset()`. This call immediately clears the internal state of the global counting semaphore `clore::net::detail::g_llm_semaphore`, releasing any threads currently blocked on the semaphore and disabling future rate‑limiting enforcement until the semaphore is re‑initialized (e.g., via `clore::net::initialize_llm_rate_limit`). No error handling or external dependencies beyond the semaphore object are involved; the function is marked `noexcept` and simply resets the synchronization primitive.

#### Side Effects

- Reset of the global semaphore `detail::g_llm_semaphore`

#### Reads From

- global variable `detail::g_llm_semaphore`

#### Writes To

- global variable `detail::g_llm_semaphore`

#### Usage Patterns

- Called during shutdown or reinitialization of LLM rate limiting

## Internal Structure

The `http` module, a component of the `clore::net` namespace, is decomposed into a minimal public API and a more extensive internal implementation. The public side provides `initialize_llm_rate_limit` and `shutdown_llm_rate_limit` to manage a global rate‑limiting subsystem, plus the `LLMError` type for uniform error propagation. Everything else lives in the `clore::net::detail` namespace, which encapsulates environment configuration (`EnvironmentConfig`), raw HTTP response handling (`RawHttpResponse`), request preparation (`configure_request`), and both synchronous (`perform_http_request`) and asynchronous (`perform_http_request_async`) HTTP execution. The module imports `std` for standard types and the `support` module for UTF‑8 text processing, file I/O, and structured logging. Under the hood, a global semaphore and an atomic request counter enforce concurrency limits, while constants like `kHttpRequestTimeout` and `kHttpConnectTimeoutMs` define default timeouts. This layering keeps the public surface focused and allows the internal helpers to evolve independently, all built on the foundational utilities provided by the `support` module.

## Related Pages

- [Module support](../support/index.md)

