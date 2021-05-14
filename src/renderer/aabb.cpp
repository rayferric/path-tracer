#include "renderer/aabb.hpp"

#include "math/math.hpp"

using namespace math;

namespace renderer {

aabb::aabb() {}

aabb::aabb(const fvec3 &min, const fvec3 &max)
		: min(min), max(max) {}

void aabb::clear() {
	min = fvec3(std::numeric_limits<float>::max());
	max = fvec3(std::numeric_limits<float>::min());
}

void aabb::add_point(const fvec3 &point) {
	min = math::min(min, point);
	max = math::max(max, point);
}

aabb::intersection aabb::intersect(const ray &ray) const {
	if (any(min > max))
		return { false };

	fvec3 min_bounds_distances = (min - ray.origin) / ray.get_dir();
	fvec3 max_bounds_distances  = (max - ray.origin) / ray.get_dir();

	fvec3 near_distances = math::min(min_bounds_distances, max_bounds_distances);
	fvec3 far_distances = math::max(min_bounds_distances, max_bounds_distances);
	
	float near = math::max(near_distances.x,
			near_distances.y, near_distances.z);
	float far = math::min(far_distances.x,
			far_distances.y, far_distances.z);

	// Miss
	if (near > far)
		return { false };
	
	// Intersection occured behind the ray
	if (far < 0)
		return { false };

	// Near might be negative
	// if we're inside the box
	return { true, near > 0 ? near : far };
}

}
