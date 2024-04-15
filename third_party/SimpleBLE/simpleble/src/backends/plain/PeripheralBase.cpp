#include "PeripheralBase.h"

#include "CharacteristicBuilder.h"
#include "DescriptorBuilder.h"
#include "ServiceBuilder.h"

#include <simpleble/Exceptions.h>
#include <algorithm>
#include "CommonUtils.h"
#include "LoggingInternal.h"

using namespace SimpleBLE;
using namespace std::chrono_literals;

const SimpleBLE::BluetoothUUID BATTERY_SERVICE_UUID = "0000180f-0000-1000-8000-00805f9b34fb";
const SimpleBLE::BluetoothUUID BATTERY_CHARACTERISTIC_UUID = "00002a19-0000-1000-8000-00805f9b34fb";

PeripheralBase::PeripheralBase() {}

PeripheralBase::~PeripheralBase() {}

void* PeripheralBase::underlying() const { return nullptr; }

std::string PeripheralBase::identifier() { return "Plain Peripheral"; }

BluetoothAddress PeripheralBase::address() { return "11:22:33:44:55:66"; }

BluetoothAddressType PeripheralBase::address_type() { return BluetoothAddressType::PUBLIC; };

int16_t PeripheralBase::rssi() { return -60; }

int16_t PeripheralBase::tx_power() { return 5; }

uint16_t PeripheralBase::mtu() {
    if (is_connected()) {
        return 247;
    } else {
        return 0;
    }
}

void PeripheralBase::connect() {
    connected_ = true;
    paired_ = true;
    SAFE_CALLBACK_CALL(this->callback_on_connected_);
}

void PeripheralBase::disconnect() {
    connected_ = false;
    SAFE_CALLBACK_CALL(this->callback_on_disconnected_);
}
bool PeripheralBase::is_connected() { return connected_; }

bool PeripheralBase::is_connectable() { return true; }

bool PeripheralBase::is_paired() { return paired_; }

void PeripheralBase::unpair() { paired_ = false; }

std::vector<Service> PeripheralBase::services() {
    if (!connected_) return {};

    std::vector<Service> service_list;

    service_list.push_back(
        ServiceBuilder(BATTERY_SERVICE_UUID,
                       {CharacteristicBuilder(BATTERY_CHARACTERISTIC_UUID, {}, true, false, false, true, false)}));
    return service_list;
}

std::vector<Service> PeripheralBase::advertised_services() { return {}; }

std::map<uint16_t, ByteArray> PeripheralBase::manufacturer_data() { return {}; }

ByteArray PeripheralBase::read(BluetoothUUID const& service, BluetoothUUID const& characteristic) { return {}; }

void PeripheralBase::write_request(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                   ByteArray const& data) {}

void PeripheralBase::write_command(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                   ByteArray const& data) {}

void PeripheralBase::notify(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                            std::function<void(ByteArray payload)> callback) {}

void PeripheralBase::indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                              std::function<void(ByteArray payload)> callback) {}

void PeripheralBase::unsubscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic) {}

ByteArray PeripheralBase::read(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                               BluetoothUUID const& descriptor) {
    return {};
}

void PeripheralBase::write(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                           BluetoothUUID const& descriptor, ByteArray const& data) {}

void PeripheralBase::set_callback_on_connected(std::function<void()> on_connected) {
    if (on_connected) {
        callback_on_connected_.load(std::move(on_connected));
    } else {
        callback_on_connected_.unload();
    }
}

void PeripheralBase::set_callback_on_disconnected(std::function<void()> on_disconnected) {
    if (on_disconnected) {
        callback_on_disconnected_.load(std::move(on_disconnected));
    } else {
        callback_on_disconnected_.unload();
    }
}
