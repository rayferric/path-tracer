#pragma once

#include "../pch.hpp"

class fps_tracker {
public:
	fps_tracker(float update_interval = 0.5f) : update_interval(update_interval), frame_count(0), elapsed_time(0.0f) {
		current_fps.store(0);
	}

	void mark_frame() {
		auto now = std::chrono::high_resolution_clock::now();

		if (last_time.time_since_epoch().count() == 0) {
			last_time = now;
			return;
		}

		float frame_time = std::chrono::duration<float>(now - last_time).count();
		last_time = now;

		elapsed_time += frame_time;
		frame_count++;

		if (elapsed_time >= update_interval) {
			current_fps.store(frame_count / elapsed_time);
			frame_count = 0;
			elapsed_time = 0.0f;
		}
	}

	float get_fps() const {
		return current_fps.load();
	}

private:
	std::chrono::high_resolution_clock::time_point last_time;
	float update_interval;
	int frame_count;
	float elapsed_time;
	std::atomic<float> current_fps;
};
