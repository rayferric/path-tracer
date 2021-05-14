#pragma once

#include "pch.hpp"

#include "scene/entity.hpp"

namespace scene {

std::shared_ptr<entity> load_gltf(
		const std::filesystem::path &path);

}
