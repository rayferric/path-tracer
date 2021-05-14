#include "pch.hpp"

#include "math/mat3.hpp"
#include "renderer/model.hpp"
#include "renderer/triangle.hpp"
#include "scene/entity.hpp"
#include "texture/image_texture.hpp"
#include "scene/load_gltf.hpp"

using namespace math;
using namespace renderer;
using namespace scene;

int main() {
	try {
	std::cout << "Hello, world!\n";
	std::shared_ptr<entity> root = load_gltf("assets/suzanne.gltf");
	std::cout << root->get_components().size() << std::endl;
	} catch (std::exception &e) {
		std::cout << e.what();
	}

	// ray r(
	// 	fvec3(0, 0, 1),
	// 	fvec3(0, 0, -1)
	// );

	// auto img = std::make_shared<image>(uvec2(1280, 720), 3, false, true);
	
	// for (uint32_t x = 0; x < 1280; x++) {
	// 	for (uint32_t y = 0; y < 720; y++) {
	// 		fvec2 coord = fvec2(x / 1280.0F, y / 720.0F) * 2 - fvec2(1);
	// 		coord.y = - coord.y;
	// 		r.set_dir(fvec3(coord.x, coord.y, -1));

	// 		auto intersection = t.intersect(r);

	// 		if (intersection.hit) {
	// 			img->write(uvec2(x, y), 0, 1.0F);
	// 			img->write(uvec2(x, y), 1, 1.0F);
	// 			img->write(uvec2(x, y), 2, 1.0F);
	// 		} else {
	// 			img->write(uvec2(x, y), 0, 0.0F);
	// 			img->write(uvec2(x, y), 1, 0.0F);
	// 			img->write(uvec2(x, y), 2, 0.0F);
	// 		}
	// 	}
	// }

	// img->save("render.png");

	return 0;
}
