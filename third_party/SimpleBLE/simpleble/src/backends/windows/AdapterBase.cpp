// This weird pragma is required for the compiler to properly include the necessary namespaces.
#pragma comment(lib, "windowsapp")
#include "AdapterBase.h"

#include "CommonUtils.h"
#include "LoggingInternal.h"
#include "PeripheralBuilder.h"
#include "Utils.h"

#include "winrt/Windows.Devices.Bluetooth.h"
#include "winrt/Windows.Devices.Enumeration.h"
#include "winrt/Windows.Devices.Radios.h"
#include "winrt/Windows.Foundation.Collections.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/base.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace SimpleBLE;
using namespace std::chrono_literals;

AdapterBase::AdapterBase(std::string device_id)
    : adapter_(async_get(BluetoothAdapter::FromIdAsync(winrt::to_hstring(device_id)))) {
    auto device_information = async_get(
        Devices::Enumeration::DeviceInformation::CreateFromIdAsync(winrt::to_hstring(device_id)));
    identifier_ = winrt::to_string(device_information.Name());

    // Configure the scanner object
    scanner_ = Advertisement::BluetoothLEAdvertisementWatcher();

    scanner_stopped_token_ = scanner_.Stopped(
        [this](const auto& w, const Advertisement::BluetoothLEAdvertisementWatcherStoppedEventArgs args) {
            this->_scan_stopped_callback();
        });

    scanner_received_token_ = scanner_.Received(
        [this](const auto& w, const Advertisement::BluetoothLEAdvertisementReceivedEventArgs args) {
            std::lock_guard<std::mutex> lock(this->scan_update_mutex_);
            if (!this->scan_is_active_) return;

            advertising_data_t data;
            data.mac_address = _mac_address_to_str(args.BluetoothAddress());
            Bluetooth::BluetoothAddressType addr_type_enum = args.BluetoothAddressType();
            switch (addr_type_enum) {
                case Bluetooth::BluetoothAddressType::Public:
                    data.address_type = SimpleBLE::BluetoothAddressType::PUBLIC;
                    break;

                case Bluetooth::BluetoothAddressType::Random:
                    data.address_type = SimpleBLE::BluetoothAddressType::RANDOM;
                    break;

                case Bluetooth::BluetoothAddressType::Unspecified:
                    data.address_type = SimpleBLE::BluetoothAddressType::UNSPECIFIED;
                    break;
            }

            data.identifier = winrt::to_string(args.Advertisement().LocalName());
            data.connectable = args.IsConnectable();
            data.rssi = args.RawSignalStrengthInDBm();

            if (args.TransmitPowerLevelInDBm()) {
                data.tx_power = args.TransmitPowerLevelInDBm().Value();
            } else {
                data.tx_power = INT16_MIN;
            }

            // Parse manufacturer data
            auto manufacturer_data = args.Advertisement().ManufacturerData();
            for (auto& item : manufacturer_data) {
                uint16_t company_id = item.CompanyId();
                ByteArray manufacturer_data_buffer = ibuffer_to_bytearray(item.Data());
                data.manufacturer_data[company_id] = manufacturer_data_buffer;
            }

            // Parse service data.
            const auto& sections = args.Advertisement().DataSections();
            for (const auto& section : sections) {
                ByteArray section_data_buffer = ibuffer_to_bytearray(section.Data());

                std::string service_uuid;
                ByteArray service_data;

                if (section.DataType() == Advertisement::BluetoothLEAdvertisementDataTypes::ServiceData128BitUuids()) {
                    service_uuid = fmt::format(
                        "{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-"
                        "{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
                        (uint8_t)section_data_buffer[15], (uint8_t)section_data_buffer[14],
                        (uint8_t)section_data_buffer[13], (uint8_t)section_data_buffer[12],
                        (uint8_t)section_data_buffer[11], (uint8_t)section_data_buffer[10],
                        (uint8_t)section_data_buffer[9], (uint8_t)section_data_buffer[8],
                        (uint8_t)section_data_buffer[7], (uint8_t)section_data_buffer[6],
                        (uint8_t)section_data_buffer[5], (uint8_t)section_data_buffer[4],
                        (uint8_t)section_data_buffer[3], (uint8_t)section_data_buffer[2],
                        (uint8_t)section_data_buffer[1], (uint8_t)section_data_buffer[0]);
                    service_data = section_data_buffer.substr(16);
                }

                else if (section.DataType() ==
                         Advertisement::BluetoothLEAdvertisementDataTypes::ServiceData32BitUuids()) {
                    service_uuid = fmt::format("{:02x}{:02x}{:02x}{:02x}-0000-1000-8000-00805f9b34fb",
                                               (uint8_t)section_data_buffer[3], (uint8_t)section_data_buffer[2],
                                               (uint8_t)section_data_buffer[1], (uint8_t)section_data_buffer[0]);
                    service_data = section_data_buffer.substr(4);
                } else if (section.DataType() ==
                           Advertisement::BluetoothLEAdvertisementDataTypes::ServiceData16BitUuids()) {
                    service_uuid = fmt::format("0000{:02x}{:02x}-0000-1000-8000-00805f9b34fb",
                                               (uint8_t)section_data_buffer[1], (uint8_t)section_data_buffer[0]);
                    service_data = section_data_buffer.substr(2);
                } else {
                    continue;
                }

                data.service_data.emplace(std::make_pair(service_uuid, service_data));
            }

            // Parse service uuids
            auto service_data = args.Advertisement().ServiceUuids();
            for (auto& service_guid : service_data) {
                std::string service_uuid = guid_to_uuid(service_guid);
                data.service_data.emplace(std::make_pair(service_uuid, ByteArray()));
            }

            this->_scan_received_callback(data);
        });
}

AdapterBase::~AdapterBase() {
    // TODO: What happens if the adapter is still scanning?

    if (scanner_received_token_) {
        scanner_.Received(scanner_received_token_);
    }

    if (scanner_stopped_token_) {
        scanner_.Stopped(scanner_stopped_token_);
    }
}

bool AdapterBase::bluetooth_enabled() {
    initialize_winrt();

    bool enabled = false;
    auto radio_collection = async_get(Radio::GetRadiosAsync());
    for (uint32_t i = 0; i < radio_collection.Size(); i++) {
        auto radio = radio_collection.GetAt(i);

        // Skip non-bluetooth radios
        if (radio.Kind() != RadioKind::Bluetooth) {
            continue;
        }

        // Assume that bluetooth is enabled if any of the radios are enabled
        if (radio.State() == RadioState::On) {
            enabled = true;
            break;
        }
    }

    return enabled;
}

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

void* AdapterBase::underlying() const { return reinterpret_cast<void*>(const_cast<BluetoothAdapter*>(&adapter_)); }

std::string AdapterBase::identifier() { return identifier_; }

BluetoothAddress AdapterBase::address() { return _mac_address_to_str(adapter_.BluetoothAddress()); }

void AdapterBase::scan_start() {
    this->seen_peripherals_.clear();

    scanner_.ScanningMode(Advertisement::BluetoothLEScanningMode::Active);
    scan_is_active_ = true;
    scanner_.Start();

    SAFE_CALLBACK_CALL(this->callback_on_scan_start_);
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
    for (auto& [address, base_peripheral] : this->seen_peripherals_) {
        PeripheralBuilder peripheral_builder(base_peripheral);
        peripherals.push_back(peripheral_builder);
    }

    return peripherals;
}

std::vector<Peripheral> AdapterBase::get_paired_peripherals() { return {}; }

void AdapterBase::set_callback_on_scan_start(std::function<void()> on_scan_start) {
    if (on_scan_start) {
        callback_on_scan_start_.load(on_scan_start);
    } else {
        callback_on_scan_start_.unload();
    }
}

void AdapterBase::set_callback_on_scan_stop(std::function<void()> on_scan_stop) {
    if (on_scan_stop) {
        callback_on_scan_stop_.load(on_scan_stop);
    } else {
        callback_on_scan_stop_.unload();
    }
}

void AdapterBase::set_callback_on_scan_updated(std::function<void(Peripheral)> on_scan_updated) {
    if (on_scan_updated) {
        callback_on_scan_updated_.load(on_scan_updated);
    } else {
        callback_on_scan_updated_.unload();
    }
}

void AdapterBase::set_callback_on_scan_found(std::function<void(Peripheral)> on_scan_found) {
    if (on_scan_found) {
        callback_on_scan_found_.load(on_scan_found);
    } else {
        callback_on_scan_found_.unload();
    }
}

// Private functions

void AdapterBase::_scan_stopped_callback() {
    std::lock_guard<std::mutex> lock(scan_update_mutex_);
    scan_is_active_ = false;
    scan_stop_cv_.notify_all();

    SAFE_CALLBACK_CALL(this->callback_on_scan_stop_);
}

void AdapterBase::_scan_received_callback(advertising_data_t data) {
    if (this->peripherals_.count(data.mac_address) == 0) {
        // If the incoming peripheral has never been seen before, create and save a reference to it.
        auto base_peripheral = std::make_shared<PeripheralBase>(data);
        this->peripherals_.insert(std::make_pair(data.mac_address, base_peripheral));
    }

    // Update the received advertising data.
    auto base_peripheral = this->peripherals_.at(data.mac_address);
    base_peripheral->update_advertising_data(data);

    // Convert the base object into an external-facing Peripheral object
    PeripheralBuilder peripheral_builder(base_peripheral);

    // Check if the device has been seen before, to forward the correct call to the user.
    if (this->seen_peripherals_.count(data.mac_address) == 0) {
        // Store it in our table of seen peripherals
        this->seen_peripherals_.insert(std::make_pair(data.mac_address, base_peripheral));
        SAFE_CALLBACK_CALL(this->callback_on_scan_found_, peripheral_builder);
    } else {
        SAFE_CALLBACK_CALL(this->callback_on_scan_updated_, peripheral_builder);
    }
}
