// ThreadPool.cpp
#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t numThreads): stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] { this->workerLoop(); });
    }
}

ThreadPool::~ThreadPool() {
    // 通知所有线程退出
    stop.store(true);
    condition.notify_all();
    // 等待所有线程结束
    for (auto &t : workers) {
        if (t.joinable()) t.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        tasks.push(std::move(task));
    }
    condition.notify_one();
}

void ThreadPool::workerLoop() {
    while (!stop.load()) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            // 等待任务到来或 stop 信号
            condition.wait(lock, [this] {
                return stop.load() || !tasks.empty();
            });
            if (stop.load() && tasks.empty()) return;
            task = std::move(tasks.front());
            tasks.pop();
        }
        // 执行任务
        task();
    }
}
