#include "core/pbr.hpp"

#include "math/math.hpp"
#include "math/quat.hpp"
#include "math/mat3.hpp"

using namespace math;

static fvec3 rand_cone_vec(float rand, float cos_theta, fvec3 normal) {
	// Generate random vector in a Z-oriented cone

	float phi = rand * 2 * math::pi;
	float sin_theta = sqrt(1 - cos_theta * cos_theta);

	fvec3 cone_vec(
		math::cos(phi) * sin_theta,
		math::sin(phi) * sin_theta,
		cos_theta
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

	return tbn * cone_vec;
}

namespace core::pbr {

float fresnel_schlick(
		const fvec3 &outcoming,
		const fvec3 &incoming,
		float ior) {
	// Halfway is normal in case of a perfectly smooth mirror
	fvec3 halfway = normalize(outcoming + incoming);
	float cos_theta = dot(outcoming, halfway);

	float f0 = (ior - 1) / (ior + 1);
	f0 *= f0;
	
	return f0 + (1 - f0) * math::pow(1 - cos_theta, 5);
}

static float fresnel_refectance(
		float ior1, float cos1,
		float ior2, float cos2) {
	float a = ior1 * cos1;
	float b = ior2 * cos2;

	float r = (a - b) / (a + b);
	return r * r;
}

float fresnel_exact(
		const fvec3 &normal,
		const fvec3 &incoming,
		float surface_ior,
		float surrounding_ior) {
	// Symbols used:
	// i - incident angle / surrounding IOR
	// t - transmitted angle / surface IOR

	// From Snell's law:
	// ior_i * sin_i = ior_t * sin_t

	float ior_i = surrounding_ior;
	float ior_t = surface_ior;

	float cos_i = dot(incoming, normal);
	float sin_i = math::sqrt(1 - cos_i * cos_i);

	float sin_t = (ior_i * sin_i) / ior_t;

	// Total internal reflection
	if (sin_t > 1)
		return 1;

	float cos_t = math::sqrt(1 - sin_t * sin_t);

	float r_s = fresnel_refectance(ior_t, cos_i, ior_i, cos_t);
	float r_p = fresnel_refectance(ior_t, cos_t, ior_i, cos_i);
	
	return (r_s + r_p) * 0.5F;
}

float distribution_lambert(
		const math::fvec3 &normal,
		const math::fvec3 &incoming) {
	float cos_theta = dot(normal, incoming);
	return math::max(cos_theta, 0) / math::pi;
}

float distribution_ggx(
		const fvec3 &normal,
		const fvec3 &outcoming,
		const fvec3 &incoming,
		float roughness) {
	fvec3 halfway = normalize(outcoming + incoming);
	float cos_theta = dot(normal, halfway);

	roughness *= roughness;
	roughness *= roughness;

	float denom = lerp(1, roughness, cos_theta * cos_theta);
	return roughness / (math::pi * denom * denom);
}

fvec3 importance_lambert(
		const fvec2 &rand,
		const fvec3 &normal,
		const fvec3 &outcoming) {
	float theta = math::acos(1 - 2 * rand.x) * 0.5F;
	return rand_cone_vec(rand.y, cos(theta), normal);
}

fvec3 importance_ggx(
		const fvec2 &rand,
		const fvec3 &normal,
		const fvec3 &outcoming,
		float roughness) {
	roughness *= roughness;
	roughness *= roughness;

	float cos_theta = math::sqrt((1 - rand.x) / (1 + (roughness - 1) * rand.x));
	fvec3 halfway = rand_cone_vec(rand.y, cos_theta, normal);
	
	return reflect(-outcoming, halfway);
}

static float geometry_smith_g1(
		const fvec3 &normal,
		const fvec3 &light_dir,
		float roughness) {
	// light_dir is either outcoming or incoming
	float cos_theta = dot(normal, light_dir);
	return cos_theta / lerp(roughness, 1, cos_theta);
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

}
