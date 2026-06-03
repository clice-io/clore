---
title: 'Module network'
description: 'The network module under clore::net is responsible for asynchronous communication with large language model (LLM) providers, currently supporting OpenAI and Anthropic. It provides the public interface call_llm_async to schedule a model call with a system prompt and user prompt, call_completion_async to initiate a completion operation using a request handle, and validate_llm_provider_environment to check that required environment variables are present before any network activity. All asynchronous operations rely on a kota::event_loop to manage callbacks and completion delivery.'
layout: doc
template: doc
---

# Module `network`

## Summary

The `network` module under `clore::net` is responsible for asynchronous communication with large language model (LLM) providers, currently supporting `OpenAI` and Anthropic. It provides the public interface `call_llm_async` to schedule a model call with a system prompt and user prompt, `call_completion_async` to initiate a completion operation using a request handle, and `validate_llm_provider_environment` to check that required environment variables are present before any network activity. All asynchronous operations rely on a `kota::event_loop` to manage callbacks and completion delivery.

Internally, the module handles provider detection from environment variables (e.g., `kAnthropicApiKeyEnv`, `kOpenAIApiKeyEnv`), constructs provider‑specific requests, and dispatches them via internal helpers such as `detect_provider_from_environment` and `request_provider_text_async`. The validated `Provider` enum selects the appropriate base URL and API key while a helper like `dispatch_completion` bridges the async result back to the caller through the event loop. This design centralises network‑related LLM interaction logic and environment validation, keeping higher‑level application code independent of provider‑specific details.

## Imported By

- [`agent`](../agent/index.md)
- [`generate:scheduler`](../generate/scheduler.md)

## Functions

### `clore::net::call_completion_async`

Declaration: `src/network/network.cppm:31`

Definition: `src/network/network.cppm:157`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function `clore::net::call_completion_async` begins by invoking `detect_provider_from_environment` to determine which `Provider` should handle the request. If the returned result is empty (indicating an environment failure), it immediately `co_awaits` `kota::fail` with the error value. Otherwise, the provider is moved into a local variable, and the call proceeds to `dispatch_completion` with the provider, the `CompletionRequest`, and the `kota::event_loop`. The task returned by `dispatch_completion` is first processed via `catch_cancel` to intercept cancellation, then unwrapped through `detail::unwrap_caught_result`, which maps the internal `kota::result` into a `CompletionResponse` or converts the cancellation error into a descriptive `LLMError`. The final `co_return` delivers the outcome to the caller.

#### Side Effects

- Performs network I/O for LLM completion request
- May cancel the operation if cancelled
- Reads environment variables to detect provider

#### Reads From

- completion request parameter `request`
- event loop reference `loop`
- environment variables via `detect_provider_from_environment()`

#### Usage Patterns

- Await the returned task to obtain a `CompletionResponse` or `LLMError`
- Used with an event loop to schedule asynchronous LLM calls

### `clore::net::call_llm_async`

Declaration: `src/network/network.cppm:25`

Definition: `src/network/network.cppm:133`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function first calls `detect_provider_from_environment()` to determine which LLM provider (e.g., `Anthropic` or `OpenAI`) is configured. If detection fails (e.g., missing API key or base URL), the coroutine immediately fails by `co_await`-ing `kota::fail` with the returned error. Otherwise, it obtains a human-readable label for the provider via `provider_label()` and then delegates the actual request to `request_provider_text_async`. This generic function is given a lambda that, for each `CompletionRequest`, invokes `dispatch_completion` with the previously detected `provider`, the request, and the appropriate event loop, chaining `.or_fail()` to propagate any errors. The final result of the overall async operation is `co_return`-ed, completing the LLM call. The entire flow is driven by coroutines and depends on environment-based provider detection and the helper functions `detect_provider_from_environment`, `provider_label`, `request_provider_text_async`, and `dispatch_completion`; error handling is done via `kota::fail` and the `.or_fail()` extension.

#### Side Effects

- detects LLM provider from environment variables
- performs network I/O to call the LLM provider
- potentially caches provider capabilities (via `get_probed_capabilities` mentioned in local context)
- may log errors or rate-limit events

#### Reads From

- parameter `model`
- parameter `system_prompt`
- parameter `request`
- parameter `loop`
- environment variables (via `detect_provider_from_environment`)
- provider label cache or mapping
- probed capabilities from `get_probed_capabilities`

#### Writes To

- sends network request to LLM provider
- returns result via coroutine task

#### Usage Patterns

- called by other coroutine functions to obtain LLM text responses
- used with `kota::event_loop` for non-blocking execution
- integrated into async completion pipelines that expect `kota::task` with `LLMError`

### `clore::net::validate_llm_provider_environment`

Declaration: `src/network/network.cppm:35`

Definition: `src/network/network.cppm:125`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The implementation of `clore::net::validate_llm_provider_environment` delegates all validation logic to the private helper `detect_provider_from_environment` in the anonymous namespace. That helper examines environment variables such as `kOpenAIApiKeyEnv`, `kAnthropicApiKeyEnv`, `kOpenAIBaseUrlEnv`, and `kAnthropicBaseUrlEnv` using the utility functions `has_nonempty_env` and `has_provider_env`. If `detect_provider_from_environment` fails, the error value is forwarded via `std::unexpected`; otherwise an empty success `std::expected<void, LLMError>` is returned. This design centralizes provider‑specific detection within the anonymous namespace, keeping the public interface a thin error‑propagation wrapper.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- environment variables via `detect_provider_from_environment`

#### Usage Patterns

- Called as a precondition before making LLM API calls to ensure the environment is properly set up.

## Internal Structure

The `network` module is decomposed into a public API surface and an internal implementation hidden within an anonymous namespace.  The public interface exposes three core functions – `call_llm_async`, `call_completion_async`, and `validate_llm_provider_environment` – that provide asynchronous LLM completion and environment validation.  All supporting logic, including provider detection (Anthropic/`OpenAI`), environment variable inspection, and asynchronous request dispatch via `kota::event_loop`, resides in the anonymous namespace, encapsulating implementation details and reducing coupling.

Internally, the module is layered to separate concerns: environment validation (`has_nonempty_env`, `has_provider_env`, `detect_provider_from_environment`), provider abstraction (the `Provider` enum and `provider_label`), and asynchronous request handling (`request_provider_text_async`, `dispatch_completion`).  The module imports only what is necessary from its dependencies (likely including `kota/event_loop` and standard library), and the use of a module partition (`network.cppm`) ensures a clear boundary between its public contract and private implementation, promoting maintainability and testability.

