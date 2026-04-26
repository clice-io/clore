---
title: 'Module generate:dryrun'
description: 'generate:dryrun 模块负责实现文档生成流程的干运行（dry-run）模式，在不实际调用大语言模型的情况下，模拟生成过程并输出预估请求量和页面摘要。它公开了 build_dry_run_page_summary_texts、build_request_estimate_page、build_llms_page 和 page_summary_cache_key_for_request 等函数，供外层调用以构建干运行页面、计算请求估计和生成缓存键。内部通过 RequestEstimate 结构体分类统计各种请求（如符号分析、模块总结、页面请求等），并依赖 config、extract、generate:analysis 和 generate:model 模块获取配置、提取结果和分析数据。'
layout: doc
template: doc
---

# Module `generate:dryrun`

## Summary

`generate:dryrun` 模块负责实现文档生成流程的干运行（dry-run）模式，在不实际调用大语言模型的情况下，模拟生成过程并输出预估请求量和页面摘要。它公开了 `build_dry_run_page_summary_texts`、`build_request_estimate_page`、`build_llms_page` 和 `page_summary_cache_key_for_request` 等函数，供外层调用以构建干运行页面、计算请求估计和生成缓存键。内部通过 `RequestEstimate` 结构体分类统计各种请求（如符号分析、模块总结、页面请求等），并依赖 `config`、`extract`、`generate:analysis` 和 `generate:model` 模块获取配置、提取结果和分析数据。

## Imports

- [`config`](../config/index.md)
- [`extract`](../extract/index.md)
- [`generate:analysis`](analysis.md)
- [`generate:model`](model.md)
- `std`

## Imported By

- [`generate:scheduler`](scheduler.md)

## Functions

### `clore::generate::build_dry_run_page_summary_texts`

Declaration: `generate/dryrun.cppm:11`

Definition: `generate/dryrun.cppm:316`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_dry_run_page_summary_texts` 遍历输入的 `prompt_requests` 向量，为每个 `request` 生成干运行的页面摘要文本。其核心算法基于三个依赖函数的协作：首先通过 `page_summary_cache_key_for_request` 判断当前请求是否需要摘要，若无缓存键则跳过；接着调用 `fallback_page_summary_for_request` 获取降级摘要，若结果为空则继续；最后以 `prompt_request_key` 生成的字符串为键，将非空摘要插入结果映射。整个流程依赖 `PagePlan` 对象 `plan` 提供的上下文，并通过条件跳过无意义请求来避免无效计算，最终返回一个按键（即请求标识）索引的摘要集合。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan`
- `prompt_requests`
- `page_summary_cache_key_for_request`
- `fallback_page_summary_for_request`

#### Usage Patterns

- Used in dry run page generation to collect summary texts for each prompt request.
- Likely called by `generate_dry_run` or related functions.

### `clore::generate::build_llms_page`

Declaration: `generate/dryrun.cppm:19`

Definition: `generate/dryrun.cppm:333`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

实现摘要：`clore::generate::build_llms_page` 负责构造一个 Markdown 格式的索引页面，输出为 `llms.txt`。它首先通过 `project_name_from_config` 提取项目名称，并预分配内容缓冲区。随后依次附加项目标题行、描述性说明以及指向 API 参考页面 (`index.md`) 的链接；若 `request_estimate_path` 非空，则附加指向 Dry Run 请求估算页面的链接。核心流程是三次调用 `append_llms_section`，分别绑定 `PageType::Module`、`PageType::Namespace` 和 `PageType::File`，将 `plan_set` 中的对应章节按标签（模块、命名空间、文件）追加到内容末尾。最终返回包含标题、固定相对路径 `"llms.txt"` 和构建好的 `content` 的 `GeneratedPage` 对象。该函数依赖 `project_name_from_config` 与 `append_llms_section` 两个辅助函数完成配置读取和分节生成。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `config::TaskConfig` `config` (via `project_name_from_config`)
- `PagePlanSet` `plan_set`
- `std::string_view` `request_estimate_path`

#### Writes To

- returned `GeneratedPage` object (fields `title`, `relative_path`, `content`)

#### Usage Patterns

- called during documentation generation to produce the `llms.txt` index page

### `clore::generate::build_request_estimate_page`

Declaration: `generate/dryrun.cppm:15`

Definition: `generate/dryrun.cppm:230`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数主要将来自 `clore::generate::(anonymous namespace)::estimate_request_count` 的统计结果与 `clore::generate::(anonymous namespace)::project_name_from_config` 提供的项目名称组合起来，组装成一个完整的 `GeneratedPage`。内部流程是：先为给定的 `plan_set` 和 `model` 计算 `RequestEstimate`，然后通过字符串拼接和 `std::format` 构建一个固定布局的 Markdown 内容，其中包括 YAML 元数据、项目名称、总计统计、页面任务和符号任务的详细分项，最后将内容包裹在 `GeneratedPage` 中并设置路径为 `"request-estimate.md"`。

该函数不涉及任何分支或循环，完全是一条顺序执行的格式化流水线。它的核心依赖是 `RequestEstimate` 结构（提供 `total_requests`、`page_requests`、`symbol_requests` 等计数器）以及 `project_name_from_config` 的返回值。生成的页面专门用于 dry‑run 模式，不触发任何模型调用，仅报告当前计划排期的预期提示任务数量。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameters `plan_set`, `model`, `config`
- callees `estimate_request_count`, `project_name_from_config`

#### Writes To

- local variable `content`
- returned `GeneratedPage`'s fields

#### Usage Patterns

- called during dry-run generation
- builds page for estimating prompt task load

### `clore::generate::page_summary_cache_key_for_request`

Declaration: `generate/dryrun.cppm:23`

Definition: `generate/dryrun.cppm:293`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数通过一个 switch 语句检查 `request.kind` 的枚举值。当 `request.kind` 为 `PromptKind::NamespaceSummary` 或 `PromptKind::ModuleSummary` 时，若 `plan.owner_keys` 非空，则返回 `plan.owner_keys.front()` 作为可选的缓存键；否则返回 `std::nullopt`。此逻辑依赖于 `PagePlan` 类型的 `owner_keys` 字段，该字段在调用上下文中由调用方（如 `build_request_estimate_page`）提供。

对于其他所有 `PromptKind` 枚举值（包括 `ModuleArchitecture`、`IndexOverview`、`FunctionAnalysis`、`TypeAnalysis`、`VariableAnalysis` 以及所有声明/实现摘要种类），该函数直接返回 `std::nullopt`，表示对应请求不参与页面摘要缓存。Switch 末尾的 `return std::nullopt` 确保了所有分支均被覆盖，但实际执行中不会到达该处。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.owner_keys`
- `request.kind`

#### Usage Patterns

- Called to produce a cache key for page summary caching
- Filters requests that should be cached based on prompt kind

## Internal Structure

模块 `generate:dryrun` 是文档生成管线中干运行（dry-run）阶段的核心，负责在无需实际调用大模型的情况下估算请求量并构建预览页面。它通过内部匿名命名空间分解了多个职责：`RequestEstimate` 结构聚合了各类分析请求（符号提取、函数/类型/变量分析、模块摘要等）的计数，`estimate_request_count` 利用来自 `generate:model` 的`plan_set` 计算这些估计值；`append_llms_section` 借助 `LabeledPage` 辅助结构生成面向 LLM 的页面片段。模块从 `config` 获取项目配置，从 `extract` 获取符号数据，并依赖 `generate:analysis` 的分析结果，形成清晰的导入层次。公开的函数如 `build_dry_run_page_summary_texts`、`build_request_estimate_page`、`build_llms_page` 和 `page_summary_cache_key_for_request` 提供了干运行场景下摘要文本生成、请求估计、LLMS 页面构建及缓存键生成等接口，使得调用者可以独立演练生成流程而无需依赖外部模型。

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)
- [Module generate:analysis](analysis.md)
- [Module generate:model](model.md)

