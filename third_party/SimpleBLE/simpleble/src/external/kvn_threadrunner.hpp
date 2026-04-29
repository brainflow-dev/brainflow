/*
 * SPDX-FileCopyrightText: 2025 Kevin Dewald <kevin@dewald.me>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef KVN_THREADRUNNER_HPP
#define KVN_THREADRUNNER_HPP

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace kvn {

class thread_runner {
  public:
    thread_runner() : _stop(false) { _thread = std::thread(&thread_runner::thread_func, this); }

    // Remove copy constructor and copy assignment
    thread_runner(const thread_runner&) = delete;
    thread_runner& operator=(const thread_runner&) = delete;

    virtual ~thread_runner() {
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
            lock.unlock();  // Unlock before notify
            _cv.notify_one();
        }
    }

  protected:
    void thread_func() {
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

}  // namespace kvn

#endif  // KVN_THREADRUNNER_HPP