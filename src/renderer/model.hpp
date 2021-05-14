#pragma once

#include "pch.hpp"

#include "renderer/material.hpp"
#include "renderer/mesh.hpp"
#include "scene/component.hpp"

namespace renderer {

class model : public scene::component {
public:
	struct surface {
		std::shared_ptr<mesh> mesh;
		std::shared_ptr<material> material;
	};

	const std::vector<surface> &get_surfaces() const;

	void add_surface(const surface &surface);

private:
	std::vector<surface> surfaces;
};

}
