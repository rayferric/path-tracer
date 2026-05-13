#pragma once

#include "../pch.hpp"

class window {
public:
	struct loop_info {
		std::function<void(float)> on_update = nullptr;
		std::function<void()> on_draw = nullptr;
		std::function<void(uint32_t, uint32_t)> on_resize = nullptr;
	};

	GLFWwindow *glfw_window;

	double get_scroll_x() const {
		return scroll_x;
	}
	double get_scroll_y() const {
		return scroll_y;
	}

	window();
	~window();

	void open(uint32_t w, uint32_t h, const std::string &title);
	void run_loop(const loop_info &info);

private:
	window::loop_info cur_loop_info;
	double scroll_x = 0.0;
	double scroll_y = 0.0;

	static void fbsz_cb(GLFWwindow *window, int width, int height);
	static void scroll_cb(GLFWwindow *window, double xoffset, double yoffset);
};
