#include <simpleble/Peripheral.h>

#include <simpleble/Exceptions.h>
#include "PeripheralBase.h"

using namespace SimpleBLE;

bool Peripheral::initialized() const { return internal_ != nullptr; }

void* Peripheral::underlying() const {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_->underlying();
}

std::string Peripheral::identifier() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_->identifier();
}

SimpleBLE::BluetoothAddress Peripheral::address() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_->address();
}

SimpleBLE::BluetoothAddressType Peripheral::address_type() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_->address_type();
}

int16_t Peripheral::rssi() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_->rssi();
}

int16_t Peripheral::tx_power() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_->tx_power();
}

uint16_t Peripheral::mtu() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_->mtu();
}

void Peripheral::connect() {
    if (!initialized()) throw Exception::NotInitialized();

    internal_->connect();
}

void Peripheral::disconnect() {
    if (!initialized()) throw Exception::NotInitialized();

    internal_->disconnect();
}

bool Peripheral::is_connected() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_->is_connected();
}

bool Peripheral::is_connectable() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_->is_connectable();
}

bool Peripheral::is_paired() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_->is_paired();
}

void Peripheral::unpair() {
    if (!initialized()) throw Exception::NotInitialized();

    internal_->unpair();
}

std::vector<Service> Peripheral::services() {
    if (!initialized()) throw Exception::NotInitialized();

    if (is_connected()) {
        return internal_->services();
    } else {
        return internal_->advertised_services();
    }

    return internal_->services();
}

std::map<uint16_t, ByteArray> Peripheral::manufacturer_data() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_->manufacturer_data();
}

ByteArray Peripheral::read(BluetoothUUID const& service, BluetoothUUID const& characteristic) {
    if (!initialized()) throw Exception::NotInitialized();
    if (!is_connected()) throw Exception::NotConnected();

    return internal_->read(service, characteristic);
}

void Peripheral::write_request(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                               ByteArray const& data) {
    if (!initialized()) throw Exception::NotInitialized();
    if (!is_connected()) throw Exception::NotConnected();

    internal_->write_request(service, characteristic, data);
}

void Peripheral::write_command(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                               ByteArray const& data) {
    if (!initialized()) throw Exception::NotInitialized();
    if (!is_connected()) throw Exception::NotConnected();

    internal_->write_command(service, characteristic, data);
}

void Peripheral::notify(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                        std::function<void(ByteArray payload)> callback) {
    if (!initialized()) throw Exception::NotInitialized();
    if (!is_connected()) throw Exception::NotConnected();

    internal_->notify(service, characteristic, std::move(callback));
}

void Peripheral::indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                          std::function<void(ByteArray payload)> callback) {
    if (!initialized()) throw Exception::NotInitialized();
    if (!is_connected()) throw Exception::NotConnected();

    internal_->indicate(service, characteristic, std::move(callback));
}

void Peripheral::unsubscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic) {
    if (!initialized()) throw Exception::NotInitialized();
    if (!is_connected()) throw Exception::NotConnected();

    internal_->unsubscribe(service, characteristic);
}

ByteArray Peripheral::read(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                           BluetoothUUID const& descriptor) {
    if (!initialized()) throw Exception::NotInitialized();
    if (!is_connected()) throw Exception::NotConnected();

    return internal_->read(service, characteristic, descriptor);
}

void Peripheral::write(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                       BluetoothUUID const& descriptor, ByteArray const& data) {
    if (!initialized()) throw Exception::NotInitialized();
    if (!is_connected()) throw Exception::NotConnected();

    internal_->write(service, characteristic, descriptor, data);
}

void Peripheral::set_callback_on_connected(std::function<void()> on_connected) {
    if (!initialized()) throw Exception::NotInitialized();

    internal_->set_callback_on_connected(std::move(on_connected));
}

void Peripheral::set_callback_on_disconnected(std::function<void()> on_disconnected) {
    if (!initialized()) throw Exception::NotInitialized();

    internal_->set_callback_on_disconnected(std::move(on_disconnected));
}
