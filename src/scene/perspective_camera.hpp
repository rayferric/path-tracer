#pragma once

#include "math/vec2.hpp"
#include "scene/camera.hpp"

namespace scene {

class perspective_camera : public camera {
public:
	ray get_ray(fvec2 screen_pos, float ratio) const override;

	float get_fov() const;

	void set_fov(float fov);

private:
	// FOV is vertical
	float fov;
	float fov_tan;
};

}
