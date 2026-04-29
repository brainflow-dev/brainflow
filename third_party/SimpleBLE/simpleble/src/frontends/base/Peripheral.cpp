#include <simpleble/Peripheral.h>

#include <simpleble/Exceptions.h>
#include "BuildVec.h"
#include "PeripheralBase.h"

using namespace SimpleBLE;

bool Peripheral::initialized() const { return internal_ != nullptr; }

PeripheralBase* Peripheral::operator->() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_.get();
}

const PeripheralBase* Peripheral::operator->() const {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_.get();
}

void* Peripheral::underlying() const { return (*this)->underlying(); }

std::string Peripheral::identifier() { return (*this)->identifier(); }

SimpleBLE::BluetoothAddress Peripheral::address() { return (*this)->address(); }

SimpleBLE::BluetoothAddressType Peripheral::address_type() { return (*this)->address_type(); }

int16_t Peripheral::rssi() { return (*this)->rssi(); }

int16_t Peripheral::tx_power() { return (*this)->tx_power(); }

uint16_t Peripheral::mtu() { return (*this)->mtu(); }

void Peripheral::connect() { return (*this)->connect(); }

void Peripheral::disconnect() { return (*this)->disconnect(); }

bool Peripheral::is_connected() { return (*this)->is_connected(); }

bool Peripheral::is_connectable() { return (*this)->is_connectable(); }

bool Peripheral::is_paired() { return (*this)->is_paired(); }

void Peripheral::unpair() { return (*this)->unpair(); }

std::vector<Service> Peripheral::services() {
    return Factory::vector(is_connected() ? internal_->available_services() : internal_->advertised_services());
}

std::map<uint16_t, ByteArray> Peripheral::manufacturer_data() { return (*this)->manufacturer_data(); }

ByteArray Peripheral::read(BluetoothUUID const& service, BluetoothUUID const& characteristic) {
    if (!is_connected()) throw Exception::NotConnected();

    return internal_->read(service, characteristic);
}

void Peripheral::write_request(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                               ByteArray const& data) {
    if (!is_connected()) throw Exception::NotConnected();

    internal_->write_request(service, characteristic, data);
}

void Peripheral::write_command(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                               ByteArray const& data) {
    if (!is_connected()) throw Exception::NotConnected();

    internal_->write_command(service, characteristic, data);
}

void Peripheral::notify(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                        std::function<void(ByteArray payload)> callback) {
    if (!is_connected()) throw Exception::NotConnected();

    internal_->notify(service, characteristic, std::move(callback));
}

void Peripheral::indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                          std::function<void(ByteArray payload)> callback) {
    if (!is_connected()) throw Exception::NotConnected();

    internal_->indicate(service, characteristic, std::move(callback));
}

void Peripheral::unsubscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic) {
    if (!is_connected()) throw Exception::NotConnected();

    internal_->unsubscribe(service, characteristic);
}

ByteArray Peripheral::read(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                           BluetoothUUID const& descriptor) {
    if (!is_connected()) throw Exception::NotConnected();

    return internal_->read(service, characteristic, descriptor);
}

void Peripheral::write(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                       BluetoothUUID const& descriptor, ByteArray const& data) {
    if (!is_connected()) throw Exception::NotConnected();

    internal_->write(service, characteristic, descriptor, data);
}

void Peripheral::set_callback_on_connected(std::function<void()> on_connected) {
    (*this)->set_callback_on_connected(std::move(on_connected));
}

void Peripheral::set_callback_on_disconnected(std::function<void()> on_disconnected) {
    (*this)->set_callback_on_disconnected(std::move(on_disconnected));
}
