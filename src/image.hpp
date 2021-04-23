#pragma once

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include "vec2.hpp"

class image {
public:
	image(uvec2 size, uint32_t channel_count, bool hdr, bool srgb) :
			size(size),
			channel_count(channel_count),
			hdr(hdr), srgb(srgb) {
		assert((size.x != 0 && size.y != 0)
				&& "Image size can't be zero.");
		assert(channel_count != 0
				&& "At least one channel is required.");
		assert(channel_count < 5
				&& "No more than 4 channels are supported.");

		data.resize(size.x * size.y * channel_count * (hdr ? 4 : 1));
	}

	static std::shared_ptr<image> load(const std::filesystem::path &path, bool srgb) {
		auto img = std::make_shared<image>();

		img->hdr = stbi_is_hdr(path.c_str());
		img->srgb = srgb;

		uint8_t *data;
		if (img->hdr) {
			data = reinterpret_cast<uint8_t *>(stbi_loadf(path.string().c_str(),
					reinterpret_cast<int32_t *>(&img->size.x),
					reinterpret_cast<int32_t *>(&img->size.y),
					reinterpret_cast<int32_t *>(&img->channel_count), 0));
		} else {
			data = stbi_load(path.string().c_str(),
					reinterpret_cast<int32_t *>(&img->size.x),
					reinterpret_cast<int32_t *>(&img->size.y),
					reinterpret_cast<int32_t *>(&img->channel_count), 0);
		}
		
		uint32_t byte_length = img->size.x * img->size.y
				* img->channel_count * (img->hdr ? 4 : 1);
		img->data = std::vector<uint8_t>(data, data + byte_length);
	}

	void save(const std::filesystem::path &path) const {
		std::string ext = path.extension().string();

		if (ext == ".png") {
			if (hdr)
				throw std::invalid_argument("Can't save HDR image as PNG.");

			stbi_write_png(path.string().c_str(), size.x, size.y,
					channel_count, data.data(), size.x * channel_count);
		} else if (ext == ".hdr") {
			if (hdr)
				throw std::invalid_argument("Can't save LDR image as HDR.");

			stbi_write_hdr(path.string().c_str(), size.x, size.y, channel_count,
					reinterpret_cast<const float *>(data.data()));
		} else
			throw std::invalid_argument("Unknown file extension.");
	}

	float read(const uvec2 &pos, uint32_t channel) const;

	void write(const uvec2 &pos, uint32_t channel, float value);

	void for_each(std::function<float(uvec2, uint32_t)> callback);

	uvec2 get_size() const;

	uint32_t get_channel_count() const;

	bool is_hdr() const;

	bool is_srgb() const;

private:
	uvec2 size;
	uint32_t channel_count;
	bool hdr, srgb;
	std::vector<uint8_t> data;

	image();
};
