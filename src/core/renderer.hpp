#pragma once

#include "pch.hpp"

#include "image/texture.hpp"
#include "scene/camera.hpp"
#include "scene/entity.hpp"

namespace core {

class renderer {
public:
	uvec2 resolution;
	std::shared_ptr<entity> root;
	std::shared_ptr<scene::camera> camera;
	std::shared_ptr<image::texture> environment;

	void load_gltf(
			const std::filesystem::path &path);

	void render(const std::filesystem::path &path) const;

private:
	struct trace_result {
		bool hit;
		math::fvec3 position;
		math::fvec2 tex_coord;
		math::fvec3 normal;
		math::fvec3 tangent;
		std::shared_ptr<core::material> material;
	};


	trace_result trace(const ray &ray) const;

	fvec3 integrate(const ray &ray) const;
};

}
