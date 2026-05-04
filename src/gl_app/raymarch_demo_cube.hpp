#pragma once

#include "../pch.hpp"

inline float sd_box(glm::vec3 p, glm::vec3 b) {
	glm::vec3 d = glm::abs(p) - b;
	return glm::length(glm::max(d, glm::vec3(0.0f))) + std::min(std::max(d.x, std::max(d.y, d.z)), 0.0f);
}

inline glm::vec3 calc_normal(glm::vec3 p) {
	const float eps = 0.001f;
	const glm::vec2 h(eps, 0.0f);
	return glm::normalize(glm::vec3(sd_box(p + glm::vec3(h.x, h.y, h.y), glm::vec3(1.0f)) - sd_box(p - glm::vec3(h.x, h.y, h.y), glm::vec3(1.0f)), sd_box(p + glm::vec3(h.y, h.x, h.y), glm::vec3(1.0f)) - sd_box(p - glm::vec3(h.y, h.x, h.y), glm::vec3(1.0f)), sd_box(p + glm::vec3(h.y, h.y, h.x), glm::vec3(1.0f)) - sd_box(p - glm::vec3(h.y, h.y, h.x), glm::vec3(1.0f))));
}

// outputs srgb8
inline void raymarch_demo_cube(uint8_t *out_buf, int width, int height, const glm::mat4 &view, const glm::mat4 &proj) {
	glm::mat4 inv_proj = glm::inverse(proj);
	glm::mat4 inv_view = glm::inverse(view);
	glm::vec3 cam_pos = glm::vec3(inv_view[3]);
	glm::vec3 light_dir = glm::normalize(glm::vec3(1.0f, 2.0f, 1.5f));

	int num_threads = std::thread::hardware_concurrency();
	if (num_threads == 0) {
		num_threads = 4;
	}

	std::vector<std::thread> threads;
	int rows_per_thread = height / num_threads;

	for (int t_i = 0; t_i < num_threads; ++t_i) {
		threads.emplace_back([&, t_i]() {
			int start_y = t_i * rows_per_thread;
			int end_y = (t_i == num_threads - 1) ? height : start_y + rows_per_thread;

			for (int y = start_y; y < end_y; ++y) {
				for (int x = 0; x < width; ++x) {
					float ndc_x = (x + 0.5f) / (float)width * 2.0f - 1.0f;
					float ndc_y = (y + 0.5f) / (float)height * 2.0f - 1.0f;
					ndc_y = -ndc_y; // matrices in GL format expect ndc [-1,-1] in bottom-left corner

					glm::vec4 ray_clip(ndc_x, ndc_y, -1.0f, 1.0f);
					glm::vec4 ray_eye = inv_proj * ray_clip;
					ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

					glm::vec3 ray_dir = glm::normalize(glm::vec3(inv_view * ray_eye));

					float t = 0.0f;
					const int max_steps = 64;
					const float max_dist = 100.0f;
					bool hit = false;

					for (int i = 0; i < max_steps; ++i) {
						glm::vec3 p = cam_pos + ray_dir * t;
						float d = sd_box(p, glm::vec3(1.0f));
						if (d < 0.001f) {
							hit = true;
							break;
						}
						t += d;
						if (t > max_dist) {
							break;
						}
					}

					glm::vec3 color(0.01f, 0.01f, 0.01f); // bg
					if (hit) {
						glm::vec3 p = cam_pos + ray_dir * t;
						glm::vec3 n = calc_normal(p);
						float diff = std::max(glm::dot(n, light_dir), 0.0f);
						color = glm::vec3(0.8f, 0.8f, 0.8f) * diff + glm::vec3(0.05f); // lambert + ambient
					}

					// gamma correct output for srgb8 storage
					color = glm::pow(color, glm::vec3(1.0f / 2.2f));

					int idx = (y * width + x) * 4;
					out_buf[idx + 0] = (uint8_t)std::clamp(color.r * 255.0f, 0.0f, 255.0f);
					out_buf[idx + 1] = (uint8_t)std::clamp(color.g * 255.0f, 0.0f, 255.0f);
					out_buf[idx + 2] = (uint8_t)std::clamp(color.b * 255.0f, 0.0f, 255.0f);
					out_buf[idx + 3] = 255;
				}
			}
		});
	}
	for (auto &t : threads) {
		t.join();
	}
}
