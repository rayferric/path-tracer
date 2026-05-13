#pragma once

#include "../pch.hpp"

#include "./imgui.hpp"

class path_tracer_gui {
public:
	std::filesystem::path gltf_file = "";
	std::filesystem::path hdri_file = "";
	float hdri_intensity = 0.0f;
	float hdri_temperature = -0.5f;

	float sun_yaw = 0.785f, sun_pitch = 0.785f;
	float sun_intensity = 0.0f;
	float sun_temperature = 0.5f;
	float sun_angular_radius = 0.05f;

	int current_samples = 0;
	bool paused = false;
	int max_samples = 1000;

	bool cuda = false;
	bool denoising = false;

	bool transparent_background = false;
	float exposure = 1.0f;

	int width = 640, height = 360;
	float ui_fps = 0.0f, render_fps = 0.0f;

	path_tracer_gui(GLFWwindow *glfw_window) : gui(glfw_window) {}

	void update();
	void trigger_all_callbacks();

	void draw() {
		gui.draw();
	}

	bool is_cursor_hovering_over() const {
		return gui.is_cursor_hovering_over();
	}

	// callbacks for gui events - assign these from outside
	std::function<void(const std::filesystem::path &)> on_gltf_changed = [](const auto &) {};
	std::function<void(const std::filesystem::path &)> on_hdri_changed = [](const auto &) {};
	std::function<void(float)> on_hdri_intensity_changed = [](float) {};
	std::function<void(float)> on_hdri_temperature_changed = [](float) {};
	std::function<void(float, float)> on_sun_direction_changed = [](float, float) {};
	std::function<void(float)> on_sun_intensity_changed = [](float) {};
	std::function<void(float)> on_sun_temperature_changed = [](float) {};
	std::function<void(float)> on_sun_angular_radius_changed = [](float) {};
	std::function<void(bool)> on_pause_toggled = [](bool) {};
	std::function<void()> on_reset_samples_requested = []() {};
	std::function<void(int)> on_max_samples_changed = [](int) {};
	std::function<void(int, int)> on_resolution_changed = [](int, int) {};
	std::function<void(bool)> on_transparent_background_toggled = [](bool) {};
	std::function<void(float)> on_exposure_changed = [](float) {};
	std::function<void(bool)> on_cuda_toggled = [](bool) {};
	std::function<void(bool)> on_denoising_toggled = [](bool) {};
	std::function<void(const std::filesystem::path &)> on_save_requested = [](const auto &) {};
	std::function<void()> on_reset_pan_and_zoom = []() {};

private:
	imgui gui;

	bool selecting_custom_res = false;
	int unapplied_custom_width = 0, unapplied_custom_height = 0;

	bool labeled_slider(const char *label, float *value, float min, float max, float width_fraction = 1.0f) {
		ImGui::BeginGroup();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("%s", label);
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * width_fraction);
		std::string id = "##" + std::string(label);
		bool changed = ImGui::SliderFloat(id.c_str(), value, min, max);
		ImGui::EndGroup();
		return changed;
	}
};
