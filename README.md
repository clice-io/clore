# clore

![C++ Standard](https://img.shields.io/badge/C++-23-blue.svg)
[![GitHub license](https://img.shields.io/github/license/clice-io/clore)](https://github.com/clice-io/clore/blob/main/LICENSE)
[![Build](https://github.com/clice-io/clore/actions/workflows/verify-build.yml/badge.svg)](https://github.com/clice-io/clore/actions/workflows/verify-build.yml)
[![Documentation](https://img.shields.io/badge/view-documentation-blue)](https://docs.clice.io/clore/)

clore is a document generator for humans and agents that combines LLVM and LLM technologies and is written in modern C++.

## How to build

Run `pixi run build`. If you don't have pixi installed, install it first.

The default build path now installs third-party dependencies via Conan before CMake configure.

```bash
pixi run build --type Debug
```

If you want to run the Conan step explicitly:

```bash
pixi run -e conan conan-install --type Debug
pixi run cmake-config --type Debug
pixi run cmake-build --type Debug
```

Current Conan-managed third-party dependencies are `spdlog`, `libcurl`, `simdjson`, and `kotatsu`.

LLVM remains on the existing managed artifact flow (`scripts/setup-llvm.py` + `config/llvm-manifest.json`) for gradual migration.

Your Conan profile should match the compiler/toolchain selected by `pixi`, because the project still configures CMake through `cmake/toolchain.cmake`.

## CLI usage

```bash
clore \
  --config clore.toml \
  --compile-commands build/compile_commands.json \
  --source-dir . \
  --output-dir docs/api \
  --dry-run
```

### Arguments

- `--config`: path to config file (default: `clore.toml`)
- `--compile-commands`: path to `compile_commands.json` (required)
- `--source-dir`: source root directory (required)
- `--output-dir`: output root directory (required)
- `--dry-run`: generate prompt-assembled docs without LLM calls
- `--model`: model name for online generation (mutually exclusive with `--dry-run`)
- `--rate-limit`: max concurrent LLM requests when `--model` is used (default: `16`)
- `--log-level`: override log level (`trace|debug|info|warn|error|off`)

## `clore.toml`

```toml
[filter]
include = ["src/"]
exclude = []

[llm]
provider = "openai"
system_prompt = "You are a C++ documentation writer."
retry_count = 3
retry_initial_backoff_ms = 250
```
