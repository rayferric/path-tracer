#include "../scene.hpp"
#include "scene_data.hpp"

void scene::load_hdri(const std::filesystem::path &file) {
	// unload existing env texture if present
	if (env_tex_idx >= 0) {
		uint32_t old_idx = static_cast<uint32_t>(env_tex_idx);
		assert(old_idx == num_textures - 1 && "Environment texture should be the last one.");
		num_textures--;

		texture_info removed_info = textures[old_idx];
		assert(removed_info.format == texture_format::hdr);
		texture_data_sz -= removed_info.width * removed_info.height * removed_info.channels * sizeof(float);
		
		env_tex_idx = -1;
	}
	
	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	float *data = stbi_loadf(file.string().c_str(), &width, &height, &channels, 0);
	stbi_set_flip_vertically_on_load(false);
	
	if (!data) {
		throw std::runtime_error("Failed to load HDRI: " + file.string());
	}

	uint32_t pixel_count = width * height;
	uint32_t data_size = pixel_count * channels * sizeof(float);
	
	// reallocate texture_data to fit new texture
	uint32_t new_offset = texture_data_sz;
	uint8_t *new_texture_data = (uint8_t*)realloc(texture_data, texture_data_sz + data_size);
	if (!new_texture_data) {
		stbi_image_free(data);
		throw std::runtime_error("Failed to allocate texture data");
	}
	texture_data = new_texture_data;
	
	// copy hdri data
	memcpy(texture_data + new_offset, data, data_size);
	texture_data_sz += data_size;
	stbi_image_free(data);
	
	// add texture_info entry
	texture_info *new_textures = (texture_info*)realloc(textures, (num_textures + 1) * sizeof(texture_info));
	if (!new_textures) {
		throw std::runtime_error("Failed to allocate texture info");
	}
	textures = new_textures;
	
	textures[num_textures] = {
		.data_offset = new_offset,
		.width = static_cast<uint32_t>(width),
		.height = static_cast<uint32_t>(height),
		.channels = static_cast<uint8_t>(channels),
		.format = texture_format::hdr
	};
	
	env_tex_idx = num_textures;
	num_textures++;
}
