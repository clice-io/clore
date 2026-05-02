---
title: 'Module support'
description: '支撑模块（support）封装了两组紧密关联的基础设施：clore::support 命名空间提供了一系列跨切面的文本处理、文件 I/O 和缓存键管理工具，包括 UTF‑8 校验与截断、路径规范化、换行符统一、BOM 剥离、透明字符串哈希与相等比较，以及编译签名的生成与缓存键的构建；这些工具共同构成了上层模块所依赖的、与平台无关的低级原语。clore::logging 命名空间则暴露了统一的日志接口，包括全局日志级别控制、LogProxy 模板、按严重级别分类的日志变量（trace、debug、info、warn、err）以及直接写入 stderr 的辅助函数；此外还提供了缓存命中率记录等监控手段。整体上，该模块负责为项目提供可复用的基础组件与一致、高效的日志输出框架，所有公开的实现均位于 support/logging.cppm 中，主要依赖标准库与 spdlog 日志库。'
layout: doc
template: doc
---

# Module `support`

## Summary

支撑模块（`support`）封装了两组紧密关联的基础设施：`clore::support` 命名空间提供了一系列跨切面的文本处理、文件 I/O 和缓存键管理工具，包括 UTF‑8 校验与截断、路径规范化、换行符统一、BOM 剥离、透明字符串哈希与相等比较，以及编译签名的生成与缓存键的构建；这些工具共同构成了上层模块所依赖的、与平台无关的低级原语。`clore::logging` 命名空间则暴露了统一的日志接口，包括全局日志级别控制、`LogProxy` 模板、按严重级别分类的日志变量（`trace`、`debug`、`info`、`warn`、`err`）以及直接写入 stderr 的辅助函数；此外还提供了缓存命中率记录等监控手段。整体上，该模块负责为项目提供可复用的基础组件与一致、高效的日志输出框架，所有公开的实现均位于 `support/logging.cppm` 中，主要依赖标准库与 `spdlog` 日志库。

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

`clore::logging::LogProxy` 是一个轻量级的空体结构体，其唯一模板形参 `Level` 作为编译期日志级别常量。它本身不持有任何数据，所有行为通过两个 `operator()` 重载委托给自由函数 `clore::logging::log`。非模板重载直接接受 `std::string_view`，而模板重载接受 `std::format_string<Args...>` 与变长参数包，并通过 `requires (sizeof...(Args) > 0)` 约束强制参数非空；该重载内部先利用 `std::format` 构造格式化字符串，再转发给 `log`。两个重载均无状态且不抛出异常，唯一的不变量是 `clore::logging::log` 的正确调用得以保证。

#### Invariants

- The log level `Level` is fixed at compile time per instantiation
- Every invocation produces exactly one call to `log` with the same `Level`
- Formatting uses `std::format` and requires `Args...` to be formattable

#### Key Members

- Template parameter `spdlog::level::level_enum Level`
- Operator `void operator()(std::string_view msg) const`
- Operator `void operator()(std::format_string<Args...> fmt, Args&&... args) const`

#### Usage Patterns

- Instantiated as type aliases for each log level (e.g., `using InfoProxy = LogProxy<spdlog::level::info>`)
- Used in conjunction with macros or wrapper functions to create level‑specific log calls
- Enables uniform formatting behavior across all severity levels

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

`clore::support::CacheKeyParts` 将缓存键的内部表示分解为两个关键部分：文件路径 `path` 和编译签名 `compile_signature`。`compile_signature` 的默认值为 `0`，表示签名尚未被赋值或无效，这是该结构体的核心不变量——缓存的合法性依赖于此字段是否为非默认值。两个字段共同唯一标识一个编译单元的缓存条目，其中 `path` 记录源文件位置，`compile_signature` 则通过哈希或版本标识符绑定特定编译配置。任何修改必须保持两个字段在逻辑上的一致配对，否则可能导致缓存命中错误。

#### Invariants

- `compile_signature` 默认初始化为 `0`
- 成员变量直接公开访问

#### Key Members

- `path`
- `compile_signature`

#### Usage Patterns

- 作为缓存键的组成部分被其他模块使用
- 通常与 `clore::support::Cache` 相关组件配合

### `clore::support::TransparentStringEqual`

Declaration: `support/logging.cppm:33`

Definition: `support/logging.cppm:33`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

结构体 `clore::support::TransparentStringEqual` 实现了一个透明的字符串相等比较器，其核心设计是通过四个对称的 `operator()` 重载覆盖 `std::string` 和 `std::string_view` 的所有参数组合。每个重载均将参数归一化为 `std::string_view` 表示后进行等价比较，利用 `noexcept` 与 `[[nodiscard]]` 确保异常安全性与返回值检查。类型别名 `is_transparent` 被定义为 `void`，以此为关联容器（如 `std::unordered_map`）提供异构查找支持，从而避免在查找时不必要的 `std::string` 构造。内部无任何数据成员，所有操作均为纯函数式比较。

#### Invariants

- All `operator()` overloads are `const` and `noexcept`
- The equality comparison is symmetric across `std::string` and `std::string_view`
- The function object is stateless with no data members

#### Key Members

- `operator()(std::string_view, std::string_view)`
- `operator()(const std::string&, std::string_view)`
- `operator()(std::string_view, const std::string&)`
- `operator()(const std::string&, const std::string&)`
- `is_transparent` type alias

#### Usage Patterns

- Used as a transparent equality predicate for unordered containers (e.g., `std::unordered_set`) to allow heterogeneous lookup with `std::string_view`
- Can be passed as the `Pred` template argument to `std::unordered_set<std::string, Hash, TransparentStringEqual>`
- Enables use of `find` and similar member functions with `std::string_view` arguments without constructing a temporary `std::string`

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

该结构体定义了一个透明哈希函数对象，专门用于字符串类型。核心实现位于接受 `std::string_view` 的 `operator()` 重载中，它直接调用 `std::hash<std::string_view>` 计算哈希值。其余两个重载（接受 `const std::string&` 或 `const char*`）均通过构造临时 `std::string_view` 对象来委托给该核心重载，从而统一哈希逻辑并避免为每个调用复制底层字符数据。

通过声明 `is_transparent` 类型别名，该结构体启用异构查找：允许在无序关联容器（如 `std::unordered_set` 或 `std::unordered_map`）中将不同字符串类型作为键进行查找，而无需显式转换为精确的键类型。其内部不维护任何可变状态；所有成员函数均为 `constexpr` 且 `noexcept`，从而支持在常量表达式环境中使用并保证无异常抛出。

#### Invariants

- 所有运算符重载均保证不抛出异常（`noexcept`）。
- 对于任何字符串类型输入，哈希值仅依赖于其字符内容和字符顺序，类型转换不影响结果。
- `is_transparent` 类型别名为 `void`，表明该哈希器是透明的。

#### Key Members

- `is_transparent` 类型别名
- `operator()(std::string_view)`
- `operator()(const std::string&)`
- `operator()(const char*)`

#### Usage Patterns

- 用作 `std::unordered_set` 或 `std::unordered_map` 的自定义哈希器。
- 允许使用 `std::string_view` 或 `const char*` 进行查找，无需创建 `std::string` 对象。
- 可与 `std::equal_to<void>` 或类似的透明比较器配合使用。

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

As a `LogProxy` for the debug log level, it is used to emit debug messages. Because it is `constexpr`, it cannot be mutated after initialization.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Used to emit debug-level log messages via its call `operator`

### `clore::logging::err`

Declaration: `support/logging.cppm:128`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

作为 `LogProxy` 模板的特化，`err` 代表日志系统中的错误级别，允许通过流式操作或函数调用语法输出错误消息。

#### Mutation

No mutation is evident from the extracted code.

### `clore::logging::g_log_level`

Declaration: `support/logging.cppm:102`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

该变量被 `clore::logging::log` 和 `clore::logging::stderr_logger` 函数读取，用于决定是否输出或记录特定级别的日志。当 `g_log_level` 被设置时，高于或等于该级别的日志消息才会被处理；未设置时，可能使用默认行为。

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- 在 `clore::logging::log` 中被读取以过滤日志级别
- 在 `clore::logging::stderr_logger` 中被读取以确定输出级别

### `clore::logging::info`

Declaration: `support/logging.cppm:126`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

As a `LogProxy`, `clore::logging::info` is used throughout the codebase by functions such as `clore::logging::cache_hit_rate` to issue info-level log entries. It is typically invoked with a format string and arguments via the `operator()` or `operator<<` methods, participating in the centralized logging infrastructure and respecting the current log level setting.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Called by `clore::logging::cache_hit_rate` to output info-level log messages

### `clore::logging::trace`

Declaration: `support/logging.cppm:124`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

该变量作为 trace 级别日志的代理对象，通过其成员函数参与日志输出逻辑。从声明推断，它可能被用于格式化并发送 trace 级别的日志消息。

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- 作为 trace 级别日志的输出入口

### `clore::logging::warn`

Declaration: `support/logging.cppm:127`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

该变量通过重载的 `operator()` 接收格式化字符串和参数，在内部检查当前日志级别是否允许 `warn` 级别输出，若允许则调用底层日志器（如 `spdlog`）记录消息。它通常与 `clore::logging::g_log_level` 配合，仅在全局日志级别不低于 `warn` 时生效。

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- 使用 `clore::logging::warn("message {}", arg)` 记录警告级别日志
- 在函数 `clore::support::enable_utf8_console` 中可能被调用以输出警告信息

## Functions

### `clore::logging::cache_hit_rate`

Declaration: `support/logging.cppm:138`

Definition: `support/logging.cppm:138`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

函数 `clore::logging::cache_hit_rate` 首先计算 `total = hits + misses`。如果 `total` 为零，则使用 `std::format` 构造包含 `name` 的错误消息并抛出 `std::logic_error`。否则，计算 `rate = (static_cast<double>(hits) * 100.0) / static_cast<double>(total)`，然后调用 `info`（即 `clore::logging::info`，类型为 `LogProxy`）将 `name`、`hits`、`misses` 及 `rate` 格式化为日志消息。内部控制流仅包含一次零值检查和百分比计算，依赖 `clore::logging::info` 日志代理（最终委托给 `clore::logging::log`）以及 `std::format` 进行字符串格式化。

#### Side Effects

- formats and logs a message to the logging system
- may throw `std::logic_error` when total is zero

#### Reads From

- parameter `name`
- parameter `hits`
- parameter `misses`

#### Writes To

- logging output via `info`

#### Usage Patterns

- used to report cache hit rate statistics
- called after cache operations to monitor efficiency

### `clore::logging::log`

Declaration: `support/logging.cppm:104`

Definition: `support/logging.cppm:104`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

Implementation: [Implementation](functions/log.md)

函数 `clore::logging::log` 的实现首先检查可选的全局日志级别 `clore::logging::g_log_level`：若该级别已设置且传入的 `lvl` 低于阈值，则立即返回，避免不必要的格式化开销。否则，直接委托给 `spdlog::default_logger_raw()` 的 `log` 方法，使用 `"{}"` 格式字符串将消息输出。该函数依赖 `spdlog` 库实现底层日志记录，并通过 `g_log_level` 提供静态的日志等级过滤控制。

#### Side Effects

- Logs a message via `spdlog::default_logger_raw()->log()`

#### Reads From

- global variable `g_log_level`
- function parameter `lvl`
- function parameter `msg`

#### Writes To

- spdlog's default logger (log output)

#### Usage Patterns

- Called by `clore::logging::LogProxy::operator()(std::string_view)` to perform the actual logging after constructing the message
- Used directly by other components that need to log with an explicit severity level

### `clore::logging::stderr_logger`

Declaration: `support/logging.cppm:130`

Definition: `support/logging.cppm:130`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

`clore::logging::stderr_logger` 使用 `spdlog::stderr_color_mt` 创建一个具有给定 `name` 的彩色标准错误日志记录器，并将其转换为 `std::string`。然后通过 `spdlog::set_default_logger` 将该记录器设为进程的默认日志记录器。最后，检查全局变量 `g_log_level` 是否持有值；如果持有，则调用 `spdlog::set_level` 将该记录器的日志级别设置为该值。该函数没有显式的错误处理或资源管理——它依赖 `spdlog` 的内部注册机制来管理记录器的生命周期，且全局日志级别的应用是一次性的。

#### Side Effects

- creates a spdlog stderr logger
- sets the created logger as the default logger
- sets the global log level if `g_log_level` has a value

#### Reads From

- `name` parameter
- `g_log_level` global variable

#### Writes To

- spdlog internal default logger
- spdlog log level

#### Usage Patterns

- used for initializing logging to stderr in application startup

### `clore::support::build_cache_key`

Declaration: `support/logging.cppm:70`

Definition: `support/logging.cppm:368`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该函数将规范化路径 `normalized_path` 与编译签名 `compile_signature` 拼接为一个字符串缓存键。内部使用 `kCacheKeyDelimiter` 作为分隔符，通过先预留容量（路径长度加分隔符后加20字节的签名十进制表示）来最小化内存重分配。依赖项包括匿名命名空间中的常量 `kCacheKeyDelimiter` 以及 `std::to_string`。无分支或循环，仅执行一次分配与两次追加操作，实现简洁的键构造。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- The parameter `normalized_path` (type `std::string_view`)
- The parameter `compile_signature` (type `std::uint64_t`)
- The module‑level constant `kCacheKeyDelimiter`

#### Usage Patterns

- Called to build a cache key for a file path and its compilation signature
- Used in conjunction with `split_cache_key` for round‑tripping cache entries

### `clore::support::build_compile_signature`

Declaration: `support/logging.cppm:66`

Definition: `support/logging.cppm:352`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

`clore::support::build_compile_signature` 首先通过 `normalize_path_string` 对 `directory` 进行归一化，然后将其与 `normalized_file` 以及 `arguments` 中的每个字符串依次追加到一个预分配的 `payload` 中，每个字段之间用 `\0` 分隔。该 payload 形成了用于编译签名的原始数据，最后通过 `llvm::xxh3_64bits` 计算并返回一个 `std::uint64_t` 哈希值。该函数不包含分支或异常处理，仅依赖 `normalize_path_string` 进行路径标准化，依赖 `llvm::xxh3_64bits` 进行非加密哈希计算。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `directory`
- parameter `normalized_file`
- parameter `arguments` (vector of strings)

#### Usage Patterns

- generating a unique digest for compilation inputs
- building a cache key for build systems
- identifying identical compile configurations

### `clore::support::canonical_log_level_name`

Declaration: `support/logging.cppm:77`

Definition: `support/logging.cppm:424`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该函数首先检查输入 `value` 是否为空，若为空则直接返回 `std::nullopt`。随后将输入复制到 `normalized` 字符串中，并通过 `std::tolower` 将其中每个字符转换为小写，从而得到一个规范化的小写版本。核心依赖是 `spdlog::level::from_str`，它用于将字符串解析为 `spdlog::level` 枚举值。若解析结果为 `spdlog::level::off` 且规范化后的字符串不是字面量 `"off"`，则说明该字符串不表示任何有效的日志级别，函数返回 `std::nullopt`；否则返回规范化后的小写字符串。注意，`"off"` 本身是合法的日志级别名称，因此需要单独处理以避免被误判为无效。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` parameter

#### Usage Patterns

- Normalizing log level names before using them with spdlog

### `clore::support::enable_utf8_console`

Declaration: `support/logging.cppm:91`

Definition: `support/logging.cppm:534`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该函数仅在 `_WIN32` 平台上编译。它首先调用 `SetConsoleCP(CP_UTF8)` 将输入代码页设为 UTF-8，若失败则通过 `clore::logging::warn` 记录错误码。随后以相同方式调用 `SetConsoleOutputCP(CP_UTF8)` 设置输出代码页，失败时同样记录警告。整个实现依赖 Windows API 和 `clore::logging` 的警告日志功能，无其他算法逻辑或内部状态变更。

#### Side Effects

- Sets the Windows console input code page to UTF-8
- Sets the Windows console output code page to UTF-8
- Logs a warning if `SetConsoleCP` or `SetConsoleOutputCP` fails

#### Reads From

- Return values of `SetConsoleCP` and `SetConsoleOutputCP`
- Error code from `GetLastError()`

#### Writes To

- Windows console input code page
- Windows console output code page
- Log output via `clore::logging::warn`

#### Usage Patterns

- Called early in program initialization to ensure UTF-8 console I/O
- Typically invoked once at startup

### `clore::support::ensure_utf8`

Declaration: `support/logging.cppm:75`

Definition: `support/logging.cppm:405`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

Implementation: [Implementation](functions/ensure-utf8.md)

该函数的核心实现是一个逐字节扫描的修正循环。它从输入 `text` 的起始偏移量开始，每次调用 `clore::support::(anonymous namespace)::valid_utf8_sequence_length` 判断当前偏移处能否解析为一个合法的 UTF‑8 序列。若返回非零长度，则通过 `std::string::append` 将对应的子串原样拷贝至结果 `normalized` 中，并将偏移量前进该长度；否则（返回 0）表示该字节无效，此时向结果中追加一个替换字符（通过常量 `clore::support::(anonymous namespace)::kUtf8Replacement` 获取），并将偏移量仅向前移动一个字节，继续扫描。

该算法的控制流仅依赖循环与条件分支，无递归或外部 I/O。所有 UTF‑8 合法性判断委托给 `valid_utf8_sequence_length`，该函数负责处理单字节、多字节序列的边界检测以及过长的编码、孤立续字节等非法情况。结果字符串通过 `reserve` 预先分配与输入等长的容量，避免重复扩容。

#### Side Effects

- 分配并构造一个新的 `std::string`

#### Reads From

- `text` 参数（`std::string_view`）
- 内部常量 `kUtf8Replacement`

#### Usage Patterns

- 在输出或进一步处理前清理字符串
- 被 `write_utf8_text_file` 和 `truncate_utf8` 调用

### `clore::support::extract_first_plain_paragraph`

Declaration: `support/logging.cppm:62`

Definition: `support/logging.cppm:303`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该函数实现了一种启发式方法，从 Markdown 文本中提取第一个非代码、非标题、非引用、非列表的纯文本段落。内部逐行扫描输入流，维护布尔标志 `in_code_block` 以跳过被三重反引号包围的代码块。对于普通行，先进行首尾空白修剪；若行首匹配 `"#"`、`">"`、`"|"`、`"- "` 或 `"* "` 且尚未开始段落则直接跳过，否则视为段落正文并累积，行间用单个空格连接。当遇到空行且段落非空时，终止收集并跳出循环。最终调用依赖函数 `strip_inline_markdown_text` 移除所有内联格式（如粗体、斜体、链接等），返回纯文本形式的第一个段落。

控制流的关键分支包括：通过 `trimmed.starts_with("\`\`\`")` 切换 ``in_code_block`` 状态；以段落是否非空决定空行或特殊行是否作为终止条件；累积行时使用 `paragraph`.push_back`(' ')` 分隔。该函数不处理嵌套格式或复杂结构，仅依赖 ``strip_inline_markdown_text`` 做后续净化。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `markdown` (a `std::string_view`)

#### Usage Patterns

- Extracts a plain text description from a Markdown‑formatted string, typically for logging or display purposes where formatting is not required.

### `clore::support::normalize_line_endings`

Declaration: `support/logging.cppm:79`

Definition: `support/logging.cppm:442`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

`clore::support::normalize_line_endings` 通过一次线性扫描输入 `text` 完成行尾归一化。算法首先为目标字符串 `normalized` 预分配与输入等长的容量以减少重分配，随后遍历每个字符。遇到回车符 `\r` 时总是追加一个换行符 `\n`；如果其后紧跟一个换行符 `\n`（即 CRLF 序列），则额外递增索引以跳过该字符。其余所有字符原样复制到输出。该函数仅依赖 `std::string` 和 `std::string_view`，不涉及其他外部服务或文件 I/O，保持了非常简洁的内部控制流。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the input parameter `text` of type `std::string_view`

#### Writes To

- the returned `std::string` object

#### Usage Patterns

- normalizing line endings in text read from files or external sources
- preparing text for consistent processing in the `clore::support` module

### `clore::support::normalize_path_string`

Declaration: `support/logging.cppm:64`

Definition: `support/logging.cppm:348`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该函数的实现完全依赖 C++17 标准库中的 `std::filesystem::path`。它首先将输入的 `path` 构造为一个 `std::filesystem::path` 对象，随后调用其 `lexically_normal()` 方法，该方法执行词法规范化，消除路径中的冗余成分（如 `.` 和 `..`）。最后通过 `generic_string()` 将规范化后的路径转换为使用正斜杠分隔的标准格式字符串并返回。整个过程中没有涉及额外的控制流分支或外部依赖，是一种纯粹基于标准库设施的简便实现。

#### Side Effects

- allocates a new `std::string` via `generic_string()`

#### Reads From

- `path` parameter (`std::string_view`)

#### Writes To

- returned `std::string`

#### Usage Patterns

- normalize paths before building compile signature in `build_compile_signature`
- convert path to canonical generic form

### `clore::support::read_utf8_text_file`

Declaration: `support/logging.cppm:85`

Definition: `support/logging.cppm:480`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该函数通过二进制模式打开 `std::ifstream`，失败时返回包含 `std::format` 错误消息的 `std::unexpected`。接着尝试用 `std::filesystem::file_size` 获取文件大小，若成功则对 `content` 调用 `reserve` 以预分配内存。数据以 8192 字节的 `chunk` 循环读取，通过 `file.read` 和 `file.gcount` 追加到 `content`。读取结束后检查流状态，若 `bad` 或非 `eof` 的 `fail` 则返回失败。最后通过依赖函数 `clore::support::strip_utf8_bom` 处理 `content`：若 BOM 存在则返回剥离后的字符串，否则返回原始 `content`。

#### Side Effects

- Reads file content from disk via `std::ifstream`
- Allocates memory for the file content string

#### Reads From

- The `path` parameter (`const std::filesystem::path&`)
- The file on the filesystem identified by `path`

#### Writes To

- The returned `std::expected<std::string, std::string>` object (local to caller)

#### Usage Patterns

- Loading configuration files or input data
- Reading UTF-8 text files for processing in applications
- Utility for reading text files with automatic BOM handling

### `clore::support::split_cache_key`

Declaration: `support/logging.cppm:73`

Definition: `support/logging.cppm:378`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该实现首先在输入的 `cache_key` 中通过 `rfind` 查找最后一个 `kCacheKeyDelimiter` 出现的位置，若未找到则立即返回包含错误消息的 `std::unexpected`，形成快速失败路径。找到分隔符后，将原始视图拆分为 `path_part` 与 `signature_part`，并逐一检查非空；任意部分为空同样返回错误。签名部分随后接受 `std::from_chars` 校验，要求完全转换为 `std::uint64_t`（即 `ec` 为 `std::errc{}` 且消耗所有字符），否则签名格式非法。只有所有检查通过后才构造 `CacheKeyParts` 并存入 `path`（隐式构造为 `std::string`）与 `compile_signature` 返回。整体控制流是线性展开的验证链，无循环或递归，依赖 `std::expected` 实现错误传播，且高度依赖匿名命名空间中的 `kCacheKeyDelimiter` 常量以及 `CacheKeyParts` 结构体。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- 参数 `cache_key`（`std::string_view`）
- 常量 `kCacheKeyDelimiter`（特定字符）
- 通过 `std::from_chars` 读取签名子串的数字表示

#### Usage Patterns

- 在日志或缓存机制中解析统一格式的缓存键以分离路径和签名
- 作为 `build_cache_key` 的反向操作使用

### `clore::support::strip_utf8_bom`

Declaration: `support/logging.cppm:83`

Definition: `support/logging.cppm:470`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

Implementation: [Implementation](functions/strip-utf8-bom.md)

函数 `clore::support::strip_utf8_bom` 在接收到一个 `std::string_view` 后，首先判断其长度是否不小于 UTF-8 BOM 字节序列（即常量 `kUtf8Bom`，对应 `0xEF, 0xBB, 0xBF`）的长度。若满足条件，则依次将输入的前三个字节通过 `static_cast<unsigned char>` 转换为无符号字符，并与 `kUtf8Bom` 中的对应字节进行相等性比较。仅在三个字节全部匹配时，才调用 `text.substr(...)` 跳过前三个字节并返回剩余部分的视图；否则直接返回原始 `text` 视图。该函数不涉及外部依赖，仅依赖同一匿名命名空间内定义的 `kUtf8Bom` 常量。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the parameter `text` of type `std::string_view`
- the internal constant `kUtf8Bom` (presumably a byte array)

#### Usage Patterns

- used by `clore::support::read_utf8_text_file` to remove a BOM before further text processing

### `clore::support::topological_order`

Declaration: `support/logging.cppm:93`

Definition: `support/logging.cppm:547`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

函数 `clore::support::topological_order` 使用 Kahn 算法实现拓扑排序。它首先遍历 `nodes`，对于每个节点，检查其在 `in_degree` 中的条目：如果节点不存在于 `in_degree` 中（即入度隐含为零）或入度为零，则将该节点插入 `std::set<std::string> ready`。`ready` 集合确保始终按字典序处理就绪节点，从而产生确定的排序结果。同时，`order` 向量预留了与 `nodes.size()` 相等的容量。

主循环持续从 `ready` 中取出第一个元素（`current`），将其追加到 `order` 并移除。然后通过 `reverse_edges` 查找以 `current` 为前置的所有依赖者。对于每个依赖者，检查其在 `in_degree` 中的入度：若入度大于零，则递减；若递减后变为零，则将该依赖者插入 `ready`。当循环因 `ready` 为空而终止时，比较 `order.size()` 与 `nodes.size()`。若 `order` 包含全部节点，返回 `order`；否则返回 `std::nullopt`，指示图中存在环。该实现依赖外部提供的 `nodes` 列表、`reverse_edges` 邻接表以及 `in_degree` 计数器，内部仅使用标准库容器和迭代器。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `nodes` parameter
- `reverse_edges` parameter
- `in_degree` parameter

#### Usage Patterns

- Used to obtain a linear order for dependency resolution
- Called when computing compilation order or task scheduling

### `clore::support::truncate_utf8`

Declaration: `support/logging.cppm:81`

Definition: `support/logging.cppm:460`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

函数 `clore::support::truncate_utf8` 首先委托给 `clore::support::ensure_utf8` 对输入字符串进行规范化，以保证其编码为有效的 UTF‑8（通常会将无效序列替换为替换字符）。若规范化后的结果字节数已不超过参数 `max_bytes`，则直接返回该结果；否则，通过 `clore::support::(anonymous namespace)::utf8_prefix_length` 计算出在 `max_bytes` 字节内能容纳的完整码点前缀长度，并以此调用 `std::string::resize` 进行截断后返回。

该实现依赖两个内部辅助函数：`ensure_utf8` 负责清洁输入，确保后续的字节长度判断和截断操作在正确的字符边界上进行；`utf8_prefix_length` 负责从 UTF‑8 字符串中定位最后一个完整码点，从而避免截断产生不完整的字符序列。整个流程遵循“先规范化、再按码点边界截断”的策略，在长度限制与字符完整性之间取得平衡。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the input `text` parameter
- the `max_bytes` parameter
- the result of `ensure_utf8`
- the result of `utf8_prefix_length`

#### Writes To

- the returned `std::string` object (newly allocated)

#### Usage Patterns

- Truncating UTF-8 strings for storage in fixed-size buffers
- Ensuring string length limits while respecting character boundaries
- Normalizing and truncating user input

### `clore::support::write_utf8_text_file`

Declaration: `support/logging.cppm:88`

Definition: `support/logging.cppm:515`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该函数首先将输入内容传递给 `clore::support::ensure_utf8`，以将任意字符串视图规范化为合法的 UTF‑8 序列（处理无效字节序列和 BOM）。随后以二进制模式打开目标文件流，若打开失败则返回包装在 `std::unexpected` 中的格式化错误信息。写入规范化数据后立即刷新缓冲区，并检查流状态；若写入失败同样返回错误。整个流程依赖 `clore::support::ensure_utf8` 作为数据预处理步骤，确保写入的文件内容始终符合 UTF‑8 编码规范。

#### Side Effects

- creates or overwrites the file at the given path
- performs file I/O (write and flush)

#### Reads From

- `path` parameter (filesystem path)
- `content` parameter (string view to write)

#### Writes To

- file specified by `path`
- returned `std::expected<void, std::string>` object

#### Usage Patterns

- writing text content to a file
- persisting string data as UTF-8

## Internal Structure

`support` 模块分为两个主要命名空间：`clore::support` 和 `clore::logging`。`support` 层提供底层工具，包括通用字符串处理（UTF-8 编码验证、BOM 剥离、行尾规范化、截断、Markdown 剥离）、透明哈希/相等仿函数（用于无序容器的异构查找）、文件 I/O（UTF-8 文本文件的读写）、路径规范化、缓存键构造与拆分，以及拓扑排序。`logging` 层基于 `support` 组建日志系统，提供 `LogProxy` 模板（预定义为 `trace`、`debug`、`info`、`warn`、`err` 等全局代理）、全局日志级别控制、以及缓存命中率记录等辅助函数。内部实现采用匿名命名空间封装 UTF-8 验证和序列长度计算等私有细节，并依赖 `spdlog` 库（通过 `spdlog::level::level_enum` 体现）。整体架构清晰地将基础设施与日志抽象分离，`support` 不依赖 `logging`，保持低层独立性。

