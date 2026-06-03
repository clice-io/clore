---
title: 'Namespace clore'
description: 'The clore namespace provides the core functionality for a task-oriented generation framework. It exposes two primary free functions: await_task_result for synchronously awaiting a task’s completion and obtaining its success or error result, and log_generation_summary for recording a high-level summary of a completed generation cycle. The namespace also contains the Options class, which internally defines a large number of nested decorator‑option structures (_DecoOptStruct_0 through _DecoOptStruct_10) to support a flexible, declarative configuration system. These internal types are implementation details that likely implement a policy‑based or decorator‑based pattern for customizing generation or task behavior. Architecturally, clore serves as the top‑level library namespace, encapsulating both the public API for task management and the complex option‑handling machinery behind a single coherent interface.'
layout: doc
template: doc
---

# Namespace `clore`

## Summary

The `clore` namespace provides the core functionality for a task-oriented generation framework. It exposes two primary free functions: `await_task_result` for synchronously awaiting a task’s completion and obtaining its success or error result, and `log_generation_summary` for recording a high-level summary of a completed generation cycle. The namespace also contains the `Options` class, which internally defines a large number of nested decorator‑option structures (`_DecoOptStruct_0` through `_DecoOptStruct_10`) to support a flexible, declarative configuration system. These internal types are implementation details that likely implement a policy‑based or decorator‑based pattern for customizing generation or task behavior. Architecturally, `clore` serves as the top‑level library namespace, encapsulating both the public API for task management and the complex option‑handling machinery behind a single coherent interface.

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

Declaration: `src/main.cpp:36`

Definition: `src/main.cpp:36`

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- All fields are optional; required is false
- Default values are handled by the decoration system
- Flags are declared with `DecoFlag` and key-value options with `DecoKV`

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

- Populated by a command-line parser generated from the decoration macros
- Checked in `main()` to configure logging, I/O paths, and execution mode
- Used to decide whether to show help, version, or perform dry-run

#### Member Types

##### `clore::Options::_DecoOptStruct_0`

Declaration: `src/main.cpp:37`

Definition: `src/main.cpp:37`

###### Member Types

###### `clore::Options::_DecoOptStruct_0::__deco_field_ty`

Declaration: `src/main.cpp:37`

Definition: `src/main.cpp:37`

###### Member Types

###### `clore::Options::_DecoOptStruct_0::__deco_field_ty::Action`

Declaration: `src/main.cpp:37`

###### `clore::Options::_DecoOptStruct_0::__deco_field_ty::Step`

Declaration: `src/main.cpp:37`

###### `clore::Options::_DecoOptStruct_0::__deco_field_ty::_deco_base_t`

Declaration: `src/main.cpp:37`

###### `clore::Options::_DecoOptStruct_0::__deco_field_ty::_deco_callback_base_t`

Declaration: `src/main.cpp:37`

###### `clore::Options::_DecoOptStruct_0::__deco_field_ty::result_type`

Declaration: `src/main.cpp:37`

###### Member Functions

###### `clore::Options::_DecoOptStruct_0::__deco_field_ty::__deco_field_ty`

Declaration: `src/main.cpp:37`

Definition: `src/main.cpp:37`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_0::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_0::_deco_base_t`

Declaration: `src/main.cpp:37`

###### Member Variables

###### `clore::Options::_DecoOptStruct_0::deco_field_ty`

Declaration: `src/main.cpp:37`

###### Member Functions

###### `clore::Options::_DecoOptStruct_0::~_DecoOptStruct_0<ResTy>`

Declaration: `src/main.cpp:37`

Definition: `src/main.cpp:37`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_0::~_DecoOptStruct_0<ResTy>();
```

##### `clore::Options::_DecoOptStruct_1`

Declaration: `src/main.cpp:42`

Definition: `src/main.cpp:42`

###### Member Types

###### `clore::Options::_DecoOptStruct_1::__deco_field_ty`

Declaration: `src/main.cpp:42`

Definition: `src/main.cpp:42`

###### Member Types

###### `clore::Options::_DecoOptStruct_1::__deco_field_ty::Action`

Declaration: `src/main.cpp:42`

###### `clore::Options::_DecoOptStruct_1::__deco_field_ty::Step`

Declaration: `src/main.cpp:42`

###### `clore::Options::_DecoOptStruct_1::__deco_field_ty::_deco_base_t`

Declaration: `src/main.cpp:42`

###### `clore::Options::_DecoOptStruct_1::__deco_field_ty::_deco_callback_base_t`

Declaration: `src/main.cpp:42`

###### `clore::Options::_DecoOptStruct_1::__deco_field_ty::result_type`

Declaration: `src/main.cpp:42`

###### Member Functions

###### `clore::Options::_DecoOptStruct_1::__deco_field_ty::__deco_field_ty`

Declaration: `src/main.cpp:42`

Definition: `src/main.cpp:42`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_1::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_1::_deco_base_t`

Declaration: `src/main.cpp:42`

###### Member Variables

###### `clore::Options::_DecoOptStruct_1::deco_field_ty`

Declaration: `src/main.cpp:42`

###### Member Functions

###### `clore::Options::_DecoOptStruct_1::~_DecoOptStruct_1<ResTy>`

Declaration: `src/main.cpp:42`

Definition: `src/main.cpp:42`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_1::~_DecoOptStruct_1<ResTy>();
```

##### `clore::Options::_DecoOptStruct_10`

Declaration: `src/main.cpp:87`

Definition: `src/main.cpp:87`

###### Member Types

###### `clore::Options::_DecoOptStruct_10::__deco_field_ty`

Declaration: `src/main.cpp:87`

Definition: `src/main.cpp:87`

###### Member Types

###### `clore::Options::_DecoOptStruct_10::__deco_field_ty::_deco_base_t`

Declaration: `src/main.cpp:87`

###### Member Functions

###### `clore::Options::_DecoOptStruct_10::__deco_field_ty::__deco_field_ty`

Declaration: `src/main.cpp:87`

Definition: `src/main.cpp:87`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_10::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_10::_deco_base_t`

Declaration: `src/main.cpp:87`

###### Member Functions

###### `clore::Options::_DecoOptStruct_10::~_DecoOptStruct_10`

Declaration: `src/main.cpp:87`

Definition: `src/main.cpp:87`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_10::~_DecoOptStruct_10();
```

##### `clore::Options::_DecoOptStruct_2`

Declaration: `src/main.cpp:47`

Definition: `src/main.cpp:47`

###### Member Types

###### `clore::Options::_DecoOptStruct_2::__deco_field_ty`

Declaration: `src/main.cpp:47`

Definition: `src/main.cpp:47`

###### Member Types

###### `clore::Options::_DecoOptStruct_2::__deco_field_ty::Action`

Declaration: `src/main.cpp:47`

###### `clore::Options::_DecoOptStruct_2::__deco_field_ty::Step`

Declaration: `src/main.cpp:47`

###### `clore::Options::_DecoOptStruct_2::__deco_field_ty::_deco_base_t`

Declaration: `src/main.cpp:47`

###### `clore::Options::_DecoOptStruct_2::__deco_field_ty::_deco_callback_base_t`

Declaration: `src/main.cpp:47`

###### `clore::Options::_DecoOptStruct_2::__deco_field_ty::result_type`

Declaration: `src/main.cpp:47`

###### Member Functions

###### `clore::Options::_DecoOptStruct_2::__deco_field_ty::__deco_field_ty`

Declaration: `src/main.cpp:47`

Definition: `src/main.cpp:47`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_2::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_2::_deco_base_t`

Declaration: `src/main.cpp:47`

###### Member Variables

###### `clore::Options::_DecoOptStruct_2::deco_field_ty`

Declaration: `src/main.cpp:47`

###### Member Functions

###### `clore::Options::_DecoOptStruct_2::~_DecoOptStruct_2<ResTy>`

Declaration: `src/main.cpp:47`

Definition: `src/main.cpp:47`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_2::~_DecoOptStruct_2<ResTy>();
```

##### `clore::Options::_DecoOptStruct_3`

Declaration: `src/main.cpp:52`

Definition: `src/main.cpp:52`

###### Member Types

###### `clore::Options::_DecoOptStruct_3::__deco_field_ty`

Declaration: `src/main.cpp:52`

Definition: `src/main.cpp:52`

###### Member Types

###### `clore::Options::_DecoOptStruct_3::__deco_field_ty::Action`

Declaration: `src/main.cpp:52`

###### `clore::Options::_DecoOptStruct_3::__deco_field_ty::Step`

Declaration: `src/main.cpp:52`

###### `clore::Options::_DecoOptStruct_3::__deco_field_ty::_deco_base_t`

Declaration: `src/main.cpp:52`

###### `clore::Options::_DecoOptStruct_3::__deco_field_ty::_deco_callback_base_t`

Declaration: `src/main.cpp:52`

###### `clore::Options::_DecoOptStruct_3::__deco_field_ty::result_type`

Declaration: `src/main.cpp:52`

###### Member Functions

###### `clore::Options::_DecoOptStruct_3::__deco_field_ty::__deco_field_ty`

Declaration: `src/main.cpp:52`

Definition: `src/main.cpp:52`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_3::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_3::_deco_base_t`

Declaration: `src/main.cpp:52`

###### Member Variables

###### `clore::Options::_DecoOptStruct_3::deco_field_ty`

Declaration: `src/main.cpp:52`

###### Member Functions

###### `clore::Options::_DecoOptStruct_3::~_DecoOptStruct_3<ResTy>`

Declaration: `src/main.cpp:52`

Definition: `src/main.cpp:52`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_3::~_DecoOptStruct_3<ResTy>();
```

##### `clore::Options::_DecoOptStruct_4`

Declaration: `src/main.cpp:55`

Definition: `src/main.cpp:55`

###### Member Types

###### `clore::Options::_DecoOptStruct_4::__deco_field_ty`

Declaration: `src/main.cpp:55`

Definition: `src/main.cpp:55`

###### Member Types

###### `clore::Options::_DecoOptStruct_4::__deco_field_ty::Action`

Declaration: `src/main.cpp:55`

###### `clore::Options::_DecoOptStruct_4::__deco_field_ty::Step`

Declaration: `src/main.cpp:55`

###### `clore::Options::_DecoOptStruct_4::__deco_field_ty::_deco_base_t`

Declaration: `src/main.cpp:55`

###### `clore::Options::_DecoOptStruct_4::__deco_field_ty::_deco_callback_base_t`

Declaration: `src/main.cpp:55`

###### `clore::Options::_DecoOptStruct_4::__deco_field_ty::result_type`

Declaration: `src/main.cpp:55`

###### Member Functions

###### `clore::Options::_DecoOptStruct_4::__deco_field_ty::__deco_field_ty`

Declaration: `src/main.cpp:55`

Definition: `src/main.cpp:55`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_4::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_4::_deco_base_t`

Declaration: `src/main.cpp:55`

###### Member Variables

###### `clore::Options::_DecoOptStruct_4::deco_field_ty`

Declaration: `src/main.cpp:55`

###### Member Functions

###### `clore::Options::_DecoOptStruct_4::~_DecoOptStruct_4<ResTy>`

Declaration: `src/main.cpp:55`

Definition: `src/main.cpp:55`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_4::~_DecoOptStruct_4<ResTy>();
```

##### `clore::Options::_DecoOptStruct_5`

Declaration: `src/main.cpp:61`

Definition: `src/main.cpp:61`

###### Member Types

###### `clore::Options::_DecoOptStruct_5::__deco_field_ty`

Declaration: `src/main.cpp:61`

Definition: `src/main.cpp:61`

###### Member Types

###### `clore::Options::_DecoOptStruct_5::__deco_field_ty::Action`

Declaration: `src/main.cpp:61`

###### `clore::Options::_DecoOptStruct_5::__deco_field_ty::Step`

Declaration: `src/main.cpp:61`

###### `clore::Options::_DecoOptStruct_5::__deco_field_ty::_deco_base_t`

Declaration: `src/main.cpp:61`

###### `clore::Options::_DecoOptStruct_5::__deco_field_ty::_deco_callback_base_t`

Declaration: `src/main.cpp:61`

###### `clore::Options::_DecoOptStruct_5::__deco_field_ty::result_type`

Declaration: `src/main.cpp:61`

###### Member Functions

###### `clore::Options::_DecoOptStruct_5::__deco_field_ty::__deco_field_ty`

Declaration: `src/main.cpp:61`

Definition: `src/main.cpp:61`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_5::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_5::_deco_base_t`

Declaration: `src/main.cpp:61`

###### Member Variables

###### `clore::Options::_DecoOptStruct_5::deco_field_ty`

Declaration: `src/main.cpp:61`

###### Member Functions

###### `clore::Options::_DecoOptStruct_5::~_DecoOptStruct_5<ResTy>`

Declaration: `src/main.cpp:61`

Definition: `src/main.cpp:61`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_5::~_DecoOptStruct_5<ResTy>();
```

##### `clore::Options::_DecoOptStruct_6`

Declaration: `src/main.cpp:67`

Definition: `src/main.cpp:67`

###### Member Types

###### `clore::Options::_DecoOptStruct_6::__deco_field_ty`

Declaration: `src/main.cpp:67`

Definition: `src/main.cpp:67`

###### Member Types

###### `clore::Options::_DecoOptStruct_6::__deco_field_ty::Action`

Declaration: `src/main.cpp:67`

###### `clore::Options::_DecoOptStruct_6::__deco_field_ty::Step`

Declaration: `src/main.cpp:67`

###### `clore::Options::_DecoOptStruct_6::__deco_field_ty::_deco_base_t`

Declaration: `src/main.cpp:67`

###### `clore::Options::_DecoOptStruct_6::__deco_field_ty::_deco_callback_base_t`

Declaration: `src/main.cpp:67`

###### `clore::Options::_DecoOptStruct_6::__deco_field_ty::result_type`

Declaration: `src/main.cpp:67`

###### Member Functions

###### `clore::Options::_DecoOptStruct_6::__deco_field_ty::__deco_field_ty`

Declaration: `src/main.cpp:67`

Definition: `src/main.cpp:67`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_6::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_6::_deco_base_t`

Declaration: `src/main.cpp:67`

###### Member Variables

###### `clore::Options::_DecoOptStruct_6::deco_field_ty`

Declaration: `src/main.cpp:67`

###### Member Functions

###### `clore::Options::_DecoOptStruct_6::~_DecoOptStruct_6<ResTy>`

Declaration: `src/main.cpp:67`

Definition: `src/main.cpp:67`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_6::~_DecoOptStruct_6<ResTy>();
```

##### `clore::Options::_DecoOptStruct_7`

Declaration: `src/main.cpp:73`

Definition: `src/main.cpp:73`

###### Member Types

###### `clore::Options::_DecoOptStruct_7::__deco_field_ty`

Declaration: `src/main.cpp:73`

Definition: `src/main.cpp:73`

###### Member Types

###### `clore::Options::_DecoOptStruct_7::__deco_field_ty::_deco_base_t`

Declaration: `src/main.cpp:73`

###### Member Functions

###### `clore::Options::_DecoOptStruct_7::__deco_field_ty::__deco_field_ty`

Declaration: `src/main.cpp:73`

Definition: `src/main.cpp:73`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_7::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_7::_deco_base_t`

Declaration: `src/main.cpp:73`

###### Member Functions

###### `clore::Options::_DecoOptStruct_7::~_DecoOptStruct_7`

Declaration: `src/main.cpp:73`

Definition: `src/main.cpp:73`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_7::~_DecoOptStruct_7();
```

##### `clore::Options::_DecoOptStruct_8`

Declaration: `src/main.cpp:78`

Definition: `src/main.cpp:78`

###### Member Types

###### `clore::Options::_DecoOptStruct_8::__deco_field_ty`

Declaration: `src/main.cpp:78`

Definition: `src/main.cpp:78`

###### Member Types

###### `clore::Options::_DecoOptStruct_8::__deco_field_ty::_deco_base_t`

Declaration: `src/main.cpp:78`

###### Member Functions

###### `clore::Options::_DecoOptStruct_8::__deco_field_ty::__deco_field_ty`

Declaration: `src/main.cpp:78`

Definition: `src/main.cpp:78`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_8::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_8::_deco_base_t`

Declaration: `src/main.cpp:78`

###### Member Functions

###### `clore::Options::_DecoOptStruct_8::~_DecoOptStruct_8`

Declaration: `src/main.cpp:78`

Definition: `src/main.cpp:78`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_8::~_DecoOptStruct_8();
```

##### `clore::Options::_DecoOptStruct_9`

Declaration: `src/main.cpp:84`

Definition: `src/main.cpp:84`

###### Member Types

###### `clore::Options::_DecoOptStruct_9::__deco_field_ty`

Declaration: `src/main.cpp:84`

Definition: `src/main.cpp:84`

###### Member Types

###### `clore::Options::_DecoOptStruct_9::__deco_field_ty::_deco_base_t`

Declaration: `src/main.cpp:84`

###### Member Functions

###### `clore::Options::_DecoOptStruct_9::__deco_field_ty::__deco_field_ty`

Declaration: `src/main.cpp:84`

Definition: `src/main.cpp:84`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_9::__deco_field_ty::__deco_field_ty();
```

###### `clore::Options::_DecoOptStruct_9::_deco_base_t`

Declaration: `src/main.cpp:84`

###### Member Functions

###### `clore::Options::_DecoOptStruct_9::~_DecoOptStruct_9`

Declaration: `src/main.cpp:84`

Definition: `src/main.cpp:84`

###### Declaration

```cpp
clore::Options::_DecoOptStruct_9::~_DecoOptStruct_9();
```

## Functions

### `clore::await_task_result`

Declaration: `src/main.cpp:110`

Definition: `src/main.cpp:110`

The function `clore::await_task_result` synchronously waits for the completion of a given task and returns its outcome. It accepts a `Task` object by reference and a `std::string_view` argument, which typically serves as a descriptive label or context for logging or error reporting. The call blocks until the task finishes, then yields a `std::expected<Value, Error>`: on success, the expected contains the task’s `value_type` result; on failure, it holds the `error_type`. The template parameters `Value` and `Error` default to `Task::value_type` and `Task::error_type` respectively, so the caller can often omit them as long as the task type provides these member types. The contract requires that `Task` satisfies these type aliases and is awaitable; the caller is responsible for ensuring the task is not already consumed or moved.

#### Usage Patterns

- safely extract result from a task
- handle incomplete or cancelled tasks
- convert exceptions to expected errors

### `clore::log_generation_summary`

Declaration: `src/main.cpp:91`

Definition: `src/main.cpp:91`

The `clore::log_generation_summary` function logs a high‑level summary of a completed generation cycle associated with the given integer identifier. Callers provide a constant reference to an integer value that identifies which generation’s summary should be recorded. The function does not return a value; its sole responsibility is to output the summary (typically to a log or console). It should be invoked after the corresponding generation work has finished to capture the result.

#### Usage Patterns

- Called after a generation phase to log results and cache performance
- Used to report summary metrics to the user or log file

## Related Pages

- [Namespace clore::agent](agent/index.md)
- [Namespace clore::config](config/index.md)
- [Namespace clore::extract](extract/index.md)
- [Namespace clore::generate](generate/index.md)
- [Namespace clore::logging](logging/index.md)
- [Namespace clore::net](net/index.md)
- [Namespace clore::support](support/index.md)

