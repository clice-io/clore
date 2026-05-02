---
title: 'Namespace clore'
description: 'clore 命名空间的核心职责是提供一组用于配置装饰选项（decorative options）的工具，并支持相关的任务管理、日志记录与结果同步操作。其主体结构是 clore::Options，这是一个聚合容器，内部定义了多个以 _DecoOptStruct_ 为前缀的嵌套结构体，每个结构体代表一组可定制的装饰策略，并通过 Step、Action、result_type、_deco_base_t 和 _deco_callback_base_t 等类型别名细化每个选项的行为和结果类型。这种分层设计使调用方能够以模板或函数参数的形式统一管理多组装饰选项，实现灵活的策略组合。'
layout: doc
template: doc
---

# Namespace `clore`

## Summary

`clore` 命名空间的核心职责是提供一组用于配置装饰选项（decorative options）的工具，并支持相关的任务管理、日志记录与结果同步操作。其主体结构是 `clore::Options`，这是一个聚合容器，内部定义了多个以 `_DecoOptStruct_` 为前缀的嵌套结构体，每个结构体代表一组可定制的装饰策略，并通过 `Step`、`Action`、`result_type`、`_deco_base_t` 和 `_deco_callback_base_t` 等类型别名细化每个选项的行为和结果类型。这种分层设计使调用方能够以模板或函数参数的形式统一管理多组装饰选项，实现灵活的策略组合。

除了配置组件，`clore` 还暴露了两个关键的自由函数：`log_generation_summary` 用于记录与生成操作相关的总结信息（接受一个整数引用），以及 `await_task_result` 用于同步等待任务完成并获取结果（返回 `std::expected<Value, Error>`）。这些函数与 `Options` 结构体共同构成了 `clore` 库在装饰场景下的配置、执行与监控闭环，其设计强调类型安全、非侵入式日志以及基于 `std::expected` 的错误处理。

## Diagram

```mermaid
graph TD
    NS["clore"]
    T0["Options"]
    NS --> T0
    T1["_DecoOptStruct_0"]
    NS --> T1
    T2["__deco_field_ty"]
    NS --> T2
    T3["Action"]
    NS --> T3
    T4["Step"]
    NS --> T4
    T5["_deco_base_t"]
    NS --> T5
    T6["_deco_callback_base_t"]
    NS --> T6
    T7["result_type"]
    NS --> T7
    T8["_deco_base_t"]
    NS --> T8
    T9["_DecoOptStruct_1"]
    NS --> T9
    T10["_DecoOptStruct_10"]
    NS --> T10
    T11["__deco_field_ty"]
    NS --> T11
    T12["_deco_base_t"]
    NS --> T12
    T13["_deco_base_t"]
    NS --> T13
    T14["__deco_field_ty"]
    NS --> T14
    T15["Action"]
    NS --> T15
    T16["Step"]
    NS --> T16
    T17["_deco_base_t"]
    NS --> T17
    T18["_deco_callback_base_t"]
    NS --> T18
    T19["result_type"]
    NS --> T19
    T20["_deco_base_t"]
    NS --> T20
    T21["_DecoOptStruct_2"]
    NS --> T21
    T22["__deco_field_ty"]
    NS --> T22
    T23["Action"]
    NS --> T23
    T24["Step"]
    NS --> T24
    T25["_deco_base_t"]
    NS --> T25
    T26["_deco_callback_base_t"]
    NS --> T26
    T27["result_type"]
    NS --> T27
    T28["_deco_base_t"]
    NS --> T28
    T29["_DecoOptStruct_3"]
    NS --> T29
    T30["__deco_field_ty"]
    NS --> T30
    T31["Action"]
    NS --> T31
    T32["Step"]
    NS --> T32
    T33["_deco_base_t"]
    NS --> T33
    T34["_deco_callback_base_t"]
    NS --> T34
    T35["result_type"]
    NS --> T35
    T36["_deco_base_t"]
    NS --> T36
    T37["_DecoOptStruct_4"]
    NS --> T37
    T38["__deco_field_ty"]
    NS --> T38
    T39["Action"]
    NS --> T39
    T40["Step"]
    NS --> T40
    T41["_deco_base_t"]
    NS --> T41
    T42["_deco_callback_base_t"]
    NS --> T42
    T43["result_type"]
    NS --> T43
    T44["_deco_base_t"]
    NS --> T44
    T45["_DecoOptStruct_5"]
    NS --> T45
    T46["__deco_field_ty"]
    NS --> T46
    T47["Action"]
    NS --> T47
    T48["Step"]
    NS --> T48
    T49["_deco_base_t"]
    NS --> T49
    T50["_deco_callback_base_t"]
    NS --> T50
    T51["result_type"]
    NS --> T51
    T52["_deco_base_t"]
    NS --> T52
    T53["_DecoOptStruct_6"]
    NS --> T53
    T54["__deco_field_ty"]
    NS --> T54
    T55["Action"]
    NS --> T55
    T56["Step"]
    NS --> T56
    T57["_deco_base_t"]
    NS --> T57
    T58["_deco_callback_base_t"]
    NS --> T58
    T59["result_type"]
    NS --> T59
    T60["_deco_base_t"]
    NS --> T60
    T61["_DecoOptStruct_7"]
    NS --> T61
    T62["__deco_field_ty"]
    NS --> T62
    T63["_deco_base_t"]
    NS --> T63
    T64["_deco_base_t"]
    NS --> T64
    T65["_DecoOptStruct_8"]
    NS --> T65
    T66["__deco_field_ty"]
    NS --> T66
    T67["_deco_base_t"]
    NS --> T67
    T68["_deco_base_t"]
    NS --> T68
    T69["_DecoOptStruct_9"]
    NS --> T69
    T70["__deco_field_ty"]
    NS --> T70
    T71["_deco_base_t"]
    NS --> T71
    T72["_deco_base_t"]
    NS --> T72
    NSC0["agent"]
    NS --> NSC0
    NSC1["config"]
    NS --> NSC1
    NSC2["extract"]
    NS --> NSC2
    NSC3["generate"]
    NS --> NSC3
    NSC4["logging"]
    NS --> NSC4
    NSC5["net"]
    NS --> NSC5
    NSC6["support"]
    NS --> NSC6
```

## Subnamespaces

- [`clore::agent`](agent/index.md)
- [`clore::config`](config/index.md)
- [`clore::extract`](extract/index.md)
- [`clore::generate`](generate/index.md)
- [`clore::logging`](logging/index.md)
- [`clore::net`](net/index.md)
- [`clore::support`](support/index.md)

## Types

### `clore::Options`

Declaration: `main.cpp:18`

Definition: `main.cpp:18`

`clore::Options` 结构体是一个聚合容器，用于配置与装饰选项相关的参数。其内部定义了一系列以 `_DecoOptStruct_` 开头的嵌套结构体，每个结构体进一步细化了特定选项的细节，包括 `Step`、`Action`、`result_type` 以及回调基类型（例如 `_deco_callback_base_t`）。通过组合这些内部类型，`clore::Options` 允许用户在多组装饰选项之间进行选择，并控制每种选项所关联的行为和结果类型。在 `clore` 库中，该结构体通常作为模板参数或函数参数传递，以统一管理一组可定制的装饰策略。

#### Invariants

- 所有选项均为可选，无必填项
- `DecoKV` 宏声明的选项支持 `KVStyle::JoinedOrSeparate` 风格
- `log_level` 的有效值受限于其帮助文本中列出的级别
- `rate_limit` 仅在 `--model` 使用时有效

#### Key Members

- `config`
- `compile_commands`
- `source_dir`
- `output_dir`
- `log_level`
- `model`
- `rate_limit`
- `dry_run`
- `agent_mode`
- `help`
- `version`

#### Usage Patterns

- 通过宏定义的元数据自动解析命令行参数并填充字段
- 其他代码可读取 `Options` 实例以获取用户指定的配置路径、日志级别、模型名称等
- `dry_run` 用于控制是否跳过 LLM 请求，`agent_mode` 启用自动化探索模式

#### Member Types

##### `clore::Options::_DecoOptStruct_0`

Declaration: `main.cpp:19`

Definition: `main.cpp:19`

###### Member Types

###### `clore::Options::_DecoOptStruct_0::__deco_field_ty`

Declaration: `main.cpp:19`

Definition: `main.cpp:19`

###### Member Types

###### `clore::Options::_DecoOptStruct_0::__deco_field_ty::Action`

Declaration: `main.cpp:19`

###### `clore::Options::_DecoOptStruct_0::__deco_field_ty::Step`

Declaration: `main.cpp:19`

###### `clore::Options::_DecoOptStruct_0::__deco_field_ty::_deco_base_t`

Declaration: `main.cpp:19`

###### `clore::Options::_DecoOptStruct_0::__deco_field_ty::_deco_callback_base_t`

Declaration: `main.cpp:19`

###### `clore::Options::_DecoOptStruct_0::__deco_field_ty::result_type`

Declaration: `main.cpp:19`

###### Member Functions

###### `clore::Options::_DecoOptStruct_0::__deco_field_ty::__deco_field_ty`

Declaration: `main.cpp:19`

Definition: `main.cpp:19`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_0::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_0::_deco_base_t`

Declaration: `main.cpp:19`

###### Member Variables

###### `clore::Options::_DecoOptStruct_0::deco_field_ty`

Declaration: `main.cpp:19`

###### Member Functions

###### `clore::Options::_DecoOptStruct_0::~_DecoOptStruct_0<ResTy>`

Declaration: `main.cpp:19`

Definition: `main.cpp:19`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_0::~_DecoOptStruct_0<ResTy>();
```

##### `clore::Options::_DecoOptStruct_1`

Declaration: `main.cpp:24`

Definition: `main.cpp:24`

###### Member Types

###### `clore::Options::_DecoOptStruct_1::__deco_field_ty`

Declaration: `main.cpp:24`

Definition: `main.cpp:24`

###### Member Types

###### `clore::Options::_DecoOptStruct_1::__deco_field_ty::Action`

Declaration: `main.cpp:24`

###### `clore::Options::_DecoOptStruct_1::__deco_field_ty::Step`

Declaration: `main.cpp:24`

###### `clore::Options::_DecoOptStruct_1::__deco_field_ty::_deco_base_t`

Declaration: `main.cpp:24`

###### `clore::Options::_DecoOptStruct_1::__deco_field_ty::_deco_callback_base_t`

Declaration: `main.cpp:24`

###### `clore::Options::_DecoOptStruct_1::__deco_field_ty::result_type`

Declaration: `main.cpp:24`

###### Member Functions

###### `clore::Options::_DecoOptStruct_1::__deco_field_ty::__deco_field_ty`

Declaration: `main.cpp:24`

Definition: `main.cpp:24`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_1::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_1::_deco_base_t`

Declaration: `main.cpp:24`

###### Member Variables

###### `clore::Options::_DecoOptStruct_1::deco_field_ty`

Declaration: `main.cpp:24`

###### Member Functions

###### `clore::Options::_DecoOptStruct_1::~_DecoOptStruct_1<ResTy>`

Declaration: `main.cpp:24`

Definition: `main.cpp:24`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_1::~_DecoOptStruct_1<ResTy>();
```

##### `clore::Options::_DecoOptStruct_10`

Declaration: `main.cpp:69`

Definition: `main.cpp:69`

###### Member Types

###### `clore::Options::_DecoOptStruct_10::__deco_field_ty`

Declaration: `main.cpp:69`

Definition: `main.cpp:69`

###### Member Types

###### `clore::Options::_DecoOptStruct_10::__deco_field_ty::_deco_base_t`

Declaration: `main.cpp:69`

###### Member Functions

###### `clore::Options::_DecoOptStruct_10::__deco_field_ty::__deco_field_ty`

Declaration: `main.cpp:69`

Definition: `main.cpp:69`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_10::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_10::_deco_base_t`

Declaration: `main.cpp:69`

###### Member Functions

###### `clore::Options::_DecoOptStruct_10::~_DecoOptStruct_10`

Declaration: `main.cpp:69`

Definition: `main.cpp:69`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_10::~_DecoOptStruct_10();
```

##### `clore::Options::_DecoOptStruct_2`

Declaration: `main.cpp:29`

Definition: `main.cpp:29`

###### Member Types

###### `clore::Options::_DecoOptStruct_2::__deco_field_ty`

Declaration: `main.cpp:29`

Definition: `main.cpp:29`

###### Member Types

###### `clore::Options::_DecoOptStruct_2::__deco_field_ty::Action`

Declaration: `main.cpp:29`

###### `clore::Options::_DecoOptStruct_2::__deco_field_ty::Step`

Declaration: `main.cpp:29`

###### `clore::Options::_DecoOptStruct_2::__deco_field_ty::_deco_base_t`

Declaration: `main.cpp:29`

###### `clore::Options::_DecoOptStruct_2::__deco_field_ty::_deco_callback_base_t`

Declaration: `main.cpp:29`

###### `clore::Options::_DecoOptStruct_2::__deco_field_ty::result_type`

Declaration: `main.cpp:29`

###### Member Functions

###### `clore::Options::_DecoOptStruct_2::__deco_field_ty::__deco_field_ty`

Declaration: `main.cpp:29`

Definition: `main.cpp:29`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_2::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_2::_deco_base_t`

Declaration: `main.cpp:29`

###### Member Variables

###### `clore::Options::_DecoOptStruct_2::deco_field_ty`

Declaration: `main.cpp:29`

###### Member Functions

###### `clore::Options::_DecoOptStruct_2::~_DecoOptStruct_2<ResTy>`

Declaration: `main.cpp:29`

Definition: `main.cpp:29`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_2::~_DecoOptStruct_2<ResTy>();
```

##### `clore::Options::_DecoOptStruct_3`

Declaration: `main.cpp:34`

Definition: `main.cpp:34`

###### Member Types

###### `clore::Options::_DecoOptStruct_3::__deco_field_ty`

Declaration: `main.cpp:34`

Definition: `main.cpp:34`

###### Member Types

###### `clore::Options::_DecoOptStruct_3::__deco_field_ty::Action`

Declaration: `main.cpp:34`

###### `clore::Options::_DecoOptStruct_3::__deco_field_ty::Step`

Declaration: `main.cpp:34`

###### `clore::Options::_DecoOptStruct_3::__deco_field_ty::_deco_base_t`

Declaration: `main.cpp:34`

###### `clore::Options::_DecoOptStruct_3::__deco_field_ty::_deco_callback_base_t`

Declaration: `main.cpp:34`

###### `clore::Options::_DecoOptStruct_3::__deco_field_ty::result_type`

Declaration: `main.cpp:34`

###### Member Functions

###### `clore::Options::_DecoOptStruct_3::__deco_field_ty::__deco_field_ty`

Declaration: `main.cpp:34`

Definition: `main.cpp:34`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_3::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_3::_deco_base_t`

Declaration: `main.cpp:34`

###### Member Variables

###### `clore::Options::_DecoOptStruct_3::deco_field_ty`

Declaration: `main.cpp:34`

###### Member Functions

###### `clore::Options::_DecoOptStruct_3::~_DecoOptStruct_3<ResTy>`

Declaration: `main.cpp:34`

Definition: `main.cpp:34`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_3::~_DecoOptStruct_3<ResTy>();
```

##### `clore::Options::_DecoOptStruct_4`

Declaration: `main.cpp:37`

Definition: `main.cpp:37`

###### Member Types

###### `clore::Options::_DecoOptStruct_4::__deco_field_ty`

Declaration: `main.cpp:37`

Definition: `main.cpp:37`

###### Member Types

###### `clore::Options::_DecoOptStruct_4::__deco_field_ty::Action`

Declaration: `main.cpp:37`

###### `clore::Options::_DecoOptStruct_4::__deco_field_ty::Step`

Declaration: `main.cpp:37`

###### `clore::Options::_DecoOptStruct_4::__deco_field_ty::_deco_base_t`

Declaration: `main.cpp:37`

###### `clore::Options::_DecoOptStruct_4::__deco_field_ty::_deco_callback_base_t`

Declaration: `main.cpp:37`

###### `clore::Options::_DecoOptStruct_4::__deco_field_ty::result_type`

Declaration: `main.cpp:37`

###### Member Functions

###### `clore::Options::_DecoOptStruct_4::__deco_field_ty::__deco_field_ty`

Declaration: `main.cpp:37`

Definition: `main.cpp:37`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_4::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_4::_deco_base_t`

Declaration: `main.cpp:37`

###### Member Variables

###### `clore::Options::_DecoOptStruct_4::deco_field_ty`

Declaration: `main.cpp:37`

###### Member Functions

###### `clore::Options::_DecoOptStruct_4::~_DecoOptStruct_4<ResTy>`

Declaration: `main.cpp:37`

Definition: `main.cpp:37`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_4::~_DecoOptStruct_4<ResTy>();
```

##### `clore::Options::_DecoOptStruct_5`

Declaration: `main.cpp:43`

Definition: `main.cpp:43`

###### Member Types

###### `clore::Options::_DecoOptStruct_5::__deco_field_ty`

Declaration: `main.cpp:43`

Definition: `main.cpp:43`

###### Member Types

###### `clore::Options::_DecoOptStruct_5::__deco_field_ty::Action`

Declaration: `main.cpp:43`

###### `clore::Options::_DecoOptStruct_5::__deco_field_ty::Step`

Declaration: `main.cpp:43`

###### `clore::Options::_DecoOptStruct_5::__deco_field_ty::_deco_base_t`

Declaration: `main.cpp:43`

###### `clore::Options::_DecoOptStruct_5::__deco_field_ty::_deco_callback_base_t`

Declaration: `main.cpp:43`

###### `clore::Options::_DecoOptStruct_5::__deco_field_ty::result_type`

Declaration: `main.cpp:43`

###### Member Functions

###### `clore::Options::_DecoOptStruct_5::__deco_field_ty::__deco_field_ty`

Declaration: `main.cpp:43`

Definition: `main.cpp:43`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_5::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_5::_deco_base_t`

Declaration: `main.cpp:43`

###### Member Variables

###### `clore::Options::_DecoOptStruct_5::deco_field_ty`

Declaration: `main.cpp:43`

###### Member Functions

###### `clore::Options::_DecoOptStruct_5::~_DecoOptStruct_5<ResTy>`

Declaration: `main.cpp:43`

Definition: `main.cpp:43`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_5::~_DecoOptStruct_5<ResTy>();
```

##### `clore::Options::_DecoOptStruct_6`

Declaration: `main.cpp:49`

Definition: `main.cpp:49`

###### Member Types

###### `clore::Options::_DecoOptStruct_6::__deco_field_ty`

Declaration: `main.cpp:49`

Definition: `main.cpp:49`

###### Member Types

###### `clore::Options::_DecoOptStruct_6::__deco_field_ty::Action`

Declaration: `main.cpp:49`

###### `clore::Options::_DecoOptStruct_6::__deco_field_ty::Step`

Declaration: `main.cpp:49`

###### `clore::Options::_DecoOptStruct_6::__deco_field_ty::_deco_base_t`

Declaration: `main.cpp:49`

###### `clore::Options::_DecoOptStruct_6::__deco_field_ty::_deco_callback_base_t`

Declaration: `main.cpp:49`

###### `clore::Options::_DecoOptStruct_6::__deco_field_ty::result_type`

Declaration: `main.cpp:49`

###### Member Functions

###### `clore::Options::_DecoOptStruct_6::__deco_field_ty::__deco_field_ty`

Declaration: `main.cpp:49`

Definition: `main.cpp:49`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_6::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_6::_deco_base_t`

Declaration: `main.cpp:49`

###### Member Variables

###### `clore::Options::_DecoOptStruct_6::deco_field_ty`

Declaration: `main.cpp:49`

###### Member Functions

###### `clore::Options::_DecoOptStruct_6::~_DecoOptStruct_6<ResTy>`

Declaration: `main.cpp:49`

Definition: `main.cpp:49`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_6::~_DecoOptStruct_6<ResTy>();
```

##### `clore::Options::_DecoOptStruct_7`

Declaration: `main.cpp:55`

Definition: `main.cpp:55`

###### Member Types

###### `clore::Options::_DecoOptStruct_7::__deco_field_ty`

Declaration: `main.cpp:55`

Definition: `main.cpp:55`

###### Member Types

###### `clore::Options::_DecoOptStruct_7::__deco_field_ty::_deco_base_t`

Declaration: `main.cpp:55`

###### Member Functions

###### `clore::Options::_DecoOptStruct_7::__deco_field_ty::__deco_field_ty`

Declaration: `main.cpp:55`

Definition: `main.cpp:55`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_7::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_7::_deco_base_t`

Declaration: `main.cpp:55`

###### Member Functions

###### `clore::Options::_DecoOptStruct_7::~_DecoOptStruct_7`

Declaration: `main.cpp:55`

Definition: `main.cpp:55`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_7::~_DecoOptStruct_7();
```

##### `clore::Options::_DecoOptStruct_8`

Declaration: `main.cpp:60`

Definition: `main.cpp:60`

###### Member Types

###### `clore::Options::_DecoOptStruct_8::__deco_field_ty`

Declaration: `main.cpp:60`

Definition: `main.cpp:60`

###### Member Types

###### `clore::Options::_DecoOptStruct_8::__deco_field_ty::_deco_base_t`

Declaration: `main.cpp:60`

###### Member Functions

###### `clore::Options::_DecoOptStruct_8::__deco_field_ty::__deco_field_ty`

Declaration: `main.cpp:60`

Definition: `main.cpp:60`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_8::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_8::_deco_base_t`

Declaration: `main.cpp:60`

###### Member Functions

###### `clore::Options::_DecoOptStruct_8::~_DecoOptStruct_8`

Declaration: `main.cpp:60`

Definition: `main.cpp:60`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_8::~_DecoOptStruct_8();
```

##### `clore::Options::_DecoOptStruct_9`

Declaration: `main.cpp:66`

Definition: `main.cpp:66`

###### Member Types

###### `clore::Options::_DecoOptStruct_9::__deco_field_ty`

Declaration: `main.cpp:66`

Definition: `main.cpp:66`

###### Member Types

###### `clore::Options::_DecoOptStruct_9::__deco_field_ty::_deco_base_t`

Declaration: `main.cpp:66`

###### Member Functions

###### `clore::Options::_DecoOptStruct_9::__deco_field_ty::__deco_field_ty`

Declaration: `main.cpp:66`

Definition: `main.cpp:66`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_9::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_9::_deco_base_t`

Declaration: `main.cpp:66`

###### Member Functions

###### `clore::Options::_DecoOptStruct_9::~_DecoOptStruct_9`

Declaration: `main.cpp:66`

Definition: `main.cpp:66`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_9::~_DecoOptStruct_9();
```

## Functions

### `clore::await_task_result`

Declaration: `main.cpp:92`

Definition: `main.cpp:92`

`clore::await_task_result` 是一个模板函数，用于同步等待一个任务完成并获取其结果。它接受一个 `Task` 类型的引用和一个 `std::string_view` 标签（通常用于日志或错误上下文），并返回一个 `std::expected<Value, Error>`。调用方必须确保 `Task` 类型满足可等待语义（例如具有 `value_type` 和 `error_type` 成员），且 `Value` 与 `Error` 与任务的结果类型兼容。函数不会抛出异常；所有错误都通过 `std::expected` 的 `unexpected` 状态传达，调用方应检查返回值以处理成功或失败的情况。

#### Usage Patterns

- retrieving result after awaiting task completion
- handling task cancellation and errors in a unified way
- converting task exceptions into expected error types

### `clore::log_generation_summary`

Declaration: `main.cpp:73`

Definition: `main.cpp:73`

函数 `clore::log_generation_summary` 接受一个 `const int &` 类型的参数，返回 `void`。调用者应提供一个整数引用，该整数通常表示某种生成操作的结果或计数；函数负责记录或输出与该生成相关的总结信息。调用者无需关心具体的日志格式或目标，只需保证提供的整数值在函数执行期间保持有效即可。

#### Usage Patterns

- Called after generation to log a summary of generated pages and cache performance
- Typically invoked with the result of a generation process

## Related Pages

- [Namespace clore::agent](agent/index.md)
- [Namespace clore::config](config/index.md)
- [Namespace clore::extract](extract/index.md)
- [Namespace clore::generate](generate/index.md)
- [Namespace clore::logging](logging/index.md)
- [Namespace clore::net](net/index.md)
- [Namespace clore::support](support/index.md)

