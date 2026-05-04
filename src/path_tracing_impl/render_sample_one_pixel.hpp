#pragma once

#include "../pch.hpp"

#include "../scene.hpp"

#include "./path_trace.hpp"
#include "./rand.hpp"
#include "material_tex_sampling.hpp"

CUDA_CALLABLE inline void render_sample_one_pixel(const scene_data &scn, glm::fvec4 *out_buf, uint32_t width, uint32_t height, uint32_t x, uint32_t y, uint32_t sample_idx, bool transparent_bg);

// IMPL

CUDA_CALLABLE inline void gen_camera_ray(const scene_data &scn, rng_state &rng, uint32_t width, uint32_t height, uint32_t x, uint32_t y, glm::fvec3 &out_pos, glm::fvec3 &out_dir) {
	glm::fvec2 aa_offset = glm::fvec2(rand_float(rng), rand_float(rng));

	glm::fvec2 ndc = ((glm::fvec2(x, y) + aa_offset) / glm::fvec2(width, height)) * 2.0f - 1.0f;
	ndc.y = -ndc.y; // [0, 0] is top-right corner, should map to [-1, +1] NDC

	float aspect_ratio = static_cast<float>(width) / height;
	float tan_half_fov = tan(scn.camera_vfov_rad * 0.5f);

	// clang-format off
	glm::fvec3 dir = glm::normalize(glm::fvec3(
        ndc.x * aspect_ratio * tan_half_fov,
        ndc.y * tan_half_fov,
        -1.0f
    ));
	// clang-format on

	out_pos = glm::fvec3(scn.camera_transform[3]);
	out_dir = glm::fmat3(scn.camera_transform) * dir;
}

CUDA_CALLABLE inline void render_sample_one_pixel(const scene_data &scn, glm::fvec4 *out_buf, uint32_t width, uint32_t height, uint32_t x, uint32_t y, uint32_t sample_idx, bool transparent_bg) {
	rng_state rng = rand_init(x, y, sample_idx);

	glm::vec3 pos, dir;
	gen_camera_ray(scn, rng, width, height, x, y, pos, dir);

	bool hit_bg;
	glm::fvec3 new_color = path_trace(scn, pos, dir, rng, 4, hit_bg); // max_bounces=4

	glm::fvec4 old_sample = out_buf[y * width + x];
	glm::fvec3 old_color = glm::fvec3(old_sample);
	float old_alpha = old_sample.w;

	// // Smart blending - needed for transparent background
	float sample_idx_f = sample_idx;
	// if (transparent_bg && !hit_bg && old_alpha == 0.0f) {
	// 	// an opaque sample will claim this pixel
	// 	old_color = new_color;
	// 	old_alpha = 1.0f / (sample_idx_f + 1.0f);
	// } else if (transparent_bg && hit_bg && old_alpha != 0.0f) {
	// 	// If a transparent sample encounters a claimed pixel
	// 	// Preserve color and blend only alpha
	// 	old_color = old_color;
	// 	old_alpha = (old_alpha * sample_idx_f) / (sample_idx_f + 1.0f);
	// } else if (!transparent_bg || !hit_bg) {
	// 	// Otherwise if an opaque sample blends with a claimed pixel (or transparent background is disabled)
	// 	float new_alpha = hit_bg ? 0.0f : 1.0f;
	// 	old_color = (old_color * sample_idx_f + new_color) / (sample_idx_f + 1.0f);
	// 	old_alpha = (old_alpha * sample_idx_f + new_alpha) / (sample_idx_f + 1.0f);
	// }
	if (!hit_bg) {
		old_color = (old_color * sample_idx_f + new_color) / (sample_idx_f + 1.0f);
	}
	float new_alpha = hit_bg ? 0.0f : 1.0f;
	old_alpha = (old_alpha * sample_idx_f + new_alpha) / (sample_idx_f + 1.0f);

	// overlay on environment
	if (!transparent_bg) {
		glm::fvec3 env_color = sample_environment(scn, dir); 
		old_color = glm::mix(env_color, old_color, old_alpha);
		old_alpha = 1.0;
	}

	out_buf[y * width + x] = glm::fvec4(old_color, old_alpha);
}
