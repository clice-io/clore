---
title: 'Module extract:ast'
description: '模块 extract:ast 负责基于 Clang AST 的符号提取，是 C++ 项目解析流水线的核心引擎。它通过自定义的递归 AST 访问器（SymbolExtractorVisitor）和消费者（SymbolExtractorConsumer）来遍历翻译单元，为每个可提取的声明计算唯一标识、源片段边界、词法上下文、文档注释和函数签名，同时收集各类关系（函数调用、成员访问、引用、继承）以及文件依赖。提取结果以 ASTResult 结构体封装，包含符号列表、关系列表和依赖文件集合，而模块唯一的公开入口点为 extract_symbols 函数。'
layout: doc
template: doc
---

# Module `extract:ast`

## Summary

模块 `extract:ast` 负责基于 Clang AST 的符号提取，是 C++ 项目解析流水线的核心引擎。它通过自定义的递归 AST 访问器（`SymbolExtractorVisitor`）和消费者（`SymbolExtractorConsumer`）来遍历翻译单元，为每个可提取的声明计算唯一标识、源片段边界、词法上下文、文档注释和函数签名，同时收集各类关系（函数调用、成员访问、引用、继承）以及文件依赖。提取结果以 `ASTResult` 结构体封装，包含符号列表、关系列表和依赖文件集合，而模块唯一的公开入口点为 `extract_symbols` 函数。

## Imports

- [`extract:compiler`](compiler.md)
- [`extract:model`](model.md)
- `std`
- [`support`](../support/index.md)

## Imported By

- [`extract:cache`](cache.md)

## Types

### `clore::extract::ASTError`

Declaration: `extract/ast.cppm:26`

Definition: `extract/ast.cppm:26`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

结构体 `clore::extract::ASTError` 的实现是一个仅包含单个 `std::string message` 成员的聚合体，其内部结构完全依赖于标准字符串的存储和生命周期管理。该设计使得错误消息能够以零额外开销的方式被动态构造、拷贝和移动，编译器自动生成的默认构造函数、析构函数以及拷贝/移动赋值运算符保证了成员的正确初始化和资源释放，无需自定义特殊成员函数。关于不变性，`message` 字段可以持有任何 UTF-8 或 ASCII 文本，不要求非空或特定格式，结构体本身不维护错误码、层级或上下文信息，其语义完全由外层提取逻辑解释。

#### Invariants

- `message` 成员始终包含有效的错误描述字符串。

#### Key Members

- `message`

#### Usage Patterns

- 作为 `clore::extract` 命名空间中错误处理的结果类型使用。

### `clore::extract::ASTResult`

Declaration: `extract/ast.cppm:37`

Definition: `extract/ast.cppm:37`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

结构体 `clore::extract::ASTResult` 是一个纯数据聚合体，其内部直接暴露三个公有成员：`symbols`（类型为 `std::vector<SymbolInfo>`）、`relations`（类型为 `std::vector<ExtractedRelation>`）以及 `dependencies`（类型为 `std::vector<std::string>`）。这些字段分别负责承载从 AST 中提取出的符号信息、元素间的关联关系以及外部依赖名列表。该结构体不包含任何自定义构造函数、析构函数或成员函数，其所有成员均可通过直接初始化或聚合初始化来赋值，因此实例化时无需特殊逻辑。

由于 `ASTResult` 仅依赖标准库容器，其内存布局完全由编译器决定，且不存在任何需要运行时维护的不变式。通常，各个字段会在提取流程中被顺序填充，并在最终使用时协同解释：`symbols` 与 `relations` 通过索引或 ID 相关联，而 `dependencies` 则独立提供后续编译或分析所需的依赖项集合。该结构体在设计上仅作为数据传输的中间容器，不涉及深拷贝控制或资源管理——由容器本身负责其内部元素的析构与移动语义。

#### Invariants

- 字段均为公共向量，可自由读写
- 无显式不变量约束

#### Key Members

- `symbols`
- `relations`
- `dependencies`

#### Usage Patterns

- 作为提取管道的输出类型
- 由分析函数填充并返回
- 消费者遍历各向量以获取提取结果

### `clore::extract::ExtractedRelation`

Declaration: `extract/ast.cppm:30`

Definition: `extract/ast.cppm:30`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`ExtractedRelation` 是一个仅包含数据的结构体，用于表示在 AST 提取过程中发现的一条二元关系。该结构体的核心字段为两个 `SymbolID` 类型成员 `from` 和 `to`，分别代表关系中的源和目标符号。另外两个布尔成员 `is_call` 和 `is_inheritance` 标记关系的具体语义：当 `is_call` 为真时，表示 `from` 调用了 `to`；当 `is_inheritance` 为真时，表示 `from` 派生于 `to`。虽然在设计上这两个标志通常不应同时为真，但结构体本身并不强制这一不变性，调用方应确保关系的类型是明确的。

#### Invariants

- `from` 和 `to` 必须为有效的 `SymbolID`
- `is_call` 和 `is_inheritance` 至少一个为 true 或均为 false

#### Key Members

- `from`：关系的源符号标识
- `to`：关系的目标符号标识
- `is_call`：是否为调用关系
- `is_inheritance`：是否为继承关系

#### Usage Patterns

- 用于构建符号之间的调用图
- 用于记录继承层次结构
- 在提取阶段填充此结构体以描述符号间依赖

## Functions

### `clore::extract::extract_symbols`

Declaration: `extract/ast.cppm:43`

Definition: `extract/ast.cppm:669`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数是符号提取的入口点，内部串联了编译器实例的创建、前端选项的强制调整（将`ProgramAction`设为`clang::frontend::ParseSyntaxOnly`并清空输出路径）、以及`SymbolExtractorAction`的调度执行。依赖`create_compiler_instance`构建`clang::CompilerInstance`，并使用`collect_dependency_files`从`SourceManager`提取文件依赖。控制流首先检查`entry.arguments`是否为空，然后依次调用`SymbolExtractorAction`的`BeginSourceFile`、`Execute`和`EndSourceFile`，每一步失败均返回`std::unexpected`。提取出的原生`RelationEdge`列表随后被转换为`ExtractedRelation`结构，填充`from`、`to`、`is_call`和`is_inheritance`字段，从而解耦内部枚举类型。

#### Side Effects

- Creates and mutates a compiler instance (via `create_compiler_instance` and frontend option modifications)
- Reads source files and includes via the clang frontend during extraction
- Populates the `ASTResult` output with symbols, relations, and dependency file paths
- Invokes `collect_dependency_files` which reads the source manager
- May produce diagnostics or cache results indirectly through clang infrastructure

#### Reads From

- The `entry` parameter of type `CompileEntry` (its `file`, `arguments` fields)
- The filesystem: source file and its transitive includes via clang
- Compiler configuration and toolchain (via `create_compiler_instance`)

#### Writes To

- The local `result` object of type `ASTResult` (its `symbols`, `dependencies`, `relations` sub-objects)
- The local `raw_relations` vector of `RelationEdge`
- The compiler instance's frontend options (`ProgramAction`, `OutputFile`, `ModuleOutputPath`)

#### Usage Patterns

- Used to extract symbols from a single compile entry
- Called as part of a larger extraction pipeline (e.g., `extract_project_async`)
- Returns an `expected` that callers handle for success or error

## Internal Structure

`extract:ast` 模块实现了从 Clang AST 中提取符号与关系的核心逻辑。它导入 `extract:compiler`（编译数据库）、`extract:model`（结果数据模型）、`support`（基础工具）和标准库。内部采用三层结构：底层为一组匿名命名空间中的辅助函数（如符号 ID 计算、声明分类、文档注释提取、签名构建、源码片段范界计算、依赖文件收集等）；中间层定义了 `SymbolExtractorVisitor`（基于 `RecursiveASTVisitor` 的遍历器，负责访问声明、调用、引用、成员表达式并记录符号与关系）、`SymbolExtractorConsumer`（接收翻译单元并驱动访问器）和 `SymbolExtractorAction`（触发入口）；顶层公开函数 `extract_symbols` 接收项目标识符并返回状态码，填充 `ASTResult` 结构（内含 `symbols`、`relations`、`dependencies` 字段）。

提取过程中，每个声明均通过 `compute_symbol_id` 获得整数 ID，依赖收集通过 `collect_dependency_files` 解析文件包含；关系类型使用 `RelationKind` 枚举（`Inheritance`、`Call`、`Reference`），并由 `RelationEdge` 与 `ExtractedRelation` 中间结构承载，最终归入 `ASTResult`。辅助结构 `SourceSnippetBounds` 与 `LexicalContextInfo` 分别用于记录源码区域和词法上下文。这种分解方式将 AST 遍历、元数据计算与数据汇总清晰分离，便于维护与测试。

## Related Pages

- [Module extract:compiler](compiler.md)
- [Module extract:model](model.md)
- [Module support](../support/index.md)

