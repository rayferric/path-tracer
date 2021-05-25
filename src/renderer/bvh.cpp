#include "renderer/bvh.hpp"

namespace renderer {

bvh_node::intersection bvh_branch::intersect(const ray &ray) {
	float ldist = left  ? left->aabb.intersect(ray)  : -1;
	float rdist = right ? right->aabb.intersect(ray) : -1;

	bvh_node::intersection result, r1, r2;

	if (ldist >= 0)
		r1 = left->intersect(ray);
	if (rdist >= 0)
		r2 = right->intersect(ray);

	if (r1.hit_data.distance >= 0)
		result = r1;
	if (r2.hit_data.distance >= 0) {
		if (result.hit_data.distance < 0 || r2.hit_data.distance < result.hit_data.distance)
			result = r2;
	}

	// if (ldist < rdist) {
	// 	if (ldist >= 0)
	// 		result = left->intersect(ray);
	// 	if (result.hit_data.distance < 0 && rdist >= 0) {
	// 		result = right->intersect(ray);
	// 	}
	// } else {
	// 	if (rdist >= 0)
	// 		result = right->intersect(ray);
	// 	if (result.hit_data.distance < 0 && ldist >= 0)
	// 		result = left->intersect(ray);
	// }

	return result;
}

bvh_node::intersection bvh_leaf::intersect(const ray &ray) {
	triangle::intersection hit_data;
	uint32_t index;

	for (uint32_t i = 0; i < triangles.size(); i++) {
		auto result = triangles[i].intersect(ray);
		if (result.distance >= 0 && (result.distance <
				hit_data.distance || hit_data.distance < 0)) {
			hit_data = result;
			index = i;
		}
	}

	return { hit_data, index };
}

}
