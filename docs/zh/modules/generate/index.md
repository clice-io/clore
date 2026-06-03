---
title: 'Module generate'
description: 'generate 模块负责文档页面的完整生成流程，提供从验证到产出的公共接口。它暴露了 generate_dry_run 用于在无实际输出的情况下验证输入与配置的有效性；generate_pages 作为同步主入口，接受范围标识、输出路径与并发控制参数并返回状态码；generate_pages_async 允许在指定事件循环上异步执行生成任务，调用方需手动调度并运行返回的任务；write_pages 则负责将已生成的页面持久化到指定存储位置。模块内部依赖 config 获取配置、extract 获取已提取的项目数据，并通过 model、llm_model、rate_limit 等变量和事件循环支持生成的可配置性与异步能力。'
layout: doc
template: doc
---

# Module `generate`

## Summary

`generate` 模块负责文档页面的完整生成流程，提供从验证到产出的公共接口。它暴露了 `generate_dry_run` 用于在无实际输出的情况下验证输入与配置的有效性；`generate_pages` 作为同步主入口，接受范围标识、输出路径与并发控制参数并返回状态码；`generate_pages_async` 允许在指定事件循环上异步执行生成任务，调用方需手动调度并运行返回的任务；`write_pages` 则负责将已生成的页面持久化到指定存储位置。模块内部依赖 `config` 获取配置、`extract` 获取已提取的项目数据，并通过 `model`、`llm_model`、`rate_limit` 等变量和事件循环支持生成的可配置性与异步能力。

## Imports

- [`config`](../config/index.md)
- [`extract`](../extract/index.md)

## Imported By

- [`agent`](../agent/index.md)
- [`agent:tools`](../agent/tools.md)

## Functions

### `clore::generate::generate_dry_run`

Declaration: `src/generate/generate.cppm:42`

Definition: `src/generate/scheduler.cppm:1957`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::generate_dry_run` 实现了完整的干运行流程，用于在不调用 LLM 或写入文件系统的情况下模拟页面生成管线，从而估算预期请求数与缓存命中情况。它首先调用 `prepare_generation_context` 与 `prepare_symbol_analyses_for_dry_run` 构建完整的计划与符号分析目标，然后创建 `PageRenderer`（其 `dry_run_` 字段为真）和 `PageGenerationScheduler`（设置 `dry_run` 标志），使调度器在解析依赖时直接使用缓存索引（通过 `cache_index_` 和 `cache_index_mutex_`）来模拟 `request_llm_async` 的返回，并跳过实际的 LLM 请求与文件写入。

内部控制流由 `DependencyTracker` 驱动：它初始化就绪候选（`initialize_ready_candidates`），然后 `PageGenerationScheduler` 的 `worker_task` 循环从 `WorkQueue` 取出工作（包括符号分析与页面提示工作），但干运行模式下所有 `perform_prompt_request` 调用都被短路，改为从缓存中获取 `cached_response` 或直接视为失败。调度器统计 `expected_llm_requests_`、`page_prompt_cache_hits_` 与 `page_prompt_cache_misses_` 等指标，并沿计划树释放依赖（`release_dependents`），最后通过 `render_generated_pages` 生成空的元数据页（`dry_run_pages_`）而不实际输出。函数返回一个整数，通常代表预期的 LLM 请求总数或错误码。

#### Side Effects

No observable side effects are evident from the extracted code.

### `clore::generate::generate_pages`

Declaration: `src/generate/generate.cppm:45`

Definition: `src/generate/scheduler.cppm:2016`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::generate_pages` 首先调用 `prepare_generation_context` 对配置与模型进行预处理，生成 `PreparedGenerationContext` 结构，其中包含按计划组织的符号分析目标 (`PreparedSymbolAnalysisTarget`)、提示请求、页面链接等，并可选地调用 `prepare_symbol_analyses_for_dry_run` 为干运行模式准备分析结果。随后构造一个 `PageGenerationScheduler` 实例，该调度器内部持有 `DependencyTracker`（追踪每个页面的依赖状态与就绪候选）、`WorkQueue`（管理待处理的工作项和延迟队列）以及 `PageRenderer`（负责组装并输出最终文件）。调度器的 `run` 方法启动事件循环并通过 `worker_task` 驱动主循环：工作线程从队列中出队 `ScheduledWork`（可能是 `SymbolAnalysisWork` 或 `PagePromptWork`），并调用 `run_symbol_analysis_task` 或 `run_page_prompt_task` 执行。符号分析工作会收集可文档化符号、构建证据元数据并生成符号级别的 LLM 请求（通过 `request_llm_async` 异步发送），其输出经解析后通过 `finish_base_symbol_prompt` 或 `finish_page_prompt_work` 更新依赖状态并释放就绪的页面。页面提示工作则基于 `PreparedPrompt` 生成 LLM 请求，并在响应到达后解析输出，然后尝试提交就绪页面进行渲染。调度器通过 `try_submit_ready_pages` 检查并触发 `RenderPageWork`，由 `PageRenderer::emit_pages_async` 异步写入输出文件。所有 LLM 请求完成后，调用 `build_directory_index_pages` 生成目录索引页面，并更新页面摘要缓存 (`update_page_summary_cache`)。最终通过 `make_generate_error` 或直接返回 `success` 值表示完成。整个流程受 `rate_limit` 约束，并支持干运行模式（仅收集分析结果而不发出 LLM 请求）。

#### Side Effects

No observable side effects are evident from the extracted code.

### `clore::generate::generate_pages_async`

Declaration: `src/generate/generate.cppm:54`

Definition: `src/generate/scheduler.cppm:1994`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数通过调用 `prepare_generation_context` 构建 `PreparedGenerationContext`，然后实例化 `PageRenderer` 与 `PageGenerationScheduler`。`scheduler` 内部持有 `DependencyTracker`、`WorkQueue` 和 `cache_index_`，并在异步工作循环中驱动符号分析、页面提示提交及渲染流水线。控制流首先调度 `collect_documentable_symbols` 与 `prepare_symbol_analyses_for_dry_run` 获得 `PreparedSymbolAnalyses`，随后利用 `DependencyTracker` 的 `initialize_ready_candidates` 标记无依赖的页面。工作线程通过 `run_queued_worker_call` 轮询 `WorkQueue`，执行 `schedule_symbol_analysis` → `run_symbol_analysis_task` 以及 `finish_page_prompt_work` 等步骤，每完成一个符号分析即调用 `release_dependents` 释放阻塞的页面；页面提示完成后检查依赖是否全部满足，符合条件则提交 `RenderPageWork` 并由 `render_ready_page` 调用 `emit_pages_async` 输出文件。缓存层利用 `prompt_cache_identity_for_page_request` 生成键值，通过 `cache_index_` 存储命中/未命中计数，同时通过 `consecutive_failures_` 追踪连续错误以触发 `retry_limit_exceeded`。所有内部依赖（如 `DependencyTracker`、`WorkQueue` 的锁和信号量）均用于控制并发安全性。

#### Side Effects

- schedules asynchronous page generation on the provided `kota::event_loop`

#### Reads From

- first `const int &` parameter
- second `const int &` parameter
- first `std::string_view` parameter
- `std::uint32_t` parameter
- second `std::string_view` parameter
- `kota::event_loop &` event loop reference

#### Writes To

- internal state or task queue of the `kota::event_loop`

#### Usage Patterns

- Callers schedule the returned task on the event loop and run it
- Used to initiate asynchronous page generation without blocking

### `clore::generate::write_pages`

Declaration: `src/generate/generate.cppm:61`

Definition: `src/generate/scheduler.cppm:2035`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::write_pages` 是生成流程的顶层协调器，它接收一个 `config` 对象和一个输出根路径 `output_root`，并驱动完整的页面生成管线。内部首先调用 `prepare_generation_context` 构建 `PreparedGenerationContext`，该结构包含所有计划、符号分析目标、提示请求和链接信息。随后创建 `PageGenerationScheduler`，它持有 `DependencyTracker`、`WorkQueue` 和 `PageRenderer` 等核心组件。`DependencyTracker` 根据 `PreparedGenerationContext` 初始化所有 `PageState`，记录每个页面的依赖计数、提示提交状态和写入状态，并维护 `ready_candidates_` 优先队列用于就绪页面调度。`WorkQueue` 管理两种工作项 (`SymbolAnalysisWork` 和 `PagePromptWork`)，通过 `enqueue` / `dequeue` 与工作线程交互，并提供 `available_` 信号量来同步任务分发。

调度器启动 `worker_count` 个 `WorkerActivity`，每个工作线程循环调用 `worker_task`：从 `WorkQueue` 出队工作，根据工作类型分别执行。对于符号分析工作，调用 `run_symbol_analysis_task`，它内部通过 `request_llm_async` 发起 LLM 请求，解析输出后调用 `DependencyTracker::finish_symbol_prompt` 更新状态，并释放依赖的页面。对于页面提示工作，调用 `run_page_prompt_task`，同样发起 LLM 请求，完成后将结果写入 `PageState`，并通过 `DependencyTracker::mark_symbol_ready` 或 `mark_page_written` 推进依赖链。当某个页面的所有依赖满足且提示完成后，调度器调用 `render_ready_page` 收集该页面的所有提示输出，结合 `PageRenderer` 的 `emit_pages` 或 `emit_pages_async` 生成最终页面文件。整个过程中 `DependencyTracker` 跟踪 `failures` 和 `finished_count`，调度器通过 `retry_limit_exceeded`、`record_consecutive_failure` 处理错误重试，并在必要时调用 `maybe_stop_workers` 或 `work_queue_.stop` 终止循环。函数最终返回写入的页面总数（来自 `PageRenderer::written_page_count`）或错误代码。

#### Side Effects

- writes page files to disk via `write_page`

#### Reads From

- context identifier (int)
- base output path (`std::string_view`)
- internal page plan or generation state

#### Writes To

- output files at the base path
- file system content

#### Usage Patterns

- called as a top-level generation entry point
- used after page plans are built
- paired with `generate_pages` or `generate_pages_async`

## Internal Structure

`generate` 模块负责文档页面的核心生成流程，它在顶层对外提供四个公共入口：干运行验证、同步生成、异步生成以及持久化写入。模块的导入层明确依赖 `config` 模块来获取运行时配置（如并发数、输出路径），并依赖 `extract` 模块获得已解析的项目结构和符号信息。在内部，生成逻辑被拆分为三个可组合的层次：验证层（`generate_dry_run`）在不产生实际页面的前提下校验输入有效性；异步驱动层（`generate_pages_async`）围绕 `kota::event_loop` 调度生成任务，允许调用者将任务提交到指定的事件循环；结果输出层（`write_pages`）独立于生成过程，将内存中的页面集合写入文件系统。`generate_pages` 则作为同步执行的封装，隐含地在内部管理事件循环的生命周期。这种分层使得调用方可以灵活组合各步骤，例如在异步工作流中先干运行检查、再异步生成、最后单独写盘，而无需耦合于特定的执行模型。

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)

