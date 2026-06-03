---
title: 'Module support'
description: 'The support module provides a collection of foundational utility functions and types for string handling, caching, and logging within the codebase. It includes transparent hash and equality functors for heterogeneous lookup, UTF‑8 text file I/O, string normalization (paths, line endings, BOM removal), and cache key construction and parsing. The module also defines the logging infrastructure through clore::logging, exposing log‑level proxies (trace, debug, info, warn, err) and a log function that dispatches messages to configured sinks. Additional helpers cover topological graph ordering, extraction of plain text paragraphs from Markdown, and console UTF‑8 enablement.'
layout: doc
template: doc
---

# Module `support`

## Summary

The `support` module provides a collection of foundational utility functions and types for string handling, caching, and logging within the codebase. It includes transparent hash and equality functors for heterogeneous lookup, UTF‑8 text file I/O, string normalization (paths, line endings, BOM removal), and cache key construction and parsing. The module also defines the logging infrastructure through `clore::logging`, exposing log‑level proxies (`trace`, `debug`, `info`, `warn`, `err`) and a `log` function that dispatches messages to configured sinks. Additional helpers cover topological graph ordering, extraction of plain text paragraphs from Markdown, and console UTF‑8 enablement.

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

Declaration: `src/support/logging.cppm:135`

Definition: `src/support/logging.cppm:135`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

The struct `clore::logging::LogProxy` is a lightweight callable wrapper whose sole purpose is to capture a specific log level as a compile‑time template parameter (`spdlog::level::level_enum Level`). Its internal structure consists only of the two `operator()` overloads, both of which immediately delegate to the free function `clore::logging::log`, passing `Level` and the composed message. The first overload accepts a raw `std::string_view` and forwards it unchanged; the second variadic overload accepts a `std::format_string<Args...>` plus corresponding arguments, formats the message using `std::format`, and then forwards the resulting `std::string`. Because `Level` is a non‑type template parameter, each instantiation is a distinct type, enabling the compiler to optimize away entire call sites when the level is inactive or to inline the level constant into the logging path. The key invariant is that every invocation of `LogProxy` results in exactly one call to the underlying logging function with the predefined severity, ensuring consistent dispatching without runtime level checks within the wrapper itself.

#### Invariants

- The log level is fixed at compile time and determined solely by the `Level` template parameter.
- All logging operations ultimately call `log(Level, ...)` which must be defined and accessible.

#### Key Members

- `operator()(std::string_view msg) const`
- `operator()(std::format_string<Args...> fmt, Args&&... args) const`

#### Usage Patterns

- Instantiated with specific log levels to create lightweight level-specific logging functors.
- Used in contexts where a callable object with a fixed log level is needed, such as in logger implementations or logging macros.
- Relies on an external `log` function that is not defined within the struct.

#### Member Functions

##### `clore::logging::LogProxy::operator()`

Declaration: `src/support/logging.cppm:142`

Definition: `src/support/logging.cppm:142`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

###### Implementation

```cpp
void operator()(std::format_string<Args...> fmt, Args&&... args) const {
        log(Level, std::format(fmt, std::forward<Args>(args)...));
    }
```

##### `clore::logging::LogProxy::operator()`

Declaration: `src/support/logging.cppm:136`

Definition: `src/support/logging.cppm:136`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

###### Implementation

```cpp
void operator()(std::string_view msg) const {
        log(Level, msg);
    }
```

### `clore::support::CacheKeyParts`

Declaration: `src/support/logging.cppm:80`

Definition: `src/support/logging.cppm:80`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The struct bundles a file path and a compile signature into a single cache key. Its internal structure consists of two flat members: `path` as a `std::string` and `compile_signature` as a `std::uint64_t` defaulting to `0`. The zero-initialized signature acts as a sentinel for unset or invalid keys; the implementation relies on the default member initializer and does not define special constructors, so all instances are trivially constructible or copyable.

#### Invariants

- `compile_signature` is initialized to 0 by default
- Equality of two keys is determined by both fields

#### Key Members

- `path`
- `compile_signature`

#### Usage Patterns

- Used as a key type in caching containers
- Constructed by providing a file path and compilation signature

### `clore::support::TransparentStringEqual`

Declaration: `src/support/logging.cppm:56`

Definition: `src/support/logging.cppm:56`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The struct `clore::support::TransparentStringEqual` provides four overloads of `operator()` that cover all combinations of `std::string` and `std::string_view` arguments. Internally, each overload normalises both arguments to `std::string_view` before performing a direct equality comparison; the two overloads that accept a `const std::string&` explicitly construct a `std::string_view` from that reference, ensuring that no temporary `std::string` copies are made. The member alias `is_transparent`, set to `void`, is the standard tag that enables heterogeneous lookup in associative containers such as `std::unordered_map` when this functor is used as the key equality predicate. All call `operator`s are `noexcept` and return `bool`, and the overall design guarantees that equality semantics are identical to comparing the underlying character sequences regardless of how the arguments are stored.

#### Invariants

- Equality comparison is symmetric and consistent with `std::string_view` comparison.
- All overloads are `noexcept`.

#### Key Members

- `is_transparent` typedef
- `operator()` overloads for comparing `std::string` and `std::string_view`

#### Usage Patterns

- Used as a transparent comparator for `std::unordered_set` or `std::unordered_map` to allow lookup with `std::string_view` without constructing `std::string`.
- Can also be used as a standalone equality functor.

#### Member Types

##### `clore::support::TransparentStringEqual::is_transparent`

Declaration: `src/support/logging.cppm:57`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
using is_transparent = void
```

#### Member Functions

##### `clore::support::TransparentStringEqual::operator()`

Declaration: `src/support/logging.cppm:64`

Definition: `src/support/logging.cppm:64`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
[[nodiscard]] auto operator()(const std::string& lhs, std::string_view rhs) const noexcept
        -> bool {
        return std::string_view{lhs} == rhs;
    }
```

##### `clore::support::TransparentStringEqual::operator()`

Declaration: `src/support/logging.cppm:69`

Definition: `src/support/logging.cppm:69`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
[[nodiscard]] auto operator()(std::string_view lhs, const std::string& rhs) const noexcept
        -> bool {
        return lhs == std::string_view{rhs};
    }
```

##### `clore::support::TransparentStringEqual::operator()`

Declaration: `src/support/logging.cppm:74`

Definition: `src/support/logging.cppm:74`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
[[nodiscard]] auto operator()(const std::string& lhs, const std::string& rhs) const noexcept
        -> bool {
        return lhs == rhs;
    }
```

##### `clore::support::TransparentStringEqual::operator()`

Declaration: `src/support/logging.cppm:59`

Definition: `src/support/logging.cppm:59`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
[[nodiscard]] auto operator()(std::string_view lhs, std::string_view rhs) const noexcept
        -> bool {
        return lhs == rhs;
    }
```

### `clore::support::TransparentStringHash`

Declaration: `src/support/logging.cppm:40`

Definition: `src/support/logging.cppm:40`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The struct provides a single hash functor that delegates all work to the `std::hash<std::string_view>` specialization. The core implementation lives in the `operator()(std::string_view)` overload, which directly invokes `std::hash<std::string_view>{}(value)`. The other two overloads—for `const std::string&` and `const char*`—simply construct a `std::string_view` from their argument and forward to that primary overload, ensuring a single source of hashing logic. The `is_transparent` typedef is a `void` tag that enables heterogeneous lookup in standard unordered associative containers; when present, the container can compare keys of different types without converting them to the stored key type first. All call `operator`s are marked `noexcept` and `[[nodiscard]]`.

#### Invariants

- Hash value depends only on the string content, not the type of the argument.
- The functor is transparent, allowing heterogeneous lookup in unordered containers.
- All `operator()` calls are `noexcept`.

#### Key Members

- `is_transparent`
- `operator()(std::string_view)`
- `operator()(const std::string&)`
- `operator()(const char*)`

#### Usage Patterns

- Used as a hash functor for `std::unordered_set` or `std::unordered_map` with transparent lookup.
- Enables hashing keys without constructing temporary `std::string` objects, improving performance.
- Can be passed as the `Hash` template parameter to associative containers.

#### Member Types

##### `clore::support::TransparentStringHash::is_transparent`

Declaration: `src/support/logging.cppm:41`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
using is_transparent = void
```

#### Member Functions

##### `clore::support::TransparentStringHash::operator()`

Declaration: `src/support/logging.cppm:43`

Definition: `src/support/logging.cppm:43`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
[[nodiscard]] auto operator()(std::string_view value) const noexcept -> std::size_t {
        return std::hash<std::string_view>{}(value);
    }
```

##### `clore::support::TransparentStringHash::operator()`

Declaration: `src/support/logging.cppm:51`

Definition: `src/support/logging.cppm:51`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
[[nodiscard]] auto operator()(const char* value) const noexcept -> std::size_t {
        return (*this)(std::string_view{value});
    }
```

##### `clore::support::TransparentStringHash::operator()`

Declaration: `src/support/logging.cppm:47`

Definition: `src/support/logging.cppm:47`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

###### Implementation

```cpp
[[nodiscard]] auto operator()(const std::string& value) const noexcept -> std::size_t {
        return (*this)(std::string_view{value});
    }
```

## Variables

### `clore::logging::debug`

Declaration: `src/support/logging.cppm:148`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

This variable provides a logging proxy specifically for debug severity messages. As a `constexpr inline` variable, it is intended to be used directly without runtime overhead, enabling conditional compilation or optimization of debug log statements.

#### Mutation

No mutation is evident from the extracted code.

### `clore::logging::err`

Declaration: `src/support/logging.cppm:151`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

As a `LogProxy` instance, it is used to log messages at the error severity level, typically by calling its member functions with format strings and arguments.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- logging error messages via member functions of `LogProxy`

### `clore::logging::g_log_level`

Declaration: `src/support/logging.cppm:125`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

This variable is read by logging functions to decide whether to emit a log message. If set, only messages with severity at or above this level are output; if not set, a default behavior may apply.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Referenced in `clore::logging::log` to filter log messages
- Referenced in `clore::logging::stderr_logger` to determine output level

### `clore::logging::info`

Declaration: `src/support/logging.cppm:149`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

This variable acts as a log proxy object that formats and emits info-level log messages. It is typically used with the stream `operator` or passed to logging functions. Its behavior is affected by the global log level (`clore::logging::g_log_level`) which controls whether messages at this level are actually output.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- emits info-level log messages via stream `operator`
- used in `clore::logging::cache_hit_rate`
- passed as argument to logging functions

### `clore::logging::trace`

Declaration: `src/support/logging.cppm:147`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

It serves as a logging proxy for trace-level messages, allowing structured log statements with formatting and arguments. It is read by surrounding code to output messages through the logging system.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- trace-level logging calls

### `clore::logging::warn`

Declaration: `src/support/logging.cppm:150`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

It is used to emit log messages at the warn severity level. The variable is part of the `clore::logging` facility and participates alongside proxies like `info`, `debug`, and `err` to provide a structured logging interface.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- warning-level logging proxy

## Functions

### `clore::logging::cache_hit_rate`

Declaration: `src/support/logging.cppm:161`

Definition: `src/support/logging.cppm:161`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

The function `clore::logging::cache_hit_rate` first validates its inputs by checking whether the sum of `hits` and `misses` is zero; if so, it throws a `std::logic_error` with a formatted message containing the `name` parameter. Next it computes the hit rate as a percentage using `static_cast<double>(hits) * 100.0 / static_cast<double>(total)`. Finally, it calls the `info` logging proxy (declared in `clore::logging`) to emit the formatted string containing the `name`, raw counts, and the computed `rate`. The implementation depends on `std::format` for string formatting and on the `info` logging facility for output.

#### Side Effects

- Logs a formatted message via `clore::logging::info`
- Throws `std::logic_error` if total is zero

#### Reads From

- Parameter `name`
- Parameter `hits`
- Parameter `misses`

#### Writes To

- Log output through `clore::logging::info`

#### Usage Patterns

- Used to monitor and log cache hit rate
- Called after cache access to record statistics

### `clore::logging::log`

Declaration: `src/support/logging.cppm:127`

Definition: `src/support/logging.cppm:127`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

Implementation: [Implementation](functions/log.md)

The function first checks whether the global filtering level `g_log_level` has been set and, if so, whether the requested `lvl` is strictly lower than that threshold. When the condition holds, `log` returns immediately without performing any output. Otherwise, it forwards the `lvl` and the formatted `msg` to the underlying logging library by invoking `spdlog::default_logger_raw()->log(lvl, "{}", msg)`. This two‑step control flow—guard against insufficient severity, then delegate to the spdlog default logger—provides a lightweight, configurable filtering mechanism without modifying the logger’s own level settings.

#### Side Effects

- Logs a message via spdlog default logger, which may write to console or file.

#### Reads From

- `g_log_level`
- lvl parameter
- msg parameter

#### Writes To

- spdlog default logger output

#### Usage Patterns

- Called by `LogProxy::operator()` to dispatch logging
- Used for conditional logging with level filtering

### `clore::logging::stderr_logger`

Declaration: `src/support/logging.cppm:153`

Definition: `src/support/logging.cppm:153`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

The function `clore::logging::stderr_logger` creates an spdlog logger on stderr by invoking `spdlog::stderr_color_mt` with a copy of the `name` parameter, then registers that logger as the global default via `spdlog::set_default_logger`. If the optional global level variable `clore::logging::g_log_level` holds a value, the function further calls `spdlog::set_level` to enforce that threshold, filtering out messages below the specified severity. The implementation has no branching beyond the existence check for `g_log_level` and delegates all output and color formatting to the spdlog library; it does not perform any additional message transformation, buffering, or stream synchronization.

#### Side Effects

- Creates a new stderr color logger
- Sets the new logger as the default logger
- Possibly sets the global log level from `g_log_level`

#### Reads From

- `name` parameter
- Global variable `g_log_level`

#### Writes To

- Global spdlog default logger state
- Global spdlog log level via `spdlog::set_level`

#### Usage Patterns

- Used during logging initialization to direct output to stderr
- Called early in program startup to configure logging

### `clore::support::build_cache_key`

Declaration: `src/support/logging.cppm:93`

Definition: `src/support/logging.cppm:391`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function `clore::support::build_cache_key` constructs a cache key string by concatenating a normalized path and a compile signature separated by a delimiter. It first allocates a `std::string` with sufficient capacity using `reserve` to avoid reallocation, then appends the `normalized_path` argument, a single character delimiter obtained from the anonymous namespace constant `kCacheKeyDelimiter`, and finally the string representation of `compile_signature` via `std::to_string`. The resulting key is returned directly. The implementation has no branching or error handling, relying on the caller to supply a correctly normalized path and a valid signature. The only dependency outside of the standard library is the `kCacheKeyDelimiter` constant defined in the enclosing anonymous namespace.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `normalized_path`
- parameter `compile_signature`

#### Usage Patterns

- Constructs cache keys for compilation artifacts
- Used to key entries in a compilation cache

### `clore::support::build_compile_signature`

Declaration: `src/support/logging.cppm:89`

Definition: `src/support/logging.cppm:375`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function constructs a serialized payload by first applying `clore::support::normalize_path_string` to the `directory` argument, then appending the result, a null byte, the `normalized_file` parameter, another null byte, and finally each element of `arguments` followed by a null byte. It reserves capacity in advance to minimize reallocations. The resulting null‑terminated concatenation is then hashed using LLVM's `llvm::xxh3_64bits` to produce a 64‑bit unsigned integer that serves as the compile signature. The only explicit dependency is the path‑normalization helper; the rest of the logic is a straightforward serialization and hashing step.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `directory`
- `normalized_file`
- `arguments`

#### Usage Patterns

- Used to generate a unique identifier for a compilation unit based on its directory, file, and arguments.

### `clore::support::canonical_log_level_name`

Declaration: `src/support/logging.cppm:100`

Definition: `src/support/logging.cppm:447`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function first handles the trivial empty-input case by returning `std::nullopt`. It then converts the input `value` to a lowercase `normalized` string by applying `std::tolower` (cast to `unsigned char` to avoid undefined behaviour) to each character. The core validation step delegates to `spdlog::level::from_str`, which interprets the lowercased name against spdlog’s built-in log-level enumeration. When the returned level equals `spdlog::level::off` and the normalized string is not literally `"off"`, the input is considered invalid and `std::nullopt` is returned; otherwise the normalized lower-case string is returned as the canonical form. This implementation depends on spdlog’s level parsing and the character conversion utilities, and performs no other external operations.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `value` of type `std::string_view`
- internal spdlog level mapping via `spdlog::level::from_str`

#### Usage Patterns

- validating and normalizing user-provided log level names
- converting log level strings to canonical form before use in logging configuration

### `clore::support::enable_utf8_console`

Declaration: `src/support/logging.cppm:114`

Definition: `src/support/logging.cppm:557`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function `clore::support::enable_utf8_console` is conditionally compiled for Windows only using a `#ifdef _WIN32` guard. On Windows, it invokes the Win32 API `SetConsoleCP(CP_UTF8)` to set the console's input code page to UTF-8, and `SetConsoleOutputCP(CP_UTF8)` to set the output code page to UTF-8. If either call returns zero (indicating failure), it logs a warning via `clore::logging::warn`, including the extended error code from `GetLastError()`. The function has no effect on non‑Windows platforms.

The implementation relies on the Windows SDK headers for `SetConsoleCP`, `SetConsoleOutputCP`, `CP_UTF8`, and `GetLastError`, and depends on the project’s logging facility `clore::logging::warn` for error reporting. No other control flow or branching occurs beyond the conditional compilation and the two error checks.

#### Side Effects

- Sets the console input code page to UTF-8.
- Sets the console output code page to UTF-8.
- Emits warning log messages on failure.

#### Reads From

- Result of `SetConsoleCP` and `SetConsoleOutputCP` to detect failure.
- System error code via `GetLastError()` when a call fails.

#### Writes To

- Windows console input code page.
- Windows console output code page.
- Logging output (via `clore::logging::warn`).

#### Usage Patterns

- Called at application startup to ensure UTF-8 support in the Windows console.

### `clore::support::ensure_utf8`

Declaration: `src/support/logging.cppm:98`

Definition: `src/support/logging.cppm:428`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

Implementation: [Implementation](functions/ensure-utf8.md)

The implementation of `clore::support::ensure_utf8` iterates over the input `text` byte by byte, using the helper function `valid_utf8_sequence_length` (defined in an anonymous namespace) to determine the length of the UTF-8 sequence starting at each `offset`. If the helper returns `0`, the current byte is not a valid UTF-8 lead byte; in that case a replacement character (`kUtf8Replacement`) is appended to the output, and the iteration advances by one. When a valid sequence length is found, the corresponding substring is appended and `offset` is incremented by that length. The result accumulates into a `std::string` preallocated to the input size, and is returned as a fully valid UTF-8 string. The only explicit dependency is the sequence-length validator, which encapsulates the UTF-8 encoding rules.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `text`
- constant `kUtf8Replacement`

#### Writes To

- returned `std::string`

#### Usage Patterns

- Ensuring text is valid UTF-8 before passing to `write_utf8_text_file`
- Sanitizing input before truncation in `truncate_utf8`

### `clore::support::extract_first_plain_paragraph`

Declaration: `src/support/logging.cppm:85`

Definition: `src/support/logging.cppm:326`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function `clore::support::extract_first_plain_paragraph` processes the input `markdown` line by line using a `std::istringstream`. It maintains a `in_code_block` flag to skip content inside fenced code blocks (delimited by triple backticks). Each line is trimmed of leading and trailing whitespace. Empty lines that occur before any content are skipped; once a non-empty line has been accumulated, a blank line signals the end of the paragraph. Lines beginning with Markdown block‑level markers (`#`, `>`, `|`, `- `, or `* `) are also skipped unless they appear at the start of an empty paragraph buffer, in which case the paragraph is considered terminated. All remaining lines are joined into a single string separated by spaces. The resulting paragraph is then passed to `clore::support::(anonymous namespace)::strip_inline_markdown_text`, which removes inline formatting (such as bold, italic, or code spans), and the clean plain text is returned.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `markdown` parameter

#### Usage Patterns

- extracting a plain text summary from Markdown
- obtaining a human-readable description from documentation

### `clore::support::normalize_line_endings`

Declaration: `src/support/logging.cppm:102`

Definition: `src/support/logging.cppm:465`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The implementation iterates over the input `text` character by character using an index loop. For each `\r` (carriage return) encountered, it appends a single `\n` (line feed) to the output `normalized` string. If the `\r` is immediately followed by `\n` (i.e., a CRLF sequence), the `\n` is skipped by incrementing the index so that it is not duplicated. All other characters are copied verbatim. The algorithm has no dependencies beyond the C++ standard library containers (`std::string` and `std::string_view`); preallocation via `reserve` reduces reallocations when the input length is known. The result is a string that uses only LF line endings regardless of the original mixed CR, LF, or CRLF conventions.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- text (`std::string_view` parameter)

#### Writes To

- returned `std::string` (local variable normalized)

#### Usage Patterns

- Normalize line endings from different platforms
- Preprocess text before further processing

### `clore::support::normalize_path_string`

Declaration: `src/support/logging.cppm:87`

Definition: `src/support/logging.cppm:371`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

Implementation: [Implementation](functions/normalize-path-string.md)

The implementation delegates path normalization entirely to `std::filesystem::path`. It constructs a temporary `std::filesystem::path` from `path`, invokes `lexically_normal()` to collapse redundant `.` and `..` components and resolve directory separators in a platform-independent way, then calls `generic_string()` to output the result using forward slashes. There is no custom control flow—the function returns the result of this chained expression directly. Its only dependency is the C++ Standard Library’s filesystem support.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `path` parameter

#### Usage Patterns

- normalizing paths for signature computation
- ensuring consistent path representation

### `clore::support::read_utf8_text_file`

Declaration: `src/support/logging.cppm:108`

Definition: `src/support/logging.cppm:503`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The implementation opens the file in binary mode via `std::ifstream`. If that fails, it returns `std::unexpected` containing a formatted error message. To reduce reallocations, it optionally queries `std::filesystem::file_size` and calls `content.reserve` with that size. It then reads the file in fixed-size chunks using a local `std::array<char, 8192>`, calling `file.read` repeatedly and appending the reported `bytes_read` to `content`. After the loop, it checks for a hard read failure (`file.bad()`) or a non‑eof failure (`file.fail() && !file.eof()`) and returns an error in that case. Finally, it passes the raw `content` through `strip_utf8_bom`; if the resulting view has the same length as the original, the BOM was absent and the function returns `content` directly, otherwise it returns a `std::string` constructed from the stripped view. The only dependency used internally is `strip_utf8_bom` for removing a leading UTF‑8 byte order mark.

#### Side Effects

- reads file from filesystem
- allocates memory for string content
- reads filesystem metadata via `file_size`

#### Reads From

- the file at `path` from the filesystem
- filesystem metadata via `std::filesystem::file_size`

#### Writes To

- heap memory allocated for the returned `std::string`

#### Usage Patterns

- load UTF-8 text files into memory
- obtain file contents as a `std::string` with error handling via `std::expected`

### `clore::support::split_cache_key`

Declaration: `src/support/logging.cppm:96`

Definition: `src/support/logging.cppm:401`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function first searches for the last occurrence of the anonymous‑namespace constant `kCacheKeyDelimiter` within the input `cache_key`. If the delimiter is absent, it returns an error message constructed via `std::format`. Otherwise it splits the string into two sub‑views: `path_part` (everything before the delimiter) and `signature_part` (everything after). Both sub‑views must be non‑empty; an empty part triggers an error. The `signature_part` is then parsed as a `std::uint64_t` using `std::from_chars`. Parsing is considered successful only when the entire substring is consumed and no error code (`std::errc`) is set. On success a `CacheKeyParts` value is returned with the path stored as a `std::string` and the parsed signature. Any failure at any of the validation or parsing steps returns a descriptive error string inside `std::unexpected`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `cache_key` parameter
- `kCacheKeyDelimiter`

#### Usage Patterns

- Decompose cache keys created by `build_cache_key`
- Validate and parse cache key strings

### `clore::support::strip_utf8_bom`

Declaration: `src/support/logging.cppm:106`

Definition: `src/support/logging.cppm:493`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

Implementation: [Implementation](functions/strip-utf8-bom.md)

The function `clore::support::strip_utf8_bom` implements a simple prefix-stripping algorithm. It first checks whether the input `std::string_view` is long enough to contain the UTF-8 BOM sequence by comparing its size to `std::size(kUtf8Bom)`. If the length requirement is met, it performs three bytewise comparisons using `static_cast<unsigned char>` against the constant `kUtf8Bom` (defined in the anonymous namespace). When all three bytes match, the function returns a view starting after the BOM via `text.substr(std::size(kUtf8Bom))`. Otherwise, it returns the original string view unchanged. The implementation has no loops, no allocation, and relies solely on the inline constant `kUtf8Bom` for the BOM byte values.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- text

#### Usage Patterns

- called by `read_utf8_text_file` to strip BOM

### `clore::support::topological_order`

Declaration: `src/support/logging.cppm:116`

Definition: `src/support/logging.cppm:570`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The implementation of `clore::support::topological_order` employs Kahn’s algorithm to produce a lexicographically smallest topological ordering of the given `nodes`. The function accepts a list of all `nodes`, a `reverse_edges` map where each key maps to its dependents, and a mutable `in_degree` map that tracks the number of unprocessed predecessors for each node. It initialises a `std::set<std::string>` called `ready` with every node whose `in_degree` is zero or missing (treating missing entries as zero). The set stores strings in lexicographic order to guarantee deterministic output.

The main loop repeatedly extracts the first element from `ready`, appends it to `order`, and locates its entry in `reverse_edges`. For each `dependent` of the current node, the corresponding `in_degree` value is decremented; if the value drops to zero, the dependent is inserted into `ready`. If the loop exhausts `ready` but the size of `order` is less than the number of `nodes`, a cycle exists and the function returns `std::nullopt`. Otherwise, it returns the completed `order`. The algorithm relies on `std::set` for sorted insertion and removal, and assumes that the `in_degree` map is correctly provided by the caller.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `nodes` parameter
- `reverse_edges` parameter
- `in_degree` parameter (passed by value)

#### Writes To

- local variables `ready`, `order`, and the parameter `in_degree` (copy)

#### Usage Patterns

- topological ordering in dependency graphs
- build system dependency resolution

### `clore::support::truncate_utf8`

Declaration: `src/support/logging.cppm:104`

Definition: `src/support/logging.cppm:483`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The function first passes its input through `clore::support::ensure_utf8` to obtain a well-formed UTF‑8 string. If the normalized string’s byte count already satisfies the `max_bytes` bound, it is returned directly. Otherwise, the internal helper `clore::support::(anonymous namespace)::utf8_prefix_length` is called to compute the largest valid UTF‑8 prefix length that fits within the limit; `normalized` is then resized to that length and returned. Both dependencies are resolved within the `clore::support` namespace, with `utf8_prefix_length` being an anonymous‑namespace helper that examines the encoded sequence boundaries to avoid splitting a multi‑byte character.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- text (`std::string_view`)
- `max_bytes` (`std::size_t`)

#### Usage Patterns

- Used to safely truncate UTF-8 text for storage or display constraints
- Called when formatting log messages or database entries to limit byte length

### `clore::support::write_utf8_text_file`

Declaration: `src/support/logging.cppm:111`

Definition: `src/support/logging.cppm:538`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

The implementation first normalizes the input content by calling `clore::support::ensure_utf8`, which produces a `std::string` containing valid UTF-8. An `std::ofstream` is opened in binary mode on the given path. If the open fails, the function returns `std::unexpected` with a formatted error message. Otherwise, it writes the normalized data directly via `file.write`, flushes the stream, and checks the stream’s state. A failed write also results in an error, while success returns an empty `std::expected<void, std::string>`. The entire I/O path relies on `ensure_utf8` to guarantee well-formed output and on `std::format` for error reporting.

#### Side Effects

- writes to filesystem

#### Reads From

- path parameter
- content parameter
- `ensure_utf8` function call (reads its argument)

#### Writes To

- file at path

#### Usage Patterns

- writing UTF-8 text files

## Internal Structure

The `support` module is decomposed into two distinct namespaces: `clore::support` and `clore::logging`. The `clore::support` namespace exposes the project’s foundational utility layer, including UTF‑8 validation and truncation (`ensure_utf8`, `truncate_utf8`), line‑ending normalization (`normalize_line_endings`), path string normalization for stable keys (`normalize_path_string`), cache‑key construction and parsing (`build_cache_key`, `split_cache_key`, `CacheKeyParts`), compile‑signature generation (`build_compile_signature`), topological ordering of directed graphs (`topological_order`), and file I/O with UTF‑8 BOM handling (`read_utf8_text_file`, `write_utf8_text_file`, `strip_utf8_bom`). Heterogeneous‑lookup support for unordered containers is provided via the transparent hasher and equality functors (`TransparentStringHash`, `TransparentStringEqual`). Internal implementation details, such as the UTF‑8 sequence‑length helper (`valid_utf8_sequence_length`), markdown‑stripping (`strip_inline_markdown_text`), and the BOM and replacement constants, reside within an anonymous namespace to enforce encapsulation and avoid export leakage.

The `clore::logging` namespace builds on the utilities from `clore::support` and external libraries (notably `spdlog`) to provide a lightweight logging façade. It defines a templated `LogProxy` for compile‑time log‑level selection and exposes pre‑instantiated inline variables (`trace`, `debug`, `info`, `warn`, `err`) that serve as callable logging proxies. The global log‑level threshold is stored in `g_log_level`. A direct‑to‑stderr path (`stderr_logger`) and a cache‑hit‑rate reporting function (`cache_hit_rate`) offer additional logging conveniences. The module’s layer structure ensures that lower‑level string‑ and path‑processing routines are separated from the log‑dispatch logic, promoting reuse and testability across the codebase.

