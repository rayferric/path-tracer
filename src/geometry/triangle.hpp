#pragma once

#include "math/vec3.hpp"
#include "geometry/ray.hpp"

namespace geometry {

struct triangle {
	struct intersection {
		float distance = -1;
		math::fvec3 barycentric;

		bool has_hit() const;
	};

	math::fvec3 a, b, c;

	triangle(const math::fvec3 &a,
			 const math::fvec3 &b,
			 const math::fvec3 &c);

	intersection intersect(const ray &ray) const;
};

}
