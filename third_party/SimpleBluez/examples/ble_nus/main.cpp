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
    std::string mac_address = argv[1];
    bluez_service.init();

    std::thread* async_thread = new std::thread(async_thread_function);

    auto adapter = bluez_service.get_first_adapter();
    if (adapter == nullptr) {
        std::cout << "No adapter found!" << std::endl;
        return -1;
    }

    adapter->discovery_filter_transport_set("le");
    adapter->StartDiscovery();
    millisecond_delay(5000);
    adapter->StopDiscovery();

    auto device = adapter->get_device(mac_address);
    if (device != nullptr) {
        device->Connect();

        // In theory, we should wait for the OnConnect event.
        millisecond_delay(1000);

        auto characteristic_tx = device->get_characteristic("6e400001-b5a3-f393-e0a9-e50e24dcca9e",
                                                            "6e400003-b5a3-f393-e0a9-e50e24dcca9e");
        auto characteristic_rx = device->get_characteristic("6e400001-b5a3-f393-e0a9-e50e24dcca9e",
                                                            "6e400002-b5a3-f393-e0a9-e50e24dcca9e");
        if (characteristic_tx != nullptr) {
            characteristic_tx->ValueChanged = [&](std::vector<uint8_t> new_value) { /* Data! */ };
            characteristic_tx->StartNotify();
        }

        if (characteristic_rx != nullptr) {
            std::string message = "Hello World";
            characteristic_rx->write_command((uint8_t*)message.c_str(), message.length());
        }

        millisecond_delay(2000);
        characteristic_tx->StopNotify();
        device->Disconnect();
        millisecond_delay(1000);
    }

    async_thread_active = false;
    while (!async_thread->joinable()) {
        millisecond_delay(10);
    }
    async_thread->join();
    delete async_thread;

    return 0;
}
