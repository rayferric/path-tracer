#pragma once

#include "pch.hpp"

#include "math/vec3.hpp"

namespace core::pbr {

float fresnel_schlick(
		const math::fvec3 &outcoming,
		const math::fvec3 &incoming,
		float ior);

float distribution_ggx(
		const math::fvec3 &normal,
		const math::fvec3 &outcoming,
		const math::fvec3 &incoming,
		float roughness);

float geometry_smith(
		const math::fvec3 &normal,
		const math::fvec3 &outcoming,
		const math::fvec3 &incoming,
		float roughness);

math::fvec3 inverse_distribution_ggx(
		float value,
		const math::fvec3 &normal,
		const math::fvec3 &outcoming,
		float roughness,
		float rand);

}
