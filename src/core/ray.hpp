#pragma once

#include "math/vec3.hpp"

namespace core {

class ray {
public:
	math::fvec3 origin;

	ray(const math::fvec3 &origin, const math::fvec3 &dir);

	math::fvec3 get_dir() const;

	void set_dir(const math::fvec3 &dir);

	ray transform(const transform &transform)

private:
	math::fvec3 dir;
};

}
