#pragma once

#include "../pch.hpp"

class texture {
public:
	texture(int width, int height, GLenum internal_format = GL_SRGB8_ALPHA8);
	~texture();
	texture(texture &&t) : tex_id(t.tex_id) {
		t.tex_id = 0;
	}

	void upload(const void *data);
	void bind(uint32_t binding = 0);

private:
	GLuint tex_id;
	int width, height;
	GLenum format;
};
