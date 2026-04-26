---
title: 'Module support'
description: 'support 模块提供了两部分核心能力：基础工具与日志设施。在 clore::support 命名空间中，它封装了 UTF‑8 文本处理（编码验证、截断、BOM 剥离、行尾规范化）、文件读写（read_utf8_text_file 与 write_utf8_text_file）、路径与缓存键的正常化与构造（normalize_path_string、build_cache_key、build_compile_signature）以及字符串比较的透明哈希与相等函数对象（TransparentStringHash、TransparentStringEqual）。这些工具旨在为上层模块提供高效、可移植且面向 Unicode 的基础支撑。'
layout: doc
template: doc
---

# Module `support`

## Summary

support 模块提供了两部分核心能力：基础工具与日志设施。在 `clore::support` 命名空间中，它封装了 UTF‑8 文本处理（编码验证、截断、BOM 剥离、行尾规范化）、文件读写（`read_utf8_text_file` 与 `write_utf8_text_file`）、路径与缓存键的正常化与构造（`normalize_path_string`、`build_cache_key`、`build_compile_signature`）以及字符串比较的透明哈希与相等函数对象（`TransparentStringHash`、`TransparentStringEqual`）。这些工具旨在为上层模块提供高效、可移植且面向 Unicode 的基础支撑。

`clore::logging` 命名空间则定义了日志记录的整体抽象：通过模板类 `LogProxy` 与预定义的编译时常量（`trace`、`debug`、`info`、`warn`、`err`）提供类型安全的日志级别代理，配合全局日志级别 `g_log_level` 和底层函数 `log` 实现分级过滤；同时还包含直接输出到标准错误的 `stderr_logger`、缓存命中率记录 `cache_hit_rate` 等实用日志辅助。所有这些组件共同构成了一个轻量、清晰的日志框架，供项目内其他模块使用。

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

`clore::logging::LogProxy` 是一个模板结构体，其唯一的模板参数 `spdlog::level::level_enum Level` 在编译期固定日志级别。内部实现依赖两个 `operator()` 重载：接受 `std::string_view` 的重载直接调用 `clore::logging::log` 并传递 `Level` 和消息；另一个重载是模板函数，接受 `std::format_string<Args...>` 和可变参数 `Args`，它通过 `std::format` 将格式化字符串与参数组合，再转发给同一条 `log` 函数。该重载通过 `requires (sizeof...(Args) > 0)` 确保仅在提供至少一个参数时启用，从而避免了与字符串视图重载的歧义。核心不变量是 `Level` 在实例化时确定，且所有日志输出最终都经由集中的 `clore::logging::log` 路由，保证了日志级别与消息格式化的分离。

#### Invariants

- The template argument `Level` must be a valid `spdlog::level::level_enum` value.
- All calls to `operator()` eventually invoke `log` with the same `Level`.
- The formatted overload requires at least one argument to avoid ambiguity with the plain-string overload.

#### Key Members

- `void operator()(std::string_view msg) const`
- `void operator()(std::format_string<Args...> fmt, Args&&... args) const`

#### Usage Patterns

- Instantiated for each log level (e.g., `LogProxy<spdlog::level::info>`) to create level-specific logging functors.
- Used as a building block for higher-level logging macros or wrapper objects that capture a level.
- Invoked call-site code to log messages without specifying the level explicitly.

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

该结构体是一个简单的聚合类型，用于存储与缓存键相关的两个关键数据：标识缓存来源路径的 `path` 字符串，以及记录编译时签名值的 `compile_signature` 整数。成员 `compile_signature` 默认初始化为 `0`，表示未设置或有效的签名缺失状态；内部不变量要求 `path` 在构造后始终有效（即使为空），而 `compile_signature` 在使用前应被显式赋值，否则零值可能被误认为一个合法的签名。结构体不提供任何自定义构造函数或成员函数，所有成员均直接暴露，以支持高效的逐字段复制与比较，并确保键的构成完全由调用方控制。

#### Invariants

- `compile_signature` 默认初始化为 0
- `path` 应包含有效的文件路径字符串

#### Key Members

- `path`
- `compile_signature`

#### Usage Patterns

- 用于组成缓存键，将路径和编译签名作为哈希或比较的一部分

### `clore::support::TransparentStringEqual`

Declaration: `support/logging.cppm:33`

Definition: `support/logging.cppm:33`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该结构体是一个透明的相等比较器，专为支持异构查找而设计。核心机制是通过定义 `is_transparent` 类型别名来启用关联容器的透明操作。内部不维护任何状态，所有成员函数均为 `noexcept` 且返回 `bool`。四个 `operator()` 重载覆盖了 `std::string_view` 与 `std::string` 之间的所有四种组合：对于纯 `std::string_view` 的比较，直接使用 `==`；当涉及 `std::string` 时，则通过隐式构造 `std::string_view` 再进行比较，从而避免不必要的字符串拷贝。每个重载均使用 `[[nodiscard]]` 标记，确保调用者处理比较结果。所有实现均保持简单对称，无异常抛出保证。

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

该结构体通过将 `is_transparent` 定义为 `void` 启用了透明哈希（heterogeneous lookup），使得它在关联容器（如 `std::unordered_set` 或 `std::unordered_map`）中能够接受 `std::string`、`std::string_view` 或 `const char*` 作为键类型，而无需显式构造 `std::string`。所有 `operator()` 重载的内部实现都统一委托给对 `std::hash<std::string_view>` 的调用：对于 `std::string` 和 `const char*` 参数，首先隐式或显式构造一个临时的 `std::string_view`，再调用该底层哈希函数。这种设计确保了哈希值完全由字符串的内容决定，且所有重载的哈希结果语义一致。

结构体的核心不变量在于：所有输入类型在哈希时会被规范化为相同的 `std::string_view` 表示，因此指向相同字符序列的不同类型参数（例如 `std::string` 和 `const char*`）会产生相同的哈希值。由于底层 `std::hash<std::string_view>` 不抛出异常，所有 `operator()` 都被声明为 `noexcept`，从而允许容器在大多数场景下安全地使用该哈希器。

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

作为日志代理，`clore::logging::debug` 通过重载的 `operator()` 或流式操作符接收日志消息，并将其传递给底层日志系统，当全局日志级别允许调试信息时输出。

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- 作为日志代理，用于输出调试信息

### `clore::logging::err`

Declaration: `support/logging.cppm:128`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

It is used as a lightweight handle to emit error-level log entries, typically invoked with a format string and arguments (e.g., `err(fmt, args...)`). Since it is `constexpr` and `inline`, it carries no runtime overhead and can be optimized away when logging is disabled.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- used as a proxy for logging at error level

### `clore::logging::g_log_level`

Declaration: `support/logging.cppm:102`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

该变量在 `clore::logging` 命名空间内定义，通过 `clore::logging::log` 和 `clore::logging::stderr_logger` 函数读取，以决定是否记录特定级别的日志。

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- 在 `clore::logging::log` 中读取以过滤日志级别
- 在 `clore::logging::stderr_logger` 中读取以过滤日志级别

### `clore::logging::info`

Declaration: `support/logging.cppm:126`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

This variable is a constant log proxy used to invoke logging at the info level. Because it is `constexpr` and `inline`, it is immutable and typically used in expressions like `clore::logging::info("message")` to dispatch formatted log output through the underlying logging system.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Invoked as a function-like object with a format string and arguments to log at info level

### `clore::logging::trace`

Declaration: `support/logging.cppm:124`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

`clore::logging::trace` is used to emit trace-level log entries through the logging system. It is a proxy object that formats and dispatches log messages at the `trace` severity, typically invoked with a format string and arguments.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- invoked with formatting arguments to log trace messages

### `clore::logging::warn`

Declaration: `support/logging.cppm:127`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

`clore::logging::warn` is a compile-time constant log proxy that forwards warning log statements to the underlying logging system. It is used in code paths such as `clore::support::enable_utf8_console` to output warning level logs. As a `constexpr` object, it is never mutated.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- used as a log proxy to emit warning-level log messages
- referenced in function `clore::support::enable_utf8_console`

## Functions

### `clore::logging::cache_hit_rate`

Declaration: `support/logging.cppm:138`

Definition: `support/logging.cppm:138`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

函数 `clore::logging::cache_hit_rate` 的实现首先计算总请求数 `total = hits + misses`。若 `total` 为0，则抛出 `std::logic_error`，防止除零并提示无效统计。随后通过 `static_cast<double>(hits) * 100.0 / static_cast<double>(total)` 计算命中率百分比，并利用 `clore::logging::info` 日志代理输出格式化字符串，包含缓存名称、命中次数、未命中次数及百分比（保留一位小数）。该函数依赖 `std::format` 进行字符串格式化，以及 `clore::logging::info` 作为日志输出通道，内部控制流仅包含简单的算术运算与零值守卫。

#### Side Effects

- 抛出 `std::logic_error` 异常
- 通过 `info` 写入日志（可能输出到 stderr 或日志系统）

#### Reads From

- 参数 `name`
- 参数 `hits`
- 参数 `misses`

#### Writes To

- 通过 `info` 写入日志
- 异常对象 `std::logic_error`

#### Usage Patterns

- 用于报告缓存性能统计
- 在缓存实现中调用时确保 `hits + misses` 非零
- 通过 `info` 记录缓存命中率

### `clore::logging::log`

Declaration: `support/logging.cppm:104`

Definition: `support/logging.cppm:104`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

Implementation: [Implementation](functions/log.md)

函数 `clore::logging::log` 首先根据全局日志级别 `clore::logging::g_log_level` 进行过滤。如果 `g_log_level` 已被设置且参数 `lvl` 的级别低于该阈值，函数会直接返回，避免不必要的日志输出。否则，它将通过调用 `spdlog::default_logger_raw()->log` 将消息 `msg` 传递到 spdlog 后端。该实现依赖于 spdlog 库的日志记录基础设施，并利用预编译的格式字符串 `"{}"` 将消息作为纯字符串记录。

#### Side Effects

- 记录一条日志消息到`spdlog`默认日志器的输出目标（如控制台或文件）

#### Reads From

- 参数`lvl`
- 参数`msg`
- 全局变量`g_log_level`

#### Usage Patterns

- 由`LogProxy::operator()`在日志记录时调用
- 用于按级别过滤日志消息

### `clore::logging::stderr_logger`

Declaration: `support/logging.cppm:130`

Definition: `support/logging.cppm:130`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

该函数通过直接调用 `spdlog::stderr_color_mt` 并将传入的 `name` 转换为 `std::string` 来创建一个彩色的 stderr 日志记录器。随后使用 `spdlog::set_default_logger` 将该记录器注册为全局默认日志器，替换之前注册的任何默认日志器。在完成默认日志器的设置后，函数检查全局变量 `g_log_level` 是否持有值；若持有，则调用 `spdlog::set_level` 将 spdlog 的全局日志级别更新为该值，从而允许通过该外部控制状态覆盖默认的日志级别。实现依赖 spdlog 库的线程安全日志器工厂和级别设置 API，以及一个可选的全局级别存储。

#### Side Effects

- Creates a spdlog logger (I/O, allocation)
- Sets the global default logger
- May set the global log level

#### Reads From

- parameter `name`
- global `g_log_level` (optional)

#### Writes To

- global default logger (via `spdlog::set_default_logger`)
- global log level (via `spdlog::set_level`, if `g_log_level` has value)

#### Usage Patterns

- Called once at program startup to initialize logging
- Used to configure logging output to stderr

### `clore::support::build_cache_key`

Declaration: `support/logging.cppm:70`

Definition: `support/logging.cppm:368`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

函数 `clore::support::build_cache_key` 通过字符串拼接构造缓存键：首先在 `key` 上调用 `reserve`，预估容量为 `normalized_path.size()` 加 1 个分隔符字节再加 20 位签名长度，然后依次追加 `normalized_path`、`kCacheKeyDelimiter` 以及通过 `std::to_string` 转换的 `compile_signature`。整个流程仅依赖 `kCacheKeyDelimiter` 常量和标准库数值转换，无分支或错误处理。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `normalized_path` parameter
- `compile_signature` parameter
- `kCacheKeyDelimiter` constant

#### Usage Patterns

- used to generate a key for caching compilation results based on file path and signature

### `clore::support::build_compile_signature`

Declaration: `support/logging.cppm:66`

Definition: `support/logging.cppm:352`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该函数通过将三个输入组件（`directory`、`normalized_file` 以及 `arguments` 中的每个字符串）以 `'\0'` 分隔拼接为一个线性 `payload`，再调用 `llvm::xxh3_64bits` 计算 64 位哈希值。其中 `directory` 在拼接前会先经过 `normalize_path_string` 规范化处理，以确保路径表示的一致性。内部控制流仅包含一次循环遍历 `arguments` 向量，所有数据均以连续空字节分隔，从而消除不同输入长度带来的歧义。该实现的唯一外部依赖是 `normalize_path_string` 和 LLVM 的 `xxh3_64bits` 哈希函数。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `directory`
- `normalized_file`
- `arguments`

#### Usage Patterns

- Creating a hash-based signature for compilation input
- Used for cache key generation

### `clore::support::canonical_log_level_name`

Declaration: `support/logging.cppm:77`

Definition: `support/logging.cppm:424`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该函数首先检查输入 `value` 是否为空，若空则立即返回 `std::nullopt`。随后将 `value` 复制到本地字符串 `normalized`，并利用 `std::tolower` 逐字符转换为小写，得到规范化的日志等级名称。核心依赖是 `spdlog::level::from_str`，它被用来将小写字符串解析为 `spdlog::level::level_enum`。若解析结果为 `spdlog::level::off` 且 `normalized` 本身并不是字符串 "off"，则说明输入不是合法的日志等级名称，同样返回 `std::nullopt`。最终，函数返回 `normalized` 字符串本身（即小写后的原始输入），作为规范化的等级名称。整个流程不依赖其他外部库或复杂控制流，仅基于 `std::tolower` 和 `spdlog` 的等级解析接口。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` parameter

#### Usage Patterns

- canonicalize log level names
- validate log level strings
- obtain normalized level string

### `clore::support::enable_utf8_console`

Declaration: `support/logging.cppm:91`

Definition: `support/logging.cppm:534`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该函数仅在 Windows 平台上生效（通过 `_WIN32` 预处理器分支控制）。首先调用 `SetConsoleCP(CP_UTF8)` 将控制台输入代码页设置为 UTF-8，若返回值为 0 则表示失败，此时使用 `clore::logging::warn` 记录错误代码（通过 `GetLastError` 获取）。接着调用 `SetConsoleOutputCP(CP_UTF8)` 设置输出代码页，同样在失败时记录警告。

函数的控制流完全由两个 Windows API 调用的返回值决定，不涉及循环或复杂逻辑。主要依赖 Windows SDK 的 `SetConsoleCP`、`SetConsoleOutputCP` 和 `GetLastError` 函数，以及 `clore::logging::warn` 用于错误日志记录。

#### Side Effects

- Changes the console input code page to UTF-8
- Changes the console output code page to UTF-8
- Logs a warning on failure of `SetConsoleCP` or `SetConsoleOutputCP`

#### Writes To

- Console input code page
- Console output code page

#### Usage Patterns

- Called during program initialization to enable UTF-8 console support

### `clore::support::ensure_utf8`

Declaration: `support/logging.cppm:75`

Definition: `support/logging.cppm:405`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

Implementation: [Implementation](functions/ensure-utf8.md)

函数 `clore::support::ensure_utf8` 遍历输入字符串的每个字节偏移，核心控制流依赖于调用 `valid_utf8_sequence_length` 来判断从当前位置开始的字节序列是否构成合法的 UTF-8 编码。若该函数返回非零长度，则直接将对应子串追加到结果中；若返回零（表示无效起始字节或编码错误），则追加一个预定义的替换字符 `kUtf8Replacement` 并仅将偏移前进一个字节，以容忍非法序列。内部使用 `reserve` 预分配与输入等长的内存以避免多次重分配。该实现不依赖任何外部状态，完全基于对每个字节的局部解码决策。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `text` parameter
- `valid_utf8_sequence_length` function
- `kUtf8Replacement` constant

#### Writes To

- returned `std::string`

#### Usage Patterns

- called by `write_utf8_text_file` to ensure valid UTF-8 before writing
- called by `truncate_utf8` to sanitize input before truncation

### `clore::support::extract_first_plain_paragraph`

Declaration: `support/logging.cppm:62`

Definition: `support/logging.cppm:303`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该函数通过逐行扫描输入的 `markdown` 字符串来收集第一个非空段落。它维护一个 `in_code_block` 布尔标志，当遇到 `\`\`\`` 行时翻转该标志，并在代码块内跳过所有行。对于非代码块行，它首先去除首尾空白，然后跳过空行、标题（`#`）、引用（`>`）、表格行（`|`）以及无序列表项（`- ` 或 `* `）。如果当前行非空且不是上述结构，则将该行追加到 `paragraph` 后（追加一个空格作为行间分隔）。当遇到一个空行且 `paragraph` 非空时，停止扫描。

收集到的原始段落随后被传入 `clore::support::(anonymous namespace)::strip_inline_markdown_text` 以移除内联 Markdown 格式（如粗体、斜体、行内代码等）。整个过程中未使用外部缓存或 IO 操作，仅依赖于标准库字符串操作和 `strip_inline_markdown_text` 这一个内部辅助函数。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `markdown` parameter of type `std::string_view`

#### Usage Patterns

- extracting the first paragraph of markdown documentation for display or logging
- obtaining a plain text summary from markdown strings

### `clore::support::normalize_line_endings`

Declaration: `support/logging.cppm:79`

Definition: `support/logging.cppm:442`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

函数 `clore::support::normalize_line_endings` 通过单次前向遍历输入字符串，将任何形式的 `\r` 换行序列统一转换为单个 `\n` 字符。具体地，当遇到 `\r` 时，它会在结果缓冲区中写入一个 `\n`，如果下一个字符恰好是 `\n`，则跳过该字符以避免产生双倍换行。对于所有其他字符，直接原样复制。实现预先调用 `std::string::reserve` 来分配与输入等长的缓冲区，从而避免多次重新分配。该函数仅依赖标准库字符串操作，无需其他内部模块或外部库。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `text` 参数

#### Writes To

- 局部变量 `normalized`（返回的新字符串）

#### Usage Patterns

- 标准化文本文件中的行结束符
- 统一跨平台换行符为 `\n`

### `clore::support::normalize_path_string`

Declaration: `support/logging.cppm:64`

Definition: `support/logging.cppm:348`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该实现完全依赖标准库的路径规范化设施。函数通过将输入 `std::string_view` 构造为 `std::filesystem::path` 对象，调用 `lexically_normal()` 方法去除冗余的 `..` 和 `.` 组件并折叠分隔符，最后通过 `generic_string()` 输出使用正斜杠的规范化字符串。整个流程无分支或循环，不依赖任何内部辅助函数或日志基础设施。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- Input parameter `path` of type `std::string_view`

#### Writes To

- Return value of type `std::string` containing the normalized path

#### Usage Patterns

- Used in `clore::support::build_compile_signature` to normalize paths before hashing or comparison

### `clore::support::read_utf8_text_file`

Declaration: `support/logging.cppm:85`

Definition: `support/logging.cppm:480`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

函数首先以二进制模式打开指定路径的文件，通过条件检查确保流成功初始化；若打开失败，立即构造一个描述错误的 `std::unexpected` 并返回。随后利用 `std::filesystem::file_size` 获取文件大小，若无错误则使用该值预分配内部 `std::string` 缓冲区以减少多次扩张的开销。实际读取以 8192 字节的固定块为粒度进行，循环调用 `file.read` 并将每次的 `bytes_read` 个字符追加至缓冲区，直至流结束。读取完成后对文件流状态进行验证，若出现硬件错误或非因到达文件末尾导致的失败，则返回表示读取失败的错误。最后，调用 `strip_utf8_bom` 检查并剥离可能存在的 UTF-8 BOM 前缀；只有当实际剥离发生时，才返回剥离后的副本，否则直接返回原始内容。

内部控制流依赖 `std::filesystem::file_size` 的可选大小提示，以及 `strip_utf8_bom` 作为后处理步骤。错误处理完全基于文件流的状态标志和 `std::filesystem` 的错误码。该实现不依赖于其他外部工具或库函数，所有逻辑均在 `clore::support` 命名空间内完成。

#### Side Effects

- 执行文件读取 I/O 操作
- 分配动态内存用于字符串内容

#### Reads From

- path 参数指定的文件
- 文件系统的元数据（文件大小）

#### Writes To

- 返回的 `std::string` 对象（通过 `std::expected` 传递所有权）

#### Usage Patterns

- 读取文本文件内容（如配置、数据文件）
- 用于需要处理 UTF-8 并自动处理 BOM 的场景

### `clore::support::split_cache_key`

Declaration: `support/logging.cppm:73`

Definition: `support/logging.cppm:378`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

函数 `clore::support::split_cache_key` 将缓存键拆分为路径和编译签名两部分。算法首先通过 `std::string_view::rfind` 在缓存键中查找最后一个 `kCacheKeyDelimiter` 分隔符（一个匿名命名空间内的常量）。若未找到，则返回一个包含格式化错误消息的 `std::unexpected`。成功找到后，字符串被分为 `path_part` 和 `signature_part` 两个子串，并对空情况做防御性检查。签名部分通过 `std::from_chars` 解析为 `std::uint64_t`，同时验证解析是否耗尽所有字符且无错误。最终结果以 `CacheKeyParts` 结构体返回，填充 `.path` 和 `.compile_signature` 字段。

控制流高度依赖标准库设施：`std::string_view` 的查找与切片、`std::from_chars` 的数字解析、`std::expected` 与 `std::unexpected` 的错误处理。常量 `kCacheKeyDelimiter` 的取值决定了键内路径与签名的分隔规则，任何格式偏离均导致错误路径。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `cache_key` parameter
- `kCacheKeyDelimiter` constant
- `signature_part` substring for `std::from_chars`

#### Usage Patterns

- used to parse a cache key into path and signature
- called after reading a cache key from storage
- pair with `build_cache_key` to round-trip cache keys

### `clore::support::strip_utf8_bom`

Declaration: `support/logging.cppm:83`

Definition: `support/logging.cppm:470`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

Implementation: [Implementation](functions/strip-utf8-bom.md)

函数 `clore::support::strip_utf8_bom` 通过一次简单的顺序判断移除 UTF-8 字节顺序标记（BOM）。它首先检查输入 `text` 的长度是否足够容纳 BOM 的三个字节，然后逐字节与常量 `kUtf8Bom` 比较。若完全匹配，则返回从 BOM 之后开始的子视图；否则直接返回原视图。整个过程不涉及额外内存分配或复杂分支，依赖仅局限于对该匿名命名空间常量的引用。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- 参数 `text`

#### Usage Patterns

- 用于从文件读取的 UTF-8 文本中剥离 BOM 前缀

### `clore::support::topological_order`

Declaration: `support/logging.cppm:93`

Definition: `support/logging.cppm:547`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该实现采用经典的 Kahn 算法来计算有向无环图的拓扑顺序。函数接收所有节点列表 `nodes`、反向边映射 `reverse_edges`（每个节点指向其所有依赖者）以及每个节点的入度计数 `in_degree`。首先，它构造一个 `std::set` 容器 `ready`，其中包含所有初始入度为 0 或未出现在 `in_degree` 中的节点（即无前置依赖的节点）。随后进入主循环：从 `ready` 中取出当前节点 `current`，将其追加到结果 `order` 中，查询 `reverse_edges` 找到所有依赖 `current` 的节点，对每个依赖者递减其在 `in_degree` 中的计数，若减后入度为 0 则将其插入 `ready`。循环持续至 `ready` 为空。

若最终 `order` 的长度小于 `nodes` 的大小，说明图中存在环，函数返回 `std::nullopt` 表示拓扑排序失败；否则返回 `order` 作为有效的拓扑序列。该函数依赖 `std::unordered_map` 快速查找反向边和入度，依赖 `std::set` 保持就绪节点按字典序输出（确保确定性），并通过引用传递 `in_degree` 以在内部修改其状态。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- nodes (const `std::vector<std::string>`&)
- `reverse_edges` (const `std::unordered_map<std::string, std::vector<std::string>>`&)
- `in_degree` (`std::unordered_map<std::string, int>`)

#### Usage Patterns

- Compute topological order for dependency resolution.
- Detect cycles in a directed graph.

### `clore::support::truncate_utf8`

Declaration: `support/logging.cppm:81`

Definition: `support/logging.cppm:460`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该函数首先委托给 `ensure_utf8` 将输入文本规范化为一个完好形成的 UTF-8 字符串 `normalized`。若 `normalized` 的长度不大于 `max_bytes`，则直接返回它。否则，调用 `utf8_prefix_length` 计算在 `max_bytes` 限制内、不截断任何多字节字符的有效前缀字节数，然后通过 `resize` 截断 `normalized` 至该长度并返回。此设计避免了在截断后产生不完整的 UTF-8 序列，同时依赖 `ensure_utf8` 处理不规则输入，以及依赖 `utf8_prefix_length` 安全地确定合法切分点。

#### Side Effects

- Allocates memory for the returned string via `ensure_utf8` and potentially reallocates on resize

#### Reads From

- `text` parameter
- `max_bytes` parameter

#### Usage Patterns

- Truncating a UTF-8 string to a maximum byte count
- Ensuring the truncated output is valid UTF-8

### `clore::support::write_utf8_text_file`

Declaration: `support/logging.cppm:88`

Definition: `support/logging.cppm:515`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该函数的实现流程首先调用`clore::support::ensure_utf8`对传入的`content`进行规范化，确保其为一个合法的UTF-8序列，结果保存在本地变量`normalized`中。随后，函数以`std::ios::binary`模式构造一个`std::ofstream`对象，尝试打开由`path`指定的文件。若文件无法打开，则立即返回一个携带格式化错误信息的`std::expected<void, std::string>`（通过`std::unexpected`构造），错误消息包含失败原因和通用路径字符串。

在成功打开文件后，函数通过`file.write(normalized.data(), static_cast<std::streamsize>(normalized.size()))`将规范化后的内容完整写入，并调用`file.flush()`确保数据落盘。接着检查流的状态（`!file`），若写入过程中发生错误（如磁盘空间不足），则返回类似的错误`std::unexpected`。若所有步骤均成功，函数返回一个表示成功的`std::expected`（默认构造的空值）。整个过程的控制流是线性的，错误处理通过早期返回来实现，唯一的外部依赖是`clore::support::ensure_utf8`，负责UTF-8合规性检查和可能的替换操作。

#### Side Effects

- Writes content to the file at the given path
- Opens and closes a file
- May allocate memory for the normalized string via `ensure_utf8`

#### Reads From

- `path` parameter (file path)
- `content` parameter (string to write)
- Filesystem state (to open the file)

#### Writes To

- File at the given `path`

#### Usage Patterns

- Writing UTF-8 text files after content normalization
- Replacing raw file writes with UTF-8 validation

## Internal Structure

`clore::support` 模块是日志和通用文本处理的基础设施层，提供 UTF‑8 合规性操作（读取、写入、截断、规范化）、路径/缓存键构造、透明字符串哈希与相等比较，以及 Markdown 段落提取等原子功能。其内部通过匿名命名空间封装 UTF‑8 序列检测和行内标记剥离等实现细节，对外只暴露少量稳定函数与类型（如 `TransparentStringHash`、`CacheKeyParts`、`read_utf8_text_file`、`normalize_path_string`）。`clore::logging` 子模块在此之上构建了基于 `spdlog` 的日志系统，包括全局日志级别变量 `g_log_level`、日志级别代理模板 `LogProxy` 及其 `trace`/`debug`/`info`/`warn`/`err` 实例，以及高层辅助函数如 `cache_hit_rate`、`canonical_log_level_name`。层次关系清晰：`logging` 依赖 `support` 的 UTF‑8 与路径工具，而 `support` 自身仅引入标准库，保持轻量可复用。

