#pragma once

#include "pch.hpp"

#include "core/aabb.hpp"
#include "core/material.hpp"
#include "core/mesh.hpp"
#include "scene/component.hpp"
#include "scene/transform.hpp"

namespace scene {

class model : public scene::component {
public:
	struct surface {
		std::shared_ptr<core::mesh> mesh;
		std::shared_ptr<core::material> material;
	};

	struct intersection {
		float distance = -1;
		fvec3 position;
		fvec2 tex_coord;
		fvec3 normal;
		fvec3 tangent;
		std::shared_ptr<core::material> material;

		bool has_hit() const;
	};

	std::vector<surface> surfaces;
	core::aabb aabb;

	void recalculate_aabb();

	intersection intersect(const ray &ray) const;
};

}
