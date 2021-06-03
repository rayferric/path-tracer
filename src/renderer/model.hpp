#pragma once

#include "pch.hpp"

#include "renderer/material.hpp"
#include "renderer/mesh.hpp"
#include "scene/component.hpp"

namespace renderer {

class model : public scene::component {
public:
	struct surface {
		std::shared_ptr<renderer::mesh> mesh;
		std::shared_ptr<renderer::material> material;
	};

	struct intersection {
		float distance;
		size_t surface_index;
		
	};

	std::vector<surface> surfaces;

	intersection intersect(const ray &ray) const;
};

}
