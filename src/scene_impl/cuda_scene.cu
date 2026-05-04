#include "../scene.hpp"

template <typename T> static void cuda_alloc_copy(T **gpu_ptr, size_t count) {
	if (count == 0) {
		*gpu_ptr = nullptr;
		return;
	}

	// gpu_ptr is set to cpu address
	const T *cpu_ptr = *gpu_ptr;

	cudaMalloc(gpu_ptr, count * sizeof(T));
	cudaMemcpy(*gpu_ptr, cpu_ptr, count * sizeof(T), cudaMemcpyHostToDevice);
}

// public api impl below

cuda_scene::cuda_scene(const scene &src) {
	// directly copy all values
	memcpy(static_cast<scene_data *>(this), static_cast<const scene_data*>(&src), sizeof(scene_data));

	// ...then fix-up buffers:
	cuda_alloc_copy(&texture_data, texture_data_sz);
	cuda_alloc_copy(&textures, num_textures);
	cuda_alloc_copy(&materials, num_materials);
	cuda_alloc_copy(&triangles_ext, num_triangles);
	cuda_alloc_copy(&triangles, num_triangles);
	cuda_alloc_copy(&bvh_nodes, num_bvh_nodes);
}

cuda_scene::~cuda_scene() {
	cudaFree(this->texture_data);
	cudaFree(this->textures);
	cudaFree(this->materials);
	cudaFree(this->triangles_ext);
	cudaFree(this->triangles);
	cudaFree(this->bvh_nodes);
}

void cuda_scene::copy_lightweight_data(const scene &src) {
	this->camera_transform = src.camera_transform;
	this->camera_vfov_rad = src.camera_vfov_rad;

	this->sunlight_dir = src.sunlight_dir;
	this->sunlight_intensity = src.sunlight_intensity;
	this->sunlight_angular_radius = src.sunlight_angular_radius;

	this->env_intensity = src.env_intensity;
}
