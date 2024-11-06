import os
import json
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
import semver


class LibMicroEmu(ConanFile):
    name = "libmicroemu"
    url = ""
    author = "Christian Gröling"
    copyright = "MIT"
    description = ""
    homepage = ""
    topics = ("embedded", "parameter", "data model")
    settings = "os", "compiler", "build_type", "arch"
    exports_sources = (
        "inc*",
        "src*",
        "CMakeLists.txt",
    )
    options = {
        "build_tests": [True, False],
        "build_microemu": [True, False],
    }
    default_options = {
        "build_tests": False,
        "build_microemu": False,
    }

    def requirements(self):
        # only require fmt, cxxopts, spdlog, gtest if not doxygen_only
        if not self.conf.get("user.build:docs_only", default=False):
            self.requires("fmt/10.2.1")
            self.requires("cxxopts/3.2.0")
            self.requires("spdlog/1.14.1")
            self.requires("gtest/[*]")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.variables["BUILD_TESTING"] = "ON" if self.options.build_tests else "OFF"
        tc.variables["BUILD_MICROEMU"] = "ON" if self.options.build_microemu else "OFF"
        tc.generate()

    def build_requirements(self):
        pass

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
