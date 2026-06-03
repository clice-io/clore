---
title: 'Module support'
description: '模块 support 提供了两大类功能：clore::support 命名空间下的通用工具集，以及 clore::logging 命名空间下的日志基础设施。通用工具包括缓存键的构建与拆分、文件路径规范化、UTF-8 文本处理（如 BOM 移除、行结束符统一、截断与验证）、文本提取（如从 Markdown 中抽取首个纯文本段落）、以及拓扑排序等辅助函数。日志子系统则定义了日志级别代理（LogProxy 模板）、全局日志级别控制、以及直接写入标准错误或通过 spdlog 输出的日志函数。该模块是代码库中日志与基础文本处理的统一入口，承担了日志记录、编码保障和关键数据规范化等公共责任。'
layout: doc
template: doc
---

# Module `support`

## Summary

模块 `support` 提供了两大类功能：`clore::support` 命名空间下的通用工具集，以及 `clore::logging` 命名空间下的日志基础设施。通用工具包括缓存键的构建与拆分、文件路径规范化、UTF-8 文本处理（如 BOM 移除、行结束符统一、截断与验证）、文本提取（如从 Markdown 中抽取首个纯文本段落）、以及拓扑排序等辅助函数。日志子系统则定义了日志级别代理（`LogProxy` 模板）、全局日志级别控制、以及直接写入标准错误或通过 `spdlog` 输出的日志函数。该模块是代码库中日志与基础文本处理的统一入口，承担了日志记录、编码保障和关键数据规范化等公共责任。

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

`clore::logging::LogProxy` 是一个模板结构体，以 `spdlog::level::level_enum` 类型的 `Level` 为模板参数。它的核心不变量是：每个 `LogProxy` 实例封装了一个固定的日志级别，并且所有日志操作最终都通过 `clore::logging::log` 函数委托，确保输出格式和路由的一致性。内部提供了两个 `operator()` 重载：一个直接接收 `std::string_view` 消息并调用 `log`；另一个是可变参数模板重载，使用 `std::format` 格式化后转发，这要求至少有一个格式化参数。这两个重载都未修改 `LogProxy` 实例状态（`const` 成员函数），因此它们是线程安全的，只要底层的 `log` 函数也是线程安全的。

#### Invariants

- 模板参数 `Level` 必须为合法的 `spdlog::level::level_enum` 值
- 所有 `operator()` 调用均使用固定的 `Level` 级别
- 格式化重载要求参数包非空（`sizeof...(Args) > 0`）

#### Key Members

- `operator()(std::string_view)`
- `operator()(std::format_string<Args...>, Args&&...)`（变参格式化版本）

#### Usage Patterns

- 创建具体级别的实例，如 `static constexpr auto error = LogProxy<spdlog::level::err>{};`
- 像函数一样调用：`error("operation failed: {}", reason)`
- 自动完成格式化并转发到底层 `log` 函数，无需每次指定级别

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

结构体 `clore::support::CacheKeyParts` 由两个数据成员组成：`std::string path` 和一个默认值为 `0` 的 `std::uint64_t compile_signature`。`path` 存储文件路径或标识符字符串，而 `compile_signature` 预期用于携带与该路径关联的编译时校验信息（如哈希或时间戳）。这两个成员共同构成缓存查找键的一部分；其不变量在于 `compile_signature` 应唯一对应于 `path` 所代表的编译单元版本，但结构体本身不强制该关系。默认构造函数确保 `compile_signature` 初始化为零，以表示未设置状态。

#### Invariants

- `compile_signature` 默认值为 `0`，表示未设置或无效签名

#### Key Members

- `path`
- `compile_signature`

#### Usage Patterns

- 直接访问 `path` 和 `compile_signature` 成员以构造缓存键或进行键匹配
- 作为 `clore::support` 命名空间中缓存机制的输入数据单元

### `clore::support::TransparentStringEqual`

Declaration: `src/support/logging.cppm:56`

Definition: `src/support/logging.cppm:56`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该实现定义了一个无状态函数对象，内部不包含任何数据成员。所有四个重载的 `operator()` 均通过将参数转换为 `std::string_view` 并委托给 `std::string_view` 的相等比较来完成实际比较：当任一参数为 `const std::string&` 时，通过显式构造 `std::string_view` 使其退化为视图比较，从而完全避免字符串拷贝。类型别名 `is_transparent` 被定义为 `void`，这是启用异构查找的关键标记。所有 `operator()` 重载均标记为 `noexcept` 和 `[[nodiscard]]`，确保调用不抛出异常且返回值不会被忽略。由于类型没有可变状态，其所有操作均保持天然线程安全。

#### Invariants

- 无内部状态，所有操作纯函数式
- 所有 `operator()` 重载保证不抛出异常
- 比较结果对称且传递，等价于 `std::string_view` 的相等性
- 与 `std::string` 及 `std::string_view` 的混合比较语义一致

#### Key Members

- `is_transparent` 类型别名
- `operator()` 四个重载：`(string_view, string_view)`, `(const string&, string_view)`, `(string_view, const string&)`, `(const string&, const string&)`

#### Usage Patterns

- 作为自定义比较器用于 `std::unordered_set` 或 `std::unordered_map`，配合透明哈希函数实现异质查找
- 在需要高效字符串键比较且避免不必要 `std::string` 构造的场景中使用

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

`clore::support::TransparentStringHash` 是一个透明的哈希函数对象，通过 `is_transparent` 类型别名启用异构查找。其内部实现围绕一个核心重载展开：接受 `std::string_view` 并直接委托给 `std::hash<std::string_view>`。其余两个重载（接受 `const std::string&` 和 `const char*`）均通过将参数隐式或显式构造为 `std::string_view` 后，再调用同一核心重载。这种设计确保了所有输入类型产生一致的哈希值，且所有重载均标记为 `noexcept`。结构体维护的不变量是：对于任何在逻辑上等价的字符串内容（无论以 `std::string`、`const char*` 还是 `std::string_view` 形式传入），`operator()` 均返回相同的哈希结果。

#### Invariants

- 所有 `operator()` 重载都是 `noexcept` 的
- 哈希计算完全基于 `std::hash<std::string_view>` 实现，无自定义状态或额外逻辑
- 接受 `const std::string&` 和 `const char*` 的参数会隐式转换为 `std::string_view`
- 类型别名 `is_transparent` 为 `void`，表明支持透明查找

#### Key Members

- `is_transparent` 类型别名
- `operator()(std::string_view) const noexcept`
- `operator()(const std::string&) const noexcept`
- `operator()(const char*) const noexcept`

#### Usage Patterns

- 作为 `std::unordered_set<std::string, TransparentStringHash>` 或 `std::unordered_map<std::string, T, TransparentStringHash>` 的哈希器，允许使用 `std::string_view` 或 `const char*` 进行查找
- 与其他支持透明哈希的容器或算法配合，例如 `std::unordered_set<std::string, TransparentStringHash>::find(std::string_view)`
- 被用作代码库中需要字符串哈希且支持异构查找的通用组件

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

As a `LogProxy`, it exposes a call `operator` that accepts a format string and arguments, enabling conditional emission of debug log entries. Its level is fixed to `spdlog::level::debug`, and it participates in the logging system alongside other level-specific proxies. The `debug` variable is not mutated after initialization.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- calling `debug()` with a format string and variadic arguments
- used in conditional log emission checks based on global log level

### `clore::logging::err`

Declaration: `src/support/logging.cppm:151`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

As a `LogProxy` specialized for `spdlog::level::err`, `clore::logging::err` provides a mechanism to log messages with error severity. It is typically used in logging statements by calling `operator()` or stream `operator`s, and it participates in conditional logging based on the current log level (`clore::logging::g_log_level`). The variable itself is never mutated after initialization.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- used to log error-level messages via `LogProxy::operator()` or stream insertion
- compared against global log level `clore::logging::g_log_level` to conditionally emit output

### `clore::logging::g_log_level`

Declaration: `src/support/logging.cppm:125`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

This variable is read by logging functions such as `clore::logging::log` and `clore::logging::stderr_logger` to decide whether a log message of a given severity should be output. It participates in filtering logic by comparing the log level of incoming messages against `clore::logging::g_log_level`.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- read by `clore::logging::log`
- read by `clore::logging::stderr_logger`
- compared against message level

### `clore::logging::info`

Declaration: `src/support/logging.cppm:149`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

This variable is used by the function `clore::logging::cache_hit_rate` to log messages at the info severity level. Being `constexpr` and `inline`, it is a compile-time constant that provides a lightweight interface for writing info-level log entries. Its usage likely involves the `operator()` or stream `operator`s defined by `LogProxy`.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Used as a log proxy in `clore::logging::cache_hit_rate` to emit info-level log messages.

### `clore::logging::trace`

Declaration: `src/support/logging.cppm:147`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

该变量作为 `LogProxy` 模板实例，专门用于生成日志级别为 `trace` 的日志条目。它参与日志系统的条件判断，在启用跟踪级别时输出日志。

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- 用于记录跟踪级别的日志信息

### `clore::logging::warn`

Declaration: `src/support/logging.cppm:150`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

作为日志框架中的静态实例，`clore::logging::warn` 通过 `LogProxy` 的调用运算符接受消息和格式化参数，并将日志输出到配置的后端。由于是 `constexpr`，其行为在编译时已知，可用于死代码消除等优化。

#### Mutation

No mutation is evident from the extracted code.

## Functions

### `clore::logging::cache_hit_rate`

Declaration: `src/support/logging.cppm:161`

Definition: `src/support/logging.cppm:161`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

该实现首先计算 `total` 为 `hits` 与 `misses` 之和，若 `total` 为零则抛出 `std::logic_error`，确保执行不会在无数据的空状态下继续。随后将 `hits` 转换为 `double` 后乘以 100.0，再除以 `total` 以得到百分比形式的有符号命中率 `rate`，该计算避免了整数除法带来的精度损失。最后，函数调用全局的 `clore::logging::info` 日志代理（`clore::logging::LogProxy` 的 `operator()` 重载），使用格式化字符串 `"{} cache: {} hits, {} misses ({:.1f}%)"` 依次输出指标名称、命中次数、未命中次数和命中率。

内部控制流极为直接：仅一条条件分支检查总和为零并抛异常，其余为计算与日志输出。依赖方面，函数依赖标准库的 `std::logic_error` 和 `std::format` 进行异常构造，以及 `clore::logging::info` 变量及其所属的 `clore::logging::LogProxy` 类型提供的格式化日志能力。外部符号均来自同一命名空间中的日志基础设施。

#### Side Effects

- Throws `std::logic_error` if `hits` and `misses` sum to zero.
- Emits a log message via `info` which writes to the configured logging sink(s).

#### Reads From

- Parameter `name` (`std::string_view`)
- Parameter `hits` (`std::size_t`)
- Parameter `misses` (`std::size_t`)

#### Writes To

- Logging output (via `info` call)

#### Usage Patterns

- Monitoring cache performance
- Debugging cache efficiency
- Reporting hit/miss statistics

### `clore::logging::log`

Declaration: `src/support/logging.cppm:127`

Definition: `src/support/logging.cppm:127`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

Implementation: [Implementation](functions/log.md)

`clore::logging::log` 的实现首先检查静态可选变量 `g_log_level` 是否已设置，并比较传入的 `lvl` 参数是否低于该阈值；如果低于，函数立即返回，不执行任何日志输出。否则，函数调用 `spdlog::default_logger_raw()->log(lvl, "{}", msg)`，将原始格式化字符串 `msg` 传递给 spdlog 的默认日志记录器进行输出。该实现完全依赖于 spdlog 库的日志基础设施，并在调用前执行一次快速的优先级过滤。

#### Side Effects

- 写入日志输出（通过 spdlog）
- 读取并比对全局日志级别 `g_log_level`

#### Reads From

- `g_log_level` 全局变量
- `lvl` 参数
- `msg` 参数

#### Writes To

- 日志输出（由 spdlog 管理）

#### Usage Patterns

- 被 `clore::logging::LogProxy::operator()` 重载调用
- 用于在代理对象中转发日志消息并执行级别过滤

### `clore::logging::stderr_logger`

Declaration: `src/support/logging.cppm:153`

Definition: `src/support/logging.cppm:153`

Declaration: [`Namespace clore::logging`](../../namespaces/clore/logging/index.md)

函数 `clore::logging::stderr_logger` 通过调用 `spdlog::stderr_color_mt` 并以传入的 `name` 构造的 `std::string` 为参数，在标准错误上创建一个带颜色的日志记录器。随后使用 `spdlog::set_default_logger` 将该记录器设为全局默认日志记录器。如果全局变量 `g_log_level` 包含值，则通过 `spdlog::set_level` 应用该日志级别。该函数没有分支或循环，其核心依赖是 spdlog 库的日志记录器创建与配置接口，以及模块级的 `g_log_level` 状态。

#### Side Effects

- 创建并注册一个 spdlog 日志器到标准错误流
- 将创建的日志器设为 spdlog 默认日志器
- 如果全局日志级别存在则设置 spdlog 日志级别

#### Reads From

- 参数 `name`
- 全局变量 `g_log_level`

#### Writes To

- spdlog 全局默认日志器
- spdlog 全局日志级别
- 标准错误流 (通过 spdlog)

#### Usage Patterns

- 在日志系统初始化时调用以设置 stderr 日志器为默认
- 用于替换或配置全局默认日志器为 stderr 输出

### `clore::support::build_cache_key`

Declaration: `src/support/logging.cppm:93`

Definition: `src/support/logging.cppm:391`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

函数 `clore::support::build_cache_key` 的实现通过字符串拼接构建缓存键。它首先构造一个 `std::string key`，并提前预留 `normalized_path.size() + 1 + 20` 字节的容量（其中 `+1` 为定界符，`+20` 为 `std::uint64_t` 的十进制表示最大长度）。然后依次追加 `normalized_path`、匿名命名空间常量 `kCacheKeyDelimiter` 以及通过 `std::to_string` 转换后的 `compile_signature` 字符串，最后返回 `key`。整个过程没有分支或循环，完全线性；依赖仅限于标准字符串操作和该翻译单元内定义的 `kCacheKeyDelimiter`。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `normalized_path` (parameter)
- `compile_signature` (parameter)

#### Writes To

- The returned `std::string` (local allocation)

#### Usage Patterns

- Used to generate a cache key from a normalized file path and a compile signature
- Called before storing or retrieving cached compile results

### `clore::support::build_compile_signature`

Declaration: `src/support/logging.cppm:89`

Definition: `src/support/logging.cppm:375`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

函数通过连接目录、规范化文件路径和编译器参数来构造一个唯一字节序列，然后使用 `llvm::xxh3_64bits` 计算该序列的 64 位哈希。内部控制流非常线性：首先调用 `clore::support::normalize_path_string` 规范化目录字符串，并在结果后追加一个空字符；接着追加 `normalized_file` 后同样追加空字符；随后遍历 `arguments` 中的每个参数，依次追加并追加空字符。整个负载事先调用 `payload.reserve` 分配足够容量以避免重分配。该函数唯一的外部依赖是 `clore::support::normalize_path_string`，用于生成跨平台一致的目录表示。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `directory`
- `normalized_file`
- `arguments`
- `clore::support::normalize_path_string` (reads its `std::string_view` argument)

#### Usage Patterns

- Called to produce a compile signature for caching compile results
- Used together with `clore::support::build_cache_key` to form a cache key

### `clore::support::canonical_log_level_name`

Declaration: `src/support/logging.cppm:100`

Definition: `src/support/logging.cppm:447`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

函数首先处理空输入：当参数 `value` 为空时立即返回 `std::nullopt`。正常路径将输入复制到 `std::string normalized`，然后通过 `std::tolower` 逐字符转换为小写，完成归一化。随后利用 `spdlog::level::from_str` 尝试将归一化后的字符串解析为日志级别枚举；若结果等于 `spdlog::level::off` 且归一化字符串本身不是 `"off"`，则判定为无效级别名称，返回 `std::nullopt`。所有有效级别均返回归一化后的小写字符串。

该实现的正确性依赖 `spdlog` 库的 `from_str` 函数对日志级别名称的识别规则，以及 `std::tolower` 在 `"C"` 本地化下的行为（仅处理 ASCII 字母）。函数不验证传入字符串是否完全由字母组成，而是将规范化后的形式委托给 `spdlog` 进行语义匹配。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- Parameter `value`

#### Usage Patterns

- Canonicalizing user-provided log level strings
- Validating log level names before configuration

### `clore::support::enable_utf8_console`

Declaration: `src/support/logging.cppm:114`

Definition: `src/support/logging.cppm:557`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该函数在 Windows 平台上通过调用 Windows API 函数 `SetConsoleCP` 和 `SetConsoleOutputCP` 将控制台的输入和输出代码页均设置为 `CP_UTF8`。每次调用后均检查返回值：若返回零表示失败，则通过 `clore::logging::warn` 记录一条包含 `GetLastError` 返回值的警告日志。整个实现受预处理器宏 `_WIN32` 保护，非 Windows 平台下函数体为空，不执行任何操作。其直接依赖项为 Windows 系统 API 以及 `clore::logging` 命名空间下的日志记录函数。

#### Side Effects

- Changes the console input code page to UTF-8 via `SetConsoleCP`
- Changes the console output code page to UTF-8 via `SetConsoleOutputCP`
- Logs a warning message when a code page setting fails using `clore::logging::warn`

#### Reads From

- Return value of `GetLastError()` when a code page setting call fails

#### Writes To

- Console input code page (set to `CP_UTF8`)
- Console output code page (set to `CP_UTF8`)
- Log output (warning message) via `clore::logging::warn` on failure

#### Usage Patterns

- Called during application initialization to enable UTF-8 console I/O on Windows
- Invoked once at program startup to ensure console handles UTF-8 encoding

### `clore::support::ensure_utf8`

Declaration: `src/support/logging.cppm:98`

Definition: `src/support/logging.cppm:428`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

Implementation: [Implementation](functions/ensure-utf8.md)

该函数遍历输入的 `text` 字符串，在每个偏移位置调用 `valid_utf8_sequence_length` 确定当前字节序列的 UTF-8 编码长度。若返回零，说明该字节不是一个有效序列的起始字节，此时向结果 `normalized` 追加替换字符 `kUtf8Replacement` 并将偏移量前进一个字节；否则将有效序列直接追加到 `normalized` 中，偏移量前进相应的序列长度。整个过程仅依赖于辅助函数 `valid_utf8_sequence_length`，通过逐字节检查来替换无效字节，从而保证输出始终是合法的 UTF-8 字符串。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `text` parameter
- `kUtf8Replacement` constant

#### Writes To

- local variable `normalized` (returned)

#### Usage Patterns

- Normalizing input strings to ensure valid UTF-8 encoding
- Used by `write_utf8_text_file` to guarantee valid UTF-8 before writing
- Used by `truncate_utf8` to ensure truncated string ends at a valid boundary

### `clore::support::extract_first_plain_paragraph`

Declaration: `src/support/logging.cppm:85`

Definition: `src/support/logging.cppm:326`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该函数逐行扫描输入的 Markdown 文本，通过 `in_code_block` 状态机跳过代码块内的行，并过滤掉以 `#`、`>`、`|`、`-` 或 `*` 开头的块级标记行。当遇到连续的非空文本行时，它们会被累积到 `paragraph` 中，直到遇到空行或新的块级标记行为止。累积的文本最终会调用 `clore::support::(anonymous namespace)::strip_inline_markdown_text` 去除内联 Markdown 格式（如链接、粗体、行内代码等），并将结果作为第一个段落的纯文本返回。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `markdown` parameter (a `std::string_view` representing Markdown input)

#### Usage Patterns

- Used to obtain a plain text summary from Markdown documentation or comments
- Applied to extract the first paragraph for previews or search indexing

### `clore::support::normalize_line_endings`

Declaration: `src/support/logging.cppm:102`

Definition: `src/support/logging.cppm:465`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

函数 `clore::support::normalize_line_endings` 通过一次线性扫描将输入文本中的 CRLF（`\r\n`）和单独的 CR（`\r`）统一转换为 LF（`\n`）。算法维护一个索引 `index` 逐字符遍历 `text`：若当前字符为 `\r`，则向结果字符串 `normalized` 追加一个 `\n`，并检查下一个字符是否为 `\n`，若是则额外跳过该字符（即消耗这一对 `\r\n`）；否则直接复制当前字符。该实现仅依赖 `std::string` 和 `std::string_view`，无其他内部或外部函数调用。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- text (parameter of type `std::string_view`)

#### Writes To

- local `normalized` string (returned by value)

#### Usage Patterns

- Normalizing line endings before further text processing or file output
- Used in pipelines that require consistent line ending style (e.g., before parsing or comparison)

### `clore::support::normalize_path_string`

Declaration: `src/support/logging.cppm:87`

Definition: `src/support/logging.cppm:371`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

Implementation: [Implementation](functions/normalize-path-string.md)

实现依赖于标准库 `std::filesystem::path` 的规范化能力。函数将输入的 `std::string_view` 构造为 `std::filesystem::path` 对象，随即调用 `lexically_normal()` 以消除路径中的 `.`、`..` 及多余分隔符，最后通过 `generic_string()` 将结果转换为使用正斜杠的通用字符串表示。整个流程不依赖任何外部库，仅使用 C++17 文件系统设施，且不对输入做额外校验或错误处理。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- 参数 `path`

#### Usage Patterns

- 用于构建编译签名时规范化路径字符串
- 在 `clore::support::build_compile_signature` 中被调用

### `clore::support::read_utf8_text_file`

Declaration: `src/support/logging.cppm:108`

Definition: `src/support/logging.cppm:503`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该函数以二进制模式打开文件流，并在打开失败时返回包含错误信息的 `std::unexpected`。成功打开后，它使用 `std::filesystem::file_size` 获取文件大小以预分配目标字符串的内存，若无法获取大小则跳过该优化。接着通过一个 8192 字节的缓冲区循环读取文件内容，逐块追加到字符串中，并检查读取失败或坏位。读取完成后，它调用依赖函数 `clore::support::strip_utf8_bom` 去除可能存在的 UTF-8 BOM；若 BOM 存在则返回去除了 BOM 的子串视图构造的新字符串，否则直接返回原始内容。该实现不进行 UTF-8 有效性验证，仅处理 BOM 移除。

#### Side Effects

- reads a file from the filesystem
- allocates memory for the string content
- strips UTF-8 BOM if present

#### Reads From

- filesystem via the `path` parameter
- file content

#### Writes To

- returned `std::expected` object containing either a string or an error message

#### Usage Patterns

- reading configuration or resource files
- loading source code files
- importing UTF-8 text data from the filesystem

### `clore::support::split_cache_key`

Declaration: `src/support/logging.cppm:96`

Definition: `src/support/logging.cppm:401`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

函数 `clore::support::split_cache_key` 首先通过 `rfind(kCacheKeyDelimiter)` 从 `cache_key` 末尾定位最后一个分隔符，若未找到则返回错误。随后将字符串在分隔符处拆分为 `path_part` 与 `signature_part`，并验证两部分均非空。签名部分使用 `std::from_chars` 解析为 `std::uint64_t` 的 `signature`，若解析失败或未完全消耗输入则返回错误。成功时构造并返回 `CacheKeyParts`，其中 `path` 为路径字符串，`compile_signature` 为解析后的哈希值。

该函数依赖匿名命名空间中的常量 `kCacheKeyDelimiter` 与结构体 `CacheKeyParts`，其内部控制流完全通过字符串视图操作和数字转换实现，不涉及额外 I/O 或外部状态。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `cache_key` parameter (`std::string_view`)

#### Writes To

- returned `CacheKeyParts` object
- temporary `std::expected` and `std::unexpected` objects

#### Usage Patterns

- Splitting a cache key into path and compile signature
- Validating cache key format and parsing signature
- Used in conjunction with `build_cache_key` for cache key decomposition

### `clore::support::strip_utf8_bom`

Declaration: `src/support/logging.cppm:106`

Definition: `src/support/logging.cppm:493`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

Implementation: [Implementation](functions/strip-utf8-bom.md)

函数 `clore::support::strip_utf8_bom` 的算法实现简洁直接：首先检查输入字符串视图 `text` 的长度是否至少等于 UTF‑8 BOM 序列的长度（由常量 `kUtf8Bom` 定义），然后逐一比较前三个字节与 BOM 的字节值。若完全匹配，则通过 `text.substr(std::size(kUtf8Bom))` 返回去掉前导 BOM 后的子视图；否则直接返回原始 `text`。内部控制流仅为一个条件分支，无循环或递归。该函数依赖于匿名命名空间中的常量 `kUtf8Bom` 来获取 BOM 字节序列，并依赖标准库的 `std::string_view` 及其 `substr` 成员方法执行视图裁剪。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `text` parameter
- `kUtf8Bom` constant

#### Usage Patterns

- Stripping BOM from text loaded by `clore::support::read_utf8_text_file`

### `clore::support::topological_order`

Declaration: `src/support/logging.cppm:116`

Definition: `src/support/logging.cppm:570`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

该函数实现了 Kahn 算法，基于入度对节点进行拓扑排序。它接受三个参数：节点列表 `nodes`、以被依赖节点为键的逆向依赖映射 `reverse_edges` 以及初始入度映射 `in_degree`。算法首先遍历所有节点，将入度为 0 或未在 `in_degree` 中出现的节点插入一个 `std::set<std::string>` 类型的就绪集合 `ready` 中，以保持确定性顺序。随后循环从 `ready` 中取出首元素加入结果 `order`，并查询 `reverse_edges` 找到所有依赖于该节点的后继。对于每个后继，在 `in_degree` 中递减其入度，若减至 0 则将其加入 `ready`。循环结束时，若 `order` 大小小于 `nodes` 大小，代表存在依赖环，返回 `std::nullopt`；否则返回完整的 `order` 向量。内部依赖仅涉及标准库容器和 `std::optional`。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `nodes` parameter - the list of all node identifiers
- `reverse_edges` parameter - mapping from each node to its dependents
- `in_degree` parameter - the in-degree counts for each node (passed by value, so a local copy is read and modified)

#### Writes To

- local `ready` set
- local `order` vector
- local copy of `in_degree`

#### Usage Patterns

- Used to compute a topological ordering of compilation units or graph nodes
- Called with dependency information to detect cycles
- Suitable for build systems and task scheduling

### `clore::support::truncate_utf8`

Declaration: `src/support/logging.cppm:104`

Definition: `src/support/logging.cppm:483`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

函数 `clore::support::truncate_utf8` 的实现首先调用 `ensure_utf8` 将输入的 `std::string_view text` 规范化为合法的 UTF‑8 字符串（替换非法字节序列）。如果规范化后的 `normalized` 字节长度不超过 `max_bytes`，则直接返回。否则，通过匿名命名空间中的 `utf8_prefix_length` 计算出在 `max_bytes` 字节内能够完整容纳的 UTF‑8 字符前缀长度，然后调用 `normalized.resize` 进行截断，确保结果不会在字符边界处断开。

该函数仅依赖于 `ensure_utf8`（提供输入清洗）和 `utf8_prefix_length`（负责从给定字节偏移往前对齐到最近的 UTF‑8 字符起始位置）。整个控制流为简单的顺序结构加一次条件判断，核心逻辑集中在 `utf8_prefix_length` 的边界对齐计算上。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `std::string_view text`
- parameter `std::size_t max_bytes`
- `clore::support::ensure_utf8` reads from `text`

#### Usage Patterns

- Truncating UTF-8 strings to fit within a byte limit
- Ensuring valid UTF-8 boundaries after truncation
- Preprocessing strings before storage or display

### `clore::support::write_utf8_text_file`

Declaration: `src/support/logging.cppm:111`

Definition: `src/support/logging.cppm:538`

Declaration: [`Namespace clore::support`](../../namespaces/clore/support/index.md)

函数首先调用 `clore::support::ensure_utf8` 对传入的内容进行规范化处理，以确保其符合 UTF-8 编码标准。随后，以二进制模式打开目标文件（`std::ofstream` 配合 `std::ios::binary`），若打开失败则返回包含错误信息的 `std::unexpected`。成功打开后，将规范化后的数据一次性写入文件，并立即刷新流。若写入或刷新操作失败（通过 `!file` 检测），同样返回带有描述性错误信息的 `std::unexpected`；全部成功则返回空值。整个过程仅依赖 `clore::support::ensure_utf8` 这一辅助函数，无分支或循环，控制流为线性的顺序执行。

#### Side Effects

- writes content to a file on disk
- allocates a temporary normalized string via `ensure_utf8`

#### Reads From

- parameter `path`: target file path
- parameter `content`: string to write
- file system state (via file open check)

#### Writes To

- file at `path` on disk

#### Usage Patterns

- used to write UTF-8 text files
- complement to `clore::support::read_utf8_text_file`
- called with a path and a string view of the content to persist

## Internal Structure

`support` 模块作为核心基础库，其内部划分为两个主要功能域：通用字符串与路径处理工具（位于 `clore::support` 命名空间）以及类型安全的日志记录系统（位于 `clore::logging` 命名空间）。该模块对外部库的依赖仅限于标准库和 `spdlog`（日志级别枚举），其余功能均通过纯 C++20 标准库实现。在内部，`support` 域通过匿名命名空间封装了多个私有辅助函数（如 UTF‑8 有效性检测、内联 Markdown 剥离），对外仅暴露 `TransparentStringHash`、`TransparentStringEqual`、`CacheKeyParts` 等数据结构和 `normalize_path_string`、`read_utf8_text_file`、`topological_order` 等算法函数。这些工具被上层的日志系统直接使用：例如 `cache_hit_rate` 通过 `log` 输出格式化消息，而 `LogProxy` 模板则提供了编译期日志级别代理，最终调用 `clore::logging::log` 进行实际输出。整体上，模块遵循清晰的职责分离——基础字符串/路径处理位于较低层次，日志系统构建在其之上，全局日志级别 `g_log_level` 则作为可配置的过滤阈值贯穿整个日志流程。

