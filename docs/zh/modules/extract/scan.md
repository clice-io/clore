---
title: 'Module extract:scan'
description: 'extract:scan 模块负责对编译条目执行扫描和分析，提取 C++ 模块声明、导入以及包含指令等依赖信息，为后续的工具链处理提供结构化的扫描结果。它通过 scan_file 对单个条目进行完整解析并产生 ScanResult 或 ScanError，通过 scan_module_decl 在不运行完整预处理器的前提下快速提取模块名和导入关系。此外，该模块还提供了 build_dependency_graph_async 用于异步构建整个项目的依赖图，以及 topological_order 计算图的拓扑排序，从而支持依赖分析和增量处理。'
layout: doc
template: doc
---

# Module `extract:scan`

## Summary

`extract:scan` 模块负责对编译条目执行扫描和分析，提取 C++ 模块声明、导入以及包含指令等依赖信息，为后续的工具链处理提供结构化的扫描结果。它通过 `scan_file` 对单个条目进行完整解析并产生 `ScanResult` 或 `ScanError`，通过 `scan_module_decl` 在不运行完整预处理器的前提下快速提取模块名和导入关系。此外，该模块还提供了 `build_dependency_graph_async` 用于异步构建整个项目的依赖图，以及 `topological_order` 计算图的拓扑排序，从而支持依赖分析和增量处理。

公开实现方面，`extract:scan` 定义了核心数据类型 `ScanResult`、`ScanCache`、`DependencyGraph` 及 `DependencyEdge` 等，这些类型作为扫描管道的输入输出，供调用者驱动扫描流程并获取依赖关系。模块依赖 `extract:compiler` 提供的编译数据库和条目，以及 `support` 模块的通用工具，自身则专注于扫描逻辑和依赖图构建的抽象，不与文件系统或具体编译器细节耦合。

## Imports

- [`extract:compiler`](compiler.md)
- [`support`](../support/index.md)

## Imported By

- [`extract:cache`](cache.md)

## Types

### `clore::extract::DependencyEdge`

Declaration: `src/extract/scan.cppm:69`

Definition: `src/extract/scan.cppm:69`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::DependencyEdge` 在定义上是一个仅包含两个 `std::string` 字段 `from` 和 `to` 的简单聚合体，用于在提取过程中表示一条有向依赖边。内部没有显式的不变量，但该结构体的语义要求 `from` 和 `to` 分别代表依赖的源头和目标（例如模块或实体名称），并且空字符串通常表示缺失或未定义的端点。成员均通过值语义直接存储，因此拷贝、移动和比较操作均基于字符串内容的逐位对比。在实现层面，该结构体常用作容器（如 `std::vector` 或 `std::set`）的元素，依赖其默认的逐成员比较规则进行排序或去重。

#### Invariants

- `from` 和 `to` 没有非空约束，可以是任意字符串；
- 该结构体不保证 `from` 和 `to` 的语义对称性。

#### Key Members

- `from`：依赖边的起始节点标识；
- `to`：依赖边的终止节点标识。

#### Usage Patterns

- 用于表示代码依赖图中节点之间的有向边；
- 在依赖分析过程中由扫描器创建，并被后续的图处理逻辑使用。

### `clore::extract::DependencyGraph`

Declaration: `src/extract/scan.cppm:74`

Definition: `src/extract/scan.cppm:74`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

结构体 `clore::extract::DependencyGraph` 内部仅维护两个向量：`files` 用于存储所有被扫描文件的路径字符串，`edges` 则记录文件间的依赖边（类型为 `DependencyEdge`）。其核心不变量是 `edges` 中每条边所引用的源文件与目标文件索引均落在 `files` 的有效范围内（即 `0 <= idx < files.size()`），从而保证图的完整性。由于该结构体没有自定义构造或析构函数，这些向量的初始化和生命周期完全由编译器生成的默认成员管理；实际使用中，`edges` 与 `files` 的同步维护（例如添加新文件时更新边引用的索引）由外部调用代码负责。

#### Invariants

- No invariants are documented or inferable from the evidence.

#### Key Members

- `files`
- `edges`

#### Usage Patterns

- The struct is likely populated by a scan or extraction process and consumed by other parts of the `clore::extract` module.

### `clore::extract::IncludeInfo`

Declaration: `src/extract/scan.cppm:42`

Definition: `src/extract/scan.cppm:42`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

结构体 `clore::extract::IncludeInfo` 是一个仅包含 `std::string path` 与 `bool is_angled` 两个成员的值类型，用于存储解析 `#include` 指令时提取的路径字符串及其包含风格（尖括号或引号）。其内部不变量要求 `path` 不包含空白字符（由调用方保证），而 `is_angled` 的取值直接反映预处理器标记的语法形式。该结构体在扫描阶段作为临时容器，不管理资源所有权，亦不定义自定义构造函数或赋值操作，依赖编译器生成的默认成员函数；其简单布局确保了通过值传递时的拷贝效率，并便于在向量或其他连续容器中批量存储。

#### Invariants

- `path` is not validated for existence or format
- `is_angled` distinguishes between angled and quoted includes
- default initialization: `path` empty, `is_angled` false

#### Key Members

- `path`
- `is_angled`

#### Usage Patterns

- Used as a plain data container within the extraction pipeline to represent include directives

### `clore::extract::ScanCache`

Declaration: `src/extract/scan.cppm:58`

Definition: `src/extract/scan.cppm:58`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`ScanCache` 的核心内部结构是一个 `std::unordered_map<std::string, ScanResult>` 成员 `scan_results`，它作为持久存储，在连续依赖扫描之间缓存已解析的扫描结果。其设计意图是避免重复扫描相同源文件，从而提升性能；但调用者必须保证在编译数据库或文件系统状态发生变化时主动清除或丢弃整个缓存，以维持正确性。内部不维护任何附加状态或失效逻辑——缓存的生存周期完全由外部管理，因此 `scan_results` 直接暴露了底层映射，默认初始化为空。重要的成员实现仅依赖于非成员函数对该映射的读写操作；结构体本身作为聚合类型，不包含自定义构造、析构或赋值行为，所有操作都隐式地由编译器生成。

#### Invariants

- Entries in `scan_results` remain valid as long as the compilation database and file system state are unchanged
- The cache must be cleared or discarded by callers when external state (compilation DB or file system) changes
- The key type `std::string` serves as a unique identifier for a scan result

#### Key Members

- `scan_results` (`std::unordered_map<std::string, ScanResult>`)

#### Usage Patterns

- Other code accesses or populates `scan_results` to avoid redundant scanning across successive dependency scans
- Callers are expected to check or clear the cache when the compilation database or file system is modified

### `clore::extract::ScanError`

Declaration: `src/extract/scan.cppm:38`

Definition: `src/extract/scan.cppm:38`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::ScanError` 是一个仅包含一个 `std::string` 类型数据成员 `message` 的简单聚合结构体，用于封装扫描过程中的错误描述。该结构体没有实现任何额外的不变量或构造函数，其全部语义依赖于 `message` 的字符串内容；构造和访问均通过公有成员直接进行，从而在实现内部作为一种轻量级的错误传递载体。

### `clore::extract::ScanResult`

Declaration: `src/extract/scan.cppm:47`

Definition: `src/extract/scan.cppm:47`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::ScanResult` 是一个聚合类型，用于承载模块扫描的完整输出。其公开字段直接暴露所有扫描结果：`module_name` 存储识别到的模块名称（可能为空表示非命名模块），`is_interface_unit` 标记当前翻译单元是否为模块接口单元，`includes` 收集所有 `#include` 指令的信息，`module_imports` 则记录导入的模块名称列表。该结构体本身不维护任何不变量，也未提供自定义构造函数或成员函数，所有字段均由外部扫描逻辑填充，作为扫描阶段的纯数据容器。这些字段的顺序和默认值（如 `module_name` 初始化为空字符串、`is_interface_unit` 默认为 `false`）保证了扫描结果的零初始化安全。

#### Invariants

- Fields are default-constructed to empty/null states
- `is_interface_unit` is `false` unless explicitly set
- No relationships between members are enforced beyond aggregate initialization

#### Key Members

- `module_name`
- `is_interface_unit`
- `includes`
- `module_imports`

#### Usage Patterns

- Returned by scanning functions to represent a parsed module
- Consumed by extraction pipeline to build further representations
- Members are accessed directly as a plain aggregate

## Functions

### `clore::extract::build_dependency_graph_async`

Declaration: `src/extract/scan.cppm:79`

Definition: `src/extract/scan.cppm:388`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::build_dependency_graph_async` 首先清空 `graph.files` 与 `graph.edges`，然后遍历 `db.entries`：对每个条目调用 `prepare_scan_entry` 生成 `PreparedScanEntry`，并将归一化后的源文件路径去重后填入 `graph.files`。接着，它利用可选的 `ScanCache` 进行缓存查找：对每个条目，若其 `cache_key` 在 `cache->scan_results` 中存在则直接复用缓存的 `ScanResult`，否则按 `cache_key` 去重构造 `MissingScanTask` 列表。对于所有缺失扫描任务，函数通过 `kota::when_all` 并发执行 `run_scan_task` 协程，等待所有任务完成后收集 `ScanResult` 至 `scanned_results`，并处理错误与数量校验。

最后，函数再次遍历所有条目，从缓存或 `scanned_results` 中获取对应的 `ScanResult`，提取其 `includes` 字段中的 `IncludeInfo`，将路径归一化后，仅保留在入口文件集合 `entry_files` 中的包含关系，并利用 `emitted_edges` 集合去重，将 `DependencyEdge`（包含 `from` 与 `to` 字段）推入 `graph.edges`。同时，若提供了 `ScanCache`，则将当前 `ScanResult` 写入 `cache->scan_results` 以备后续重用。整个过程依赖异步事件循环 `loop` 驱动协程，并借助匿名命名空间中的 `PreparedScanEntry`、`MissingScanTask` 等内部类型组织数据流。

#### Side Effects

- clears and repopulates `graph.files` and `graph.edges`
- updates `cache->scan_results` with scan results when cache is provided
- launches asynchronous scan tasks that perform I/O and modify internal state

#### Reads From

- `db` (compilation database) entries and their properties
- `cache->scan_results` (if cache is not null)
- prepared scan entry data derived from `db.entries`

#### Writes To

- `graph.files` and `graph.edges`
- `cache->scan_results` (if cache is not null)
- internal vectors `prepared_entries`, `cached_results`, `scanned_results`, and lookup structures

#### Usage Patterns

- called to build a dependency graph for a project's compilation database
- used in conjunction with an event loop for async concurrency
- integrated into higher-level extraction pipelines

### `clore::extract::scan_file`

Declaration: `src/extract/scan.cppm:62`

Definition: `src/extract/scan.cppm:256`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::scan_file` 首先检查 `entry.arguments` 是否为空，若为空则立即返回 `ScanError`。随后，它尝试从 `entry.normalized_file`（或通过 `normalize_entry_file` 标准化的路径）读取文件内容，并对该文本调用 `scan_module_decl` 快速解析模块声明（如 `module` 或 `import` 关键字），将结果填入本地的 `ScanResult result`。接着，函数通过 `create_compiler_instance` 构造一个 Clang 编译器实例，并将前端动作设为仅预处理，随后创建一个 `ScanAction` 对象（内部关联 `result`）。`ScanAction` 依次调用 `BeginSourceFile` 和 `ExecuteAction` 驱动预处理过程；`ScanPPCallbacks` 回调会在过程中捕获 `#include` 指令和模块导入等细节，并填充 `result` 的 `includes` 与 `module_imports` 字段。若 `ExecuteAction` 返回错误，则函数记录错误信息并返回 `ScanError`；反之，最终调用 `EndSourceFile` 并返回完整的 `ScanResult`。

整个流程依赖外部函数 `scan_module_decl` 实现预解析，以及 Clang 预处理器基础设施与 `ScanPPCallbacks` 回调协作完成详细扫描。内部控制流兼具快速文本扫描与完整编译前端回调，从而在保证性能的同时获得精准的依赖关系。

#### Side Effects

- reads source file from disk
- creates compiler instance (may invoke toolchain)
- runs preprocessor (may read additional files such as headers)
- modifies compiler frontend options (mutation of instance)
- populates `ScanResult` object with extracted data

#### Reads From

- `entry.arguments`
- `entry.file`
- `entry.normalized_file`
- content of the source file at `normalized_file`
- implicitly reads additional files during preprocessing

#### Writes To

- `result` (`ScanResult`) via `scan_module_decl` and `ScanAction`
- `frontend_opts` (mutated locally)
- `instance` (compiler instance mutated locally)

#### Usage Patterns

- Called to scan a single source file for symbols as part of the extraction pipeline
- Typically invoked with entries from a compilation database returned by `load_compdb`
- May be used alongside other extraction functions like `extract_symbols`

### `clore::extract::scan_module_decl`

Declaration: `src/extract/scan.cppm:67`

Definition: `src/extract/scan.cppm:159`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/scan-module-decl.md)

The function `clore::extract::scan_module_decl` implements a lightweight module‑declaration scanner that avoids the overhead of a full preprocessor run by using Clang’s dependency‑directives scanner (`clang::scanSourceForDependencyDirectives`). It operates directly on the raw file content (`std::string_view file_content`) and populates the supplied `ScanResult` with the module name, interface‑unit flag, and a deduplicated list of module imports.

The algorithm first tokenises the file content into `tokens` and `directives` via the Clang scanner. Two local lambdas, `is_whitespace_only` and `is_punctuation_only`, help classify token text. It then iterates over each directive. For directives of kind `cxx_export_module_decl` or `cxx_module_decl`, it skips the `export` and `module` keywords, then concatenates subsequent tokens until a semicolon is encountered. A special case detects a module‑fragment declaration (e.g. `module;` or `module :...`) by checking if the first non‑whitespace token after `module` is punctuation; in that case the module name is left empty and `is_interface_unit` is not set. Otherwise the collected name is stored in `result.module_name`, and `result.is_interface_unit` is set to `true` only for an `export module` declaration. For directives of kind `cxx_import_decl`, the scanner extracts the import name, normalises it via `normalize_partition_import`, and appends it to `result.module_imports` if not already present. The function makes no changes to the `ScanResult` if the scanner itself fails, effectively falling back to no module detection.

#### Side Effects

- Modifies the `ScanResult` object passed by reference by setting `module_name`, `is_interface_unit`, and appending to `module_imports`.

#### Reads From

- Reads from the `file_content` string view.
- Uses Clang's `clang::scanSourceForDependencyDirectives` to parse tokens and directives internally.

#### Writes To

- Writes to `result.module_name` (`std::string`).
- Writes to `result.is_interface_unit` (`bool`).
- Writes to `result.module_imports` via `push_back` (`std::vector<std::string>`).

#### Usage Patterns

- Called by `clore::extract::scan_file` to quickly extract module-level declarations without full preprocessing.
- Designed for scanning source files to determine module name, interface status, and imported modules.

### `clore::extract::topological_order`

Declaration: `src/extract/scan.cppm:84`

Definition: `src/extract/scan.cppm:513`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数首先为 `graph.files` 中的每个文件在 `in_degree` 映射中初始化为零，然后遍历 `graph.edges`：对于每条边，将 `edge.to` 的邻接表 `adj` 中添加 `edge.from`，并将 `edge.from` 的 `in_degree` 值递增。完成图构建后，调用 `clore::support::topological_order` 并传入 `graph.files`、`adj` 和 `in_degree` 执行拓扑排序。若排序失败（即存在循环依赖），函数直接返回一个包含描述信息的 `std::unexpected` 包装的 `ScanError`；否则成功时返回排序后的文件列表。该实现依赖 `clore::support::topological_order` 完成核心的 Kahn 算法流程，而自身仅负责将 `DependencyGraph` 的边列表转换为入度和邻接表数据结构。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `graph.files` (the set of file names)
- `graph.edges` (dependency edges)

#### Usage Patterns

- Called during the extraction phase to determine processing order of source files
- Used to detect circular dependencies in the project include graph

## Internal Structure

`extract:scan` 模块将预处理扫描分解为多个内聚阶段，以实现异步依赖图构建。它导入 `extract:compiler` 获取标准化编译条目，并利用 `support` 中的缓存键构建与文件路径工具。内部通过匿名命名空间隐藏实现细节：`ScanPPCallbacks` 处理 `#include` 与模块导入指令，`ScanAction` 封装单次文件扫描的执行逻辑，`PreparedScanEntry` 和 `MissingScanTask` 为异步调度提供任务抽象。公开的 `scan_file` 和 `scan_module_decl` 分别支持完整预处理扫描和快速模块声明扫描，而 `build_dependency_graph_async` 协调这些任务，通过 `ScanCache` 按 `cache_key` 跳过已扫描条目，最终输出 `DependencyGraph` 及其拓扑序。这种分层使缓存、任务调度与依赖解析职责清晰分离，且所有可变状态生命周期均由调用方通过 `kota::event_loop` 管理。

## Related Pages

- [Module extract:compiler](compiler.md)
- [Module support](../support/index.md)

