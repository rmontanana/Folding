import re
from conan import ConanFile
from conan.tools.files import copy
from conan.tools.cmake import cmake_layout


class FoldingConan(ConanFile):
    name = "folding"
    version = "X.X.X"
    description = "K-Fold and stratified K-Fold header-only library"
    url = "https://github.com/rmontanana/folding"
    license = "MIT"
    homepage = "https://github.com/rmontanana/folding"
    topics = ("kfold", "stratified folding", "mdlp")
    no_copy_source = True
    exports_sources = "folding.hpp"
    package_type = "header-library"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("libtorch/2.7.0")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.15]")
        # Test dependencies
        self.test_requires("catch2/3.8.1")
        self.test_requires("arff-files/1.2.0")

    def layout(self):
        cmake_layout(self)

    def init(self):
        # Read the CMakeLists.txt file to get the version
        with open("folding.hpp", "r") as f:
            content = f.read()
            match = re.search(
                r'const std::string FOLDING_VERSION = "([^"]+)";', content
            )
            if match:
                self.version = match.group(1)

    def package(self):
        copy(self, "*.hpp", src=self.source_folder, dst=self.package_folder)

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
