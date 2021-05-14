#pragma once

#include "math/vec3.hpp"
#include "renderer/ray.hpp"

namespace renderer {

struct aabb {
	struct intersection {
		bool hit;
		float distance;
	};

	math::fvec3 min, max;

	aabb();

	aabb(const math::fvec3 &min, const math::fvec3 &max);

	void clear();

	void add_point(const math::fvec3 &point);

	intersection intersect(const ray &ray) const;
};

}
