---
title: 'Module generate:cache'
description: 'clore::generate::cache 模块负责为 LLM 生成请求的响应提供持久化缓存机制。它公开了生成缓存键、同步/异步加载与存储缓存索引、查找已缓存响应以及规范化文本用于散列的接口，使得调用方可以避免重复的请求处理。该模块依赖于 protocol 和 support 模块，通过 CacheIndex 结构体管理条目，并使用 CacheError 报告操作中的错误。公共实现范围包括 make_prompt_response_cache_key、save_cache_entry、save_cache_entry_async、load_cache_index、load_cache_index_async、find_cached_response 和 normalize_text_for_hashing 等函数。'
layout: doc
template: doc
---

# Module `generate:cache`

## Summary

`clore::generate::cache` 模块负责为 LLM 生成请求的响应提供持久化缓存机制。它公开了生成缓存键、同步/异步加载与存储缓存索引、查找已缓存响应以及规范化文本用于散列的接口，使得调用方可以避免重复的请求处理。该模块依赖于 `protocol` 和 `support` 模块，通过 `CacheIndex` 结构体管理条目，并使用 `CacheError` 报告操作中的错误。公共实现范围包括 `make_prompt_response_cache_key`、`save_cache_entry`、`save_cache_entry_async`、`load_cache_index`、`load_cache_index_async`、`find_cached_response` 和 `normalize_text_for_hashing` 等函数。

## Imports

- [`protocol`](../protocol/index.md)
- [`support`](../support/index.md)

## Imported By

- [`generate:scheduler`](scheduler.md)

## Types

### `clore::generate::cache::CacheError`

Declaration: `src/generate/cache.cppm:35`

Definition: `src/generate/cache.cppm:35`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

`clore::generate::cache::CacheError` 仅包含一个 `std::string message` 成员，用于存储与缓存操作相关的错误描述。结构体未定义任何构造函数或赋值操作符，完全依赖于编译器生成的默认实现，因此 `message` 的初始状态为空字符串，且在复制或移动时遵循 `std::string` 的常规语义。不变量方面，`message` 的内容在构造后既可保持为空（表示未指定错误）也可被赋值为具体文本，但结构体本身不对其进行约束或验证。

#### Key Members

- 成员 `message`：存储错误描述文本。

### `clore::generate::cache::CacheIndex`

Declaration: `src/generate/cache.cppm:39`

Definition: `src/generate/cache.cppm:39`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

`clore::generate::cache::CacheIndex` 结构体内部完全由一个 `std::unordered_map<std::string, std::string>` 类型的字段 `entries` 构成，实现了一个轻量级的键‑值缓存索引。其核心不变量是 `entries` 中的每个键唯一，且键和值均为有效的 `std::string` 实例，借此将缓存键直接映射到对应的缓存条目（例如文件路径或序列化内容）。该结构体不引入额外开销或生命周期管理，完全依赖标准库的哈希映射提供快速的插入与查找操作，并作为缓存系统的核心索引结构。

#### Invariants

- Keys in `entries` are unique per instance.
- No ordering of elements is guaranteed.
- Both keys and values are mutable after construction.

#### Key Members

- `entries` member of type `std::unordered_map<std::string, std::string>`

#### Usage Patterns

- Accessed and modified via the `entries` member.
- Likely used as a cache container for generated output or intermediate data.
- No special methods or inheritance observed.

## Functions

### `clore::generate::cache::find_cached_response`

Declaration: `src/generate/cache.cppm:54`

Definition: `src/generate/cache.cppm:366`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

该函数通过在 `CacheIndex` 结构的 `entries` 映射中以 `cache_key` 转换后的 `std::string` 进行查找来检索缓存响应。若映射中包含该键，则返回其对应的 `std::string_view` 值；否则返回 `std::nullopt`。整个流程仅依赖 `CacheIndex` 类型中 `entries` 成员的数据结构（预期为关联容器）以及 `std::string` 从 `cache_key` 的隐式转换，无需额外 I/O 或异步操作。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `CacheIndex` parameter `index` (specifically its `entries` member)
- the `cache_key` parameter

#### Usage Patterns

- lookup cached response by key
- check if response exists in cache
- used before generation to avoid redundant computation

### `clore::generate::cache::load_cache_index`

Declaration: `src/generate/cache.cppm:48`

Definition: `src/generate/cache.cppm:271`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

函数首先通过调用 `all_jsonl_files` 获取指定工作空间根目录下所有 JSONL 文件的路径列表，若此调用失败则直接返回错误。接着遍历每个文件，使用 `clore::support::read_utf8_text_file` 读取完整文本内容；若读取失败则跳过该文件。成功读取后，将文本按行分割，对每一非空行调用 `kota::codec::json::parse` 解析为 JSON 对象。从每个解析成功的对象中依次提取 `"key"` 和 `"resp"` 字段，验证它们存在且为有效字符串后，将键值对插入到 `CacheIndex` 结构体的 `entries` 映射中。所有处理步骤中遇到的任何错误（文件缺失、JSON 格式错误、字段缺失等）均被忽略，函数继续处理后续内容。

内部控制流完全由循环和提前返回组成：`all_jsonl_files` 的结果是第一个错误抑点；随后两层 `for` 循环（外层遍历文件，内层遍历行）中的每个失败步骤均使用 `continue` 跳过当前条目或文件，确保单点失败不影响整体。函数不依赖外部可变状态，仅依赖文件系统中按约定放置的 JSONL 文件；关键依赖包括 `clore::support::read_utf8_text_file`（文本读取）、`kota::codec::json::parse`（JSON 解析）以及 `all_jsonl_files`（目录扫描）。最终返回的 `CacheIndex` 包含了所有有效缓存条目的键值映射。

#### Side Effects

- Reads files from the filesystem via `all_jsonl_files` and `read_utf8_text_file`
- Allocates memory for the constructed `CacheIndex` and its entries

#### Reads From

- Parameter `workspace_root`
- JSONL files discovered in the workspace root directory
- File contents read by `clore::support::read_utf8_text_file`

#### Writes To

- The `CacheIndex` object returned (specifically its `entries` member)

#### Usage Patterns

- Called to load the cache index before performing cache lookups
- Typically invoked once at startup or when a cache reload is needed
- Used in conjunction with `find_cached_response` to query cached results

### `clore::generate::cache::load_cache_index_async`

Declaration: `src/generate/cache.cppm:57`

Definition: `src/generate/cache.cppm:375`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

`load_cache_index_async` 是一个基于 `kota::task` 的协程,它通过内部使用 `kota::queue` 将对同步函数 `load_cache_index` 的调用派发到给定的 `kota::event_loop` 上异步执行。首先将 `load_cache_index(workspace_root)` 包装为一个无参数 lambda 并通过 `kota::queue` 提交到事件循环,然后 `co_await` 其结果。如果队列任务本身失败(即 `queued_result.has_error()` 为真),则构造一个包含格式化错误消息的 `CacheError` 并调用 `co_await kota::fail` 抛出该错误。若队列任务成功返回 `std::expected<CacheIndex, CacheError>`,则检查其是否含有错误值:若 `queued_result->has_value()` 为假,则通过 `kota::fail` 传播内部错误。否则正常返回内部的 `CacheIndex` 值。整个函数实质上是对同步版本 `load_cache_index` 的异步封装,其错误处理链完整覆盖了调用调度的异常和 `load_cache_index` 自身的错误。

#### Side Effects

- 通过 `kota::queue` 提交同步函数到事件循环，可能触发文件 I/O 读取缓存索引文件
- 使用 `kota::fail` 可能修改异步任务状态以报告错误

#### Reads From

- `clore::generate::cache::load_cache_index_async` 的 `workspace_root` 参数
- 通过 `clore::generate::cache::load_cache_index` 读取的文件系统资源（如缓存索引文件）

#### Usage Patterns

- 在需要异步获取缓存索引的场景中调用，例如初始化或需要索引时
- 调用者通常使用 `co_await` 等待返回的 `kota::task` 以获取 `CacheIndex` 或处理 `CacheError`

### `clore::generate::cache::make_prompt_response_cache_key`

Declaration: `src/generate/cache.cppm:43`

Definition: `src/generate/cache.cppm:238`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

该函数首先从 `request.response_format` 和 `request.tool_choice` 中分别提取指纹，其中 `response_format_fingerprint` 可能失败并直接返回 `CacheError`。随后调用 `normalize_text_for_hashing` 对 `request.prompt` 和 `system_prompt` 进行规范化，并使用 `llvm::xxh3_64bits` 计算各自的 64 位哈希值，作为后续缓存匹配的摘要部分。

缓存键的核心构造通过一个预分配容量的 `std::string` 完成：依次追加 `request_key`、制表符、`prompt_hash` 的十进制字符串、制表符、`system_prompt_hash` 的十进制字符串、制表符、`response_format` 指纹、制表符、`tool_choice` 指纹，最后附加一个表示 `request.output_contract` 枚举值的字符（通过将枚举值偏移到 `'0'` 获得）。组合后的字符串即为最终缓存键。该过程依赖 `normalize_text_for_hashing` 保证输入文本的一致性，并依赖 `response_format_fingerprint` 和 `tool_choice_fingerprint` 对复杂类型序列化，从而使不同表示下语义相同的请求生成相同的缓存键。

#### Side Effects

- Allocates memory for the returned cache key string (internal `std::string` allocation and ownership transfer to the caller)

#### Reads From

- `request_key` parameter
- `system_prompt` parameter
- `request.prompt` field
- `request.response_format` field
- `request.tool_choice` field
- `request.output_contract` field
- `clore::generate::cache::normalize_text_for_hashing` (reads its input string)

#### Writes To

- Local `std::string key` (built and returned)

#### Usage Patterns

- Called to generate a composite cache key for prompt-response pairs before caching or retrieval

### `clore::generate::cache::normalize_text_for_hashing`

Declaration: `src/generate/cache.cppm:211`

Definition: `src/generate/cache.cppm:211`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

Implementation: [Implementation](functions/normalize-text-for-hashing.md)

函数 `clore::generate::cache::normalize_text_for_hashing` 实现了一个简单的文本规范化流程，其核心目的是为后续哈希计算生成一个格式一致的键。算法分两步：首先跳过输入字符串 `text` 开头的所有空白字符（使用 `std::isspace` 并以 `unsigned char` 避免符号扩展问题）；然后从第一个非空白字符起遍历剩余部分，维护一个 `prev_space` 布尔标志。当遇到空白字符时，仅将 `prev_space` 置为 `true`；遇到非空白字符时，若 `prev_space` 为 `true` 且结果字符串 `result` 非空，则在添加当前字符前先插入一个空格，并重置 `prev_space`。整个过程将任意连续的空白序列压缩为单个空格，同时去除首尾空白，最终返回规范化后的字符串。该函数的全部控制流仅依赖标准库的 `std::isspace` 和 `std::string` 的内存管理，无外部依赖，时间复杂度为 O(n)。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `text` (`std::string_view`)

#### Writes To

- local `result` string (returned by value)

#### Usage Patterns

- Called by `make_prompt_response_cache_key` to normalize text before hashing.

### `clore::generate::cache::save_cache_entry`

Declaration: `src/generate/cache.cppm:50`

Definition: `src/generate/cache.cppm:322`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

函数 `save_cache_entry` 首先获取全局静态 `std::mutex cache_file_mutex` 上的独占锁，确保同一工作空间内的所有写入操作互斥。接着调用 `cache_directory(workspace_root)` 获取缓存根目录，若失败则立即返回对应的 `CacheError`。成功获得目录路径后，使用 `fs::create_directories` 确保目录存在；任何创建错误都会包装为 `CacheError` 返回。然后通过与 `current_jsonl_filename()` 返回的文件名拼接得到 `jsonl_path`，再调用 `build_jsonl_line(cache_key, response)` 生成完整的 JSONL 行。最后以二进制追加模式打开文件，执行写入和刷新；若文件打开或写入失败，均以 `CacheError` 报告失败，否则返回代表成功的 `std::expected<void, CacheError>`。核心依赖为匿名命名空间中的 `cache_directory`、`current_jsonl_filename` 和 `build_jsonl_line`，它们分别处理目录解析、日期滚动的文件名生成以及缓存行的格式化。

#### Side Effects

- Acquires and releases a static mutex (`cache_file_mutex`), synchronizing access.
- Creates directories on the file system via `fs::create_directories`.
- Appends data to a JSONL file on disk, writing the line to the file and flushing it.

#### Reads From

- `workspace_root` (`string_view` parameter)
- `cache_key` (`string_view` parameter)
- `response` (`string_view` parameter)
- `current_jsonl_filename()` (returns a path that may depend on current time or other state)
- `cache_directory(workspace_root)` (function returning a path or error)

#### Writes To

- The file system: a JSONL file located at `*dir / current_jsonl_filename()` (where `dir` is derived from `workspace_root`)
- The directories created by `fs::create_directories`

#### Usage Patterns

- Called by synchronous cache-saving code paths after a response has been generated.
- Used to persist a cache entry keyed by `cache_key` for later retrieval via cached response lookup.

### `clore::generate::cache::save_cache_entry_async`

Declaration: `src/generate/cache.cppm:60`

Definition: `src/generate/cache.cppm:395`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

`save_cache_entry_async` 将同步的 `save_cache_entry` 包装为异步协程。它通过 `kota::queue` 在给定的 `kota::event_loop` 上调度工作项，该工作项捕获传入的 `workspace_root`、`cache_key` 和 `response` 并调用 `save_cache_entry`。如果队列调度本身因取消而失败，则生成一个描述队列失败的 `CacheError`；否则，如果 `save_cache_entry` 返回错误，则将该错误转发给调用者。若一切成功，协程无值返回。该函数依赖 `save_cache_entry` 完成实际的缓存写入（包括 JSONL 行构建、索引加载与更新等步骤），并通过 `kota::event_loop` 确保所有操作在正确的异步上下文中执行。

#### Side Effects

- Writes a cache entry to the filesystem via `save_cache_entry`
- Schedules a task on the given `kota::event_loop` via `kota::queue`

#### Reads From

- Parameter `workspace_root`
- Parameter `cache_key`
- Parameter `response`
- Parameter `loop` (event loop reference)

#### Writes To

- Filesystem cache entry at the location derived from `workspace_root` and `cache_key` (by `save_cache_entry`)

#### Usage Patterns

- Called when a new prompt‑response pair needs to be stored in the cache asynchronously
- Typically awaited by the caller to receive completion or error
- Used in conjunction with `load_cache_index_async` for full async cache workflows

## Internal Structure

该模块负责为 LLM 生成的响应提供持久化缓存，其核心分解为三个职责层：键生成、索引管理与异步 I/O。键生成层通过 `normalize_text_for_hashing` 对系统提示与用户请求进行规范化，并利用 `tool_choice_fingerprint` 与 `response_format_fingerprint` 计算工具选择与响应格式的特征值，最终由 `make_prompt_response_cache_key` 组装出稳定的复合缓存键。索引管理层的 `load_cache_index` 与 `load_cache_index_async` 从 JSONL 文件中读入 `CacheIndex`，而 `find_cached_response` 在此索引上执行只读查找；`save_cache_entry` 与 `save_cache_entry_async` 负责将新的响应记录序列化为 JSONL 行（借助 `build_jsonl_line`、`escape_json_string`）并追加到当日文件。异步 I/O 层基于 `kota::event_loop` 调度缓存写入与索引加载，内部使用 `cache_file_mutex` 保护对同一缓存文件的并发访问。

模块内部依赖 `protocol` 与 `support` 两个公共模块：前者提供请求/响应的结构化类型（如 `ResponseFormat`、`ToolChoice`），后者提供文件路径规范化、UTF-8 文本处理等工具。实现上采用匿名命名空间隔离辅助函数（如 `cache_directory`、`current_jsonl_filename`、`format_iso_timestamp`、`all_jsonl_files`），避免污染外部命名空间。整个模块以同步与异步两种接口衔接上层调用，并在 `CacheError` 中通过 `message` 记录失败原因，确保错误信息可追溯。

## Related Pages

- [Module protocol](../protocol/index.md)
- [Module support](../support/index.md)

