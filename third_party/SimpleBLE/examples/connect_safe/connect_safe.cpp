#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "simpleble/SimpleBLE.h"

std::vector<SimpleBLE::Safe::Peripheral> peripherals;

int main(int argc, char* argv[]) {
    auto adapter_list = SimpleBLE::Safe::Adapter::get_adapters();

    if (!adapter_list.has_value()) {
        std::cout << "Failed to " << std::endl;
        return 1;
    }

    if (adapter_list->size() == 0) {
        std::cout << "No adapter was found." << std::endl;
        return 1;
    }

    // Pick the first detected adapter as the default.
    // TODO: Allow the user to pick an adapter.
    SimpleBLE::Safe::Adapter adapter = adapter_list->at(0);

    adapter.set_callback_on_scan_start([]() { std::cout << "Scan started." << std::endl; });

    adapter.set_callback_on_scan_stop([]() { std::cout << "Scan stopped." << std::endl; });

    adapter.set_callback_on_scan_found([&](SimpleBLE::Safe::Peripheral peripheral) {
        std::cout << "Found device: " << peripheral.identifier().value_or("UNKNOWN") << " ["
                  << peripheral.address().value_or("UNKNOWN") << "]" << std::endl;
        peripherals.push_back(peripheral);
    });

    // Scan for 5 seconds and return.
    adapter.scan_for(5000);

    std::cout << "The following devices were found:" << std::endl;
    for (int i = 0; i < peripherals.size(); i++) {
        std::cout << "[" << i << "] " << peripherals[i].identifier().value_or("UNKNOWN") << " ["
                  << peripherals[i].address().value_or("UNKNOWN") << "]" << std::endl;
    }

    int selection = -1;
    std::cout << "Please select a device to connect to: ";
    std::cin >> selection;

    if (selection >= 0 && selection < peripherals.size()) {
        auto peripheral = peripherals[selection];
        std::cout << "Connecting to " << peripheral.identifier().value_or("UNKNOWN") << " ["
                  << peripheral.address().value_or("UNKNOWN") << "]" << std::endl;

        // If the connection wasn't successful, no exception will be thrown.
        bool connect_was_successful = peripheral.connect();
        if (!connect_was_successful) {
            std::cout << "Failed to connect to " << peripheral.identifier().value_or("UNKNOWN") << " ["
                      << peripheral.address().value_or("UNKNOWN") << "]" << std::endl;
            return 1;
        }

        std::cout << "Successfully connected, listing services." << std::endl;
        auto services = peripheral.services();

        if (!services.has_value()) {
            std::cout << "Failed to list services." << std::endl;
            return 1;
        }

        for (auto service : *services) {
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
