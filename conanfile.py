from conans import ConanFile, CMake

class ConanFile(ConanFile):
	settings = "os", "compiler", "build_type", "arch"
	requires = "assimp/5.0.1"
	generators = "cmake_find_package"
	default_options = {
		'assimp:shared': True
	}

	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()
