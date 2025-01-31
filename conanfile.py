from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake


class ConanClass(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("boost/1.86.0")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generator = "Ninja"
