#pragma once

#include "../pch.hpp"

#include "../scene.hpp"

struct mat_sample {
	glm::fvec3 albedo;
	float opacity;
	float roughness;
	float metallic;
	glm::fvec3 emissive;
	glm::fvec3 normal; // global vector, not a texture sample
	float ior;
};

CUDA_CALLABLE inline mat_sample sample_material(const scene_data &scn, uint32_t mat_idx, glm::fvec2 uv);
CUDA_CALLABLE inline glm::fvec3 sample_environment(const scene_data &scn, glm::fvec3 dir);

// IMPL

CUDA_CALLABLE inline glm::fvec4 _sample_texture(const scene_data &scn, int32_t tex_idx, glm::fvec2 uv) {
	if (tex_idx < 0) {
		return glm::fvec4(1.0f);
	}

	const texture_info &tex = scn.textures[tex_idx];

	// bilinear sampling
	glm::fvec2 c(uv.x * tex.width - 0.5f, uv.y * tex.height - 0.5f);

	glm::ivec2 i_c(floor(c.x), floor(c.y));
	glm::ivec2 i_res(tex.width, tex.height);

#define WRAP(val, max_val) (((val) % (max_val) < 0) ? ((val) % (max_val) + (max_val)) : ((val) % (max_val)))

	auto tl = glm::uvec2(WRAP(i_c.x, i_res.x), WRAP(i_c.y, i_res.y));
	auto tr = glm::uvec2(WRAP(i_c.x + 1, i_res.x), WRAP(i_c.y, i_res.y));
	auto bl = glm::uvec2(WRAP(i_c.x, i_res.x), WRAP(i_c.y + 1, i_res.y));
	auto br = glm::uvec2(WRAP(i_c.x + 1, i_res.x), WRAP(i_c.y + 1, i_res.y));

#undef WRAP

	glm::fvec2 delta = glm::fract(c);

	auto read_pixel = [&](glm::uvec2 p) -> glm::fvec4 {
		uint32_t offset = (p.y * tex.width + p.x) * tex.channels;

		glm::fvec4 result(0.0f);

		if (tex.format == texture_format::hdr) {
			float *pixel = (float *)(scn.texture_data + tex.data_offset + offset * sizeof(float));
			for (uint32_t i = 0; i < tex.channels && i < 4; i++) {
				result[i] = pixel[i];
			}
		} else {
			uint8_t *pixel = scn.texture_data + tex.data_offset + offset;
			for (uint32_t i = 0; i < tex.channels && i < 4; i++) {
				float linear = pixel[i] / 255.0f;
				if (tex.format == texture_format::srgb_color && i < 3) {
					result[i] = powf(linear, 2.2f);
				} else {
					result[i] = linear;
				}
			}
		}

		// fill missing channels with defaults
		if (tex.channels < 4) {
			result.w = 1.0f;
		}

		return result;
	};

	glm::fvec4 t = glm::mix(read_pixel(tl), read_pixel(tr), delta.x);
	glm::fvec4 b = glm::mix(read_pixel(bl), read_pixel(br), delta.x);

	return glm::mix(t, b, delta.y);
}

CUDA_CALLABLE inline mat_sample sample_material(const scene_data &scn, uint32_t mat_idx, glm::fvec2 uv, glm::fvec3 normal, glm::fvec3 tangent) {
	const material &mat = scn.materials[mat_idx];

	glm::fvec3 albedo = mat.albedo;
	if (mat.albedo_tex_idx >= 0) {
		glm::fvec3 tex_albedo = glm::fvec3(_sample_texture(scn, mat.albedo_tex_idx, uv));
		// ^ sRGB to linear
		tex_albedo = glm::pow(tex_albedo, glm::fvec3(2.2f));
		albedo *= tex_albedo;
	}

	float opacity = mat.opacity;
	if (mat.opacity_tex_idx >= 0) {
		opacity *= _sample_texture(scn, mat.opacity_tex_idx, uv).a;
	}

	float roughness = mat.roughness;
	if (mat.roughness_tex_idx >= 0) {
		roughness *= _sample_texture(scn, mat.roughness_tex_idx, uv).g;
	}

	float metallic = mat.metallic;
	if (mat.metallic_tex_idx >= 0) {
		metallic *= _sample_texture(scn, mat.metallic_tex_idx, uv).b;
	}

	glm::fvec3 emissive = mat.emissive;
	if (mat.emissive_tex_idx >= 0) {
		emissive *= glm::fvec3(_sample_texture(scn, mat.emissive_tex_idx, uv));
	}

	if (mat.normal_tex_idx >= 0) {
		glm::fvec3 normal_sample = glm::fvec3(_sample_texture(scn, mat.normal_tex_idx, uv));
		normal_sample = normal_sample * 2.0f - 1.0f; // [0,1] to [-1,1]
		glm::fvec3 binormal = glm::cross(normal, tangent);
		glm::fmat3 tbn(tangent, binormal, normal);
		normal = glm::normalize(tbn * normal_sample);
	}

	constexpr float ior = 1.33f;

	return {albedo, opacity, roughness, metallic, emissive, normal, ior};
}

CUDA_CALLABLE inline glm::fvec3 sample_environment(const scene_data &scn, glm::fvec3 dir) {
	glm::fvec3 color = scn.env_intensity;
	if (scn.env_tex_idx >= 0) {
		// map direction to equirectangular UV
		float u = std::atan2(dir.z, dir.x) * 0.1591f + 0.5f;
		float v = std::asin (dir.y)        * 0.3183f + 0.5f;

		color *= glm::fvec3(_sample_texture(scn, scn.env_tex_idx, glm::fvec2(u, v)));
	}
	return color;
}
