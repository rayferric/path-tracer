#include "texture.hpp"

texture::texture(int width, int height, GLenum internal_format) {
	this->width = width;
	this->height = height;

	glGenTextures(1, &tex_id);

	glBindTexture(GL_TEXTURE_2D, tex_id);

	GLenum format;
	switch (internal_format) {
		case GL_RGBA8:
		case GL_SRGB8_ALPHA8:
			format = GL_RGBA;
			break;
		case GL_RGB8:
		case GL_SRGB8:
			format = GL_RGB;
			break;
		default:
			throw std::runtime_error("Unsupported internal format: " + std::to_string(internal_format));
	}
	this->format = format;

	glTexImage2D(
	    GL_TEXTURE_2D,
	    0,
	    internal_format,
	    width,
	    height,
	    0,
	    format,
	    GL_UNSIGNED_BYTE,
	    nullptr // no data
	);

	// glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

texture::~texture() {
	glDeleteTextures(1, &tex_id);
}

void texture::upload(const void *data) {
	glBindTexture(GL_TEXTURE_2D, tex_id);
	
	glTexSubImage2D(
	    GL_TEXTURE_2D,
	    0,
	    0, 0,
	    this->width, this->height,
	    this->format,
	    GL_UNSIGNED_BYTE,
	    data
	);

	glGenerateMipmap(GL_TEXTURE_2D);
}

void texture::bind(uint32_t binding) {
	glActiveTexture(GL_TEXTURE0 + binding);
	glBindTexture(GL_TEXTURE_2D, tex_id);
}
