#include "scene/perspective_camera.hpp"

#include "math/math.hpp"
#include "math/vec3.hpp"

namespace scene {

ray perspective_camera::get_ray(
		fvec2 screen_pos, float ratio) const {
	fvec2 ndc = screen_pos * 2 - fvec2::one;
	fvec2 dir = fov_tan * ndc;
	dir.x *= ratio;

	// Constructor normalizes direction
	core::ray ray(
		fvec3::zero,
		fvec3(dir.x, dir.y, -1)
	);

	return ray.transform(get_parent()
			->get_global_transform());
}

float perspective_camera::get_fov() const {
	return fov;
}

void perspective_camera::set_fov(float fov) {
	this->fov = fov;
	this->fov_tan = tan(fov);
}

}