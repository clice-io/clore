import os
from pathlib import Path

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, cmake_layout


class Clore(ConanFile):
    name = "clore"
    version = "0.1.0"
    package_type = "application"
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps", "CMakeToolchain"

    def layout(self):
        cmake_layout(self, src_folder=".")

    def requirements(self):
        self.requires("spdlog/1.17.0")
        self.requires("libcurl/8.19.0")