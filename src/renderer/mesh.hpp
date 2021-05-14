#pragma once

#include "pch.hpp"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "renderer/aabb.hpp"

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
	aabb aabb;

	// void recalculate_normals(bool shade_smooth = false);

	// void recalculate_tangents();

	void recalculate_aabb();
};

}
