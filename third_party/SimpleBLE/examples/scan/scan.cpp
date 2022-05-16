#include <chrono>
#include <iostream>
#include <thread>
#include <sstream>
#include <iomanip>

#include "simpleble/Adapter.h"

std::string byte_array_to_string(SimpleBLE::ByteArray& bytes) {
    std::ostringstream oss;
    for (auto byte : bytes) {
        oss << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)((uint8_t)byte) << " ";
    }
    return oss.str();
}

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

    adapter.set_callback_on_scan_found([](SimpleBLE::Peripheral peripheral) {
        std::cout << "Found device: " << peripheral.identifier() << " [" << peripheral.address() << "] " << peripheral.rssi() << " dBm" << std::endl;
    });

    adapter.set_callback_on_scan_updated([](SimpleBLE::Peripheral peripheral) {
        std::cout << "Updated device: " << peripheral.identifier() << " [" << peripheral.address() << "] " << peripheral.rssi() << " dBm" << std::endl;
    });

    // Scan for 5 seconds.
    adapter.scan_for(5000);

    std::cout << "Scan complete." << std::endl;

    // Sleep for an additional second before continuing.
    // If there are any detections during this period, it means that the
    // internal peripheral took longer to stop than anticipated.
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::vector<SimpleBLE::Peripheral> peripherals = adapter.scan_get_results();
    std::cout << "The following devices were found:" << std::endl;
    for (size_t i = 0; i < peripherals.size(); i++) {
        std::string connectable_string = peripherals[i].is_connectable() ? "Connectable" : "Non-Connectable";
        std::string peripheral_string = peripherals[i].identifier() + " [" + peripherals[i].address() + "]";

        std::cout << "[" << i << "] " << peripheral_string << " " << connectable_string << std::endl;
        std::map<uint16_t, SimpleBLE::ByteArray> manufacturer_data = peripherals[i].manufacturer_data();
        for (auto& [manufacturer_id, data] : manufacturer_data) {
            std::cout << "    Manufacturer ID: " << manufacturer_id << std::endl;
            std::cout << "    Manufacturer data: " << byte_array_to_string(data) << std::endl;
        }
    }

    return 0;
}
