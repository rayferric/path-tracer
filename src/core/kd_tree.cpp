#include "core/kd_tree.hpp"

#include "math/vec3.hpp"
#include "math/vec4.hpp"

using namespace math;

namespace core {

// kd_tree_node::intersection kd_tree_branch::intersect(const ray &ray) {
// 	kd_tree_node::intersection result, r1, r2;

// 	float ldist = left  ? left->aabb.intersect(ray)  : -1;
// 	float rdist = right ? right->aabb.intersect(ray) : -1;

// 	// if (ldist > 0) r1 = left->intersect(ray);
// 	// if (rdist > 0) r2 = right->intersect(ray);

// 	// if (r1.distance < 0) return r2;
// 	// if (r2.distance < 0) return r1;

// 	// return r1.distance < r2.distance ? r1 : r2;

// 	if (ldist >= 0) {
// 		if (rdist >= 0) {
// 			if (ldist < rdist) {
// 				result = left->intersect(ray);
// 				if (result.distance < 0)
// 					result = right->intersect(ray);
// 			} else {
// 				result = right->intersect(ray);
// 				if (result.distance < 0)
// 					result = left->intersect(ray);
// 			}
// 		} else
// 			result = left->intersect(ray);
// 	} else if (rdist >= 0)
// 		result = right->intersect(ray);

// 	return result;
// }

// kd_tree_node::intersection kd_tree_branch::intersect(const ray &ray) {
// 	kd_tree_node::intersection result;
// 	std::stack<kd_tree_node> stack;

// 	while (!result && !stack.empty()) {
// 		kd_tree_node *node = stack.top();
// 		stack.pop();

// 		while (typeid(*node) == typeid(kd_tree_branch)) {
// 			auto branch = static_cast<kd_tree_branch *>(node);

// 			float dist = branch->aabb.intersect(ray);


// 		}
// 	}
// }

// fvec3 hsv2rgb(const fvec3 &c) {
//     fvec4 K = fvec4(1, 0.666667F, 0.333333F, 3);
//     fvec3 p = abs(fract(fvec3(c.x + K.x, c.x + K.y, c.x + K.z)) * 6 - fvec3(K.w));
//     return c.z * lerp(fvec3(K.x), saturate(p - fvec3(K.x)), c.y);
// }

// kd_tree_node::intersection kd_tree_leaf::intersect(const ray &ray) {
// 	// float dist = aabb.intersect(ray);
// 	// return { dist, hsv2rgb(fvec3(reinterpret_cast<size_t>(this) % 257 / 257.0F, 0.7F, 1)), 0 };

// 	triangle::intersection nearest_hit;
// 	uint32_t index = 0;

// 	for (uint32_t i = 0; i < triangles.size(); i++) {
// 		auto hit = triangles[i].intersect(ray);
// 		if (hit.distance >= 0 && (hit.distance <
// 				nearest_hit.distance || nearest_hit.distance < 0)) {
// 			nearest_hit = hit;
// 			index = i;
// 		}
// 	}

// 	return { nearest_hit.distance, nearest_hit.barycentric, indices[index] };
// }

}
