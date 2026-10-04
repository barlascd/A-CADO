#pragma once
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class TaskSystem {
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_ = false;
public:
    explicit TaskSystem(unsigned threads = std::max(1u, std::thread::hardware_concurrency())) {
        for (unsigned i = 0; i < threads; ++i)
            workers_.emplace_back([this] {
                for (;;) {
                    std::function<void()> job;
                    {
                        std::unique_lock lock(mutex_);
                        cv_.wait(lock, [this] { return stop_ || !queue_.empty(); });
                        if (stop_ && queue_.empty()) return;
                        job = std::move(queue_.front());
                        queue_.pop();
                    }
                    job();
                }
            });
    }
    ~TaskSystem() {
        { std::lock_guard lock(mutex_); stop_ = true; }
        cv_.notify_all();
        for (auto& w : workers_) w.join();
    }
    template <typename F>
    auto submit(F&& function) -> std::future<std::invoke_result_t<F>> {
        using R = std::invoke_result_t<F>;
        auto task = std::make_shared<std::packaged_task<R()>>(std::forward<F>(function));
        auto fut = task->get_future();
        { std::lock_guard lock(mutex_); queue_.emplace([task] { (*task)(); }); }
        cv_.notify_one();
        return fut;
    }
};
