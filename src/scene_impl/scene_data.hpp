#pragma once

#include "../pch.hpp"

enum class texture_format : uint8_t { srgb_color, linear_noncolor, hdr };

struct texture_info {
	uint32_t data_offset;

	uint32_t width;
	uint32_t height;
	uint8_t channels;
	texture_format format;
};

struct material {
	glm::fvec3 albedo;
	float opacity;
	float roughness;
	float metallic;
	glm::fvec3 emissive;

	float ior;
	bool shadow_catcher;

	// texture indices into separate texture array, -1 if not present
	int32_t albedo_tex_idx; // sRGB, needs ^2.2
	int32_t opacity_tex_idx;
	int32_t roughness_tex_idx;
	int32_t metallic_tex_idx;
	int32_t emissive_tex_idx;

	int32_t normal_tex_idx;
};

struct triangle {
	glm::fvec3 corners[3];
};

struct triangle_ext {
	glm::fvec2 uv[3];
	glm::fvec3 normal[3];
	uint32_t mat_idx;
};

// pack to 32 bytes, fits in single cache line transaction
struct bvh_node {
	glm::vec3 aabb_min;
	glm::vec3 aabb_max;
	uint32_t left_idx_or_tri_begin; // special: highest bit = is_leaf
	uint32_t right_idx_or_tri_count;
};

struct scene_data {
	uint8_t *texture_data;
	uint32_t texture_data_sz;

	texture_info *textures;
	uint32_t num_textures;

	material *materials;
	uint32_t num_materials;

	triangle *triangles;
	triangle_ext *triangles_ext;
	uint32_t num_triangles;

	bvh_node *bvh_nodes; // first node is the root
	uint32_t num_bvh_nodes;

	glm::mat4 camera_transform;
	float camera_vfov_rad;

	glm::fvec3 sunlight_dir;
	glm::fvec3 sunlight_intensity;
	float sunlight_angular_radius;

	glm::fvec3 env_intensity;
	int32_t env_tex_idx; // -1 if not present
};
