#pragma once

#include <memory>

#include "clang/Frontend/CompilerInstance.h"

#include "extract/compdb.h"

namespace clore::extract {

auto create_compiler_instance(const CompileEntry& entry,
                              bool suppress_diagnostics = true)
    -> std::unique_ptr<clang::CompilerInstance>;

}  // namespace clore::extract
