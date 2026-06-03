---
title: 'Module extract:cache'
description: 'extract:cache 模块负责管理 C++ 提取过程的缓存系统，提供键值存储、依赖跟踪和结果序列化能力。它公开了 CacheKeyParts、DependencySnapshot、CacheRecord、CliceCacheData（包含 CliceCachePCMEntry、CliceCachePCHEntry、CliceCacheDepEntry）等核心数据结构，以及 build_cache_key、split_cache_key、hash_file、build_compile_signature、capture_dependency_snapshot、dependencies_changed 和 load_extract_cache / save_extract_cache、load_clice_cache / save_clice_cache 等公共函数。通过这些接口，调用方可以生成并解析缓存键、计算文件或编译单元摘要、捕获依赖快照并检测变化，以及将提取结果（如 AST 和依赖信息）持久化到磁盘，从而在后续编译中跳过重复的提取工作。该模块依赖于 extract:ast、extract:compiler、extract:scan 和 support 模块，共同构成提取管线的缓存基础设施。'
layout: doc
template: doc
---

# Module `extract:cache`

## Summary

`extract:cache` 模块负责管理 C++ 提取过程的缓存系统，提供键值存储、依赖跟踪和结果序列化能力。它公开了 `CacheKeyParts`、`DependencySnapshot`、`CacheRecord`、`CliceCacheData`（包含 `CliceCachePCMEntry`、`CliceCachePCHEntry`、`CliceCacheDepEntry`）等核心数据结构，以及 `build_cache_key`、`split_cache_key`、`hash_file`、`build_compile_signature`、`capture_dependency_snapshot`、`dependencies_changed` 和 `load_extract_cache` / `save_extract_cache`、`load_clice_cache` / `save_clice_cache` 等公共函数。通过这些接口，调用方可以生成并解析缓存键、计算文件或编译单元摘要、捕获依赖快照并检测变化，以及将提取结果（如 AST 和依赖信息）持久化到磁盘，从而在后续编译中跳过重复的提取工作。该模块依赖于 `extract:ast`、`extract:compiler`、`extract:scan` 和 `support` 模块，共同构成提取管线的缓存基础设施。

## Imports

- [`extract:ast`](ast.md)
- [`extract:compiler`](compiler.md)
- [`extract:scan`](scan.md)
- [`support`](../support/index.md)

## Types

### `clore::extract::cache::CacheError`

Declaration: `src/extract/cache.cppm:36`

Definition: `src/extract/cache.cppm:36`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

结构体 `clore::extract::cache::CacheError` 的内部状态由唯一的成员 `message` 承载，该成员是一个 `std::string` 实例。`message` 负责记录与缓存提取错误相关的描述性文本，是错误信息的唯一载体。由于结构体没有任何其他数据成员或虚函数，其实例的内存布局等价于一个 `std::string` 对象，不引入额外开销。

该结构体是一个聚合类型，因此其构造完全依赖于 `message` 的默认初始化或直接聚合初始化。实现中不存在自定义构造函数、析构函数或拷贝/移动控制函数，所有特殊成员函数均由编译器隐式生成。不变量仅要求 `message` 在错误对象的生命周期内保持有效，没有额外的约束或变体机制。

#### Invariants

- The `message` field holds a human-readable error description.

#### Key Members

- `message`

#### Usage Patterns

- Returned or thrown as an error type within the `clore::extract::cache` module.
- Its `message` member is expected to be accessed to obtain error details.

### `clore::extract::cache::CacheKeyParts`

Declaration: `src/extract/cache.cppm:40`

Definition: `src/extract/cache.cppm:40`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

`clore::extract::cache::CacheKeyParts` 是一个纯数据容器，组合了 `path` 与 `compile_signature` 两个字段，用作缓存查找的分解键。`path` 是 `std::string` 类型，保存源文件路径；`compile_signature` 是 `std::uint64_t` 类型，携带编译签名（通常根据编译器选项、源文件内容等计算得出）。该结构不强加内部不变量——合法状态由调用者保证，例如 `path` 通常应为非空，而 `compile_signature` 的任何值都是有效的。其存在是为了将原本可能作为复合键（如 `std::pair` 或哈希元组）处理的逻辑拆分为具名字段，从而提高代码可读性并便于在缓存子系统内部按部件进行序列化或比较。

#### Invariants

- `path` stores a file path as a string.
- `compile_signature` stores a 64-bit unsigned integer representing a compile-time signature.
- Both members together uniquely identify a cache entry.

#### Key Members

- `path`
- `compile_signature`

#### Usage Patterns

- Used as a key part for caching extract results based on file path and compile signature.
- Aggregate initialization allows straightforward construction in cache lookup code.

### `clore::extract::cache::CacheRecord`

Declaration: `src/extract/cache.cppm:52`

Definition: `src/extract/cache.cppm:52`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

`CacheRecord`结构体封装了一个提取缓存条目，其内部按身份标识与缓存负载组织。`compile_signature`和`source_hash`构成唯一键，用于校验缓存命中；零值默认初始化表示记录未设置。`ast_deps`维护编译期依赖快照，`scan`和`ast`分别存储扫描与抽象语法树结果。核心不变量是：只有`compile_signature`和`source_hash`均匹配时，后续三个数据字段才被视为对当前源有效；任何字段的修改必须伴随相应键的更新。

#### Invariants

- `compile_signature` and `source_hash` are initialized to zero
- No further invariants are specified in the evidence

#### Key Members

- `compile_signature`
- `source_hash`
- `ast_deps`
- `scan`
- `ast`

#### Usage Patterns

- Populated and stored in a cache keyed by compile signature and source hash
- Retrieved to reuse previously computed scan and AST results

### `clore::extract::cache::CliceCacheData`

Declaration: `src/extract/cache.cppm:84`

Definition: `src/extract/cache.cppm:84`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

`clore::extract::cache::CliceCacheData` 是一个聚合结构体，通过三个 `std::vector` 成员来存储提取的缓存数据：`paths` 保存源文件路径字符串，`pch` 保存预编译头条目（类型为 `CliceCachePCHEntry`），`pcm` 保存模块条目（类型为 `CliceCachePCMEntry`）。该结构体没有用户声明的构造函数、析构函数或赋值运算符，因此其内存布局和生命周期完全由默认成员初始化器和标准容器的复制/移动语义管理。不变量要求这三个向量在逻辑上保持一致——例如，`pch` 和 `pcm` 中的每个条目都可能引用 `paths` 中的某个索引——但这种一致性依赖调用方在填充和访问数据时加以保证，结构体本身不提供校验机制。

#### Invariants

- No explicit invariants are stated; the vectors are independent containers.

#### Key Members

- `paths` — a vector of file paths as strings
- `pch` — a vector of `CliceCachePCHEntry` objects
- `pcm` — a vector of `CliceCachePCMEntry` objects

#### Usage Patterns

- Used to aggregate cache entries for PCH and PCM along with their associated paths
- Likely serialized or passed around within the caching subsystem

### `clore::extract::cache::CliceCacheDepEntry`

Declaration: `src/extract/cache.cppm:62`

Definition: `src/extract/cache.cppm:62`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

结构体 `clore::extract::cache::CliceCacheDepEntry` 以紧凑的 12 字节布局表示单条依赖项记录，用于工作区缓存的数据交换格式。两个字段 `path`（`std::uint32_t`）与 `hash`（`std::uint64_t`）分别存储依赖文件的路径标识符（推测为路径表索引）及其内容哈希，默认零值表示“未设置”状态。该布局与上游模块 `clore::src::server::workspace.cpp` 中的 `CacheData` 保持二进制兼容性，因此字段顺序、对齐及总计大小均受跨模块 ABI 约束。所有成员均通过默认初始化置零，无需自定义构造函数或析构函数；`path` 和 `hash` 的取值由外部缓存序列化逻辑保证，不设访问器或修改器。

#### Invariants

- `path` 和 `hash` 均为无符号整数类型
- 默认初始化时两个成员均为零
- 结构体为平凡类型，无自定义构造函数或析构函数

#### Key Members

- `path`：32 位路径标识符
- `hash`：64 位内容哈希

#### Usage Patterns

- 作为 `CliceCache` 或其内部容器中的元素类型
- 与 server 端的 `CacheData` 结构体保持二进制兼容，用于磁盘缓存读写
- 通过直接比较 `hash` 和 `path` 来实现依赖项的快速匹配

### `clore::extract::cache::CliceCachePCHEntry`

Declaration: `src/extract/cache.cppm:67`

Definition: `src/extract/cache.cppm:67`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该结构表示一个预编译头（PCH）缓存条目，其内部由六个字段共同构成缓存键值与状态。`filename` 存储PCH文件的路径字符串；`source_file` 是一个32位标识符，可能索引至外部源文件表；`hash` 用于快速校验内容一致性；`bound` 记录绑定次数或关联性计数；`build_at` 为构建时间戳，用于失效判断；`deps` 保存依赖项列表，类型为 `CliceCacheDepEntry` 的向量。这些字段之间维持着隐式的不变性：例如 `hash` 应反映 `filename`、`deps` 及 `source_file` 的组合特征，而 `build_at` 的单调递增性则确保时间顺序的合理性。整个条目的完整性依赖于 `deps` 向量中每个依赖项的正确解析与 `bound` 计数器的同步更新。

#### Invariants

- All integer fields are zero-initialized.
- The `filename` and `deps` are empty by default.
- The `deps` vector contains only valid `CliceCacheDepEntry` objects if non-empty.

#### Key Members

- `filename` - the path or name of the PCH file
- `source_file` - index of the source file associated with the PCH
- `hash` - 64-bit content hash of the PCH
- `bound` - unsigned integer counting bindings to this entry
- `build_at` - 64-bit timestamp of when the PCH was built
- `deps` - vector of `CliceCacheDepEntry` representing dependencies

#### Usage Patterns

- Used as the value type in a cache container (e.g., a map from filename to entry).
- Entries are created when a PCH is built and looked up during cache validation.
- The `deps` field is populated with the dependencies of the PCH to track changes.

### `clore::extract::cache::CliceCachePCMEntry`

Declaration: `src/extract/cache.cppm:76`

Definition: `src/extract/cache.cppm:76`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

内部结构由五个成员组成：`filename` 存储缓存的 PCM 文件路径，`source_file` 为源文件索引（默认 0 表示未关联），`module_name` 记录模块名，`build_at` 为构建时间戳（默认 0 表示未构建），`deps` 为依赖项向量（可包含零个或多个 `CliceCacheDepEntry`）。初始化时 `source_file` 与 `build_at` 的零值作为无效或未设置状态的标记，`filename` 与 `module_name` 在有效条目中应非空。`deps` 的默认构造为空向量，其元素通过分隔的 `CliceCacheDepEntry` 对象表示模块间的依赖关系，该结构体本身不维护额外的不变量，各字段直接支持序列化与比较操作。

#### Invariants

- `source_file` defaults to `0`
- `build_at` defaults to `0`
- `deps` holds a vector of `CliceCacheDepEntry` objects

#### Key Members

- `filename` field
- `source_file` field
- `module_name` field
- `build_at` field
- `deps` field

#### Usage Patterns

- Used within the cache system to store PCM entry data
- Likely serialized or deserialized as part of cache persistence

### `clore::extract::cache::DependencySnapshot`

Declaration: `src/extract/cache.cppm:45`

Definition: `src/extract/cache.cppm:45`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

`clore::extract::cache::DependencySnapshot` 是缓存机制中用于记录依赖项状态的核心数据载体。其内部结构由四个字段构成：`files` 存储依赖项的路径列表，`hashes` 存储每个文件对应的内容哈希值，`mtimes` 存储每个文件的最后修改时间，`build_at` 是一个时间戳，标记该快照生成的时间点。这三个向量隐含的不变量是它们必须保持长度一致，且索引对齐，即第 *i* 个文件路径、哈希值与修改时间一一对应。`build_at` 默认初始化为 `0`，通常表示无效或未设置的时间戳，在实现中需要被正确赋值为实际构建时刻。该结构体本身不提供成员函数，其正确性依赖于调用方在填充数据时维护向量的同步性和时间戳的准确性，因此所有修改操作必须确保 `files`、`hashes`、`mtimes` 通过原子性操作同时调整。

#### Invariants

- The vectors `files`, `hashes`, and `mtimes` are expected to have the same size (parallel sequences).
- The `build_at` field provides a timestamp for the snapshot, defaulting to zero.

#### Key Members

- `files`
- `hashes`
- `mtimes`
- `build_at`

#### Usage Patterns

- Used to cache dependency information and compare against current file states to determine if a rebuild is necessary.
- Likely serialized and deserialized for persistence across build invocations.
- Consumed by code that validates or updates cached dependency data.

## Functions

### `clore::extract::cache::build_cache_key`

Declaration: `src/extract/cache.cppm:92`

Definition: `src/extract/cache.cppm:244`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该实现通过预分配输出缓冲区避免了多次内存分配：先计算 `normalized_path` 的长度、一个字节的分隔符以及 `compile_signature` 最多 20 个字符的十进制表示，然后依次追加这三部分。分隔符由常量 `kCacheKeyDelimiter` 提供，数字转换依赖 `std::to_string`。整个函数无分支、无循环，控制流为纯线性拼接，唯一的依赖是 `clore::extract::cache` 命名空间下的分隔符定义和标准库的数字格式化。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `normalized_path` parameter
- `compile_signature` parameter
- `kCacheKeyDelimiter` constant

#### Usage Patterns

- Used by cache save and load functions to generate keys for file-based caching of extraction data
- Pairs with `split_cache_key` for round-trip parsing

### `clore::extract::cache::build_compile_signature`

Declaration: `src/extract/cache.cppm:90`

Definition: `src/extract/cache.cppm:240`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该函数完全委托给 `clore::extract::build_compile_signature`，没有引入任何额外的验证、转换或缓存机制。它的存在是为了在 `clore::extract::cache` 命名空间中提供一个一致的接口，而实际计算由底层模块完成。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `entry` parameter of type `const CompileEntry &`

#### Usage Patterns

- called to compute a hash‑based compile signature for caching purposes
- used internally by other cache functions such as `load_extract_cache` or `save_extract_cache`

### `clore::extract::cache::capture_dependency_snapshot`

Declaration: `src/extract/cache.cppm:99`

Definition: `src/extract/cache.cppm:298`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function first normalizes the input file paths via `normalize_path_string`, sorts and deduplicates them, then records the current system clock in nanoseconds as `snapshot.build_at`.  For each file it computes a `DependencyHashTaskResult` containing the file's content hash, modification time, and an optional error message.  

Hashing and status retrieval are performed in parallel using `std::thread`: a worker lambda acquires the file's `mtime` via `llvm::sys::fs::status` and computes the hash via `hash_file`.  If `hash_file` fails (e.g. the file was deleted between runs), the function records `hash=0` instead of failing.  A shared mutex (`error_mutex`) guards a `first_error` optional that short-circuits remaining tasks on the first failure.  After all threads join, the results are assembled into the `DependencySnapshot` vectors `files`, `hashes`, and `mtimes`, and the snapshot is returned.  Key internal dependencies include `normalize_path_string`, `hash_file`, and filesystem status calls from LLVM.

#### Side Effects

- Reads file status and content from the filesystem using `llvm::sys::fs::status` and `hash_file`
- Creates and joins multiple `std::thread` objects
- Acquires a `std::mutex` to check and set the first error
- Records the current system time via `std::chrono::system_clock::now`
- Modifies local state such as `normalized`, `task_results`, and `snapshot`

#### Reads From

- Input parameter `files` (a `const std::vector<std::string>&`)
- Filesystem for each file to obtain status and content via `llvm::sys::fs::status` and `hash_file`
- System clock for timestamp via `std::chrono::system_clock::now`
- Hardware concurrency via `std::thread::hardware_concurrency`

#### Writes To

- Local variables `normalized`, `task_results`, `snapshot`, `first_error`, `threads`
- Fields of `snapshot`: `build_at`, `files`, `hashes`, `mtimes`
- Shared optional `first_error` under mutex

#### Usage Patterns

- Called to capture the current state of a set of dependency files
- Used to compare against a previous snapshot to detect changes
- Typically invoked as part of cache invalidation or rebuild logic

### `clore::extract::cache::dependencies_changed`

Declaration: `src/extract/cache.cppm:102`

Definition: `src/extract/cache.cppm:417`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该函数首先执行快速健全性检查：若快照的 `build_at` 非正或 `files` 为空，则立即返回 `true`。接着验证 `files`、`hashes` 与 `mtimes` 三个集合的大小是否一致，若不匹配也直接视为已变更。对于小型依赖集（文件数不超过 `kParallelThreshold`，即 16），采用顺序遍历：逐一调用 `check_single_dependency` 检查每个依赖项，一旦发现变更即提前返回 `true`，否则返回 `false`。对于大型依赖集，启动并行路径：根据 `hardware_threads` 计算线程数（至少为 1），将索引范围均分给多个工作线程。每个工作线程在循环前先检查原子标志 `changed`（使用 `memory_order_relaxed`），若已为 `true` 则提前返回；否则对已分配范围内的每个索引调用 `check_single_dependency`，若发现变更则设置 `changed` 并返回。所有线程汇合后，函数返回 `changed` 的最终值。

该函数的全部依赖项仅为同一匿名命名空间中的辅助函数 `check_single_dependency`，后者负责判断单个依赖项（给定其在快照中的索引）是否发生变更。并行路径的实现避免了昂贵的线程创建开销用于小规模检查，并通过提前终止和原子标志实现了高效的短路行为。

#### Side Effects

- Creates and joins multiple `std::thread` objects when the dependency set size exceeds 16
- Uses `std::atomic<bool>` for inter-thread synchronization and early termination

#### Reads From

- `const DependencySnapshot& snapshot` (reads `build_at`, `files`, `hashes`, `mtimes` members)
- `std::thread::hardware_concurrency()` for thread count
- `check_single_dependency(snapshot, index)` for each dependency

#### Usage Patterns

- Called before recompilation or extraction to test cache validity
- Part of cache invalidation logic in extract cache system

### `clore::extract::cache::hash_file`

Declaration: `src/extract/cache.cppm:97`

Definition: `src/extract/cache.cppm:286`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该函数利用 LLVM 的 `llvm::MemoryBuffer::getFile` 将给定路径的文件映射到内存中；若读取失败，则构造一个 `CacheError`，其 `message` 字段包含格式化的操作系统错误信息。成功时，将整个文件缓冲区直接传递给 `llvm::xxh3_64bits` 以计算 64 位 `xxHash3` 值并返回。该实现不进行分块或流式处理，依赖 LLVM 的缓冲区抽象来处理任意大小的文件，适用于需要快速、确定性哈希的场景。所有错误均通过 `std::expected` 的 `std::unexpected` 路径传播，无外部依赖除 LLVM 基础库外。

#### Side Effects

- Reads file content from the filesystem via `llvm::MemoryBuffer::getFile`

#### Reads From

- Parameter `path` (file path)
- Contents of the file at `path` (through `llvm::MemoryBuffer::getFile`)

#### Usage Patterns

- Used to generate a hash of a source file for cache key derivation
- Called before caching dependencies or compile signatures to detect file changes

### `clore::extract::cache::load_clice_cache`

Declaration: `src/extract/cache.cppm:111`

Definition: `src/extract/cache.cppm:686`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该函数首先通过 `clore::extract::cache::(anonymous namespace)::clice_cache_file_path` 将输入的 `workspace_root` 转换为缓存文件路径。如果路径构造失败，立即返回对应的 `CacheError`。接着检查文件是否存在；若不存在且无系统错误，则返回一个空的 `CliceCacheData` 实例表示缓存未命中。若文件存在，则调用 `clore::support::read_utf8_text_file` 读取其完整内容到字符串。读取失败时同样返回错误。成功读取后，使用 `json::from_json` 将 JSON 内容反序列化为 `CliceCacheData` 结构。若 JSON 解析失败，记录警告并返回空的 `CliceCacheData`（视为过期缓存），否则返回填充好的数据结构。整个过程依赖 `clice_cache_file_path` 的文件路径构造、文件系统存在性检查、文本文件读取以及 JSON 反序列化，控制流简单且无重试逻辑。

#### Side Effects

- 读取文件系统（检查文件存在、读取文件内容）
- 调用 `logging::warn` 记录警告消息

#### Reads From

- 参数 `workspace_root`
- 文件系统（通过 `clice_cache_file_path(workspace_root)` 确定的路径，文件存在性与内容）
- 全局或外部依赖：`clore::support::read_utf8_text_file` 和 `json::from_json` 的内部状态

#### Writes To

- 日志系统（通过 `logging::warn`）

#### Usage Patterns

- 在提取流程开始时加载缓存，若返回空 `CliceCacheData` 则表示无可用缓存或缓存失效
- 与 `save_clice_cache` 配合实现缓存生命周期管理

### `clore::extract::cache::load_extract_cache`

Declaration: `src/extract/cache.cppm:104`

Definition: `src/extract/cache.cppm:473`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该函数首先通过 `cache_file_path` 获取缓存文件路径，检查文件是否存在——若不存在则返回空映射；若存在但无法读取或其内容无法反序列化为 `SerializedCacheData`，则记录警告并返回空映射。反序列化后，还会校验 `data.format_version` 是否与 `kExtractCacheFormatVersion` 一致，版本不匹配时同样警告并返回空映射。

通过上述验证后，函数遍历 `data.entries` 中的每个 `SerializedCacheEntry`，依次验证 `entry.source_file` 索引有效，调用 `decode_dependency_snapshot` 解码 `entry.ast_deps` 得到 `DependencySnapshot`，利用 `normalize_path_string` 规范化路径字符串，再通过 `build_cache_key` 结合规范化路径和 `entry.compile_signature` 构造缓存的键，最后构建 `CacheRecord` 并存入 `records` 映射。依赖的内部函数包括 `decode_dependency_snapshot`、`normalize_path_string` 和 `build_cache_key`，它们均位于 `clore::extract::cache` 匿名命名空间内。

#### Side Effects

- reads the cache file from the filesystem
- logs warning messages via `logging::warn` when the cache file is stale or has an invalid format version

#### Reads From

- `workspace_root` parameter
- cache file located at path derived from `workspace_root`
- `kExtractCacheFormatVersion` constant
- `cache_file_path` helper (implicit filesystem state)
- `clore::support::read_utf8_text_file` reads the file content

#### Usage Patterns

- called with a workspace root to retrieve previously saved extract cache data
- used before extracting to check if a cached result exists
- typically paired with `save_extract_cache` for caching extraction results

### `clore::extract::cache::save_clice_cache`

Declaration: `src/extract/cache.cppm:113`

Definition: `src/extract/cache.cppm:726`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

函数 `clore::extract::cache::save_clice_cache` 通过串行化、临时文件与原子重命名实现缓存持久化。首先获取 `clice_cache_file_path` 确定目标路径，并确保其父目录存在。随后调用 `json::to_json` 将 `CliceCacheData` 串行化为 JSON 字符串。为避免写入过程中并发读取损坏，算法生成一个唯一的临时文件路径：利用当前时间戳、进程 ID 与一个递增尝试次数经 `llvm::xxh3_64bits` 计算 nonce，形成形如 `<cache_path>.tmp.<pid>.<timestamp>.<nonce>` 的路径，并在循环中通过 `fs::exists` 检测最多 32 次以确保路径不存在。之后通过 `write_utf8_text_file` 将串行化内容写入临时文件。

写入成功后将临时文件通过 `fs::rename` 原子替换目标缓存文件。若重命名因权限拒绝、文件已存在或操作不允许而失败，则尝试先 `fs::remove` 移除已存在的目标文件（忽略文件不存在的错误），再重新执行重命名。整个流程依赖 `llvm::sys::Process::getProcessId` 获取进程 ID、`llvm::xxh3_64bits` 生成 nonce，以及 `clice_cache_file_path`、`write_utf8_text_file` 等辅助函数。任何步骤出错均返回包含错误描述的 `CacheError`。

#### Side Effects

- creates directories via `fs::create_directories`
- writes a temporary file via `clore::support::write_utf8_text_file`
- renames the temporary file to the final cache path via `fs::rename`
- removes the existing cache file when rename fails with certain errors

#### Reads From

- `workspace_root` parameter
- `data` parameter
- filesystem state via `fs::exists`
- system clock for timestamp generation
- process ID via `llvm::sys::Process::getProcessId`

#### Writes To

- cache directory determined by `clice_cache_file_path`
- temporary file at a unique path near the cache file
- final cache file at the path returned by `clice_cache_file_path`

#### Usage Patterns

- called to persist a `CliceCacheData` snapshot to disk
- used after cache computation completes to store results
- likely invoked by `clore::extract::cache::save_extract_cache` or similar cache-saving routines

### `clore::extract::cache::save_extract_cache`

Declaration: `src/extract/cache.cppm:107`

Definition: `src/extract/cache.cppm:549`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

首先通过 `cache_directory` 确定缓存根目录并确保其存在，然后遍历传入的 `records` 映射：对每条记录调用 `split_cache_key` 解析出路径与编译签名，并校验签名与记录中的 `compile_signature` 一致。接着使用 `intern_path` lambda 对路径字符串执行规范化（经 `normalize_path_string`）并去重，将结果存入 `SerializedCacheData::paths` 表；同时调用 `encode_dependency_snapshot` 将依赖快照编码为索引列表。所有记录转换后形成 `SerializedCacheEntry` 列表，通过 `json::to_json` 序列化为 JSON 字符串。

写入阶段先为最终缓存路径（`cache.json`）构造一个唯一的临时文件路径（组合进程 ID、时间戳和基于 nonce 的哈希，最多尝试 32 次避免冲突），然后用 `clore::support::write_utf8_text_file` 写入序列化内容。最后通过 `fs::rename` 将临时文件原子地移到目标位置；若因权限不足或文件已存在等错误导致重命名失败，则先尝试删除目标文件再重命名。任一环节出错均返回 `CacheError`。

#### Side Effects

- Creates or ensures existence of the extract cache directory via filesystem operations
- Writes a temporary JSON file to the cache directory
- Removes the existing cache file if a rename conflict occurs
- Renames the temporary file to the final cache path, atomically replacing the cache

#### Reads From

- `workspace_root` parameter
- records parameter
- filesystem state for existence checks of temporary file candidates and the final cache file
- internal helper `normalize_path_string` reads its input string

#### Writes To

- filesystem: cache directory (via `create_directories`)
- filesystem: temporary file in cache directory (via `write_utf8_text_file`)
- filesystem: final cache file `cache.json` (via `rename`)
- filesystem: may remove existing cache file when replacing

#### Usage Patterns

- Called to persist a collection of `CacheRecord` entries after an extraction pass
- Used in the cache management pipeline alongside `load_extract_cache`
- Typically invoked when the caller has built the records from compilation and dependency analysis

### `clore::extract::cache::split_cache_key`

Declaration: `src/extract/cache.cppm:95`

Definition: `src/extract/cache.cppm:254`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

函数 `clore::extract::cache::split_cache_key` 通过解析传入的 `cache_key` 字符串，将其拆分为路径和编译签名两部分。首先利用 `std::string_view::rfind` 查找最后一个 `kCacheKeyDelimiter` 分隔符的位置；若未找到，立即返回一个包含格式化错误信息的 `CacheError`。若找到分隔符，则通过 `substr` 提取 `path_part` 与 `signature_part`，并校验两部分均非空；任意部分为空时同样返回错误。随后，使用 `std::from_chars` 将 `signature_part` 的字符序列转换为 `std::uint64_t` 类型的 `signature`；转换失败或未完全消耗输入字符串时，返回带有具体错误描述的 `CacheError`。成功转换后，函数返回一个 `CacheKeyParts` 结构体，其中 `path` 字段为路径字符串的拷贝，`compile_signature` 字段为解析出的签名值。该函数依赖 `kCacheKeyDelimiter` 常量、`CacheError` 错误类型以及 `CacheKeyParts` 输出类型，内部仅使用标准库的字符串操作和字符转换设施，无其他外部依赖。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `cache_key` (type `std::string_view`)
- constant `kCacheKeyDelimiter`

#### Usage Patterns

- decompose keys built by `build_cache_key`
- validate and extract parts of a cache key before further processing

## Internal Structure

`extract:cache` 模块提供了提取结果的持久化机制，将 AST、扫描和依赖信息序列化到文件系统，以实现增量构建。它公开了缓存键构建与拆分（`build_cache_key`、`split_cache_key`）、文件哈希（`hash_file`）、依赖快照捕获与变化检测（`capture_dependency_snapshot`、`dependencies_changed`）、编译签名生成（`build_compile_signature`）以及两类缓存数据的保存与加载（`*_extract_cache` 和 `*_clice_cache` 系列函数）。内部按用途分解为公共接口和匿名命名空间的实现细节，其中私有结构如 `SerializedCacheData`、`CachedPathHash` 封装了磁盘格式，`normalize_path_string`、`cache_directory` 等辅助函数处理路径规范化与目录定位，并通过临时文件与重命名实现原子写入。

模块直接依赖 `extract:ast`、`extract:compiler`、`extract:scan` 和 `support`，其中前三个提供提取、编译数据库和扫描的领域类型，`support` 提供日志、文件操作等底层工具。内部通过 `CliceCacheData` 统一管理三种条目（PCM、PCH、依赖），依赖快照使用 `DependencySnapshot` 记录文件列表与哈希值，并通过并行哈希任务（`DependencyHashTaskResult`）提升性能。整个模块围绕“缓存键 → 编译单元 → 文件路径 → 序列化数据”的层次组织，外部调用者只需使用顶层函数，无需关心底层存储布局。

## Related Pages

- [Module extract:ast](ast.md)
- [Module extract:compiler](compiler.md)
- [Module extract:scan](scan.md)
- [Module support](../support/index.md)

