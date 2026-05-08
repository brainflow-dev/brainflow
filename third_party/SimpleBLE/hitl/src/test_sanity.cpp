#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include "simpleble/SimpleBLE.h"

TEST(BLESanityTest, FullSequentialTest) {
    // Initialize adapter
    auto adapters = SimpleBLE::Adapter::get_adapters();
    ASSERT_FALSE(adapters.empty()) << "No Bluetooth adapters found";
    SimpleBLE::Adapter adapter = adapters[0];
    EXPECT_TRUE(adapter.bluetooth_enabled());

    // Scan for device
    SimpleBLE::Peripheral target_peripheral;
    const std::string TARGET_MAC = "11:22:33:44:55:66";  // Replace with your device's MAC
    const std::string TARGET_NAME = "SimpleBLE DUT";
    bool device_found = false;

    adapter.set_callback_on_scan_found([&](SimpleBLE::Peripheral peripheral) {
        std::cout << "Found device: " << peripheral.identifier() << " (" << peripheral.address() << ")" << std::endl;

        if (peripheral.address() == TARGET_MAC || peripheral.identifier() == TARGET_NAME) {
            target_peripheral = peripheral;
            device_found = true;
        }
    });

    adapter.scan_for(10000);
    ASSERT_TRUE(device_found) << "Target device not found during scan";
    EXPECT_EQ(target_peripheral.identifier(), TARGET_NAME);
    EXPECT_TRUE(target_peripheral.is_connectable());

    // Connect and check services
    ASSERT_NO_THROW(target_peripheral.connect()) << "Failed to connect to target device";

    auto services = target_peripheral.services();
    EXPECT_FALSE(services.empty()) << "No services found on connected device";

    // Print services for debugging
    for (auto& service : services) {
        std::cout << "Found service: " << service.uuid() << std::endl;
    }

    // Cleanup
    if (target_peripheral.is_connected()) {
        target_peripheral.disconnect();
    }
}
