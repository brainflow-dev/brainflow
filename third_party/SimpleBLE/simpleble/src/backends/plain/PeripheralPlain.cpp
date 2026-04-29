#include "PeripheralPlain.h"

#include "CharacteristicBase.h"
#include "DescriptorBase.h"
#include "ServiceBase.h"

#include <simpleble/Exceptions.h>

#include <memory>

#include "CommonUtils.h"
#include "LoggingInternal.h"

using namespace SimpleBLE;
using namespace std::chrono_literals;

const SimpleBLE::BluetoothUUID BATTERY_SERVICE_UUID = "0000180f-0000-1000-8000-00805f9b34fb";
const SimpleBLE::BluetoothUUID BATTERY_CHARACTERISTIC_UUID = "00002a19-0000-1000-8000-00805f9b34fb";

PeripheralPlain::PeripheralPlain() {}

PeripheralPlain::~PeripheralPlain() {}

void* PeripheralPlain::underlying() const { return nullptr; }

std::string PeripheralPlain::identifier() { return "Plain Peripheral"; }

BluetoothAddress PeripheralPlain::address() { return "11:22:33:44:55:66"; }

BluetoothAddressType PeripheralPlain::address_type() { return BluetoothAddressType::PUBLIC; };

int16_t PeripheralPlain::rssi() { return -60; }

int16_t PeripheralPlain::tx_power() { return 5; }

uint16_t PeripheralPlain::mtu() {
    if (is_connected()) {
        return 247;
    } else {
        return 0;
    }
}

void PeripheralPlain::connect() {
    connected_ = true;
    paired_ = true;
    SAFE_CALLBACK_CALL(this->callback_on_connected_);
}

void PeripheralPlain::disconnect() {
    connected_ = false;
    SAFE_CALLBACK_CALL(this->callback_on_disconnected_);
}
bool PeripheralPlain::is_connected() { return connected_; }

bool PeripheralPlain::is_connectable() { return true; }

bool PeripheralPlain::is_paired() { return paired_; }

void PeripheralPlain::unpair() { paired_ = false; }

SharedPtrVector<ServiceBase> PeripheralPlain::available_services() {
    if (!connected_) return {};

    SharedPtrVector<ServiceBase> service_list;
    SharedPtrVector<DescriptorBase> descriptor_list;
    SharedPtrVector<CharacteristicBase> characteristic_list = {std::make_shared<CharacteristicBase>(
        BATTERY_CHARACTERISTIC_UUID, descriptor_list, true, false, false, true, false)};

    service_list.push_back(std::make_shared<ServiceBase>(BATTERY_SERVICE_UUID, characteristic_list));
    return service_list;
}

SharedPtrVector<ServiceBase> PeripheralPlain::advertised_services() { return {}; }

std::map<uint16_t, ByteArray> PeripheralPlain::manufacturer_data() { return {{0x004C, "test"}}; }

ByteArray PeripheralPlain::read(BluetoothUUID const& service, BluetoothUUID const& characteristic) { return {}; }

void PeripheralPlain::write_request(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                    ByteArray const& data) {}

void PeripheralPlain::write_command(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                    ByteArray const& data) {}

void PeripheralPlain::notify(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                             std::function<void(ByteArray payload)> callback) {
    if (callback) {
        callback_mutex_.lock();
        callbacks_[{service, characteristic}] = std::move(callback);
        callback_mutex_.unlock();

        task_runner_.dispatch(
            [this, service, characteristic]() -> std::optional<std::chrono::seconds> {
                std::lock_guard<std::mutex> lock(callback_mutex_);
                auto it = this->callbacks_.find({service, characteristic});

                if (it == this->callbacks_.end()) {
                    return std::nullopt;
                }

                it->second("Hello from notify");
                return 1s;
            },
            1s);
    }
}

void PeripheralPlain::indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                               std::function<void(ByteArray payload)> callback) {
    if (callback) {
        callback_mutex_.lock();
        callbacks_[{service, characteristic}] = std::move(callback);
        callback_mutex_.unlock();

        task_runner_.dispatch(
            [this, service, characteristic]() -> std::optional<std::chrono::seconds> {
                std::lock_guard<std::mutex> lock(callback_mutex_);
                auto it = this->callbacks_.find({service, characteristic});

                if (it == this->callbacks_.end()) {
                    return std::nullopt;
                }

                it->second("Hello from notify");
                return 1s;
            },
            1s);
    }
}

void PeripheralPlain::unsubscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    callbacks_.erase({service, characteristic});
}

ByteArray PeripheralPlain::read(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                BluetoothUUID const& descriptor) {
    return {};
}

void PeripheralPlain::write(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                            BluetoothUUID const& descriptor, ByteArray const& data) {}

void PeripheralPlain::set_callback_on_connected(std::function<void()> on_connected) {
    if (on_connected) {
        callback_on_connected_.load(std::move(on_connected));
    } else {
        callback_on_connected_.unload();
    }
}

void PeripheralPlain::set_callback_on_disconnected(std::function<void()> on_disconnected) {
    if (on_disconnected) {
        callback_on_disconnected_.load(std::move(on_disconnected));
    } else {
        callback_on_disconnected_.unload();
    }
}
