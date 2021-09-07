#include "Bluez.h"
#include <mutex>

using namespace SimpleBLE;

Bluez* Bluez::get() {
    static std::mutex get_mutex;       // Static mutex to ensure thread safety when accessing the logger
    std::scoped_lock lock(get_mutex);  // Unlock the mutex on function return
    static Bluez instance;             // Static instance of the logger to ensure proper lifecycle management
    return &instance;
}

Bluez::Bluez() {
    bluez_service.init();
    async_thread_active = true;
    async_thread = new std::thread(&Bluez::async_thread_function, this);
}

Bluez::~Bluez() {
    async_thread_active = false;
    while (!async_thread->joinable()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    async_thread->join();
    delete async_thread;
}

void Bluez::async_thread_function() {
    while (async_thread_active) {
        bluez_service.run_async();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}
