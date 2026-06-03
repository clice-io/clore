---
title: 'Module generate:dryrun'
description: '该模块负责生成文档生成管线的干运行（dry-run）输出，用于预览即将执行的生成计划。它公开了四个主要函数：build_llms_page 构建 LLM 交互页面，build_request_estimate_page 构建请求估计页面，build_dry_run_page_summary_texts 生成每个页面的摘要文本，以及 page_summary_cache_key_for_request 提供缓存键。模块内部通过 RequestEstimate 结构体统计各类文档生成请求（如符号分析、模块摘要、页面计数等）的数量，并利用 estimate_request_count 进行计算，同时依赖 config、extract、generate:analysis 和 generate:model 模块获取配置、提取结果和分析支持。'
layout: doc
template: doc
---

# Module `generate:dryrun`

## Summary

该模块负责生成文档生成管线的干运行（dry-run）输出，用于预览即将执行的生成计划。它公开了四个主要函数：`build_llms_page` 构建 LLM 交互页面，`build_request_estimate_page` 构建请求估计页面，`build_dry_run_page_summary_texts` 生成每个页面的摘要文本，以及 `page_summary_cache_key_for_request` 提供缓存键。模块内部通过 `RequestEstimate` 结构体统计各类文档生成请求（如符号分析、模块摘要、页面计数等）的数量，并利用 `estimate_request_count` 进行计算，同时依赖 `config`、`extract`、`generate:analysis` 和 `generate:model` 模块获取配置、提取结果和分析支持。

## Imports

- [`config`](../config/index.md)
- [`extract`](../extract/index.md)
- [`generate:analysis`](analysis.md)
- [`generate:model`](model.md)

## Imported By

- [`generate:scheduler`](scheduler.md)

## Functions

### `clore::generate::build_dry_run_page_summary_texts`

Declaration: `src/generate/dryrun.cppm:27`

Definition: `src/generate/dryrun.cppm:332`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数遍历输入的 `prompt_requests`，对每个 `request` 调用 `page_summary_cache_key_for_request(plan, request)` 来检查是否存在有效的缓存键；若不存在则直接跳过。对于有效的请求，它通过 `fallback_page_summary_for_request(plan, request)` 生成摘要文本，并仅在摘要非空时将其以 `prompt_request_key(request)` 为键存入返回的 `summary_texts` 映射中。整个过程依赖于 `page_summary_cache_key_for_request`、`fallback_page_summary_for_request` 和 `prompt_request_key` 三个辅助函数，没有额外的复杂控制流或状态管理。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- 参数 `plan`（类型 `const PagePlan&`）
- 参数 `prompt_requests`（类型 `const std::vector<PromptRequest>&`）

#### Writes To

- 局部变量 `summary_texts`（类型 `std::unordered_map<std::string, std::string>`）并返回该映射

#### Usage Patterns

- 用于 dry run 阶段收集每个请求的页面摘要文本
- 被 `generate_dry_run` 或其他生成流程调用以预取摘要数据

### `clore::generate::build_llms_page`

Declaration: `src/generate/dryrun.cppm:35`

Definition: `src/generate/dryrun.cppm:349`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_llms_page` 通过将所有已规划页面类型聚合为单一机器可读索引文件（路径固定为 `"llms.txt"`）来实现 `LLMs` 页面的生成。算法首先从 `config` 中提取 `project_name`，然后构建 Markdown 内容缓冲区，依次写入标题、项目名称、一段固定说明文字，以及指向 `index.md` 的链接；若 `request_estimate_path` 非空，则追加一条 Dry Run 请求估计的链接。核心控制流是通过三次调用 `append_llms_section`，分别以 `PageType::Module`、`PageType::Namespace` 和 `PageType::File` 为参数，将 `plan_set` 中对应类型的页面条目按标签分组追加到 `content` 末尾。该函数依赖 `project_name_from_config`（从 `config` 提取项目名称）、`append_llms_section`（格式化并追加特定类型的页面列表）以及 `GeneratedPage` 结构体（封装最终输出页面的标题、路径和内容）。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan_set`
- `config`
- `request_estimate_path`

#### Usage Patterns

- Called to generate the `llms.txt` page
- Used in the page generation pipeline

### `clore::generate::build_request_estimate_page`

Declaration: `src/generate/dryrun.cppm:31`

Definition: `src/generate/dryrun.cppm:246`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数首先调用 `estimate_request_count`，传入 `plan_set` 和 `model`，得到一个 `RequestEstimate` 对象 `estimate`，其中汇总了各类提示任务的数量。接着通过 `project_name_from_config` 从 `config` 中提取项目名称。随后，函数以固定模板向一个预分配的 `std::string content` 中写入 YAML 头信息、标题以及项目名，再依次输出 `estimate` 中各个字段的数值——包括总请求数、页面请求数、符号请求数，以及更细分的命名空间、模块、索引、函数、类型、变量的分析与摘要请求。整个过程中没有分支或循环，仅依赖字符串格式化操作；最后将填充好的 `content` 与固定路径 `"request-estimate.md"` 和标题组装为 `GeneratedPage` 对象并返回。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan_set` (const `PagePlanSet`&) for estimating task counts
- `model` (const `extract::ProjectModel`&) for estimation
- `config` (const `config::TaskConfig`&) to extract project name

#### Writes To

- Constructs and returns a `GeneratedPage` with fields `title`, `relative_path`, and `content`

#### Usage Patterns

- Used in dry-run code generation to produce a summary page of estimated prompt tasks
- Typically called during the generation phase of a documentation pipeline

### `clore::generate::page_summary_cache_key_for_request`

Declaration: `src/generate/dryrun.cppm:39`

Definition: `src/generate/dryrun.cppm:309`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::page_summary_cache_key_for_request` 通过一个基于 `PromptRequest` 的 `kind` 字段的 `switch` 语句来决定是否以及如何生成缓存键。对于 `PromptKind::NamespaceSummary` 和 `PromptKind::ModuleSummary` 两类请求，它尝试从 `PagePlan` 的 `owner_keys` 集合中提取第一个元素作为缓存键；如果该集合为空，则返回 `std::nullopt` 表示不可缓存。对于其余所有已知的 `PromptKind` 值（包括 `ModuleArchitecture`、`IndexOverview`、各种 `*Analysis` 以及 `*Summary` 类型），该函数直接返回 `std::nullopt`，表明这些请求无需缓存键，每次调用都应重新计算结果。`switch` 语句后的兜底返回 `std::nullopt` 保证了未明确匹配的输入也能被安全处理。该函数的内部逻辑完全依赖于 `PromptKind` 枚举的成员值和 `PagePlan::owner_keys` 这一容器成员。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.owner_keys`
- `request.kind`

#### Usage Patterns

- Used to compute a cache key for page summaries when the request is for namespace or module summary.

## Internal Structure

模块 `generate:dryrun` 是文档生成管线中用于“试运行”阶段的核心组件，它不实际调用 LLM 而是模拟各页面生成所需的请求开销与摘要文本。其公共接口由四个顶层函数构成：`build_dry_run_page_summary_texts`、`page_summary_cache_key_for_request`、`build_request_estimate_page` 和 `build_llms_page`，它们分别负责生成摘要内容、缓存键、请求估算页面以及 LLM 交互页面。这些函数通过导入的 `config`、`extract`、`generate:analysis` 和 `generate:model` 模块获取配置、提取结果、分析提示和模型数据结构，形成清晰的外部依赖层。

内部实现采用了匿名命名空间封装辅助逻辑，包括 `RequestEstimate` 结构体（细粒度记录各类型请求数量，如符号分析、声明摘要、页面请求等）以及 `estimate_request_count`、`append_llms_section`、`fallback_page_summary_for_request`、`llms_entry_label` 等函数，它们协同完成请求估算和页面内容组装。另有一个 `LabeledPage` 结构体用于组织带标签的页面计划。这种分层使公共 API 保持简洁，而复杂的状态收集与格式化逻辑被隔离在内部单元中，便于单元测试和后续维护。

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)
- [Module generate:analysis](analysis.md)
- [Module generate:model](model.md)

