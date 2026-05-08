#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class ThreadRunner {
  public:
    ThreadRunner() : _stop(false) { _thread = std::thread(&ThreadRunner::threadFunc, this); }

    ~ThreadRunner() {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _stop = true;
            _cv.notify_one();
        }
        _thread.join();
    }

    void enqueue(std::function<void()> func) {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _queue.push(std::move(func));
            _cv.notify_one();
        }
    }

  private:
    void threadFunc() {
        while (true) {
            std::function<void()> func;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _cv.wait(lock, [this] { return _stop || !_queue.empty(); });
                if (_stop && _queue.empty()) {
                    return;
                }
                func = std::move(_queue.front());
                _queue.pop();
            }
            func();
        }
    }

    std::thread _thread;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::queue<std::function<void()>> _queue;
    bool _stop;
};
