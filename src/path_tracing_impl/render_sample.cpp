#include "../pch.hpp"

#include "./render_sample_one_pixel.hpp"

// runs func(x, y) for every pixel in (width, height)
// pixels are processed in spatial groups to maximize cache locality between threads
// uses all available cpu threads, returns when all pixels are processed
static void multithread_image_ops(uint32_t width, uint32_t height, const std::function<void(uint32_t, uint32_t)> func, uint32_t tile_size = 32) {
	uint32_t num_tiles_x = (width + tile_size - 1) / tile_size;
	uint32_t num_tiles_y = (height + tile_size - 1) / tile_size;

	uint32_t num_threads = std::thread::hardware_concurrency();
	if (num_threads == 0) {
		num_threads = 16;
	}

	// group size should roughly match thread count
	uint32_t group_size = (uint32_t)std::sqrt(num_threads);
	// ^ floor -> oversubscription (possibly more threads than tiles in group)
	group_size = std::max(group_size, 2u); // minimum 2x2=4 tiles per group

	uint32_t num_groups_x = (num_tiles_x + group_size - 1) / group_size;
	uint32_t num_groups_y = (num_tiles_y + group_size - 1) / group_size;

	// build tile order: all tiles in group 0, then all in group 1, etc
	std::vector<uint32_t> tile_order;
	for (uint32_t gy = 0; gy < num_groups_y; gy++) {
		for (uint32_t gx = 0; gx < num_groups_x; gx++) {
			// add all tiles in this group
			for (uint32_t ty = 0; ty < group_size; ty++) {
				for (uint32_t tx = 0; tx < group_size; tx++) {
					uint32_t tile_x = gx * group_size + tx;
					uint32_t tile_y = gy * group_size + ty;
					if (tile_x < num_tiles_x && tile_y < num_tiles_y) {
						tile_order.push_back(tile_y * num_tiles_x + tile_x);
					}
				}
			}
		}
	}

	uint32_t num_tiles = tile_order.size();
	std::atomic<uint32_t> next_tile_idx{0};

	auto worker = [&]() {
		while (true) {
			uint32_t idx = next_tile_idx.fetch_add(1, std::memory_order_relaxed);
			if (idx >= num_tiles) {
				break;
			}

			uint32_t tile_linear = tile_order[idx];
			uint32_t tile_x = tile_linear % num_tiles_x;
			uint32_t tile_y = tile_linear / num_tiles_x;

			uint32_t pixel_x_start = tile_x * tile_size;
			uint32_t pixel_y_start = tile_y * tile_size;
			uint32_t pixel_x_end = std::min(pixel_x_start + tile_size, width);
			uint32_t pixel_y_end = std::min(pixel_y_start + tile_size, height);

			// render this tile
			for (uint32_t py = pixel_y_start; py < pixel_y_end; py++) {
				for (uint32_t px = pixel_x_start; px < pixel_x_end; px++) {
					func(px, py);
				}
			}
		}
	};

	std::vector<std::thread> threads;
	threads.reserve(num_threads);
	for (uint32_t i = 0; i < num_threads; i++) {
		threads.emplace_back(worker);
	}

	for (auto &t : threads) {
		t.join();
	}
}

void render_sample(const scene_data &scn, glm::fvec4 *out_buf, uint32_t width, uint32_t height, uint32_t sample_idx, bool transparent_bg) {
	auto per_pixel_job = [&](uint32_t x, uint32_t y) {
		render_sample_one_pixel(scn, out_buf, width, height, x, y, sample_idx, transparent_bg);
	};
	multithread_image_ops(width, height, per_pixel_job);
}
