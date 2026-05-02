---
title: 'Module generate:dryrun'
description: '该模块负责为文档生成管线提供“干运行”（dry run）预览能力，在不实际调用大语言模型的情况下估算并呈现生成请求的规模、类型分布及页面结构。其核心职责包括计算各分类（模块摘要、命名空间页面、类型分析、函数分析等）的请求数量、构建请求估计页面、生成干运行页面摘要文本、以及创建LLMs格式的接入点页面。内部通过 RequestEstimate 结构体聚合请求统计，并利用 LabeledPage 辅助构建带标签的LLM节段。'
layout: doc
template: doc
---

# Module `generate:dryrun`

## Summary

该模块负责为文档生成管线提供“干运行”（dry run）预览能力，在不实际调用大语言模型的情况下估算并呈现生成请求的规模、类型分布及页面结构。其核心职责包括计算各分类（模块摘要、命名空间页面、类型分析、函数分析等）的请求数量、构建请求估计页面、生成干运行页面摘要文本、以及创建`LLMs`格式的接入点页面。内部通过 `RequestEstimate` 结构体聚合请求统计，并利用 `LabeledPage` 辅助构建带标签的LLM节段。

公开实现范围主要包含三个函数：`build_request_estimate_page` 根据上下文标识符构造请求估计页面的整数句柄；`build_dry_run_page_summary_texts` 为目标页面生成摘要文本并返回结果状态；`build_llms_page` 构建`LLMs`专用页面。此外还提供 `page_summary_cache_key_for_request` 用于生成缓存键。所有函数均以整数参数传递标识符，无直接I/O或持久化操作，属于纯计算与内部表示生成。

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

该函数遍历 `prompt_requests` 中的每个请求，对每个请求调用 `page_summary_cache_key_for_request` 检查是否存在有效的缓存键；若不存在则跳过该请求。若存在，则调用 `fallback_page_summary_for_request` 生成回退摘要文本；若生成的摘要为空也跳过该请求。最后将非空摘要以 `prompt_request_key` 为键插入到 `summary_texts` 映射中。整个流程依赖 `page_summary_cache_key_for_request` 判断请求是否需要摘要，以及 `fallback_page_summary_for_request` 提供静态的页面摘要文本，从而在不实际调用 LLM 的情况下完成干运行阶段的摘要生成。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan` parameter
- `prompt_requests` parameter
- internal callees: `page_summary_cache_key_for_request`, `fallback_page_summary_for_request`, `prompt_request_key`

#### Usage Patterns

- Used in dry-run generation to collect precomputed summary texts for prompt requests
- Called as part of the dry-run page building pipeline

### `clore::generate::build_llms_page`

Declaration: `generate/dryrun.cppm:19`

Definition: `generate/dryrun.cppm:333`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_llms_page` 通过组合头部信息与三个分类列表来生成一个 `llms.txt` 页面。首先，它调用 `project_name_from_config` 取得项目名称，并以此构造 Markdown 标题与摘要行。随后，根据 `request_estimate_path` 是否为非空，决定是否嵌入一个指向 Dry Run 估算页面的链接。接着，按固定顺序依次调用 `append_llms_section`，分别处理 `PageType::Module`、`PageType::Namespace` 和 `PageType::File` 三类计划集，将各分类的条目列表追加到 `content` 中。最终将拼接完成的字符串与标题、路径打包为 `GeneratedPage` 结构体返回。整个流程完全依赖 `plan_set`、`config` 和 `request_estimate_path` 三个输入，通过本地字符串构建与三次相同的 `append_llms_section` 调用来完成页面组装。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- Parameter `plan_set` (const `PagePlanSet&`)
- Parameter `config` (const `config::TaskConfig&`)
- Parameter `request_estimate_path` (`std::string_view`)
- Result of `project_name_from_config(config)`

#### Writes To

- Local variable `content` (`std::string`)
- Returned `GeneratedPage` object

#### Usage Patterns

- Called by the documentation generation pipeline to create the `llms.txt` page
- Used in dry-run or final page generation workflows

### `clore::generate::build_request_estimate_page`

Declaration: `generate/dryrun.cppm:15`

Definition: `generate/dryrun.cppm:230`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数通过调用 `estimate_request_count` 获得一个 `RequestEstimate` 实例，该实例聚合了计划中所有页面和符号类别的预估提示任务数量，再借助 `project_name_from_config` 获取项目名称。随后它将标题、描述等元信息拼接成 Markdown 头部，并将 `RequestEstimate` 中的各字段（如 `total_requests`、`page_requests`、`symbol_requests` 以及各细分类别的请求数）通过 `std::format` 格式化为表格式的摘要内容。最终构造并返回一个 `GeneratedPage` 对象，其 `relative_path` 固定为 `"request-estimate.md"`，`title` 为 `"Dry Run Request Estimate"`。整个过程没有复杂控制流，主要依赖 `estimate_request_count` 和 `project_name_from_config` 两个函数，以及 `std::format` 进行字符串格式化。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan_set` (`const PagePlanSet&`)
- `model` (`const extract::ProjectModel&`)
- `config` (`const config::TaskConfig&`)

#### Writes To

- returned `GeneratedPage` object (`.title`, `.relative_path`, `.content`)

#### Usage Patterns

- called during dry-run generation to produce an estimate page without invoking the LLM
- used in `build_request_estimate_page` path when `TaskConfig` indicates dry-run mode

### `clore::generate::page_summary_cache_key_for_request`

Declaration: `generate/dryrun.cppm:23`

Definition: `generate/dryrun.cppm:293`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::page_summary_cache_key_for_request` 通过 `switch` 语句检查传入的 `request.kind`，以决定是否为该请求生成缓存键。对于 `PromptKind::NamespaceSummary` 和 `PromptKind::ModuleSummary` 两种类型，若 `plan.owner_keys` 非空，则返回 `plan.owner_keys.front()` 作为字符串；否则返回 `std::nullopt`。对于其余枚举值（如 `PromptKind::ModuleArchitecture`、`PromptKind::IndexOverview`、`PromptKind::FunctionAnalysis` 等），函数直接返回 `std::nullopt`。该实现依赖 `PagePlan` 与 `PromptRequest` 类型及其内部的 `PromptKind` 枚举，核心控制流仅根据请求种类进行分支，仅对需要所有者标识符的摘要类型生成缓存键，其余情况均表示无需缓存。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- The `plan` parameter, specifically its `owner_keys` member
- The `request` parameter, specifically its `kind` member

#### Usage Patterns

- Called to generate cache keys for page summary requests during page generation
- Used to avoid redundant computations when the same summary request is repeated

## Internal Structure

模块 `generate:dryrun` 负责在不实际调用语言模型的情况下，为文档生成管线提供干运行支持。它主要分解为两个公共入口点：`build_dry_run_page_summary_texts` 仅基于配置和提取信息计算页面摘要文本，`build_request_estimate_page` 则根据标识参数构造请求估算页面的内部句柄。内部拆分为一组匿名命名空间内的辅助结构（如 `RequestEstimate` 和 `LabeledPage`）与函数（如 `estimate_request_count` 和 `append_llms_section`），它们共同实现估算计数、生成标签以及构建 LLMS 章节等任务。依赖关系明确：模块导入 `config`、`extract`、`generate:analysis` 和 `generate:model`，分别用于解析配置、获取源码结构、处理分析响应以及访问页面计划等数据模型。这种分层使得干运行逻辑与管线核心保持解耦，同时通过公共接口提供可预测的预估输出，便于测试与流程控制。

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)
- [Module generate:analysis](analysis.md)
- [Module generate:model](model.md)

