#pragma once

#include "../pch.hpp"

struct rng_state {
	uint32_t seed;
};

CUDA_CALLABLE inline rng_state rand_init(uint32_t x, uint32_t y, uint32_t sample_idx);
CUDA_CALLABLE inline float rand_float(rng_state &rng);
CUDA_CALLABLE inline glm::fvec3 rand_cone_dir(rng_state &rng, glm::fvec3 normal, float cos_theta);
CUDA_CALLABLE inline glm::fvec3 rand_cone_dir_uniform(rng_state &rng, glm::fvec3 normal, float angular_radius);

// IMPL

CUDA_CALLABLE inline rng_state rand_init(uint32_t x, uint32_t y, uint32_t sample_idx) {
	rng_state rng;
	rng.seed = x * 1973 + y * 9277 + sample_idx * 26699 + 1;
	rng.seed = (x * 1973u + y * 9277u + sample_idx * 26699u) | 1u;
	return rng;
}

CUDA_CALLABLE inline float rand_float(rng_state &rng) {
	// pcg hash
	rng.seed = rng.seed * 747796405u + 2891336453u;
	uint32_t word = ((rng.seed >> ((rng.seed >> 28u) + 4u)) ^ rng.seed) * 277803737u;
	word = (word >> 22u) ^ word;
	return word / 4294967296.0f;
}

CUDA_CALLABLE inline glm::fvec3 rand_cone_dir(rng_state &rng, glm::fvec3 normal, float cos_theta) {
	// random vector in a Z-oriented cone

	float phi = rand_float(rng) * 2.0f * std::numbers::pi_v<float>;
	float sin_theta = sqrt(1 - cos_theta * cos_theta);

	glm::fvec3 cone_vec(std::cos(phi) * sin_theta, std::sin(phi) * sin_theta, cos_theta);

	// coordinate system such that Z = normal

	glm::fvec3 non_parallel_vec = abs(normal.z) < 0.999f ? glm::fvec3(0, 0, 1) : glm::fvec3(1, 0, 0);
	glm::fvec3 tangent = glm::normalize(glm::cross(normal, non_parallel_vec));
	glm::fvec3 binormal = cross(normal, tangent);
	glm::fmat3 tbn(tangent, binormal, normal);

	return tbn * cone_vec;
}

CUDA_CALLABLE inline glm::fvec3 rand_cone_dir_uniform(rng_state &rng, glm::fvec3 normal, float angular_radius) {
    float max_cos_theta = cos(angular_radius);
	float cos_theta = rand_float(rng) * (1 - max_cos_theta) + max_cos_theta;
	return rand_cone_dir(rng, normal, cos_theta);
}
