#include <simplebluez/Bluez.h>

#include <cstdlib>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

SimpleBluez::Bluez bluez;

std::atomic_bool async_thread_active = true;
void async_thread_function() {
    while (async_thread_active) {
        bluez.run_async();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

void millisecond_delay(int ms) {
    for (int i = 0; i < ms; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main(int argc, char* argv[]) {
    int selection = -1;

    bluez.init();
    std::thread* async_thread = new std::thread(async_thread_function);

    auto adapters = bluez.get_adapters();
    std::cout << "The following adapters were found:" << std::endl;
    for (int i = 0; i < adapters.size(); i++) {
        std::cout << "[" << i << "] " << adapters[i]->identifier() << " [" << adapters[i]->address() << "]"
                  << std::endl;
    }

    std::cout << "Please select an adapter to scan: ";
    std::cin >> selection;
    if (selection < 0 || selection >= adapters.size()) {
        std::cout << "Invalid selection" << std::endl;
        return 1;
    }

    auto adapter = adapters[selection];
    std::cout << "Scanning " << adapter->identifier() << " [" << adapter->address() << "]" << std::endl;

    adapter->discovery_filter(SimpleBluez::Adapter::DiscoveryFilter::LE);

    adapter->set_on_device_updated([](std::shared_ptr<SimpleBluez::Device> device) {
        std::cout << "Update received for " << device->address() << std::endl;
        std::cout << "\tName " << device->name() << std::endl;
        std::cout << "\tRSSI " << std::dec << device->rssi() << std::endl;
        auto manufacturer_data = device->manufacturer_data();
        for (auto& [manufacturer_id, value_array] : manufacturer_data) {
            std::cout << "\tManuf ID 0x" << std::setfill('0') << std::setw(4) << std::hex << (int)manufacturer_id;
            std::cout << ": ";
            for (int i = 0; i < value_array.size(); i++) {
                std::cout << std::setfill('0') << std::setw(2) << std::hex << ((int)value_array[i]) << " ";
            }
            std::cout << std::endl;
        }
    });

    adapter->discovery_start();
    millisecond_delay(3000);
    adapter->discovery_stop();

    // Sleep for a bit to allow the adapter to stop discovering.
    millisecond_delay(3000);

    async_thread_active = false;
    while (!async_thread->joinable()) {
        millisecond_delay(10);
    }
    async_thread->join();
    delete async_thread;

    return 0;
}
