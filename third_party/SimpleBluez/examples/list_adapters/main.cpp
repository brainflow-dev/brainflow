#include "BluezService.h"

#include <stdlib.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

BluezService bluez_service;

volatile bool async_thread_active = true;
void async_thread_function() {
    while (async_thread_active) {
        bluez_service.run_async();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void millisecond_delay(int ms) {
    for (int i = 0; i < ms; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main(int argc, char* argv[]) {
    bluez_service.init();
    std::thread* async_thread = new std::thread(async_thread_function);

    auto internal_adapters = bluez_service.get_all_adapters();
    for (auto& adapter : internal_adapters) {
        std::cout << "Adapter: " << adapter->get_identifier() << " [" << adapter->Address() << "]" << std::endl;
    }

    async_thread_active = false;
    while (!async_thread->joinable()) {
        millisecond_delay(10);
    }
    async_thread->join();
    delete async_thread;

    return 0;
}
