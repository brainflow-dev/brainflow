#pragma once

#include <chrono>
#include <functional>
#include <future>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>

class TaskRunner {
  private:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;
    using Task = std::function<std::optional<Duration>()>;

    struct ScheduledTask {
        TimePoint executionTime;
        Task task;

        bool operator>(const ScheduledTask& other) const { return executionTime > other.executionTime; }
    };

    std::priority_queue<ScheduledTask, std::vector<ScheduledTask>, std::greater<ScheduledTask>> taskQueue;
    std::mutex taskQueueMutex;
    std::condition_variable taskQueueCV;
    bool running = false;
    std::thread workerThread;

  public:
    TaskRunner() = default;

    ~TaskRunner() { stop(); }

    void start() {
        if (!running) {
            running = true;
            workerThread = std::thread(&TaskRunner::workerLoop, this);
        }
    }

    void stop() {
        if (running) {
            {
                std::unique_lock<std::mutex> lock(taskQueueMutex);
                running = false;
                taskQueueCV.notify_one();
            }
            workerThread.join();
        }
    }

    void dispatch(Task task, Duration delay) {
        TimePoint executionTime = Clock::now() + delay;
        {
            std::unique_lock<std::mutex> lock(taskQueueMutex);
            taskQueue.push({executionTime, std::move(task)});
            taskQueueCV.notify_one();
        }
        if (!running) {
            start();
        }
    }

  private:
    void workerLoop() {
        while (true) {
            std::unique_lock<std::mutex> lock(taskQueueMutex);
            taskQueueCV.wait(lock, [this] { return !running || !taskQueue.empty(); });

            if (!running) {
                break;
            }

            if (taskQueue.empty()) {
                continue;
            }

            // NOTE: If a new task is added to the queue with a shorter delay than the current top task, the worker
            // thread will not wake up until the top task's execution time is reached. This is not ideal, but it's good
            // enough for this simple implementation.

            auto now = Clock::now();
            if (taskQueue.top().executionTime > now) {
                taskQueueCV.wait_until(lock, taskQueue.top().executionTime);
            }

            auto task = std::move(taskQueue.top().task);
            taskQueue.pop();
            lock.unlock();

            auto result = task();
            if (result.has_value()) {
                dispatch(std::move(task), *result);
            }
        }
    }
};