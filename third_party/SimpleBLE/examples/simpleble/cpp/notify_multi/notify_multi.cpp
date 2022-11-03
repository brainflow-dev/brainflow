#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include "../common/utils.hpp"

#include "simpleble/SimpleBLE.h"

using namespace std::chrono_literals;

int main() {
    std::vector<SimpleBLE::Peripheral> aux_p;  // Helper vector to hold scan results.

    // Fetch the adapter object that will be used for this test.
    auto adapter_optional = Utils::getAdapter();
    if (!adapter_optional.has_value()) {
        return EXIT_FAILURE;
    }
    auto adapter = adapter_optional.value();

    // Configure adapter callbacks.
    adapter.set_callback_on_scan_found([&](SimpleBLE::Peripheral peripheral) {
        std::cout << "Found device: " << peripheral.identifier() << " [" << peripheral.address() << "]" << std::endl;
        aux_p.push_back(peripheral);
    });
    adapter.set_callback_on_scan_start([]() { std::cout << "Scan started." << std::endl; });
    adapter.set_callback_on_scan_stop([]() { std::cout << "Scan stopped." << std::endl; });

    // Create two uninitialized peripherals that will hold our connections for this example.
    SimpleBLE::Peripheral peripherals[2];

    // Variable that enables printing per peripheral.
    std::atomic_bool print_allowed{false};

    for (size_t iter = 0; iter < 2; iter++) {
        aux_p.clear();

        adapter.scan_for(5000);

        std::cout << "The following devices were found:" << std::endl;
        for (size_t i = 0; i < aux_p.size(); i++) {
            std::cout << "[" << i << "] " << aux_p[i].identifier() << " [" << aux_p[i].address() << "]" << std::endl;
        }

        auto selection = Utils::getUserInputInt("Please select a device to connect to", aux_p.size() - 1);
        if (!selection.has_value()) {
            return EXIT_FAILURE;
        }

        peripherals[iter] = aux_p[selection.value()];

        std::cout << "Connecting to " << peripherals[iter].identifier() << " [" << peripherals[iter].address() << "]"
                  << std::endl;
        peripherals[iter].connect();

        std::cout << "Successfully connected, printing services and characteristics.." << std::endl;

        // Store all service and characteristic uuids in a vector.
        std::vector<std::pair<SimpleBLE::BluetoothUUID, SimpleBLE::BluetoothUUID>> uuids;
        for (auto service : peripherals[iter].services()) {
            for (auto characteristic : service.characteristics()) {
                uuids.push_back(std::make_pair(service.uuid(), characteristic.uuid()));
            }
        }

        std::cout << "The following services and characteristics were found:" << std::endl;
        for (size_t i = 0; i < uuids.size(); i++) {
            std::cout << "[" << i << "] " << uuids[i].first << " " << uuids[i].second << std::endl;
        }

        selection = Utils::getUserInputInt("Please select a characteristic to read", uuids.size() - 1);

        if (!selection.has_value()) {
            return EXIT_FAILURE;
        }

        // Subscribe to the characteristic.
        peripherals[iter].notify(uuids[selection.value()].first, uuids[selection.value()].second,
                                 [&, iter](SimpleBLE::ByteArray bytes) {
                                     if (print_allowed) {
                                         std::cout << "Peripheral " << iter << " received: ";
                                         Utils::print_byte_array(bytes);
                                     }
                                 });
    }

    // Once both devices are connected and streaming, enable printing for 5 seconds, then disconnect from both.
    print_allowed = true;

    std::this_thread::sleep_for(5s);

    peripherals[1].disconnect();

    std::this_thread::sleep_for(3s);

    peripherals[0].disconnect();

    return EXIT_SUCCESS;
}
