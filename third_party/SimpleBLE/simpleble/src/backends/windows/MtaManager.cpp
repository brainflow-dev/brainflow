#include "MtaManager.h"

namespace SimpleBLE {
namespace WinRT {

MtaManager::MtaManager() {
    mta_thread_ = std::thread(&MtaManager::mta_thread_func, this);
}

MtaManager::~MtaManager() {
    stop();
    if (mta_thread_.joinable()) {
        mta_thread_.join();
    }
}

void MtaManager::submit_task(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(task_mutex_);
        task_queue_.push(std::move(task));
    }
    task_cv_.notify_one();
}

void MtaManager::stop() {
    {
        std::lock_guard<std::mutex> lock(task_mutex_);
        running_ = false;
    }
    task_cv_.notify_one();
}

void MtaManager::mta_thread_func() {
    winrt::init_apartment(winrt::apartment_type::multi_threaded);
    while (running_) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(task_mutex_);
            task_cv_.wait(lock, [this] { return !task_queue_.empty() || !running_; });
            if (!running_ && task_queue_.empty()) break;
            task = std::move(task_queue_.front());
            task_queue_.pop();
        }
        if (task) task();
    }
}

}  // namespace WinRT
}  // namespace SimpleBLE
