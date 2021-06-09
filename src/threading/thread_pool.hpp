#pragma once

#include "pch.hpp"

namespace threading {

class thread_pool {
public:
	thread_pool(uint32_t thread_count = 0);

	~thread_pool();

	void schedule(std::function<void()> &&job);

	void wait();

	uint32_t get_thread_count() const;

private:
	std::vector<std::thread> threads;
	std::queue<std::function<void()>> queue;
	std::condition_variable notifier, wait_notifier;
	std::mutex mutex, wait_mutex;
	std::atomic_bool terminating = false;
	size_t wait_jobs = 0;

	void process();
};

}
