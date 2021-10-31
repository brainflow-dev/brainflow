#include <chrono>
#include <iostream>
#include <thread>

#include "simpleble/Adapter.h"

int main(int argc, char* argv[]) {
    auto adapter_list = SimpleBLE::Adapter::get_adapters();

    if (adapter_list.size() == 0) {
        std::cout << "No adapter was found." << std::endl;
    }

    // Pick the first detected adapter as the default.
    // TODO: Allow the user to pick the adapter.
    SimpleBLE::Adapter adapter = adapter_list[0];

    adapter.set_callback_on_scan_start([]() { std::cout << "Scan started." << std::endl; });

    adapter.set_callback_on_scan_stop([]() { std::cout << "Scan stopped." << std::endl; });

    adapter.set_callback_on_scan_found([](SimpleBLE::Peripheral peripheral) {
        std::cout << "Found device: " << peripheral.identifier() << " [" << peripheral.address() << "]" << std::endl;
    });

    adapter.set_callback_on_scan_updated([](SimpleBLE::Peripheral peripheral) {
        std::cout << "Updated device: " << peripheral.identifier() << " [" << peripheral.address() << "]" << std::endl;
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
    for (int i = 0; i < peripherals.size(); i++) {
        std::cout << "[" << i << "] " << peripherals[i].identifier() << " [" << peripherals[i].address() << "]"
                  << std::endl;
    }

    return 0;
}
