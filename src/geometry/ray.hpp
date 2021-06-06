#pragma once

#include "math/vec3.hpp"
#include "scene/transform.hpp"

namespace geometry {

class ray {
public:
	math::fvec3 origin;

	ray(const math::fvec3 &origin, const math::fvec3 &dir);

	ray transform(const scene::transform &transform) const;

	math::fvec3 get_dir() const;

	void set_dir(const math::fvec3 &dir);

private:
	math::fvec3 dir;
};

}
