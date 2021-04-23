#include <iostream>

#include "vec3.hpp"
#include "image.hpp"

int main() {
	std::cout << "Hello, world!" << std::endl;
	std::cout << std::filesystem::current_path().string() << std::endl;
	auto img = image::load("test.png", true);
	img->write(uvec2(1, 0), 1, 0.1F);
	std::cout << img->read(uvec2(0, 0), 0) << std::endl;
	std::cout << img->read(uvec2(1, 0), 1) << std::endl;
	std::cout << img->read(uvec2(0, 1), 2) << std::endl;
}
