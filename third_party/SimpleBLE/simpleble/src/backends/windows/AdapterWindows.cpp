// This weird pragma is required for the compiler to properly include the necessary namespaces.
#pragma comment(lib, "windowsapp")
#include "AdapterWindows.h"
#include "BackendWinRT.h"

#include "BuilderBase.h"
#include "CommonUtils.h"
#include "LoggingInternal.h"
#include "PeripheralWindows.h"
#include "Utils.h"
#include "MtaManager.h"

#include "winrt/Windows.Devices.Bluetooth.h"
#include "winrt/Windows.Devices.Radios.h"
#include "winrt/Windows.Devices.Enumeration.h"
#include "winrt/Windows.Foundation.Collections.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/base.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace SimpleBLE;
using namespace SimpleBLE::WinRT;
using namespace std::chrono_literals;

AdapterWindows::AdapterWindows(std::string device_id)
    : adapter_(async_get(BluetoothAdapter::FromIdAsync(winrt::to_hstring(device_id)))),
      radio_(async_get(adapter_.GetRadioAsync())) {
    // IMPORTANT NOTE: This function must be executed in the MTA context. In this case, this is managed by the BackendWinRT class.
    auto device_information = async_get(
        Devices::Enumeration::DeviceInformation::CreateFromIdAsync(winrt::to_hstring(device_id)));
    identifier_ = winrt::to_string(device_information.Name());

    // Configure the scanner object
    scanner_ = Advertisement::BluetoothLEAdvertisementWatcher();

    // Register member functions directly as callback handlers
    radio_state_changed_token_ = radio_.StateChanged({this, &AdapterWindows::on_power_state_changed});
    scanner_stopped_token_ = scanner_.Stopped({this, &AdapterWindows::_on_scanner_stopped});
    scanner_received_token_ = scanner_.Received({this, &AdapterWindows::_on_scanner_received});
}

AdapterWindows::~AdapterWindows() {
    _callback_on_scan_stop.unload();

    MtaManager::get().execute_sync([this]() {
        scanner_.Stop();

        if (scanner_received_token_) {
            scanner_.Received(scanner_received_token_);
        }

        if (scanner_stopped_token_) {
            scanner_.Stopped(scanner_stopped_token_);
        }
    });
}

// FIXME: this should return wether this particular adapter is enabled, not if any adapter is enabled.
bool AdapterWindows::bluetooth_enabled() { return BackendWinRT::get()->bluetooth_enabled(); }

void* AdapterWindows::underlying() const { return reinterpret_cast<void*>(const_cast<BluetoothAdapter*>(&adapter_)); }

std::string AdapterWindows::identifier() { return identifier_; }

BluetoothAddress AdapterWindows::address() {
    return MtaManager::get().execute_sync<BluetoothAddress>([this]() {
        return _mac_address_to_str(adapter_.BluetoothAddress());
    });
}

void AdapterWindows::power_on() {
    MtaManager::get().execute_sync([this]() { async_get(radio_.SetStateAsync(RadioState::On)); });
}

void AdapterWindows::power_off() {
    MtaManager::get().execute_sync([this]() { async_get(radio_.SetStateAsync(RadioState::Off)); });
}

bool AdapterWindows::is_powered() {
    return MtaManager::get().execute_sync<bool>([this]() { return radio_.State() == RadioState::On; });
}

void AdapterWindows::scan_start() {
    this->seen_peripherals_.clear();

    MtaManager::get().execute_sync([this]() {
        scanner_.ScanningMode(Advertisement::BluetoothLEScanningMode::Active);
        scan_is_active_ = true;
        scanner_.Start();
    });

    SAFE_CALLBACK_CALL(this->_callback_on_scan_start);
}

void AdapterWindows::scan_stop() {
    MtaManager::get().execute_sync([this]() {
        scanner_.Stop();
    });

    std::unique_lock<std::mutex> lock(scan_stop_mutex_);
    if (scan_stop_cv_.wait_for(lock, 1s, [this] { return !this->scan_is_active_; })) {
        // Scan stopped
    } else {
        // Scan did not stop, this can be because some other process
        // is using the adapter.
    }
}

void AdapterWindows::scan_for(int timeout_ms) {
    scan_start();
    std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
    scan_stop();
}

bool AdapterWindows::scan_is_active() { return scan_is_active_; }

SharedPtrVector<PeripheralBase> AdapterWindows::scan_get_results() { return Util::values(seen_peripherals_); }

SharedPtrVector<PeripheralBase> AdapterWindows::get_paired_peripherals() {
    return MtaManager::get().execute_sync<SharedPtrVector<PeripheralBase>>([this]() {
        SharedPtrVector<PeripheralBase> peripherals;
        winrt::hstring aqs_filter = BluetoothLEDevice::GetDeviceSelectorFromPairingState(true);
        auto dev_info_collection = async_get(Devices::Enumeration::DeviceInformation::FindAllAsync(aqs_filter));

        for (const auto& dev_info : dev_info_collection) {
            try {
                BluetoothLEDevice device = async_get(BluetoothLEDevice::FromIdAsync(dev_info.Id()));
                if (device == nullptr) {
                    SIMPLEBLE_LOG_WARN(fmt::format("Could not get BluetoothLEDevice for paired device ID: {}", winrt::to_string(dev_info.Id())));
                    continue;
                }

                BluetoothAddress address = _mac_address_to_str(device.BluetoothAddress());
                if (this->peripherals_.count(address) == 0) {
                    // If the peripheral has never been seen before, create and save a reference to it.
                    auto base_peripheral = std::make_shared<PeripheralWindows>(device);
                    this->peripherals_.insert(std::make_pair(address, base_peripheral));
                }

                peripherals.push_back(this->peripherals_.at(address));
            } catch (const Exception::WinRTException& e){
                SIMPLEBLE_LOG_ERROR(fmt::format("WinRT error processing paired device {} : {}", winrt::to_string(dev_info.Id()), e.what()));

                // NOTE: For debugging purposes, we'll print the error message and continue.
                fmt::print("WinRT error processing paired device {} : {}", winrt::to_string(dev_info.Id()), e.what());
                //throw Exception::WinRTException(e.code().value, winrt::to_string(e.message()));
                continue;
            }
        }
        return peripherals;
    });
}

SharedPtrVector<PeripheralBase> AdapterWindows::get_connected_peripherals() {
    return MtaManager::get().execute_sync<SharedPtrVector<PeripheralBase>>([this]() {
        SharedPtrVector<PeripheralBase> peripherals;
        winrt::hstring aqs_filter = BluetoothLEDevice::GetDeviceSelectorFromConnectionStatus(BluetoothConnectionStatus::Connected);
        auto dev_info_collection = async_get(Devices::Enumeration::DeviceInformation::FindAllAsync(aqs_filter));

        for (const auto& dev_info : dev_info_collection) {
            try {
                BluetoothLEDevice device = async_get(BluetoothLEDevice::FromIdAsync(dev_info.Id()));
                if (device == nullptr) {
                    SIMPLEBLE_LOG_WARN(fmt::format("Could not get BluetoothLEDevice for connected device ID: {}", winrt::to_string(dev_info.Id())));
                    continue;
                }

                BluetoothAddress address = _mac_address_to_str(device.BluetoothAddress());
                if (this->peripherals_.count(address) == 0) {
                    // If the peripheral has never been seen before, create and save a reference to it.
                    auto base_peripheral = std::make_shared<PeripheralWindows>(device);
                    this->peripherals_.insert(std::make_pair(address, base_peripheral));
                }

                peripherals.push_back(this->peripherals_.at(address));
            } catch (const winrt::hresult_error& e) {
                SIMPLEBLE_LOG_ERROR(fmt::format("WinRT error processing connected device {} : {}", winrt::to_string(dev_info.Id()), winrt::to_string(e.message())));

                // NOTE: For debugging purposes, we'll print the error message and continue.
                fmt::print("WinRT error processing connected device {} : {}", winrt::to_string(dev_info.Id()), winrt::to_string(e.message()));
                // throw Exception::WinRTException(e.code().value, winrt::to_string(e.message()));
                continue;
            }
        }
        return peripherals;
    });
}

// Private functions

void AdapterWindows::_scan_stopped_callback() {
    std::lock_guard<std::mutex> lock(scan_update_mutex_);
    scan_is_active_ = false;
    scan_stop_cv_.notify_all();

    SAFE_CALLBACK_CALL(this->_callback_on_scan_stop);
}

void AdapterWindows::_scan_received_callback(advertising_data_t data) {
    if (this->peripherals_.count(data.mac_address) == 0) {
        // If the incoming peripheral has never been seen before, create and save a reference to it.
        auto base_peripheral = std::make_shared<PeripheralWindows>(data);
        this->peripherals_.insert(std::make_pair(data.mac_address, base_peripheral));
    }

    // Update the received advertising data.
    auto base_peripheral = this->peripherals_.at(data.mac_address);
    base_peripheral->update_advertising_data(data);

    // Convert the base object into an external-facing Peripheral object
    Peripheral peripheral = Factory::build(base_peripheral);

    // Check if the device has been seen before, to forward the correct call to the user.
    if (this->seen_peripherals_.count(data.mac_address) == 0) {
        // Store it in our table of seen peripherals
        this->seen_peripherals_.insert(std::make_pair(data.mac_address, base_peripheral));
        SAFE_CALLBACK_CALL(this->_callback_on_scan_found, peripheral);
    } else {
        SAFE_CALLBACK_CALL(this->_callback_on_scan_updated, peripheral);
    }
}

void AdapterWindows::on_power_state_changed(Radio const& sender, Foundation::IInspectable const&) {
    auto state = sender.State();
    if (state == RadioState::On) {
        SAFE_CALLBACK_CALL(this->_callback_on_power_on);
    } else if (state == RadioState::Off) {
        SAFE_CALLBACK_CALL(this->_callback_on_power_off);
    }
}

void AdapterWindows::_on_scanner_stopped(
    const Advertisement::BluetoothLEAdvertisementWatcher& watcher,
    const Advertisement::BluetoothLEAdvertisementWatcherStoppedEventArgs args) {
    // This callback is already in the MTA context as it's called by WinRT
    _scan_stopped_callback();
}

void AdapterWindows::_on_scanner_received(
    const Advertisement::BluetoothLEAdvertisementWatcher& watcher,
    const Advertisement::BluetoothLEAdvertisementReceivedEventArgs args) {
    // This callback is already in the MTA context as it's called by WinRT
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
    const auto manufacturer_data = args.Advertisement().ManufacturerData();
    for (const auto& item : manufacturer_data) {
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
            service_data = section_data_buffer.slice_from(16);
        }

        else if (section.DataType() ==
                    Advertisement::BluetoothLEAdvertisementDataTypes::ServiceData32BitUuids()) {
            service_uuid = fmt::format("{:02x}{:02x}{:02x}{:02x}-0000-1000-8000-00805f9b34fb",
                                        (uint8_t)section_data_buffer[3], (uint8_t)section_data_buffer[2],
                                        (uint8_t)section_data_buffer[1], (uint8_t)section_data_buffer[0]);
            service_data = section_data_buffer.slice_from(4);
        } else if (section.DataType() ==
                    Advertisement::BluetoothLEAdvertisementDataTypes::ServiceData16BitUuids()) {
            service_uuid = fmt::format("0000{:02x}{:02x}-0000-1000-8000-00805f9b34fb",
                                        (uint8_t)section_data_buffer[1], (uint8_t)section_data_buffer[0]);
            service_data = section_data_buffer.slice_from(2);
        } else {
            continue;
        }

        data.service_data.emplace(std::make_pair(service_uuid, service_data));
    }

    // Parse service uuids
    const auto service_data = args.Advertisement().ServiceUuids();
    for (const auto& service_guid : service_data) {
        std::string service_uuid = guid_to_uuid(service_guid);
        data.service_data.emplace(std::make_pair(service_uuid, ByteArray()));
    }

    this->_scan_received_callback(data);
}
