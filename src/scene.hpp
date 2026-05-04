#pragma once

#include "./pch.hpp"

#include "./scene_impl/scene_data.hpp"

class scene : public scene_data {
public:
	scene(const std::filesystem::path &gltf_file);
	~scene();

	void load_hdri(const std::filesystem::path &file);

private:
	void load_gltf(const std::filesystem::path &file);
	void build_bvh();
};

#ifdef ENABLE_CUDA
class cuda_scene : public scene_data {
public:
	cuda_scene(const scene &src);
	~cuda_scene();

	void copy_lightweight_data(const scene &src);
};
#endif
