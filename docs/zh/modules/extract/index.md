---
title: 'Module extract'
description: '模块 clore::extract 负责从项目源代码中提取结构化信息，包括抽象语法树、依赖关系、模块定义等，并为此提供异步执行、缓存评估与持久化机制。其公开接口仅包含 extract_project_async 函数，该函数接收项目标识和事件循环引用，返回状态码以指示提取是否成功启动。模块内部实现了从扫描缓存加载、并行 AST 提取、模块信息构建到缓存保存的完整流水线，并利用 support 和 config 模块提供的底层工具与配置支持。'
layout: doc
template: doc
---

# Module `extract`

## Summary

模块 `clore::extract` 负责从项目源代码中提取结构化信息，包括抽象语法树、依赖关系、模块定义等，并为此提供异步执行、缓存评估与持久化机制。其公开接口仅包含 `extract_project_async` 函数，该函数接收项目标识和事件循环引用，返回状态码以指示提取是否成功启动。模块内部实现了从扫描缓存加载、并行 AST 提取、模块信息构建到缓存保存的完整流水线，并利用 `support` 和 `config` 模块提供的底层工具与配置支持。

## Imports

- [`config`](../config/index.md)
- `std`
- [`support`](../support/index.md)

## Imported By

- [`agent`](../agent/index.md)
- [`agent:tools`](../agent/tools.md)
- [`generate`](../generate/index.md)
- [`generate:analysis`](../generate/analysis.md)
- [`generate:common`](../generate/common.md)
- [`generate:diagram`](../generate/diagram.md)
- [`generate:dryrun`](../generate/dryrun.md)
- [`generate:evidence`](../generate/evidence.md)
- [`generate:evidence_builder`](../generate/index.md)
- [`generate:model`](../generate/model.md)
- [`generate:page`](../generate/page.md)
- [`generate:planner`](../generate/planner.md)
- [`generate:scheduler`](../generate/scheduler.md)
- [`generate:symbol`](../generate/symbol.md)

## Types

### `clore::extract::ExtractError`

Declaration: `extract/extract.cppm:21`

Definition: `extract/extract.cppm:21`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

实现上，`clore::extract::ExtractError` 仅包含一个 `std::string message` 成员，用于存储错误描述文本。整个结构体没有自定义构造函数、析构函数或赋值运算符，依赖编译器生成的默认特殊成员函数，因此其生命周期和资源管理完全由 `message` 的 `std::string` 实现负责。不变量在于 `message` 中始终存放可解释的错误信息；对于该结构体的使用方而言，直接读取 `message` 即可获得错误详情，无需额外解析。

#### Invariants

- 包含错误描述字符串
- 无其他约束或保证

#### Key Members

- `message` 成员

#### Usage Patterns

- 作为提取函数的错误返回类型
- 调用者通过读取 `message` 获取错误详情

## Functions

### `clore::extract::extract_project_async`

Declaration: `extract/extract.cppm:25`

Definition: `extract/extract.cppm:539`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::extract_project_async` 实现了异步提取整个项目模型的核心流程。算法首先加载编译数据库并应用筛选，随后并行加载提取缓存和 Clice 缓存（通过 `load_caches_async`），并为每个编译条目计算缓存键、编译签名和源哈希，构建 `cache_evaluations` 映射以判断扫描和 AST 结果是否有效。接着调用 `build_dependency_graph_async` 结合种子扫描缓存构建依赖图，并对其执行拓扑排序以获得文件处理顺序。之后启动 `extract_ast_batch_async` 并行处理所有条目，该任务使用缓存评估结果决定是否跳过 AST 解析。

主循环按拓扑序遍历每个条目：若该条目缓存中的 AST 有效，则直接从 `cache_records` 读取 `ASTResult`；否则从并行任务结果中获取。针对每个条目，函数遍历其包含的扫描结果和 AST，通过 `resolve_symbol_location_path` 协程解析符号声明与定义的文件路径至规范形式，并利用 `filter_root` 过滤掉不在项目范围内的符号和包含项，最终将符号、关系及包含信息合并至 `ProjectModel` 中。处理完成后，函数调用 `rebuild_model_indexes` 和 `build_module_info` 完善模型结构，并通过后台队列协程解析源代码片段。最后，异步保存更新后的缓存记录（`save_caches_async`）并记录性能指标。

#### Side Effects

- loads compilation database from disk
- loads/saves cache files
- logs progress and metrics
- mutates `ProjectModel` (inserts symbols, files, relations)
- mutates `cache_records` and `cache_evaluations`
- uses `kota::event_loop` for asynchronous I/O and task scheduling

#### Reads From

- `config::TaskConfig` (`compile_commands_path`, `workspace_root`, filter, etc.)
- compilation database file
- cache files on disk
- `CompileEntry` fields (file, directory, `cache_key`, `source_hash`)
- scan cache (`seeded_scan_cache`)
- dependency graph
- parallel AST results

#### Writes To

- `ProjectModel` (symbols, files, relations, `file_order`, namespaces, modules)
- cache records (in memory and persisted to disk)
- log output (via `logging::info` and `logging::cache_hit_rate`)
- `resolved_path_cache` (internal cache)

#### Usage Patterns

- top-level extraction entry point
- called in an asynchronous context with `co_await`
- used to generate the project model for further analysis or IDE features

## Internal Structure

模块 `extract` 按功能分解为两层：缓存管理层和提取执行层。缓存层负责加载、验证和持久化扫描结果、闭包缓存及提取缓存记录，通过 `load_caches_async`、`save_caches_async` 等异步函数与 `CacheEvaluation`、`LoadedCaches` 等内部类型协同工作。提取执行层在缓存就绪后，对每个条目解析 AST 并建立依赖图，由 `extract_ast_entry`、`build_module_info`、`extract_ast_batch_async` 等函数驱动，通过 `run_worker_task_async` 和 `run_cache_io_async` 实现并行 I/O 与计算分离。

该模块导入 `config` 获取项目配置，导入 `support` 获得文本处理、文件操作及缓存键管理工具，标准库提供并发与容器支持。内部逻辑围绕 `PreparedEntryState` 构建索引，利用 `ParallelASTResult` 和 `DepGraph` 汇聚各文件的提取结果，最终由 `extract_project_async` 整合并返回状态码，整个过程在 `kota::event_loop` 上异步推进。

## Related Pages

- [Module config](../config/index.md)
- [Module support](../support/index.md)

