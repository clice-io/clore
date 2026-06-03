---
title: 'Clore 架构概览'
description: 'Guide: Clore 架构概览'
layout: doc
template: doc
---

# Clore 架构概览

## 项目概述

**Clore** 是一个 AI 增强的 C++ 代码文档自动生成工具。它通过 Clang 工具链提取 C++ 项目的结构化信息（符号、类型、命名空间、继承关系等），然后利用大型语言模型（LLM）为代码生成高质量的 Markdown 格式文档。该项目完全采用 **C++20 模块**（Modules）构建，使用基于 `kota` 事件循环的异步编程模型。

> 核心思想：**先提取，后生成**——先通过静态分析建立完整的项目模型，再基于该模型调用 LLM 生成文档内容。

---

## 顶层架构（模块划分）

Clore 由六个核心命名空间 / 模块域组成：

```
┌─────────────────────────────────────────────────────┐
│                     clore                            │
│  ┌─────────┐  ┌──────────┐  ┌────────────────┐      │
│  │  config  │  │ extract  │  │   generate     │      │
│  └────┬────┘  └────┬─────┘  └───────┬────────┘      │
│       │             │                │               │
│  ┌────▼────┐  ┌────▼─────┐  ┌───────▼────────┐      │
│  │  agent  │  │   net    │  │   support      │      │
│  └─────────┘  └──────────┘  └────────────────┘      │
└─────────────────────────────────────────────────────┘
```

### 1. `clore::config` — 配置管理
- **模块**：`config`, `config:load`, `config:schema`, `config:normalize`, `config:validate`
- **职责**：加载并解析 TOML 格式的配置文件，规范化路径，验证配置有效性
- **核心类型**：`TaskConfig`、`LLMConfig`、`FilterRule`
- **关键函数**：`load_config()`、`normalize()`、`validate()`

### 2. `clore::extract` — 代码提取引擎
- **模块**：`extract`, `extract:ast`, `extract:scan`, `extract:compiler`, `extract:model`, `extract:filter`, `extract:merge`, `extract:cache`
- **职责**：加载编译数据库（`compile_commands.json`），扫描依赖图，通过 Clang 提取 AST 符号信息，构建项目模型
- **核心类型**：`ProjectModel`、`SymbolInfo`、`SymbolID`、`SymbolKind`、`FileInfo`、`NamespaceInfo`、`ModuleUnit`
- **关键函数**：`extract_project_async()`、`load_compdb()`、`extract_symbols()`、`build_dependency_graph_async()`

### 3. `clore::generate` — 文档生成引擎
- **模块**：`generate`, `generate:planner`, `generate:analysis`, `generate:evidence`, `generate:evidence_builder`, `generate:scheduler`, `generate:markdown`, `generate:render/*`, `generate:cache`, `generate:dryrun`, `generate:model`
- **职责**：规划页面结构，构建 LLM prompt 证据包，调用 AI 分析代码，渲染 Markdown 页面，写入文件系统
- **核心类型**：`PagePlan`、`PagePlanSet`、`PageType`、`EvidencePack`、`LinkResolver`、`SymbolAnalysisStore`、`GeneratedPage`
- **关键函数**：`build_page_plan_set()`、`generate_pages_async()`、`build_evidence_for_*()`、`render_page_markdown()`

### 4. `clore::net` — 网络 / LLM 客户端
- **模块**：`network`, `network:http`, `network:protocol`, `network:schema`, `network:client`, `network:anthropic`, `network:openai`
- **职责**：封装 LLM API 调用（支持 Anthropic Claude 和 `OpenAI`），处理速率限制、协议适配、结构化输出、工具调用
- **核心类型**：`CompletionRequest`、`CompletionResponse`、`PromptRequest`、`ProbedCapabilities`、`LLMError`
- **关键函数**：`call_llm_async()`、`call_completion_async()`、`detect_provider_from_environment()`

### 5. `clore::agent` — 代理模式
- **模块**：`agent`, `agent:tools`
- **职责**：提供交互式 AI 代理模式，支持工具调用（tool calling），可迭代式文档生成
- **核心类型**：`AgentError`、`ToolError`
- **关键函数**：`run_agent_async()`、`dispatch_tool_call()`、`build_tool_definitions()`

### 6. `clore::support` — 基础设施
- **模块**：`support`（对应 `src/support/logging.cppm`）
- **职责**：日志记录、字符串哈希、工具函数等

---

## 核心数据流

以下是 clore 的完整数据处理管道：

```
┌──────────┐   ┌──────────────┐   ┌───────────────┐   ┌────────────┐   ┌──────────┐
│   TOML   │   │ compile_     │   │   Clang AST   │   │    LLM     │   │ Markdown │
│  Config  │──▶│ commands.json│──▶│   Extraction  │──▶│  Analysis  │──▶│  Pages   │
└──────────┘   └──────────────┘   └───────┬───────┘   └──────┬─────┘   └──────────┘
                                          │                   │
                                          ▼                   ▼
                                   ┌──────────────┐   ┌──────────────┐
                                   │ ProjectModel │   │  Generated   │
                                   │  (符号、文件、 │   │    Pages     │
                                   │   命名空间)   │   │  (title,     │
                                   └──────────────┘   │   path,      │
                                                      │   content)   │
                                                      └──────────────┘
```

### 阶段 1：配置加载

```
main()
  └─▶ Options 解析（命令行参数）
  └─▶ config::load_config(config_path)     → TaskConfig
  └─▶ config::normalize(task_config)        → 规范化路径
  └─▶ config::validate(task_config)         → 验证配置
  └─▶ net::validate_llm_provider_environment() → 验证环境
```

### 阶段 2：代码提取

```
extract::extract_project_async(config, loop) → kota::task<ProjectModel>
  ├─▶ load_compdb(config.compile_commands_path)             → 加载编译数据库
  ├─▶ 根据 filter 规则过滤编译条目
  ├─▶ 检查缓存（scan/ast 缓存命中优化）
  ├─▶ build_dependency_graph_async(...)                     → 扫描依赖图
  ├─▶ 对每个文件并行提取 AST（extract_symbols per file）
  ├─▶ 合并符号信息、建立关系图（继承、调用、引用）
  ├─▶ rebuild_model_indexes()                              → 建立查找索引
  ├─▶ build_module_info()                                  → 提取模块信息
  ├─▶ rebuild_lookup_maps()                                → 建立名称查找映射
  └─▶ resolve_source_snippet()                             → 解析源码片段
```

### 阶段 3：文档生成

```
generate::generate_pages_async(config, model, llm_model, rate_limit, output_root, loop) → ...
  ├─▶ build_page_plan_set(config, model)                   → PagePlanSet
  │     ├─▶ 枚举模块/文件页面
  │     ├─▶ 枚举命名空间页面
  │     ├─▶ 枚举索引页面
  │     ├─▶ 验证路径冲突
  │     └─▶ 拓扑排序（按依赖关系排序）
  │
  ├─▶ PageGenerationScheduler 执行页面生成
  │     ├─▶ 对每个页面计划：
  │     │     ├─▶ 构建证据包（EvidencePack）
  │     │     │     ├─▶ 目标符号事实（SymbolFact）
  │     │     │     ├─▶ 本地上下文符号
  │     │     │     ├─▶ 依赖上下文符号
  │     │     │     └─▶ 反向使用上下文
  │     │     ├─▶ 调用 LLM 分析（build_symbol_analysis_prompt）
  │     │     ├─▶ 解析 LLM 响应（parse_markdown_prompt_output）
  │     │     └─▶ 缓存分析结果
  │     │
  │     └─▶ 渲染页面
  │           ├─▶ render_page_markdown()                   → MarkdownDocument
  │           ├─▶ render_markdown()                         → 字符串
  │           └─▶ write_page()                              → 写入文件
  │
  └─▶ write_pages() → 写入所有生成页面
```

### 阶段 4：代理模式（可选）

```
agent::run_agent_async(config, model, llm_model, output_root, loop)
  └─▶ run_agent_loop()  → 交互式工具调用循环
        ├─▶ build_tool_definitions()  → 定义可用工具
        ├─▶ dispatch_tool_call()      → 分派工具调用
        └─▶ 迭代式改进文档
```

---

## 关键数据结构

### `extract::ProjectModel`
项目模型的中心数据载体，容纳所有提取到的代码信息。

| 字段 | 类型 | 用途 |
|------|------|------|
| `symbols` | `map<SymbolID, SymbolInfo>` | 所有符号（类、函数、变量等） |
| `files` | `map<string, FileInfo>` | 源文件信息，包含包含关系 |
| `namespaces` | `map<string, NamespaceInfo>` | 命名空间及其子符号 |
| `file_order` | `vector<string>` | 拓扑排序后的文件顺序 |
| `modules` | `map<string, ModuleUnit>` | C++20 模块单元信息 |
| `symbol_ids_by_qualified_name` | `map<string, vector<SymbolID>>` | 全限定名 → 符号 ID 映射（支持重载） |
| `module_name_to_sources` | `map<string, vector<string>>` | 模块名 → 源文件映射 |
| `uses_modules` | `bool` | 项目是否使用 C++20 模块 |

### `extract::SymbolInfo`
单个符号的完整信息。

| 字段 | 用途 |
|------|------|
| `id`, `kind`, `name`, `qualified_name` | 符号标识与分类 |
| `declaration_location`, `definition_location` | 源码位置 |
| `doc_comment`, `source_snippet` | 文档注释和源码片段 |
| `parent`, `children` | 父子层次结构 |
| `bases`, `derived` | 继承关系 |
| `calls`, `called_by` | 调用关系 |
| `references`, `referenced_by` | 引用关系 |
| `is_template`, `template_params` | 模板信息 |

### `generate::PagePlan`
单个页面的生成计划。

| 字段 | 用途 |
|------|------|
| `page_id`, `page_type` | 页面唯一标识和类型（File/Module/Namespace/Index） |
| `title`, `relative_path` | 页面标题和输出路径 |
| `owner_keys` | 归属的符号/页面键 |
| `depends_on_pages` | 依赖的页面 ID（用于拓扑排序） |
| `linked_pages` | 关联页面（交叉引用） |
| `prompt_requests` | 需要 LLM 分析的请求列表 |

### `generate::EvidencePack`
发送给 LLM 的上下文证据包。

| 字段 | 用途 |
|------|------|
| `page_id`, `prompt_kind` | 来源页面和 prompt 类型 |
| `subject_name`, `subject_kind` | 分析主题 |
| `target_facts` | 主体符号的事实列表 |
| `local_context` | 同一作用域内的上下文符号 |
| `dependency_context` | 依赖的符号（被调用/继承的） |
| `reverse_usage_context` | 使用者符号（调用者/派生类） |
| `related_page_summaries` | 关联页面的摘要 |
| `source_snippets` | 源码片段 |

---

## 异步架构

Clore 使用 `kota` 库作为异步运行时，这是一个基于协程的事件循环库：

- **`kota::event_loop`** — 事件循环，驱动所有异步任务
- **`kota::task<T, E>`** — 返回 `T` 或错误 `E` 的协程任务
- **`co_await`** — 协程等待语法
- 支持任务取消（`.catch_cancel()`）和超时

主事件循环在 `main()` 中创建，所有提取和生成操作作为协程任务调度到同一个循环上执行。

---

## 缓存策略

Clore 实现了多层缓存机制以提升重复执行的效率：

1. **编译签名缓存** — 根据编译参数生成签名，判断是否需要重新提取
2. **源码哈希缓存** — 检测文件是否修改
3. **扫描缓存（`ScanCache`）** — 缓存依赖图扫描结果，避免重复文件扫描
4. **AST 缓存** — 缓存 Clang AST 提取结果，加速符号提取
5. **分析缓存** — 缓存 LLM 分析结果（`generate:cache` 模块）
6. **Clice 缓存** — 缓存 Clang 编译实例状态

---

## 多 Provider 支持

`clore::net` 层通过协议抽象支持多个 LLM 提供商：

```
clore::net::call_llm_async()
  └─▶ detect_provider_from_environment()
        ├─▶ ANTHROPIC_API_KEY → clore::net::anthropic
        │     └─▶ anthropic::call_completion_async()
        │           └─▶ HTTP POST → api.anthropic.com
        │
        └─▶ OPENAI_API_KEY → clore::net::openai
              └─▶ openai::call_completion_async()
                    └─▶ HTTP POST → api.openai.com
```

提供商通过环境变量自动探测，上层 API 完全透明。

