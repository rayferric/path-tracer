#pragma once

#include "pch.hpp"

namespace util {

class thread_pool {
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
    std::atomic<int> active_tasks;

public:
    thread_pool() : stop(false), active_tasks(0) {
        size_t num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) num_threads = 4; // fallback
        num_threads = 1;
        
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] { 
                            return stop || !tasks.empty(); 
                        });
                        
                        if (stop && tasks.empty()) return;
                        
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    
                    active_tasks++;
                    task();
                    active_tasks--;
                    condition.notify_all();
                }
            });
        }
    }

    void submit(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.push(std::move(task));
        }
        condition.notify_one();
    }

    void wait() {
        std::unique_lock<std::mutex> lock(queue_mutex);
        condition.wait(lock, [this] {
            return tasks.empty() && active_tasks == 0;
        });
    }

    ~thread_pool() {
        stop = true;
        condition.notify_all();
        for (auto& worker : workers) {
            worker.join();
        }
    }
};

}
