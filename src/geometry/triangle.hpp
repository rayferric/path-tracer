#pragma once

#include "geometry/ray.hpp"
#include "math/vec3.hpp"

struct triangle {
	struct intersection {
		bool hit;
		float distance;
		math::fvec3 barycentric;
	};

	const math::fvec3 a, b, c;

	triangle(const math::fvec3 &a,
			 const math::fvec3 &b,
			 const math::fvec3 &c);

	intersection intersect(const ray &ray) const;
};