#pragma once

#include "core/ray.hpp"
#include "scene/component.hpp"

namespace scene {

class camera : public component {
public:
	ray get_ray(fvec2 screen_pos, float ratio) const = 0;
};

}
