#include <iostream>

#include "simpleble/Adapter.h"
#include "simpleble/Utils.h"

int main() {
    std::cout << "Using SimpleBLE version: " << SimpleBLE::get_simpleble_version() << std::endl;
    std::cout << "Bluetooth enabled: " << SimpleBLE::Adapter::bluetooth_enabled() << std::endl;

    auto adapter_list = SimpleBLE::Adapter::get_adapters();

    if (adapter_list.empty()) {
        std::cout << "No adapter found" << std::endl;
        return EXIT_FAILURE;
    }

    for (auto& adapter : adapter_list) {
        std::cout << "Adapter: " << adapter.identifier() << " [" << adapter.address() << "]" << std::endl;

        auto peripherals = adapter.get_paired_peripherals();
        for (auto& peripheral : peripherals) {
            std::cout << "  Peripheral: " << peripheral.identifier() << " [" << peripheral.address() << "]" << std::endl;
        }
    }

    return EXIT_SUCCESS;
}
