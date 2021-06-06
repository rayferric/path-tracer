#pragma once

#include "pch.hpp"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "core/kd_tree.hpp"
#include "core/material.hpp"
#include "core/ray.hpp"
#include "core/vertex.hpp"

namespace core {

struct mesh {
	struct intersection {
		float distance = -1;
		math::fvec3 barycentric;
		uint32_t index;

		bool has_hit() const;
	};

	std::vector<vertex> vertices;
	std::vector<math::uvec3> triangles;
	core::aabb aabb;
	std::shared_ptr<kd_tree_node> kd_tree = nullptr;
	std::shared_ptr<core::material> material = nullptr;

	// void recalculate_normals(bool shade_smooth = false);

	// void recalculate_tangents();

	void recalculate_aabb();

	void build_kd_tree(bool use_sah = true, uint8_t max_depth = 25);

	intersection intersect(const ray &ray) const;
};

}
