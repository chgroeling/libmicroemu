import os
import json
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
import semver


class LibMicroEmu(ConanFile):
    name = "microemu"
    url = ""
    author: "cgg"
    copyright = "Intern"
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
        "cpptest": [True, False],
        "doxygen": [True, False],
    }
    default_options = {
        "build_tests": False,
        "build_microemu": False,
        "cpptest": False,
        "doxygen": False,
    }

    def requirements(self):
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
        self.tool_requires("cmake/3.23.5")
        self.tool_requires("make/4.4.1")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
