---
title: 'Namespace clore::net::anthropic::protocol'
description: 'The clore::net::anthropic::protocol namespace provides the core protocol‑layer logic for interacting with the Anthropic API. It encapsulates all message formatting, URL construction, request serialization, and response parsing, abstracting the wire‑level details so that higher‑level code can work with strongly‑typed representations of API calls and results. This namespace acts as the bridge between raw HTTP communication and domain‑specific usage of the Anthropic Messages API.'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::protocol`

## Summary

The `clore::net::anthropic::protocol` namespace provides the core protocol‑layer logic for interacting with the Anthropic API. It encapsulates all message formatting, URL construction, request serialization, and response parsing, abstracting the wire‑level details so that higher‑level code can work with strongly‑typed representations of API calls and results. This namespace acts as the bridge between raw HTTP communication and domain‑specific usage of the Anthropic Messages API.

Notable declarations include `build_messages_url` and `build_request_json`, which construct the endpoint URL and JSON payload for outgoing requests, and `parse_response`, `parse_response_text`, and `text_from_response`, which deserialize incoming responses and extract textual content. Helper functions such as `parse_tool_arguments` and `append_tool_outputs` manage tool‑use interactions by extracting tool call details and appending tool outputs back into the request flow. Numerous variables (e.g., `stop_reason_value`, `model_value`, `api_base`, `status`, `error_value`) serve as symbolic constants for protocol field names and status identifiers, ensuring consistency throughout the protocol implementation.

## Subnamespaces

- [`clore::net::anthropic::protocol::detail`](detail/index.md)

## Functions

### `clore::net::anthropic::protocol::append_tool_outputs`

Declaration: `network/anthropic.cppm:209`

Definition: `network/anthropic.cppm:628`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::append_tool_outputs` appends a tool output to an ongoing protocol message or request structure. The caller passes an existing state identifier (first `int` parameter), a tool output represented as a `const int &`, and an additional `int` parameter that likely controls the append operation (e.g., a limit or offset). The return `int` communicates the resulting state or a success indicator.

The contract requires that the first parameter corresponds to a valid, modifiable protocol context (such as a partially built request), and that the second parameter holds a properly encoded tool output. The third parameter may constrain the operation, such as a maximum number of outputs or a step index. The function does not handle invalid parameters; the caller must ensure they are appropriate for the protocol version in use.

#### Usage Patterns

- Delegates to generic protocol function
- Used to incorporate tool outputs into a message history

### `clore::net::anthropic::protocol::build_messages_url`

Declaration: `network/anthropic.cppm:201`

Definition: `network/anthropic.cppm:224`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

Declaration: [Declaration](functions/build-messages-url.md)

The function `clore::net::anthropic::protocol::build_messages_url` accepts a `std::string_view` (presumably a resource identifier or API key component) and returns a `std::string` representing the fully constructed URL for the Anthropic Messages API endpoint. Callers provide the necessary input to parameterize the URL; the function encapsulates the endpoint path and any required base URL logic. The returned string is suitable for use in HTTP requests to the Anthropic service.

#### Usage Patterns

- called by `clore::net::anthropic::detail::Protocol::build_url` to produce the messages endpoint URL

### `clore::net::anthropic::protocol::build_request_json`

Declaration: `network/anthropic.cppm:203`

Definition: `network/anthropic.cppm:235`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::build_request_json` constructs the JSON request payload for the Anthropic API. It accepts an argument of type `const int &` that represents the data necessary to build the request body. The function returns an `int` indicating the result of the construction, typically used to signal success or provide a handle for the built request. This function is part of the protocol layer and is expected to be called before submitting a request to the Anthropic messages endpoint.

#### Usage Patterns

- Construct HTTP request payload for Anthropic API
- Serialize `CompletionRequest` to JSON string

### `clore::net::anthropic::protocol::parse_response`

Declaration: `network/anthropic.cppm:205`

Definition: `network/anthropic.cppm:460`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::parse_response` accepts a raw response payload as a `std::string_view` and returns an integral value. Callers provide the complete response text received from the Anthropic API; the function interprets that content and returns a result that can be subsequently consumed by other protocol helpers such as `clore::net::anthropic::protocol::text_from_response` or `clore::net::anthropic::protocol::parse_response_text`. The returned `int` acts as a handle or status identifier — a successful parse yields a non‑error value, while an invalid or malformed response produces a designated error indicator. Callers are responsible for ensuring the input string is a well‑formed API response before invoking this function, and for checking the returned value against known outcomes before using it in further protocol operations.

#### Usage Patterns

- Used to parse Anthropic API response JSON into a structured result for further processing.

### `clore::net::anthropic::protocol::parse_response_text`

Declaration: `network/anthropic.cppm:215`

Definition: `network/anthropic.cppm:636`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::parse_response_text` is a template function that extracts the textual portion of an Anthropic API response. It accepts a constant reference to a response object (represented here as `const int &`) and returns the parsed text as an integer (mocked as `int`). The caller is responsible for providing a valid response structure from which the text can be isolated; the function assumes the response is well-formed and focuses solely on retrieving the main content, ignoring any control or metadata fields.

#### Usage Patterns

- delegation to generic `parse_response_text`
- template instantiation for response types

### `clore::net::anthropic::protocol::parse_tool_arguments`

Declaration: `network/anthropic.cppm:218`

Definition: `network/anthropic.cppm:641`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::parse_tool_arguments` is a template, parameterized by `T`, that accepts a `const int &` (interpreted as a handle or identifier to a response containing tool calls) and returns an `int` (a handle or identifier to the parsed tool arguments). Callers should provide the response handle and, upon success, receive a handle that can be used to access the extracted tool arguments for further processing—for example, when constructing subsequent tool-use messages. The exact interpretation of the integer handles and the type `T` is governed by the protocol implementation; the function assumes the input handle references a well‑formed response containing tool use data.

#### Usage Patterns

- Extract typed tool arguments from a `ToolCall`
- Bridge between Anthropic-specific and generic protocol parsing

### `clore::net::anthropic::protocol::text_from_response`

Declaration: `network/anthropic.cppm:207`

Definition: `network/anthropic.cppm:623`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::text_from_response` extracts the textual content from a parsed Anthropic API response. It accepts a reference to a response object (typically obtained from `parse_response`) and returns the resulting text as a value suitable for further processing or display.

The caller must supply a valid, fully‑parsed response; the function does not perform validation of the input. It is the caller’s responsibility to ensure the response object was correctly constructed and that the expected text content is present.

#### Usage Patterns

- extract text from Anthropic `CompletionResponse`
- called by higher-level response parsing code

## Related Pages

- [Namespace clore::net::anthropic](../index.md)
- [Namespace clore::net::anthropic::protocol::detail](detail/index.md)

