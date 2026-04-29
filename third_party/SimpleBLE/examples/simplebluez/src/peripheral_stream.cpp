#include <simplebluez/Bluez.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <thread>
#include "simplebluez/Types.h"

SimpleBluez::Bluez bluez;

std::atomic_bool async_thread_active = true;
void async_thread_function() {
    while (async_thread_active) {
        bluez.run_async();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

std::atomic_bool app_running = true;
void signal_handler(int signal) { app_running = false; }

void millisecond_delay(int ms) {
    for (int i = 0; i < ms; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);
    bluez.init();
    std::thread* async_thread = new std::thread(async_thread_function);
    auto adapter = bluez.get_adapters()[0];

    std::cout << "Initializing SimpleBluez Peripheral Mode Demo" << std::endl;

    // --- ADAPTER SETUP ---
    std::map<std::string, std::shared_ptr<SimpleBluez::Device>> peripherals;
    adapter->set_on_device_updated([&peripherals](std::shared_ptr<SimpleBluez::Device> device) {
        const bool device_connected = device->connected(); // THIS CAUSES A DEADLOCK!
        const bool is_new_device = peripherals.find(device->address()) == peripherals.end();

        if (device_connected && is_new_device) {
            peripherals[device->address()] = device;
            std::cout << "New peripheral: " << device->name() << " [" << device->address() << "]" << std::endl;

            // NOTE: This moment can also be used to deregister the advertisement if only one connection is needed.
        } else if (!device_connected && !is_new_device) {
            peripherals.erase(device->address());
            std::cout << "Lost peripheral: " << device->name() << " [" << device->address() << "]" << std::endl;
        }
    });

    // -- APPLICATION SETUP --
    auto svc_manager = bluez.root_custom()->service_mgr_add("main");

    // --- SERVICE DEFINITION ---
    auto service0 = svc_manager->service_add("my_service");
    service0->uuid("12345678-1234-5678-1234-567812345678");
    service0->primary(true);

    auto characteristic0 = service0->characteristic_add("my_characteristic");
    characteristic0->uuid("12345678-AAAA-5678-1234-567812345678");

    characteristic0->set_on_read_value([characteristic0]() {
        std::cout << "ReadValue called" << std::endl;
    });

    characteristic0->set_on_write_value([characteristic0](const SimpleBluez::ByteArray& value) {
        std::cout << "WriteValue called with value: " << value.toHex() << std::endl;
    });

    characteristic0->set_on_notify([characteristic0](bool notify) {
        std::cout << "Notify called with notify: " << notify << std::endl;
    });

    // Register the services and characteristics.
    adapter->register_application(svc_manager->path());

    // NOTE: This long delay is not necessary. However, once an application is registered
    // you want to wait until all services have been added to the adapter. This is done by
    // checking the UUIDs property of org.bluez.Adapter1.
    millisecond_delay(1000);

    // --- ADVERTISEMENT DEFINITION ---

    auto advertisement = bluez.root_custom()->advertisement_add("fried_potato");
    std::map<uint16_t, SimpleBluez::ByteArray> data;
    data[0x1024] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
    advertisement->manufacturer_data(data);
    advertisement->timeout(10);
    advertisement->local_name("SimpleBluez");

    // --- MAIN EVENT LOOP ---
    while (app_running) {
        // Handle advertising state.
        if (!advertisement->active()) {
            adapter->register_advertisement(advertisement);
            std::cout << "Advertising on " << adapter->identifier() << " [" << adapter->address() << "]" << std::endl;
        }

        // TODO: Handle connection events.

        // TODO: Handle data updates.
        static int value = 0;
        characteristic0->value({(uint8_t)(value), (uint8_t)(value + 1), (uint8_t)(value + 2)});
        value = (value * 1103515245 + 12345) & 0xFFFFFF;

        // This should eventually become a yield.
        millisecond_delay(100);
    }

    // --- CLEANUP ---

    for (auto& peripheral : peripherals) {
        std::cout << "Disconnecting from " << peripheral.second->name() << " [" << peripheral.second->address() << "]" << std::endl;
        peripheral.second->disconnect();
    }

    adapter->unregister_advertisement(advertisement);
    adapter->unregister_application(svc_manager->path());

    async_thread_active = false;
    async_thread->join();
    delete async_thread;

    return 0;
}