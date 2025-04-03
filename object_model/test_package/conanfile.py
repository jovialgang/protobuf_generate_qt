from conans import ConanFile, CMake
import os

class TestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    build_requires = "build_tools/[>=2.7]@ntt/stable"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def test(self):
        os.chdir("bin")
        self.run(".%strivial_test" % os.sep)
