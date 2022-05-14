#include <simplebluez/Bluez.h>
#include <simplebluez/Exceptions.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <thread>

SimpleBluez::Bluez bluez;

std::atomic_bool async_thread_active = true;
void async_thread_function() {
    while (async_thread_active) {
        bluez.run_async();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

void millisecond_delay(int ms) {
    for (int i = 0; i < ms; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

std::vector<std::shared_ptr<SimpleBluez::Device>> peripherals;

int main(int argc, char* argv[]) {
    int selection = -1;

    bluez.init();
    std::thread* async_thread = new std::thread(async_thread_function);

    auto agent = bluez.get_agent();
    agent->set_capabilities(SimpleBluez::Agent::Capabilities::KeyboardDisplay);

    // Configure all callback handlers for the agent, as part of this example.
    agent->set_on_request_pin_code([&]() {
        std::cout << "Agent::RequestPinCode" << std::endl;
        return "123456";
    });

    agent->set_on_display_pin_code([&](const std::string& pin_code) {
        std::cout << "Agent::DisplayPinCode: " << pin_code << std::endl;
        return true;
    });

    agent->set_on_request_passkey([&]() {
        std::cout << "Agent::RequestPasskey" << std::endl;
        return 123456;
    });

    agent->set_on_display_passkey([&](uint32_t passkey, uint16_t entered) {
        std::cout << "Agent::DisplayPasskey: " << passkey << " (" << entered << " entered)" << std::endl;
    });

    agent->set_on_request_confirmation([&](uint32_t passkey) {
        std::cout << "Agent::RequestConfirmation: " << passkey << std::endl;
        return true;
    });

    agent->set_on_request_authorization([&]() {
        std::cout << "Agent::RequestAuthorization" << std::endl;
        return true;
    });

    agent->set_on_authorize_service([&](const std::string& uuid) {
        std::cout << "Agent::AuthorizeService: " << uuid << std::endl;
        return true;
    });

    bluez.register_agent();

    auto adapters = bluez.get_adapters();
    std::cout << "Available adapters:" << std::endl;
    for (int i = 0; i < adapters.size(); i++) {
        std::cout << "[" << i << "] " << adapters[i]->identifier() << " [" << adapters[i]->address() << "]"
                  << std::endl;
    }

    std::cout << "Please select an adapter to scan: ";
    std::cin >> selection;
    if (selection < 0 || selection >= adapters.size()) {
        std::cout << "Invalid selection" << std::endl;
        return 1;
    }

    auto adapter = adapters[selection];
    std::cout << "Scanning " << adapter->identifier() << " [" << adapter->address() << "]" << std::endl;

    adapter->discovery_filter(SimpleBluez::Adapter::DiscoveryFilter::LE);

    adapter->set_on_device_updated([](std::shared_ptr<SimpleBluez::Device> device) {
        if (std::find(peripherals.begin(), peripherals.end(), device) == peripherals.end()) {
            std::cout << "Found device: " << device->name() << " [" << device->address() << "]" << std::endl;
            peripherals.push_back(device);
        }
    });

    adapter->discovery_start();
    millisecond_delay(3000);
    adapter->discovery_stop();

    std::cout << "The following devices were found:" << std::endl;
    for (int i = 0; i < peripherals.size(); i++) {
        std::cout << "[" << i << "] " << peripherals[i]->name() << " [" << peripherals[i]->address() << "]"
                  << std::endl;
    }

    std::cout << "Please select a device to pair to: ";
    std::cin >> selection;

    if (selection >= 0 && selection < peripherals.size()) {
        auto peripheral = peripherals[selection];
        std::cout << "Pairing to " << peripheral->name() << " [" << peripheral->address() << "]" << std::endl;

        for (int attempt = 0; attempt < 3; attempt++) {
            try {
                peripheral->connect();

                // At this point, the connection will be established,
                // but services might not be resolved yet

                for (int i = 0; i < 10; i++) {
                    std::cout << "Waiting for services to resolve..." << std::endl;
                    millisecond_delay(100);
                    if (peripheral->services_resolved()) {
                        break;
                    }
                }

                if (peripheral->connected() && peripheral->services_resolved()) {
                    break;
                }

            } catch (SimpleDBus::Exception::SendFailed& e) {
                millisecond_delay(100);
            }
        }

        if (!peripheral->connected() || !peripheral->services_resolved()) {
            std::cout << "Failed to connect to " << peripheral->name() << " [" << peripheral->address() << "]"
                      << std::endl;
            return 1;
        }

        std::cout << "Successfully connected, listing services." << std::endl;
        for (auto service : peripheral->services()) {
            std::cout << "Service: " << service->uuid() << std::endl;
            for (auto characteristic : service->characteristics()) {
                std::cout << "  Characteristic: " << characteristic->uuid() << std::endl;
            }
        }

        millisecond_delay(2000);
        peripheral->disconnect();

        if (peripheral->paired()) {
            adapter->device_remove(peripheral->path());
        }

        // Sleep for an additional second before returning.
        // If there are any unexpected events, this example will help debug them.
        millisecond_delay(1000);
    }

    async_thread_active = false;
    while (!async_thread->joinable()) {
        millisecond_delay(10);
    }
    async_thread->join();
    delete async_thread;

    return 0;
}
