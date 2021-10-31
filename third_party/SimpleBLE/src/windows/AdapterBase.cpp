// This weird pragma is required for the compiler to properly include the necessary namespaces.
#pragma comment(lib, "windowsapp")
#include "AdapterBase.h"

#include "PeripheralBuilder.h"
#include "Utils.h"

#include "winrt/Windows.Devices.Bluetooth.h"
#include "winrt/Windows.Devices.Enumeration.h"
#include "winrt/Windows.Devices.Radios.h"
#include "winrt/Windows.Foundation.Collections.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/base.h"

#include <iostream>
#include <sstream>

using namespace SimpleBLE;
using namespace std::chrono_literals;

AdapterBase::AdapterBase(std::string device_id)
    : adapter_(async_get(BluetoothAdapter::FromIdAsync(winrt::to_hstring(device_id)))) {
    auto device_information = async_get(
        Devices::Enumeration::DeviceInformation::CreateFromIdAsync(winrt::to_hstring(device_id)));
    identifier_ = winrt::to_string(device_information.Name());

    // Configure the scanner object
    scanner_ = Advertisement::BluetoothLEAdvertisementWatcher();
    scanner_.Stopped([=](const auto& w, const Advertisement::BluetoothLEAdvertisementWatcherStoppedEventArgs args) {
        this->_scan_stopped_callback();
    });
    scanner_.Received([&](const auto& w, const Advertisement::BluetoothLEAdvertisementReceivedEventArgs args) {
        advertising_data_t data;
        data.mac_address = _mac_address_to_str(args.BluetoothAddress());
        data.identifier = winrt::to_string(args.Advertisement().LocalName());
        data.connectable = args.IsConnectable();

        // Parse manufacturer data
        auto manufacturer_data = args.Advertisement().ManufacturerData();
        for (auto& item : manufacturer_data) {
            uint16_t company_id = item.CompanyId();
            ByteArray manufacturer_data_buffer = ibuffer_to_bytearray(item.Data());
            data.manufacturer_data[company_id] = manufacturer_data_buffer;
        }

        this->_scan_received_callback(data);
    });
}

AdapterBase::~AdapterBase() {}

std::vector<std::shared_ptr<AdapterBase>> AdapterBase::get_adapters() {
    initialize_winrt();

    auto device_selector = BluetoothAdapter::GetDeviceSelector();
    auto device_information_collection = async_get(
        Devices::Enumeration::DeviceInformation::FindAllAsync(device_selector));

    std::vector<std::shared_ptr<AdapterBase>> adapter_list;
    for (uint32_t i = 0; i < device_information_collection.Size(); i++) {
        auto dev_info = device_information_collection.GetAt(i);
        adapter_list.push_back(std::make_shared<AdapterBase>(winrt::to_string(dev_info.Id())));
    }
    return adapter_list;
}

std::string AdapterBase::identifier() { return identifier_; }

BluetoothAddress AdapterBase::address() { return _mac_address_to_str(adapter_.BluetoothAddress()); }

void AdapterBase::scan_start() {
    scanner_.ScanningMode(Advertisement::BluetoothLEScanningMode::Active);
    scan_is_active_ = true;
    scanner_.Start();

    if (callback_on_scan_start_) {
        callback_on_scan_start_();
    }
}

void AdapterBase::scan_stop() {
    scanner_.Stop();

    std::unique_lock<std::mutex> lock(scan_stop_mutex_);
    if (scan_stop_cv_.wait_for(lock, 1s, [=] { return !this->scan_is_active_; })) {
        // Scan stopped
    } else {
        // Scan did not stop, this can be because some other process
        // is using the adapter.
    }
}

void AdapterBase::scan_for(int timeout_ms) {
    scan_start();
    std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
    scan_stop();
}

bool AdapterBase::scan_is_active() { return scan_is_active_; }

std::vector<Peripheral> AdapterBase::scan_get_results() {
    std::vector<Peripheral> peripherals;
    for (auto& [address, base_peripheral] : this->peripherals_) {
        PeripheralBuilder peripheral_builder(base_peripheral);
        peripherals.push_back(peripheral_builder);
    }

    return peripherals;
}

void AdapterBase::set_callback_on_scan_start(std::function<void()> on_scan_start) {
    callback_on_scan_start_ = on_scan_start;
}
void AdapterBase::set_callback_on_scan_stop(std::function<void()> on_scan_stop) {
    callback_on_scan_stop_ = on_scan_stop;
}
void AdapterBase::set_callback_on_scan_updated(std::function<void(Peripheral)> on_scan_updated) {
    callback_on_scan_updated_ = on_scan_updated;
}
void AdapterBase::set_callback_on_scan_found(std::function<void(Peripheral)> on_scan_found) {
    callback_on_scan_found_ = on_scan_found;
}

// Private functions

void AdapterBase::_scan_stopped_callback() {
    scan_is_active_ = false;
    scan_stop_cv_.notify_all();
    if (callback_on_scan_stop_) {
        callback_on_scan_stop_();
    }
}

void AdapterBase::_scan_received_callback(advertising_data_t data) {
    if (this->peripherals_.count(data.mac_address) == 0) {
        // Create a new PeripheralBase object
        std::shared_ptr<PeripheralBase> base_peripheral = std::make_shared<PeripheralBase>(data);

        // Store it in our table of seem peripherals
        this->peripherals_.insert(std::make_pair(data.mac_address, base_peripheral));

        // Convert the base object into an external-facing Peripheral object
        PeripheralBuilder peripheral_builder(base_peripheral);
        if (this->callback_on_scan_found_) {
            this->callback_on_scan_found_(peripheral_builder);
        }
    } else {
        // Load the existing PeripheralBase object
        std::shared_ptr<PeripheralBase> base_peripheral = this->peripherals_.at(data.mac_address);

        // Convert the base object into an external-facing Peripheral object
        PeripheralBuilder peripheral_builder(base_peripheral);
        if (this->callback_on_scan_updated_) {
            this->callback_on_scan_updated_(peripheral_builder);
        }
    }
}
