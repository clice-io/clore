---
title: 'Module network'
description: 'The network module provides a portable abstraction for sending asynchronous requests to large language model (LLM) providers, currently supporting OpenAI and Anthropic. Its public interface consists of three primary functions: call_llm_async and call_completion_async for initiating asynchronous LLM and generic completion requests on a kota::event_loop, and validate_llm_provider_environment for checking that necessary environment variables (such as API keys and base URLs) are properly configured before making any network calls.'
layout: doc
template: doc
---

# Module `network`

## Summary

The `network` module provides a portable abstraction for sending asynchronous requests to large language model (LLM) providers, currently supporting `OpenAI` and Anthropic. Its public interface consists of three primary functions: `call_llm_async` and `call_completion_async` for initiating asynchronous LLM and generic completion requests on a `kota::event_loop`, and `validate_llm_provider_environment` for checking that necessary environment variables (such as API keys and base `URLs`) are properly configured before making any network calls.

Beneath this surface, the module owns the logic for detecting and selecting an LLM provider from environment settings, dispatching completion events, and furnishing human-readable labels for each supported provider. Internal enumerations (`Provider`) and helper functions manage provider-specific configuration and environment checks, while the public functions return integer handles or status codes that allow callers to track operations and verify readiness.

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

The function first determines the appropriate LLM provider by calling `detect_provider_from_environment()`, which returns an expected `Provider` value. If this detection fails (i.e., no compatible environment variables are set), it immediately `co_awaits` `kota::fail` with the contained error, propagating the failure as a `kota::task<CompletionResponse, LLMError>`. Otherwise, it moves the resolved `Provider` into a local variable and invokes `dispatch_completion` with that provider, the forwarded `CompletionRequest`, and the given `kota::event_loop`. The resulting coroutine is chained with `.catch_cancel()` to intercept cancellation, and the final result is unwrapped via `detail::unwrap_caught_result`, which maps a cancellation into a descriptive `LLMError`.

Internally, control flow is entirely coroutine‑based, depending on the `kota::task` framework for asynchronous execution, error propagation, and cancellation handling. The key dependencies are the anonymous‑namespace helpers `detect_provider_from_environment`, `dispatch_completion`, and the utility `unwrap_caught_result`, which together encapsulate provider selection and the actual network request logic.

#### Side Effects

- Reads environment variables via `detect_provider_from_environment`
- Moves the `request` parameter, transferring ownership
- Initiates asynchronous network I/O via `dispatch_completion`
- May modify internal state of the event loop

#### Reads From

- `request` parameter (moved from)
- `loop` parameter
- Environment variables (implied by `detect_provider_from_environment`)
- Possibly global or static state in `dispatch_completion`

#### Writes To

- Network output (send request)
- Event loop internal state
- Ownership transfer of `request` (move)

#### Usage Patterns

- Called to asynchronously perform an LLM completion
- Used in coroutine contexts with `co_await`
- Typically invoked when a user issues a completion request

### `clore::net::call_llm_async`

Declaration: `network/network.cppm:18`

Definition: `network/network.cppm:126`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The implementation of `clore::net::call_llm_async` begins by invoking the anonymous‑namespace helper `detect_provider_from_environment` to determine which LLM provider (e.g., `Provider::OpenAI` or `Provider::Anthropic`) should handle the request, based on the presence and non‑emptiness of environment variables such as `kAnthropicApiKeyEnv` or `kOpenAIApiKeyEnv`. If detection fails, the function immediately fails the coroutine via `kota::fail`. On success, it calls `provider_label` to obtain a human‑readable label for the provider, then passes control to `request_provider_text_async`.

The central control flow delegates the actual work to `request_provider_text_async`, which is given a lambda that, for each completion request, calls `dispatch_completion` with the detected `provider`, the request data, and the event loop. The lambda uses `or_fail()` to convert the result of `dispatch_completion` into the expected error type. The outer call to `request_provider_text_async` is also chained with `.or_fail()`, ensuring that any error from the provider‑specific dispatch propagates upward as the final result of `call_llm_async`. Key internal dependencies include the environment‑detection logic (`detect_provider_from_environment`, `has_nonempty_env`, `has_provider_env`) and the provider‑specific dispatch function `dispatch_completion`, which ultimately selects the correct HTTP endpoint and serialization logic based on the `Provider` enum member.

#### Side Effects

- performs asynchronous network I/O via `dispatch_completion`
- reads environment variables to detect the LLM provider
- may propagate or create `LLMError` instances on failure

#### Reads From

- environment variables via `detect_provider_from_environment`
- parameters: `model`, `system_prompt`, `request`
- the event loop reference `loop`

#### Writes To

- returns a `kota::task` whose eventual resolution writes either a `std::string` or an `LLMError`
- may write to error state via `co_await` of `kota::fail`

#### Usage Patterns

- initiate an asynchronous LLM completion from a coroutine context
- call with a `PromptRequest`, model name, system prompt, and an event loop
- use as part of a higher-level async pipeline that consumes `kota::task<std::string, LLMError>`

### `clore::net::validate_llm_provider_environment`

Declaration: `network/network.cppm:28`

Definition: `network/network.cppm:118`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The implementation of `clore::net::validate_llm_provider_environment` delegates entirely to the anonymous‑namespace helper `detect_provider_from_environment`. This inner function probes the process environment for known LLM provider credentials and base‑URL variables (such as `kAnthropicApiKeyEnv`, `kOpenAIApiKeyEnv`, `kAnthropicBaseUrlEnv`, and `kOpenAIBaseUrlEnv`) using `has_provider_env` and `has_nonempty_env`. It returns a `std::expected` over an internally defined `Provider` enum (`Provider::Anthropic` or `Provider::OpenAI`) on success, or an `LLMError` if no supported provider’s environment variables are set.

The outer function captures the `provider_result` from `detect_provider_from_environment` and, if it is not a valid value, immediately returns `std::unexpected` with the moved error. A successful detection yields an empty `std::expected<void, LLMError>`. No other control flow or dependency is present; every possible path terminates at either a successful void result or an error propagation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- environment variables (through `detect_provider_from_environment`)

#### Usage Patterns

- early validation before LLM operations
- ensuring provider configuration exists

## Internal Structure

The `network` module provides asynchronous network abstractions for LLM interactions, importing only the C++ standard library. Its public interface consists of `call_llm_async`, `call_completion_async`, and `validate_llm_provider_environment`, each accepting a `kota::event_loop` reference to drive completion callbacks. Internally, the module is decomposed into an anonymous namespace that encapsulates provider-specific logic: an enumeration of supported providers (`Anthropic`, `OpenAI`), environment variable detection and validation functions, a provider label utility, and a generic `request_provider_text_async` template for dispatching requests. This layered design isolates provider detection and request assembly from the public API, allowing new providers to be added by extending the internal helpers without altering the external interface.

