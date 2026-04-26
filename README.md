# clore

![C++ Standard](https://img.shields.io/badge/C++-23-blue.svg)
[![GitHub license](https://img.shields.io/github/license/clice-io/clore)](https://github.com/clice-io/clore/blob/main/LICENSE)
[![Build](https://github.com/clice-io/clore/actions/workflows/verify-build.yml/badge.svg)](https://github.com/clice-io/clore/actions/workflows/verify-build.yml)
[![Documentation](https://img.shields.io/badge/view-documentation-blue)](https://docs.clice.io/clore/)

clore is a document generator for humans and agents that combines LLVM and LLM technologies and is written in modern C++.

## How to build

Run `pixi run build`. If you don't have pixi installed, [install it first](https://pixi.sh/).

The default build path uses the `pixi` environment for both the LLVM/Clang toolchain and third-party dependencies before CMake configure. This applies on every supported platform, including Windows.

```bash
pixi run build RelWithDebInfo
```

If you want to run each step explicitly:

```bash
pixi run config RelWithDebInfo
pixi run check RelWithDebInfo
pixi run format
pixi run build RelWithDebInfo
```

LLVM, Clang, `spdlog`, and `libcurl` are provided by the `pixi` environment (conda-forge packages such as `clang`, `clangdev`, `llvmdev`, `spdlog`, and `libcurl`). On Windows the activated toolchain uses `clang.exe`, `clang++.exe`, and `llvm-rc.exe`.

## CLI usage

### Standard mode

Generate documentation from a compilation database. In standard mode, exactly one of `--dry-run` or `--model` is required:

```bash
clore \
  --config clore.en.toml \
  --compile-commands build/RelWithDebInfo/compile_commands.json \
  --source-dir . \
  --output-dir docs/en \
  --model deepseek-chat \
  --rate-limit 16
```

### Agent mode

Enable autonomous codebase exploration and guide generation:

```bash
clore \
  --config clore.en.toml \
  --experimental-agent-mode \
  --compile-commands build/RelWithDebInfo/compile_commands.json \
  --source-dir . \
  --output-dir docs/en \
  --model deepseek-chat \
  --rate-limit 16
```

### Arguments

| Flag | Description |
| --- | --- |
| `--config` | Path to config file; for typical use, start from `clore.en.toml` or `clore.zh.toml` and pass that path here |
| `--compile-commands` | Path to `compile_commands.json` |
| `--source-dir` | Source root directory for relative output paths |
| `--output-dir` | Output root directory |
| `--dry-run` | Write assembled prompts to `--output-dir` and skip LLM requests |
| `--model` | Model name for online generation (e.g. `deepseek-chat`) |
| `--rate-limit` | Maximum concurrent LLM requests when `--model` is used |
| `--experimental-agent-mode` | Enable agent-driven autonomous exploration and guide generation |
| `--log-level` | Override log level (`trace\|debug\|info\|warn\|error\|off`) |
| `-h`, `--help` | Show help message |
| `-v`, `--version` | Show version |

## Configuration

Configuration files support localization. Use `clore.en.toml` or `clore.zh.toml` as a starting point.

### `clore.en.toml`

```toml
[filter]
include = ["src/"]
exclude = []

[llm]
system_prompt = "You are a C++ documentation writer. Please generate concise and accurate documentation for C++ code elements. Focus on the purpose, design intent, and role of each element within the codebase. The output documentation must be in Markdown format and suitable for embedding within a document page. Use English in the generated documentation."
retry_limit = 20
```

### LLM provider selection

Provider selection is automatic at the network layer:

- use Anthropic when both `ANTHROPIC_BASE_URL` and `ANTHROPIC_API_KEY` are set
- otherwise use OpenAI when both `OPENAI_BASE_URL` and `OPENAI_API_KEY` are set
