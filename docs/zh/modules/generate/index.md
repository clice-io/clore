---
title: 'Module generate'
description: 'generate 模块负责文档生成的完整流程，包括生成前验证、同步与异步页面构建以及结果输出。它对外提供四个核心函数：generate_dry_run 用于在无副作用的前提下模拟生成过程并返回状态或预估统计；generate_pages 是同步阻塞的入口，接收分析数据、输出路径、并发数及格式标识，在成功时保证输出目录中包含完整且引用一致的文档集；write_pages 将已生成的页面集合写入指定目录；generate_pages_async 则在给定的 kota::event_loop 上执行异步页面生成，调用者必须显式调度返回的任务以触发实际工作。模块内部管理配置、模型、速率限制、页面集合及输出根路径等状态，并依赖 config 模块提供配置数据、依赖 extract 模块提供预处理的符号与分析结果。'
layout: doc
template: doc
---

# Module `generate`

## Summary

`generate` 模块负责文档生成的完整流程，包括生成前验证、同步与异步页面构建以及结果输出。它对外提供四个核心函数：`generate_dry_run` 用于在无副作用的前提下模拟生成过程并返回状态或预估统计；`generate_pages` 是同步阻塞的入口，接收分析数据、输出路径、并发数及格式标识，在成功时保证输出目录中包含完整且引用一致的文档集；`write_pages` 将已生成的页面集合写入指定目录；`generate_pages_async` 则在给定的 `kota::event_loop` 上执行异步页面生成，调用者必须显式调度返回的任务以触发实际工作。模块内部管理配置、模型、速率限制、页面集合及输出根路径等状态，并依赖 `config` 模块提供配置数据、依赖 `extract` 模块提供预处理的符号与分析结果。

## Imports

- [`config`](../config/index.md)
- [`extract`](../extract/index.md)
- `std`

## Imported By

- [`agent`](../agent/index.md)
- [`agent:tools`](../agent/tools.md)

## Functions

### `clore::generate::generate_dry_run`

Declaration: `generate/generate.cppm:25`

Definition: `generate/scheduler.cppm:1888`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::generate_dry_run` 首先通过 `prepare_generation_context` 和 `prepare_symbol_analyses_for_dry_run` 构建完整的 `PreparedGenerationContext` 与 `PreparedSymbolAnalyses`。随后创建一个 `PageGenerationScheduler` 实例，其构造函数将 `dry_run` 参数设为 `true`，并基于给定配置、模型、上下文、LLM 模型、速率限制、输出根路径及事件循环进行初始化。调度器执行 `run` 方法，遍历所有计划页面：内部会依次调用 `run_symbol_analysis_task` 与 `run_page_prompt_task` 来处理依赖分析与提示请求，但干运行模式下 `PageRenderer` 不会实际执行 `emit_pages` 或 `update_summaries` 等含输出写入的操作。`DependencyTracker` 全程追踪各 `PageState` 的 `unsatisfied_deps` 与 `completed_prompts`，同时 `scheduler.expected_llm_requests_`、`page_prompt_cache_hits_` 等计数被累计以反映预期 LLM 调用次数与缓存命中情况。函数最终返回 `scheduler.dry_run_pages()` 的结果，该值表示干运行中预计生成的页面数量。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the two const int& parameters

#### Usage Patterns

- called to simulate generation without side effects

### `clore::generate::generate_pages`

Declaration: `generate/generate.cppm:28`

Definition: `generate/scheduler.cppm:1947`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::generate_pages` 是页面生成管线的顶层协调函数。它首先调用 `prepare_generation_context` 从配置与模型参数构建一个 `PreparedGenerationContext`，其中包含计划集合、提示请求、符号分析目标及链接映射。随后实例化一个 `PageGenerationScheduler`，传入该上下文、LLM 模型标识、速率限制、输出根目录和事件循环。调度器的 `run` 方法驱动一个 `WorkQueue`，该队列区分两类工作项：`SymbolAnalysisWork`（执行深层符号分析）和 `PagePromptWork`（根据提示生成页面内容）。一个 `DependencyTracker` 维护页面间的依赖关系，确保所有必需的符号分析完成后才提交页面的提示请求。提示响应可能从持久缓存（`cache_index_` 与 `prompt_cache_identity_for_page_request`）中直接获取，或通过 `request_llm_async` 发起新的 LLM 调用获取。

当所有工作项处理完毕，调度器调用 `render_generated_pages` 将生成的页面写入输出目录，并调用 `build_directory_index_pages` 生成目录索引页面。整个过程依赖 `PageRenderer` 执行最终的文件写出与摘要收集，同时通过 `record_consecutive_failure` 和 `fail_page_work` 实现错误重试限制，确保在 LLM 调用失败或缓存缺失时不会无限重复。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Usage Patterns

- Called as the main generation entry point

### `clore::generate::generate_pages_async`

Declaration: `generate/generate.cppm:37`

Definition: `generate/scheduler.cppm:1925`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数的核心实现围绕 `PageGenerationScheduler` 展开。首先，它使用传入的配置、模型、输出根目录和事件循环构造一个调度器实例，并在内部通过 `prepare_generation_context` 解析计划与符号分析目标，初始化 `DependencyTracker` 以追踪页面状态与依赖关系。随后，调度器启动一个多阶段异步流水线：通过 `WorkQueue` 管理并发工作量，利用 `worker_task` 循环从队列中取出 `ScheduledWork` 并执行——工作类型包括符号分析任务（调用 `run_symbol_analysis_task`）和页面提示任务（调用 `run_page_prompt_task`）。每个任务完成后会更新依赖状态，并通过 `try_submit_ready_pages` 将依赖就绪的页面提交为新的提示工作。该过程受 `rate_limit` 和 `consecutive_failures_` 控制，确保在 LLM 请求频率和失败重试方面具有可配置的鲁棒性。

当所有页面提示工作完成后，调度器通过 `maybe_stop_workers` 终止工作线程，并转入渲染阶段：`render_ready_page` 收集页面输出并调用 `PageRenderer::emit_pages_async` 将生成的内容写入文件系统。整个过程完全异步运行在给定的 `kota::event_loop` 上，调用者需要将返回的任务调度到该循环中并启动执行；函数内部通过 `WorkQueue` 的信号量（`available_`）与事件循环协同调度，确保流水线不会阻塞外部循环。

#### Side Effects

- schedules work on the provided event loop
- potentially writes generated pages to the output location

#### Reads From

- first integer parameter
- second integer parameter
- output path string view
- 32-bit integer parameter
- second string view parameter
- event loop reference (for state reading)

#### Writes To

- event loop (by scheduling work)
- generated pages (implied by 'page generation')

#### Usage Patterns

- callers schedule the returned task on the event loop
- run the scheduled task on the loop

### `clore::generate::write_pages`

Declaration: `generate/generate.cppm:44`

Definition: `generate/scheduler.cppm:1966`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function `clore::generate::write_pages` orchestrates the end-to-end generation of documentation pages. It first prepares the generation context by calling `prepare_generation_context`, which builds the collection of documentable symbols and prompt requests. A `PageGenerationScheduler` is then instantiated with the configuration, LLM model, `PreparedGenerationContext`, output root, event loop, and optional dry-run flag. The scheduler’s `run` method drives the core workflow: it initializes `DependencyTracker`, submits symbol analysis tasks and page prompt requests via `submit_after_symbol_analysis` and `schedule_symbol_analysis`, and processes the asynchronous work queue (`WorkQueue`) through worker tasks. Each worker invocation of `worker_task` polls the queue, runs `run_symbol_analysis_task` or `run_page_prompt_task`, and updates state through the tracker. After LLM requests complete, the scheduler calls `render_ready_page` to generate final output via `PageRenderer::emit_pages_async` and `render_generated_pages`. The function returns a status code indicating success or failure, with metrics gathered from the scheduler’s counters (`llm_requests_issued_`, `written_output_count_`, etc.).

#### Side Effects

- Writes documentation pages to the filesystem

#### Reads From

- The integer parameter (likely a page plan set identifier)
- The string view parameter (likely an output path)

#### Writes To

- Files on disk

#### Usage Patterns

- Called during documentation generation to write all pages

## Internal Structure

`generate`模块按照功能关注点进行了清晰的分解：提供干运行验证、同步生成、独立写出以及异步生成四项公开入口，分别对应 `generate_dry_run`、`generate_pages`、`write_pages` 和 `generate_pages_async`。这些函数共享同一声明单元（`generate.cppm`）内的一组内部变量，包括配置对象、LLM 模型句柄、速率限制参数以及页面容器，从而在避免重复声明的同时保持状态一致性。模块显式导入 `config`（读取外部设置）、`extract`（利用已提取的符号数据）以及标准库，未引入其他业务模块，体现了向下的依赖方向。内部层次上，同步流程（`generate_pages` → 隐式构建页面计划 → 可选调用 `write_pages` 落盘）与异步流程（`generate_pages_async` 接受 `kota::event_loop` 引用，将生成任务调度到循环之上）并行存在，前者阻塞直至完成，后者将生命周期交给调用者控制。干运行函数则独立模拟完整流程但不产生输出，方便调用者评估生成范围与成本。这种结构使调用方可根据场景选择同步或异步路径，同时通过公开的写出函数将页面渲染与序列化解耦。

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)

