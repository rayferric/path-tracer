#pragma once

#include "../pch.hpp"

#include "./rand.hpp"

///
/// PUBLIC API
///

CUDA_CALLABLE inline glm::fvec3 fresnel_f0(glm::fvec3 outcoming, glm::fvec3 incoming, glm::fvec3 f0);
CUDA_CALLABLE inline float fresnel_ior(glm::fvec3 outcoming, glm::fvec3 incoming, float ior);

CUDA_CALLABLE inline glm::vec3 eval_brdf(glm::fvec3 normal, glm::fvec3 outcoming, glm::fvec3 incoming, glm::vec3 albedo, float metallic, float roughness);

CUDA_CALLABLE inline glm::fvec3 importance_diffuse(rng_state &rng, glm::fvec3 normal, glm::fvec3 outcoming);
CUDA_CALLABLE inline glm::fvec3 importance_specular(rng_state &rng, glm::fvec3 normal, glm::fvec3 outcoming, float roughness);

CUDA_CALLABLE inline float pdf_diffuse(glm::fvec3 normal, glm::fvec3 incoming);
CUDA_CALLABLE inline float pdf_specular(glm::fvec3 normal, glm::fvec3 outcoming, glm::fvec3 incoming, float roughness);

///
/// IMPLEMENTATION BELOW
///

#define PBR_PI std::numbers::pi_v<float>

// Fresnel

CUDA_CALLABLE inline glm::fvec3 fresnel_schlick_f0(glm::fvec3 outcoming, glm::fvec3 incoming, glm::fvec3 f0) {
	// Halfway is the normal in case of a perfectly smooth mirror
	glm::fvec3 halfway = normalize(outcoming + incoming);
	float cos_theta = dot(outcoming, halfway);

	return glm::mix(f0, glm::fvec3(1), glm::pow(1 - cos_theta, 5));
}

CUDA_CALLABLE inline float fresnel_schlick_ior(glm::fvec3 outcoming, glm::fvec3 incoming, float ior) {
	float f0 = (ior - 1) / (ior + 1);
	f0 *= f0;
	return fresnel_schlick_f0(outcoming, incoming, glm::fvec3(f0)).x;
}

// Importance

CUDA_CALLABLE inline glm::fvec3 importance_lambert(rng_state &rng, glm::fvec3 normal, glm::fvec3 outcoming) {
	float cos_theta = std::sqrt(rand_float(rng));
	return rand_cone_dir(rng, normal, cos_theta);
}

CUDA_CALLABLE inline glm::fvec3 importance_ggx(rng_state &rng, glm::fvec3 normal, glm::fvec3 outcoming, float roughness) {
	roughness *= roughness;
	roughness *= roughness;

	float x = rand_float(rng);
	float cos_theta = glm::sqrt((1 - x) / (1 + (roughness - 1) * x));
	glm::fvec3 halfway = rand_cone_dir(rng, normal, cos_theta);

	return reflect(-outcoming, halfway);
}

// Geometric occlusion

CUDA_CALLABLE inline float geometry_smith_g1(glm::fvec3 normal, const glm::fvec3 &light_dir, float k) {
	// light_dir is either outcoming or incoming
	float cos_theta = dot(normal, light_dir);
	return cos_theta / std::lerp(k, 1.0f, cos_theta);
}

CUDA_CALLABLE inline float geometry_smith(glm::fvec3 normal, glm::fvec3 outcoming, glm::fvec3 incoming, float roughness) {
	float r = roughness + 1;
	float k = (r * r) / 8;

	return geometry_smith_g1(normal, outcoming, k) * geometry_smith_g1(normal, incoming, k);
}

// Distribution

CUDA_CALLABLE inline float distribution_lambert(glm::fvec3 normal, glm::fvec3 incoming) {
	float cos_theta = dot(normal, incoming);
	return cos_theta / PBR_PI;
}

CUDA_CALLABLE inline float distribution_ggx(glm::fvec3 normal, glm::fvec3 outcoming, glm::fvec3 incoming, float roughness) {
	roughness *= roughness;
	roughness *= roughness;

	glm::fvec3 halfway = normalize(outcoming + incoming);
	float cos_phi = dot(normal, halfway);

	float denom = std::lerp(1, roughness, cos_phi * cos_phi);

	float cos_theta = dot(normal, incoming);
	return cos_theta * roughness / (PBR_PI * denom * denom);
}

// BRDF

CUDA_CALLABLE inline glm::vec3 brdf_diffuse(glm::fvec3 normal, glm::fvec3 incoming, glm::vec3 albedo, float metallic) {
	return distribution_lambert(normal, incoming) * albedo * (1 - metallic);
}

CUDA_CALLABLE inline glm::vec3 brdf_specular(glm::fvec3 normal, glm::fvec3 outcoming, glm::fvec3 incoming, float roughness) {
	float dist = distribution_ggx(normal, outcoming, incoming, roughness);
	float geo = geometry_smith(normal, outcoming, incoming, roughness);

	float n_dot_o = dot(normal, outcoming);
	float n_dot_i = dot(normal, incoming);

	return glm::vec3((dist * geo) / (4 * n_dot_o * n_dot_i));
}

// Public API

CUDA_CALLABLE inline glm::fvec3 fresnel_f0(glm::fvec3 outcoming, glm::fvec3 incoming, glm::fvec3 f0) {
	return fresnel_schlick_f0(outcoming, incoming, f0);
}

CUDA_CALLABLE inline float fresnel_ior(glm::fvec3 outcoming, glm::fvec3 incoming, float ior) {
	return fresnel_schlick_ior(outcoming, incoming, ior);
}

CUDA_CALLABLE inline glm::vec3 eval_brdf(glm::fvec3 normal, glm::fvec3 outcoming, glm::fvec3 incoming, glm::vec3 albedo, float metallic, float roughness) {
	glm::fvec3 diffuse_brdf = brdf_diffuse(normal, incoming, albedo, metallic);
	glm::fvec3 specular_brdf = brdf_specular(normal, outcoming, incoming, roughness);

	glm::fvec3 f0 = glm::mix(glm::fvec3(0.04f), albedo, metallic);
	glm::fvec3 fresnel_term = fresnel_f0(outcoming, incoming, f0);

	return glm::mix(diffuse_brdf, specular_brdf, fresnel_term);
}

CUDA_CALLABLE inline glm::fvec3 importance_diffuse(rng_state &rng, glm::fvec3 normal, glm::fvec3 outcoming) {
	return importance_lambert(rng, normal, outcoming);
}

CUDA_CALLABLE inline glm::fvec3 importance_specular(rng_state &rng, glm::fvec3 normal, glm::fvec3 outcoming, float roughness) {
	return importance_ggx(rng, normal, outcoming, roughness);
}

CUDA_CALLABLE inline float pdf_diffuse(glm::fvec3 normal, glm::fvec3 incoming) {
	return distribution_lambert(normal, incoming);
}

CUDA_CALLABLE inline float pdf_specular(glm::fvec3 normal, glm::fvec3 outcoming, glm::fvec3 incoming, float roughness) {
	float dist = distribution_ggx(normal, outcoming, incoming, roughness);
	float geo = geometry_smith(normal, outcoming, incoming, roughness);

	float n_dot_o = dot(normal, outcoming);
	float n_dot_i = dot(normal, incoming);

	return (dist * geo) / (4 * n_dot_o * n_dot_i);
}
