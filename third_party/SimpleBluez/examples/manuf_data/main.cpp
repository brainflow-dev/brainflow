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

/**
 * This is a minimal example in which we exchange some data
 * with a device exposing the Nordic UART service.
 */
int main(int argc, char* argv[]) {
    bluez_service.init();

    std::thread* async_thread = new std::thread(async_thread_function);

    auto adapter = bluez_service.get_first_adapter();
    if (adapter == nullptr) {
        std::cout << "No adapter found!" << std::endl;
        return -1;
    }

    adapter->discovery_filter_transport_set("le");

    adapter->OnDeviceUpdated = [](std::shared_ptr<BluezDevice> device) {
        std::cout << "Update received for " << device->get_address() << std::endl;
        std::cout << "\tName " << device->get_name() << std::endl;
        auto manufacturer_data = device->get_manufacturer_data();
        for (auto& [manufacturer_id, value_array] : manufacturer_data) {
            std::cout << "\tManuf ID " << std::dec << (int)manufacturer_id << std::endl;
            std::cout << "\t\t";
            for (int i = 0; i < value_array.size(); i++) {
                std::cout << std::setfill('0') << std::setw(2) << std::hex << ((int)value_array[i]) << " ";
            }
            std::cout << std::endl;
        }
    };

    adapter->StartDiscovery();
    millisecond_delay(5000);
    adapter->StopDiscovery();

    async_thread_active = false;
    while (!async_thread->joinable()) {
        millisecond_delay(10);
    }
    async_thread->join();
    delete async_thread;

    return 0;
}
