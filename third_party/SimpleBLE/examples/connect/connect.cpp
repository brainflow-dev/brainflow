#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "simpleble/SimpleBLE.h"

std::vector<SimpleBLE::Peripheral> peripherals;

int main() {
    auto adapter_list = SimpleBLE::Adapter::get_adapters();

    if (adapter_list.size() == 0) {
        std::cout << "No adapter was found." << std::endl;
        return 1;
    }

    std::cout << "Available adapters: \n";
    int i = 0;
    for (auto& adapter : adapter_list) {
        std::cout << "[" << i++ << "] " << adapter.identifier() << " [" << adapter.address() << "]" << std::endl;
    }

    int adapter_selection = -1;
    while(adapter_selection < 0 || adapter_selection > adapter_list.size() - 1) {
        std::cout << "Please select an adapter: ";
        std::cin >> adapter_selection;
    }

    SimpleBLE::Adapter& adapter = adapter_list[adapter_selection];

    adapter.set_callback_on_scan_start([]() { std::cout << "Scan started." << std::endl; });

    adapter.set_callback_on_scan_stop([]() { std::cout << "Scan stopped." << std::endl; });

    adapter.set_callback_on_scan_found([&](SimpleBLE::Peripheral peripheral) {
        std::cout << "Found device: " << peripheral.identifier() << " [" << peripheral.address() << "]" << std::endl;
        peripherals.push_back(peripheral);
    });

    // Scan for 5 seconds and return.
    adapter.scan_for(5000);

    std::cout << "The following devices were found:" << std::endl;
    for (size_t i = 0; i < peripherals.size(); i++) {
        std::cout << "[" << i << "] " << peripherals[i].identifier() << " [" << peripherals[i].address() << "]"
                  << std::endl;
    }

    int selection = -1;
    std::cout << "Please select a device to connect to: ";
    std::cin >> selection;

    if (selection >= 0 && selection < peripherals.size()) {
        auto peripheral = peripherals[selection];
        std::cout << "Connecting to " << peripheral.identifier() << " [" << peripheral.address() << "]" << std::endl;
        peripheral.connect();

        std::cout << "Successfully connected, listing services." << std::endl;
        for (auto service : peripheral.services()) {
            std::cout << "Service: " << service.uuid << std::endl;
            for (auto characteristic : service.characteristics) {
                std::cout << "  Characteristic: " << characteristic << std::endl;
            }
        }
        peripheral.disconnect();

        // Sleep for an additional second before returning.
        // If there are any unexpected events, this example will help debug them.
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
