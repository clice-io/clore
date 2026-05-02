---
title: 'Module support'
description: 'The support module provides a collection of foundational utilities and infrastructure that underpin text processing, logging, and caching within the broader system. Its core responsibility is to offer robust, cross‑platform primitives for handling UTF‑8 encoded data, normalizing paths and line endings, constructing deterministic cache keys, and managing transparent string lookup for unordered containers. The module also houses the central logging framework, including configurable log levels, typed log proxies for compile‑time severity dispatch, and helper functions for reporting cache hit rates.'
layout: doc
template: doc
---

# Module `support`

## Summary

The `support` module provides a collection of foundational utilities and infrastructure that underpin text processing, logging, and caching within the broader system. Its core responsibility is to offer robust, cross‑platform primitives for handling UTF‑8 encoded data, normalizing paths and line endings, constructing deterministic cache keys, and managing transparent string lookup for unordered containers. The module also houses the central logging framework, including configurable log levels, typed log proxies for compile‑time severity dispatch, and helper functions for reporting cache hit rates.

The public‑facing implementation encompasses a set of well‑defined functions and types. In the `clore::support` namespace, it exposes transparent hash and equality functors (`TransparentStringHash`, `TransparentStringEqual`), data structures for cache key decomposition (`CacheKeyParts`), and a suite of UTF‑8 utilities for reading, writing, truncating, and validating text files. Path normalization (`normalize_path_string`), line ending normalization, BOM stripping, and canonical log level resolution are also part of this scope. The `clore::logging` namespace contributes a global log level (`g_log_level`), a generic logging function (`log`), a direct‑to‑stderr sink (`stderr_logger`), and constexpr `LogProxy` instances for each severity level (`trace`, `debug`, `info`, `warn`, `err`), which serve as the primary entry points for emitting log messages throughout the application.

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

The template struct `clore::logging::LogProxy` captures a compile‑time log level via its non‑type template parameter `spdlog::level::level_enum Level`; it has no data members. Its two `operator()` overloads constitute the entire implementation. The first overload accepts a `std::string_view` message and directly delegates to the free function `clore::logging::log(Level, msg)`. The second overload is a constrained function template that accepts a `std::format_string<Args...> fmt` followed by a variadic argument pack `Args&&... args` (the constraint requires `sizeof...(Args) > 0`); it formats the message using `std::format` and then passes the resulting string and the level to the same `clore::logging::log`. This design ensures that every invocation goes through the central logging point while keeping the level fixed at compile time, eliminating runtime dispatch on the level inside the proxy itself. The internal invariant is that the level is encoded in the type, and all logging logic is deferred to the underlying `log` function.

#### Invariants

- The logging level `Level` is fixed at compile time via a non-type template parameter.
- All messages are forwarded to a free function `log` that must be defined in the surrounding scope.
- The variadic overload requires at least one argument (`sizeof...(Args) > 0`).

#### Key Members

- `void operator()(std::string_view msg) const`
- `void operator()(std::format_string<Args...> fmt, Args&&... args) const` (template, requires `sizeof...(Args) > 0`)

#### Usage Patterns

- Instantiated with a specific log level (e.g., `LogProxy<spdlog::level::info>`) to create a level‑specific logger object.
- Called with either a plain string or a format string and arguments to produce a formatted log message at the predetermined level.

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

The struct `clore::support::CacheKeyParts` is an internal aggregate that bundles a source file path and a compile‑time signature for use as a cache key. Its two data members are the `std::string` field `path` and the `std::uint64_t` field `compile_signature`, the latter default‑initialized to zero. A zero `compile_signature` conventionally represents an unset or invalid signature; the struct does not enforce this invariant through member functions. No constructors, assignment `operator`s, or other special members are user‑declared, so the type relies entirely on compiler‑generated defaults, and all initialization and copying are performed via aggregate initialization or member‑wise operations.

#### Invariants

- `compile_signature` defaults to `0` if not explicitly set.
- `path` is a `std::string` with no additional constraints implied by the evidence.

#### Key Members

- `path`
- `compile_signature`

#### Usage Patterns

- Defined as a fundamental part of cache key representation within the logging module.
- Expected to be aggregated into a larger cache key or used directly to identify compiled artifacts.

### `clore::support::TransparentStringEqual`

Declaration: `support/logging.cppm:33`

Definition: `support/logging.cppm:33`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The internal structure of `clore::support::TransparentStringEqual` is minimal: it defines the public alias `is_transparent` as `void` to enable heterogeneous lookup in compatible containers, and provides four overloads of `operator()`. Each overload compares two string-like arguments, normalizing all inputs to `std::string_view` before performing equality. The overload taking two `std::string` arguments delegates to the equality `operator` of `std::string` directly, while the three mixed‑type variants convert a `const std::string&` to `std::string_view` via its implicit conversion, ensuring all comparisons are performed on `std::string_view` for consistency. All call `operator`s are marked `noexcept` and `[[nodiscard]]`, and they maintain the invariant that equal character sequences are always considered equal regardless of their original storage type.

#### Invariants

- Provides equality comparison for strings
- Supports heterogeneous lookup via `is_transparent`
- All `operator()` overloads are `noexcept`

#### Key Members

- `is_transparent` type alias
- Four `operator()` overloads (each combination of `std::string_view` and `const std::string&`)

#### Usage Patterns

- Used as a comparator in associative containers to enable transparent lookup
- Allows efficient searching with `std::string_view` without constructing temporary `std::string` objects

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

The implementation of `clore::support::TransparentStringHash` is built around a single, authoritative hashing path: the `operator()` overload that accepts a `std::string_view`. This overload directly invokes `std::hash<std::string_view>`, ensuring a consistent hash computation for all input types. The remaining two overloads—those taking `const std::string&` and `const char*`—are thin wrappers that explicitly convert their argument to a `std::string_view` and then forward the call to the primary overload. This design guarantees that no matter which input form is used, the same hashing logic applies, eliminating duplication and reducing the risk of accidental inconsistency. The typedef `is_transparent` is set to `void`, which enables heterogeneous lookup in unordered associative containers when combined with a transparent key equality comparator. All overloads are declared `noexcept` and return `std::size_t`, so the functor can be used in contexts that require exception safety and a standard hash result type.

#### Invariants

- Hash values are identical to `std::hash<std::string_view>` for equivalent string content
- All three `operator()` overloads produce the same hash for equal string content
- The `is_transparent` type alias enables heterogeneous lookup in unordered containers

#### Key Members

- `is_transparent`
- `operator()(std::string_view)`
- `operator()(const std::string&)`
- `operator()(const char*)`

#### Usage Patterns

- Used as the hash functor in `std::unordered_set` or `std::unordered_map` with transparent key equality
- Enables lookup with `std::string_view` or `const char*` without constructing a `std::string`
- Serves as a building block for string-based associative containers that require heterogeneous access

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

`debug` is a compile-time `constexpr inline` instance of the `LogProxy` template specialized for `spdlog::level::debug`. It is consumed as a callable proxy that forwards log requests at the debug severity, sitting alongside peer proxies such as `clore::logging::trace`, `clore::logging::info`, `clore::logging::warn`, and `clore::logging::err`. Because it is `constexpr inline`, it carries no mutable state and serves purely as a routing handle to the underlying logging facility.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- invoked as a log proxy for `spdlog::level::debug` messages
- used alongside other severity-level proxies in `clore::logging`

### `clore::logging::err`

Declaration: `support/logging.cppm:128`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

The variable serves as a statically-typed logging interface for error severity. Since it is `constexpr`, its value is determined at compile time and it is not mutated during program execution. The provided evidence does not show any subsequent reads or mutation of this variable beyond its declaration.

#### Mutation

No mutation is evident from the extracted code.

### `clore::logging::g_log_level`

Declaration: `support/logging.cppm:102`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

This variable is used by `clore::logging::log` and `clore::logging::stderr_logger` to determine whether a log message should be emitted based on its severity level. When set, only messages at or above the specified level are processed; when unset, the filtering behavior may be defined elsewhere.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- read by `clore::logging::log` to decide message output
- read by `clore::logging::stderr_logger` to control logging behavior

### `clore::logging::info`

Declaration: `support/logging.cppm:126`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

As a `constexpr inline` proxy specialized on `spdlog::level::info`, it is consumed as a stateless dispatcher that forwards formatted messages to the underlying logger at info severity. It is referenced by `clore::logging::cache_hit_rate`, which uses it to emit informational diagnostics such as cache hit-rate statistics, alongside sibling proxies like `debug`, `trace`, `warn`, and `err`.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- invoked from `clore::logging::cache_hit_rate` to report informational messages
- used as a severity-tagged dispatcher built on `LogProxy<spdlog::level::info>`
- serves as the info-level counterpart to other `LogProxy` instances in the namespace

### `clore::logging::trace`

Declaration: `support/logging.cppm:124`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

`trace` is a stateless proxy object specialized on `spdlog::level::trace`, used as the public handle for trace-severity logging within the `clore::logging` namespace. It sits alongside peer proxies such as `debug`, `info`, `warn`, and `err`, forming a uniform set of severity-tagged log entry points that callers invoke to dispatch formatted messages at the corresponding level.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- invoked as a namespace-scope entry point for trace-level logging
- parallels other severity proxies like `debug`, `info`, `warn`, and `err`

### `clore::logging::warn`

Declaration: `support/logging.cppm:127`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

`warn` is a `constexpr inline` instance of `LogProxy<spdlog::level::warn>`, providing a callable entry point for emitting log records at the `spdlog::level::warn` severity. It is referenced by callers such as `clore::support::enable_utf8_console` to dispatch warning messages through the underlying logger, alongside sibling proxies like `debug`, `trace`, `info`, and `err`.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- invoked as a logging entry point for warning-level messages
- used by `clore::support::enable_utf8_console`
- parallels other level proxies (`debug`, `trace`, `info`, `err`) in the namespace

## Functions

### `clore::logging::cache_hit_rate`

Declaration: `support/logging.cppm:138`

Definition: `support/logging.cppm:138`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

The function first computes the total as the sum of the `hits` and `misses` arguments. If `total` is zero, a `std::logic_error` is thrown with a formatted message that includes the `name` parameter, ensuring the cache hit rate is only computed for a non-empty cache. Otherwise, the hit rate is calculated as a percentage: `(static_cast<double>(hits) * 100.0) / static_cast<double>(total)`. The result is then passed to the logging proxy `clore::logging::info` along with `name`, `hits`, `misses`, and the computed `rate` (formatted to one decimal place).  

The function depends on `std::format` for string formatting, `std::logic_error` for error reporting, and the `clore::logging::info` variable (an instance of `clore::logging::LogProxy`) to emit the log message. No external I/O or complex data structures are involved; the control flow is linear with an early error exit for the zero-total case.

#### Side Effects

- throws `std::logic_error` on zero total
- logs via `info`

#### Reads From

- parameter `name`
- parameter `hits`
- parameter `misses`

#### Writes To

- logging output via `info`

#### Usage Patterns

- reporting cache hit rates
- logging after cache lookups

### `clore::logging::log`

Declaration: `support/logging.cppm:104`

Definition: `support/logging.cppm:104`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

Implementation: [Implementation](functions/log.md)

The function `clore::logging::log` implements a level‑gated dispatch to the spdlog library. Its control flow begins by checking whether the module‑level global `g_log_level` has been set (via its `std::optional` state). If a threshold exists and the incoming `lvl` is strictly less than that threshold, the function returns immediately without performing any logging work. This early‑exit optimization avoids unnecessary formatting and I/O when the message would be filtered out by the current log level configuration. When the level passes the gate, the call is forwarded to `spdlog::default_logger_raw()->log(lvl, "{}", msg)`, which writes the formatted message through the default spdlog logger. No additional buffering or transformation is applied; the raw `spdlog::level::level_enum` and the `std::string_view` message are passed directly. The function therefore depends on `g_log_level` (a `std::optional<spdlog::level::level_enum>` defined in the same `clore::logging` namespace) and on the spdlog runtime API for the actual output. Its behaviour is fully synchronous and non‑allocating for the threshold check; allocation may occur inside spdlog depending on the sink configuration.

#### Side Effects

- Writes a log record through `spdlog::default_logger_raw()` at the specified severity level

#### Reads From

- parameter `lvl`
- parameter `msg`
- global optional `g_log_level`
- the `spdlog` default logger via `spdlog::default_logger_raw()`

#### Writes To

- the `spdlog` default logger's output sinks

#### Usage Patterns

- Invoked by `clore::logging::LogProxy::operator()(std::string_view)` to route formatted messages to `spdlog`
- Used as the underlying logging primitive that respects the `g_log_level` threshold

### `clore::logging::stderr_logger`

Declaration: `support/logging.cppm:130`

Definition: `support/logging.cppm:130`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

The implementation of `clore::logging::stderr_logger` delegates to the `spdlog` library. It calls `spdlog::stderr_color_mt` with the provided `name` (converted to `std::string`) to either create or retrieve a thread-safe, color-capable logger that writes to standard error. That logger is then promoted to the global default via `spdlog::set_default_logger`. After establishing the default, the function checks the optional global variable `g_log_level`; if it contains a value, that level is applied to all loggers through `spdlog::set_level`. This single‑point level override ensures that any threshold configured earlier (e.g., from an environment variable or command‑line argument) is respected immediately, without requiring callers to manage per‑logger levels individually.

#### Side Effects

- Creates a new spdlog logger instance
- Sets the global default spdlog logger
- Sets the spdlog log level if `g_log_level` has a value

#### Reads From

- `name` function parameter
- `g_log_level` global variable

#### Writes To

- Global spdlog default logger
- Global spdlog log level (conditional)

#### Usage Patterns

- Initializing logging configuration at startup
- Switching global logging to stderr with a specific logger name

### `clore::support::build_cache_key`

Declaration: `support/logging.cppm:70`

Definition: `support/logging.cppm:368`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function constructs a cache key string by concatenating the given `normalized_path`, a delimiter character stored in the anonymous namespace constant `kCacheKeyDelimiter`, and the string representation of `compile_signature` obtained via `std::to_string`. It first reserves enough capacity—the length of the path plus one for the delimiter plus twenty digits—to avoid reallocation, then appends the three parts in order. No branching or iteration is used; the entire logic is a linear sequence of `std::string` append operations. The only external dependency beyond the standard library is the module-internal `kCacheKeyDelimiter` constant.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `normalized_path`
- `compile_signature`
- `kCacheKeyDelimiter`

#### Writes To

- local `std::string key` (returned)

#### Usage Patterns

- building cache keys for compile results
- combining a file path with a signature

### `clore::support::build_compile_signature`

Declaration: `support/logging.cppm:66`

Definition: `support/logging.cppm:352`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function constructs a single null‑delimited payload by appending the normalized directory (via `clore::support::normalize_path_string`), the `normalized_file`, and each element of `arguments`, separated by `\0` bytes.  It then hashes the entire payload using `llvm::xxh3_64bits` and returns the resulting `std::uint64_t`.  The algorithm is a simple linear concatenation followed by a fixed‑size hash; control flow consists solely of a loop over the argument vector. There are no conditional branches or error paths.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `directory`
- parameter `normalized_file`
- parameter `arguments` (each element read)
- output of `clore::support::normalize_path_string`

#### Usage Patterns

- computing a hash key for compile caching from directory, file, and arguments

### `clore::support::canonical_log_level_name`

Declaration: `support/logging.cppm:77`

Definition: `support/logging.cppm:424`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The implementation of `clore::support::canonical_log_level_name` first guards against an empty input by returning `std::nullopt`.  It then creates a lowercased copy of the input string by applying `std::tolower` to each character, which standardises the casing for the subsequent validation step.  The core validation delegates to `spdlog::level::from_str`: if the returned level equals `spdlog::level::off` and the normalized string is not literally `"off"`, the function concludes that the original string does not name a recognised log level and returns `std::nullopt`.  Otherwise, it returns the lowercased string, signalling a valid canonical name.  The function thus relies on `spdlog`’s own level‑parsing routine to determine acceptability, while the manual casing normalisation guarantees that the output always has a consistent form.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the function parameter `value` of type `std::string_view`

#### Usage Patterns

- Canonicalizing user-provided log level strings before use
- Validating log level configuration entries
- Mapping raw input to a consistent lowercase representation

### `clore::support::enable_utf8_console`

Declaration: `support/logging.cppm:91`

Definition: `support/logging.cppm:534`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function is guarded by the `_WIN32` preprocessor directive, so the implementation is a no‑op on non‑Windows platforms. On Windows, it calls `SetConsoleCP` and `SetConsoleOutputCP` with the constant `CP_UTF8` to switch the console’s input and output code pages to UTF‑8. If either call fails (returns `0`), the function retrieves the error code via `GetLastError` and issues a warning through `clore::logging::warn`, formatting the error as a `std::uint32_t`. The only dependencies are the Windows API functions and the project’s logging facility; no additional data structures or complex control flow are involved.

#### Side Effects

- Sets the console input code page to `CP_UTF8`
- Sets the console output code page to `CP_UTF8`
- Logs a warning if either code page change fails

#### Reads From

- Preprocessor symbol `_WIN32`
- Macro `CP_UTF8`
- Windows system error state via `GetLastError()`

#### Writes To

- Console input code page
- Console output code page
- Log system via `clore::logging::warn`

#### Usage Patterns

- Called during program initialization on Windows to enable UTF-8 console support

### `clore::support::ensure_utf8`

Declaration: `support/logging.cppm:75`

Definition: `support/logging.cppm:405`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

Implementation: [Implementation](functions/ensure-utf8.md)

The function `clore::support::ensure_utf8` implements a validation and repair pass over a `std::string_view` input. It reserves storage for the result and then iterates through the input by calling the helper `clore::support::(anonymous namespace)::valid_utf8_sequence_length` at each offset. If the sequence length is zero—indicating an invalid lead byte—the single byte is replaced with the constant `kUtf8Replacement` (the Unicode replacement character U+FFFD encoded as UTF-8) and the offset advances by one. Otherwise it appends the complete valid sequence and jumps the offset forward by that length. The entire algorithm depends solely on `valid_utf8_sequence_length` and the replacement literal; no other local functions or global state are involved in the loop.

#### Side Effects

- Allocates memory for a new `std::string`
- Appends data to the newly allocated string

#### Reads From

- `text` parameter
- `valid_utf8_sequence_length` function result
- `kUtf8Replacement` constant value
- Bytes of input `text`

#### Writes To

- Output `std::string` that is returned

#### Usage Patterns

- Used by `write_utf8_text_file` to sanitize input before writing
- Used by `truncate_utf8` to ensure truncated result is valid UTF-8

### `clore::support::extract_first_plain_paragraph`

Declaration: `support/logging.cppm:62`

Definition: `support/logging.cppm:303`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The implementation iterates line‑by‑line through the input using `std::istringstream` and `std::getline`. Each line is trimmed of leading and trailing whitespace via `std::isspace`. A Boolean flag `in_code_block` toggles on encountering triple backticks; lines inside fenced code blocks are skipped entirely. If the trimmed line is empty, the function either continues (if no paragraph has started) or breaks out of the loop (ending the paragraph). Lines beginning with `#`, `>`, `|`, `- `, or `* ` (headings, block quotes, table rows, list items) also cause an early break if a paragraph has been started, otherwise they are skipped. All other non‑empty, non‑special lines are appended to the `paragraph` string, separated by a single space. After the loop, the accumulated text is passed through `clore::support::strip_inline_markdown_text` to strip inline formatting (bold, italic, code, etc.) before the result is returned. This function depends solely on `strip_inline_markdown_text` for the final cleanup; no other external functions or data structures are used.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `markdown` parameter
- `strip_inline_markdown_text` function

#### Usage Patterns

- Extracting plain text from Markdown documentation or log messages

### `clore::support::normalize_line_endings`

Declaration: `support/logging.cppm:79`

Definition: `support/logging.cppm:442`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function `clore::support::normalize_line_endings` normalizes line endings in a `std::string_view` input by converting both carriage-return (CR, `'\r'`) and carriage-return–line-feed (CRLF, `"\r\n"`) sequences to a single newline character (`'\n'`). The algorithm traverses the input character by character via an index-based loop. When it encounters a CR, it appends an LF to the result buffer and, if the next character is also LF (i.e., a CRLF pair), it increments the index to skip that LF. All other characters are copied unchanged. The output `std::string` is pre-allocated with `text.size()` capacity for efficiency. No external dependencies beyond the C++ standard library are used; the function relies solely on `std::string_view`, `std::string`, and plain character comparisons.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `text` (parameter of type `std::string_view`)

#### Writes To

- returned `std::string` with normalized line endings

#### Usage Patterns

- normalizing line endings in input text to Unix LF format
- preprocessing text for consistent newline representation before further processing

### `clore::support::normalize_path_string`

Declaration: `support/logging.cppm:64`

Definition: `support/logging.cppm:348`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function delegates to the standard library's `std::filesystem::path` constructor and its `lexically_normal` member to resolve relative components like `..` and `.` and then converts the result to a generic string via `generic_string`. There is no branching or error handling; the single expression directly returns the normalized path. The only dependency is the C++17 filesystem library.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `path` parameter

#### Usage Patterns

- used by `clore::support::build_compile_signature` to normalize path strings before constructing a hash

### `clore::support::read_utf8_text_file`

Declaration: `support/logging.cppm:85`

Definition: `support/logging.cppm:480`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function `clore::support::read_utf8_text_file` first attempts to open the specified `path` as a binary input stream (`std::ifstream` with `std::ios::binary`). If the file cannot be opened, it returns a `std::unexpected` error message via `std::expected`. It then queries `std::filesystem::file_size` to pre‑reserve memory for `content`, improving performance. The file is read in 8192‑byte chunks using a fixed‑size `std::array<char, 8192>`, and each chunk is appended to `content` until the end of file is reached. After reading, the function checks the stream state; if a non‑recoverable error occurred (other than end‑of‑file), it returns an error. Finally, it calls the dependent function `strip_utf8_bom` on the raw `content`. If the BOM stripping changed the string (i.e., a BOM was present), the function returns the stripped view converted to `std::string`; otherwise it returns the original `content` to avoid an unnecessary allocation.

#### Side Effects

- reads file content from disk via `std::ifstream`
- allocates memory for the `std::string` content
- queries file size via `std::filesystem::file_size`

#### Reads From

- the file at the given `path`
- filesystem metadata (file size)

#### Usage Patterns

- load UTF-8 text files for logging configuration
- read source files for processing or analysis
- implement file-based data loading in support utilities

### `clore::support::split_cache_key`

Declaration: `support/logging.cppm:73`

Definition: `support/logging.cppm:378`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The implementation of `clore::support::split_cache_key` proceeds in three sequential validation stages. It first searches the input cache key from the right for the delimiter constant `kCacheKeyDelimiter` using `rfind`. If no delimiter is found, the function returns an `std::unexpected` error constructed with `std::format`. Otherwise, it splits the view into a path part and a signature part via `substr`. If either part is empty, another error is returned.  

The signature part is parsed as a `std::uint64_t` using `std::from_chars`. This call provides both a pointer to the first unconverted character and an error code. The function verifies that the conversion completed without error (`ec == std::errc{}`) and that the entire signature part was consumed (`ptr == signature_part.data() + signature_part.size()`). If parsing fails, an error is returned. On success, a `CacheKeyParts` struct is constructed with the path as a `std::string` and the parsed signature. No external utilities beyond `std::from_chars` and `std::format` are used; the delimiter constant and result type are defined within the `clore::support` namespace.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `cache_key`
- `kCacheKeyDelimiter`

#### Usage Patterns

- Used to split a combined cache key into its path and compile signature components for validation or further processing.
- Complementary to `build_cache_key`.

### `clore::support::strip_utf8_bom`

Declaration: `support/logging.cppm:83`

Definition: `support/logging.cppm:470`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

Implementation: [Implementation](functions/strip-utf8-bom.md)

The implementation first verifies that the input `text` is at least as long as the `kUtf8Bom` byte sequence (three bytes). It then compares the first three bytes of `text` against `kUtf8Bom`, using `static_cast<unsigned char>` to avoid sign‑extension issues with `char` types. If all three bytes match, it returns a `std::string_view` starting after the BOM by calling `text.substr(std::size(kUtf8Bom))`. Otherwise, it returns the original `text` unchanged. The only dependency is the constant `kUtf8Bom`, which is defined in the anonymous namespace within the same translation unit.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `text` of type `std::string_view`
- constant `kUtf8Bom` (likely a three‑byte array `{0xEF, 0xBB, 0xBF}`)

#### Usage Patterns

- Stripping the UTF‑8 BOM from file contents before processing in `clore::support::read_utf8_text_file`

### `clore::support::topological_order`

Declaration: `support/logging.cppm:93`

Definition: `support/logging.cppm:547`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The implementation performs a topological sort of a set of nodes using Kahn’s algorithm. It accepts a list of `nodes`, a `reverse_edges` map that records for each node the set of nodes that depend on it, and an `in_degree` map that tracks the number of outstanding dependencies for each node. Internally, a `std::set<std::string>` named `ready` collects all nodes whose in-degree is either missing or zero, and the algorithm repeatedly extracts the lexicographically smallest such node (via `ready.begin()`), appends it to the `order` vector, and then iterates over the node’s dependents in `reverse_edges`. For each dependent, the corresponding entry in `in_degree` is decremented; if the count reaches zero, the dependent is inserted into `ready`. If after exhausting `ready` the total number of nodes in `order` is less than the original node count, a cycle must exist, and the function returns `std::nullopt`. Otherwise it returns the computed `order`. The implementation depends on standard library containers (`std::vector`, `std::unordered_map`, `std::set`) and the `std::optional` wrapper for safe error signalling.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `nodes` (const `std::vector<std::string>`&)
- `reverse_edges` (const `std::unordered_map<std::string, std::vector<std::string>>`&)
- `in_degree` (`std::unordered_map<std::string, int>`)

#### Usage Patterns

- Used for dependency resolution and ordering tasks, such as scheduling build steps or validating `DAGs`.
- Returns the topological order or indicates a cycle via `std::nullopt`.

### `clore::support::truncate_utf8`

Declaration: `support/logging.cppm:81`

Definition: `support/logging.cppm:460`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The implementation first normalizes the input using the helper `clore::support::ensure_utf8`, which ensures the string is well-formed UTF-8 (replacing any invalid sequences). If the normalized string already fits within `max_bytes`, the function returns it immediately. Otherwise, it truncates the normalized string by resizing it to the byte count returned by `clore::support::(anonymous namespace)::utf8_prefix_length`, which determines the longest valid UTF-8 prefix that does not exceed `max_bytes`. The final result is a valid UTF-8 string guaranteed to be no longer than the requested byte limit. Both `ensure_utf8` and `utf8_prefix_length` are internal helpers defined in an anonymous namespace; the former ensures input integrity, the latter performs the actual boundary-aware truncation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `text` parameter
- `max_bytes` parameter
- `ensure_utf8` call result
- `utf8_prefix_length` helper function

#### Writes To

- returned `std::string`

#### Usage Patterns

- Truncating log messages to fit a byte limit
- Limiting user-provided strings to a maximum UTF-8 byte length
- Ensuring strings do not exceed storage constraints while maintaining encoding validity

### `clore::support::write_utf8_text_file`

Declaration: `support/logging.cppm:88`

Definition: `support/logging.cppm:515`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The implementation of `clore::support::write_utf8_text_file` first normalizes the provided `content` by delegating to `clore::support::ensure_utf8`, which ensures the string is valid UTF‑8 and applies any required normalization. After obtaining the `normalized` string, the function attempts to open a `std::ofstream` in binary mode at the given `path`. If the stream fails to open (checked via `file.is_open()`), it immediately returns an error using `std::unexpected` with a formatted diagnostic message. Otherwise, it writes all bytes of `normalized` via `file.write`, flushes the stream with `file.flush()`, and then verifies the stream state. A failed write or flush also yields an error. On success, the function returns a `std::expected` containing no value. The control flow is a straightforward linear sequence with two early‑return error paths, relying solely on `ensure_utf8` for content preprocessing and on the standard library’s file I/O and error‑reporting facilities.

#### Side Effects

- writes to the file system at the path specified by the `path` parameter

#### Reads From

- `path` parameter
- `content` parameter

#### Writes To

- the file system at the location specified by `path`

#### Usage Patterns

- callers provide a filesystem path and string content to write a UTF-8 encoded file
- used when a function needs to persist text data to disk with error handling

## Internal Structure

The `support` module is decomposed into two principal layers. The lower layer, implemented in an anonymous namespace within `support/logging.cppm`, provides low‑level UTF‑8 validation and manipulation primitives (e.g., `valid_utf8_sequence_length`, `is_continuation_byte`, `utf8_prefix_length`) as well as internal constants (`kUtf8Bom`, `kUtf8Replacement`, `kCacheKeyDelimiter`). The upper layer (namespace `clore::support`) exposes a cohesive public API that includes transparent string hashing and equality functors, path normalization, line‑ending normalization, UTF‑8 BOM stripping and truncation, file I/O helpers for UTF‑8 text, cache‑key construction and splitting, compile‑signature generation, topological ordering, and console encoding setup. The logging subsystem (`clore::logging`) is built on top of these utilities and `spdlog`, providing a global log‑level toggle, a central `log` function, and type‑safe `LogProxy` entry points for each severity. The module imports only `std` directly; all internal layering is expressed via anonymous namespace isolation and the clear distinction between private helpers and public interfaces.

