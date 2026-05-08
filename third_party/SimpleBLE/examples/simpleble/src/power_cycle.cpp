#include <iostream>
#include <thread>

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

        adapter.set_callback_on_power_on([&]() {
            std::cout << "Adapter powered on" << std::endl;
        });

        adapter.set_callback_on_power_off([&]() {
            std::cout << "Adapter powered off" << std::endl;
        });

        std::cout << "Adapter powered: " << adapter.is_powered() << std::endl;

        std::cout << "Powering off adapter" << std::endl;
        adapter.power_off();
        std::cout << "Adapter powered: " << adapter.is_powered() << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(5));

        std::cout << "Powering on adapter" << std::endl;
        adapter.power_on();
        std::cout << "Adapter powered: " << adapter.is_powered() << std::endl;
    }

    return EXIT_SUCCESS;
}
