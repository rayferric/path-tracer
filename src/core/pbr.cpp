#include "core/pbr.hpp"

#include "math/math.hpp"
#include "math/quat.hpp"
#include "math/mat3.hpp"

using namespace math;

// light_dir is either outcoming or incoming
static float geometry_smith_g1(
		const fvec3 &normal,
		const fvec3 &light_dir,
		float roughness) {
	float cos_theta = dot(normal, light_dir);
	return cos_theta / lerp(roughness, 1, cos_theta);
}

namespace core::pbr {

// halfway == normal in case of a perfectly smooth mirror
float fresnel_schlick(
		const fvec3 &outcoming,
		const fvec3 &incoming,
		float ior) {
	fvec3 halfway = normalize(outcoming + incoming);
	float cos_theta = dot(outcoming, halfway);

	float f0 = (ior - 1) / (ior + 1);
	f0 *= f0;
	
	return f0 + (1 - f0) * math::pow(1 - cos_theta, 5);
}

float distribution_ggx(
		const fvec3 &normal,
		const fvec3 &outcoming,
		const fvec3 &incoming,
		float roughness) {
	fvec3 halfway = normalize(outcoming + incoming);
	float cos_theta = dot(normal, halfway);

	float r2 = roughness * roughness;
	float r4 = r2 * r2;

	float d = lerp(1, r4, cos_theta * cos_theta);
	return r4 / (math::pi * d * d);
}

float geometry_smith(
		const fvec3 &normal,
		const fvec3 &outcoming,
		const fvec3 &incoming,
		float roughness) {
	float r = roughness + 1;
	float k = (r * r) / 8;

	return geometry_smith_g1(normal, outcoming, k) *
		   geometry_smith_g1(normal,  incoming, k);
}

// Returns random incoming
math::fvec3 inverse_distribution_ggx(
		float value,
		const fvec3 &normal,
		const fvec3 &outcoming,
		float roughness,
		float rand) {
	// Find theta algebraically

	float r2 = roughness * roughness;
	float r4 = r2 * r2;

	float d = math::sqrt(r4 / (value * math::pi));
	float cos_theta_sq = (d - 1) / (r4 - 1);
	
	// Generate random vector in a Z-oriented cone

	float phi = rand * 2 * math::pi;
	float r = sqrt(1 - cos_theta_sq);

	fvec3 cone_vec(
		math::cos(phi) * r,
		math::sin(phi) * r,
		math::sqrt(cos_theta_sq)
	);

	// Create coordinate system such that Z = normal
	
	fvec3 non_parallel_vec = fvec3::zero;
	if (abs(normal.x) < (1 / math::sqrt3))
		non_parallel_vec.x = 1;
	else if (abs(normal.y) < (1 / math::sqrt3))
		non_parallel_vec.y = 1;
	else
		non_parallel_vec.z = 1;
	
	fvec3 tangent = normalize(cross(normal, non_parallel_vec));
	fvec3 binormal = cross(normal, tangent);
	fmat3 tbn(tangent, binormal, normal);

	fvec3 halfway = tbn * cone_vec;
	return math::reflect(-outcoming, halfway);
}

}
