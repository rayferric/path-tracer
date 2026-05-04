#include "./viewport2d.hpp"
#include "./window.hpp"
#include <algorithm>

void viewport2d::update_from_glfw_input(const window &win, float dt, float tex_w, float tex_h, bool prevent_capture) {
	int win_w, win_h;
	glfwGetFramebufferSize(win.glfw_window, &win_w, &win_h);

	double raw_mx, raw_my;
	glfwGetCursorPos(win.glfw_window, &raw_mx, &raw_my);
	
	// invert mouse Y to match OpenGL coords
	float mouse_x = static_cast<float>(raw_mx);
	float mouse_y = static_cast<float>(win_h) - static_cast<float>(raw_my);

	double current_scroll_x = win.get_scroll_x();
	double current_scroll_y = win.get_scroll_y();

	float scroll_delta_x = static_cast<float>(current_scroll_x - last_scroll_x);
	float scroll_delta_y = static_cast<float>(current_scroll_y - last_scroll_y);
	
	last_scroll_x = current_scroll_x;
	last_scroll_y = current_scroll_y;

	float scroll_delta = scroll_delta_y;

	if (target_zoom < 0.0f) {
		target_zoom = zoom;
		target_pan = pan;
	}

	// zoom
	if (scroll_delta != 0.0f && !prevent_capture) {
		target_zoom = target_zoom * (1.0f + scroll_delta * zoom_sensitivity);
		target_zoom = std::clamp(target_zoom, 0.001f, 1000.0f);
	}

	// pan
	if (glfwGetMouseButton(win.glfw_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (!is_dragging && !prevent_capture) {
			is_dragging = true;
			last_mouse_x = mouse_x;
			last_mouse_y = mouse_y;
		}
	} else {
		is_dragging = false;
	}
	if (is_dragging) {
		target_pan.x += (mouse_x - last_mouse_x);
		target_pan.y += (mouse_y - last_mouse_y);
		last_mouse_x = mouse_x;
		last_mouse_y = mouse_y;
	}

	// smoothing

	float pan_decay = std::pow(pan_smoothing, dt * 60.0f);
	pan = target_pan * (1.0f - pan_decay) + pan * pan_decay;

	float old_zoom = zoom;
	float zoom_decay = std::pow(zoom_smoothing, dt * 60.0f);
	zoom = target_zoom * (1.0f - zoom_decay) + zoom * zoom_decay;

	// update pan whenever zoom changes to keep the cursor position anchored 
	if (std::abs(zoom - old_zoom) > 0.000001f && !prevent_capture) {
		float base_scale = std::min(win_w / tex_w, win_h / tex_h);
		
		float old_scale = base_scale * old_zoom;
		float new_scale = base_scale * zoom;

		float old_draw_w = tex_w * old_scale;
		float old_draw_h = tex_h * old_scale;
		float old_draw_x = (win_w - old_draw_w) * 0.5f + pan.x;
		float old_draw_y = (win_h - old_draw_h) * 0.5f + pan.y;

		float mouse_uv_x = (mouse_x - old_draw_x) / old_draw_w;
		float mouse_uv_y = (mouse_y - old_draw_y) / old_draw_h;

		float new_draw_w = tex_w * new_scale;
		float new_draw_h = tex_h * new_scale;

		float exact_draw_x = mouse_x - mouse_uv_x * new_draw_w;
		float exact_draw_y = mouse_y - mouse_uv_y * new_draw_h;

		float diff_x = exact_draw_x - ((win_w - new_draw_w) * 0.5f) - pan.x;
		float diff_y = exact_draw_y - ((win_h - new_draw_h) * 0.5f) - pan.y;

		pan.x += diff_x;
		pan.y += diff_y;
		target_pan.x += diff_x;
		target_pan.y += diff_y;
	}
}

void viewport2d::reset() {
	target_zoom = 1.0f;
	target_pan = glm::vec2(0.0f, 0.0f);
}
