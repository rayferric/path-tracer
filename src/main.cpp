#include "pch.hpp"

#include "core/pbr.hpp"
#include "core/renderer.hpp"
#include "image/image_texture.hpp"

using namespace math;

int main() {
	// // Interview code
	// core::renderer renderer;
	// renderer.sample_count = 10000;
	// renderer.bounce_count = 4;
	// renderer.resolution = uvec2(1024);
	// renderer.environment_factor = fvec3::zero;
	// renderer.load_gltf("assets/cornell-box/cornell-box.gltf");
	// renderer.render("renders/cornell-box-alt.png");
	
	core::renderer renderer;
	renderer.sample_count = 10000;
	renderer.bounce_count = 4;
	renderer.resolution = uvec2(1920, 1080);
	// renderer.resolution = uvec2(640, 360);
	// renderer.resolution = uvec2(2048);
	// renderer.resolution = uvec2(1024);
	// renderer.thread_count = 1;

	// auto hdri = image::image::load("assets/cityscape.hdr", false);
	// auto hdri = image::image::load("assets/dirt-road.hdr", false);
	// renderer.environment = std::make_shared<image::image_texture>(hdri);
	// renderer.environment_factor = fvec3(1.3, 3.6, 10);
	renderer.environment_factor = fvec3::zero;
	// renderer.environment_factor = fvec3(3);
	renderer.transparent_background = true;

	// renderer.camera_index = 1;
	// renderer.sun_light_index = 0;

	renderer.load_gltf("assets/test/test.gltf");

	// renderer.sun_light->angular_radius = 0.2F; // Low poly shadow fix
	// renderer.sun_light->angular_radius = 0.05F; // Porshe sun 

	renderer.render("renders/test.png");
	
	// // Render visualization of kd-tree levels
	// for (uint32_t i = 0; i < 20; i++) {
	// 	renderer.visualize_kd_tree_depth = i;
	// 	renderer.render("renders/.dev-cerberus-kd/" + util::to_string(i) + ".png");
	// }

	return 0;
}
