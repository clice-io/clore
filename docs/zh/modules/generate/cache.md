---
title: 'Module generate:cache'
description: 'clore::generate::cache 模块负责为 LLM 生成的输出提供持久化缓存设施，以避免重复计算相同提示。其公共接口包括：缓存索引的类型 CacheIndex 与错误类型 CacheError；同步/异步加载缓存索引的函数 load_cache_index 与 load_cache_index_async；同步/异步保存缓存条目的函数 save_cache_entry 与 save_cache_entry_async；在已加载的索引中查找缓存响应的 find_cached_response；以及生成唯一缓存键的 make_prompt_response_cache_key 和规范化文本用于哈希的 normalize_text_for_hashing。该模块依赖 protocol、support 和标准库，内部通过批量的 JSONL 文件管理缓存条目，并支持基于事件循环的异步操作。'
layout: doc
template: doc
---

# Module `generate:cache`

## Summary

`clore::generate::cache` 模块负责为 LLM 生成的输出提供持久化缓存设施，以避免重复计算相同提示。其公共接口包括：缓存索引的类型 `CacheIndex` 与错误类型 `CacheError`；同步/异步加载缓存索引的函数 `load_cache_index` 与 `load_cache_index_async`；同步/异步保存缓存条目的函数 `save_cache_entry` 与 `save_cache_entry_async`；在已加载的索引中查找缓存响应的 `find_cached_response`；以及生成唯一缓存键的 `make_prompt_response_cache_key` 和规范化文本用于哈希的 `normalize_text_for_hashing`。该模块依赖 `protocol`、`support` 和标准库，内部通过批量的 JSONL 文件管理缓存条目，并支持基于事件循环的异步操作。

## Imports

- [`protocol`](../protocol/index.md)
- `std`
- [`support`](../support/index.md)

## Imported By

- [`generate:scheduler`](scheduler.md)

## Types

### `clore::generate::cache::CacheError`

Declaration: `generate/cache.cppm:16`

Definition: `generate/cache.cppm:16`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

`clore::generate::cache::CacheError` 仅包含一个 `std::string message` 数据成员，没有其他状态或辅助函数。内部不变量要求 `message` 在构造后保持有效字符串（可为空），但未通过构造函数显式约束；直接暴露成员允许调用方任意赋值，因此不变量完全依赖使用方维护。该结构体未定义任何特殊成员函数，完全依赖编译器生成的默认构造函数、析构函数和拷贝/移动操作，作为轻量级错误传递类型使用。

### `clore::generate::cache::CacheIndex`

Declaration: `generate/cache.cppm:20`

Definition: `generate/cache.cppm:20`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

`clore::generate::cache::CacheIndex` 作为一个轻量数据结构实现，其底层存储完全由 `std::unordered_map<std::string, std::string> entries` 构成。该成员负责维护从键到值的映射关系，提供平均常数时间的插入、查找与删除操作。内部不变量要求 `entries` 中的键必须唯一且非空，但字符串内容不作格式限制。由于 `entries` 是公开字段，实现上不封装任何访问控制逻辑，调用方直接通过该成员管理缓存索引条目。

#### Invariants

- The `entries` map contains string keys mapped to string values.
- Keys are expected to be unique within the map.

#### Key Members

- `entries`: the underlying `std::unordered_map<std::string, std::string>` that holds the cache index data.

#### Usage Patterns

- Used to look up cached results by key.
- Populated and queried by cache management functions.

## Functions

### `clore::generate::cache::find_cached_response`

Declaration: `generate/cache.cppm:35`

Definition: `generate/cache.cppm:347`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

函数`clore::generate::cache::find_cached_response` 实现了基于内存哈希索引的查找算法。它接受一个`CacheIndex`类型引用和`std::string_view`类型的`cache_key`参数。内部控制流首先将`cache_key`隐式转换为`std::string`以匹配`CacheIndex::entries`的键类型，然后在`index.entries`（该字段是一个关联容器）上调用`find`操作。若迭代器`it`指向`entries.end()`，则返回`std::nullopt`；否则返回`it->second`即对应的缓存响应值。该函数唯一依赖是`CacheIndex::entries`容器，其性能由底层哈希表（通常为`std::unordered_map`）决定，平均查找时间为常数。它不涉及文件系统或异步操作，完全在内存中完成查找。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `index` of type `const CacheIndex&`
- parameter `cache_key` of type `std::string_view`
- field `index.entries` (a map)

#### Usage Patterns

- called to retrieve a previously cached generation response
- used in conjunction with `save_cache_entry` or similar caching functions

### `clore::generate::cache::load_cache_index`

Declaration: `generate/cache.cppm:29`

Definition: `generate/cache.cppm:252`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

函数 `clore::generate::cache::load_cache_index` 的实现首先调用内部函数 `all_jsonl_files` 扫描工作区根目录下的所有 JSONL 文件。若该步骤失败，直接返回 `CacheError`。对于每一个成功读取的文件，通过 `kota::codec::json::parse` 逐行解析 JSON 对象，仅提取 `key` 和 `resp` 字段的非空字符串值，并将其插入到 `CacheIndex::entries` 映射中。文件读取或解析失败的条目会被静默跳过，最终返回构建完成的 `CacheIndex` 实例。该函数依赖文件系统扫描、文本读取、JSON 解析以及 `CacheIndex` 与 `CacheError` 类型。

#### Side Effects

- Reads multiple JSONL files from disk
- Allocates memory for the `CacheIndex` object

#### Reads From

- `workspace_root` parameter
- JSONL files on disk
- file content strings

#### Writes To

- the returned `CacheIndex` instance's entries map

#### Usage Patterns

- Loading cache index for lookup
- Called before `find_cached_response`

### `clore::generate::cache::load_cache_index_async`

Declaration: `generate/cache.cppm:38`

Definition: `generate/cache.cppm:356`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

该函数将同步的 `clore::generate::cache::load_cache_index` 调用包装为异步协程。它通过 `kota::queue` 将同步逻辑调度到传入的 `kota::event_loop &loop` 中执行，并等待队列任务完成。若队列任务本身失败（例如异常导致调度错误），则构造一个 `CacheError` 并调用 `co_await kota::fail` 传播错误；若任务成功但内部的 `load_cache_index` 返回了错误（即 `std::expected` 不含值），则将该 `CacheError` 原样转发。仅当两者均成功时，才将加载得到的 `CacheIndex` 通过 `co_return` 移出返回。

内部控制流完全由协程的 `co_await` 驱动，依赖 `load_cache_index` 完成文件扫描与索引构建，而 `kota::queue` 确保工作线程安全地执行同步操作，避免阻塞事件循环。由于 `load_cache_index` 本身可能因路径无效、文件解析错误等返回 `CacheError`，该函数仅负责传递或包装这些错误，不涉及索引内容的理解或修改。

#### Side Effects

- Loads the cache index from the filesystem via a queued task on the event loop

#### Reads From

- `workspace_root` parameter
- `loop` parameter
- Cache index file (via `load_cache_index`)

#### Usage Patterns

- Called to obtain the cache index asynchronously before cache lookups or modifications
- Used in coroutine contexts that need to interact with the generation cache

### `clore::generate::cache::make_prompt_response_cache_key`

Declaration: `generate/cache.cppm:24`

Definition: `generate/cache.cppm:219`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

函数首先通过 `response_format_fingerprint` 和 `tool_choice_fingerprint` 将 `request` 中的结构化配置（输出格式与工具选择）转换为紧凑指纹字符串，若指纹生成失败则提前返回错误。随后对 `request.prompt` 及 `system_prompt` 字符串调用 `normalize_text_for_hashing` 进行规范化，并借助 `llvm::xxh3_64bits` 分别计算两者的64位哈希值。最终将所有组件依次拼接为制表符分隔的字符串：`request_key`、两个哈希的十进制表示、响应格式指纹、工具选择指纹，并在末尾附加一个表示 `request.output_contract` 枚举值的字符。该函数依赖 `normalize_text_for_hashing` 进行文本归一化，依赖 `response_format_fingerprint` 与 `tool_choice_fingerprint` 两个匿名命名空间内的辅助函数完成配置的序列化，整个过程的错误仅来源于指纹生成步骤。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `request_key`
- `system_prompt`
- `request.prompt`
- `request.response_format`
- `request.tool_choice`
- `request.output_contract`
- `normalize_text_for_hashing`
- `llvm::xxh3_64bits`

#### Usage Patterns

- used to generate a unique cache key for storing or retrieving cached responses
- called by cache lookup and insertion functions

### `clore::generate::cache::normalize_text_for_hashing`

Declaration: `generate/cache.cppm:192`

Definition: `generate/cache.cppm:192`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

Implementation: [Implementation](functions/normalize-text-for-hashing.md)

该函数通过两个阶段实现文本规范化。第一阶段从前向后扫描输入并跳过所有前导空白字符（使用 `std::isspace` 检查每个字符），因此只保留第一个非空白字符之后的文本。第二阶段从第一个非空白字符开始遍历剩余字符：遇到空白字符时，仅设置一个布尔标记 `prev_space`，不直接向结果追加空白；遇到非空白字符时，若 `prev_space` 为真且结果非空，则在追加该字符前先插入一个空格，然后重置标记。这一逻辑将任意长度的连续空白序列压缩为一个空格，同时确保结果字符串不会以空格开头（因为前导空白已跳过）且末尾不会有多余空格（只有当非空白字符之后有空白时才可能追加空格，而末尾空白序列永远不会触发追加）。整个过程完全基于标准库函数 `std::isspace` 和 `std::string` 的 `reserve`、`push_back`，无需其他外部依赖。

#### Side Effects

- allocates a new string

#### Reads From

- `text` parameter

#### Writes To

- local variable `result`
- return value (new `std::string`)

#### Usage Patterns

- called by `clore::generate::cache::make_prompt_response_cache_key` to normalize prompt or response text before hashing
- used to ensure consistent cache keys regardless of whitespace variation

### `clore::generate::cache::save_cache_entry`

Declaration: `generate/cache.cppm:31`

Definition: `generate/cache.cppm:303`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

函数通过互斥锁 `cache_file_mutex` 序列化对缓存文件的并发写入，确保线程安全。内部流程首先调用 `cache_directory(workspace_root)` 获取缓存目录路径，若失败则返回 `CacheError`；成功后使用 `fs::create_directories` 确保目录存在，失败同样返回错误。接着由 `current_jsonl_filename()` 生成当前 JSONL 文件名，拼接出完整路径。调用 `build_jsonl_line(cache_key, response)` 构建单行记录，然后以二进制追加模式打开文件，写入该行并刷新缓冲区。任何打开或写入失败都返回描述性 `CacheError`。该函数依赖于 `cache_directory`、`build_jsonl_line` 和 `current_jsonl_filename` 三个辅助函数，所有错误均封装为 `CacheError` 结构体。

#### Side Effects

- writes a JSONL line to the prompt response cache file
- creates cache directories if they do not exist
- acquires a static mutex for synchronization

#### Reads From

- parameter `workspace_root`
- parameter `cache_key`
- parameter `response`
- result of `cache_directory(workspace_root)`
- result of `current_jsonl_filename()`
- result of `build_jsonl_line(cache_key, response)`
- static mutex `cache_file_mutex`

#### Writes To

- filesystem: creates directories under `workspace_root` cache path
- filesystem: appends data to the JSONL cache file

#### Usage Patterns

- called after a successful generation to persist the response
- synchronous version of `save_cache_entry_async`
- invoked by higher-level generation routines

### `clore::generate::cache::save_cache_entry_async`

Declaration: `generate/cache.cppm:41`

Definition: `generate/cache.cppm:376`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

函数 `clore::generate::cache::save_cache_entry_async` 是一个基于 `kota::task` 的异步协程，它将实际工作委托给同步版本 `save_cache_entry`。内部控制流先将参数转移至 `kota::queue` 调用中，该调用在给定的 `kota::event_loop` 上调度闭包，捕获 `workspace_root`、`cache_key` 和 `response` 并通过移动传入。`kota::queue` 返回 `queued_result`，如果该结果表示取消（通过 `catch_cancel`），则函数直接通过 `kota::fail` 返回一个 `CacheError`，其 `.message` 指示保存任务失败。否则检查 `queued_result` 是否持有 `std::expected`，若内部值无值（即同步 `save_cache_entry` 返回错误），则将该错误移动至 `kota::fail` 传播。成功时协程完成返回。主要依赖包括 `kota::event_loop`、`kota::queue`、`kota::fail`、`kota::task` 以及核心同步函数 `save_cache_entry`（位于同一命名空间）和 `CacheError` 结构体。

#### Side Effects

- Writes cache entry to disk via `save_cache_entry`
- Invokes asynchronous I/O through event loop

#### Reads From

- `workspace_root` string parameter
- `cache_key` string parameter
- `response` string parameter
- `loop` event loop parameter
- Result of queued `save_cache_entry` call

#### Writes To

- Cache storage (disk) via `save_cache_entry`
- Coroutine state

#### Usage Patterns

- Called to persist a generated response asynchronously
- Used after response generation to avoid blocking the main thread

## Internal Structure

`generate:cache` 模块实现了针对 LLM 响应结果的持久化缓存机制。它公开了 `load_cache_index` / `load_cache_index_async`、`save_cache_entry` / `save_cache_entry_async` 以及 `find_cached_response` 等同步与异步接口，并通过 `make_prompt_response_cache_key` 和内部文本规范化函数保证缓存键的唯一性与可再现性。模块采用基于 JSONL 文件的存储布局，缓存索引（`CacheIndex`）作为内存中的条目映射，避免每次查询时全量扫描磁盘。在实现结构上，它引入匿名命名空间封装了 `cache_directory`、`build_jsonl_line`、`format_iso_timestamp`、`tool_choice_fingerprint` 等文件路径组装、行构造与指纹生成逻辑，同时利用 `kota::event_loop` 将 I/O 操作委托给事件循环，实现非阻塞的文件加载与写入。

模块的依赖仅限于 `protocol`（LLM 请求/响应类型）和 `support`（UTF‑8 文本处理、文件读写、路径规范化等基础设施），自身不介入更上层的协议编排。内部分层清晰：基础层处理文件系统操作与文本格式化，中层管理索引加载与条目查找/保存，外层通过同步/异步函数向调用者提供统一接口。缓存键由提示、响应格式和工具选择等参数复合生成，写入时还结合时间戳与锁机制（`cache_file_mutex`）保护并发安全，形成一套完整且可独立测试的缓存子系统。

## Related Pages

- [Module protocol](../protocol/index.md)
- [Module support](../support/index.md)

