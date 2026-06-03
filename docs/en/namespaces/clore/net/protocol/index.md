---
title: 'Namespace clore::net::protocol'
description: 'The clore::net::protocol namespace provides a collection of functions and shared data structures that mediate communication between the application layer and remote services. Its primary responsibility is to handle the lifecycle of network protocol messages: constructing request payloads (e.g., build_request_json), parsing raw responses (parse_response, parse_response_text), extracting textual content from completion responses (text_from_response), and validating output format (validate_json_output, validate_markdown_fragment_output). It also manages interactions with tool‑calling mechanisms through parse_tool_arguments and append_tool_outputs, which extract arguments from tool calls and aggregate their results into a processing context. The namespace acts as a thin, self‑contained abstraction layer that isolates core logic from the specifics of the wire format, using integer return values to signal success or protocol‑specific error conditions. Variables such as parsed_calls, message_value, response, and content serve as internal storage for intermediate parsing and composition steps within these functions.'
layout: doc
template: doc
---

# Namespace `clore::net::protocol`

## Summary

The `clore::net::protocol` namespace provides a collection of functions and shared data structures that mediate communication between the application layer and remote services. Its primary responsibility is to handle the lifecycle of network protocol messages: constructing request payloads (e.g., `build_request_json`), parsing raw responses (`parse_response`, `parse_response_text`), extracting textual content from completion responses (`text_from_response`), and validating output format (`validate_json_output`, `validate_markdown_fragment_output`). It also manages interactions with tool‑calling mechanisms through `parse_tool_arguments` and `append_tool_outputs`, which extract arguments from tool calls and aggregate their results into a processing context. The namespace acts as a thin, self‑contained abstraction layer that isolates core logic from the specifics of the wire format, using integer return values to signal success or protocol‑specific error conditions. Variables such as `parsed_calls`, `message_value`, `response`, and `content` serve as internal storage for intermediate parsing and composition steps within these functions.

## Functions

### `clore::net::protocol::append_tool_outputs`

Declaration: `src/network/protocol.cppm:485`

Definition: `src/network/protocol.cppm:556`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

The function `clore::net::protocol::append_tool_outputs` appends tool outputs from a given `CompletionResponse` to a context identified by an integer handle. It accepts an initial integer (representing a session or request identifier), the response object containing tool results, and an integer parameter that may control filtering, maximum number of outputs, or a target index. The returned integer indicates the count of appended outputs or a status code. Callers must supply a valid `CompletionResponse` and a context identifier that refers to an active processing session; the precise contract for each parameter and the return value is defined by the protocol specification.

#### Usage Patterns

- Processing tool call results to reconstruct full message sequence
- Appending assistant tool call and tool result messages after an API response with tool calls

### `clore::net::protocol::build_request_json`

Declaration: `src/network/openai.cppm:467`

Definition: `src/network/openai.cppm:475`

Implementation: [`Module openai`](../../../../modules/openai/index.md)

Builds a JSON request payload from the provided `const int &` argument. The caller supplies an integer that likely identifies or configures the request; the function assembles the corresponding JSON representation needed by the protocol layer. It returns an `int` status code that the caller can use to verify success or handle errors in subsequent protocol steps.

#### Usage Patterns

- called by higher-level networking functions to build request payloads
- used in the `OpenAI` protocol implementation to convert request objects to JSON strings

### `clore::net::protocol::parse_response`

Declaration: `src/network/openai.cppm:469`

Definition: `src/network/openai.cppm:542`

Implementation: [`Module openai`](../../../../modules/openai/index.md)

The `clore::net::protocol::parse_response` function accepts a raw response payload as a `std::string_view` and returns an `int` that indicates the result of parsing. Callers are responsible for providing a complete, protocol‑compatible response. The integer return value communicates success (typically zero) or a specific error condition; the exact mapping is defined by the protocol module.

#### Usage Patterns

- Parse a JSON response from an LLM API into a structured result
- Used after receiving the HTTP response body of a chat completion request

### `clore::net::protocol::parse_response_text`

Declaration: `src/network/protocol.cppm:491`

Definition: `src/network/protocol.cppm:604`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

Extracts the text portion from a given `CompletionResponse`. Returns an integer status that indicates whether the operation succeeded (typically zero) or failed with a protocol‑specific error code. The caller should check this return value to determine if the extracted text is valid before using it.

#### Usage Patterns

- Parse structured LLM response into type T
- Used after receiving a completion response

### `clore::net::protocol::parse_tool_arguments`

Declaration: `src/network/protocol.cppm:494`

Definition: `src/network/protocol.cppm:619`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

The function `clore::net::protocol::parse_tool_arguments` is a template function that takes a constant reference to a `ToolCall` and returns an `int`. It is responsible for extracting and interpreting the arguments provided in the tool call, converting them into a form that can be validated or used by subsequent protocol handlers. The return value typically indicates success or failure of the parsing step, and its exact meaning may depend on the template parameter. Callers should pass a fully constructed `ToolCall` and rely on the returned `int` to determine whether the argument structure was valid and complete.

#### Usage Patterns

- used to convert tool call arguments into typed structure
- called in tool invocation handling

### `clore::net::protocol::text_from_response`

Declaration: `src/network/protocol.cppm:483`

Definition: `src/network/protocol.cppm:540`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

The function `clore::net::protocol::text_from_response` accepts a constant reference to a `CompletionResponse` and returns an `int`. It extracts the textual content from the response, enabling callers to retrieve the generated text in a straightforward manner. The caller must ensure that the provided response is valid and contains text output as expected by the protocol; otherwise, the behavior is undefined. The returned integer typically indicates a status code or the length of the extracted text, as defined by the protocol layer.

#### Usage Patterns

- Extract text from a completion response for further processing

### `clore::net::protocol::validate_json_output`

Declaration: `src/network/protocol.cppm:479`

Definition: `src/network/protocol.cppm:500`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

The public function `clore::net::protocol::validate_json_output` is part of the protocol layer and accepts a single `std::string_view` argument. Its responsibility is to determine whether the provided string is a syntactically valid JSON output. The caller supplies the JSON text and receives an `int` return value that indicates the result of the validation; a zero value typically denotes success, while a non‑zero value signals an invalid input or some other validation failure. This function is intended to be used by callers that need to confirm the structural correctness of a JSON response or payload before proceeding with further processing in the protocol flow.

#### Usage Patterns

- Validating JSON output from an LLM before further processing
- Called as a validation step in protocol handling

### `clore::net::protocol::validate_markdown_fragment_output`

Declaration: `src/network/protocol.cppm:481`

Definition: `src/network/protocol.cppm:509`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

This function validates a markdown fragment output. It accepts a `std::string_view` representing the fragment content and returns an `int` indicating whether the output meets the required format and constraints. A return value of zero signifies success; a non-zero value indicates a specific validation failure.

#### Usage Patterns

- Validating LLM responses expected to be markdown fragments
- Called during response parsing in protocol module

## Related Pages

- [Namespace clore::net](../index.md)

