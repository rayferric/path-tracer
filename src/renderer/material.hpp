#pragma once

#include "pch.hpp"

#include "math/vec3.hpp"
#include "texture/texture.hpp"

namespace renderer {

struct material {
	bool culling, translucent;

	math::fvec3 albedo_mul;
	float opacity_mul;
	float occlusion_exp;
	float roughness_mul;
	float metallic_mul;
	math::fvec3 emissive_mul;

	std::shared_ptr<texture>
			albedo_map    = nullptr,
			opacity_map   = nullptr,
			normal_map    = nullptr,
			occlusion_map = nullptr,
			roughness_map = nullptr,
			metallic_map  = nullptr,
			emissive_map  = nullptr;
};

}
