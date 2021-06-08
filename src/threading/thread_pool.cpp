#include "threading/thread_pool.hpp"

namespace threading {

thread_pool::thread_pool(uint32_t thread_count) {
	if (thread_count == 0)
		thread_count = std::thread::hardware_concurrency();

	threads.reserve(thread_count);
	while (thread_count-- != 0)
		threads.emplace_back(&thread_pool::process, this);
}

thread_pool::~thread_pool() {
	// Halt all threads currently waiting on notifier
	terminating = true;
	notifier.notify_all();

	// Wait for threads that still might be processing
	for (std::thread &thread : threads)
		thread.join();

	// Process remaining jobs on this thread
	while (!queue.empty()) {
		queue.front()();
		queue.pop();
	}
}

void thread_pool::schedule(std::function<void()> &&job) {
	{
		std::lock_guard guard(mutex);
		queue.push(std::move(job));
	}
	
	notifier.notify_one();

	{	
		std::lock_guard guard(wait_mutex);
		wait_jobs++;
	}
}

void thread_pool::wait() {
	std::unique_lock lock(wait_mutex);

	if (wait_jobs != 0) {
		wait_notifier.wait(lock, [this] {
			return wait_jobs == 0;
		});
	}
}

uint32_t thread_pool::get_thread_count() const {
	return threads.size();
}

void thread_pool::process() {
	while (true) {
		std::function<void()> job;

		{
			std::unique_lock lock(mutex);

			if (terminating)
				return;

			if (queue.empty()) {
				notifier.wait(lock, [this] {
					return !queue.empty() || terminating;
				});
			}

			if (terminating)
				return;
			
			job = std::move(queue.front());
			queue.pop();
		}

		job();

		{
			std::lock_guard guard(wait_mutex);
			wait_jobs--;
		}

		wait_notifier.notify_all();
	}
}

}
