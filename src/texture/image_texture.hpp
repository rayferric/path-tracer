#pragma once

#include "pch.hpp"

#include "math/vec2.hpp"
#include "math/vec4.hpp"
#include "texture/image.hpp"
#include "texture/texture.hpp"

class image_texture : public texture {
public:
	image_texture(const std::shared_ptr<image> &img);

	math::fvec4 sample(const math::fvec2 &coord) const override; 

private:
	std::shared_ptr<image> img;

	math::fvec4 read_pixel(const math::uvec2 &pixel) const;
};
