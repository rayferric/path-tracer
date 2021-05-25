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

	std::vector<surface> surfaces;
};

}
