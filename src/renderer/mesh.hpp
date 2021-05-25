#pragma once

#include "pch.hpp"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "renderer/bvh.hpp"
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
	std::shared_ptr<bvh_node> bvh = nullptr;
	std::shared_ptr<renderer::material> material = nullptr;

	// void recalculate_normals(bool shade_smooth = false);

	// void recalculate_tangents();

	void build_bvh(uint32_t triangles_per_node = 50, float tolerance = 0.9F);
};

}
