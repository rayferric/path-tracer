#pragma once

#include "pch.hpp"

#include "scene/entity.hpp"

namespace core {

class renderer {
public:
	uvec2 resolution;
	std::shared_ptr<entity> root;

	void load_gltf(
			const std::filesystem::path &path);
};

}
