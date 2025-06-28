import re
from conan import ConanFile
from conan.tools.files import copy
from conan.tools.cmake import CMakeToolchain


class FoldingConan(ConanFile):
    name = "folding"
    version = "X.X.X"
    description = "K-Fold and stratified K-Fold header-only library"
    url = "https://github.com/rmontanana/folding"
    license = "MIT"
    homepage = "https://github.com/rmontanana/ArffFiles"
    topics = ("kfold", "stratified folding")
    no_copy_source = True
    exports_sources = "folding.hpp"
    package_type = "header-library"
    # build_type = "Debug"

    def requirements(self):
        # Build dependency
        self.requires("libtorch/2.7.0")

    def build_requirements(self):
        # Test dependencies
        self.test_requires("catch2/3.8.1")
        self.test_requires("arff-files/1.2.0")
        self.test_requires("fimdlp/2.0.1")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

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
        copy(self, "*.hpp", self.source_folder, self.package_folder)

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
