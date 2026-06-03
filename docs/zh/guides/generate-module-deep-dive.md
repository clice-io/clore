---
title: 'Clore 生成模块深度解析'
description: 'Guide: Clore 生成模块深度解析'
layout: doc
template: doc
---

# 生成模块深度解析（`clore::generate`）

## 概述

`clore::generate` 是 Clore 的文档生成引擎，负责将提取得到的 `ProjectModel` 转化为一组 Markdown 文档页面。它通过页面计划（Page Planning）、AI 分析（LLM-based Analysis）和 Markdown 渲染（Rendering）三个阶段完成文档生成。

**模块文件**（14 个 C++20 模块单元）：

| 模块 | 文件 | 符号数 | 职责 |
|------|------|--------|------|
| `generate` | `src/generate/generate.cppm` | 19 | 顶层入口（`generate_pages_async`） |
| `generate:planner` | `src/generate/planner.cppm` | 75 | 页面规划（`PagePlan` 生成） |
| `generate:analysis` | `src/generate/analysis.cppm` | 126 | LLM prompt 构建与响应解析 |
| `generate:evidence` | `src/generate/evidence.cppm` | 242 | 证据包（`EvidencePack`）构建 |
| `generate:evidence_builder` | `src/generate/evidence_builder.cppm` | 72 | 证据包辅助构建 |
| `generate:scheduler` | `src/generate/scheduler.cppm` | 448 | 页面生成调度器（核心编排） |
| `generate:markdown` | `src/generate/markdown.cppm` | 254 | Markdown 文档模型与渲染 |
| `generate:render/common` | `src/generate/render/common.cppm` | 168 | 通用渲染工具 |
| `generate:render/diagram` | `src/generate/render/diagram.cppm` | 96 | Mermaid 图表渲染 |
| `generate:render/page` | `src/generate/render/page.cppm` | 178 | 页面布局渲染 |
| `generate:render/symbol` | `src/generate/render/symbol.cppm` | 300 | 符号文档渲染 |
| `generate:model` | `src/generate/model.cppm` | 242 | 生成阶段数据模型 |
| `generate:cache` | `src/generate/cache.cppm` | 96 | 生成缓存 |
| `generate:dryrun` | `src/generate/dryrun.cppm` | 73 | 干运行模式 |

---

## 页面规划（Planner）

### 流程

```
build_page_plan_set(config, model) → PagePlanSet
  ├─▶ 1. 枚举内容页面
  │     ├─▶ if model.uses_modules:
  │     │     └─▶ enumerate_module_pages(builder)    → 每个模块一页
  │     └─▶ else:
  │           └─▶ enumerate_file_pages(builder)       → 每个文件一页
  │
  ├─▶ 2. 枚举命名空间页面
  │     └─▶ enumerate_namespace_pages(builder)        → 每个命名空间一页
  │
  ├─▶ 3. 枚举索引页面
  │     └─▶ enumerate_index_page(builder)             → 项目首页
  │
  ├─▶ 4. 验证路径冲突
  │     └─▶ validate_no_path_conflicts(path_entries)
  │
  └─▶ 5. 拓扑排序
        └─▶ topological_sort(plans, ...)              → 按依赖顺序排列
```

### 页面类型（`PageType`）

| 类型 | 说明 |
|------|------|
| `Index` | 项目索引/首页 |
| `File` | 源文件页面（传统项目） |
| `Module` | C++20 模块页面（模块项目） |
| `Namespace` | 命名空间页面 |

### `PagePlan` 结构

每个页面包含：
- **`page_id`**：唯一标识（如 `"ns:clore::extract"` 或 `"module:extract:ast"`）
- **`owner_keys`**：归属的符号/实体键列表
- **`depends_on_pages`**：依赖的其他页面 ID（用于拓扑排序）
- **`linked_pages`**：关联页面 ID（交叉引用）
- **`prompt_requests`**：需要 LLM 分析的请求列表（如类型分析、函数分析等）

---

## Prompt 请求与 AI 分析

### Prompt 类型（`PromptKind`）

| 类型 | 用途 |
|------|------|
| `IndexOverview` | 项目首页概览 |
| `NamespaceSummary` | 命名空间摘要 |
| `ModuleSummary` / `ModuleArchitecture` | 模块摘要与架构 |
| `TypeAnalysis` / `TypeDeclarationSummary` / `TypeImplementationSummary` | 类型分析 |
| `FunctionAnalysis` / `FunctionDeclarationSummary` / `FunctionImplementationSummary` | 函数分析 |
| `VariableAnalysis` | 变量分析 |

### 证据包构建

```
build_evidence_for_*(model, plan, ...) → EvidencePack
  ├─▶ SymbolFact 收集：
  │     ├─▶ 目标符号事实（qualified_name、kind、signature、doc_comment 等）
  │     ├─▶ 本地上下文符号（同命名空间/文件的其他符号）
  │     ├─▶ 依赖上下文符号（被调用/继承的符号）
  │     └─▶ 反向使用上下文符号（调用者/派生类）
  │
  ├─▶ 关联页面摘要收集
  └─▶ 源码片段收集
```

### Prompt 构建与响应解析

```
build_symbol_analysis_prompt(...)
  └─▶ 将 EvidencePack 格式化为 LLM 可读的文本提示

parse_markdown_prompt_output(raw_response, target_key)
  └─▶ 解析 LLM 返回的 Markdown 内容

apply_symbol_analysis_response(analyses, prompt_outputs, ...)
  └─▶ 将分析结果存储到 SymbolAnalysisStore
      ├─▶ FunctionAnalysis：overview、details、usage_patterns、side_effects 等
      ├─▶ TypeAnalysis：overview、details、key_members、invariants 等
      └─▶ VariableAnalysis：overview、details、usage_patterns、mutation_sources 等
```

### 分析缓存

`generate:cache` 模块提供：
- **`CacheIndex`**：缓存索引，将分析结果持久化到磁盘
- **缓存键**：基于符号标识和 prompt 类型生成
- 缓存命中时跳过 LLM 调用，直接从缓存加载分析结果

---

## 页面渲染系统

### 渲染管道

```
render_page_markdown(plan, model, analyses, link_resolver, output_root, ...) → MarkdownDocument
  ├─▶ 根据 page_type 选择渲染策略：
  │     ├─▶ Index 页面   → build_index_page_root()
  │     ├─▶ Namespace 页面 → build_namespace_page_root()
  │     ├─▶ Module 页面  → build_module_page_root()
  │     └─▶ File 页面    → build_file_page_root()
  │
  ├─▶ 页面包含：
  │     ├─▶ 标题/Frontmatter
  │     ├─▶ 依赖/导入关系图（Mermaid 图表）
  │     ├─▶ 命名空间结构图
  │     ├─▶ 符号文档列表
  │     │     ├─▶ 类型（类/结构体/枚举）
  │     │     ├─▶ 函数/方法
  │     │     └─▶ 变量
  │     └─▶ 关联页面链接
  │
  └─▶ render_markdown(document) → std::string
        └─▶ 递归遍历 MarkdownNode 树 → 输出字符串
```

### 符号文档渲染

`append_symbol_doc_pages()` 负责渲染单个符号的文档：

1. 根据 `SymbolDocView`（Declaration / Details / Implementation）选择视图
2. 使用 `add_symbol_doc_links()` 添加交叉引用链接
3. 调用 `add_symbol_analysis_sections()` / `add_symbol_analysis_detail_sections()` 添加 AI 分析结果
4. 通过 `build_symbol_source_locations()` 渲染源码位置链接
5. 通过 `push_location_paragraph()` 生成位置段落

### Markdown 文档模型

`generate:markdown` 模块定义了一个 Markdown 抽象语法树（AST）：

```
MarkdownDocument
  ├─ Frontmatter（title, description, layout, page_template）
  └─ MarkdownNode（树形结构）
       ├─ Paragraph（段落）
       │    └─ InlineFragment（内联元素）
       │         ├─ TextFragment（纯文本）
       │         ├─ CodeFragment（代码片段）
       │         └─ LinkFragment（链接）
       ├─ CodeFence（代码块）
       ├─ BlockQuote（引用块）
       ├─ BulletList（列表）
       ├─ SemanticSection（语义节）
       │    ├─ kind（Section/Namespace/Module/Type/Function/Variable/File/Index）
       │    ├─ heading
       │    ├─ level
       │    ├─ subject_key
       │    └─ children（子节点）
       ├─ MermaidDiagram（Mermaid 图表）
       └─ RawMarkdown（原始 Markdown）
```

### 图表渲染

`generate:render/diagram` 模块使用 **Mermaid`.js`** 语法生成三类图表：

| 函数 | 图表类型 | 用途 |
|------|----------|------|
| `render_import_diagram_code()` | 导入关系图 | 模块/文件的导入依赖 |
| `render_namespace_diagram_code()` | 命名空间图 | 命名空间层级结构 |
| `render_file_dependency_diagram_code()` | 文件依赖图 | 文件间的包含/依赖关系 |
| `render_module_dependency_diagram_code()` | 模块依赖图 | C++20 模块间的依赖关系 |

图表节点数超过阈值（`should_emit_mermaid()`）时自动省略，避免输出过大。

### 交叉引用系统

`LinkResolver`（定义在 `generate:model` 中）负责管理所有交叉引用：

```
LinkResolver
  ├─ name_to_path       : 符号名 → 页面路径
  ├─ namespace_to_path  : 命名空间名 → 页面路径
  ├─ module_to_path     : 模块名 → 页面路径
  └─ page_id_to_title   : 页面 ID → 标题
```

构建方式：`build_link_resolver(plan_set)` 遍历所有页面计划，建立映射表。

---

## 调度器（Scheduler）

`PageGenerationScheduler`（定义在 `generate:scheduler` 中）是生成阶段的核心编排器：

### 工作模式

1. 创建 Worker 线程池，每线程关联一个独立的 `kota::event_loop`
2. 按 `generation_order` 提交页面生成任务
3. 每个任务包含：
   - 构建证据包（可能依赖先前的分析结果）
   - 调用 LLM 分析（串行化，受速率限制）
   - 渲染 Markdown 页面
4. 支持任务依赖：一个页面的生成可能依赖其他页面的摘要
5. 速率限制：通过 `llm::RateLimiter` 控制 LLM API 调用频率

### 依赖管理

页面间的依赖通过 `depends_on_pages` 字段表达。调度器确保：

- 先处理无依赖的页面
- 被依赖的页面完成后，再处理依赖它的页面
- 关联页面的摘要信息在构建证据包时可用

---

## 干运行模式（Dry Run）

`generate:dryrun` 模块提供在不实际调用 LLM 的情况下预览生成结果：

```
generate_dry_run(config, model)
  └─▶ build_page_plan_set(config, model) → 规划页面
  └─▶ build_dry_run_page_summary_texts() → 生成摘要文本
  └─▶ build_request_estimate_page()       → 估计 API 调用次数
  └─▶ build_llms_page()                   → 生成 LLM 调用清单
```

干运行模式输出：
- 页面计划概览
- 每个页面需要的 LLM 分析次数
- 预估的 token 消耗
- LLM 调用详细清单（用于审查）

---

## 关键设计决策

### 为什么页面计划需要拓扑排序？

页面生成可能依赖其他页面的分析结果（如命名空间页面引用其子页面的摘要）。拓扑排序确保页面按依赖顺序处理，使得被依赖页面的结果在依赖页面前就绪。

### 为什么证据包包含局部和反向上下文？

- **局部上下文**（`local_context`）：帮助 LLM 理解符号的"邻居"，避免孤立分析
- **依赖上下文**（`dependency_context`）：提供被调用/继承者的信息，帮助理解符号的功能依赖
- **反向使用上下文**（`reverse_usage_context`）：提供调用者/派生者的信息，帮助理解符号的使用模式

这种三层的上下文设计使 LLM 能够生成更准确、更具上下文化的文档。

### 为什么使用 Mermaid 图表？

Mermaid`.js` 是一种基于文本的图表语法，可以嵌入 Markdown 文档，被 `GitHub`、`GitLab` 等主流平台原生支持。这使得 Clore 生成的文档具有即时可视化的能力，无需额外的渲染工具。

