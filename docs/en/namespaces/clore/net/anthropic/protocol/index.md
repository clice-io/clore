---
title: 'Namespace clore::net::anthropic::protocol'
description: 'The clore::net::anthropic::protocol namespace encapsulates the protocol‑specific logic for communicating with the Anthropic Messages API. It provides a collection of standalone functions that handle the entire request‑response lifecycle: building the endpoint URL via build_messages_url, constructing the JSON request body with build_request_json, parsing raw API responses with parse_response, and extracting structured output such as text (text_from_response, parse_response_text) or tool arguments (parse_tool_arguments). The namespace also includes utilities to manage tool invocation results (append_tool_outputs) and exposes key protocol variables like model_value, stop_reason, content_value, tools, tool_choice, and api_base.'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::protocol`

## Summary

The `clore::net::anthropic::protocol` namespace encapsulates the protocol‑specific logic for communicating with the Anthropic Messages API. It provides a collection of standalone functions that handle the entire request‑response lifecycle: building the endpoint URL via `build_messages_url`, constructing the JSON request body with `build_request_json`, parsing raw API responses with `parse_response`, and extracting structured output such as text (`text_from_response`, `parse_response_text`) or tool arguments (`parse_tool_arguments`). The namespace also includes utilities to manage tool invocation results (`append_tool_outputs`) and exposes key protocol variables like `model_value`, `stop_reason`, `content_value`, `tools`, `tool_choice`, and `api_base`.

Architecturally, this namespace acts as a dedicated protocol abstraction layer within the `clore::net::anthropic` module. By isolating Anthropic‑specific message formatting, URL construction, and response parsing, it enables higher‑level networking and client code to remain agnostic of the wire format details. The functions are designed to be composed: for example, `parse_response` returns an opaque handle that can be fed into `parse_response_text` or `parse_tool_arguments`. This design promotes separation of concerns, making the protocol logic independently testable and reusable across different integration contexts.

## Subnamespaces

- [`clore::net::anthropic::protocol::detail`](detail/index.md)

## Functions

### `clore::net::anthropic::protocol::append_tool_outputs`

Declaration: `network/anthropic.cppm:209`

Definition: `network/anthropic.cppm:628`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::append_tool_outputs` appends the outputs of tool invocations into the caller‑supplied result structure. It accepts the necessary identifiers or parameters that identify which tool outputs to incorporate and that determine the target location for the appended data. The function does not modify its source arguments and leaves ownership of the appended content with the caller.

#### Usage Patterns

- Appending tool outputs to a conversation history
- Converting a completion response and tool outputs into an updated message list

### `clore::net::anthropic::protocol::build_messages_url`

Declaration: `network/anthropic.cppm:201`

Definition: `network/anthropic.cppm:224`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

Declaration: [Declaration](functions/build-messages-url.md)

Constructs the complete URL for the Anthropic Messages API endpoint. Given a string view that identifies the target resource (for example, an API key or version token), `clore::net::anthropic::protocol::build_messages_url` returns the fully qualified URL as a `std::string`. The caller must supply a valid string view; no validation is performed within the function.

#### Usage Patterns

- Called by `clore::net::anthropic::detail::Protocol::build_url`

### `clore::net::anthropic::protocol::build_request_json`

Declaration: `network/anthropic.cppm:203`

Definition: `network/anthropic.cppm:235`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::build_request_json` is responsible for constructing the JSON request body that is sent as part of the Anthropic Messages API interaction. It accepts a `const int &` parameter representing the input data (e.g., the conversation context or message content) and returns an `int` that encodes the resulting JSON payload. Callers provide the necessary request information and use the returned value as the serialised request body for subsequent HTTP transmission.

#### Usage Patterns

- Called to generate the request body for an Anthropic API call

### `clore::net::anthropic::protocol::parse_response`

Declaration: `network/anthropic.cppm:205`

Definition: `network/anthropic.cppm:460`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::parse_response` accepts a raw response payload as a `std::string_view` and returns an `int` value whose interpretation is defined by the protocol layer. Callers should provide a complete response string from the Anthropic API; the function then parses it into an internal representation. The returned integer indicates success or an error condition, and may be used as input to other protocol functions such as `parse_response_text` or `text_from_response`.

#### Usage Patterns

- Called to deserialize an Anthropic API response string into a structured object
- Used in error handling to detect API errors or truncation
- Consumed by higher-level protocol functions that need a `CompletionResponse`

### `clore::net::anthropic::protocol::parse_response_text`

Declaration: `network/anthropic.cppm:215`

Definition: `network/anthropic.cppm:636`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::parse_response_text` extracts the textual portion from a structured Anthropic API response. It is a template, parameterized by the response type `T`, and accepts a `const int&` (likely a simplified placeholder for a JSON or structured response object). Callers typically invoke this function after calling `parse_response` to retrieve the generated text content. The function returns an integer value that indicates the outcome of the extraction, where a successful parse generally corresponds to a non‑error code. The caller is responsible for ensuring that the provided response is well‑formed and contains a valid text block; otherwise, the behavior is implementation‑defined and may result in an error return value or an exception.

#### Usage Patterns

- Called when parsing a text response from the Anthropic API
- Used to convert raw API response to a typed `expected` result

### `clore::net::anthropic::protocol::parse_tool_arguments`

Declaration: `network/anthropic.cppm:218`

Definition: `network/anthropic.cppm:641`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The function template `clore::net::anthropic::protocol::parse_tool_arguments` is a protocol-level utility responsible for parsing tool arguments from an Anthropic API response. The caller supplies a constant reference to an integer that represents the raw input data (such as a serialized arguments payload or an opaque handle). The function returns an integer that encodes the parsed result—either a status code indicating success or failure, or a handle that can be used to access the structured arguments. The caller can assume that the input reference remains unmodified after the call, and that the returned integer is valid for the duration of the current request-handling context. This contract allows callers to safely invoke the function without needing to manage memory or ownership of the input data.

#### Usage Patterns

- Used to deserialize tool call arguments into a strongly typed object

### `clore::net::anthropic::protocol::text_from_response`

Declaration: `network/anthropic.cppm:207`

Definition: `network/anthropic.cppm:623`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::text_from_response` extracts the textual content from an Anthropic API response. It accepts a reference to a response object (currently modeled as a `const int&`) and returns an integral value that represents the result of the extraction or the extracted text itself, depending on the interpretation of the return type. Callers are responsible for providing a valid response object; the function handles the protocol-specific parsing to isolate the text portion for further use.

#### Usage Patterns

- Used to retrieve the textual content from a `CompletionResponse`.

## Related Pages

- [Namespace clore::net::anthropic](../index.md)
- [Namespace clore::net::anthropic::protocol::detail](detail/index.md)

