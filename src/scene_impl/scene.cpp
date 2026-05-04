#include "../scene.hpp"

scene::scene(const std::filesystem::path &gltf_file) {
	this->texture_data = nullptr;
	texture_data_sz = 0;
	this->textures = nullptr;
	num_textures = 0;
	this->materials = nullptr;
	num_materials = 0;
	this->triangles = nullptr;
	this->triangles_ext = nullptr;
	num_triangles = 0;
	this->bvh_nodes = nullptr;
	num_bvh_nodes = 0;

	camera_transform = glm::mat4(1.0f);
	camera_vfov_rad = glm::radians(45.0f);

	sunlight_dir = glm::fvec3(0.0f);
	sunlight_intensity = glm::fvec3(0.0f);
	sunlight_angular_radius = 0.0f;

	env_intensity = glm::fvec3(0.0f);
	env_tex_idx = -1;

	load_gltf(gltf_file);
	build_bvh();
}

scene::~scene() {
	free(texture_data);
	free(textures);
	free(materials);
	free(triangles_ext);
	free(triangles);
	free(bvh_nodes);
}
