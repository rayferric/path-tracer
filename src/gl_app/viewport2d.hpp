#pragma once

#include "../pch.hpp"

class window;

class viewport2d {
public:
	float zoom_sensitivity = 0.1f;
	float zoom_smoothing = 0.7f;
	float pan_smoothing = 0.7f;

	float zoom = 1.0f;
	glm::vec2 pan{0.0f, 0.0f};

	void update_from_glfw_input(const window &win, float dt, float tex_w, float tex_h, bool prevent_capture);

	void reset();

private:
	float target_zoom = -1.0f; // uninitialized
	glm::vec2 target_pan{0.0f, 0.0f};

	bool is_dragging = false;
	float last_mouse_x = 0.0f;
	float last_mouse_y = 0.0f;
	double last_scroll_x = 0.0;
	double last_scroll_y = 0.0;
};
