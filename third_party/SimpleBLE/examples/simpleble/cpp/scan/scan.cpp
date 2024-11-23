#include <iomanip>
#include <iostream>
#include <sstream>

#include "../common/utils.hpp"

#include "simpleble/Adapter.h"

int main() {
    auto adapter_optional = Utils::getAdapter();

    if (!adapter_optional.has_value()) {
        return EXIT_FAILURE;
    }

    auto adapter = adapter_optional.value();

    adapter.set_callback_on_scan_found([](SimpleBLE::Peripheral peripheral) {
        std::cout << "Found device: " << peripheral.identifier() << " [" << peripheral.address() << "] "
                  << peripheral.rssi() << " dBm" << std::endl;
    });

    adapter.set_callback_on_scan_updated([](SimpleBLE::Peripheral peripheral) {
        std::cout << "Updated device: " << peripheral.identifier() << " [" << peripheral.address() << "] "
                  << peripheral.rssi() << " dBm" << std::endl;
    });

    adapter.set_callback_on_scan_start([]() { std::cout << "Scan started." << std::endl; });

    adapter.set_callback_on_scan_stop([]() { std::cout << "Scan stopped." << std::endl; });

    // Scan for 5 seconds.
    adapter.scan_for(5000);

    std::cout << "Scan complete." << std::endl;

    std::vector<SimpleBLE::Peripheral> peripherals = adapter.scan_get_results();
    std::cout << "The following devices were found:" << std::endl;
    for (size_t i = 0; i < peripherals.size(); i++) {
        std::string connectable_string = peripherals[i].is_connectable() ? "Connectable" : "Non-Connectable";
        std::string peripheral_string = peripherals[i].identifier() + " [" + peripherals[i].address() + "] " +
                                        std::to_string(peripherals[i].rssi()) + " dBm";

        std::cout << "[" << i << "] " << peripheral_string << " " << connectable_string << std::endl;

        std::cout << "    Tx Power: " << std::dec << peripherals[i].tx_power() << " dBm" << std::endl;
        std::cout << "    Address Type: " << peripherals[i].address_type() << std::endl;

        std::vector<SimpleBLE::Service> services = peripherals[i].services();
        for (auto& service : services) {
            std::cout << "    Service UUID: " << service.uuid() << std::endl;
            std::cout << "    Service data: " << service.data() << std::endl;
        }

        std::map<uint16_t, SimpleBLE::ByteArray> manufacturer_data = peripherals[i].manufacturer_data();
        for (auto& [manufacturer_id, data] : manufacturer_data) {
            std::cout << "    Manufacturer ID: " << manufacturer_id << std::endl;
            std::cout << "    Manufacturer data: " << data << std::endl;
        }
    }
    return EXIT_SUCCESS;
}
