#pragma once

#include "pch.hpp"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "renderer/kd_tree.hpp"
#include "renderer/material.hpp"

namespace renderer {

struct mesh {
	struct vertex {
		math::fvec3 position;
		math::fvec2 tex_coord;
		math::fvec3 normal;
		math::fvec3 tangent;
	};

	std::vector<vertex> vertices;
	std::vector<math::uvec3> triangles;
	std::shared_ptr<kd_tree_node> kd_tree = nullptr;
	std::shared_ptr<renderer::material> material = nullptr;

	// void recalculate_normals(bool shade_smooth = false);

	// void recalculate_tangents();

	void build_kd_tree(uint8_t depth = 10);
};

}
