#pragma once

#include "pch.hpp"

#include "math/vec2.hpp"
#include "math/vec3.hpp"

namespace core::pbr {

float fresnel_schlick(
		const math::fvec3 &outcoming,
		const math::fvec3 &incoming,
		float ior);

// Surround with air by default
float fresnel_exact(
		const math::fvec3 &outcoming,
		const math::fvec3 &incoming,
		float surface_ior,
		float surrounding_ior = 1);

float distribution_lambert(
		const math::fvec3 &normal,
		const math::fvec3 &incoming);

float distribution_ggx(
		const math::fvec3 &normal,
		const math::fvec3 &outcoming,
		const math::fvec3 &incoming,
		float roughness);

math::fvec3 importance_lambert(
		const math::fvec2 &rand,
		const math::fvec3 &normal,
		const math::fvec3 &outcoming);

math::fvec3 importance_ggx(
		const math::fvec2 &rand,
		const math::fvec3 &normal,
		const math::fvec3 &outcoming,
		float roughness);

float geometry_smith(
		const math::fvec3 &normal,
		const math::fvec3 &outcoming,
		const math::fvec3 &incoming,
		float roughness);

}
