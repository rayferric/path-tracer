#include "renderer/ray.hpp"

using namespace math;

namespace renderer {

ray::ray(const fvec3 &origin, const fvec3 &dir)
		: origin(origin), dir(normalize(dir)) {}

math::fvec3 ray::get_dir() const {
	return dir;
}

void ray::set_dir(const math::fvec3 &dir) {
	this->dir = normalize(dir);
}

}
