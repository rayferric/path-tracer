#pragma once

#include "math/vec3.hpp"
#include "renderer/ray.hpp"

namespace renderer {

struct triangle {
	struct intersection {
		float distance = -1;
		math::fvec3 barycentric;
	};

	math::fvec3 a, b, c;

	triangle(const math::fvec3 &a,
			 const math::fvec3 &b,
			 const math::fvec3 &c);

	intersection intersect(const ray &ray) const;
};

}
