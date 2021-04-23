#include "pch.hpp"

#include "math/vec3.hpp"
#include "texture/image.hpp"
#include "texture/image_texture.hpp"

using namespace math;

int main() {
	std::cout << "Hello, world!" << std::endl;
	std::cout << std::filesystem::current_path().string() << std::endl;
	auto img = image::load("crafting_table_top.png", true);
	auto tex = std::make_shared<image_texture>(img);
	auto out_img = std::make_shared<image>(uvec2(512), 3, false, true);
	for (int x = 0; x < 512; x++) {
		for (int y = 0; y < 512; y++) {
			// fvec4 color = fvec4(x / 512.0F, 1 - (y / 512.0F), 0, 0);
			fvec4 color = tex->sample(fvec2(x / 512.0F, 1 - (y / 512.0F)));

			out_img->write(uvec2(x, y), 0, color.x);
			out_img->write(uvec2(x, y), 1, color.y);
			out_img->write(uvec2(x, y), 2, color.z);
		}
	}
	out_img->save("out.png");
}
