---
title: 'Module config:schema'
description: '模块 config:schema 负责定义 CLORE 工具的配置数据结构，为整个项目的配置系统提供类型骨架。其公开实现范围涵盖三个核心结构体：FilterRule（包含文件包含/排除规则）、LLMConfig（语言模型调用参数，如系统提示和重试限制）以及 TaskConfig（任务级配置，汇总项目路径、编译数据库路径、输出根目录、过滤规则与 LLM 设置）。这些类型共同构成了配置文件的静态模式，供解析、验证与序列化组件使用。'
layout: doc
template: doc
---

# Module `config:schema`

## Summary

模块 `config:schema` 负责定义 CLORE 工具的配置数据结构，为整个项目的配置系统提供类型骨架。其公开实现范围涵盖三个核心结构体：`FilterRule`（包含文件包含/排除规则）、`LLMConfig`（语言模型调用参数，如系统提示和重试限制）以及 `TaskConfig`（任务级配置，汇总项目路径、编译数据库路径、输出根目录、过滤规则与 LLM 设置）。这些类型共同构成了配置文件的静态模式，供解析、验证与序列化组件使用。

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

`clore::config::FilterRule` 的实现内部仅包含两个 `std::vector<std::string>` 数据成员：`include` 和 `exclude`。这两个无序模式列表共同构成过滤规则的核心载体，没有额外的辅助成员或状态标志。由于未定义用户提供的构造函数、析构函数或赋值运算符，该类型的对象完全依赖编译器合成的特殊成员函数进行构造、拷贝和移动，其语义相当于一个简单的聚合体。外部代码通过直接读取或修改这两个向量来设置过滤条件，过滤逻辑本身并不集成在结构体内，而是由使用方遍历 `include` 与 `exclude` 的条目并执行模式匹配。此结构不维护内部不变量或附加约束（例如模式字符串的格式验证或去重），所有输入有效性由调用者保证。

#### Invariants

- Both `include` and `exclude` vectors can be empty
- No implied ordering or mutual exclusivity between the two lists

#### Key Members

- `include`
- `exclude`

#### Usage Patterns

- Used as a building block in configuration systems to filter items based on inclusion and exclusion criteria
- Likely checked by other code to decide whether to allow or deny a given element

### `clore::config::LLMConfig`

Declaration: `config/schema.cppm:12`

Definition: `config/schema.cppm:12`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

`clore::config::LLMConfig` 是一个聚合类型，包含两个公有数据成员：`system_prompt` 为 `std::string` 类型，`retry_limit` 为 `std::uint32_t` 类型。成员 `retry_limit` 的默认值为 0，这意味着在默认构造的实例中，重试行为被禁用。由于结构体采用聚合初始化，其成员可以通过花括号初始化列表直接赋值，且编译器隐式生成了默认构造函数、析构函数、复制及移动语义。该结构体没有显式设定的类不变量，因此所有合法值均可被接受，但 `retry_limit` 为 0 的约定赋予了默认配置特定的语义含义。

#### Invariants

- `retry_limit` 为无符号整数，值域为 `[0, UINT32_MAX]`
- `system_prompt` 可以是任意字符串，无长度或内容约束

#### Key Members

- `system_prompt`
- `retry_limit`

#### Usage Patterns

- 直接作为配置参数传递给 LLM 相关的函数或类
- 通过聚合初始化或成员赋值创建实例
- 可能被序列化为 JSON 或 YAML 等格式以持久化配置

### `clore::config::TaskConfig`

Declaration: `config/schema.cppm:17`

Definition: `config/schema.cppm:17`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

`clore::config::TaskConfig` 以扁平字段形式直接存储核心路径配置，包括 `compile_commands_path`、`project_root`、`output_root` 和 `workspace_root`（均为 `std::string`），并包含两个聚合子对象 `filter`（`FilterRule`）和 `llm`（`LLMConfig`）。结构体不提供任何方法或构造函数，所有成员均为公有，因此不变量由调用方维护；设计上期望这些字符串代表有效的文件系统路径，且子对象在其各自类型中保持合法状态。字段的声明顺序（路径串在前，子对象在后）决定了默认的内存布局，但未引入额外的运行时约束或初始化逻辑。

#### Invariants

- All path fields should refer to valid filesystem locations when the struct is used
- `filter` and `llm` sub-configurations must be fully initialized before task execution

#### Key Members

- `compile_commands_path`
- `project_root`
- `output_root`
- `workspace_root`
- `filter`
- `llm`

#### Usage Patterns

- Instantiated and populated from a configuration file or command-line arguments
- Passed to task infrastructure components to control LLM interaction and file filtering
- Used as a data transfer object for task setup

## Internal Structure

`config:schema` 模块定义了核心配置数据结构，实现了关注点分离：纯数据定义与解析逻辑解耦。它仅导入标准库，保持轻量独立。内部层次清晰——`TaskConfig` 复合了 `LLMConfig` 和 `FilterRule`，分别管理 AI 模型参数与路径筛选规则。`FilterRule` 的 `include`/`exclude` 字段体现了最小化、正交的设计，`LLMConfig` 包含 `system_prompt` 和 `retry_limit`，聚焦于运行时可调参数。整个模块位于单个接口文件 `schema.cppm` 中，形成自包含的类型定义层，为上层配置解析及业务逻辑提供类型安全的基础。

