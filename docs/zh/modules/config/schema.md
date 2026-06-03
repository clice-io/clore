---
title: 'Module config:schema'
description: '模块 config:schema 负责定义核心配置数据结构，作为整个配置系统的公共接口。它提供了 clore::config::TaskConfig，其中包含项目根目录、工作空间根目录、输出根目录、编译命令路径、筛选规则与 LLM 配置等字段；clore::config::LLMConfig 用于描述大语言模型相关的系统提示与重试限制；以及 clore::config::FilterRule 用于指定文件包含/排除模式。这些结构体构成了用户可读写的配置模式，使配置的序列化、验证和访问具有一致的类型基础。'
layout: doc
template: doc
---

# Module `config:schema`

## Summary

模块 `config:schema` 负责定义核心配置数据结构，作为整个配置系统的公共接口。它提供了 `clore::config::TaskConfig`，其中包含项目根目录、工作空间根目录、输出根目录、编译命令路径、筛选规则与 LLM 配置等字段；`clore::config::LLMConfig` 用于描述大语言模型相关的系统提示与重试限制；以及 `clore::config::FilterRule` 用于指定文件包含/排除模式。这些结构体构成了用户可读写的配置模式，使配置的序列化、验证和访问具有一致的类型基础。

## Imported By

- [`config:load`](load.md)
- [`config:normalize`](normalize.md)
- [`config:validate`](validate.md)

## Types

### `clore::config::FilterRule`

Declaration: `src/config/schema.cppm:13`

Definition: `src/config/schema.cppm:13`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

`clore::config::FilterRule` 的实现是一个简单的聚合体，其内部状态仅由两个 `std::vector<std::string>` 数据成员 `include` 和 `exclude` 构成。这种布局将过滤逻辑的输入与内部规则存储分离：两个向量各自独立维护一个字符串列表，分别表示需要包含的和需要排除的模式。结构体不维护任何额外的计数、标志或预处理结果，所有操作（如添加、删除模式）完全依赖于对这两个向量的直接修改。由于没有自定义构造函数或析构函数，默认构造时两个向量均为空，用户需通过外部接口设置内容。该设计的核心不变量是 `include` 和 `exclude` 列表之间不存在隐式顺序或互斥关系，过滤器的实际语义由调用方在外部组合时定义。

#### Invariants

- 包含列表和排除列表独立存在，不隐含优先级或覆盖关系
- 列表中的字符串顺序不影响过滤逻辑

#### Key Members

- `include`：存储需要包含的字符串模式
- `exclude`：存储需要排除的字符串模式

#### Usage Patterns

- 作为函数参数传递过滤条件
- 在配置文件解析中填充该结构体
- 用于遍历集合时根据包含/排除列表决定元素保留或丢弃

### `clore::config::LLMConfig`

Declaration: `src/config/schema.cppm:18`

Definition: `src/config/schema.cppm:18`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

`clore::config::LLMConfig` 的实现由两个数据成员构成：`std::string system_prompt` 和 `std::uint32_t retry_limit`。`system_prompt` 未显式初始化，因此在默认构造后为空字符串；`retry_limit` 被默认初始化为 `0`，表示无重试限制。该结构体是可直接聚合初始化的平凡类型，不定义任何用户提供的构造函数、析构函数或拷贝控制成员。

实现层面的不变性主要来自类型本身的语义：`retry_limit` 作为无符号整数，其值始终非负，用于控制重试次数。`system_prompt` 则通过 `std::string` 的 RAII 管理动态内存，允许空状态的存在。两个成员的默认值共同定义了该配置的最小安全状态（无重试、无系统提示），需要在后续初始化中按需改写。

#### Invariants

- `retry_limit` 默认值为 0，调用方应确保其为非负整数
- `system_prompt` 可包含任意字符串，无内容约束

#### Key Members

- `system_prompt`
- `retry_limit`

#### Usage Patterns

- 作为配置数据结构在模块 `clore.config` 内部传递
- 被高层配置类或解析器填充后传递给 LLM 执行组件

### `clore::config::TaskConfig`

Declaration: `src/config/schema.cppm:23`

Definition: `src/config/schema.cppm:23`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

`clore::config::TaskConfig` 作为一个扁平的数据容器，聚合并存储了任务执行的完整配置。其六个公开成员（`project_root`、`workspace_root`、`output_root`、`compile_commands_path`、`filter` 和 `llm`）分别对应项目根目录、工作区根目录、输出根目录、编译命令数据库路径、文件过滤规则以及大语言模型配置。所有成员均为直接存储的简单值类型（`std::string` 或值类型对象），结构体本身不包含任何校验或转换逻辑，因此配置的有效性完全依赖于外部使用者对成员值的正确设置，且成员之间不存在隐式依赖关系。

#### Invariants

- No invariants are visible from the evidence; all fields are publicly assignable.

#### Key Members

- `compile_commands_path`
- `project_root`
- `output_root`
- `workspace_root`
- `filter`
- `llm`

#### Usage Patterns

- The type is intended to be populated with task configuration data, likely from a configuration file or user input.
- No usage examples are present in the evidence; the struct appears as a simple data container.

## Internal Structure

`config:schema` 模块定义了 `clore::config` 命名空间下的核心配置数据结构，包括 `FilterRule`、`LLMConfig` 和 `TaskConfig`。该模块是一个轻量的纯数据描述层，不依赖于其他内部模块或外部库，专注于为配置解析与验证提供类型化骨架。通过将路径、过滤规则和 LLM 参数聚合到 `TaskConfig` 中，模块清晰地表达了配置模型的层次关系，同时保持了各结构体的单一职责。

