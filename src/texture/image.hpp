#pragma once

#include "pch.hpp"

#include "math/vec2.hpp"

class image {
public:
	image(math::uvec2 size, uint32_t channel_count, bool hdr, bool srgb);

	static std::shared_ptr<image> load(const std::filesystem::path &path, bool srgb);

	void save(const std::filesystem::path &path) const;

	float read(const math::uvec2 &pos, uint32_t channel) const;

	void write(const math::uvec2 &pos, uint32_t channel, float value);

	const math::uvec2 &get_size() const;

	uint32_t get_channel_count() const;

	bool is_hdr() const;

	bool is_srgb() const;

private:
	math::uvec2 size;
	uint32_t channel_count;
	bool hdr, srgb;
	std::vector<uint8_t> data;

	image();
};
