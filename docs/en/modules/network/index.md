---
title: 'Module network'
description: 'The network module provides the asynchronous networking infrastructure for interacting with large language model (LLM) providers, specifically Anthropic and OpenAI. It encapsulates provider detection from environment variables, configuration validation, and the initiation of non‑blocking LLM requests and completion operations. The public interface consists of three key functions: validate_llm_provider_environment ensures the runtime environment is correctly set up, call_llm_async dispatches an LLM request, and call_completion_async handles completion‑style calls. All asynchronous operations are scheduled on a kota::event_loop and return an integer handle or status code. Internally, the module uses a Provider enum (with Anthropic and OpenAI members), environment variable constants for API keys and base URLs, and helper functions to detect the active provider and route requests accordingly.'
layout: doc
template: doc
---

# Module `network`

## Summary

The `network` module provides the asynchronous networking infrastructure for interacting with large language model (LLM) providers, specifically Anthropic and `OpenAI`. It encapsulates provider detection from environment variables, configuration validation, and the initiation of non‑blocking LLM requests and completion operations. The public interface consists of three key functions: `validate_llm_provider_environment` ensures the runtime environment is correctly set up, `call_llm_async` dispatches an LLM request, and `call_completion_async` handles completion‑style calls. All asynchronous operations are scheduled on a `kota::event_loop` and return an integer handle or status code. Internally, the module uses a `Provider` enum (with `Anthropic` and `OpenAI` members), environment variable constants for API keys and base `URLs`, and helper functions to detect the active provider and route requests accordingly.

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

The implementation of `clore::net::call_completion_async` follows a straightforward sequential coroutine flow. It first invokes the internal helper `detect_provider_from_environment` to determine the LLM provider (e.g., `Provider::Anthropic` or `Provider::OpenAI`) by inspecting environment variables such as `kAnthropicApiKeyEnv` and `kOpenAIApiKeyEnv`. If provider detection fails (returns an error), the function immediately `co_awaits` `kota::fail` to propagate the error. Otherwise, it moves the detected `provider` and delegates the actual network request to `dispatch_completion`, passing the provider, the `CompletionRequest`, and the `kota::event_loop`. The result of this dispatch is then wrapped through `detail::unwrap_caught_result` to convert a cancellation (via `catch_cancel`) into a structured `LLMError` with a descriptive message, before being `co_returned` as a `kota::task<CompletionResponse, LLMError>`.

Key dependencies include the anonymous-namespace helper functions `detect_provider_from_environment`, `dispatch_completion`, and `detail::unwrap_caught_result`. The `detect_provider_from_environment` function itself relies on `has_provider_env` and `has_nonempty_env` to check for provider‑specific environment variables like `kAnthropicBaseUrlEnv` and `kOpenAIBaseUrlEnv`. The overall control flow is entirely linear: environment validation first, then provider‑aware completion dispatch, each step using coroutine `co_await` or `co_return` for asynchronous error handling.

#### Side Effects

- Initiates an asynchronous network request via `dispatch_completion`.

#### Reads From

- Environment variables via `detect_provider_from_environment`
- `CompletionRequest` parameter `request`
- `kota::event_loop` reference `loop`

#### Usage Patterns

- Used to start an async LLM completion request.
- Called within a coroutine context where `co_await` is available.

### `clore::net::call_llm_async`

Declaration: `network/network.cppm:18`

Definition: `network/network.cppm:126`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The implementation of `clore::net::call_llm_async` begins by invoking the anonymous-namespace helper `detect_provider_from_environment()`, which inspects environment variables (such as `kAnthropicApiKeyEnv` and `kOpenAIApiKeyEnv`) to determine the intended LLM provider. If detection fails (e.g., no recognized API key is set), the function propagates the error via `kota::fail`. On success, it retrieves a human-readable label for the provider using `provider_label`, then delegates the actual asynchronous request to `request_provider_text_async`. That function is called with a completion lambda that maps a `CompletionRequest` to a `CompletionResponse` by calling `dispatch_completion`, which handles the per-provider network interaction (e.g., Anthropic or `OpenAI`). The entire coroutine chain is wrapped with `.or_fail()` to convert internal errors into the expected `LLMError` type. The returned `kota::task` yields a plain `std::string` upon success.

#### Side Effects

- Initiates HTTP request via `dispatch_completion`
- Reads environment variables for provider detection

#### Reads From

- model parameter
- `system_prompt` parameter
- request parameter
- environment variables for provider detection

#### Usage Patterns

- Used for async text completion requests to LLM providers
- Called within event loop coroutines

### `clore::net::validate_llm_provider_environment`

Declaration: `network/network.cppm:28`

Definition: `network/network.cppm:118`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function `clore::net::validate_llm_provider_environment` delegatess all validation logic to the anonymous‑namespace helper `detect_provider_from_environment`. This helper probes for a supported provider by checking environment variables such as `kAnthropicApiKeyEnv`, `kAnthropicBaseUrlEnv`, `kOpenAIApiKeyEnv`, and `kOpenAIBaseUrlEnv` via `has_provider_env` and `has_nonempty_env`. It returns a `std::expected<void, LLMError>` that either indicates success or carries an error describing which provider environment is misconfigured or absent. The outer function simply propagates that result without additional processing, making its control flow a straight pass‑through to the detection step.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- environment variables via `detect_provider_from_environment()`

#### Usage Patterns

- Called to check whether the LLM provider environment is properly configured before making API calls

## Internal Structure

The `network` module is implemented as a C++20 module in `network.cppm` and exposes three public entry points: `validate_llm_provider_environment`, `call_llm_async`, and `call_completion_async`. These form a thin asynchronous API for interacting with large language model (LLM) providers (Anthropic and `OpenAI`). The module imports the standard library and relies on an external `kota::event_loop` abstraction for scheduling non‑blocking operations.

Internally, the module uses an anonymous namespace to encapsulate the provider detection and dispatch machinery. An enumeration `Provider` distinguishes between `Anthropic` and `OpenAI`, and a set of static helpers—such as `detect_provider_from_environment`, `has_provider_env`, `has_nonempty_env`, `provider_label`, and `dispatch_completion`—handle environment variable validation, provider selection, and generic completion dispatching. The template function `request_provider_text_async` bridges the public API to the concrete provider‑specific logic. This layering keeps the public interface clean while allowing the implementation to be extended with additional providers or backends without affecting callers.

