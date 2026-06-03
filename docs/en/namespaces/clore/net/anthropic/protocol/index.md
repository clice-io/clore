---
title: 'Namespace clore::net::anthropic::protocol'
description: 'The clore::net::anthropic::protocol namespace encapsulates the core data structures and routines for constructing and parsing messages exchanged with the Anthropic API. It defines variables for request and response components—such as json, request, response, status, history, content, system_text, tools, tool_choice, and model—and exposes functions that handle URL construction (build_messages_url), JSON payload generation (build_request_json), response validation (parse_response), extraction of textual content and tool arguments (text_from_response, parse_response_text, parse_tool_arguments), and appending tool outputs to a request payload (append_tool_outputs). The function signatures consistently accept or return int values, which typically serve as status codes, identifiers, or handles for internal protocol objects.'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::protocol`

## Summary

The `clore::net::anthropic::protocol` namespace encapsulates the core data structures and routines for constructing and parsing messages exchanged with the Anthropic API. It defines variables for request and response components—such as `json`, `request`, `response`, `status`, `history`, `content`, `system_text`, `tools`, `tool_choice`, and `model`—and exposes functions that handle URL construction (`build_messages_url`), JSON payload generation (`build_request_json`), response validation (`parse_response`), extraction of textual content and tool arguments (`text_from_response`, `parse_response_text`, `parse_tool_arguments`), and appending tool outputs to a request payload (`append_tool_outputs`). The function signatures consistently accept or return `int` values, which typically serve as status codes, identifiers, or handles for internal protocol objects.

Architecturally, this namespace acts as the intermediary between the lower-level network layer and higher-level application logic. It abstracts the wire format of the Anthropic Messages API, allowing callers to work with typed, protocol‑specific operations without directly handling raw strings or JSON. The functions are designed to be composed in a pipeline: a raw response is parsed into a structured form, textual and tool‑call data are extracted, and request payloads are assembled for subsequent API calls. This separation of protocol concerns from networking and business logic promotes maintainability and testability within the `clore::net::anthropic` subsystem.

## Subnamespaces

- [`clore::net::anthropic::protocol::detail`](detail/index.md)

## Functions

### `clore::net::anthropic::protocol::append_tool_outputs`

Declaration: `src/network/anthropic.cppm:218`

Definition: `src/network/anthropic.cppm:637`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::append_tool_outputs` appends tool output data to an internal structure used for building an Anthropic API request. The caller supplies three parameters: an integer representing a context or sequence identifier, a constant reference to an integer containing the output content, and an integer for an index or size. It returns an integer that indicates a status code or a handle to the updated request payload. This operation is part of the request construction pipeline, intended for use alongside related utilities such as `build_request_json` and `parse_response`.

#### Usage Patterns

- Appending tool outputs to message history for Anthropic protocol
- Delegating to generic protocol function

### `clore::net::anthropic::protocol::build_messages_url`

Declaration: `src/network/anthropic.cppm:210`

Definition: `src/network/anthropic.cppm:233`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

Declaration: [Declaration](functions/build-messages-url.md)

This function constructs the full URL for the Anthropic Messages API endpoint. It accepts a single `std::string_view` argument — typically an API key or resource identifier — and returns a complete `std::string` URL. The caller is responsible for supplying a valid, non-empty string; the function does not validate the input. The returned URL is intended for use in subsequent HTTP requests to the Anthropic service.

#### Usage Patterns

- called by `Protocol::build_url` to generate the final URL endpoint

### `clore::net::anthropic::protocol::build_request_json`

Declaration: `src/network/anthropic.cppm:212`

Definition: `src/network/anthropic.cppm:244`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::build_request_json` is responsible for constructing the JSON payload of a request to the Anthropic API. It accepts a reference to an integer that identifies or contains the parameters needed to form the request, and returns an integer representing the outcome or a handle to the built JSON. The caller must provide a valid reference; the function assumes the referenced data is appropriately initialized. The contract does not specify the exact structure of the input, but the function is expected to produce a complete JSON request body suitable for subsequent transmission to the API endpoint.

#### Usage Patterns

- Called when preparing an HTTP request to the Anthropic API endpoint

### `clore::net::anthropic::protocol::parse_response`

Declaration: `src/network/anthropic.cppm:214`

Definition: `src/network/anthropic.cppm:469`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

This function parses a raw response string from the Anthropic API. It accepts a `std::string_view` containing the protocol-level response and returns an `int` value that indicates the outcome of the parsing operation, typically representing a success code, an error identifier, or a handle for further processing. Callers are expected to provide a well-formed response string as received from the network layer; the return value governs subsequent steps such as extracting text or tool arguments via companion functions.

#### Usage Patterns

- converts raw HTTP response body to structured result
- called after receiving Anthropic API response
- handles error payloads and missing fields

### `clore::net::anthropic::protocol::parse_response_text`

Declaration: `src/network/anthropic.cppm:224`

Definition: `src/network/anthropic.cppm:645`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

Parses the text content from an Anthropic API response. This function template, `clore::net::anthropic::protocol::parse_response_text`, accepts a response object of type `T` by constant reference and extracts the textual portion of that response. It returns an integral value that represents the parsed text result, as defined by the protocol layer.

Callers are expected to supply a valid response object from an Anthropic API call. The function isolates the text component from the response structure, enabling downstream processing of the message content.

#### Usage Patterns

- Called within the Anthropic protocol namespace to parse response text into a specific type T.

### `clore::net::anthropic::protocol::parse_tool_arguments`

Declaration: `src/network/anthropic.cppm:227`

Definition: `src/network/anthropic.cppm:650`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The template function `clore::net::anthropic::protocol::parse_tool_arguments` is responsible for extracting and parsing the arguments of a tool call from an Anthropic API response. The caller supplies a `const int &` parameter, which likely represents a tool identifier or index within the response. The function returns an `int`—typically indicating success status, a parsed value, or an error code. As a template parameterized by `typename T`, the function may adapt its parsing logic to accommodate different argument types or conversion targets, providing flexibility while maintaining a uniform invocation interface.

#### Usage Patterns

- used to deserialize tool arguments into a specific type
- called when handling tool use responses from the Anthropic API

### `clore::net::anthropic::protocol::text_from_response`

Declaration: `src/network/anthropic.cppm:216`

Definition: `src/network/anthropic.cppm:632`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::text_from_response` extracts the textual content from a given response object. The caller provides a reference to a response (represented here as an `int`) and receives an integer value, which may serve as a status code or identifier for the extracted text. This routine is part of the protocol‑level parsing layer and is expected to be used after obtaining a response from the Anthropic API.

#### Usage Patterns

- extract text content from an Anthropic `CompletionResponse`

## Related Pages

- [Namespace clore::net::anthropic](../index.md)
- [Namespace clore::net::anthropic::protocol::detail](detail/index.md)

