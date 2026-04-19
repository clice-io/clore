import os
from pathlib import Path

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMakeToolchain, cmake_layout
from conan.tools.scm import Git


class Clore(ConanFile):
    name = "clore"
    version = "0.1.0"
    package_type = "application"
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps"

    def layout(self):
        cmake_layout(self, src_folder=".")
        self.kotatsu_path = os.path.join("build", "subprojects", "kotatsu")

    def requirements(self):
        self.requires("spdlog/1.17.0")
        self.requires("libcurl/8.19.0")
        self.requires("simdjson/4.6.1")

    def source(self):
        git = Git(self)
        if not os.path.exists(self.kotatsu_path):
            git.clone("https://github.com/clice-io/kotatsu", self.kotatsu_path)

    def setup_llvm(self, tc: CMakeToolchain, include_dir: Path, library_dir: Path, llvm_shared_name: str, clang_cpp_name: str):
        llvm_shared_library = (library_dir / llvm_shared_name).as_posix()
        clang_cpp_library = (library_dir / clang_cpp_name).as_posix()

        missing = [path for path in (llvm_shared_library, clang_cpp_library) if not Path(path).is_file()]
        if missing:
            raise ConanInvalidConfiguration(
                "missing required llvm/clang libraries in pixi environment: "
                + ", ".join(missing)
            )
        tc.variables["CLORE_LLVM_INCLUDE_DIR"] = include_dir.as_posix()
        tc.variables["CLORE_LLVM_LIBRARY_DIR"] = library_dir.as_posix()
        tc.variables["CLORE_LLVM_SHARED_LIBRARY"] = llvm_shared_library
        tc.variables["CLORE_CLANG_CPP_LIBRARY"] = clang_cpp_library

    def generate(self):
        tc = CMakeToolchain(self)
        pixi_prefix = os.getenv("CONDA_PREFIX")
        if pixi_prefix:
            prefix = Path(pixi_prefix).resolve()
            clean_pixi = prefix.as_posix()
            llvm_cmake_dir = f"{clean_pixi}/lib/cmake/llvm"
            conan_gen_dir = self.generators_folder.replace("\\", "/")
            tc.variables["CMAKE_PREFIX_PATH"] = f"{llvm_cmake_dir};{clean_pixi};{conan_gen_dir}"
            tc.variables["CMAKE_C_COMPILER"] = f"{clean_pixi}/bin/clang"
            tc.variables["CMAKE_CXX_COMPILER"] = f"{clean_pixi}/bin/clang++"
            tc.variables["CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS"] = (
                f"{clean_pixi}/bin/clang-scan-deps"
            )
            tc.preprocessor_definitions["_LIBCPP_DISABLE_AVAILABILITY"] = "1"
            tc.variables["CLORE_LLVM_PREFIX"] = clean_pixi
            system_name = str(self.settings.os)
            if system_name == "Linux":
                self.setup_llvm(
                    tc=tc,
                    include_dir=prefix / "include",
                    library_dir=prefix / "lib",
                    llvm_shared_name="libLLVM.so",
                    clang_cpp_name="libclang-cpp.so",
                )
            elif system_name == "Macos":
                self.setup_llvm(
                    tc=tc,
                    include_dir=prefix / "include",
                    library_dir=prefix / "lib",
                    llvm_shared_name="libLLVM.dylib",
                    clang_cpp_name="libclang-cpp.dylib",
                )
            elif system_name == "Windows":
                self.setup_llvm(
                    tc=tc,
                    include_dir=prefix / "Library" / "include",
                    library_dir=prefix / "Library" / "lib",
                    llvm_shared_name="LLVM.lib",
                    clang_cpp_name="clang-cpp.lib",
                )
        tc.variables["KOTATSU"] = self.kotatsu_path
        tc.generate()
