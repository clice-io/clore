---
title: 'Module generate'
description: '模块 clore::generate 负责组织和执行文档页面的生成任务，包含干运行、同步生成、异步生成以及页面写入四种公开操作。它对外暴露 generate_dry_run、generate_pages、generate_pages_async 和 write_pages 四个函数，分别对应模拟验证、完整生成、基于事件循环的异步生成和结果持久化。模块内部管理生成所需的配置参数（如模型选择、速率限制、输出根路径），依赖 config 与 extract 模块提供的基础设施，是文档生成子系统的控制核心。'
layout: doc
template: doc
---

# Module `generate`

## Summary

模块 `clore::generate` 负责组织和执行文档页面的生成任务，包含干运行、同步生成、异步生成以及页面写入四种公开操作。它对外暴露 `generate_dry_run`、`generate_pages`、`generate_pages_async` 和 `write_pages` 四个函数，分别对应模拟验证、完整生成、基于事件循环的异步生成和结果持久化。模块内部管理生成所需的配置参数（如模型选择、速率限制、输出根路径），依赖 `config` 与 `extract` 模块提供的基础设施，是文档生成子系统的控制核心。

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

Definition: `generate/scheduler.cppm:1932`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数是生成管线的干运行入口，用于在生产模式下绕过文件写入而仅模拟全流程。内部首先通过 `prepare_generation_context` 构造 `PreparedGenerationContext`，再以 `dry_run = true` 初始化 `PageGenerationScheduler`。调度器实例化 `PageRenderer` 时将该标志存入其 `dry_run_` 字段，从而控制后续所有输出操作仅更新内存中的 `dry_run_pages_` 计数，而非实际落盘。算法沿袭常规生成的依赖驱动调度：`DependencyTracker` 管理页面状态与符号分析依赖，`WorkQueue` 协调并发任务，`PageGenerationScheduler` 的 `run` 方法驱动循环，依次执行符号分析、页面提示词请求及结果处理。最终函数返回 `PageRenderer::dry_run_pages()` 的值，即干运行期间模拟生成的页面数。

#### Side Effects

No observable side effects are evident from the extracted code.

### `clore::generate::generate_pages`

Declaration: `generate/generate.cppm:28`

Definition: `generate/scheduler.cppm:1991`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function orchestrates a multi‑stage generation pipeline that transforms a prepared generation context into fully rendered documentation pages. Internally, it constructs a `PageGenerationScheduler` backed by a `DependencyTracker` and a `WorkQueue`. The scheduler first submits symbol‑analysis tasks for every documentable symbol; as each analysis finishes, it releases dependent page‑prompt tasks, which are themselves fed into the work queue. The `DependencyTracker` maintains a directed graph of unsatisfied dependencies per `PageState` and exposes ready candidates via `pop_ready_candidate`. All LLM‑bound work is dispatched through `request_llm_async`, and both prompt outputs and rendered pages are cached using a `cache_index_` to avoid redundant computation.

Worker threads (`WorkerActivity`) repeatedly dequeue `ScheduledWork` items — either `SymbolAnalysisWork` or `PagePromptWork` — execute the corresponding task, record successes or failures via `PageGenerationScheduler::finish_symbol_prompt` / `finish_page_prompt_work`, and then call `try_submit_ready_pages` to push newly unblocked work. Once all prompts for a page are complete, a `RenderPageWork` item triggers `PageRenderer::emit_pages`. The function also handles concurrency limits (`rate_limit`), retry logic (`consecutive_failures_` / `retry_limit_exceeded_`), dry‑run mode, and deferred re‑evaluation of symbol analysis through `WorkQueue::enqueue_deferred` / `flush_deferred`. The entire flow terminates when the scheduler’s run loop ends, after which directory index pages are built and the function returns the final result code.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Usage Patterns

- no explicit usage patterns evident from the evidence

### `clore::generate::generate_pages_async`

Declaration: `generate/generate.cppm:37`

Definition: `generate/scheduler.cppm:1969`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数负责协调整个异步页面生成流程。它在提供的 `kota::event_loop` 上调度一个协程任务，该任务首先通过 `prepare_generation_context` 和 `collect_documentable_symbols` 准备上下文，然后构造一个 `PageGenerationScheduler` 实例。调度器内部持有 `DependencyTracker`、`WorkQueue`、`PageRenderer` 以及 LLM 请求相关的配置（`config_`、`model_`、`rate_limit`）。算法首先按计划组织所有页面状态，初始化依赖关系，然后启动多个 worker 任务（`worker_task`）并发处理符号分析与页面提示生成。

控制流围绕 `WorkQueue` 的任务调度与 `DependencyTracker` 的就绪候选机制展开。每个 worker 从队列中取出 `ScheduledWork`，根据工作类型分别执行 `run_symbol_analysis_task` 或 `run_page_prompt_task`。符号分析结果通过 `finish_*_symbol_prompt` 释放下游依赖，触发页面提示的生成；页面提示生成结果则通过 `finish_page_prompt_work` 积累输出。当所有依赖满足后，通过 `render_ready_page` 调用 `PageRenderer` 将页面渲染为文件。整个过程利用 `WorkQueue::flush_deferred`、`DependencyTracker::release_dependents` 等方法确保因果序，同时借助 `persistent_cache_key` 和 `prompt_cache_identity_for_page_request` 实现 LLM 请求的缓存。失败处理通过 `mark_page_failed` 记录错误并可能触发重试，最终在 `PageGenerationScheduler::run` 返回前等待所有工作完成或超出重试限制。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const int &`
- `const int &`
- `std::string_view`
- `std::uint32_t`
- `std::string_view`
- `kota::event_loop &`

#### Usage Patterns

- Callers must schedule the returned task on the loop and run it.
- Used for asynchronous page generation.

### `clore::generate::write_pages`

Declaration: `generate/generate.cppm:44`

Definition: `generate/scheduler.cppm:2010`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::write_pages` 协调整个页面生成管线。它首先调用 `prepare_generation_context` 构造 `PreparedGenerationContext`，其中包含 `plan_set`、`prompt_requests_by_plan`、`symbol_analysis_targets` 等结构。随后创建一个 `PageRenderer` 实例（持有 `output_root_` 与 `dry_run_` 标志），并构建 `PageGenerationScheduler`，传入配置、模型、上下文、LLM 模型、速率限制以及事件循环。调度器的 `run` 方法启动多个 `worker_task`，通过 `WorkQueue` 分发 `ScheduledWork`（如 `PagePromptWork` 和 `SymbolAnalysisWork`）。内部使用 `DependencyTracker` 管理页面依赖：通过 `initialize_ready_candidates` 确定无依赖的初始候选页，当符号分析完成后调用 `release_dependents` 或 `mark_symbol_ready` 以解锁等待的页面，并通过 `pop_ready_candidate` 获取可提交的页面。每个页面依次执行符号分析（`run_symbol_analysis_task`）和页面提示（`run_page_prompt_task`），期间利用 `PagePromptWork::cache_identity` 和 `prompt_cache_identity_for_page_request` 进行缓存判断，并通过 `perform_prompt_request` 发起 LLM 请求。所有任务通过 `WorkQueue::enqueue_deferred` 和 `flush_deferred` 实现异步调度，并受 `WorkQueue::stopped_` 和 `retry_limit_exceeded_` 控制。最终，`render_ready_page` 调用 `PageRenderer::emit_pages` 或 `emit_pages_async` 将生成的 `PagePromptWork::output_key` 写入磁盘，同时 `update_page_summary_cache` 汇集摘要，并在所有页面完成后触发 `build_directory_index_pages` 生成目录索引页。

#### Side Effects

- Writes generated documentation pages to the output location specified by the string view parameter.

#### Reads From

- The integer reference parameter (likely representing a page plan or set of pages)
- The string view parameter (likely representing an output directory path)

#### Writes To

- Files on the filesystem corresponding to generated pages

#### Usage Patterns

- Called during the documentation generation process to persist all generated pages
- Likely invoked by higher-level generation functions such as `clore::generate::generate_pages`

## Internal Structure

`generate` 模块对外提供四个公共入口点：同步的 `generate_dry_run`、`generate_pages`、`write_pages`，以及异步的 `generate_pages_async`。模块内部将页面生成流程拆解为配置解析、模型加载、速率限制和输出路径管理四个独立环节，这些环节共享模块级别的变量 `config`、`model`、`rate_limit`、`output_root` 和 `llm_model`，实现了状态与逻辑的分离。`generate_pages` 和 `generate_dry_run` 分别代表执行写入与不写入的实际生成与干运行模式，而 `write_pages` 专门负责将生成的页面计划序列化到文件系统。

模块依赖关系清晰：通过 `import config` 获取全局生成配置，`import extract` 获得从源代码提取的结构化信息作为生成输入，同时依赖标准库 `std` 提供基础设施。在实现结构上，模块采用分层设计：底层依赖配置与提取模块提供原始数据，中层通过限速与模型管理控制生成流程，上层则通过同步或异步接口暴露给调用者。异步版本 `generate_pages_async` 将生成任务包装为可调度对象，需由调用者挂载在 `kota::event_loop` 上显式运行，从而将事件循环生命周期管理完全交给调用方，降低了模块与异步引擎的耦合度。

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)

