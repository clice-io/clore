---
title: 'Namespace clore::generate'
description: 'clore::generate 命名空间是代码文档生成管线的核心实现域，封装了从符号分析数据到最终 Markdown 文档页面的完整转换过程。它定义了页面规划（如 PagePlanSet、PagePlan）、渲染（如 render_page_markdown、render_page_bundle）以及证据组装（如 build_evidence_for_module_summary、build_evidence_for_function_analysis）等关键函数。命名空间还提供了多种结构化类型，如 MarkdownNode、Paragraph、CodeFence 和 BulletList，用于以树状方式构建文档内容；LinkResolver 则负责将实体名称解析为输出目录中的相对路径，确保交叉引用正确。'
layout: doc
template: doc
---

# Namespace `clore::generate`

## Summary

`clore::generate` 命名空间是代码文档生成管线的核心实现域，封装了从符号分析数据到最终 Markdown 文档页面的完整转换过程。它定义了页面规划（如 `PagePlanSet`、`PagePlan`）、渲染（如 `render_page_markdown`、`render_page_bundle`）以及证据组装（如 `build_evidence_for_module_summary`、`build_evidence_for_function_analysis`）等关键函数。命名空间还提供了多种结构化类型，如 `MarkdownNode`、`Paragraph`、`CodeFence` 和 `BulletList`，用于以树状方式构建文档内容；`LinkResolver` 则负责将实体名称解析为输出目录中的相对路径，确保交叉引用正确。

在架构上，`clore::generate` 承担着编排角色：它协调多个子流程——包括页面顶层结构构建（如 `build_page_root`、`build_file_page_root`）、LLM 提示构建（如 `build_prompt`、`build_symbol_analysis_prompt`）、格式化输出（如 `format_evidence_text`）以及最终的页面写入（如 `write_pages`）。通过 `PageType`、`SemanticKind` 和 `PromptKind` 等枚举，命名空间对页面类型、语义种类和提示种类进行了系统分类，使生成管线能够根据符号类别和文档需求灵活选择合适的处理策略。整体而言，该命名空间提供了一个模块化且可扩展的框架，供上层调用者（如 `generate_pages`）驱动文档生成任务。

## Diagram

```mermaid
graph TD
    NS["generate"]
    T0["SymbolSection"]
    NS --> T0
    T1["TextSection"]
    NS --> T1
    T2["BlockQuote"]
    NS --> T2
    T3["BulletList"]
    NS --> T3
    T4["CodeFence"]
    NS --> T4
    T5["CodeFragment"]
    NS --> T5
    T6["EvidencePack"]
    NS --> T6
    T7["Frontmatter"]
    NS --> T7
    T8["FunctionAnalysis"]
    NS --> T8
    T9["GenerateError"]
    NS --> T9
    T10["GeneratedPage"]
    NS --> T10
    T11["GenerationSummary"]
    NS --> T11
    T12["LinkFragment"]
    NS --> T12
    T13["LinkResolver"]
    NS --> T13
    T14["LinkTarget"]
    NS --> T14
    T15["ListItem"]
    NS --> T15
    T16["MarkdownDocument"]
    NS --> T16
    T17["MarkdownFragmentResponse"]
    NS --> T17
    T18["MarkdownNode"]
    NS --> T18
    T19["MermaidDiagram"]
    NS --> T19
    T20["PageDocLayout"]
    NS --> T20
    T21["PageIdentity"]
    NS --> T21
    T22["PagePlan"]
    NS --> T22
    T23["PagePlanSet"]
    NS --> T23
    T24["PageType"]
    NS --> T24
    T25["Paragraph"]
    NS --> T25
    T26["PathError"]
    NS --> T26
    T27["PlanError"]
    NS --> T27
    T28["PromptError"]
    NS --> T28
    T29["PromptKind"]
    NS --> T29
    T30["PromptRequest"]
    NS --> T30
    T31["RawMarkdown"]
    NS --> T31
    T32["RenderError"]
    NS --> T32
    T33["SemanticKind"]
    NS --> T33
    T34["SemanticSection"]
    NS --> T34
    T35["SymbolAnalysisStore"]
    NS --> T35
    T36["SymbolDocPlan"]
    NS --> T36
    T37["SymbolDocView"]
    NS --> T37
    T38["SymbolFact"]
    NS --> T38
    T39["SymbolTargetKeyView"]
    NS --> T39
    T40["TextFragment"]
    NS --> T40
    T41["TypeAnalysis"]
    NS --> T41
    T42["VariableAnalysis"]
    NS --> T42
    NSC0["__detail"]
    NS --> NSC0
    NSC1["cache"]
    NS --> NSC1
```

## Subnamespaces

- [`clore::generate::cache`](cache/index.md)

## Types

### `clore::generate::BlockQuote`

Declaration: `generate/markdown.cppm:62`

Definition: `generate/markdown.cppm:62`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

`clore::generate::BlockQuote` 表示 Markdown 文档中的块引用（blockquote）元素。在文档生成过程中，该结构体用于封装引用内容，例如引述文本或嵌套的其他块级元素（如段落、列表、代码块等）。它通常作为 `clore::generate::MarkdownNode` 层次结构的一部分，与其他结构体（如 `Paragraph`、`BulletList`、`CodeFence`）共同构成 Markdown 文档的语义化表示。

#### Invariants

- `fragments` 中的元素顺序表示内联内容的逻辑顺序。

#### Key Members

- `fragments`

#### Usage Patterns

- 作为文档生成中块引用的表示被其他生成代码构造和填充。
- 其 `fragments` 成员被遍历以生成对应的输出格式。

### `clore::generate::BulletList`

Declaration: `generate/markdown.cppm:49`

Definition: `generate/markdown.cppm:49`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

`clore::generate::BulletList` 表示一个 Markdown 项目符号列表，是文档生成过程中的一种结构化节点。它通常包含多个 `clore::generate::ListItem` 实例，用于呈现无序列表内容。该类型在构造 `clore::generate::MarkdownDocument` 树时使用，作为 `Paragraph`、`CodeFence` 等节点的同级元素，以支持生成包含列表格式的文档片段。

#### Invariants

- `items` 存储所有的列表项，顺序与显示顺序一致。
- `items` 可以包含任意数量的 `ListItem`，包括零个。

#### Key Members

- `items`：保存所有子弹列表项的向量。

#### Usage Patterns

- 通过直接初始化或聚合初始化创建 `BulletList` 实例并填充 `items`。
- 将 `BulletList` 对象传递给 Markdown 生成函数以渲染为无序列表。

### `clore::generate::CodeFence`

Declaration: `generate/markdown.cppm:53`

Definition: `generate/markdown.cppm:53`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `language` should be a valid language identifier string.
- `code` may be empty or contain any text.

#### Key Members

- `language`: language identifier for the code block.
- `code`: the code content.

#### Usage Patterns

- Used as a data holder for code fences in Markdown generation.
- Likely constructed and then serialized into Markdown output.

### `clore::generate::CodeFragment`

Declaration: `generate/markdown.cppm:29`

Definition: `generate/markdown.cppm:29`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Key Members

- `code` field

#### Usage Patterns

- Used to represent a fragment of generated code as a string

### `clore::generate::EvidencePack`

Declaration: `generate/evidence.cppm:22`

Definition: `generate/evidence.cppm:22`

Implementation: [`Module generate:evidence`](../../../modules/generate/evidence.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- All fields are intended to be fully populated before use
- `page_id` and `prompt_kind` uniquely identify the generation task
- `subject_name` and `subject_kind` describe the target symbol
- Vectors may be empty if no relevant evidence exists

#### Key Members

- `subject_name` and `subject_kind`
- `target_facts`
- `local_context`, `dependency_context`, `reverse_usage_context`
- `page_id` and `prompt_kind`

#### Usage Patterns

- Constructed by evidence gathering subsystems
- Passed to prompt builders or directly to LLM-based documenters
- Consumed by documentation generation pipelines for a single symbol

### `clore::generate::Frontmatter`

Declaration: `generate/markdown.cppm:18`

Definition: `generate/markdown.cppm:18`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `layout` defaults to `"doc"`
- `page_template` defaults to `"doc"`
- All fields are `std::string` and can be set to any value

#### Key Members

- `title`
- `description`
- `layout`
- `page_template`

#### Usage Patterns

- Constructed with default values for typical documentation pages
- Fields are assigned individually before serialization
- Consumed by frontmatter generation code in the `clore::generate` namespace

### `clore::generate::FunctionAnalysis`

Declaration: `generate/model.cppm:81`

Definition: `generate/model.cppm:81`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `has_side_effects` should be true if and only if `side_effects` is non-empty; no explicit enforcement is shown
- Fields are independent but intended to be consistent with analysis data

#### Key Members

- `overview_markdown`
- `details_markdown`
- `has_side_effects`
- `side_effects`
- `reads_from`
- `writes_to`
- `usage_patterns`

#### Usage Patterns

- Cached and reused across namespace, module, file, and symbol documentation pages
- Populated by analysis code and consumed by documentation generation

### `clore::generate::GenerateError`

Declaration: `generate/model.cppm:69`

Definition: `generate/model.cppm:69`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- no explicit invariants defined in evidence

#### Key Members

- `message`

#### Usage Patterns

- likely used as an exception type or error result in generation functions
- typically constructed with a descriptive string

### `clore::generate::GeneratedPage`

Declaration: `generate/model.cppm:55`

Definition: `generate/model.cppm:55`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- 所有字段默认初始化为空字符串
- 字段内容由外部填充，无内部一致性约束

#### Key Members

- `title` 字段：页面标题
- `relative_path` 字段：页面相对路径
- `content` 字段：页面内容

#### Usage Patterns

- 作为生成流程的输出数据单元
- 被其他模块填充后传递或存储
- 直接访问其字段以获取页面信息

### `clore::generate::GenerationSummary`

Declaration: `generate/model.cppm:61`

Definition: `generate/model.cppm:61`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- 每个计数器都是从 0 开始，只能递增
- 所有计数器均为非负整数

#### Key Members

- `written_output_count`
- `symbol_analysis_cache_hits`
- `symbol_analysis_cache_misses`
- `page_prompt_cache_hits`
- `page_prompt_cache_misses`

#### Usage Patterns

- 在代码生成过程中递增相应计数器
- 用于后续性能分析或调试日志
- 作为生成结果的元数据返回

### `clore::generate::LinkFragment`

Declaration: `generate/markdown.cppm:33`

Definition: `generate/markdown.cppm:33`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- No documented invariants

#### Key Members

- `label`
- `target`
- `code_style`

#### Usage Patterns

- No documented usage patterns

### `clore::generate::LinkResolver`

Declaration: `generate/model.cppm:174`

Definition: `generate/model.cppm:174`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

`clore::generate::LinkResolver` 是一个映射结构，负责将实体名称（如限定类型名称、命名空间名称、模块名称以及文件路径）解析到它们在输出目录中的页面相对路径。它主要用于在生成的 Markdown 文档中创建跨文件引用链接，确保链接目标能够准确指向对应实体的文档页面。

在文档生成过程中，当需要插入指向其他实体（如类型、命名空间或模块）的交叉引用时，通过查询 `LinkResolver` 获取目标实体的相对路径，从而构造出正确的 Markdown 链接。该结构不关心单个成员的实现细节，而是作为整个链接解析机制的核心数据索引，为后续的链接生成步骤提供支持。

#### Invariants

- Maps are populated before resolution and remain read-only during use
- Each key maps to at most one value
- A null pointer indicates an unresolvable name

#### Key Members

- `name_to_path`
- `namespace_to_path`
- `module_to_path`
- `page_id_to_title`
- `resolve`
- `resolve_namespace`
- `resolve_module`
- `resolve_page_title`

#### Usage Patterns

- Used by link generation to find relative paths for entity references
- Called during markdown output to construct cross-references
- Provides separate maps for different name categories

#### Member Functions

##### `clore::generate::LinkResolver::resolve`

Declaration: `generate/model.cppm:180`

Definition: `generate/model.cppm:180`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
auto (const int &) const -> int;
```

##### `clore::generate::LinkResolver::resolve_module`

Declaration: `generate/model.cppm:190`

Definition: `generate/model.cppm:190`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
auto (const int &) const -> int;
```

##### `clore::generate::LinkResolver::resolve_namespace`

Declaration: `generate/model.cppm:185`

Definition: `generate/model.cppm:185`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
auto (const int &) const -> int;
```

##### `clore::generate::LinkResolver::resolve_page_title`

Declaration: `generate/model.cppm:195`

Definition: `generate/model.cppm:195`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
auto (const int &) const -> int;
```

### `clore::generate::LinkTarget`

Declaration: `generate/render/common.cppm:11`

Definition: `generate/render/common.cppm:11`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `label` and `target` are expected to be valid strings
- `code_style` is a boolean flag, initialized to `false`

#### Key Members

- `label`
- `target`
- `code_style`

#### Usage Patterns

- Used to represent a link in code generation contexts
- Passed to functions that render hyperlinks with optional code styling

### `clore::generate::ListItem`

Declaration: `generate/markdown.cppm:45`

Definition: `generate/markdown.cppm:45`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `fragments` 存储了列表项的所有行内片段

#### Key Members

- `fragments`

#### Usage Patterns

- 被其他代码用于构建和表示 Markdown 列表的单个列表项

### `clore::generate::MarkdownDocument`

Declaration: `generate/markdown.cppm:94`

Definition: `generate/markdown.cppm:94`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

Insufficient evidence to summarize; provide more EVIDENCE.

### `clore::generate::MarkdownFragmentResponse`

Declaration: `generate/model.cppm:77`

Definition: `generate/model.cppm:77`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

Insufficient evidence to summarize; provide more EVIDENCE.

### `clore::generate::MarkdownNode`

Declaration: `generate/markdown.cppm:73`

Definition: `generate/markdown.cppm:73`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

Insufficient evidence to summarize; provide more EVIDENCE.

### `clore::generate::MermaidDiagram`

Declaration: `generate/markdown.cppm:58`

Definition: `generate/markdown.cppm:58`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Key Members

- `code`

#### Usage Patterns

- Instantiated to store a Mermaid diagram code string
- Accessed to retrieve or set the diagram source

### `clore::generate::PageDocLayout`

Declaration: `generate/render/symbol.cppm:19`

Definition: `generate/render/symbol.cppm:19`

Implementation: [`Module generate:symbol`](../../../modules/generate/symbol.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- 每个向量只包含对应类型符号的文档计划
- `index_paths` 键值对表示从符号名称到文件路径的映射

#### Key Members

- `type_docs`
- `variable_docs`
- `function_docs`
- `index_paths`

#### Usage Patterns

- 用于在文档生成过程中收集和组织符号文档计划
- 由渲染阶段消费以生成最终页面

### `clore::generate::PageIdentity`

Declaration: `generate/model.cppm:207`

Definition: `generate/model.cppm:207`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- Fields are default-initialized
- No constraints on string contents

#### Key Members

- `page_type`
- `normalized_owner_key`
- `qualified_name`
- `source_relative_path`

#### Usage Patterns

- Carried as part of page generation metadata
- Used to uniquely identify a page within the generation system

### `clore::generate::PagePlan`

Declaration: `generate/model.cppm:39`

Definition: `generate/model.cppm:39`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

证据不足，无法总结；请提供更多证据。

#### Invariants

- `page_id` 通常应唯一标识页面（默认空串可能非唯一）
- `depends_on_pages` 列表中的页面ID应存在且无循环依赖

#### Key Members

- `page_id`
- `page_type`
- `title`
- `relative_path`
- `owner_keys`
- `depends_on_pages`
- `linked_pages`
- `prompt_requests`

#### Usage Patterns

- 生成管线填充 `PagePlan` 并传递给页面生成器
- 依赖图分析读取 `depends_on_pages` 确定构建顺序
- 外部工具根据 `page_type` 和 `prompt_requests` 生成内容

### `clore::generate::PagePlanSet`

Declaration: `generate/model.cppm:50`

Definition: `generate/model.cppm:50`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- The size of `generation_order` is equal to the size of `plans` if each plan has a unique identifier.
- Each string in `generation_order` corresponds to a key or identifier for an element in `plans`.

#### Key Members

- `plans`
- `generation_order`

#### Usage Patterns

- Populated during generation setup with plan data and ordering information.
- Iterated over in generation algorithms using the `generation_order` to determine processing sequence.
- Accessed to retrieve specific plans by their order or identifier.

### `clore::generate::PageType`

Declaration: `generate/model.cppm:9`

Definition: `generate/model.cppm:9`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- 底层类型为 `std::uint8_t`，所有枚举值在 0 到 3 之间

#### Key Members

- `File`
- `Namespace`
- `Index`
- `Module`

#### Usage Patterns

- 用于指定生成的文档页面的种类
- 可能作为 `Page` 类或生成函数的参数或数据成员

#### Member Variables

##### `clore::generate::PageType::File`

Declaration: `generate/model.cppm:13`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
File
```

##### `clore::generate::PageType::Index`

Declaration: `generate/model.cppm:10`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
Index
```

##### `clore::generate::PageType::Module`

Declaration: `generate/model.cppm:11`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
Module
```

##### `clore::generate::PageType::Namespace`

Declaration: `generate/model.cppm:12`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
Namespace
```

### `clore::generate::Paragraph`

Declaration: `generate/markdown.cppm:41`

Definition: `generate/markdown.cppm:41`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- Fragments are stored in the order they appear in the paragraph.

#### Key Members

- `fragments`

#### Usage Patterns

- Constructed by populating `fragments` with inline elements.
- Iterated over to process or render the paragraph.

### `clore::generate::PathError`

Declaration: `generate/model.cppm:203`

Definition: `generate/model.cppm:203`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `message` 成员的值为任意可读字符串，无格式约束

#### Key Members

- `clore::generate::PathError::message`

#### Usage Patterns

- 作为路径生成失败时的返回类型或异常包装

### `clore::generate::PlanError`

Declaration: `generate/planner.cppm:11`

Definition: `generate/planner.cppm:11`

Implementation: [`Module generate:planner`](../../../modules/generate/planner.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `message` should contain a non-empty, meaningful error description when an error occurs

#### Key Members

- `message`

#### Usage Patterns

- Returned as an error type from planning functions
- Inspected by callers to extract error details

### `clore::generate::PromptError`

Declaration: `generate/evidence.cppm:90`

Definition: `generate/evidence.cppm:90`

Implementation: [`Module generate:evidence`](../../../modules/generate/evidence.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `message` 包含可读的错误描述

#### Key Members

- `message`

#### Usage Patterns

- 作为错误结果返回
- 用于传递生成阶段的错误信息

### `clore::generate::PromptKind`

Declaration: `generate/model.cppm:18`

Definition: `generate/model.cppm:18`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- All enumerators are distinct and each represents a specific prompt category.
- The underlying type is `std::uint8_t`, ensuring compact storage.

#### Key Members

- `NamespaceSummary`
- `ModuleSummary`
- `ModuleArchitecture`
- `IndexOverview`
- `FunctionAnalysis`
- `TypeAnalysis`
- `VariableAnalysis`
- `FunctionDeclarationSummary`
- `FunctionImplementationSummary`
- `TypeDeclarationSummary`
- `TypeImplementationSummary`

#### Usage Patterns

- Used to select or dispatch prompt generation logic based on the kind of code element.

#### Member Variables

##### `clore::generate::PromptKind::FunctionAnalysis`

Declaration: `generate/model.cppm:23`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
FunctionAnalysis
```

##### `clore::generate::PromptKind::FunctionDeclarationSummary`

Declaration: `generate/model.cppm:26`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
FunctionDeclarationSummary
```

##### `clore::generate::PromptKind::FunctionImplementationSummary`

Declaration: `generate/model.cppm:27`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
FunctionImplementationSummary
```

##### `clore::generate::PromptKind::IndexOverview`

Declaration: `generate/model.cppm:22`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
IndexOverview
```

##### `clore::generate::PromptKind::ModuleArchitecture`

Declaration: `generate/model.cppm:21`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
ModuleArchitecture
```

##### `clore::generate::PromptKind::ModuleSummary`

Declaration: `generate/model.cppm:20`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
ModuleSummary
```

##### `clore::generate::PromptKind::NamespaceSummary`

Declaration: `generate/model.cppm:19`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
NamespaceSummary
```

##### `clore::generate::PromptKind::TypeAnalysis`

Declaration: `generate/model.cppm:24`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
TypeAnalysis
```

##### `clore::generate::PromptKind::TypeDeclarationSummary`

Declaration: `generate/model.cppm:28`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
TypeDeclarationSummary
```

##### `clore::generate::PromptKind::TypeImplementationSummary`

Declaration: `generate/model.cppm:29`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
TypeImplementationSummary
```

##### `clore::generate::PromptKind::VariableAnalysis`

Declaration: `generate/model.cppm:25`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

###### Declaration

```cpp
VariableAnalysis
```

### `clore::generate::PromptRequest`

Declaration: `generate/model.cppm:34`

Definition: `generate/model.cppm:34`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `kind` 总是有效的 `PromptKind` 值
- `target_key` 可为空但非空字符串表示有效目标

#### Key Members

- `kind` 成员
- `target_key` 成员

#### Usage Patterns

- 通过值传递用于生成请求
- 在生成器内部解析 `kind` 和 `target_key`
- 默认构造为 `NamespaceSummary` 和空键

### `clore::generate::RawMarkdown`

Declaration: `generate/markdown.cppm:66`

Definition: `generate/markdown.cppm:66`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

`clore::generate::RawMarkdown` 表示一个原始 Markdown 内容块，通常作为文档生成管线中的中间表示或输入。它用于承载未经解析或预处理的 Markdown 文本，以便在后继阶段（例如渲染或拼接为完整文档）中进行进一步处理。该结构体与 `clore::generate::MarkdownDocument`、`clore::generate::MarkdownFragmentResponse` 等类型协同工作，共同构成从结构化事实到最终 Markdown 输出的转换流程。

#### Invariants

- `markdown` 包含任意字符串内容，通常为 Markdown 格式的文本。

#### Key Members

- `markdown`：存储原始 Markdown 字符串的成员变量。

#### Usage Patterns

- 作为数据传输对象，在生成过程中传递 Markdown 内容。
- 可能被其他模块读取用于写入文件或进一步处理。

### `clore::generate::RenderError`

Declaration: `generate/model.cppm:73`

Definition: `generate/model.cppm:73`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `message` may contain any string value

#### Key Members

- `std::string message`

#### Usage Patterns

- Returned by rendering functions to report errors
- Inspected by callers to obtain error details

### `clore::generate::SemanticKind`

Declaration: `generate/markdown.cppm:7`

Definition: `generate/markdown.cppm:7`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- Each enumerator corresponds to a unique semantic kind.
- The underlying type is `std::uint8_t`.

#### Key Members

- `Index`
- `Namespace`
- `Module`
- `Type`
- `Function`
- `Variable`
- `File`
- `Section`

#### Usage Patterns

- Used to specify the kind of a semantic symbol.
- Used to dispatch generation logic per kind.

#### Member Variables

##### `clore::generate::SemanticKind::File`

Declaration: `generate/markdown.cppm:14`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

###### Declaration

```cpp
File
```

##### `clore::generate::SemanticKind::Function`

Declaration: `generate/markdown.cppm:12`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

###### Declaration

```cpp
Function
```

##### `clore::generate::SemanticKind::Index`

Declaration: `generate/markdown.cppm:8`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

###### Declaration

```cpp
Index
```

##### `clore::generate::SemanticKind::Module`

Declaration: `generate/markdown.cppm:10`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

###### Declaration

```cpp
Module
```

##### `clore::generate::SemanticKind::Namespace`

Declaration: `generate/markdown.cppm:9`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

###### Declaration

```cpp
Namespace
```

##### `clore::generate::SemanticKind::Section`

Declaration: `generate/markdown.cppm:15`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

###### Declaration

```cpp
Section
```

##### `clore::generate::SemanticKind::Type`

Declaration: `generate/markdown.cppm:11`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

###### Declaration

```cpp
Type
```

##### `clore::generate::SemanticKind::Variable`

Declaration: `generate/markdown.cppm:13`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

###### Declaration

```cpp
Variable
```

### `clore::generate::SemanticSection`

Declaration: `generate/markdown.cppm:70`

Definition: `generate/markdown.cppm:84`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `level` 通常为正整数（默认2），表示标题级别
- 若 `omit_if_empty` 为 true 且 `children` 为空，则章节可能被省略

#### Key Members

- `SemanticKind kind`
- `std::string heading`
- `std::uint8_t level`
- `bool omit_if_empty`
- `bool code_style_heading`
- `std::vector<MarkdownNode> children`
- `std::string subject_key`

#### Usage Patterns

- 作为树节点通过 `children` 容器构建嵌套章节结构
- 在生成文档时根据 `kind` 和 `heading` 格式化输出
- 用于模块或命名空间等实体的语义注释生成

### `clore::generate::SymbolAnalysisStore`

Declaration: `generate/model.cppm:125`

Definition: `generate/model.cppm:125`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- The fields are initialized by default constructors of respective cache types
- All fields are public and can be accessed directly

#### Key Members

- `functions`
- `types`
- `variables`

#### Usage Patterns

- Used as a member in other structures or passed to analysis functions
- Accessed to retrieve or update analysis results for symbols

### `clore::generate::SymbolDocPlan`

Declaration: `generate/render/symbol.cppm:13`

Definition: `generate/render/symbol.cppm:13`

Implementation: [`Module generate:symbol`](../../../modules/generate/symbol.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `symbol` may be null if the plan is a placeholder or root container
- `children` may be empty indicating no nested symbols
- `index_path` is expected to be a valid relative path string

#### Key Members

- `symbol`
- `index_path`
- `children`

#### Usage Patterns

- Used to build a tree of documentation plans for symbols in a module
- `children` vector enables recursive traversal for generating nested documentation pages

### `clore::generate::SymbolDocView`

Declaration: `generate/render/common.cppm:17`

Definition: `generate/render/common.cppm:17`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Member Variables

##### `clore::generate::SymbolDocView::Declaration`

Declaration: `generate/render/common.cppm:18`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

###### Declaration

```cpp
Declaration
```

##### `clore::generate::SymbolDocView::Details`

Declaration: `generate/render/common.cppm:20`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

###### Declaration

```cpp
Details
```

##### `clore::generate::SymbolDocView::Implementation`

Declaration: `generate/render/common.cppm:19`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

###### Declaration

```cpp
Implementation
```

### `clore::generate::SymbolFact`

Declaration: `generate/evidence.cppm:9`

Definition: `generate/evidence.cppm:9`

Implementation: [`Module generate:evidence`](../../../modules/generate/evidence.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `id` is globally unique
- `qualified_name` is the fully qualified name
- `is_template` is `true` if the symbol is a template specialization

#### Key Members

- `id`
- `qualified_name`
- `kind_label`
- `doc_comment`
- `declaration_file`
- `declaration_line`
- `signature`
- `access`
- `is_template`
- `template_params`

#### Usage Patterns

- Populated by extraction phase
- Consumed by generation phase to produce documentation output

### `clore::generate::SymbolTargetKeyView`

Declaration: `generate/model.cppm:136`

Definition: `generate/model.cppm:136`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `qualified_name` and signature are non-owning string views
- `qualified_name` and signature must reference valid null-terminated strings for the lifetime of the view

#### Key Members

- `qualified_name`
- signature

#### Usage Patterns

- Used as a lightweight key to identify symbol targets
- Passed to functions that need to reference a symbol's identity without copying strings

### `clore::generate::TextFragment`

Declaration: `generate/markdown.cppm:25`

Definition: `generate/markdown.cppm:25`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `text` can be any valid `std::string`.
- No additional constraints beyond those of `std::string`.

#### Key Members

- `text`: the stored string content

#### Usage Patterns

- Used to pass or store a piece of text in the markdown generation pipeline.
- Likely aggregated into larger structures or sequences.

### `clore::generate::TypeAnalysis`

Declaration: `generate/model.cppm:91`

Definition: `generate/model.cppm:91`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- No invariants are enforced by the type.

#### Key Members

- `overview_markdown`
- `details_markdown`
- `invariants`
- `key_members`
- `usage_patterns`

#### Usage Patterns

- Used as a container for caching analysis results across documentation pages.
- Each field corresponds to a section of generated documentation, consumed by documentation templates.

### `clore::generate::VariableAnalysis`

Declaration: `generate/model.cppm:99`

Definition: `generate/model.cppm:99`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

Insufficient evidence to summarize; provide more EVIDENCE.

## Variables

### `clore::generate::add_prompt_output`

Declaration: `generate/render/common.cppm:142`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

Variable `clore::generate::add_prompt_output` declared at `generate/render/common.cppm:142`.

### `clore::generate::add_symbol_analysis_detail_sections`

Declaration: `generate/render/common.cppm:170`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

Variable `clore::generate::add_symbol_analysis_detail_sections` is declared at `generate/render/common.cppm:170` with type deduced as `auto`.

### `clore::generate::add_symbol_analysis_sections`

Declaration: `generate/render/common.cppm:176`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

`clore::generate::add_symbol_analysis_sections` is a public variable declared in `generate/render/common.cppm` at line 176 with type deduced via `auto`. Based on its name and surrounding context, it is likely a function or callable that processes symbol analysis sections during rendering.

### `clore::generate::add_symbol_doc_links`

Declaration: `generate/render/symbol.cppm:43`

Implementation: [`Module generate:symbol`](../../../modules/generate/symbol.md)

A callable variable declared in `generate/render/symbol.cppm` that adds documentation links to a symbol page.

#### Usage Patterns

- called in `render_symbol_page` to add documentation links

### `clore::generate::append_symbol_doc_pages`

Declaration: `generate/render/symbol.cppm:60`

Implementation: [`Module generate:symbol`](../../../modules/generate/symbol.md)

The variable `clore::generate::append_symbol_doc_pages` is declared at `generate/render/symbol.cppm:60` with `auto` type deduction, suggesting it is initialized from an expression whose type is deduced. Its role is not fully determined from the available evidence.

### `clore::generate::append_type_member_sections`

Declaration: `generate/render/symbol.cppm:49`

Implementation: [`Module generate:symbol`](../../../modules/generate/symbol.md)

A variable declared as `auto append_type_member_sections` in `clore::generate` namespace at `generate/render/symbol.cppm:49`.

### `clore::generate::push_link_paragraph`

Declaration: `generate/render/common.cppm:92`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

变量 `clore::generate::push_link_paragraph` 声明为 `auto`，但其初始化表达式和具体类型未在提供的证据中显示。

### `clore::generate::push_location_paragraph`

Declaration: `generate/render/common.cppm:399`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

Variable `clore::generate::push_location_paragraph` is a public callable (likely a function or lambda) that appends a formatted location paragraph for symbols.

#### Usage Patterns

- called from `clore::generate::build_symbol_source_locations`

### `clore::generate::push_optional_link_paragraph`

Declaration: `generate/render/common.cppm:111`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

变量 `clore::generate::push_optional_link_paragraph` 声明为 `auto`，用于控制可选链接段落的推送。

## Functions

### `clore::generate::analysis_details_markdown`

Declaration: `generate/model.cppm:157`

Definition: `generate/model.cppm:373`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

`clore::generate::analysis_details_markdown` 接受一个 `SymbolAnalysisStore` 和一个表示符号标识符的 `int`，并生成该符号详细分析结果的 Markdown 表示。调用者需要提供有效的存储引用和符号 ID；函数返回一个整数句柄，该句柄可以在后续文档构建操作中使用（例如传递给 `render_markdown` 或嵌入到更大的 Markdown 树中）。

#### Usage Patterns

- Used to obtain the details markdown for a symbol during page generation
- Called by page rendering functions to include detailed analysis content

### `clore::generate::analysis_markdown`

Declaration: `generate/model.cppm:342`

Definition: `generate/model.cppm:342`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

`clore::generate::analysis_markdown` 是生成符号分析结果 Markdown 表示的函数。它接受一个 `SymbolAnalysisStore` 的常量引用、一个标识目标符号的 `const int &`，以及一个 `FieldAccessor &&` 转发引用可调用对象，用于从分析存储中定制提取或转换所需数据。函数返回一个 `int`，表示生成的 Markdown 内容的句柄或标识符。

调用者有责任提供有效的符号标识符和匹配的 `FieldAccessor`，该访问器应能从给定的 `SymbolAnalysisStore` 中正确获取分析字段。该函数不修改传入的存储或符号标识符。

#### Usage Patterns

- Used to extract specific markdown fields from analysis data for symbol documentation generation.

### `clore::generate::analysis_overview_markdown`

Declaration: `generate/model.cppm:154`

Definition: `generate/model.cppm:366`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

函数 `clore::generate::analysis_overview_markdown` 接受一个 `const SymbolAnalysisStore &` 和一个 `const int &` 作为输入，返回一个 `int`。其目的是根据给定的符号分析存储以及一个标识具体分析目标的整数参数，生成该分析的概述性 Markdown 内容。

调用者必须确保传入的 `SymbolAnalysisStore` 包含有效的符号分析数据，且第二个参数所指示的符号标识符在存储中存在对应的分析记录。函数返回一个整数值，通常表示生成的 Markdown 文本的长度或操作的状态码，调用者应据此判断输出是否成功生成。

#### Usage Patterns

- Used to obtain the overview portion of a symbol's analysis for rendering.

### `clore::generate::analysis_prompt_kind_for_symbol`

Declaration: `generate/analysis.cppm:27`

Definition: `generate/analysis.cppm:286`

Implementation: [`Module generate:analysis`](../../../modules/generate/analysis.md)

函数 `clore::generate::analysis_prompt_kind_for_symbol` 接受一个表示符号的 `const int &` 参数，并返回一个 `int`，该值标识应对该符号使用的分析提示种类。调用者可将返回值与 `PromptKind` 枚举中的值进行比较，以决定该符号的分析应生成哪种类型的提示。该函数为每个输入符号提供一个确定的、可重复的结果，在符号标识符与分析提示分类之间建立明确的映射。

#### Usage Patterns

- Used to map a `SymbolInfo` to a `PromptKind` for analysis
- Called during analysis prompt generation to determine which prompt to use for a symbol

### `clore::generate::apply_symbol_analysis_response`

Declaration: `generate/analysis.cppm:39`

Definition: `generate/analysis.cppm:348`

Implementation: [`Module generate:analysis`](../../../modules/generate/analysis.md)

`clore::generate::apply_symbol_analysis_response` 接收一个由调用者提供的符号分析响应（以 `std::string_view` 形式传入），并将其应用到当前生成上下文中的指定目标上。函数会通过传入的可变引用修改相关状态，并返回一个整数以指示处理结果。调用者负责确保提供的响应格式与预期的分析协议一致，并在调用前正确设置所有上下文参数。

#### Usage Patterns

- Called after receiving a prompt response to persist parsed analysis data
- Used in the generation pipeline to update symbol analysis results

### `clore::generate::build_dry_run_page_summary_texts`

Declaration: `generate/dryrun.cppm:11`

Definition: `generate/dryrun.cppm:316`

Implementation: [`Module generate:dryrun`](../../../modules/generate/dryrun.md)

`clore::generate::build_dry_run_page_summary_texts` 负责在干运行模式下为指定页面构建摘要文本。调用者需提供两个整数参数（可能标识页面和目标位置），函数返回一个整数，通常表示操作结果的状态或生成的文本数量。该函数不修改外部状态，专注于文本生成合同。

#### Usage Patterns

- Used in dry run page generation to collect summary texts for each prompt request.
- Likely called by `generate_dry_run` or related functions.

### `clore::generate::build_evidence_for_function_analysis`

Declaration: `generate/evidence.cppm:40`

Definition: `generate/evidence_builder.cppm:53`

Implementation: [`Module generate:evidence_builder`](../../../modules/generate/index.md) | [`Module generate:evidence`](../../../modules/generate/evidence.md)

`clore::generate::build_evidence_for_function_analysis` 为函数分析阶段组装证据。调用方提供函数标识符、其外围作用域上下文以及（可能表示子页面或索引的）整数值；该函数返回一个整数，用于在后续的提示构建或页面生成过程中引用该证据包。调用方应确保提供的标识符在上下文中有效，且返回的键仅在本次生成会话中有效。

#### Usage Patterns

- Called during evidence construction for function analysis
- Possibly used in a builder pattern for generating evidence packs

### `clore::generate::build_evidence_for_function_declaration_summary`

Declaration: `generate/evidence.cppm:67`

Definition: `generate/evidence_builder.cppm:238`

Implementation: [`Module generate:evidence_builder`](../../../modules/generate/index.md) | [`Module generate:evidence`](../../../modules/generate/evidence.md)

此函数构建用于生成函数声明摘要的证据。调用者提供函数声明及其上下文的必要标识（例如函数声明 ID、分析与限制参数），并接收一个证据包，该包可被后续流程消费，以构造面向 AI 的提示或直接填充摘要模板。它是声明摘要生成管线的一部分，负责将原始符号信息浓缩为结构化、可重用的证据表示。

### `clore::generate::build_evidence_for_function_implementation_summary`

Declaration: `generate/evidence.cppm:72`

Definition: `generate/evidence_builder.cppm:268`

Implementation: [`Module generate:evidence_builder`](../../../modules/generate/index.md) | [`Module generate:evidence`](../../../modules/generate/evidence.md)

函数 `clore::generate::build_evidence_for_function_implementation_summary` 负责生成用于构建函数实现摘要的结构化证据数据。调用者需提供两个 `const int &` 类型参数和一个 `int` 类型参数，这些参数共同标识待摘要的函数实现及其关联的分析上下文。该函数利用输入构建证据包并返回一个 `int` 值，成功时返回零，否则返回非零错误码。

调用者应当确保传入的标识符有效（例如代表有效的符号分析存储或源位置），并检查返回值以确认证据构造完成。构造完成的证据后续可被其他生成函数消费，用于生成最终的函数实现摘要文本。

#### Usage Patterns

- called when generating page markdown for function implementation summaries
- used in conjunction with `build_evidence_for_function_analysis` and `render_page_markdown`

### `clore::generate::build_evidence_for_index_overview`

Declaration: `generate/evidence.cppm:64`

Definition: `generate/evidence_builder.cppm:204`

Implementation: [`Module generate:evidence_builder`](../../../modules/generate/index.md) | [`Module generate:evidence`](../../../modules/generate/evidence.md)

`clore::generate::build_evidence_for_index_overview` 生成用于索引概览页面的证据数据。调用者必须提供两个 `const int &` 参数，分别标识页面生成所需的上下文（例如，导航模型和源符号集合）。该函数返回一个 `int` 值，代表构造完成的证据集（或对应的错误状态），调用者可将该结果直接传递给下游的页面渲染或打包步骤。

#### Usage Patterns

- Called during page generation to produce evidence for the index overview.

### `clore::generate::build_evidence_for_module_architecture`

Declaration: `generate/evidence.cppm:58`

Definition: `generate/evidence_builder.cppm:173`

Implementation: [`Module generate:evidence_builder`](../../../modules/generate/index.md) | [`Module generate:evidence`](../../../modules/generate/evidence.md)

生成并返回用于描述给定模块架构的格式化证据文本。调用方需提供完整的模块标识符、关联文件标识符以及需要包含的证据深度层级。该函数负责整合模块的内部结构与对外关系，以结构化文本的形式构建证据，供后续的文档生成或分析管道使用。

#### Usage Patterns

- Called during page generation for module-level documentation
- Used to populate evidence for architecture summary sections
- Invoked by `clore::generate::build_page_root` or similar page builders

### `clore::generate::build_evidence_for_module_summary`

Declaration: `generate/evidence.cppm:52`

Definition: `generate/evidence_builder.cppm:142`

Implementation: [`Module generate:evidence_builder`](../../../modules/generate/index.md) | [`Module generate:evidence`](../../../modules/generate/evidence.md)

此函数为模块摘要页面构建证据包。调用者须提供模块标识符、相关的符号集合引用与分析上下文，以及一个用于选项或格式化的整数参数。返回一个不透明的句柄，用于后续的证据消费流程（例如排版为 Markdown）。调用者负责确保传入的标识符有效且引用数据在调用期间保持稳定；函数不拥有或延长参数生命周期。

#### Usage Patterns

- called when generating module documentation pages

### `clore::generate::build_evidence_for_namespace_summary`

Declaration: `generate/evidence.cppm:35`

Definition: `generate/evidence_builder.cppm:21`

Implementation: [`Module generate:evidence_builder`](../../../modules/generate/index.md) | [`Module generate:evidence`](../../../modules/generate/evidence.md)

调用方负责提供用于构建命名空间摘要页面所需的核心标识符：第一个参数表示目标命名空间的标识符，后续两个参数表示其所处模块或文件等上下文依赖的标识符，最后一个参数为整数选项（通常为位标志或配置值）。该函数基于这些输入构建并返回一个代表完整证据集合的整数句柄，该句柄可在下游的页面布局、提示构建或文档生成流程中使用。

调用方应保证所有标识符在当前的符号分析存储和页面规划集中有效，并确保传入的选项值符合预定义的枚举或常量约定。返回值仅在当前生成会话中有效，如果后续对底层分析数据进行了修改，则该句柄可能失效；调用方不应持久化该值或在多个生成会话间重用。

#### Usage Patterns

- called from page building functions for namespace summaries
- used in `clore::generate::build_namespace_page_root` or similar

### `clore::generate::build_evidence_for_type_analysis`

Declaration: `generate/evidence.cppm:44`

Definition: `generate/evidence_builder.cppm:82`

Implementation: [`Module generate:evidence_builder`](../../../modules/generate/index.md) | [`Module generate:evidence`](../../../modules/generate/evidence.md)

`clore::generate::build_evidence_for_type_analysis` 负责为类型分析阶段组装相关的证据数据。调用方需提供两个上下文标识符（以 `const int &` 形式传入）以及一个附加 `int` 参数，这些参数共同标识待分析的类型和目标位置。函数返回一个 `int` 值，该值是对应证据包的标识符（例如引用句柄或键），供后续分析流程（如 `build_symbol_analysis_prompt` 或 `analysis_markdown`）使用。调用方应确保传入的标识符有效且与当前分析会话中的符号与作用域一致，否则函数行为未定义。

#### Usage Patterns

- called to build evidence for type analysis pages
- used in documentation generation pipeline

### `clore::generate::build_evidence_for_type_declaration_summary`

Declaration: `generate/evidence.cppm:77`

Definition: `generate/evidence_builder.cppm:302`

Implementation: [`Module generate:evidence_builder`](../../../modules/generate/index.md) | [`Module generate:evidence`](../../../modules/generate/evidence.md)

`clore::generate::build_evidence_for_type_declaration_summary` 负责为给定类型的声明摘要构建所需的证据数据。调用者需提供该类型的源位置、相关的符号分析结果以及一个用于区分摘要类别的标志。该函数返回的证据包会被上游组件（如 `build_prompt`）消费，以生成类型声明页面的摘要部分或对应的 LLM 提示。其契约要求输入数据必须完整且对应同一个符号，否则行为未定义。

#### Usage Patterns

- called during `build_page_root` or `render_page_markdown` to provide evidence for type declaration pages
- used in conjunction with `build_evidence_for_type_analysis` and `build_evidence_for_type_implementation_summary`

### `clore::generate::build_evidence_for_type_implementation_summary`

Declaration: `generate/evidence.cppm:82`

Definition: `generate/evidence_builder.cppm:334`

Implementation: [`Module generate:evidence_builder`](../../../modules/generate/index.md) | [`Module generate:evidence`](../../../modules/generate/evidence.md)

`clore::generate::build_evidence_for_type_implementation_summary` 负责构建用于生成类型实现摘要页面所需的证据包。它接受以下参数：一个引用到符号分析存储的常量引用、一个表示目标类型标识符的常量引用，以及一个表示操作标志或枚举的整数。返回一个整型状态码，以指示是否成功生成证据。

调用者应确保所提供的分析存储包含目标类型的分析数据，并且标识符有效。函数的返回值可以用于检查调用是否成功，或在失败时进行错误处理。

#### Usage Patterns

- Called during page generation for type implementation summary pages
- Used by page planning functions like `build_page_plan_set`

### `clore::generate::build_evidence_for_variable_analysis`

Declaration: `generate/evidence.cppm:48`

Definition: `generate/evidence_builder.cppm:113`

Implementation: [`Module generate:evidence_builder`](../../../modules/generate/index.md) | [`Module generate:evidence`](../../../modules/generate/evidence.md)

负责为给定的变量分析构建证据数据。调用者需提供分析上下文和变量标识符等必要输入，函数将返回代表生成证据的整型值。该证据通常用于后续的提示构造或页面渲染流程，作为变量分析摘要的素材来源。返回值约定表示成功生成的证据标识，调用方应据此进一步处理。

#### Usage Patterns

- called to build evidence for variable analysis prompts
- used in symbol analysis pipeline for variables

### `clore::generate::build_file_page_root`

Declaration: `generate/render/page.cppm:345`

Definition: `generate/render/page.cppm:345`

Implementation: [`Module generate:page`](../../../modules/generate/page.md)

`clore::generate::build_file_page_root` 负责为某个文件页面构建根节点。调用者需要传入六个标识该文件及其所属上下文的整数参数（例如文件 ID、所属模块 ID、命名空间 ID 等），函数将根据这些参数生成并返回一个整数句柄，代表该文件页面的根结构。此句柄供后续页面渲染步骤使用，调用者在调用前应确保提供的参数足以唯一确定目标文件在文档生成管线中的位置。

#### Usage Patterns

- Called during file page generation to create the root section.
- Used as part of the page building pipeline for file documentation.

### `clore::generate::build_index_page_root`

Declaration: `generate/render/page.cppm:447`

Definition: `generate/render/page.cppm:447`

Implementation: [`Module generate:page`](../../../modules/generate/page.md)

函数 `clore::generate::build_index_page_root` 负责构建索引页面（通常为文档站点的主索引或目录页）的根内容。调用者需提供与索引页相关的全套上下文参数（例如页面计划、符号分析存储等），这些参数通过引用传入的整数句柄传递。该函数返回一个整数，表示成功构建的页面标识或生成状态码，调用者应确保在调用前相关上下文已正确初始化。

#### Usage Patterns

- called to generate the top-level index page of a documentation site
- produces the root `SemanticSection` for index page rendering

### `clore::generate::build_link_resolver`

Declaration: `generate/model.cppm:201`

Definition: `generate/model.cppm:471`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

函数 `clore::generate::build_link_resolver` 根据传入的 `PagePlanSet` 集合构造并返回一个 `LinkResolver` 对象。调用者应保证传入的集合完整、准确地描述了所有待生成的页面及其关系，以便链接解析器能够正确映射页面标题、命名空间、模块等目标。该函数是生成管线的核心工具，供后续页面渲染和交叉引用环节使用。

#### Usage Patterns

- Called during page generation setup to create a resolver for linking symbols to page paths
- Used to populate a `LinkResolver` that enables module, namespace, and generic name resolution

### `clore::generate::build_list_section`

Declaration: `generate/render/common.cppm:133`

Definition: `generate/render/common.cppm:133`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

函数 `clore::generate::build_list_section` 负责构造文档生成过程中使用的列表部分。调用者提供三个 `int` 参数以指定列表的结构或内容，并返回一个表示生成结果或状态的 `int` 值。其确切行为由传入的参数组合决定，但整体契约承诺生成一个适合嵌入到较大文档结构中的列表分段。

#### Usage Patterns

- Used within page generation pipelines to wrap a bullet list under a section heading

### `clore::generate::build_llms_page`

Declaration: `generate/dryrun.cppm:19`

Definition: `generate/dryrun.cppm:333`

Implementation: [`Module generate:dryrun`](../../../modules/generate/dryrun.md)

`clore::generate::build_llms_page` 负责构造一个面向大语言模型（LLM）交互的页面，用于代码生成流程中的干运行阶段。调用者需提供三个整数参数，这些参数标识了构建该页面所需的上下文信息（例如目标符号、文件或模块标识符）。函数返回一个整数，表示所构建页面的引用，调用者可使用该结果进行后续的页面渲染、缓存或写入操作。调用者应确保输入参数的有效性，并遵守前置条件（如对应页面类型在此流程中受支持、上下文标识符已正确解析等）。

#### Usage Patterns

- called during documentation generation to produce the `llms.txt` index page

### `clore::generate::build_module_page_root`

Declaration: `generate/render/page.cppm:255`

Definition: `generate/render/page.cppm:255`

Implementation: [`Module generate:page`](../../../modules/generate/page.md)

`clore::generate::build_module_page_root` 负责构造模块级文档页面的根结构。调用者提供七个整数参数，这些参数共同指定模块的上下文信息（如模块标识、分析数据、配置索引等），函数返回一个整数值代表该页面的根对象或处理结果。此函数与 `clore::generate::build_file_page_root`、`clore::generate::build_namespace_page_root` 等页面根构建函数处于同一层级，但专门针对模块页面，为后续渲染管道提供所需的基础框架。

#### Usage Patterns

- Called during documentation generation to produce the page content for a module
- Used by higher-level page rendering functions

### `clore::generate::build_namespace_page_root`

Declaration: `generate/render/page.cppm:165`

Definition: `generate/render/page.cppm:165`

Implementation: [`Module generate:page`](../../../modules/generate/page.md)

`clore::generate::build_namespace_page_root` 负责为给定的命名空间生成文档页面的根内容。它接受多个标识符参数，这些参数共同标识页面计划、命名空间目标以及相关的文档和符号分析上下文。该函数返回一个整数，指示操作的结果或生成的文档结构，供后续渲染步骤使用。

调用者应确保提供的参数引用有效的、已解析的页面和命名空间标识。该函数是页面生成流水线的一部分，与其他类似函数（如 `clore::generate::build_module_page_root`）协作，以一致的方式为不同作用域构建页面根内容。

#### Usage Patterns

- Used as the entry point for building the namespace page root.
- Called from higher-level page generation logic to compose the namespace documentation tree.

### `clore::generate::build_page_doc_layout`

Declaration: `generate/render/symbol.cppm:37`

Definition: `generate/render/symbol.cppm:897`

Implementation: [`Module generate:symbol`](../../../modules/generate/symbol.md)

`clore::generate::build_page_doc_layout` 接受两个 `const int &` 参数，通常代表符号标识符及其上下文，用于构造一个 `PageDocLayout` 布局。该布局描述了单个文档页面中所有符号文档内容的组织方式，后续可通过 `clore::generate::for_each_symbol_doc_group` 遍历其分组。调用方将返回的布局传递给下游渲染函数以生成最终页面内容。

#### Usage Patterns

- Called during page generation to build a layout of sub-document plans
- Used to organize symbol documentation by kind

### `clore::generate::build_page_plan_set`

Declaration: `generate/planner.cppm:15`

Definition: `generate/planner.cppm:369`

Implementation: [`Module generate:planner`](../../../modules/generate/planner.md)

`clore::generate::build_page_plan_set` 根据两个整数参数构造并返回一个页面计划集的状态码。调用者应在生成流程初始化时调用此函数，确保传入的参数在语义上与预期的页面标识符或索引一致。返回的整数指示计划集是否成功构建：零通常表示成功，非零值表示错误条件。此计划集随后可用于其他生成函数，例如 `build_link_resolver`。

#### Usage Patterns

- Called by page generation entry points
- Used to create page plan before rendering
- Part of documentation generation pipeline

### `clore::generate::build_page_root`

Declaration: `generate/render/page.cppm:546`

Definition: `generate/render/page.cppm:546`

Implementation: [`Module generate:page`](../../../modules/generate/page.md)

`clore::generate::build_page_root` 是一个公共函数，用于构建一个页面的根内容。它接受七个整数引用参数，这些参数共同标识需要生成根内容的页面上下文。调用者必须提供所有必要的标识符，并确保它们指向有效的、已解析的页面实体。该函数返回一个整数，表示构建操作的状态或结果代码。具体的参数语义和返回值含义由页面的生成管线定义，调用者应参考相关文档以了解合法的参数组合。

#### Usage Patterns

- Called by page generation routines to obtain the root `SemanticSectionPtr` for a given page plan.
- Used in the rendering pipeline to construct the top-level section of a page.

### `clore::generate::build_prompt`

Declaration: `generate/evidence.cppm:94`

Definition: `generate/evidence.cppm:651`

Implementation: [`Module generate:evidence`](../../../modules/generate/evidence.md)

函数 `clore::generate::build_prompt` 接受一个整型标识符和一个 `const EvidencePack &` 引用，构造并返回一个表示该提示的整数值。调用方负责提供有效的标识符以及包含完整相关证据的 `EvidencePack` —— 该返回的整数通常用于标识或索引所构建的提示，以便后续组件引用。该函数是提示构建流程中的一个核心步骤，其契约要求传入的证据包必须已正确填充，且标识符在调用上下文中具有明确含义。

#### Usage Patterns

- Called during prompt construction for different prompt kinds
- Used by higher-level prompt builders

### `clore::generate::build_prompt_section`

Declaration: `generate/render/common.cppm:124`

Definition: `generate/render/common.cppm:124`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

函数 `clore::generate::build_prompt_section` 构造 LLM 提示中的一个特定片段。它接受两个整数标识符（可能指定页面种类或符号上下文）和一个指向 `const int` 数组的指针（该数组提供该片段的证据数据），并返回一个 `int` 值，该值传达结果片段的大小或构造状态。调用者负责确保所提供指针指向具有有效内容的数组，并且两个整数标识符正确描述该提示段所属的父上下文。

#### Usage Patterns

- Building prompt sections with or without additional markdown content

### `clore::generate::build_related_page_targets`

Declaration: `generate/render/common.cppm:504`

Definition: `generate/render/common.cppm:504`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

`clore::generate::build_related_page_targets` 负责构造与指定页面相关联的目标页面集合，并返回生成的目标数量。调用方应提供两个 `const int&` 参数和一个 `int` 参数，以标识当前页面、关联上下文及可能的限制条件。该函数是页面导航和交叉引用生成流程的一部分，其结果可直接用于链接解析或页面计划构建。

#### Usage Patterns

- used in page rendering to generate navigation links to related pages

### `clore::generate::build_request_estimate_page`

Declaration: `generate/dryrun.cppm:15`

Definition: `generate/dryrun.cppm:230`

Implementation: [`Module generate:dryrun`](../../../modules/generate/dryrun.md)

函数 `clore::generate::build_request_estimate_page` 接受三个 `const int&` 参数，分别表示请求标识符、相关符号标识符以及估计上下文中的附加数据。调用者负责提供有效且一致的标识，以描述待估计的生成请求。该函数在干运行模式下为这些输入构建一个请求估计页面，并返回一个 `int` 作为估计结果（例如页面计数或状态码）。调用者应根据返回值判断估计是否成功，并确保在调用前参数所引用的对象保持有效。

#### Usage Patterns

- called during dry-run generation
- builds page for estimating prompt task load

### `clore::generate::build_string_list`

Declaration: `generate/render/common.cppm:148`

Definition: `generate/render/common.cppm:148`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

`clore::generate::build_string_list` 接受一个 `const int &` 参数并返回一个 `int`。它用于构建一个与参数标识的实体相关联的字符串列表。调用者必须确保传入的引用在文档生成流程中有效，且函数在正确的上下文中被调用。返回值表示构建结果的状态或句柄，具体含义由生成阶段的约定决定。

#### Usage Patterns

- building a bullet list of code-spanned text items
- helper for rendering markdown lists

### `clore::generate::build_symbol_analysis_prompt`

Declaration: `generate/analysis.cppm:46`

Definition: `generate/analysis.cppm:429`

Implementation: [`Module generate:analysis`](../../../modules/generate/analysis.md)

`clore::generate::build_symbol_analysis_prompt` 构建一个专门用于引导符号分析流程的提示。该函数接受多个标识参数（包括常量引用和值传递的整数），这些参数共同指定要分析的目标符号及其所处的评估上下文。调用者必须提供一致且有效的符号标识与可配置的选项；函数返回一个整数值，用于指示操作状态或生成的提示的唯一标识。该函数不直接参与证据收集或页面渲染，而是专注于产生后续分析步骤所需的提示结构。

#### Usage Patterns

- used to generate analysis prompts for symbols
- called by prompt generation pipeline
- dispatches based on prompt kind

### `clore::generate::build_symbol_link_list`

Declaration: `generate/render/common.cppm:360`

Definition: `generate/render/common.cppm:360`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

`clore::generate::build_symbol_link_list` 接受两个 `const int &` 引用类型参数、一个 `int` 值参数和一个 `bool` 标志参数，返回一个 `int` 值。调用者使用此函数从提供的符号信息中生成一个符号链接列表；返回值标识该列表，供后续渲染或引用使用。调用者负责确保所有引用参数指向有效数据，且布尔标志正确反映所需的链接生成行为。

#### Usage Patterns

- Called when generating page content that lists symbols with navigation links
- Used to produce symbol lists in documentation pages

### `clore::generate::build_symbol_source_locations`

Declaration: `generate/render/common.cppm:412`

Definition: `generate/render/common.cppm:412`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

函数 `clore::generate::build_symbol_source_locations` 为调用者构建一个符号的源代码位置表示。它接受一个符号标识符以及相关的上下文参数（分别对应三个 `const int &` 和一个 `int`），并返回一个 `int` 值，该值携带该符号在源代码中出现的所有位置信息。调用者可以通过此函数获得符号的源位置集合，以便在生成页面内容时渲染或链接到这些位置。该函数是符号页面构建流程的一部分，与其他 `build_*` 函数协作，共同生成文档中的证据片段或页面根节点。

#### Usage Patterns

- Used to generate source-location markdown for symbol documentation pages

### `clore::generate::code_spanned_fragments`

Declaration: `generate/markdown.cppm:124`

Definition: `generate/markdown.cppm:693`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

`clore::generate::code_spanned_fragments` 为调用者生成一组代码跨度片段，这些片段可用于后续的 Markdown 渲染步骤。它接受一个表示代码跨度上下文的 `int` 参数，并返回一个指示生成结果状态或片段数量的 `int` 值。此函数的设计使其适用于需要将代码范围分解为独立片段进行格式化或处理的场景。

#### Usage Patterns

- Used to extract code spans from a given string
- Delegates to `append_code_spanned_fragments`

### `clore::generate::code_spanned_markdown`

Declaration: `generate/markdown.cppm:126`

Definition: `generate/markdown.cppm:699`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

`clore::generate::code_spanned_markdown` 接受一个整数参数，并返回一个整数。调用者应提供一个标识符（如代码片段或节点句柄），用于指定需要生成代码跨度（code span）表示的内容。返回的整数代表生成的 markdown 片段，可被后续渲染或组合函数（如 `clore::generate::render_page_markdown` 或 `clore::generate::normalize_markdown_fragment`）使用。该函数是 markdown 生成管线的一部分，负责将原始代码内容封装为符合语法规范的代码跨度格式。

#### Usage Patterns

- convert plain markdown to code-spanned markdown

### `clore::generate::collect_implementation_symbols`

Declaration: `generate/render/common.cppm:314`

Definition: `generate/render/common.cppm:314`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

函数 `clore::generate::collect_implementation_symbols` 根据两个整数参数标识的上下文收集实现符号，并使用模板参数 `Predicate` 指定的可调用对象进行过滤。调用者必须提供符合 `Predicate &&` 签名的谓词，该谓词接收每个符号并返回一个可转换为 `bool` 的值，决定该符号是否被纳入收集结果。函数返回一个整数，表示收集到的符号数量或操作状态码。

#### Usage Patterns

- Called during page generation to gather symbols for documentation output
- Used by page-building functions like `build_page_root` and `collect_namespace_symbols`

### `clore::generate::collect_namespace_symbols`

Declaration: `generate/render/common.cppm:289`

Definition: `generate/render/common.cppm:289`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

`clore::generate::collect_namespace_symbols` 是一个模板函数，用于收集指定命名空间中满足给定条件的符号。它接受一个表示命名空间的 `const int &` 引用、一个 `int` 类型的上下文标识，以及一个 `Predicate &&` 可调用对象，返回一个 `int` 值，通常表示收集到的符号数量或状态。调用者必须确保提供的命名空间引用有效，且 `Predicate` 可调用对象接受合适的参数并返回可转换为 `bool` 的值，以控制哪些符号被纳入收集结果。

#### Usage Patterns

- Used to gather symbols for namespace page generation
- Called with `is_page_level_symbol` and custom predicates

### `clore::generate::compute_page_path`

Declaration: `generate/model.cppm:214`

Definition: `generate/model.cppm:576`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

根据提供的 `PageIdentity` 计算出该页面的输出路径。此函数是生成管线的一部分，调用者使用它来获取给定页面身份所对应的唯一路径标识。返回的整数表示操作的结果，通常为 0 表示成功，非零值指示错误。调用者负责确保传入的 `PageIdentity` 有效且已完成适当的验证。

#### Usage Patterns

- Used to determine output file path for a documentation page
- Called during page generation to map identities to filesystem locations

### `clore::generate::doc_label`

Declaration: `generate/render/common.cppm:279`

Definition: `generate/render/common.cppm:279`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

`clore::generate::doc_label` 接受一个 `SymbolDocView` 参数，返回一个用于标识该符号文档视图的整数标签。该标签在生成文档的过程中可用于引用或匹配对应的符号文档，从而支持文档内容的组织、索引和交叉链接。调用方应确保传入的视图有效，并理解返回的标签仅在当前生成上下文中具有意义。

#### Usage Patterns

- Used to obtain display labels for `SymbolDocView` values in documentation generation
- Likely called when rendering section headings or labels for symbol documentation views

### `clore::generate::escape_mermaid_label`

Declaration: `generate/render/diagram.cppm:13`

Definition: `generate/render/diagram.cppm:109`

Implementation: [`Module generate:diagram`](../../../modules/generate/diagram.md)

`clore::generate::escape_mermaid_label` 对 Mermaid 图中使用的标签进行转义，确保标签内容不会与 Mermaid 语法冲突。调用者传入一个代表待转义标签的整数标识符，函数返回一个对应的已转义标签标识符，该标识符可安全地用于后续 Mermaid 图生成操作（如 `make_mermaid`）。调用者无需关心转义的具体规则，只需保证传入的标识符有效即可。

#### Usage Patterns

- Used when rendering Mermaid diagram code to ensure labels do not break the diagram syntax.

### `clore::generate::find_declaration_page`

Declaration: `generate/render/common.cppm:473`

Definition: `generate/render/common.cppm:473`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

函数 `clore::generate::find_declaration_page` 接受两个 `const int &` 参数和一个 `int` 参数，并返回一个 `int`。调用者使用它来定位与给定上下文关联的声明页面的标识符。返回的 `int` 值可用于后续的页面渲染或链接解析操作。

#### Usage Patterns

- Used internally during page generation to locate a link to a symbol's declaration page.
- Often invoked when rendering pages that need to provide a cross-reference to the declaration of a symbol.

### `clore::generate::find_doc_index_path`

Declaration: `generate/render/symbol.cppm:40`

Definition: `generate/render/symbol.cppm:804`

Implementation: [`Module generate:symbol`](../../../modules/generate/symbol.md)

函数 `clore::generate::find_doc_index_path` 根据提供的 `PageDocLayout` 和一个整数参数，返回一个整数，表示文档索引页面的路径（例如，用于定位或标识生成文档中的索引页面）。调用方负责提供有效的布局和索引标识符，函数返回的整数可用于进一步的路由或页面生成流程。

#### Usage Patterns

- look up documentation index path by qualified name
- get pointer to path or nullptr if not found

### `clore::generate::find_function_analysis`

Declaration: `generate/model.cppm:145`

Definition: `generate/model.cppm:323`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

接受一个 `const SymbolAnalysisStore &` 和一个整数符号标识符，返回指向常量 `FunctionAnalysis` 的指针。如果存储中存有与给定标识符对应的函数分析信息，返回该对象的地址；否则返回 `nullptr`。调用者应确保存储对象在访问期间保持有效，并且返回的指针在存储对象生命周期内有效；此函数不会修改存储内容。

#### Usage Patterns

- Used by code generation routines to retrieve a stored function analysis for further processing.

### `clore::generate::find_implementation_pages`

Declaration: `generate/render/common.cppm:433`

Definition: `generate/render/common.cppm:433`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

`clore::generate::find_implementation_pages` 负责根据调用者提供的一组标识符（类型为 `const int &` 的引用以及一个 `int` 值）定位并返回对应符号实现的文档页面索引。调用者需传入足够的信息（例如符号、作用域或位置标识符）来精确指定目标实体；函数返回的 `int` 值表示找到的实现页面，可用于后续的页面渲染、链接构建等流程。

该函数假定所有输入参数均有效，且目标实现页面已存在于当前生成上下文中。若未找到匹配的实现页面，其行为未定义，调用者应仅在确认相关页面已构建后调用此函数。返回的值仅作为内部页面标识符使用，调用者不应依赖其具体数值或顺序。

#### Usage Patterns

- Generates implementation page links for symbol documentation
- Used in page rendering to provide navigation to implementation details

### `clore::generate::find_module_for_file`

Declaration: `generate/render/common.cppm:496`

Definition: `generate/render/common.cppm:496`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

`clore::generate::find_module_for_file` 根据给定的文件标识符和上下文参数，确定该文件所属的模块，并返回该模块的标识符。调用者提供文件标识符（`const int &`）及一个关联的整数参数，函数返回模块标识符（`int`）。该函数是生成流水线中将文件映射到其所在模块的核心查找入口，供后续模块级页面构建与证据收集使用。

#### Usage Patterns

- Determining module membership for a source file
- Mapping file paths to module names for documentation generation

### `clore::generate::find_type_analysis`

Declaration: `generate/model.cppm:148`

Definition: `generate/model.cppm:329`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

函数 `clore::generate::find_type_analysis` 接受一个 `const SymbolAnalysisStore &` 引用和一个整数标识符，返回指向 `const TypeAnalysis` 的指针。调用者应提供一个有效的存储对象，以及该存储中已为某个类型符号记录的分析标识符。返回的指针指向由存储内部管理的分析结果，调用者不得释放该内存。如果给定的标识符在存储中找不到对应的分析，函数返回 `nullptr`，调用者应当在使用前验证返回值非空。

#### Usage Patterns

- Look up type analysis for a given symbol target key
- Retrieve cached analysis data for a type

### `clore::generate::find_variable_analysis`

Declaration: `generate/model.cppm:151`

Definition: `generate/model.cppm:335`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

函数 `clore::generate::find_variable_analysis` 根据提供的一个 `SymbolAnalysisStore` 和一个表示符号索引的整数，查找并返回一个指向 `VariableAnalysis` 的常量指针。如果该索引对应的变量分析存在，则返回对应的分析对象；否则返回 `nullptr`。调用者应确保传入的整数是有效的符号索引，并需在访问返回的指针前检查其是否为 `nullptr`。

#### Usage Patterns

- retrieving a variable analysis for a given symbol key
- querying analysis results
- accessed by documentation generation functions

### `clore::generate::for_each_symbol_doc_group`

Declaration: `generate/render/symbol.cppm:27`

Definition: `generate/render/symbol.cppm:27`

Implementation: [`Module generate:symbol`](../../../modules/generate/symbol.md)

`clore::generate::for_each_symbol_doc_group` 遍历给定的 `PageDocLayout` 中的所有符号文档组。对于每一个组，它调用传入的 `Visitor` 可调用对象，使其能够在组上执行自定义处理。该函数不控制迭代过程，访问者负责定义对每个组所执行的操作。

#### Usage Patterns

- Iterating over all symbol doc groups in a page layout to generate markdown sections

### `clore::generate::format_evidence_text`

Declaration: `generate/evidence.cppm:86`

Definition: `generate/evidence.cppm:580`

Implementation: [`Module generate:evidence`](../../../modules/generate/evidence.md)

Formats the provided evidence pack into a text representation suitable for downstream processing. Returns an integer that indicates the status of the operation; a value of zero typically denotes success, while non‑zero values signal an error condition. The caller must supply a valid `EvidencePack` and is responsible for interpreting the returned status code.

#### Usage Patterns

- called when the full evidence text is needed without truncation

### `clore::generate::format_evidence_text_bounded`

Declaration: `generate/evidence.cppm:88`

Definition: `generate/evidence.cppm:584`

Implementation: [`Module generate:evidence`](../../../modules/generate/evidence.md)

`clore::generate::format_evidence_text_bounded` 接受一个 `EvidencePack` 和一个 `int` 值，生成该证据包的格式化文本，但将输出限制在由该整数参数指定的边界内。该函数返回一个整数值，用于指示结果（例如实际输出的文本长度或状态码）。调用者应确保提供的边界合理，以满足下游消费场景对长度或规模的约束。

#### Usage Patterns

- Called by `clore::generate::build_prompt` to format evidence with a size limit.

### `clore::generate::generate_dry_run`

Declaration: `generate/generate.cppm:25`

Definition: `generate/scheduler.cppm:1888`

Implementation: [`Module generate:scheduler`](../../../modules/generate/scheduler.md) | [`Module generate`](../../../modules/generate/index.md)

执行文档生成的干运行，模拟完整的生成流程而不产生任何实际的输出或副作用。它接受两个 `const int &` 参数，分别表示生成所需的输入配置与上下文，并返回一个 `int` 值，通常用于指示干运行的结果状态（例如成功或失败代码），或提供生成的统计信息（如预估的页面数量）。调用者可通过此函数在提交正式生成前验证输入的有效性、预览生成范围或评估潜在的成本。

#### Usage Patterns

- called to simulate generation without side effects

### `clore::generate::generate_pages`

Declaration: `generate/generate.cppm:28`

Definition: `generate/scheduler.cppm:1947`

Implementation: [`Module generate:scheduler`](../../../modules/generate/scheduler.md) | [`Module generate`](../../../modules/generate/index.md)

`clore::generate::generate_pages` 是文档生成的入口函数。它接受分析数据和配置参数，执行完整的页面构建流程，并将结果写入目标位置。调用者需要提供两个整数参数（通常表示文档索引结构与符号分析存储）、输出路径（`std::string_view`）、并发任务数（`std::uint32_t`）以及输出格式标识（`std::string_view`）。函数返回一个整数，表示操作结果：零通常代表成功，非零值指示错误或异常终止。该函数是同步阻塞的，调用者应确保所有前置数据已就绪，并在调用后根据返回值决定后续处理。

该函数不负责数据采集或分析，仅负责将已经准备好的内容渲染为文档页面。它隐含承诺：若返回成功，则输出目录下已存在完整生成的文档集，且页面之间的链接和引用均基于同一套页面规划解析。调用者无需关心内部页面计划的构建顺序或缓存策略。

#### Usage Patterns

- Called as the main generation entry point

### `clore::generate::generate_pages_async`

Declaration: `generate/generate.cppm:37`

Definition: `generate/scheduler.cppm:1925`

Implementation: [`Module generate:scheduler`](../../../modules/generate/scheduler.md) | [`Module generate`](../../../modules/generate/index.md)

`clore::generate::generate_pages_async` 是一个异步页面生成函数，它在指定的 `kota::event_loop` 上执行。该函数接受页面范围、输出路径、配置标记和事件循环引用等参数，并返回一个表示生成任务的对象。

调用者的职责是将返回的任务调度到所提供的 `kota::event_loop` 上，并确保其运行以触发实际的生成过程。若不调度并运行该任务，页面生成不会发生。

#### Usage Patterns

- callers schedule the returned task on the event loop
- run the scheduled task on the loop

### `clore::generate::is_base_symbol_analysis_prompt`

Declaration: `generate/analysis.cppm:31`

Definition: `generate/analysis.cppm:325`

Implementation: [`Module generate:analysis`](../../../modules/generate/analysis.md)

该函数判断传入的 `PromptKind` 值是否属于“基础符号分析”提示类别。调用者可利用此函数对提示类型进行快速分类，以在生成分析提示或选择处理逻辑时作出分支决策。

#### Usage Patterns

- used to filter prompt kinds for base symbol analysis
- called by other analysis functions to categorize prompts

### `clore::generate::is_declaration_summary_prompt`

Declaration: `generate/analysis.cppm:33`

Definition: `generate/analysis.cppm:330`

Implementation: [`Module generate:analysis`](../../../modules/generate/analysis.md)

函数 `clore::generate::is_declaration_summary_prompt` 是一个谓词，用来确定给定的 `PromptKind` 值是否表示一种声明摘要提示。调用者使用它来筛选或判定提示类型：当且仅当该提示对应于某个符号（如函数、类型或变量）的声明摘要时，返回 `true`。返回值直接用于在生成过程中选择性地构建或处理声明摘要相关的证据与提示内容，与 `clore::generate::is_page_summary_prompt` 等同类函数一起构成提示分类的判断接口。

#### Usage Patterns

- Used in conditional logic to branch based on whether a prompt kind is a declaration summary
- May be called during prompt construction or evidence building

### `clore::generate::is_function_kind`

Declaration: `generate/model.cppm:162`

Definition: `generate/model.cppm:393`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

`clore::generate::is_function_kind` 接受一个表示符号种类的整数，并返回一个布尔值，指示该整数是否对应于函数符号种类。调用者可以使用此谓词来筛选符号分类，例如在构建符号列表或确定页面类型时。该函数是只读查询，不会修改任何状态。

#### Usage Patterns

- Used to filter symbols in generation logic
- Classifies symbol kinds as function-like

### `clore::generate::is_page_level_symbol`

Declaration: `generate/model.cppm:166`

Definition: `generate/model.cppm:405`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

`clore::generate::is_page_level_symbol` 是一个谓词函数，用于判断由两个 `const int &` 参数共同标识的符号是否属于页面级别。调用者可通过此函数确定符号是否需要在其专属页面中呈现，或者是否应被纳入其他页面的内容中。

#### Usage Patterns

- called during page generation to filter symbols for page creation
- used in `render_page_bundle` or similar pipelines

### `clore::generate::is_page_summary_prompt`

Declaration: `generate/model.cppm:133`

Definition: `generate/model.cppm:297`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

函数 `clore::generate::is_page_summary_prompt` 判定给定的 `PromptKind` 是否代表一个页面摘要提示。调用者使用此函数来区分页面摘要提示与其他提示类别（如声明摘要或符号分析），以决定如何处理或路由该 `PromptKind`。当 `PromptKind` 对应页面摘要时返回 `true`，否则返回 `false`。

#### Usage Patterns

- Used to classify prompt kinds for conditional logic in page generation.

### `clore::generate::is_symbol_analysis_prompt`

Declaration: `generate/model.cppm:134`

Definition: `generate/model.cppm:301`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

函数 `clore::generate::is_symbol_analysis_prompt` 接受一个 `PromptKind` 作为输入，并返回一个 `bool` 值。它用于判断给定的提示类型是否属于符号分析提示类别，返回 `true` 表示该提示应当触发符号分析相关流程，返回 `false` 则表示不属于该类别。

调用者可以在决策逻辑中使用此函数来分支处理符号分析提示与其他类型的提示。该函数是纯查询操作，无副作用，其契约完全由输入 `PromptKind` 决定返回值。

#### Usage Patterns

- Used to classify prompt kinds into symbol analysis categories
- Conditional dispatch in prompt generation or evidence building

### `clore::generate::is_type_kind`

Declaration: `generate/model.cppm:160`

Definition: `generate/model.cppm:380`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

`clore::generate::is_type_kind` 接受一个整数作为输入，并返回一个布尔值，指示该整数是否代表一个类型种类（type kind）。调用方可以传入来自种类枚举或类似分类体系的值，函数会基于内部映射确定该值是否对应一个类型种类。对于任何整数值输入，该函数的行为都是确定的：如果该值被识别为类型种类则返回 `true`，否则返回 `false`。该函数不依赖外部状态，不产生副作用，每次调用都是纯计算性的。

#### Usage Patterns

- Used in code generation logic to filter or categorize symbols by kind.
- Likely called by `build_evidence_for_type_*` functions to determine if a symbol is a type.

### `clore::generate::is_variable_kind`

Declaration: `generate/model.cppm:164`

Definition: `generate/model.cppm:401`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

函数 `clore::generate::is_variable_kind` 接受一个表示符号种类的整数参数。它返回 `true` 如果该种类对应于变量（例如普通变量或成员变量），否则返回 `false`。该谓词用于在符号分类、分析及文档生成流程中区分变量与其他符号（如函数、类型或命名空间）。调用者应确保传入的值是有效的符号种类标识符，且结果仅用于决策，不涉及具体存储或输出格式。

#### Usage Patterns

- used to filter symbols by variable-like kinds
- used in predicate checks for symbol classification

### `clore::generate::make_blockquote`

Declaration: `generate/markdown.cppm:113`

Definition: `generate/markdown.cppm:169`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

将输入的内容包装为 Markdown 块引用（blockquote）。调用者需提供一个整数参数，表示待引用的 Markdown 内容（如通过 `make_raw_markdown` 等函数构建的节点标识）；函数返回一个 `MarkdownNode`，该节点在渲染后会被包裹为块级引用格式。

#### Usage Patterns

- Create blockquote markdown nodes
- Called when generating markdown content

### `clore::generate::make_code`

Declaration: `generate/markdown.cppm:101`

Definition: `generate/markdown.cppm:136`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

`clore::generate::make_code` 接受一个整数参数（通常表示需要格式化的代码片段或符号的标识），并返回一个整数，该整数代表生成的 Markdown 代码表示（例如，内联代码或代码块）的内部节点 ID。调用者应确保提供的参数在上下文中有效且对应可序列化的代码内容，返回的节点 ID 可用于后续的 Markdown 文档构建操作（如组合段落、节或页面）。该函数不直接执行任何副作用的写入或渲染，仅生成结构化的 Markdown 节点。

#### Usage Patterns

- 在 Markdown 渲染中创建内联代码
- 包装字符串为代码片段

### `clore::generate::make_code_fence`

Declaration: `generate/markdown.cppm:109`

Definition: `generate/markdown.cppm:156`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

函数 `clore::generate::make_code_fence` 创建一个表示 Markdown 代码围栏的 `MarkdownNode`。调用者传入两个整数参数，分别指定代码的语言标识和代码正文内容，该函数返回一个结构化的节点，用于在文档中呈现格式化的代码块。此函数是构建 Markdown 文档的基础工具之一，与 `clore::generate::make_mermaid` 和 `clore::generate::make_blockquote` 等节点构造函数处于同一层级，负责生成常见的 Markdown 容器元素。

#### Usage Patterns

- Used to generate a code fence `MarkdownNode` for embedding code snippets in documentation.

### `clore::generate::make_link`

Declaration: `generate/markdown.cppm:103`

Definition: `generate/markdown.cppm:140`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

`clore::generate::make_link` 创建并返回一个链接表示，该链接基于两个整数参数和一个布尔参数配置。调用者传入目标标识符、附加标识符和是否启用特定行为，函数返回一个整数句柄，该句柄可在后续生成操作中引用此链接。

调用方负责确保提供的参数在其使用上下文中有效，并期望返回的整数标识符在整个生成过程中保持稳定。此函数不直接处理文本或目标解析，而是提供底层节点构造的基本契约。

#### Usage Patterns

- Building inline links for markdown generation
- Creating link fragments with optional code formatting

### `clore::generate::make_link_target`

Declaration: `generate/render/common.cppm:81`

Definition: `generate/render/common.cppm:81`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

调用 `clore::generate::make_link_target` 时需要三个整数参数和一个布尔参数。该函数根据这些参数构造并返回一个 `LinkTarget` 值，该值封装了用于文档生成中链接的目标信息。整数参数共同指定目标的身份（如符号索引或页面编号），布尔参数控制链接的某些特性（例如是否使用绝对路径）。调用者可将此 `LinkTarget` 传递给其他链接生成函数，以在输出的文档中创建正确的超链接。

#### Usage Patterns

- 在生成文档页面时创建链接目标
- 配合 `make_relative_link_target` 使用以生成相对路径

### `clore::generate::make_mermaid`

Declaration: `generate/markdown.cppm:111`

Definition: `generate/markdown.cppm:165`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

调用者可以通过整数参数调用 `clore::generate::make_mermaid` 来请求生成一个表示 Mermaid 图的 `MarkdownNode`。该函数负责根据参数构造符合 Mermaid 语法的标记，调用者可将返回的节点插入到更大的文档结构中，在其输出时将被渲染为对应的图表。节点本身封装了完整的 Mermaid 内容，无需额外格式化。

#### Usage Patterns

- Wrap Mermaid diagram code into a `MarkdownNode` for markdown generation

### `clore::generate::make_paragraph`

Declaration: `generate/markdown.cppm:105`

Definition: `generate/markdown.cppm:148`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

函数 `clore::generate::make_paragraph` 创建一个表示 Markdown 段落的节点。它接收一个整数参数，该参数通常标识源文档中的内容片段或相关元数据，并返回一个 `MarkdownNode` 实例。调用者负责提供有效的整数，并可以将返回的节点用于组装更大的 Markdown 文档结构。

#### Usage Patterns

- creating a paragraph markdown element
- wrapping text into a paragraph node

### `clore::generate::make_raw_markdown`

Declaration: `generate/markdown.cppm:107`

Definition: `generate/markdown.cppm:152`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

`clore::generate::make_raw_markdown` 创建一个 `MarkdownNode`，表示一段原始 Markdown 文本，其内容不会经过额外的转义或格式化处理。调用者通过一个整数参数指定要使用的原始 Markdown 内容（例如通过内部表示中的某个标识符）。返回的节点可直接被插入到 Markdown 文档树中，渲染时会原样输出，因此调用者需确保提供的文本是有效的 Markdown 且不包含未闭合的结构。

#### Usage Patterns

- Constructing a `MarkdownNode` from a raw markdown string
- Wrapping preformatted markdown content for inclusion in a document

### `clore::generate::make_relative_link_target`

Declaration: `generate/render/common.cppm:57`

Definition: `generate/render/common.cppm:57`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

`clore::generate::make_relative_link_target` 接受两个整数参数，分别表示源和目标位置的标识符，并返回一个用于唯一标识相对链接目标的整数。该函数是文档生成过程中构建页面间导航链接的基础设施，调用者可通过其返回值在链接解析中引用一个相对目标。函数假定传入的标识符有效且属于同一个生成上下文；未定义传入无效标识符时的行为。

#### Usage Patterns

- generates relative href attributes in page links
- used when constructing cross-references between documentation pages

### `clore::generate::make_section`

Declaration: `generate/markdown.cppm:115`

Definition: `generate/markdown.cppm:173`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

`clore::generate::make_section` 根据给定的 `SemanticKind` 枚举值、四个整数参数和两个布尔标志构建一个 section，并返回一个 `int` 作为该 section 的标识符或句柄。调用方应提供所有六个参数，并确保参数含义与当前文档生成上下文一致；返回的整数可用于后续对 section 的引用或组装。各参数的具体语义由上层调用约定，函数本身不校验除类型外的参数合法性。

#### Usage Patterns

- Creating a semantic section for markdown generation
- Building a section node in the page structure

### `clore::generate::make_source_link_target`

Declaration: `generate/render/common.cppm:383`

Definition: `generate/render/common.cppm:383`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

该函数生成一个指向源代码位置的链接目标，调用者通过提供描述源文件或位置的分量（三个整数引用和一个整数）来指定目标。返回的 `LinkTarget` 对象可供后续链接构建使用，从而在文档中生成可导航至对应源代码区域的链接。

#### Usage Patterns

- Used in page rendering to create source code links

### `clore::generate::make_source_relative`

Declaration: `generate/model.cppm:169`

Definition: `generate/model.cppm:432`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

`clore::generate::make_source_relative` 接受两个 `const int&` 参数，返回一个 `int` 值。它根据两个源位置句柄计算出一个表示相对引用的整数标识符，通常用于在生成的文档中建立源文件路径或链接的相对关系。调用方应提供有效的源标识符；内部通过缓存机制保证多次调用同一对标识符时的效率与稳定性。该结果是后续文档渲染步骤中生成相对路径或链接的关键输入之一。

#### Usage Patterns

- Called by page generation functions to make paths relative for output
- Used when building link targets or source locations

### `clore::generate::make_symbol_target_key`

Declaration: `generate/model.cppm:141`

Definition: `generate/model.cppm:306`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

`clore::generate::make_symbol_target_key` 接受一个表示符号标识符的 `const int &`，并生成一个整数键，该键可作为后续查找或映射操作中的目标键使用。调用方应提供有效的符号标识符；返回的键保证在生成上下文内具有唯一性和可逆性——对应的逆操作由 `clore::generate::parse_symbol_target_key` 提供。

#### Usage Patterns

- Used to generate target keys for symbols during page building
- Called when constructing link targets and symbol references

### `clore::generate::make_text`

Declaration: `generate/markdown.cppm:99`

Definition: `generate/markdown.cppm:132`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

`clore::generate::make_text` 是一个公开函数，负责根据提供的单个 `int` 参数生成并返回一个 `int` 类型的文本表示。调用者需要传递一个合法的整数输入，并接收一个整数作为结果，该结果可用于后续引用或处理生成的文本内容。函数的具体语义由其参数和返回值的含义决定，调用者应参考相关文档以了解输入值与输出文本之间的对应关系。

#### Usage Patterns

- building markdown inline fragments
- converting `std::string` to `TextFragment`

### `clore::generate::namespace_of`

Declaration: `generate/render/common.cppm:53`

Definition: `generate/render/common.cppm:53`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

函数 `clore::generate::namespace_of` 接受一个整数参数（通常表示某个符号的唯一标识），返回该符号所属命名空间的整数标识。调用方应保证传入的标识对应已知符号；如果符号不属于任何命名空间，返回值可能指示全局作用域或未命名命名空间。该函数常用于确定符号的层级归属，以便后续生成命名空间相关的文档结构。

#### Usage Patterns

- Used to compute the namespace portion of a qualified symbol name for page generation or analysis.

### `clore::generate::normalize_frontmatter_title`

Declaration: `generate/render/symbol.cppm:33`

Definition: `generate/render/symbol.cppm:885`

Implementation: [`Module generate:symbol`](../../../modules/generate/symbol.md)

`clore::generate::normalize_frontmatter_title` 接受一个表示前端标题的整数输入，并返回其规范化形式的整数标识符。该函数确保标题符合文档生成的内部格式约定，消除不一致性，使得后续的页面渲染和链接解析能够基于统一的标题表示。调用者应在将标题嵌入最终输出之前调用此函数，以保证标题在文档中的表现一致。

#### Usage Patterns

- Used to clean page titles extracted from frontmatter metadata
- Likely invoked during page rendering in the generate module

### `clore::generate::normalize_markdown_fragment`

Declaration: `generate/analysis.cppm:21`

Definition: `generate/analysis.cppm:267`

Implementation: [`Module generate:analysis`](../../../modules/generate/analysis.md)

`clore::generate::normalize_markdown_fragment` 接受一对 `std::string_view` 参数：第一个是待规范化的原始 Markdown 片段，第二个是规范化上下文。该函数对片段执行标准化处理，使其符合一致的格式要求，以用于后续的文档生成步骤。返回值是一个 `int`，表示操作的结果状态。调用者应检查该值以确认规范化是否成功。

#### Usage Patterns

- normalize AI-generated markdown fragments before rendering
- validate and clean markdown snippets for documentation pages

### `clore::generate::page_summary_cache_key_for_request`

Declaration: `generate/dryrun.cppm:23`

Definition: `generate/dryrun.cppm:293`

Implementation: [`Module generate:dryrun`](../../../modules/generate/dryrun.md)

函数 `clore::generate::page_summary_cache_key_for_request` 负责为给定的页面摘要请求生成一个唯一且确定性的缓存键。调用者需提供两个 `const int &` 参数，其具体语义由调用上下文约定（通常为请求标识或上下文状态），函数返回一个 `int` 类型的键值。此键用于在缓存中索引或检索对应的页面摘要数据，确保相同输入始终映射到相同键。

#### Usage Patterns

- Called to produce a cache key for page summary caching
- Filters requests that should be cached based on prompt kind

### `clore::generate::page_supports_symbol_subpages`

Declaration: `generate/render/symbol.cppm:35`

Definition: `generate/render/symbol.cppm:893`

Implementation: [`Module generate:symbol`](../../../modules/generate/symbol.md)

判断给定的页面标识符是否支持符号子页面。此函数接受一个 `const int &` 类型的页面标识符，并返回一个 `bool` 值，指示该页面是否能够包含符号子页面。调用者可以使用此结果来决定是否对相应页面执行子页面相关的渲染或布局操作。

#### Usage Patterns

- Checking if a page can have subpages for symbols

### `clore::generate::page_type_name`

Declaration: `generate/model.cppm:16`

Definition: `generate/model.cppm:263`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

函数 `clore::generate::page_type_name` 接受一个 `PageType` 参数并返回一个 `int`。调用者应提供代表某种页面分类的 `PageType` 值，该函数返回一个整数，用于标识或表示对应页面类型的规范化名称。此返回值通常用于内部路由、缓存键或者生成页面标题的查找依据。调用者需要确保传入的 `PageType` 枚举值在有效范围内，以便获得稳定且有意义的整数映射。

#### Usage Patterns

- Used wherever a string representation of a page type is needed, such as in page generation or naming.

### `clore::generate::parse_markdown_prompt_output`

Declaration: `generate/analysis.cppm:24`

Definition: `generate/analysis.cppm:281`

Implementation: [`Module generate:analysis`](../../../modules/generate/analysis.md)

`clore::generate::parse_markdown_prompt_output` 解析来自提示的 Markdown 格式输出，将结构化内容提取为调用者可用的形式。它接受两个 `std::string_view` 参数——分别代表原始输出和关联的预期格式或上下文信息——并返回一个整型结果代码，指示解析是否成功或标识提取的数据。调用者必须确保传入的字符串视图在函数执行期间保持有效。

#### Usage Patterns

- used in the generation pipeline to normalize markdown responses from prompt outputs

### `clore::generate::parse_structured_response`

Declaration: `generate/analysis.cppm:18`

Definition: `generate/analysis.cppm:252`

Implementation: [`Module generate:analysis`](../../../modules/generate/analysis.md)

`clore::generate::parse_structured_response` 是一个模板函数，负责将结构化文本响应（如 AI 生成的格式化输出）解析为调用方可用的整数状态或标识符。调用方提供两个 `std::string_view` 参数：第一个是待解析的响应正文，第二个是用于控制解析行为或指定响应格式的上下文标识符。返回值 `int` 指示解析结果，通常零表示成功，负数表示特定错误原因。函数的实际解析逻辑由模板参数 `T` 特化，但无论特化如何，调用方始终应检查返回值以验证解析是否按预期完成。输入必须是符合预期结构化模式的有效文本，否则函数可能返回错误码。

#### Usage Patterns

- called to parse a structured JSON response from an AI prompt into an analysis object

### `clore::generate::parse_symbol_target_key`

Declaration: `generate/model.cppm:143`

Definition: `generate/model.cppm:312`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

函数 `clore::generate::parse_symbol_target_key` 接受一个 `int` 参数，将其解析为一个 `SymbolTargetKeyView` 类型的结果。调用者负责提供一个有效的整数键，该函数返回一个对应的符号目标键视图，供下游生成管道中的查找或引用操作使用。该函数不拥有返回视图所引用的数据；调用者必须确保该视图在其使用期间保持有效。

#### Usage Patterns

- Used to convert a `std::string_view` symbol target key into a structured view
- Called when processing symbol references that may include signatures

### `clore::generate::prompt_kind_name`

Declaration: `generate/model.cppm:32`

Definition: `generate/model.cppm:273`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

`clore::generate::prompt_kind_name` 将 `PromptKind` 值转换为一个整数名称标识符。调用方提供一个 `PromptKind` 枚举值，函数返回与该提示种类相对应的唯一整数，该整数可用于在生成过程中标识或比较提示类型。

#### Usage Patterns

- used in prompt building and identification functions
- called when generating prompt names for different analysis kinds

### `clore::generate::prompt_output_of`

Declaration: `generate/render/common.cppm:71`

Definition: `generate/render/common.cppm:71`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

函数 `clore::generate::prompt_output_of` 接受一个输入对象（通过 `const int &`）和两个 `int` 参数，并返回一个 `int` 值。调用者应提供这些参数以指定提示上下文和配置细节；函数负责根据这些参数生成并返回对应的提示输出。该输出值的具体含义由调用环境中的约定所定义。

#### Usage Patterns

- Used to retrieve prompt output for a specific kind and target key
- Called by `build_evidence_for_*` functions to access cached analysis results
- Checked for `nullptr` to handle missing outputs

### `clore::generate::prompt_request_key`

Declaration: `generate/model.cppm:131`

Definition: `generate/model.cppm:290`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

`clore::generate::prompt_request_key` 接受一个 `PromptRequest` 对象，返回一个作为该请求唯一键的整数。调用者可使用此键来标识、索引或缓存给定的提示请求。该键在请求的生命周期内应保持稳定，可安全用于基于值的关联容器或去重操作。

#### Usage Patterns

- used as a cache key for prompt requests
- used to index or group prompts by kind and target

### `clore::generate::render_file_dependency_diagram_code`

Declaration: `generate/render/diagram.cppm:20`

Definition: `generate/render/diagram.cppm:222`

Implementation: [`Module generate:diagram`](../../../modules/generate/diagram.md)

函数 `clore::generate::render_file_dependency_diagram_code` 生成表示文件依赖关系的图表代码。它接受三个整数标识符（通常为目标文件、项目或模块以及额外的范围或格式参数），并返回一个图表标识符或状态码。调用方应提供正确的标识符，返回的图表代码可用于嵌入 Markdown 文档或供渲染管线进一步处理。该函数属于 clore 生成框架中的图表生成系列，与 `render_import_diagram_code` 和 `render_module_dependency_diagram_code` 等函数功能类似。

#### Usage Patterns

- Called by page rendering functions to generate file dependency diagrams
- Used in documentation generation to produce Mermaid diagram markup

### `clore::generate::render_import_diagram_code`

Declaration: `generate/render/diagram.cppm:15`

Definition: `generate/render/diagram.cppm:124`

Implementation: [`Module generate:diagram`](../../../modules/generate/diagram.md)

函数 `clore::generate::render_import_diagram_code` 接受一个 `const int &` 类型的目标标识符，返回一个 `int` 状态码。它负责在当前代码分析上下文中，为指定的符号或模块生成导入关系图的渲染代码（通常为 Mermaid 格式）。调用者必须确保传入的标识符已在符号分析数据中注册，且调用时机位于分析数据收集完成之后。该函数由页面渲染流程调用；返回非零值表明渲染过程遇到错误，生成的图表可能不完整或未输出。

#### Usage Patterns

- Called during documentation generation to produce import dependency diagrams
- Used in page rendering pipelines for module pages

### `clore::generate::render_markdown`

Declaration: `generate/markdown.cppm:122`

Definition: `generate/markdown.cppm:730`

Implementation: [`Module generate:markdown`](../../../modules/generate/markdown.md)

The function `clore::generate::render_markdown` accepts a `const MarkdownDocument &` and returns an `int`. Its primary responsibility is to render the provided Markdown document into a final output artifact (such as a complete string, file, or stream). The return value serves as a status code indicating success or a specific failure condition. Callers must supply a fully constructed `MarkdownDocument`; the function then performs the top-level transformation to produce the final rendered form.

#### Usage Patterns

- Called by rendering functions to produce a markdown string from a `MarkdownDocument`
- Used as a building block for page generation

### `clore::generate::render_module_dependency_diagram_code`

Declaration: `generate/render/diagram.cppm:24`

Definition: `generate/render/diagram.cppm:289`

Implementation: [`Module generate:diagram`](../../../modules/generate/diagram.md)

该函数接收一个模块标识符，用于生成描述该模块与其它模块间依赖关系的图表代码。它属于文档生成管线中的渲染步骤，输出通常为 Mermaid 格式，以供后续嵌入页面。调用者应传入有效的模块标识符，函数返回一个整数值表示操作结果。

#### Usage Patterns

- called during documentation generation to produce module dependency diagram
- result embedded into Mermaid code blocks

### `clore::generate::render_namespace_diagram_code`

Declaration: `generate/render/diagram.cppm:17`

Definition: `generate/render/diagram.cppm:168`

Implementation: [`Module generate:diagram`](../../../modules/generate/diagram.md)

函数 `clore::generate::render_namespace_diagram_code` 负责生成描述命名空间结构的可视化图代码（例如适用于 Mermaid 或类似工具的文本表示）。它接受两个参数：第一个参数（类型为 `const int &`）标识目标命名空间或相关的文档上下文，第二个参数（类型为 `int`）指定额外的配置选项（例如图的深度或显示模式）。返回的 `int` 值指示操作的结果状态——通常为成功时的零值，非零值表示错误。调用者必须确保提供的标识符对应一个有效的命名空间，且选项参数在预期范围内。该函数不直接产生输出文件，而是返回可供后续渲染使用的结构化图代码。

#### Usage Patterns

- Called during documentation generation to produce namespace diagram markdown.

### `clore::generate::render_page_bundle`

Declaration: `generate/render/page.cppm:565`

Implementation: [`Module generate:page`](../../../modules/generate/page.md)

调用 `clore::generate::render_page_bundle` 可将六个整数常量引用表示的输入数据渲染为一个页面捆绑包，并返回一个整数指示操作结果。该函数是文档生成管线中的公共接口，调用者仅需提供符合内部约定的参数，无需关心渲染的具体机制。

调用者应保证传入的六个 `const int &` 参数均指向有效状态；返回值零通常表示成功，非零值表示错误或需由上层处理的状态码。函数本身不负责验证参数的语义正确性，因此调用者必须确保参数与当前生成上下文兼容。

#### Usage Patterns

- Called from page generation pipeline (e.g., `generate_pages` or `generate_pages_async`) to produce a single page bundle.
- Used as the main rendering step after all analysis and link resolution is available.

### `clore::generate::render_page_bundle`

Declaration: `generate/render/page.cppm:573`

Implementation: [`Module generate:page`](../../../modules/generate/page.md)

调用 `clore::generate::render_page_bundle` 以渲染一个页面束（bundle），即一组在逻辑上相关联的生成页面。调用者必须提供五个标识参数（均为 `const int &`），这些参数共同明确指定要渲染的束所涉及的作用域、目标或源标识符。函数返回一个 `int` 指示处理结果，通常为正整数表示成功生成的页面数，或为零或负数表示失败或未生成。

调用者应当确保提供的标识符在当前的页面计划上下文中有效且彼此兼容，因为此函数假定这些标识符均对应于同一束定义。违反此契约可能导致未定义行为或错误输出。

#### Usage Patterns

- Called by higher-level page generation functions like `generate_pages`
- Used to render a single page bundle as part of a larger documentation generation pipeline

### `clore::generate::render_page_markdown`

Declaration: `generate/render/page.cppm:582`

Definition: `generate/render/page.cppm:582`

Implementation: [`Module generate:page`](../../../modules/generate/page.md)

函数 `clore::generate::render_page_markdown` 根据一组页面标识与配置参数生成对应的 Markdown 文本。调用者应确保传入的六个整数参数有效且指向同一页面的上下文；该函数返回一个整型状态码，用于指示渲染操作是否成功。

#### Usage Patterns

- Used in page generation pipeline to retrieve the final Markdown string for a specific page

### `clore::generate::render_page_markdown`

Declaration: `generate/render/page.cppm:602`

Definition: `generate/render/page.cppm:602`

Implementation: [`Module generate:page`](../../../modules/generate/page.md)

函数 `clore::generate::render_page_markdown` 负责根据一组资源标识符生成页面的完整 Markdown 内容。调用者提供五个 `const int &` 参数，这些参数引用构建页面所需的各类内部资源（例如页面规划、符号分析或证据包），函数返回一个 `int` 值，该值标识所生成的 Markdown 文档或代表操作结果。

调用者在调用时需确保传入的每个 `const int &` 参数都指向有效且与当前渲染上下文匹配的资源标识符。函数不修改任何参数，也不依赖外部状态。返回值可被后续操作使用，例如写入文件或传递给 `clore::generate::render_markdown` 等渲染工具。无需了解实现细节，只需正确传递标识符。

#### Usage Patterns

- Used when no precomputed symbol analysis data is available

### `clore::generate::short_name_of`

Declaration: `generate/render/common.cppm:45`

Definition: `generate/render/common.cppm:45`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

函数 `clore::generate::short_name_of` 接受一个整数作为输入，并返回另一个整数，表示与给定标识符关联的“简短名称”。它用于将符号或对象的标识符映射到一个紧凑、可区分的整数值，以便在文档生成管线中作为轻量级引用或索引使用。

调用者需保证传入的整数标识符在当前的上下文中是有效且已定义的。返回值是该标识符对应的简短名称的数值表示；其具体语义（例如是否为稳定标识符或仅用于局部作用域）由调用上下文决定。函数对于无效或未注册的输入不保证行为稳定，因此应仅在合适的输入集合上调用。

#### Usage Patterns

- Used to obtain a symbol's simple name for display purposes
- Can be used in formatting documentation output
- Likely called by rendering functions needing short names

### `clore::generate::should_emit_mermaid`

Declaration: `generate/render/diagram.cppm:11`

Definition: `generate/render/diagram.cppm:105`

Implementation: [`Module generate:diagram`](../../../modules/generate/diagram.md)

函数 `clore::generate::should_emit_mermaid` 接受两个 `int` 参数并返回 `bool`，用于判断在当前的生成上下文中是否应当输出 Mermaid 图。调用者负责提供与生成场景相关的两个整数参数（通常表示符号标识符或配置选项），函数根据内部规则返回一个布尔值，指示是否应包含对应的 Mermaid 图表。

该函数是 `clore::generate` 渲染管线的一部分，用于在生成文档时按需决定图表的启用状态。调用者应当将返回值作为是否调用图表生成逻辑的依据，而不应假设具体的判定条件。

#### Usage Patterns

- called before rendering mermaid diagrams such as dependency graphs
- used to decide diagram inclusion in documentation pages

### `clore::generate::store_fallback_analysis`

Declaration: `generate/analysis.cppm:35`

Definition: `generate/analysis.cppm:335`

Implementation: [`Module generate:analysis`](../../../modules/generate/analysis.md)

函数 `clore::generate::store_fallback_analysis` 负责将回退分析的结果存储到由第一个参数 `int &` 引用的对象中。调用者必须提供一个可修改的整数引用以及两个常量整数引用作为输入，函数会利用后两个参数的信息更新第一个参数，以反映相应的回退分析状态。

调用者有责任确保传入的第一个参数引用一个有效的、可写入的整数对象，且该对象在该函数返回后仍然可用。后两个参数作为只读输入，用于提供分析所需的上下文。函数严格按照其参数约定执行，不会在调用者提供的存储之外分配资源。

#### Usage Patterns

- Used to insert fallback analysis entries into a `SymbolAnalysisStore`
- Called when primary analysis is unavailable or incomplete

### `clore::generate::strip_inline_markdown`

Declaration: `generate/render/common.cppm:33`

Definition: `generate/render/common.cppm:33`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

`clore::generate::strip_inline_markdown` 接受一个整数标识的 Markdown 内容（例如来自内部表示的片段），并返回一个新整数标识的 Markdown 内容，其中移除了内联格式标记。调用者输入的内容应表示有效的 Markdown 数据结构；函数会剥离如加粗、斜体、内联代码、链接等内联语法，仅保留文本主体。返回的整数句柄代表去除格式后的版本，可用于需要纯文本内容的后续处理（如索引、搜索或文本比较）。调用者需确保传入的句柄有效，且函数不修改原始输入。

#### Usage Patterns

- cleaning markdown for plain text output
- preprocessing text for display in contexts where markdown is not allowed

### `clore::generate::symbol_analysis_markdown_for`

Declaration: `generate/render/common.cppm:161`

Definition: `generate/render/common.cppm:161`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

`clore::generate::symbol_analysis_markdown_for` 接受三个 `const int &` 标识符参数，生成表示关联符号分析结果的 Markdown 内容。调用者必须确保所提供的标识符对应于有效的分析存储和符号实体。函数返回一个 `int`，指示操作结果或所生成内容的标识符。

#### Usage Patterns

- Called to produce symbol analysis markdown for a page
- Uses page type to choose between overview and detailed analysis

### `clore::generate::symbol_doc_view_for`

Declaration: `generate/render/common.cppm:269`

Definition: `generate/render/common.cppm:269`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

`clore::generate::symbol_doc_view_for` 生成并返回一个 `SymbolDocView`，该视图封装了指定符号的文档数据。第一个 `const int &` 参数标识目标符号，第二个 `const int &` 参数提供生成视图所需的上下文信息（例如所属模块或页面）。调用者必须保证传入的引用在调用期间保持有效且指向已注册的符号与上下文。返回的 `SymbolDocView` 所有权转移给调用者，可被用于后续文档渲染或分析操作，其生命周期由调用者管理。

#### Usage Patterns

- Determining symbol documentation view during page generation

### `clore::generate::symbol_prompt_kinds_for_symbol`

Declaration: `generate/analysis.cppm:29`

Definition: `generate/analysis.cppm:299`

Implementation: [`Module generate:analysis`](../../../modules/generate/analysis.md)

`clore::generate::symbol_prompt_kinds_for_symbol` 接受一个符号标识符（`const int &`）并返回一个 `int` 值，该值编码了适用于该符号的所有 prompt 种类。调用者使用此函数来查询给定符号应参与生成哪些类型的 prompt（例如，符号分析 prompt 或声明摘要 prompt）。返回的整数可以用于与 `is_symbol_analysis_prompt`、`is_declaration_summary_prompt` 等标签检查函数协作，以在后续的 prompt 构建步骤中进行条件分支。该函数只负责种类决策，不执行 prompt 的组装或渲染。

#### Usage Patterns

- Used to obtain the list of prompts to generate for a symbol
- Called by prompt-building logic to decide which summary prompts to include

### `clore::generate::trim_ascii`

Declaration: `generate/render/common.cppm:23`

Definition: `generate/render/common.cppm:23`

Implementation: [`Module generate:common`](../../../modules/generate/common.md)

`clore::generate::trim_ascii` 接受一个 `int` 值并返回一个 `int`，用于对输入执行基于 ASCII 的修剪操作。调用者可以依赖此函数来标准化或清理标识符、字符码或字节值，确保输出仅保留修剪后的有效 ASCII 部分。返回值的具体范围和行为由实现定义，但契约保证结果代表输入经过 ASCII 修剪后的逻辑表示。

#### Usage Patterns

- Used as a utility to sanitize or normalize text input before further processing
- Likely called by other generate functions to trim whitespace from strings

### `clore::generate::validate_no_path_conflicts`

Declaration: `generate/model.cppm:216`

Definition: `generate/model.cppm:644`

Implementation: [`Module generate:model`](../../../modules/generate/model.md)

`clore::generate::validate_no_path_conflicts` 校验给定的输入是否存在路径冲突，并返回一个整型结果指示验证状态。调用者应在执行任何可能因目标路径重叠而出错的生成操作之前调用此函数，以确保输出路径的唯一性和无冲突性。该函数的契约要求传入一个标识待验证路径或上下文的整数参数，并返回一个表示验证通过或失败的代码。

#### Usage Patterns

- Used to ensure unique output paths before generating pages
- Called during page generation to prevent path collisions

### `clore::generate::write_page`

Declaration: `generate/render/page.cppm:666`

Definition: `generate/render/page.cppm:666`

Implementation: [`Module generate:page`](../../../modules/generate/page.md)

函数 `clore::generate::write_page` 接受一个 `const int &` 类型的页面标识符和一个 `int` 类型的输出目标（例如文件描述符或流句柄），负责将单个生成页面持久化到该目标中。调用者应确保提供的标识符对应于一个已构造完成的页面，且输出目标处于可写入状态。返回的 `int` 值指示操作结果：通常为零表示成功，非零表示写入过程中发生了错误。该函数不管理页面的构造或释放，仅处理渲染后内容的输出，是下游写入流程的原子单元。

#### Usage Patterns

- called to output a single generated page file
- used in the page generation pipeline after building the page content

### `clore::generate::write_pages`

Declaration: `generate/generate.cppm:44`

Definition: `generate/scheduler.cppm:1966`

Implementation: [`Module generate:scheduler`](../../../modules/generate/scheduler.md) | [`Module generate`](../../../modules/generate/index.md)

函数 `clore::generate::write_pages` 负责将已生成的文档页面集写出到指定的输出位置。它接受一个 `const int &` 作为页面集合的标识符，以及一个 `std::string_view` 表示目标目录路径。返回一个 `int` 指示操作结果（通常为成功或错误码）。调用者需确保提供的页面集合标识符有效，且输出路径可写；在调用本函数前，应已完成所有必要的页面生成步骤。

#### Usage Patterns

- Called during documentation generation to write all pages

## Related Pages

- [Namespace clore](../index.md)
- [Namespace clore::generate::cache](cache/index.md)

