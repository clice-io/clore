#include "extract/tooling.h"

#include <memory>

#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/Utils.h"
#include "llvm/Support/VirtualFileSystem.h"

#include "extract/scan.h"

namespace clore::extract {

auto create_compiler_instance(const CompileEntry& entry,
                              bool suppress_diagnostics)
    -> std::unique_ptr<clang::CompilerInstance> {
    auto driver_args = sanitize_driver_arguments(entry);
    if(driver_args.empty()) {
        return nullptr;
    }

    llvm::SmallVector<const char*> argv;
    argv.reserve(driver_args.size() + 1);
    for(auto& arg : driver_args) {
        argv.push_back(arg.c_str());
    }
    argv.push_back(entry.file.c_str());

    llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> vfs =
        llvm::vfs::createPhysicalFileSystem();

    clang::DiagnosticOptions diag_opts;
    auto diagnostics = clang::CompilerInstance::createDiagnostics(
        *vfs, diag_opts, new clang::IgnoringDiagConsumer(), true);
    if(!diagnostics) {
        return nullptr;
    }

    std::unique_ptr<clang::CompilerInvocation> invocation;

    const bool is_cc1 = argv.size() >= 2 && llvm::StringRef(argv[1]) == "-cc1";
    if(is_cc1) {
        invocation = std::make_unique<clang::CompilerInvocation>();
        if(!clang::CompilerInvocation::CreateFromArgs(*invocation,
                                                      llvm::ArrayRef(argv).drop_front(2),
                                                      *diagnostics,
                                                      argv[0])) {
            return nullptr;
        }
    } else {
        clang::CreateInvocationOptions options{
            .Diags = diagnostics,
            .VFS = vfs,
            .ProbePrecompiled = false,
        };
        invocation = clang::createInvocation(argv, options);
        if(!invocation) {
            return nullptr;
        }
    }

    invocation->getFrontendOpts().DisableFree = false;
    invocation->getFileSystemOpts().WorkingDir = entry.directory;

    auto instance = std::make_unique<clang::CompilerInstance>(std::move(invocation));
    instance->createDiagnostics(*vfs, new clang::IgnoringDiagConsumer(), true);
    instance->getDiagnostics().setSuppressAllDiagnostics(suppress_diagnostics);
    instance->createFileManager(vfs);

    if(!instance->createTarget()) {
        return nullptr;
    }

    return instance;
}

}  // namespace clore::extract
