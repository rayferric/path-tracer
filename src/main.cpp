#include "pch.hpp"

#include "geometry/aabb.hpp"
#include "geometry/triangle.hpp"
#include "math/vec3.hpp"
#include "texture/image.hpp"
#include "texture/image_texture.hpp"

using namespace math;

int main() {
	// auto img = image::load("crafting_table_top.png", true);
	// auto tex = std::make_shared<image_texture>(img);
	// auto out_img = std::make_shared<image>(uvec2(512), 3, false, true);
	// for (int x = 0; x < 512; x++) {
	// 	for (int y = 0; y < 512; y++) {
	// 		// fvec4 color = fvec4(x / 512.0F, 1 - (y / 512.0F), 0, 0);
	// 		fvec4 color = tex->sample(fvec2(x / 512.0F, 1 - (y / 512.0F)));

	// 		out_img->write(uvec2(x, y), 0, color.x);
	// 		out_img->write(uvec2(x, y), 1, color.y);
	// 		out_img->write(uvec2(x, y), 2, color.z);
	// 	}
	// }
	// out_img->save("out.png");

	aabb box(
		math::fvec3(-1, -1, -1),
		math::fvec3(1, 1, 1)
	);

	ray r = {
		math::fvec3(0),
		math::normalize(math::fvec3(1, 1, 1))
	};

	aabb::intersection result = box.intersect(r);

	std::cout << result.hit << std::endl;
	std::cout << result.distance << std::endl;
}
