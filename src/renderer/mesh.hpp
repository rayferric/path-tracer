#pragma once

#include "pch.hpp"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "renderer/kd_tree.hpp"
#include "renderer/material.hpp"
#include "renderer/ray.hpp"

namespace renderer {

struct mesh {
	struct vertex {
		math::fvec3 position;
		math::fvec2 tex_coord;
		math::fvec3 normal;
		math::fvec3 tangent;
	};

	struct intersection {
		float distance = -1;
		math::fvec3 barycentric;
		uint32_t index;
	};

	std::vector<vertex> vertices;
	std::vector<math::uvec3> triangles;
	renderer::aabb aabb;
	std::shared_ptr<kd_tree_node> kd_tree = nullptr;
	std::shared_ptr<renderer::material> material = nullptr;

	// void recalculate_normals(bool shade_smooth = false);

	// void recalculate_tangents();

	void recalculate_aabb();

	void build_kd_tree(uint8_t depth = 20);

	intersection intersect(const ray &ray) const;
};

}
