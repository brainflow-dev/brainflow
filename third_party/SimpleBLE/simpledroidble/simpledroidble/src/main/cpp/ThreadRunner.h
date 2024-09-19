#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <jni.h>
#include "android_utils.h"
#include <fmt/core.h>
#include <optional>

class ThreadRunner {
public:
    ThreadRunner() : _stop(false), _jvm(nullptr) {}

    ~ThreadRunner() {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _stop = true;
            _cv.notify_one();
        }
        if (_thread && _thread->joinable()) {
            _thread->join();
        }
    }

    void set_jvm(JavaVM *jvm) {
        _jvm = jvm;
    }

    void enqueue(std::function<void()> func) {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (!_thread) {
                // Thread is lazily started upon the first task enqueue
                _thread.emplace(&ThreadRunner::threadFunc, this);
            }
            _queue.push(std::move(func));
        }
        _cv.notify_one();
    }

private:
    void threadFunc() {
        if (!_jvm) {
            log_error("No JVM provided");
            return;
        }

        // Attach the thread to the JVM
        JNIEnv *env;
        jint result = _jvm->AttachCurrentThread(&env, NULL);
        if (result != JNI_OK) {
            log_error("Failed to attach thread");
            return;
        }

        // Run the thread loop
        while (true) {
            std::function<void()> func;

            // Retrieve and execute the next task
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _cv.wait(lock, [this] { return _stop || !_queue.empty(); });
                if (_stop && _queue.empty()) {
                    break;
                }
                func = std::move(_queue.front());
                _queue.pop();
            }

            try {
                func();
            } catch (const std::exception &e) {
                log_error(fmt::format("Exception in thread: {}", e.what()));
            }
        }

        // Detach the thread from the JVM
        _jvm->DetachCurrentThread();
    }

    JavaVM *_jvm;
    std::optional<std::thread> _thread;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::queue<std::function<void()>> _queue;
    bool _stop;
};
