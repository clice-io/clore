---
title: 'Namespace clore::net::protocol'
description: 'The clore::net::protocol namespace provides a centralized set of utilities for constructing, parsing, and validating messages exchanged over the network according to a defined protocol. Its primary responsibility is to handle the serialization and deserialization of JSON‑based requests and responses, with a strong emphasis on tool calls and completion responses. Notable declarations include functions for building request JSON (build_request_json), parsing raw response strings (parse_response), extracting textual content from responses (text_from_response, parse_response_text), manipulating tool call arguments and outputs (parse_tool_arguments, append_tool_outputs), and validating output formats such as JSON or markdown fragments (validate_json_output, validate_markdown_fragment_output). Architecturally, this namespace acts as the protocol abstraction layer, insulating the rest of the application from the low‑level details of message formatting and verification while ensuring that all communication adheres to the expected structure and semantics.'
layout: doc
template: doc
---

# Namespace `clore::net::protocol`

## Summary

The `clore::net::protocol` namespace provides a centralized set of utilities for constructing, parsing, and validating messages exchanged over the network according to a defined protocol. Its primary responsibility is to handle the serialization and deserialization of JSON‑based requests and responses, with a strong emphasis on tool calls and completion responses. Notable declarations include functions for building request JSON (`build_request_json`), parsing raw response strings (`parse_response`), extracting textual content from responses (`text_from_response`, `parse_response_text`), manipulating tool call arguments and outputs (`parse_tool_arguments`, `append_tool_outputs`), and validating output formats such as JSON or markdown fragments (`validate_json_output`, `validate_markdown_fragment_output`). Architecturally, this namespace acts as the protocol abstraction layer, insulating the rest of the application from the low‑level details of message formatting and verification while ensuring that all communication adheres to the expected structure and semantics.

## Functions

### `clore::net::protocol::append_tool_outputs`

Declaration: `network/protocol.cppm:469`

Definition: `network/protocol.cppm:540`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

The function `clore::net::protocol::append_tool_outputs` appends tool outputs derived from a given `CompletionResponse` to the internal state identified by the first integer parameter. The third integer parameter may control the number of outputs appended or otherwise modify the operation. It returns an integer status code that indicates success or failure. Callers are responsible for providing a valid `CompletionResponse` containing the tool outputs and ensuring the identifier refers to an existing target.

#### Usage Patterns

- Used by callers to incorporate tool execution results back into the conversation history
- Called after a tool call response is received and tool outputs have been collected

### `clore::net::protocol::build_request_json`

Declaration: `network/openai.cppm:457`

Definition: `network/openai.cppm:465`

Implementation: [`Module openai`](../../../../modules/openai/index.md)

The function `clore::net::protocol::build_request_json` constructs a JSON representation of a request as defined by the protocol. It accepts a single `const int &` parameter and returns an `int` outcome. Callers are responsible for providing a valid integer that represents the necessary request context; the returned integer indicates the result of the construction operation.

#### Usage Patterns

- serializing a completion request to JSON
- preparing data for HTTP request
- converting `CompletionRequest` to JSON string

### `clore::net::protocol::parse_response`

Declaration: `network/openai.cppm:459`

Definition: `network/openai.cppm:532`

Implementation: [`Module openai`](../../../../modules/openai/index.md)

`clore::net::protocol::parse_response` accepts a raw response string as a `std::string_view` and returns an `int` that represents the parsed result. Its primary responsibility is to interpret the response data according to the protocol’s format and produce an integer value that callers can use for further processing, such as status codes or identifiers.

The caller is expected to provide a well-formed response string that conforms to the protocol conventions. The function’s contract does not define the behavior for malformed or unexpected input; callers should ensure the input is valid before invoking this function.

#### Usage Patterns

- Called to parse a raw JSON response from an LLM API endpoint
- Used in the protocol layer to convert HTTP response body to domain object
- Typically invoked by higher-level functions that handle network responses

### `clore::net::protocol::parse_response_text`

Declaration: `network/protocol.cppm:475`

Definition: `network/protocol.cppm:588`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

The template function `clore::net::protocol::parse_response_text` accepts a `const CompletionResponse &` and returns an `int`. It is the caller’s entry point for extracting and parsing the textual component from a completion response. The `int` return value encodes the outcome of the parsing operation; its exact meaning depends on the template argument and should be interpreted according to the specific context in which the function is instantiated. Callers must supply a well-formed `CompletionResponse` and can rely on the function to handle the text‑parsing logic without needing to understand its internal implementation.

#### Usage Patterns

- Used to parse structured LLM responses into a specified type.
- Combines text extraction and JSON parsing with error handling.

### `clore::net::protocol::parse_tool_arguments`

Declaration: `network/protocol.cppm:478`

Definition: `network/protocol.cppm:603`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

The function `clore::net::protocol::parse_tool_arguments` is a public template that accepts a `const ToolCall &` and returns an `int`. Its caller-facing responsibility is to parse the tool arguments contained in the given `ToolCall` object, producing an integer result that typically represents a status code (e.g., success or error) or a count of parsed arguments. The caller is expected to provide a valid `ToolCall` instance, and the return value indicates the outcome of the parsing operation as defined by the protocol conventions.

#### Usage Patterns

- Used to deserialize tool call arguments into a typed structure.
- Typically called when processing a `ToolCall` to extract its arguments as a specific type.

### `clore::net::protocol::text_from_response`

Declaration: `network/protocol.cppm:467`

Definition: `network/protocol.cppm:524`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

The function `clore::net::protocol::text_from_response` extracts or processes the textual component of a `CompletionResponse`. Callers supply a `const CompletionResponse&` and receive an `int` that indicates the result of the operation—typically a status code or the number of characters processed. The contract requires that the provided response is valid and that the caller interprets the returned integer according to the protocol’s conventions.

#### Usage Patterns

- safely extract text content from `CompletionResponse`
- handle refusal, tool calls, or missing text errors
- convert response to text or error

### `clore::net::protocol::validate_json_output`

Declaration: `network/protocol.cppm:463`

Definition: `network/protocol.cppm:484`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

The caller-facing responsibility of `clore::net::protocol::validate_json_output` is to verify that the provided `std::string_view` represents a well-formed JSON output according to protocol expectations. The function returns an `int` status code that indicates whether the validation succeeded or failed, enabling callers to determine if the content is acceptable for further processing. The precise contract is that the input must be a syntactically valid JSON string that also meets any additional structural constraints defined by the protocol; a non-zero return value signals a validation failure.

#### Usage Patterns

- Validate JSON output before further processing
- Check if LLM output is valid JSON

### `clore::net::protocol::validate_markdown_fragment_output`

Declaration: `network/protocol.cppm:465`

Definition: `network/protocol.cppm:493`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

The function `clore::net::protocol::validate_markdown_fragment_output` accepts a `std::string_view` representing a markdown fragment output and returns an `int`. It verifies that the provided fragment conforms to the expected structural and semantic constraints of the protocol's markdown output format. A return value of zero typically indicates a valid fragment, while non-zero values signal specific validation failures, allowing callers to detect and handle malformed outputs. This function is part of the protocol's set of output validation utilities, used alongside similar validators like `clore::net::protocol::validate_json_output`.

#### Usage Patterns

- validation step in LLM response processing
- safeguard before markdown fragment interpretation

## Related Pages

- [Namespace clore::net](../index.md)

