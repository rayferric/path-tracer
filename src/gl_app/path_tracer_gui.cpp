#include "./path_tracer_gui.hpp"
#include "imgui.h"

void path_tracer_gui::update() {
	gui.begin();
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSizeConstraints(ImVec2(275, 0), ImVec2(275, FLT_MAX));
	ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

	if (ImGui::Button("Select glTF File")) {
		auto selection = pfd::open_file("Select a file", ".", {"GLTF Scenes (*.gltf *.glb)", "*.gltf *.glb"}).result();
		if (!selection.empty()) {
			gltf_file = selection[0];
			on_gltf_changed(gltf_file);
		}
	}
	ImGui::SameLine();
	ImGui::Text("%s", gltf_file.empty() ? "No file selected" : gltf_file.filename().string().c_str());

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::GetStyle().Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
	if (ImGui::CollapsingHeader("Environment", ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
		ImGui::Spacing();
		if (ImGui::Button("Select HDRI File")) {
			auto selection = pfd::open_file("Select a file", ".", {"HDRI Envmaps (*.hdr)", "*.hdr"}).result();
			if (!selection.empty()) {
				hdri_file = selection[0];
				on_hdri_changed(hdri_file);
			}
		}
		ImGui::SameLine();
		ImGui::Text("%s", hdri_file.empty() ? "No file selected" : hdri_file.filename().string().c_str());

		if (labeled_slider("HDRI Intensity", &hdri_intensity, 0.0f, 10.0f)) {
			on_hdri_intensity_changed(hdri_intensity);
		}
		if (labeled_slider("HDRI Temperature", &hdri_temperature, -1.0f, 1.0f)) {
			on_hdri_temperature_changed(hdri_temperature);
		}
		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Sun Light", ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
		ImGui::BeginGroup();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Sun Yaw");
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetStyle().ItemSpacing.x * 0.5f);
		bool yaw_changed = ImGui::SliderFloat("##sun_yaw", &sun_yaw, -3.14f, 3.14f);
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Sun Pitch");
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		bool pitch_changed = ImGui::SliderFloat("##sun_pitch", &sun_pitch, -1.57f, 1.57f);
		ImGui::EndGroup();

		if (yaw_changed || pitch_changed) {
			on_sun_direction_changed(sun_yaw, sun_pitch);
		}

		if (labeled_slider("Sun Intensity", &sun_intensity, 0.0f, 100.0f)) {
			on_sun_intensity_changed(sun_intensity);
		}

		if (labeled_slider("Sun Temperature", &sun_temperature, -1.0f, 1.0f)) {
			on_sun_temperature_changed(sun_temperature);
		}

		if (labeled_slider("Angular Radius", &sun_angular_radius, 0.0f, 0.1f)) {
			on_sun_angular_radius_changed(sun_angular_radius);
		}
		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Sampling", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
		ImGui::Text("Samples: %d", current_samples);
		float button_width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
		ImVec4 button_color = paused ? ImVec4(0.8f, 0.2f, 0.2f, 1.0f) : ImGui::GetStyleColorVec4(ImGuiCol_Button);
		ImGui::PushStyleColor(ImGuiCol_Button, button_color);
		if (ImGui::Button(paused ? "Resume" : "Pause", ImVec2(button_width, 0))) {
			paused = !paused;
			on_pause_toggled(paused);
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (ImGui::Button("Reset", ImVec2(button_width, 0))) {
			on_reset_samples_requested();
		}

		ImGui::SetNextItemWidth((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f);
		if (ImGui::InputInt(" Max Samples", &max_samples)) {
			on_max_samples_changed(max_samples);
		}
		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Output", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
		struct predef_res {
			int width;
			int height;
			std::string label;
			bool is_custom;
		};
		std::vector<predef_res> predef_resolutions = {
		    {640, 360, "640x360 (SD)", false}, {1280, 720, "1280x720 (HD)", false}, {1920, 1080, "1920x1080 (Full HD)", false}, {2560, 1440, "2560x1440 (QHD)", false}, {3840, 2160, "3840x2160 (4K)", false}, {0, 0, "Custom", true} // placeholder for custom
		};

		// find current resolution
		int current_choice = -1;
		for (int i = 0; i < predef_resolutions.size() - 1; i++) {
			if (width == predef_resolutions[i].width && height == predef_resolutions[i].height) {
				current_choice = i;
				break;
			}
		}
		if (current_choice < 0) {
			selecting_custom_res = true;
		}
		if (selecting_custom_res) {
			current_choice = predef_resolutions.size() - 1;
		}

		ImGui::Spacing();
		ImGui::Text("Resolution:");
		float menu_width = ImGui::GetContentRegionAvail().x;
		ImGui::SetNextItemWidth(menu_width);
		if (ImGui::BeginCombo("##res_combo", predef_resolutions[current_choice].label.c_str())) {
			for (int i = 0; i < predef_resolutions.size(); i++) {
				bool is_selected = (current_choice == i);
				if (ImGui::Selectable(predef_resolutions[i].label.c_str(), is_selected)) {
					current_choice = i;
					selecting_custom_res = predef_resolutions[i].is_custom;
					if (!selecting_custom_res) {
						width = predef_resolutions[i].width;
						height = predef_resolutions[i].height;
						on_resolution_changed(width, height);
					} else {
						unapplied_custom_width = width;
						unapplied_custom_height = height;
					}
				}
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if (selecting_custom_res) {
			ImGui::Spacing();

			ImGui::SetNextItemWidth(menu_width * 0.35f);
			ImGui::InputInt("##custom_width", &unapplied_custom_width, 0, 0);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(menu_width * 0.1f);
			ImGui::Text("x");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(menu_width * 0.35f);
			ImGui::InputInt("##custom_height", &unapplied_custom_height, 0, 0);
			ImGui::SameLine();
			if (ImGui::Button("Apply", ImVec2(ImGui::GetContentRegionAvail().x, 20.0f))) {
				width = unapplied_custom_width;
				height = unapplied_custom_height;
				on_resolution_changed(width, height);
			}
		}

		ImGui::Spacing();
		if (ImGui::Checkbox("Transparent background", &transparent_background)) {
			on_transparent_background_toggled(transparent_background);
		}

		if (labeled_slider("Exposure", &exposure, 0.0f, 10.0f)) {
			on_exposure_changed(exposure);
		}
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (!cuda_available()) {
		ImGui::BeginDisabled();
	}
	if (ImGui::Checkbox("Use CUDA", &cuda)) {
		on_cuda_toggled(cuda);
	}
	if (!cuda_available()) {
		ImGui::EndDisabled();
	}
	// ImGui::SameLine();
	// if (ImGui::Checkbox("Denoise", &denoising)) {
	// 	on_denoising_toggled(denoising);
	// }

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::Button("Save Render", ImVec2(-1.0f, 30.0f))) {
		auto destination = pfd::save_file("Save render as", "render.png", {"PNG Files (*.png)", "*.png"}).result();
		if (!destination.empty()) {
			on_save_requested(destination);
		}
	}

	ImGui::Spacing();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
	if (ImGui::Button("Reset Pan & Zoom", ImVec2(-1.0f, 20.0f))) {
		on_reset_pan_and_zoom();
	}
	ImGui::PopStyleColor();

	// fps stats - ui fps on left, render fps at half width
	ImGui::Spacing();
	float half_width = ImGui::GetContentRegionAvail().x * 0.5f + ImGui::GetStyle().ItemSpacing.x;
	ImGui::Text("UI FPS:");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%.1f", ui_fps);
	ImGui::SameLine();
	ImGui::SetCursorPosX(half_width);
	ImGui::Text("Render FPS:");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%.1f", render_fps);

	ImGui::End();
	gui.end();
}

void path_tracer_gui::trigger_all_callbacks() {
	on_gltf_changed(gltf_file);
	on_hdri_changed(hdri_file);
	on_hdri_intensity_changed(hdri_intensity);
	on_sun_direction_changed(sun_yaw, sun_pitch);
	on_sun_intensity_changed(sun_intensity);
	on_sun_angular_radius_changed(sun_angular_radius);
	on_pause_toggled(paused);
	on_reset_samples_requested();
	on_max_samples_changed(max_samples);
	on_resolution_changed(width, height);
	on_cuda_toggled(cuda);
	on_denoising_toggled(denoising);
	on_reset_pan_and_zoom();
}
