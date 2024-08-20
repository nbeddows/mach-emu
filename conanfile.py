from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.build import can_run
import os

class MachEmuRecipe(ConanFile):
    name = "mach_emu"
    version = "1.6.2"
    package_type = "library"
    test_package_folder = "tests/conan_package_test"

    # Optional metadata
    license = "MIT"
    author = "Nicolas Beddows <nicolas.beddows@gmail.com>"
    url = "https://github.com/nbeddows"
    description = "8 bit Machine Emulator Engine"
    topics = ("emulator", "i8080")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False], "with_i8080_test_suites": [True, False], "with_python": [True, False], "with_save": [True, False], "with_zlib": [True, False]}
    default_options = {"gtest*:build_gmock": False, "zlib*:shared": True, "shared": True, "fPIC": True, "with_i8080_test_suites": False, "with_python": False, "with_save": True, "with_zlib": True}

    # Sources are located in the same place as this recipe, copy them to the recipe
    # "tests/CMakeLists.txt",\
    exports_sources = "CMakeLists.txt",\
        "CHANGELOG.md",\
        "LICENSE.md",\
        "README.md",\
        "docs/*",\
        "include/*",\
        "resource/*",\
        "source/*",\
        "tests/pythonTestDeps.cmake",\
        "tests/programs/*",\
        "tests/include/*",\
        "tests/source/*",

    def requirements(self):
        if self.options.with_save:
            self.requires("base64/0.5.2")
            self.requires("hash-library/8.0")

        if(self.settings.os == "baremetal"):
            self.requires("arduinojson/7.0.1")
        else:
            self.requires("nlohmann_json/3.11.3")

        if self.options.get_safe("with_python", False):
            self.requires("pybind11/2.12.0")
        if self.options.get_safe("with_zlib", False):
            self.requires("zlib/1.3.1")

    def build_requirements(self):
        if not self.conf.get("tools.build:skip_test", default=False):
            self.test_requires("gtest/1.14.0")

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

        if "arm" in self.settings.arch:
            self.options.rm_safe("with_python")

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

        if not self.options.with_save:
            self.options.rm_safe("with_zlib")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.cache_variables["enable_python_module"] = self.options.get_safe("with_python", False)
        tc.cache_variables["enable_zlib"] = self.options.get_safe("with_zlib", False)
        tc.cache_variables["enable_base64"] = self.options.with_save
        tc.cache_variables["enable_hash_library"] = self.options.with_save
        tc.variables["build_os"] = self.settings.os
        tc.variables["build_arch"] = self.settings.arch
        tc.variables["archive_dir"] = self.cpp_info.libdirs[0]
        tc.variables["runtime_dir"] = self.cpp_info.bindirs[0]
        if self.settings.os == "Windows" and self.options.get_safe("with_zlib", False) and self.dependencies["zlib"].options.shared:
            tc.variables["zlib_bin_dir"] = self.dependencies["zlib"].cpp_info.bindirs[0].replace("\\", "/")
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

        if can_run(self) and not self.conf.get("tools.build:skip_test", default=False):
            testFilter = "--gtest_filter=*"
            if not self.options.with_i8080_test_suites:
                testFilter += ":-*8080*:*CpuTest*"
            testsDir = os.path.join(self.source_folder, "artifacts", str(self.settings.build_type), str(self.settings.arch), self.cpp_info.bindirs[0])
            self.run(os.path.join(testsDir, "mach_emu_test " + testFilter + " " + os.path.join(self.source_folder + "/tests/programs/")))
            if self.options.get_safe("with_python", False):
                testFilter = "-k "
                if self.options.with_i8080_test_suites:
                    testFilter += "*"
                else:
                    testFilter += "MachineTest"
                cmd = os.path.join(self.source_folder, "tests/source/meen_test/test_Machine.py -v " + testFilter)
                self.run("python " + cmd)

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = [self.name]
