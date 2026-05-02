---
title: 'Module extract:scan'
description: 'extract:scan 模块负责对 C++ 源文件进行快速扫描以提取模块与依赖信息。它提供了文件级扫描（scan_file）、模块声明快速解析（scan_module_decl）以及异步依赖图构建（build_dependency_graph_async）等公开接口，并利用 ScanCache 在连续扫描间复用结果以避免重复工作。模块公开的数据结构包括 ScanResult（包含模块名、是否为接口单元、模块导入和包含指令）、DependencyGraph（记录文件间的依赖边和拓扑顺序）以及 ScanError 等，调用者可通过这些类型获取扫描结果。'
layout: doc
template: doc
---

# Module `extract:scan`

## Summary

`extract:scan` 模块负责对 C++ 源文件进行快速扫描以提取模块与依赖信息。它提供了文件级扫描（`scan_file`）、模块声明快速解析（`scan_module_decl`）以及异步依赖图构建（`build_dependency_graph_async`）等公开接口，并利用 `ScanCache` 在连续扫描间复用结果以避免重复工作。模块公开的数据结构包括 `ScanResult`（包含模块名、是否为接口单元、模块导入和包含指令）、`DependencyGraph`（记录文件间的依赖边和拓扑顺序）以及 `ScanError` 等，调用者可通过这些类型获取扫描结果。

## Imports

- [`extract:compiler`](compiler.md)
- `std`
- [`support`](../support/index.md)

## Imported By

- [`extract:cache`](cache.md)

## Types

### `clore::extract::DependencyEdge`

Declaration: `extract/scan.cppm:51`

Definition: `extract/scan.cppm:51`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::DependencyEdge` 是一个扁平的数据聚合，其内部仅包含两个 `std::string` 成员：`from` 和 `to`。该结构没有定义构造函数、特殊成员函数或访问控制，因此成员均以公开方式直接暴露。设计上无额外不变量：所有字段均可独立赋值，且不对字符串内容施加任何格式或语义约束（例如不允许空字符串或自环）。成员实现仅依赖 `std::string` 的默认行为，未引入自定义拷贝、移动或析构逻辑。本质上，它充当了有向边上两个端点名称的简易容器，在提取流程中作为“从 `from` 到 `to`”的依赖关系载体。

#### Invariants

- No invariants are documented beyond the default properties of `std::string`.

#### Key Members

- `from`
- `to`

#### Usage Patterns

- Acts as a data container for a dependency edge in extraction processes.

### `clore::extract::DependencyGraph`

Declaration: `extract/scan.cppm:56`

Definition: `extract/scan.cppm:56`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

结构体 `clore::extract::DependencyGraph` 的直接内部表示由两个公开的 `std::vector` 成员构成：`files` 存储所有被扫描的文件路径字符串，`edges` 存储连接这些文件的 `DependencyEdge` 对象。核心不变量在于每个 `DependencyEdge` 所引用的文件索引必须落在 `files` 的有效下标范围内，从而保证图结构始终指向已存在的顶点。该设计让新文件或边的添加只需对相应向量进行 `push_back`，并在构造或修改边时维护索引合法性。

#### Invariants

- The `files` vector contains paths of all scanned source files.
- The `edges` vector contains all discovered dependency relationships.

#### Key Members

- `clore::extract::DependencyGraph::files`
- `clore::extract::DependencyGraph::edges`

#### Usage Patterns

- Populated by the extraction pipeline when scanning source modules.
- Consumed by downstream consumers to analyze or visualize dependencies.

### `clore::extract::IncludeInfo`

Declaration: `extract/scan.cppm:24`

Definition: `extract/scan.cppm:24`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该结构体是一个简单的聚合体，用于存储预处理包含指令的解析结果。它包含两个公有数据成员：`path`（`std::string` 类型）用于保留原始包含路径的文本，`is_angled`（`bool` 类型）指示该包含是否以尖括号形式（例如 `#include <...>`）编写。两个成员均以默认值初始化，因此默认构造产生的实例中 `path` 为空字符串且 `is_angled` 为 `false`。由于没有自定义构造函数或成员函数，该类型的对象通常通过聚合初始化或逐个成员赋值来填充正确的值。使用方需确保在读取字段前已正确设置它们，尤其是在需要区分空路径与有效路径的场景下。

#### Invariants

- `path` is a valid `std::string` (may be empty)
- `is_angled` is either `true` or `false`

#### Key Members

- `path`: the textual representation of the include target
- `is_angled`: indicates whether the include uses angle brackets (`#include <...>`) or quotes (`#include "..."`)

#### Usage Patterns

- Used as a building block for representing parsed include directives in the `clore::extract` module
- Likely consumed by higher-level extraction logic that processes include chains

### `clore::extract::ScanCache`

Declaration: `extract/scan.cppm:40`

Definition: `extract/scan.cppm:40`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::ScanCache` 的内部结构仅由一个 `std::unordered_map<std::string, ScanResult>` 字段 `scan_results` 构成，该映射将文件路径（以 `std::string` 表示）映射到对应的已扫描结果。其核心不变量要求映射中的每个键必须对应一个唯一且完整的依赖扫描结果（`ScanResult`），且整个缓存的生命周期与调用者内部的编译数据库及文件系统状态保持同步：当外部状态发生变化时，缓存应立即被视为失效，调用者负责清空或替换 `scan_results` 的内容。重要成员实现方面，该结构体未定义任何自定义构造函数或修改方法，完全依赖默认成员函数；对于 `scan_results` 的插入、查找和清除操作均通过 `std::unordered_map` 的公共接口间接完成，因此不存在额外的内部同步或验证逻辑。

#### Invariants

- 缓存中的扫描结果在依赖关系稳定时保持有效
- 编译数据库或文件系统变化后缓存可能失效
- 调用者负责在环境变化时丢弃缓存

#### Key Members

- `scan_results`

#### Usage Patterns

- 扫描函数通过此缓存避免重复扫描相同的依赖项
- 调用者在环境变化时创建新的 `ScanCache` 实例或清空现有实例

### `clore::extract::ScanError`

Declaration: `extract/scan.cppm:20`

Definition: `extract/scan.cppm:20`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

结构体 `clore::extract::ScanError` 的内部实现仅由一个 `std::string message` 成员组成，用于存储扫描期间遇到的错误描述。该结构体未定义任何自定义构造函数、析构函数或成员函数，其默认构造、复制、移动和析构行为完全由编译器合成，并依赖 `std::string` 的相应操作。由于 `message` 是唯一的数据成员，该结构体不维持任何超越字符串本身的不变性约束；`message` 的内容完全由调用方负责设置和解释。

#### Invariants

- The `message` member always contains a valid string (may be empty).
- The struct has no other state or constraints beyond the string.

#### Key Members

- `message`

#### Usage Patterns

- Returned from `clore::extract` scanning functions to indicate failure.
- Inspected by callers to obtain the error description.

### `clore::extract::ScanResult`

Declaration: `extract/scan.cppm:29`

Definition: `extract/scan.cppm:29`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::ScanResult` 是一个聚合类型，用于承载对单个源文件进行词法扫描后提取的模块信息。其内部结构保证了字段的默认状态：`module_name` 为空字符串，`is_interface_unit` 为 `false`，`includes` 和 `module_imports` 均为空向量。扫描过程中，各个字段被独立填充：`module_name` 记录遇到的具名模块（若有），`module_imports` 收集源文件中所有模块导入语句的名称，`includes` 保存经过规范化处理的包含路径信息（类型为 `IncludeInfo`），而 `is_interface_unit` 专用于标记该源文件是否声明为模块接口单元。这些字段之间不存在强约束关系，但调用方需确保 `module_imports` 和 `includes` 按源文件中的出现顺序追加，以维持后续分析的可预测性。

#### Invariants

- `module_name` may be empty if no module name was declared
- `is_interface_unit` defaults to `false`
- `includes` and `module_imports` are initially empty vectors
- No guarantees about the ordering or uniqueness of elements in the vectors

#### Key Members

- `module_name`
- `is_interface_unit`
- `includes`
- `module_imports`

#### Usage Patterns

- Returned by scanning functions to represent a parsed C++ module unit
- Consumed by downstream extraction or analysis code to access module metadata

## Functions

### `clore::extract::build_dependency_graph_async`

Declaration: `extract/scan.cppm:61`

Definition: `extract/scan.cppm:370`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::build_dependency_graph_async` 通过协程异步构建模块依赖图。它首先清空 `DependencyGraph` 的 `files` 和 `edges`，然后遍历 `CompilationDatabase` 的每一项，调用 `prepare_scan_entry` 生成 `PreparedScanEntry`，并将标准化文件名存入 `graph.files`。接着，它利用可选的 `ScanCache` 查询缓存：若 `cache_key` 已存在于 `cache->scan_results` 中，则直接使用缓存结果；否则将任务收集到 `MissingScanTask` 中（按 `cache_key` 去重）。缺失的任务通过 `run_scan_task` 并行执行，使用 `kota::when_all` 等待所有任务完成，并处理可能的 `ScanError`。

在所有扫描结果就绪后，函数再次遍历所有入口项：从缓存或扫描结果中获取 `ScanResult`，提取其 `includes` 列表，仅当包含的目标文件名位于入口文件集合（`entry_files`）中时，才构造 `DependencyEdge`（包含 `from` 和 `to` 字段）并去重后添加到 `graph.edges`。最后，若提供了缓存，则用本次扫描结果更新 `cache->scan_results`。整个流程通过 `co_await` 与 `kota::fail` 实现异步控制流和错误传递，依赖于 `normalize_argument_path`、`prepare_scan_entry`、`run_scan_task` 以及 `kota::event_loop` 提供的并发调度。

#### Side Effects

- 清空并填充 `DependencyGraph` 的 `files` 和 `edges` 成员
- 若 `ScanCache` 非空，则更新其中的 `scan_results` 映射
- 通过 `kota::event_loop` 调度异步扫描任务（可能触发文件 I/O）

#### Reads From

- `const CompilationDatabase &db`
- `ScanCache *cache`（若非空，读取其 `scan_results`）
- 通过 `prepare_scan_entry` 预处理后的编译条目数据

#### Writes To

- `DependencyGraph &graph`（修改 `files` 和 `edges`）
- `ScanCache *cache`（若非空，插入或更新扫描结果）
- 局部变量 `scanned_results`、`cached_results` 等

#### Usage Patterns

- 作为依赖图构建阶段的核心入口，通常在 `extract_project_async` 或类似提取流程中调用
- 配合 `CompilationDatabase` 和可选缓存，用于增量或全量分析

### `clore::extract::scan_file`

Declaration: `extract/scan.cppm:44`

Definition: `extract/scan.cppm:238`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数首先验证传入的 `CompileEntry` 参数是否含有有效的命令行参数，若 `entry.arguments` 为空则直接返回一个包含错误信息的 `ScanError`。随后它创建一个 `ScanResult` 结构体，并通过读取 `entry.normalized_file` 对应的源文件内容，调用 `scan_module_decl` 快速扫描模块声明，将发现的模块名和接口单元标志填入 `result`。接着利用 `create_compiler_instance` 构造一个 Clang 编译器实例，并配置其前端操作仅执行预处理（`RunPreprocessorOnly`）。核心步骤是实例化一个 `ScanAction` 对象（其构造函数绑定到 `result` 的引用），通过 `BeginSourceFile` 与编译器实例关联，然后执行 `ExecuteAction` 以触发自定义预处理器回调（例如 `InclusionDirective`），收集包含指令和导入信息。若执行成功，调用 `EndSourceFile` 清理资源并返回填充完整的 `ScanResult`；任何阶段失败都会包装为 `ScanError` 并返回 `std::unexpected`。

内部控制流紧密依赖 Clang 的 `Preprocessor` 接口和 `ScanPPCallbacks` 回调类，其中 `InclusionDirective` 负责记录每个 `IncludeInfo` 条目，同时 `scan_module_decl` 提供轻量级模块声明预解析。整个函数不依赖异步或拓扑排序，仅同步单文件扫描，其产出可直接用于后续的依赖图构建（如 `build_dependency_graph_async` 所消费的 `ScanResult`）。

#### Side Effects

- Reads source file content from disk via `std::ifstream`
- Creates a compiler instance (likely invoking external toolchain processes) via `create_compiler_instance`
- Executes a Clang preprocessor action that may perform filesystem I/O and interact with system headers

#### Reads From

- `entry.file`
- `entry.arguments`
- `entry.normalized_file`
- source file content on disk
- compiler instance configuration returned by `create_compiler_instance`
- `ScanAction::BeginSourceFile`, `Execute`, and `EndSourceFile` interactions

#### Usage Patterns

- Invoked for each `CompileEntry` in an extraction pipeline to gather module and symbol data
- Combined with the fast text‑based scan to reduce reliance on full compilation for module detection

### `clore::extract::scan_module_decl`

Declaration: `extract/scan.cppm:49`

Definition: `extract/scan.cppm:141`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/scan-module-decl.md)

函数首先调用 `clang::scanSourceForDependencyDirectives`，将文件内容解析为令牌与指令列表；若扫描失败则直接返回。随后定义 `is_whitespace_only` 和 `is_punctuation_only` 两个辅助 lambda，用于识别令牌类型。遍历每条指令：对于 `cxx_export_module_decl` 或 `cxx_module_decl`，跳过 `export` 和 `module` 关键字，收集后续令牌（连接标识符、点、冒号）作为模块名称；遇到分号或纯标点时终止，并将纯标点情形视为全局模块片段。若收集到有效名称，则设置 `result.module_name`，并根据指令类型设置 `result.is_interface_unit`。对于 `cxx_import_decl`，跳过 `import` 关键字，收集后续令牌作为导入名称，调用 `normalize_partition_import` 进行规范化，并仅在 `result.module_imports` 中尚未存在时添加该导入。整个算法的核心依赖是 Clang 的依赖指令扫描基础设施，无需运行完整预处理器即可快速提取模块声明信息。

#### Side Effects

- Modifies the `ScanResult` object passed by reference, setting `module_name`, `is_interface_unit`, and appending to `module_imports`.

#### Reads From

- `file_content` parameter (string view of file source)
- `result.module_imports` member (to avoid duplicate entries)

#### Writes To

- `result.module_name`
- `result.is_interface_unit`
- `result.module_imports`

#### Usage Patterns

- Called by `clore::extract::scan_file` during source file scanning to populate module metadata.
- Used as a lightweight alternative to full preprocessing for extracting module information.

### `clore::extract::topological_order`

Declaration: `extract/scan.cppm:66`

Definition: `extract/scan.cppm:495`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::topological_order` 通过构建邻接表和入度表来执行依赖图的拓扑排序。首先遍历 `graph.files` 初始化每个文件的入度为 0，再遍历 `graph.edges`，对于每条边将 `edge.to` 作为键、`edge.from` 添加至邻接值列表，并递增 `edge.from` 的入度。这一构造将依赖关系反转：入度反映的是有多少其他文件依赖当前文件。随后将文件列表、邻接表和入度表一并委托给 `clore::support::topological_order` 计算排序结果。

若 `clore::support::topological_order` 返回空值（表示检测到循环依赖），函数构造一个包含描述信息的 `ScanError` 并返回 `std::unexpected`；否则直接展开排序结果作为成功值返回。整个流程完全依赖外部的拓扑排序工具，内部仅负责将 `DependencyGraph` 的数据结构转换为算法所需的映射形式。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `graph.files`
- `graph.edges`

#### Usage Patterns

- computes compilation order from include graph
- validates acyclic dependency graph for projects

## Internal Structure

模块 `extract:scan` 是 `clore::extract` 命名空间内负责 C++ 源文件依赖扫描的核心模块。它对外公开了三个主要函数：`scan_file`（对单个文件执行完整扫描并返回 `ScanResult` 或 `ScanError`）、`scan_module_decl`（快速扫描模块声明，不运行完整预处理器）以及 `build_dependency_graph_async`（异步构建包含文件间依赖关系的 `DependencyGraph`，支持可选的 `ScanCache` 以复用扫描结果）。辅助函数 `topological_order` 对依赖图进行拓扑排序，为后续处理提供线性顺序。

内部实现通过匿名命名空间划分为多个职责清晰的单元：`ScanPPCallbacks` 实现 Clang 预处理器回调，收集 `#include` 指令和模块导入；`ScanAction` 封装一次原子性的扫描操作，包括解析预处理结果；`PreparedScanEntry` 和 `MissingScanTask` 管理扫描任务的预处理与缓存键生成，结合 `ScanCache` 避免重复工作。模块依赖 `extract:compiler`（提供编译数据库条目与标准化编译选项）和 `support`（提供文本处理、文件 I/O 及缓存键工具），并通过事件循环（`kota::event_loop`）支持异步执行。这种分层使得扫描逻辑与底层的 Clang 交互、缓存管理和任务调度解耦，便于维护与测试。

## Related Pages

- [Module extract:compiler](compiler.md)
- [Module support](../support/index.md)

