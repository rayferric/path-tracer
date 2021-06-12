#include "pch.hpp"

#include "core/pbr.hpp"
#include "core/renderer.hpp"
#include "image/image_texture.hpp"

using namespace math;

int main() {
	core::renderer renderer;
	renderer.sample_count = 100;

	auto hdri = image::image::load("assets/helipad.hdr", false);
	renderer.environment = std::make_shared<image::image_texture>(hdri);
	
	renderer.load_gltf("assets/dragon/dragon.gltf");

	renderer.render("render.png");

	return 0;
}
