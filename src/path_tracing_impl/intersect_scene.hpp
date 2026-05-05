#pragma once

#include "../pch.hpp"

#include "../scene.hpp"

struct hit_info {
	float t;
	glm::fvec3 position;
	glm::fvec3 normal;
	glm::fvec3 tangent;
	glm::fvec2 uv;
	uint32_t mat_idx;
	bool hit;
};

CUDA_CALLABLE inline hit_info intersect_scene(const scene_data &scn, glm::fvec3 pos, glm::fvec3 dir);

///
/// IMPLEMENTATION BELOW
///

// returns true if ray intersects aabb, outputs t_min and t_max along ray
CUDA_CALLABLE inline bool intersect_aabb(glm::fvec3 pos, glm::fvec3 dir_inv, glm::fvec3 aabb_min, glm::fvec3 aabb_max, float &t_min, float &t_max) {
	// slab method using precomputed inv_direction
	glm::fvec3 t0 = (aabb_min - pos) * dir_inv;
	glm::fvec3 t1 = (aabb_max - pos) * dir_inv;

	glm::fvec3 tmin_vec = glm::min(t0, t1);
	glm::fvec3 tmax_vec = glm::max(t0, t1);

	t_min = glm::max(glm::max(tmin_vec.x, tmin_vec.y), tmin_vec.z);
	t_max = glm::min(glm::min(tmax_vec.x, tmax_vec.y), tmax_vec.z);

	return t_max >= t_min && t_max >= 0;
}

// moller-trumbore algorithm
// returns true if ray intersects triangle, outputs barycentric coords (u,v) and distance t
CUDA_CALLABLE inline bool intersect_triangle(glm::fvec3 pos, glm::fvec3 dir, glm::fvec3 v0, glm::fvec3 v1, glm::fvec3 v2, float &t, float &u, float &v) {
	const float epsilon = 1e-8f;

	glm::fvec3 edge1 = v1 - v0;
	glm::fvec3 edge2 = v2 - v0;

	glm::fvec3 h = glm::cross(dir, edge2);
	float a = glm::dot(edge1, h);

	// ray parallel to triangle
	if (a > -epsilon && a < epsilon) {
		return false;
	}

	float f = 1.0f / a;
	glm::fvec3 s = pos - v0;
	u = f * glm::dot(s, h);

	if (u < 0.0f || u > 1.0f) {
		return false;
	}

	glm::fvec3 q = glm::cross(s, edge1);
	v = f * glm::dot(dir, q);

	if (v < 0.0f || u + v > 1.0f) {
		return false;
	}

	t = f * glm::dot(edge2, q);

	return t > epsilon;
}

CUDA_CALLABLE inline glm::fvec3 calc_tangent(const glm::fvec3 &v0, const glm::fvec3 &v1, const glm::fvec3 &v2, const glm::fvec2 &uv0, const glm::fvec2 &uv1, const glm::fvec2 &uv2, const glm::fvec3 &normal) {
	glm::fvec3 edge1 = v1 - v0;
	glm::fvec3 edge2 = v2 - v0;
	glm::fvec2 duv1 = uv1 - uv0;
	glm::fvec2 duv2 = uv2 - uv0;

	float det = duv1.x * duv2.y - duv1.y * duv2.x;

	glm::fvec3 tangent;
	if (fabsf(det) > 1e-8f) {
		float inv_det = 1.0f / det;
		tangent = (edge1 * duv2.y - edge2 * duv1.y) * inv_det;
	} else {
		// degenerate UVs, pick arbitrary tangent perpendicular to normal
		tangent = fabsf(normal.x) < 0.9f ? glm::fvec3(1, 0, 0) : glm::fvec3(0, 1, 0);
	}

	// gram-schmidt orthogonalize against normal
	tangent = glm::normalize(tangent - normal * glm::dot(normal, tangent));

	return tangent;
}

CUDA_CALLABLE inline hit_info intersect_scene(const scene_data &scn, glm::fvec3 pos, glm::fvec3 dir) {
	hit_info closest_hit = {std::numeric_limits<float>::infinity(), {}, {}, {}, {}, 0, false};

	uint32_t stack[24];
	int stack_ptr = 0;
	stack[stack_ptr++] = 0;

	glm::fvec3 inv_dir = 1.0f / dir;

	while (stack_ptr > 0) {
		uint32_t node_idx = stack[--stack_ptr];
		const bvh_node &node = scn.bvh_nodes[node_idx];

		float tmin, tmax;
		if (!intersect_aabb(pos, inv_dir, node.aabb_min, node.aabb_max, tmin, tmax)) {
			continue;
		}
		if (tmin > closest_hit.t) {
			continue;
		}

		bool is_leaf = (node.left_idx_or_tri_begin & 0x80000000) != 0;

		if (is_leaf) {
			uint32_t tri_begin = node.left_idx_or_tri_begin & 0x7FFFFFFF;
			uint32_t tri_count = node.right_idx_or_tri_count;

			for (uint32_t i = 0; i < tri_count; i++) {
				const triangle &tri = scn.triangles[tri_begin + i];

				float t, u, v;
				if (!intersect_triangle(pos, dir, tri.corners[0], tri.corners[1], tri.corners[2], t, u, v)) {
					continue;
				}
				if (t > closest_hit.t) {
					continue;
				}

				const triangle_ext &ext = scn.triangles_ext[tri_begin + i];
				float w = 1.0f - u - v;

				closest_hit.t = t;
				closest_hit.position = pos + dir * t;
				closest_hit.normal = glm::normalize(ext.normal[0] * w + ext.normal[1] * u + ext.normal[2] * v);
				closest_hit.uv = ext.uv[0] * w + ext.uv[1] * u + ext.uv[2] * v;
				closest_hit.tangent = calc_tangent(tri.corners[0], tri.corners[1], tri.corners[2], ext.uv[0], ext.uv[1], ext.uv[2], closest_hit.normal);
				closest_hit.mat_idx = ext.mat_idx;
				closest_hit.hit = true;
			}
		} else {
			uint32_t left_idx = node.left_idx_or_tri_begin;
			uint32_t right_idx = node.right_idx_or_tri_count;

            assert(stack_ptr + 2 <= 24 && "BVH stack overflow, tree is too deep.");

			stack[stack_ptr++] = left_idx;
			stack[stack_ptr++] = right_idx;
		}
	}

	return closest_hit;
}
