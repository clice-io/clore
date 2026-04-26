---
title: 'Module config:schema'
description: '模块 config:schema 负责声明 Clore 工具链中核心配置数据结构的公共类型定义，包括 TaskConfig、LLMConfig 和 FilterRule。这些类型通过公开字段暴露了任务执行所需的所有配置项：路径配置（项目根目录、工作区根目录、输出根目录、编译命令数据库路径）、LLM 行为配置（系统提示、重试次数）以及文件过滤规则（包含/排除模式）。作为配置模块的数据合同，它独立于序列化或解析逻辑，仅定义结构体及其成员的可访问性，供配置加载、验证和消费方统一引用。'
layout: doc
template: doc
---

# Module `config:schema`

## Summary

模块 `config:schema` 负责声明 Clore 工具链中核心配置数据结构的公共类型定义，包括 `TaskConfig`、`LLMConfig` 和 `FilterRule`。这些类型通过公开字段暴露了任务执行所需的所有配置项：路径配置（项目根目录、工作区根目录、输出根目录、编译命令数据库路径）、LLM 行为配置（系统提示、重试次数）以及文件过滤规则（包含/排除模式）。作为配置模块的数据合同，它独立于序列化或解析逻辑，仅定义结构体及其成员的可访问性，供配置加载、验证和消费方统一引用。

## Imports

- `std`

## Imported By

- [`config:load`](load.md)
- [`config:normalize`](normalize.md)
- [`config:validate`](validate.md)

## Types

### `clore::config::FilterRule`

Declaration: `config/schema.cppm:7`

Definition: `config/schema.cppm:7`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

`clore::config::FilterRule` 的内部结构由两个公开的 `std::vector<std::string>` 成员 `include` 和 `exclude` 组成，分别代表允许匹配和拒绝匹配的模式集合。该结构本身未定义任何构造函数、赋值运算符或其他成员函数，因此其生命周期完全由默认的隐式构造和析构管理。核心的不变性假设是 `include` 与 `exclude` 列表在逻辑上互斥——即同一个模式不应同时出现在两个列表中——但结构体自身不强制执行这一约束，依赖外部调用方或解析逻辑来保证语义一致性。`include` 和 `exclude` 向量的添加、删除、遍历等操作均委托给 `std::vector` 的标准接口，无额外封装或检查。

#### Invariants

- Each member is independently mutable
- No constraints between include and exclude lists

#### Key Members

- `include`
- `exclude`

#### Usage Patterns

- Used to represent filter configurations
- Lists can be populated by callers to define filtering behavior

### `clore::config::LLMConfig`

Declaration: `config/schema.cppm:12`

Definition: `config/schema.cppm:12`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

`clore::config::LLMConfig` 是一个聚合类型，内部通过两个公有成员保存 LLM 调用的基本配置。`system_prompt` 字段为 `std::string` 类型，用于存储系统级提示词；`retry_limit` 字段为 `std::uint32_t` 类型，默认初始化为 `0`，表示不进行重试。作为实现层的数据载体，该结构体不提供自定义构造函数或校验逻辑，所有成员均为直接访问，使用者需在构造后自行保证 `retry_limit` 的非负性（已由无符号类型保证）以及 `system_prompt` 的有效性。

#### Invariants

- `retry_limit` 初始化为0

#### Key Members

- `system_prompt`
- `retry_limit`

#### Usage Patterns

- 作为配置对象传递给LLM接口
- 从配置文件中解析

### `clore::config::TaskConfig`

Declaration: `config/schema.cppm:17`

Definition: `config/schema.cppm:17`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

结构体 `clore::config::TaskConfig` 将所有任务级配置参数聚合为一个纯聚合类型，不定义自定义构造函数或成员函数。其成员包括四个 `std::string` 路径字段（`compile_commands_path`、`project_root`、`output_root` 和 `workspace_root`），分别指定编译命令数据库位置、项目根、输出根和工作区根；以及一个 `FilterRule` 实例 `filter` 和一个 `LLMConfig` 实例 `llm`，分别控制过滤规则与语言模型配置。该结构无显式不变量约束，所有成员均为公开，调用方负责在赋值后维护路径间的语义一致性（例如 `workspace_root` 应为 `project_root` 的子目录）。

#### Invariants

- Paths are stored as `std::string`.
- `filter` is of type `FilterRule`.
- `llm` is of type `LLMConfig`.

#### Key Members

- `compile_commands_path`
- `project_root`
- `output_root`
- `workspace_root`
- `filter`
- `llm`

#### Usage Patterns

- Used as a configuration container passed to task execution functions.
- Aggregates directory paths and sub-configurations for filtering and LLM interaction.

## Internal Structure

`config:schema` 模块定义了配置层核心的数据类型，包括 `FilterRule`、`LLMConfig` 和 `TaskConfig`，所有字段均为公有访问，设计为纯值对象。该模块仅依赖标准库，内部层次平坦，没有进一步拆分或隐藏的实现细节；它作为 `config` 模块的一个分区，集中存放配置模式的实体定义，供其他分区或使用者直接引用。

