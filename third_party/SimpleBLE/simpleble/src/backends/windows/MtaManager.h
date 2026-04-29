#pragma once

#include <winrt/base.h>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <simpleble/Config.h>

namespace SimpleBLE {
    namespace WinRT {
class MtaManager {
public:
    static MtaManager& get() {
        static MtaManager instance;
        return instance;
    }

    void submit_task(std::function<void()> task);
    void stop();

    template<typename T>
    T execute_sync(std::function<T()> task) {
        // When flag is disabled, run in calling thread
        if (!Config::WinRT::experimental_use_own_mta_apartment) {
            try {
                return task();
            } catch (...) {
                throw;
            }
        }

        // Move the promise into the lambda to extend its lifetime
        auto result_promise = std::make_shared<std::promise<T>>(); // Use shared_ptr to manage lifetime
        auto result_future = result_promise->get_future();
        submit_task([promise = std::move(result_promise), task]() {
            try {
                T result = task();
                promise->set_value(result);
            } catch (...) {
                promise->set_exception(std::current_exception());
            }
        });
        return result_future.get();
    }

    void execute_sync(std::function<void()> task) {
        // When flag is disabled, run in calling thread
        if (!Config::WinRT::experimental_use_own_mta_apartment) {
            try {
                task();
                return;
            } catch (...) {
                throw;
            }
        }

        // Move the promise into the lambda to extend its lifetime
        auto result_promise = std::make_shared<std::promise<void>>(); // Use shared_ptr to manage lifetime
        auto result_future = result_promise->get_future();
        submit_task([promise = std::move(result_promise), task]() {
            try {
                task();
                promise->set_value();
            } catch (...) {
                promise->set_exception(std::current_exception());
            }
        });
        result_future.get();
    }

private:
    MtaManager();
    ~MtaManager();
    MtaManager(const MtaManager&) = delete;
    MtaManager& operator=(const MtaManager&) = delete;

    std::thread mta_thread_;
    std::mutex task_mutex_;
    std::condition_variable task_cv_;
    std::queue<std::function<void()>> task_queue_;
    std::atomic<bool> running_{true};

        void mta_thread_func();
    };
}  // namespace WinRT
}  // namespace SimpleBLE
