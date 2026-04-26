---
title: 'Module support'
description: 'The support module provides foundational utilities for UTF-8 text handling, file I/O, path and cache key management, and a structured logging framework. Its public interface includes functions for reading and writing UTF-8 text files (with BOM stripping), normalizing paths and line endings, building deterministic cache keys from compilation signatures, and transparently hashing and comparing string-like types for heterogeneous lookup. The module also exposes helper types such as TransparentStringHash and TransparentStringEqual that enable efficient container lookups without temporary allocations.'
layout: doc
template: doc
---

# Module `support`

## Summary

The `support` module provides foundational utilities for UTF-8 text handling, file I/O, path and cache key management, and a structured logging framework. Its public interface includes functions for reading and writing UTF-8 text files (with BOM stripping), normalizing paths and line endings, building deterministic cache keys from compilation signatures, and transparently hashing and comparing string-like types for heterogeneous lookup. The module also exposes helper types such as `TransparentStringHash` and `TransparentStringEqual` that enable efficient container lookups without temporary allocations.

Within the same module, the `clore::logging` namespace implements a flexible logging system with compile-time log level proxies (`trace`, `debug`, `info`, `warn`, `err`) and a central `log` function that dispatches messages to sinks. It also provides utility functions for querying the cache hit rate, normalizing log level names, and writing to stderr. Together, these components offer a set of low‑level, reusable building blocks that support the broader application’s text processing, caching, and diagnostic logging needs.

## Imports

- `std`

## Imported By

- [`agent`](../agent/index.md)
- [`agent:tools`](../agent/tools.md)
- [`anthropic`](../anthropic/index.md)
- [`client`](../client/index.md)
- [`config:load`](../config/load.md)
- [`extract`](../extract/index.md)
- [`extract:ast`](../extract/ast.md)
- [`extract:cache`](../extract/cache.md)
- [`extract:compiler`](../extract/compiler.md)
- [`extract:merge`](../extract/merge.md)
- [`extract:model`](../extract/model.md)
- [`extract:scan`](../extract/scan.md)
- [`generate:analysis`](../generate/analysis.md)
- [`generate:cache`](../generate/cache.md)
- [`generate:diagram`](../generate/diagram.md)
- [`generate:model`](../generate/model.md)
- [`generate:page`](../generate/page.md)
- [`generate:planner`](../generate/planner.md)
- [`generate:scheduler`](../generate/scheduler.md)
- [`http`](../http/index.md)
- [`openai`](../openai/index.md)
- [`protocol`](../protocol/index.md)
- [`schema`](../schema/index.md)

## Types

### `clore::logging::LogProxy`

Declaration: `support/logging.cppm:112`

Definition: `support/logging.cppm:112`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

The `clore::logging::LogProxy` struct is a templated callable whose non-type template parameter `Level` encodes a specific severity level from `spdlog::level::level_enum`. Its two `operator()` overloads serve as thin wrappers that forward the message payload to the single logging entry point `clore::logging::log`. The `std::string_view` overload passes the plain message directly; the variadic overload, constrained by a `requires` clause to require at least one argument, formats the message using `std::format` before forwarding. Both overloads always pass the stored `Level` as the severity, ensuring that every call to the proxy consistently emits log output at the intended granularity without repeating the level value at each usage site. The proxy itself holds no state; it is an empty, trivially copyable utility that exists solely to bind a severity level to a callable interface, enabling convenient composition with higher‑level logging primitives.

#### Invariants

- template parameter `Level` is a `spdlog::level::level_enum`
- `operator()` overloads always call `log(Level, ...)`
- format-based overload requires `sizeof...(Args) > 0`

#### Key Members

- `operator()(std::string_view msg)`
- `operator()(std::format_string<Args...> fmt, Args&&... args)`

#### Usage Patterns

- used to create type-safe log callables for different log levels
- instances of `LogProxy` can be stored and passed as logging handlers, ensuring consistent level handling

#### Member Functions

##### `clore::logging::LogProxy::operator()`

Declaration: `support/logging.cppm:119`

Definition: `support/logging.cppm:119`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

###### Implementation

```cpp
void operator()(std::format_string<Args...> fmt, Args&&... args) const {
        log(Level, std::format(fmt, std::forward<Args>(args)...));
    }
```

##### `clore::logging::LogProxy::operator()`

Declaration: `support/logging.cppm:113`

Definition: `support/logging.cppm:113`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

###### Implementation

```cpp
void operator()(std::string_view msg) const {
        log(Level, msg);
    }
```

### `clore::support::CacheKeyParts`

Declaration: `support/logging.cppm:57`

Definition: `support/logging.cppm:57`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The struct `clore::support::CacheKeyParts` is a plain data aggregate bundling the two fields `path` and `compile_signature`. Its purpose is to represent a composite key that uniquely identifies a cache entry, likely within a compilation‑caching subsystem. The `path` field stores the file path as a `std::string`, while `compile_signature` is a `std::uint64_t` default‑initialized to `0` and intended to capture a hash or fingerprint of the compilation inputs. No special member functions or invariants are user‑defined; the struct relies on default construction and copy semantics. The default value of `0` for `compile_signature` serves as a sentinel for an unset or trivial signature, and callers are responsible for ensuring that a non‑zero signature is assigned when the key is used in lookups.

#### Invariants

- `compile_signature` is default-initialized to 0 if not explicitly provided.
- The struct is trivially copyable and movable via default compiler-generated operations.

#### Key Members

- `path` – the file path component of the cache key
- `compile_signature` – an integer hash or signature representing compilation inputs

#### Usage Patterns

- Constructed and passed to cache lookup or storage functions within `clore::support`.
- Likely compared or hashed to uniquely identify compiled module signatures.

### `clore::support::TransparentStringEqual`

Declaration: `support/logging.cppm:33`

Definition: `support/logging.cppm:33`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The struct `clore::support::TransparentStringEqual` defines a set of four `operator()` overloads that perform equality comparison between `std::string` and `std::string_view` in any combination. Each overload is `noexcept` and returns `bool`. To avoid duplication and unnecessary construction, mixed-type overloads convert the `std::string` operand to a `std::string_view` before comparing (via `std::string_view{lhs}` or `std::string_view{rhs}`), ensuring that only a view is formed and no copy of the underlying character data is made. The `is_transparent` type alias (defined as `void`) marks the functor as transparent, allowing standard associative containers (e.g., `std::set`, `std::unordered_set`) to perform heterogeneous lookups using `std::string_view` keys without requiring temporary `std::string` objects. The struct is trivially default constructible and contains no data members, making it cheap to pass by value.

#### Invariants

- All overloads are `noexcept` and return `bool`.
- Comparison is consistent with `operator==` for `std::string` and `std::string_view`.
- No mutable state is stored; the functor is stateless.
- Equal strings are guaranteed to compare equal regardless of argument types.

#### Key Members

- `using is_transparent = void`
- `operator()(std::string_view, std::string_view) const noexcept`
- `operator()(const std::string&, std::string_view) const noexcept`
- `operator()(std::string_view, const std::string&) const noexcept`
- `operator()(const std::string&, const std::string&) const noexcept`

#### Usage Patterns

- Passed as the comparison key to `std::set`, `std::map`, `std::unordered_set`, or `std::unordered_map` to enable lookup with `std::string_view` without constructing `std::string` temporaries.
- Used in heterogeneous lookup scenarios where keys are stored as `std::string` but lookup is performed with `std::string_view`.

#### Member Types

##### `clore::support::TransparentStringEqual::is_transparent`

Declaration: `support/logging.cppm:34`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
using is_transparent = void
```

#### Member Functions

##### `clore::support::TransparentStringEqual::operator()`

Declaration: `support/logging.cppm:46`

Definition: `support/logging.cppm:46`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
[[nodiscard]] auto operator()(std::string_view lhs, const std::string& rhs) const noexcept
        -> bool {
        return lhs == std::string_view{rhs};
    }
```

##### `clore::support::TransparentStringEqual::operator()`

Declaration: `support/logging.cppm:36`

Definition: `support/logging.cppm:36`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
[[nodiscard]] auto operator()(std::string_view lhs, std::string_view rhs) const noexcept
        -> bool {
        return lhs == rhs;
    }
```

##### `clore::support::TransparentStringEqual::operator()`

Declaration: `support/logging.cppm:51`

Definition: `support/logging.cppm:51`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
[[nodiscard]] auto operator()(const std::string& lhs, const std::string& rhs) const noexcept
        -> bool {
        return lhs == rhs;
    }
```

##### `clore::support::TransparentStringEqual::operator()`

Declaration: `support/logging.cppm:41`

Definition: `support/logging.cppm:41`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
[[nodiscard]] auto operator()(const std::string& lhs, std::string_view rhs) const noexcept
        -> bool {
        return std::string_view{lhs} == rhs;
    }
```

### `clore::support::TransparentStringHash`

Declaration: `support/logging.cppm:17`

Definition: `support/logging.cppm:17`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The struct `clore::support::TransparentStringHash` is implemented as an empty, stateless functor with no data members. Its core invariant is that all hashing is performed exclusively through `std::hash<std::string_view>`. The single source of truth for hashing is the `operator()` overload that takes `std::string_view`; it directly invokes `std::hash<std::string_view>` on the given view. The other two overloads—for `const std::string&` and `const char*`—are forwarding adapters. Each one converts its argument to a `std::string_view` and then delegates to the `std::string_view` overload via `(*this)(std::string_view{value})`. This design ensures that the hash value of any string-like argument is identical to the hash of a `std::string_view` containing the same character sequence. The `is_transparent` alias is defined as `void`, which together with the heterogeneous `operator()` overloads enables heterogeneous lookup in unordered associative containers, allowing key lookups using `std::string_view` or `const char*` without constructing a temporary `std::string` object.

#### Invariants

- All `operator()` overloads are noexcept
- Equal string inputs produce equal hash values
- Delegates exclusively to `std::hash<std::string_view>`

#### Key Members

- `is_transparent`
- `operator()(``std::string_view`)
- `operator()(`const `std::string`&)
- `operator()(`const char*)

#### Usage Patterns

- Used as the Hash template parameter in `std::unordered_set` or `std::unordered_map` for string keys
- Enables lookups with `std::string_view`, `std::string`, or const char* without constructing a key type

#### Member Types

##### `clore::support::TransparentStringHash::is_transparent`

Declaration: `support/logging.cppm:18`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
using is_transparent = void
```

#### Member Functions

##### `clore::support::TransparentStringHash::operator()`

Declaration: `support/logging.cppm:24`

Definition: `support/logging.cppm:24`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
[[nodiscard]] auto operator()(const std::string& value) const noexcept -> std::size_t {
        return (*this)(std::string_view{value});
    }
```

##### `clore::support::TransparentStringHash::operator()`

Declaration: `support/logging.cppm:20`

Definition: `support/logging.cppm:20`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
[[nodiscard]] auto operator()(std::string_view value) const noexcept -> std::size_t {
        return std::hash<std::string_view>{}(value);
    }
```

##### `clore::support::TransparentStringHash::operator()`

Declaration: `support/logging.cppm:28`

Definition: `support/logging.cppm:28`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
[[nodiscard]] auto operator()(const char* value) const noexcept -> std::size_t {
        return (*this)(std::string_view{value});
    }
```

## Variables

### `clore::logging::debug`

Declaration: `support/logging.cppm:125`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

Used to emit log messages at debug level. The proxy interacts with the global log level `clore::logging::g_log_level` to conditionally output messages, and supports formatting via `fmt` and `args`.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- used to log debug messages
- invoked with format string and arguments

### `clore::logging::err`

Declaration: `support/logging.cppm:128`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

This proxy is invoked by calling it with a format string and optional arguments (e.g., `err("error occurred: {}", code)`), which forwards the log message to the underlying logging system at the error level. It is never mutated; its role is purely as a callable log sink.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- logged via call `operator` with format string and arguments

### `clore::logging::g_log_level`

Declaration: `support/logging.cppm:102`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

It is read by `clore::logging::log` and `clore::logging::stderr_logger` to determine whether a fixed log level should be applied, bypassing the default level logic.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Read by `clore::logging::log` to check for a global log level override
- Read by `clore::logging::stderr_logger` to check for a global log level override

### `clore::logging::info`

Declaration: `support/logging.cppm:126`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

This variable provides a compile-time interface for issuing info-level log entries. Being `constexpr`, its value is fixed at compile time and it is not mutated. It is typically used with function-call syntax to log formatted messages via `spdlog` at the info level.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- used as a logging proxy for info level messages

### `clore::logging::trace`

Declaration: `support/logging.cppm:124`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

It serves as a constant proxy for emitting log messages at the `trace` severity level, providing a type-safe interface for logging.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- emitting trace-level log messages

### `clore::logging::warn`

Declaration: `support/logging.cppm:127`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

It acts as a compile-time constant logger handle for warning severity, used in functions like `clore::support::enable_utf8_console` to emit warning log output without runtime overhead.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- used as a logging proxy for warning messages

## Functions

### `clore::logging::cache_hit_rate`

Declaration: `support/logging.cppm:138`

Definition: `support/logging.cppm:138`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

The function first computes `total` as the sum of `hits` and `misses`. If `total` is zero, it throws a `std::logic_error` with a message constructed via `std::format` that includes the `name` parameter. Otherwise, it calculates `rate` as `(static_cast<double>(hits) * 100.0) / static_cast<double>(total)` and logs the result using `clore::logging::info`, passing the formatted string containing the cache name, hit count, miss count, and the computed percentage to one decimal place.  

Internally, the function depends on `std::format` for string formatting, `clore::logging::info` (a `clore::logging::LogProxy` instance) for output, and the `std::logic_error` exception class for error signaling. No other support functions from the `clore::support` namespace are invoked. The control flow is linear: validate input, compute the percentage, and emit the log message.

#### Side Effects

- Logs a formatted message (via `info`) to the logging system
- May throw `std::logic_error` if total is zero

#### Reads From

- Parameter `name`
- Parameter `hits`
- Parameter `misses`

#### Writes To

- Log output (via `info`)

#### Usage Patterns

- Called to report cache hit/miss statistics
- Typically used in performance monitoring sections

### `clore::logging::log`

Declaration: `support/logging.cppm:104`

Definition: `support/logging.cppm:104`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

Implementation: [Implementation](functions/log.md)

The implementation of `clore::logging::log` applies a two-step dispatch: it first checks the global filter `clore::logging::g_log_level`, and only proceeds to the underlying logger if the requested severity meets the threshold. When `g_log_level` has a value and the incoming `lvl` is strictly less than that stored level, the function returns immediately, performing no I/O or formatting. Otherwise, it forwards the call directly to `spdlog::default_logger_raw()->log`, passing `lvl` and formatting the `msg` argument as the sole format string parameter (`"{}"`). This avoids the overhead of constructing a formatted output when the message would be suppressed.

The only external dependency is the `spdlog` library for the logger instance and level type. The filtering state is held in the file‑scoped variable `clore::logging::g_log_level`, which is a `std::optional<spdlog::level::level_enum>`. No additional string processing, caching, or level‑name resolution is performed inside `log`; those responsibilities are delegated to callers (e.g., `LogProxy::operator()`) or to other functions in the `clore::logging` namespace.

#### Side Effects

- Logs a message to the spdlog default logger output.

#### Reads From

- `g_log_level` global
- `lvl` parameter
- `msg` parameter

#### Writes To

- spdlog default logger output

#### Usage Patterns

- called by `LogProxy::operator()(std::string_view)`
- used directly for logging with an explicit level

### `clore::logging::stderr_logger`

Declaration: `support/logging.cppm:130`

Definition: `support/logging.cppm:130`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

The function `clore::logging::stderr_logger` initialises a spdlog logger that writes coloured output to the standard error stream. It begins by invoking `spdlog::stderr_color_mt` with the provided `name` to create a thread-safe, multi‑threaded logger instance. The freshly created logger is then installed as the global default via `spdlog::set_default_logger`. Afterwards, if the module‑level variable `g_log_level` holds a value (an `std::optional` of `spdlog::level::level_enum`), the logger’s severity threshold is adjusted by calling `spdlog::set_level` with that value. The implementation has no branching beyond the optional level assignment and relies entirely on the spdlog library for underlying sink creation, formatting, and output.

#### Side Effects

- Creates or retrieves a spdlog stderr color logger with the given name
- Sets that logger as the default for all subsequent spdlog calls
- Possibly sets the global spdlog log level based on `g_log_level`

#### Reads From

- `name` parameter (`std::string_view`)
- `g_log_level` (global `std::optional<spdlog::level::level_enum>`)

#### Writes To

- spdlog internal default logger state
- spdlog internal global log level (conditionally)

#### Usage Patterns

- Called at startup to configure stderr logging with a specific logger name
- Used to switch the default logger to stderr output
- Invocations optionally apply a previously stored log level

### `clore::support::build_cache_key`

Declaration: `support/logging.cppm:70`

Definition: `support/logging.cppm:368`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function concatenates the caller-supplied `normalized_path` and the decimal representation of `compile_signature` separated by the anonymous-namespace constant `kCacheKeyDelimiter`. It pre-reserves storage equal to the sum of the path length, one byte for the delimiter, and twenty additional bytes (enough for a 64‑bit integer in decimal) to avoid reallocations during construction. After appending the path and the delimiter, it converts `compile_signature` via `std::to_string` and appends the result, then returns the assembled `std::string`. The implementation has no branching or external dependencies beyond the delimiter constant and the standard conversion utility.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `normalized_path` parameter
- `compile_signature` parameter
- `kCacheKeyDelimiter` constant

#### Usage Patterns

- Used to form keys for caching compiled results
- Called when building a cache entry identifier

### `clore::support::build_compile_signature`

Declaration: `support/logging.cppm:66`

Definition: `support/logging.cppm:352`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function constructs a single hash-based compile signature by concatenating normalized directory, normalized file path, and all compiler arguments into a flat null-byte-delimited payload. It first reserves a conservative capacity, then appends the result of `clore::support::normalize_path_string` applied to the input `directory`, followed by a null byte, then the `normalized_file` and another null byte. Each element of the `arguments` vector is appended with a trailing null byte in order. The accumulated payload is hashed using `llvm::xxh3_64bits` to produce the final `std::uint64_t` signature. The only explicit dependency called is `normalize_path_string`; the rest of the logic is a straightforward linear concatenation without branching or error handling.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `directory`
- parameter `normalized_file`
- parameter `arguments`
- callee `normalize_path_string(directory)`

#### Usage Patterns

- generating a hash-based signature for compile options
- used in caching or deduplication of compilation results

### `clore::support::canonical_log_level_name`

Declaration: `support/logging.cppm:77`

Definition: `support/logging.cppm:424`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The implementation normalizes the input by converting every character to lowercase via `std::tolower`, then hands the result to `spdlog::level::from_str`. If the input is empty it immediately returns `std::nullopt`. Because `spdlog::level::from_str` returns `spdlog::level::off` for both the literal `"off"` and for any unrecognised string, the function performs an explicit check: when the returned level equals `spdlog::level::off` and the normalized string is not `"off"`, the result is considered invalid and `std::nullopt` is returned. Otherwise the normalized (lowercased) string is returned inside `std::optional<std::string>`. This design relies on the spdlog library’s log level parser and on standard library character conversion; no external containers or file I/O are involved.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- input parameter `value`

#### Usage Patterns

- validate log level name
- normalize log level to lowercase
- used before setting log level

### `clore::support::enable_utf8_console`

Declaration: `support/logging.cppm:91`

Definition: `support/logging.cppm:534`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The implementation of `clore::support::enable_utf8_console` is conditionally compiled for Windows only, guarded by the `_WIN32` preprocessor macro. Under that platform, it invokes `SetConsoleCP` with `CP_UTF8` to set the input code page and `SetConsoleOutputCP` with `CP_UTF8` to set the output code page. Each call’s return value is checked; if either fails (returns zero), a warning message is logged via `clore::logging::warn`, including the system error code obtained from `GetLastError`. The function is a thin wrapper around these two Windows API calls, with no additional algorithmic logic or data manipulation.

#### Side Effects

- Changes the console input code page to UTF-8
- Changes the console output code page to UTF-8
- Logs warning messages when API calls fail

#### Writes To

- Windows console input code page
- Windows console output code page
- Log output (via `clore::logging::warn`)

#### Usage Patterns

- Called during program initialization to enable UTF-8 console support on Windows

### `clore::support::ensure_utf8`

Declaration: `support/logging.cppm:75`

Definition: `support/logging.cppm:405`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

Implementation: [Implementation](functions/ensure-utf8.md)

The function `clore::support::ensure_utf8` iterates over the input `text` character‑by‑character using an `offset` index. At each position it invokes `valid_utf8_sequence_length` to determine the byte length of a valid UTF‑8 sequence starting there. If that length is zero — indicating an invalid leading byte or misplaced continuation byte — the algorithm appends the constant `kUtf8Replacement` (the Unicode replacement character U+FFFD encoded in UTF‑8) to `normalized` and advances `offset` by one. Otherwise it appends the identified valid sequence via `text.substr(offset, sequence_length)` and increments `offset` by that `sequence_length`. The result is a `std::string` that is guaranteed to contain only well‑formed UTF‑8, with any malformed bytes replaced by the standard replacement character.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `text` parameter

#### Writes To

- local `normalized` string (returned by value)

#### Usage Patterns

- called by `write_utf8_text_file` to ensure output is valid UTF-8
- called by `truncate_utf8` to sanitize input before truncation

### `clore::support::extract_first_plain_paragraph`

Declaration: `support/logging.cppm:62`

Definition: `support/logging.cppm:303`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The algorithm reads the input Markdown line by line using a `std::istringstream` and tracks a `in_code_block` flag. For each line, whitespace is trimmed from both ends. Lines that begin or end a code fence (triple backticks) toggle the flag and are skipped; lines inside a code block are also skipped. An empty trimmed line acts as a paragraph separator: if a paragraph has already been accumulated, the loop breaks, otherwise the empty line is ignored. Lines starting with block-level constructs—heading (`#`), blockquote (`>`), table (`|`), or list markers (`- `, `* `)—also cause early termination if a paragraph is present, or are skipped otherwise. All other non‑empty lines are concatenated into a `paragraph` string, separated by a single space. After the loop, the accumulated paragraph is passed to the helper function `clore::support::(anonymous namespace)::strip_inline_markdown_text` which removes inline Markdown formatting from the extracted text.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `markdown` parameter
- the `strip_inline_markdown_text` function

#### Usage Patterns

- extracting the first paragraph from Markdown documentation
- obtaining a plain text summary from Markdown strings

### `clore::support::normalize_line_endings`

Declaration: `support/logging.cppm:79`

Definition: `support/logging.cppm:442`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The algorithm performs a single linear pass over the input `text`, building a new `std::string normalized` with a pre-allocated capacity equal to the input size. For each character, it checks whether the current byte is a carriage return (`\r`). If so, it unconditionally pushes a line feed (`\n`) into the output; if the following byte is also a line feed (`\n`), it skips that byte by incrementing the index, effectively collapsing a CRLF pair into a single LF. All other characters are copied unchanged. The function returns the resulting `normalized` string. No external dependencies are used beyond the C++ standard library.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the input `string_view` parameter `text`

#### Writes To

- the returned `std::string` object

#### Usage Patterns

- normalize line endings for text processing
- prepare strings for hash or comparison ignoring line ending variations

### `clore::support::normalize_path_string`

Declaration: `support/logging.cppm:64`

Definition: `support/logging.cppm:348`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The implementation of `clore::support::normalize_path_string` defers entirely to the C++ standard library. It constructs a temporary `std::filesystem::path` from the input `path`, invokes `lexically_normal()` on that object to collapse redundant separators and resolve `"."` / `".."` components according to lexical rules, and finally converts the result to a `generic_string()` — that is, a string using forward slashes as the path separator regardless of the operating system convention. No additional validation or transformation is performed; the function returns the normalized path as a `std::string` value. Its only dependency is the `<filesystem>` header provided by the C++ standard library.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `path` parameter (`std::string_view`)

#### Usage Patterns

- used by `clore::support::build_compile_signature` to normalize path arguments before hashing

### `clore::support::read_utf8_text_file`

Declaration: `support/logging.cppm:85`

Definition: `support/logging.cppm:480`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function `clore::support::read_utf8_text_file` opens the specified `std::filesystem::path` as a binary input stream (`std::ifstream` with `std::ios::binary`). After confirming the file opened successfully, it attempts to pre‑reserve storage by querying `std::filesystem::file_size`; if no error occurs, it calls `content.reserve` with the file size, sized to `std::size_t`. The file is then read in buffered chunks of 8192 bytes via repeated `file.read` calls, appending each chunk’s content to `std::string content` using `std::ifstream::gcount` to determine the number of bytes actually read. Any `bad()` or non‑end‑of‑file `fail()` condition on the stream after reading causes an `std::unexpected` error return, describing the failure with `std::format`.

After a successful read, the function invokes `clore::support::strip_utf8_bom` on the accumulated `content`. If the resulting `std::string_view` has the same size as the original `content`, no BOM was present and the original `content` is returned; otherwise a new `std::string` constructed from the stripped view is returned, effectively discarding any leading UTF‑8 BOM.

#### Side Effects

- reads a file from the filesystem
- allocates memory for the string

#### Reads From

- the `path` parameter of type `const std::filesystem::path&`
- the file contents via `std::ifstream`
- file size via `std::filesystem::file_size`

#### Writes To

- the returned `std::string` content
- local variable `content`
- local variable `normalized`
- local variable `chunk` buffer

#### Usage Patterns

- used to load text files for processing
- used where UTF-8 BOM stripping is required
- error handling via `std::expected`

### `clore::support::split_cache_key`

Declaration: `support/logging.cppm:73`

Definition: `support/logging.cppm:378`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function locates the last occurrence of `kCacheKeyDelimiter` in the input `cache_key` using `std::string_view::rfind`. If the delimiter is not found, it returns `std::unexpected` with a formatted error message. Otherwise, it splits the string into `path_part` and `signature_part` via `substr`. Both parts must be non‑empty; if either is empty an error is returned. The `signature_part` is parsed as a `std::uint64_t` using `std::from_chars`. The conversion must succeed and consume the entire signature substring; otherwise an error is returned. On success the function constructs a `CacheKeyParts` object, setting the `.path` field from `path_part` and the `.compile_signature` field from the parsed integer value, and returns it in a `std::expected` value.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `cache_key` parameter
- `kCacheKeyDelimiter` constant

#### Writes To

- return value of type `std::expected<CacheKeyParts, std::string>`

#### Usage Patterns

- parsing a combined cache key into its path and signature components
- validating cache key format before further processing

### `clore::support::strip_utf8_bom`

Declaration: `support/logging.cppm:83`

Definition: `support/logging.cppm:470`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

Implementation: [Implementation](functions/strip-utf8-bom.md)

The function checks whether the input `std::string_view text` begins with the UTF‑8 byte‑order mark (BOM) stored in the constant `kUtf8Bom`. It first verifies that `text.size()` is at least `std::size(kUtf8Bom)` (three bytes) and then compares each of the first three bytes using `static_cast<unsigned char>` to avoid sign extension. If all three match, it returns the substring starting after the BOM; otherwise it returns the original `text` unchanged. No additional helpers or external dependencies are required; the only dependency is the constant `kUtf8Bom` defined in the same anonymous namespace.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `text`
- constant `kUtf8Bom`

#### Usage Patterns

- called by `clore::support::read_utf8_text_file` to strip BOM from file contents

### `clore::support::topological_order`

Declaration: `support/logging.cppm:93`

Definition: `support/logging.cppm:547`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The implementation of `clore::support::topological_order` performs a Kahn-style topological sort over the given `nodes` using the supplied `reverse_edges` map and initial `in_degree` counts. It begins by constructing a `std::set<std::string> ready` containing every node whose in-degree is either missing from `in_degree` or equal to zero, ensuring lexicographic ordering of the frontier. In each iteration, the smallest element is removed from `ready`, appended to the result `order`, and its dependents (retrieved from `reverse_edges`) have their in-degree decremented. Any dependent whose in-degree becomes zero is inserted into `ready`. If the loop exhausts `ready` before all nodes have been processed, a cycle exists and the function returns `std::nullopt`; otherwise it returns the completed `order`. The algorithm depends only on standard library containers and performs no I/O or logging, making it a pure computational utility.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- nodes
- `reverse_edges`
- `in_degree`

#### Usage Patterns

- Topological ordering of dependencies
- Cycle detection in directed graphs
- Build system task scheduling

### `clore::support::truncate_utf8`

Declaration: `support/logging.cppm:81`

Definition: `support/logging.cppm:460`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function begins by invoking `clore::support::ensure_utf8` on the input `text` to obtain a normalized and valid UTF‑8 string. If the normalized string’s byte count is already less than or equal to `max_bytes`, the function returns that string directly. Otherwise, it calls `clore::support::(anonymous namespace)::utf8_prefix_length` on the normalized string and `max_bytes` to compute the largest byte offset that terminates on a complete UTF‑8 character boundary. The normalized string is then resized to that offset and returned. No explicit iteration over code points is performed – the prefix‑length helper isolates the correct truncation point, and `ensure_utf8` ensures the input is well‑formed before any truncation takes place.

#### Side Effects

- allocates memory for the returned `std::string`
- may allocate memory within `ensure_utf8` call

#### Reads From

- `text` parameter
- `max_bytes` parameter

#### Usage Patterns

- truncating UTF-8 text to a byte limit without breaking multi-byte characters
- normalizing and truncating input lengths for logging or display

### `clore::support::write_utf8_text_file`

Declaration: `support/logging.cppm:88`

Definition: `support/logging.cppm:515`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The implementation of `clore::support::write_utf8_text_file` first normalizes the input `std::string_view content` by calling `clore::support::ensure_utf8`, which ensures the text is valid and consistently encoded UTF‑8. It then attempts to open a binary `std::ofstream` at the given `std::filesystem::path`. If the file cannot be opened, the function returns `std::unexpected` with an error message produced via `std::format`. On success, it writes the normalized bytes using `file.write`, flushes the stream, and checks the stream state. A failed write also yields a `std::unexpected` error. If both steps succeed, a default‑constructed `std::expected<void, std::string>` (containing no error) is returned.

The internal control flow is linear: normalize, open, write, flush, validate. The only dependency is `clore::support::ensure_utf8`, which is called exactly once per invocation. No other functions from the supporting modules (e.g., logging or cache routines) are involved in this writing path.

#### Side Effects

- Writes to a file on disk
- Creates or overwrites the specified file
- Performs file I/O operations

#### Reads From

- `path` parameter: the file path to write to
- `content` parameter: the string content to write
- Filesystem state for file creation

#### Writes To

- The file specified by `path` on disk

#### Usage Patterns

- Used to write UTF-8 text files after content normalization
- Provides error handling for file write failures

## Internal Structure

The `support` module is the lowest‑level foundation in the project, providing self‑contained utility functions and type adaptors. It imports only the standard library (`std`) and has no dependencies on other project modules, making it a leaf dependency. Internally it is partitioned into two distinct namespaces: `clore::support` and `clore::logging`. The `clore::support` namespace groups core text‑processing and polymorphic key‑handling facilities — including UTF‑8 BOM stripping, line‑ending normalization, inline Markdown removal, transparent hash/equal functors for heterogeneous lookup, path normalization, and file I/O for UTF‑8 files. The `clore::logging` namespace builds on top of these primitives to offer a lightweight, template‑based logging proxy (`LogProxy`) specialized at compile‑time for each severity level (trace, debug, info, warn, err), a global log‑level override, and a central `log()` dispatcher. This two‑tier layout keeps logging infrastructure separate from general‑purpose utilities while ensuring that all logging code can reuse the string‑processing and normalization routines provided by the `support` layer. The module’s implementation structure is flat, with no internal sub‑modules or hidden dependencies, which enforces a clear, acyclic dependency order and simplifies reuse across the rest of the codebase.

