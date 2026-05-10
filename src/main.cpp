#include "./pch.hpp"

#include "./gl_app/async_renderer.hpp"
#include "./gl_app/fps_camera.hpp"
#include "./gl_app/fps_tracker.hpp"
#include "./gl_app/path_tracer_gui.hpp"
#include "./gl_app/raymarch_demo_cube.hpp"
#include "./gl_app/shader.hpp"
#include "./gl_app/texture.hpp"
#include "./gl_app/uniform_buffer.hpp"
#include "./gl_app/viewer_shader_src.hpp"
#include "./gl_app/viewport2d.hpp"
#include "./gl_app/window.hpp"

#include "./path_tracing.hpp"
#include "./scene.hpp"
#include <filesystem>

bool mat4_equal(const glm::mat4 &a, const glm::mat4 &b, float eps = 0.0001f) {
    for (int i = 0; i < 4; i++) {
        if (!glm::all(glm::epsilonEqual(a[i], b[i], eps))) {
            return false;
        }
    }
    return true;
}

// convert direction vector to yaw,pitch angles
// yaw: rotation around y axis, 0 = -z direction
// pitch: elevation from horizon, 0 = horizontal, pi/2 = straight up
glm::vec2 dir_to_angles(glm::vec3 dir) {
    dir = glm::normalize(dir);
    
    float yaw = atan2(dir.x, -dir.z);
    float pitch = -asin(dir.y);
    
    return glm::vec2(yaw, pitch);
}
glm::vec3 angles_to_dir(glm::vec2 angles) {
    float yaw = angles.x;
    float pitch = -angles.y;
    
    float cos_pitch = cos(pitch);
    
    return glm::vec3(
        cos_pitch * sin(yaw),
        sin(pitch),
        -cos_pitch * cos(yaw)
    );
}

glm::fvec3 temp_to_tint(float temp) {
    if (temp < 0.0f) {
        return glm::mix(glm::fvec3(1.0f), glm::fvec3(0.2f, 0.5f, 1.0f), -temp);
    } else {
        return glm::mix(glm::fvec3(1.0f), glm::fvec3(1.0f, 0.4f, 0.2f), temp);
    }
}

int main() {
	window win;
	win.open(1280, 720, "Path Tracer");

	viewport2d viewport;
	async_renderer renderer;
	std::unique_ptr<texture> render_output;
	fps_camera camera;

	path_tracer_gui pt_gui(win.glfw_window);
	pt_gui.on_gltf_changed = [&](const std::filesystem::path &path) {
		if (!path.empty()) {
			std::cout << "Load gltf: " << path << std::endl;
			renderer.load_scene(path);
			renderer.with_scene([&](scene &s) {
				camera.set_pose_approx_view_mat(glm::inverse(s.camera_transform));
				float vfov = s.camera_vfov_rad;
				float hfov = 2.0f * glm::atan(glm::tan(vfov * 0.5f) * ((float)pt_gui.width / (float)pt_gui.height));
				if (pt_gui.width > pt_gui.height) {
					camera.major_fov_deg = glm::degrees(hfov);
				} else {
					camera.major_fov_deg = glm::degrees(vfov);
				}
				pt_gui.hdri_file = "";
				pt_gui.hdri_intensity = 0.0f;
				if (glm::length(s.sunlight_dir) > 0.0001) {
					pt_gui.sun_intensity = glm::length(s.sunlight_intensity);
					s.sunlight_intensity = glm::fvec3(pt_gui.sun_intensity) * temp_to_tint(pt_gui.sun_temperature);
					glm::fvec2 angles = dir_to_angles(s.sunlight_dir);
					pt_gui.sun_yaw = angles.x;
					pt_gui.sun_pitch = angles.y;
				}
			});
		}
	};
	pt_gui.on_hdri_changed = [&](const std::filesystem::path &path) {
		if (!path.empty()) {
			std::cout << "Load hdri: " << path << std::endl;
			renderer.load_hdri(path);
			pt_gui.hdri_intensity = 1.0f;
			pt_gui.on_hdri_intensity_changed(1.0f);
			renderer.restart_sampling();
		}
	};
	pt_gui.on_hdri_intensity_changed = [&](float intensity) {
		std::cout << "HDRI intensity: " << intensity << std::endl;
		renderer.with_scene([&](scene &s) {
			s.env_intensity = glm::fvec3(intensity) * temp_to_tint(pt_gui.hdri_temperature);
		});
		renderer.restart_sampling();
	};
	pt_gui.on_hdri_temperature_changed = [&](float temperature) {
		std::cout << "HDRI temperature: " << temperature << std::endl;
		renderer.with_scene([&](scene &s) {
			s.env_intensity = glm::fvec3(pt_gui.hdri_intensity) * temp_to_tint(temperature);
		});
		renderer.restart_sampling();
	};
	pt_gui.on_sun_direction_changed = [&](float yaw, float pitch) {
		std::cout << "Sun direction: " << yaw << ", " << pitch << std::endl;
		renderer.with_scene([&](scene &s) {
			s.sunlight_dir = angles_to_dir(glm::fvec2(yaw, pitch));
		});
		renderer.restart_sampling();
	};
	pt_gui.on_sun_intensity_changed = [&](float intensity) {
		std::cout << "Sun intensity: " << intensity << std::endl;
		renderer.with_scene([&](scene &s) {
			s.sunlight_intensity = glm::fvec3(intensity) * temp_to_tint(pt_gui.sun_temperature);
		});
		renderer.restart_sampling();
	};
	pt_gui.on_sun_temperature_changed = [&](float temperature) {
		std::cout << "Sun temperature: " << temperature << std::endl;
		renderer.with_scene([&](scene &s) {
			s.sunlight_intensity = glm::fvec3(pt_gui.sun_intensity) * temp_to_tint(temperature);
		});
		renderer.restart_sampling();
	};
	pt_gui.on_sun_angular_radius_changed = [&](float angular_radius) {
		std::cout << "Sun angular radius: " << angular_radius << std::endl;
		renderer.with_scene([&](scene &s) {
			s.sunlight_angular_radius = angular_radius;
		});
		renderer.restart_sampling();
	};
	pt_gui.on_pause_toggled = [&](bool paused) {
		std::cout << "Pause toggled: " << paused << std::endl;
		if (paused) {
			renderer.stop();
		} else {
			renderer.start();
		}
	};
	pt_gui.on_reset_samples_requested = [&]() {
		std::cout << "Sampling reset requested" << std::endl;
		renderer.restart_sampling();
	};
	pt_gui.on_max_samples_changed = [&](int samples) {
		std::cout << "Max samples: " << samples << std::endl;
		renderer.set_num_samples_cap(samples);
	};
	pt_gui.on_resolution_changed = [&](int width, int height) {
		std::cout << "Resolution changed: " << width << "x" << height << std::endl;
		renderer.resize(width, height);
		render_output = std::make_unique<texture>(width, height, GL_SRGB8_ALPHA8);
		std::vector<uint8_t> zeros(width * height * 4, 0);
		render_output->upload(zeros.data());
		camera.width = width;
		camera.height = height;
	};
	pt_gui.on_transparent_background_toggled = [&](bool enabled) {
		std::cout << "Transparent background: " << enabled << std::endl;
		renderer.set_transparent_background(enabled);
	};
	pt_gui.on_exposure_changed = [&](float exposure) {
		renderer.set_exposure(exposure);
	};
	pt_gui.on_cuda_toggled = [&](bool enabled) {
		std::cout << "CUDA: " << enabled << std::endl;
		renderer.set_cuda_enabled(enabled);
	};
	// pt_gui.on_denoising_toggled = [](bool enabled) {
	// 	std::cout << "Denoising: " << enabled << std::endl;
	// };
	pt_gui.on_save_requested = [&](const std::filesystem::path &path) {
		if (!path.empty()) {
			std::cout << "Save render: " << path << std::endl;
			renderer.save_frame(path);
		}
	};
	pt_gui.on_reset_pan_and_zoom = [&]() {
		viewport.reset();
	};
	pt_gui.trigger_all_callbacks();

	shader viewer_shader;
	viewer_shader.compile(viewer_shader_vert, viewer_shader_frag);

	GLuint dummy_vao;
	glGenVertexArrays(1, &dummy_vao);

	uniform_buffer viewer_ubo;

	fps_tracker ui_fps_tracker;

	window::loop_info info;

	glm::mat4 last_view_mat, last_proj_mat;

	info.on_update = [&](float dt) {
		pt_gui.ui_fps = ui_fps_tracker.get_fps();
		pt_gui.render_fps = renderer.get_fps();
		pt_gui.current_samples = renderer.num_samples();
		pt_gui.update();

		bool prevent_capture = pt_gui.is_cursor_hovering_over();
		camera.update_fps_pose_from_glfw_input(win, dt, prevent_capture);

		bool rmb_captured = camera.is_cursor_captured();
		viewport.update_from_glfw_input(win, dt, pt_gui.width, pt_gui.height, prevent_capture || rmb_captured);

		glm::mat4 v = camera.calc_view_mat();
		glm::mat4 p = camera.calc_proj_mat();
		if (!mat4_equal(v, last_view_mat) || !mat4_equal(p, last_proj_mat)) {
			renderer.with_scene([&](scene &s) {
				s.camera_transform = glm::inverse(v);
				s.camera_vfov_rad = glm::atan(1.0f / p[1][1]) * 2.0f;
			});
			last_view_mat = v;
			last_proj_mat = p;
			renderer.restart_sampling();
		}
	};

	info.on_draw = [&]() {
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		int win_w, win_h;
		glfwGetFramebufferSize(win.glfw_window, &win_w, &win_h);

		viewer_ubo.update(glm::vec2((float)win_w, (float)win_h), glm::vec2((float)pt_gui.width, (float)pt_gui.height), viewport.pan, viewport.zoom);
		viewer_ubo.bind(0);

		renderer.try_with_next_frame([&](const uint8_t *frame_data) {
			render_output->upload(frame_data);
		});

		viewer_shader.bind();
		render_output->bind(0);
		glBindVertexArray(dummy_vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

		pt_gui.draw();
		ui_fps_tracker.mark_frame();
	};

	info.on_resize = [&](uint32_t w, uint32_t h) {
		glViewport(0, 0, w, h);
	};

	win.run_loop(info);

	return 0;
}
