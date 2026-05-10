#pragma once

#include "../pch.hpp"

#include "../scene.hpp"
#include "../path_tracing.hpp"

#include "./fps_tracker.hpp"
#include "./tonemapping.hpp"

class async_renderer {
public:
    async_renderer() {
		running = false;
		use_cuda = false;
		num_samples_cap = 1000;
		sampling_restart_requested = false;

		resize(640, 360);
    }
	~async_renderer() {
		stop();
		#ifdef ENABLE_CUDA
		if (hdr_buf_cuda) {
			cuda_free(hdr_buf_cuda);
		}
		#endif
	}

    void resize(int width, int height) {
		stop();
        this->width = width;
        this->height = height;
        buf_a.resize(width * height * 4, 255);
        buf_b.resize(width * height * 4, 255);
        front_buf = buf_a.data();
        back_buf = buf_b.data();
        hdr_buf.resize(width * height);
		std::fill(hdr_buf.begin(), hdr_buf.end(), glm::fvec4(0.0f));
		#ifdef ENABLE_CUDA
		if (hdr_buf_cuda) {
			cuda_free(hdr_buf_cuda);
		}
		#endif
		hdr_buf_cuda = nullptr;
		frame_ready = false;
		current_sample_idx = 0;
		start();
    }

	void set_cuda_enabled(bool enabled) {
		use_cuda = enabled;
	}

	void set_transparent_background(bool enabled) {
		transparent_background = enabled;
		restart_sampling();
	}

	void set_exposure(float value) {
		exposure = value;
	}

	void set_num_samples_cap(int samples) {
		num_samples_cap = samples;
	}

	void start() {
		if (running) {
			return;
		}

		running = true;
		render_thread = std::thread([this]() {
			render_loop();
		});
	}

	void stop() {
		if (!running) {
			return;
		}

		running = false;
		if (render_thread.joinable()) {
			render_thread.join();
		}
	}

	// load a scene from gltf file
	// builds BVH and copies to GPU automatically
	void load_scene(const std::filesystem::path &path) {
		std::lock_guard<std::mutex> lock(scene_mutex);
		scn.reset();
		scn = std::make_unique<scene>(path);
#ifdef ENABLE_CUDA
		cuda_scn = nullptr;
#endif
	}

	void load_hdri(const std::filesystem::path &path) {
		std::lock_guard<std::mutex> lock(scene_mutex);
		scn->load_hdri(path);
		#ifdef ENABLE_CUDA
		cuda_scn = nullptr;
		#endif
	}

	// access the scene with automatic locking
	// use like: renderer.with_scene([](scene &s) { s.camera_transform = ...; });
	void with_scene(const std::function<void(scene &)> &func) {
		std::lock_guard<std::mutex> lock(scene_mutex);
		if (scn) {
			func(*scn);
		}
	}

	void with_last_frame(const std::function<void(const uint8_t *)> &func) {
		std::lock_guard<std::mutex> lock(swap_mutex);
		func(front_buf);
	}
	void try_with_next_frame(const std::function<void(const uint8_t *)> &func) {
		if (frame_ready) {
			frame_ready = false;
			with_last_frame(func);
		}
	}

	void save_frame(const std::filesystem::path &path) {
		with_last_frame([&](const uint8_t *data) {
			stbi_write_png(path.string().c_str(), width, height, 4, data, width * 4);
		});
	}

	float get_fps() const {
		return fps_tracking.get_fps();
	}

	int num_samples() const {
		return current_sample_idx;
	}

	void restart_sampling() {
		sampling_restart_requested = true;
	}

private:
	void render_loop() {
		while (running) {
			if (sampling_restart_requested) {
				current_sample_idx = 0;
				sampling_restart_requested = false;
			}

			bool rendered_anything = false;
			if (current_sample_idx == 0) {
				std::fill(hdr_buf.begin(), hdr_buf.end(), glm::fvec4(0.0f));
			}
			if (current_sample_idx < num_samples_cap) {
				std::lock_guard<std::mutex> lock(scene_mutex);
				if (scn) {
					#ifdef ENABLE_CUDA
					if (cuda_available() && use_cuda) {
						if (!cuda_scn) {
							cuda_scn = std::make_unique<cuda_scene>(*scn);
						}
						cuda_scn->copy_lightweight_data(*scn);
						if (!hdr_buf_cuda) {
							hdr_buf_cuda = static_cast<glm::fvec4 *>(cuda_malloc(width * height * sizeof(glm::fvec4)));
						}
						cuda_memcpy(hdr_buf_cuda, hdr_buf.data(), width * height * sizeof(glm::fvec4), cuda_memcpy_kind::host_to_device);
						render_sample_cuda(*cuda_scn, hdr_buf_cuda, width, height, current_sample_idx, transparent_background);
						cuda_memcpy(hdr_buf.data(), hdr_buf_cuda, width * height * sizeof(glm::fvec4), cuda_memcpy_kind::device_to_host);
					}
					#else
					if (false) {}
					#endif

					else {
						render_sample(*scn, hdr_buf.data(), width, height, current_sample_idx, transparent_background);
					}
					rendered_anything = true;
					current_sample_idx++;
				}
			}
			if (rendered_anything) {
				tonemap_buffer_srgb8(back_buf, hdr_buf.data(), hdr_buf.size(), exposure);
				fps_tracking.mark_frame();

				// swap buffers
				{
					std::lock_guard<std::mutex> lock(swap_mutex);
					std::swap(front_buf, back_buf);
					frame_ready = true;
				}
			}
		}
	}

	std::thread render_thread;
	std::atomic<bool> running;
	std::atomic<bool> use_cuda;
	std::atomic<bool> transparent_background = false;
	std::atomic<float> exposure = 1.0f;

	fps_tracker fps_tracking;

	int width, height;
	std::vector<uint8_t> buf_a;
	std::vector<uint8_t> buf_b;
	uint8_t *front_buf;
	uint8_t *back_buf;
	std::mutex swap_mutex;
	std::vector<glm::fvec4> hdr_buf;
	std::atomic<bool> frame_ready;
	std::atomic<int> current_sample_idx;
	std::atomic<int> num_samples_cap;
	std::atomic<bool> sampling_restart_requested;

	std::mutex scene_mutex;
	std::unique_ptr<scene> scn;
    #ifdef ENABLE_CUDA
	std::unique_ptr<cuda_scene> cuda_scn;
	glm::fvec4 *hdr_buf_cuda = nullptr;
    #endif
};
