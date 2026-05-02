---
title: 'Namespace clore::net::protocol'
description: 'The clore::net::protocol namespace provides the core communication layer for interacting with a remote service (e.g., an LLM API). It is responsible for constructing JSON‑formatted requests (build_request_json), parsing raw network responses (parse_response), and extracting structured data (e.g., text_from_response, parse_response_text). The namespace also includes tool‑call lifecycle functions: parse_tool_arguments deserialises tool call arguments into a caller‑specified type, and append_tool_outputs collects tool results for subsequent requests. Additionally, it offers validation utilities for both JSON (validate_json_output) and Markdown fragment (validate_markdown_fragment_output) outputs, ensuring that responses conform to expected formats.'
layout: doc
template: doc
---

# Namespace `clore::net::protocol`

## Summary

The `clore::net::protocol` namespace provides the core communication layer for interacting with a remote service (e.g., an LLM API). It is responsible for constructing JSON‑formatted requests (`build_request_json`), parsing raw network responses (`parse_response`), and extracting structured data (e.g., `text_from_response`, `parse_response_text`). The namespace also includes tool‑call lifecycle functions: `parse_tool_arguments` deserialises tool call arguments into a caller‑specified type, and `append_tool_outputs` collects tool results for subsequent requests. Additionally, it offers validation utilities for both JSON (`validate_json_output`) and Markdown fragment (`validate_markdown_fragment_output`) outputs, ensuring that responses conform to expected formats.

Architecturally, `clore::net::protocol` serves as a well‑defined abstraction between higher‑level application logic and the raw byte streams of the network. By centralising serialisation, deserialisation, and validation tasks, it isolates the rest of the codebase from protocol‑specific details and error conventions. The functions return integral status codes, enabling callers to uniformly handle success or failure without requiring deep knowledge of the underlying protocol. Notable declarations include the tool‑argument and response‑text parsing templates, which allow type‑safe extraction of content from service replies.

## Functions

### `clore::net::protocol::append_tool_outputs`

Declaration: `network/protocol.cppm:473`

Definition: `network/protocol.cppm:544`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

The function `clore::net::protocol::append_tool_outputs` is responsible for adding tool outputs from a given `CompletionResponse` to a tool output collection. The caller supplies an integer to identify the tool call or session context, the response containing output data, and a second integer that likely specifies a tool index or output target. The function returns an `int` to indicate success or failure of the append operation. The caller must ensure that the `CompletionResponse` is valid and that the integer arguments refer to an existing context; otherwise, the behavior is defined by the protocol contract.

#### Usage Patterns

- Called to integrate tool outputs into a conversation history
- Used after receiving a completion response containing tool calls
- Part of protocol‑level message construction for LLM interactions

### `clore::net::protocol::build_request_json`

Declaration: `network/openai.cppm:457`

Definition: `network/openai.cppm:465`

Implementation: [`Module openai`](../../../../modules/openai/index.md)

The function `clore::net::protocol::build_request_json` constructs a JSON‑formatted request for the network protocol. It accepts a single parameter of type `const int &` (typically a request identifier or context handle) and returns an `int` that indicates the result of the operation, such as a success code or error status. Callers should supply a valid integer argument when they need to generate a serialized request payload; the returned integer allows the caller to verify that the request was built successfully and to respond accordingly.

#### Usage Patterns

- Serializing a `CompletionRequest` into a JSON string for network transmission
- Building the request payload for an `OpenAI` API call

### `clore::net::protocol::parse_response`

Declaration: `network/openai.cppm:459`

Definition: `network/openai.cppm:532`

Implementation: [`Module openai`](../../../../modules/openai/index.md)

The function `parse_response` accepts a `std::string_view` representing a raw network response and returns an `int`. Its caller‑facing contract is to parse the provided response and produce a status code that indicates success or failure. The caller must supply a complete and well‑formed response string; the meaning of the returned `int` is defined by the `clore::net::protocol` module and should be interpreted according to its conventions.

#### Usage Patterns

- parsing an LLM API JSON response
- extracting a `CompletionResponse` from raw response text
- validating response structure and error conditions

### `clore::net::protocol::parse_response_text`

Declaration: `network/protocol.cppm:479`

Definition: `network/protocol.cppm:592`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

The function template `clore::net::protocol::parse_response_text` parses the text content of a provided `CompletionResponse` according to the template parameter `T`. It returns an `int` that signals the result of the parsing operation, typically an error code or a count of parsed elements. The caller is responsible for supplying a valid `CompletionResponse` and ensuring that the type `T` matches the expected structure of the response text.

#### Usage Patterns

- Used to convert a `CompletionResponse` into a structured type `T` after an LLM completion
- Typically invoked after receiving a response that contains a JSON body to be deserialized

### `clore::net::protocol::parse_tool_arguments`

Declaration: `network/protocol.cppm:482`

Definition: `network/protocol.cppm:607`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

The function `clore::net::protocol::parse_tool_arguments` accepts a `const ToolCall &` and returns an `int` that signals success or failure. It is a template parameterised by `T`, indicating that callers provide a target type expected from the tool call’s arguments. The contract is that the tool call must contain data parseable into `T`; if parsing fails, a non‑zero error code is returned. The caller is responsible for interpreting the return value according to the protocol’s error conventions.

#### Usage Patterns

- deserialize tool call arguments into the expected type T
- called during LLM response processing to extract typed tool parameters

### `clore::net::protocol::text_from_response`

Declaration: `network/protocol.cppm:471`

Definition: `network/protocol.cppm:528`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

The function `clore::net::protocol::text_from_response` accepts a `const CompletionResponse &` and returns an `int`. It extracts the textual content from the response and provides an integer result that may represent the length of the extracted text, a success status, or other metadata. The caller is responsible for supplying a valid `CompletionResponse` containing the expected textual data; the function does not validate the response structure. The returned integer is intended for further processing or error checking by the caller.

#### Usage Patterns

- Extracting text from a `CompletionResponse` when text is expected
- Validating that the response is not a refusal and does not contain tool calls

### `clore::net::protocol::validate_json_output`

Declaration: `network/protocol.cppm:467`

Definition: `network/protocol.cppm:488`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

This function checks the syntactic and semantic correctness of a given JSON string. It accepts a `std::string_view` representing the output to validate. Returns an integer status code that signals whether the validation succeeded (often zero) or failed (non-zero). Callers should pass the raw JSON output they have received and inspect the return value to determine if the output is well-formed and meets protocol expectations.

#### Usage Patterns

- validate JSON output from an LLM response
- called before further processing of tool call outputs or responses

### `clore::net::protocol::validate_markdown_fragment_output`

Declaration: `network/protocol.cppm:469`

Definition: `network/protocol.cppm:497`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

The function `clore::net::protocol::validate_markdown_fragment_output` verifies that a given output string conforms to the protocol’s expectations for a markdown fragment. It accepts a `std::string_view` representing the raw output and returns an `int` to indicate the result of validation. The contract requires the caller to supply the output as a `std::string_view`; the return value signals whether the output is acceptable (typically zero for success, non-zero for a specific validation failure). This function is part of the protocol’s validation suite and does not modify its argument or any global state.

#### Usage Patterns

- Used to validate that LLM output is a markdown fragment in the protocol pipeline
- Likely called after extracting text from a completion response

## Related Pages

- [Namespace clore::net](../index.md)

