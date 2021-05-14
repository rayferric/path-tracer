import os
from conans import ConanFile, CMake

class ConanFile(ConanFile):
	settings = "os", "compiler", "build_type", "arch"
	requires = "assimp/5.0.1"
	generators = "cmake_find_package"
	default_options = {
		'assimp:shared': True
	}

	def imports(self):
		dest = os.getenv('CONAN_IMPORT_PATH', 'bin')
		self.copy('*.dll', dst=dest, src='bin')

	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()
