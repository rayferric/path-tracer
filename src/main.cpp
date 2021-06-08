#include "pch.hpp"

#include "core/renderer.hpp"

// fvec3 tonemap_approx_aces(const fvec3 &hdr) {
// 	constexpr float a = 2.51F;
// 	const fvec3 b(0.03F);
// 	constexpr float c = 2.43F;
// 	const fvec3 d(0.59F);
// 	const fvec3 e(0.14F);
// 	return saturate((hdr * (a * hdr + b)) / (hdr * (c * hdr + d) + e));
// }

int main() {
	core::renderer renderer;
	renderer.resolution = math::uvec2(1024);
	
	renderer.load_gltf("assets/suzanne/suzanne.gltf");

	renderer.render("render.png");

	return 0;
}
