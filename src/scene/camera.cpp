#include "scene/camera.hpp"

#include "math/math.hpp"
#include "math/vec3.hpp"

using namespace geometry;
using namespace math;

namespace scene {

ray camera::get_ray(const fvec2 &screen_pos, float ratio) const {
	fvec2 ndc = screen_pos * 2 - fvec2::one;
	fvec2 dir = fov_tan * ndc;
	dir.x *= ratio;

	// Constructor normalizes direction
	geometry::ray ray(
		fvec3::zero,
		fvec3(dir.x, dir.y, -1)
	);

	return ray.transform(get_entity()
			->get_global_transform());
}

float camera::get_fov() const {
	return fov;
}

void camera::set_fov(float fov) {
	this->fov = fov;
	this->fov_tan = math::tan(fov);
}

}