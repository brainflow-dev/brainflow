#include <iostream>

#include "simpleble/AdapterSafe.h"

int main() {
    auto bluetooth_enabled = SimpleBLE::Safe::Adapter::bluetooth_enabled();

    if (!bluetooth_enabled.has_value()) {
        std::cout << "Failed to determine Bluetooth status" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Bluetooth enabled: " << bluetooth_enabled.value() << std::endl;

    auto adapter_list = SimpleBLE::Safe::Adapter::get_adapters();

    if (!adapter_list.has_value()) {
        std::cout << "Failed to list adapters" << std::endl;
        return EXIT_FAILURE;
    }

    if (adapter_list->empty()) {
        std::cout << "No adapter found" << std::endl;
        return EXIT_FAILURE;
    }

    for (auto& adapter : *adapter_list) {
        std::cout << "Adapter: " << adapter.identifier().value() << " [" << adapter.address().value() << "]"
                  << std::endl;
    }

    return EXIT_SUCCESS;
}
