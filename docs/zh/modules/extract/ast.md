---
title: 'Module extract:ast'
description: 'extract:ast 模块负责在整个提取流程中执行 Clang AST 解析与符号提取。它接收来自编译数据库的单个编译条目，使用 Clang 的前端动作、AST 消费者和递归访问器遍历完整的函数、类、变量等声明，并收集它们的位置、签名、文档注释以及它们之间的调用、继承和引用关系。该模块的公开入口是 clore::extract::extract_symbols 函数，它接受一个表示编译条目的索引，返回一个 std::expected<ASTResult, ASTError>，其中包含提取出的符号列表、关系边以及收集到的依赖文件路径。模块内部定义了 SymbolExtractorVisitor、SymbolExtractorConsumer 和 SymbolExtractorAction 等实现细节，所有用户只应通过 extract_symbols 接口与模块交互。'
layout: doc
template: doc
---

# Module `extract:ast`

## Summary

`extract:ast` 模块负责在整个提取流程中执行 Clang AST 解析与符号提取。它接收来自编译数据库的单个编译条目，使用 Clang 的前端动作、AST 消费者和递归访问器遍历完整的函数、类、变量等声明，并收集它们的位置、签名、文档注释以及它们之间的调用、继承和引用关系。该模块的公开入口是 `clore::extract::extract_symbols` 函数，它接受一个表示编译条目的索引，返回一个 `std::expected<ASTResult, ASTError>`，其中包含提取出的符号列表、关系边以及收集到的依赖文件路径。模块内部定义了 `SymbolExtractorVisitor`、`SymbolExtractorConsumer` 和 `SymbolExtractorAction` 等实现细节，所有用户只应通过 `extract_symbols` 接口与模块交互。

## Imports

- [`extract:compiler`](compiler.md)
- [`extract:model`](model.md)
- [`support`](../support/index.md)

## Imported By

- [`extract:cache`](cache.md)

## Types

### `clore::extract::ASTError`

Declaration: `src/extract/ast.cppm:49`

Definition: `src/extract/ast.cppm:49`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

结构体 `clore::extract::ASTError` 是一个简单的聚合类型，内部仅包含一个 `std::string` 类型的 `message` 成员。没有用户声明的构造函数、析构函数或赋值运算符，因此编译器隐式生成默认的特殊成员函数，使得该结构体可被平凡地默认构造、拷贝和移动。由于所有成员操作都委托给 `std::string`，因此 `ASTError` 本身不持有额外的资源或管理逻辑，其生命周期和行为完全由 `message` 的复制与销毁驱动。该实现不维护复杂的不变量，主要依赖 `std::string` 内部的空状态检查与内存管理，适用场景为轻量级错误信息的存储与传递。

#### Invariants

- `message` 字段应包含描述性错误文本，但无格式或内容限制

#### Key Members

- `std::string message`：错误描述文本

#### Usage Patterns

- 作为错误信息载体，由 `clore::extract` 中的函数可能返回或填充
- 调用者可通过读取 `message` 获取错误详情

### `clore::extract::ASTResult`

Declaration: `src/extract/ast.cppm:60`

Definition: `src/extract/ast.cppm:60`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

结构体 `clore::extract::ASTResult` 是一个纯聚合数据结构，其内部直接包含三个 `std::vector` 成员：`symbols`（存储 `SymbolInfo`）、`relations`（存储 `ExtractedRelation`）以及 `dependencies`（存储 `std::string`）。在实现层面，提取函数会依次向这三个向量追加数据，使得 `symbols` 和 `relations` 各自保持构建时的插入顺序，而 `dependencies` 则记录外部的模块或文件依赖。这三个成员之间没有强制性的索引关联或唯一性约束，它们的填充依赖统一的遍历流程以保证结果集的语义一致性；调用方应假定所有向量在提取完成后同时包含完整且对应的信息。

#### Invariants

- Each `std::vector` member may be empty.
- No ordering guarantees are implied for any of the vectors.
- The struct is intended to be used as a value type.

#### Key Members

- `symbols`: extracted symbol information
- `relations`: relationships between symbols
- `dependencies`: file dependency strings

#### Usage Patterns

- Returned from extraction functions.
- Consumed to populate a project's symbol and relation database.
- Iterated to access extracted elements.

### `clore::extract::ExtractedRelation`

Declaration: `src/extract/ast.cppm:53`

Definition: `src/extract/ast.cppm:53`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::ExtractedRelation` 结构体内部仅包含四个公有数据成员：`from` 和 `to`（类型均为 `SymbolID`），以及 `is_call` 和 `is_inheritance`（均为 `bool`）。两个布尔标志采用默认值 `false`，分别标识该关系是否为调用边（`is_call == true`）或继承边（`is_inheritance == true`）。当 `is_inheritance` 为真时，约定 `from` 代表派生类标识符，`to` 代表基类标识符。该结构体不提供额外的构造函数或方法，其不变性隐含要求两个标志不同时为真，但未通过代码强制检查，完全依赖于使用方的正确设定。整份结构的设计聚焦于以最简形式承载一条已提取关系的元数据，为后续的图构建或查询提供基础数据单元。

#### Invariants

- `from` 和 `to` 应指向已注册的 `SymbolID`
- `is_call` 和 `is_inheritance` 默认均为 `false`，表示无特定关系类型
- 当 `is_inheritance` 为 `true` 时，`from` 表示派生类，`to` 表示基类

#### Key Members

- `from`
- `to`
- `is_call`
- `is_inheritance`

#### Usage Patterns

- 其他模块可以创建 `ExtractedRelation` 实例并填充其字段，用于记录调用关系或继承关系
- 关系图构建或分析代码可能读取这些标志来区分边的类型

## Functions

### `clore::extract::extract_symbols`

Declaration: `src/extract/ast.cppm:66`

Definition: `src/extract/ast.cppm:692`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数首先验证输入的 `CompileEntry` 是否包含空参数列表，若是则立即返回一个包含错误消息的 `ASTError`。随后创建一个 Clang 编译器实例并强制设置为纯语法分析模式（`clang::frontend::ParseSyntaxOnly`），同时清空输出路径以避免物化编译产物。接着构造一个 `SymbolExtractorAction` 对象，该对象持有指向输出容器 `result.symbols` 和 `raw_relations` 的引用；通过 `BeginSourceFile` 和 `Execute` 执行完整的 AST 遍历，若过程中出现错误则消耗 LLVM 的错误对象并返回 `ASTError`。成功执行后调用 `EndSourceFile`，并通过 `collect_dependency_files` 填充 `result.dependencies`。最后遍历 `raw_relations` 中的每条 `RelationEdge`，将其转换为 `ExtractedRelation`（根据 `kind` 设置 `is_call` 和 `is_inheritance` 标志）并存入 `result.relations`。整个流程依赖 Clang 的前端工具链、自定义的 AST Consumer 和 Visitor（位于匿名命名空间 `SymbolExtractorConsumer` 和 `SymbolExtractorVisitor`），以及辅助函数如 `create_compiler_instance` 和 `collect_dependency_files` 来完成符号提取和关系收集。

#### Side Effects

- creates a compiler instance via `create_compiler_instance`
- modifies frontend options to set `ParseSyntaxOnly`
- reads source files through the clang frontend during extraction
- populates `ASTResult` with symbols, relations, and dependency files

#### Reads From

- `entry` parameter (compile entry arguments and file path)
- source files and compilation database through the clang frontend

#### Writes To

- `result.symbols` via `SymbolExtractorAction`
- `raw_relations` vector
- `result.dependencies` via `collect_dependency_files`
- `result.relations` after transformation

#### Usage Patterns

- used to extract symbol information from a single compilation entry
- called as part of the symbol extraction pipeline

## Internal Structure

模块 `extract:ast` 负责从 C++ 源代码中提取符号定义及其关系，是整个提取流程的核心 AST 分析模块。其顶层入口 `extract_symbols` 接受一个表示编译条目的索引整型，并通过 Clang 的前端动作机制触发分析。内部结构分为三个主要层次：`SymbolExtractorAction` 作为 `ASTFrontendAction` 创建 `SymbolExtractorConsumer`；`SymbolExtractorConsumer` 在 `HandleTranslationUnit` 中实例化并运行 `SymbolExtractorVisitor`（继承自 `RecursiveASTVisitor`）；而 `SymbolExtractorVisitor` 通过重写 `VisitNamedDecl`、`VisitCallExpr`、`VisitMemberExpr` 等方法遍历 AST，对每个声明计算唯一符号 ID、提取名称、签名、源代码片段、词法上下文、文档注释等信息，并记录调用、引用、继承等关系。辅助函数（如 `compute_symbol_id`、`classify_decl`、`describe_lexical_context`、`get_source_snippet_bounds`）将原始 Clang 数据结构转换为内部定义的 `ExtractedRelation`、`RelationEdge`、`SourceSnippetBounds`、`LexicalContextInfo` 等结构体，最终汇总为 `ASTResult`（包含符号、关系、依赖文件列表）。模块通过导入 `extract:compiler` 获取编译选项和条目，使用 `extract:model` 定义输出类型，并依赖 `support` 模块提供的工具函数，实现了从编译数据库项到结构化符号信息的清晰分解。

## Related Pages

- [Module extract:compiler](compiler.md)
- [Module extract:model](model.md)
- [Module support](../support/index.md)

