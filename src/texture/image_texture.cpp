#include "texture/image_texture.hpp"

#include "math/math.hpp"

image_texture::image_texture(const std::shared_ptr<image> &img)
		: img(img) {}

math::fvec4 image_texture::sample(const math::fvec2 &coord) const {
	// Nearest
	
	// const math::uvec2 &size = img->get_size();
	// auto pixel = math::uvec2(coord.x * size.x, (1 - coord.y) * size.y) % size;

	// return read_pixel(pixel);

	// Bilinear

	const math::uvec2 &size = img->get_size();
	math::fvec2 center(coord.x * size.x - 0.5F, (1 - coord.y) * size.y - 0.5F);

	auto tl = math::uvec2(math::floor(center.x), math::floor(center.y)) % size;
	auto tr = math::uvec2( math::ceil(center.x), math::floor(center.y)) % size;
	auto bl = math::uvec2(math::floor(center.x),  math::ceil(center.y)) % size;
	auto br = math::uvec2( math::ceil(center.x),  math::ceil(center.y)) % size;

	math::fvec2 delta = math::fract(center);

	math::fvec4 t = math::lerp(read_pixel(tl), read_pixel(tr), delta.x);
	math::fvec4 b = math::lerp(read_pixel(bl), read_pixel(br), delta.x);

	return math::lerp(t, b, delta.y);
}

math::fvec4 image_texture::read_pixel(const math::uvec2 &pixel) const {
	math::fvec4 color;

	switch (img->get_channel_count()) {
	case 4:
		color.w = img->read(pixel, 3);
	case 3:
		color.z = img->read(pixel, 2);
	case 2:
		color.y = img->read(pixel, 1);
	case 1:
		color.x = img->read(pixel, 0);
	}

	return color;
}
