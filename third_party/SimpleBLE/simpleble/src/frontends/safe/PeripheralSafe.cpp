#include <simpleble/Exceptions.h>
#include <simpleble/PeripheralSafe.h>

using namespace SimpleBLE::Safe;

using SPeripheral = SimpleBLE::Safe::Peripheral;
using UPeripheral = SimpleBLE::Peripheral;

SPeripheral::Peripheral(UPeripheral& peripheral) : internal_(peripheral) {}
SPeripheral::Peripheral(UPeripheral&& peripheral) : internal_(std::move(peripheral)) {}

std::optional<std::string> SPeripheral::identifier() noexcept {
    try {
        return internal_.identifier();
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<SimpleBLE::BluetoothAddress> SPeripheral::address() noexcept {
    try {
        return internal_.address();
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<SimpleBLE::BluetoothAddressType> SPeripheral::address_type() noexcept {
    try {
        return internal_.address_type();
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<int16_t> SPeripheral::rssi() noexcept {
    try {
        return internal_.rssi();
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<int16_t> SPeripheral::tx_power() noexcept {
    try {
        return internal_.tx_power();
    } catch (...) {
        return std::nullopt;
    }
}
std::optional<uint16_t> SPeripheral::mtu() noexcept {
    try {
        return internal_.mtu();
    } catch (...) {
        return std::nullopt;
    }
}

bool SPeripheral::connect() noexcept {
    try {
        internal_.connect();
        return true;
    } catch (...) {
        return false;
    }
}

bool SPeripheral::disconnect() noexcept {
    try {
        internal_.disconnect();
        return true;
    } catch (...) {
        return false;
    }
}

std::optional<bool> SPeripheral::is_connected() noexcept {
    try {
        return internal_.is_connected();
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<bool> SPeripheral::is_connectable() noexcept {
    try {
        return internal_.is_connectable();
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<bool> SPeripheral::is_paired() noexcept {
    try {
        return internal_.is_paired();
    } catch (...) {
        return std::nullopt;
    }
}

bool SPeripheral::unpair() noexcept {
    try {
        internal_.unpair();
        return true;
    } catch (...) {
        return false;
    }
}

std::optional<std::map<uint16_t, SimpleBLE::ByteArray>> SPeripheral::manufacturer_data() noexcept {
    try {
        return internal_.manufacturer_data();
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<std::vector<SimpleBLE::Service>> SPeripheral::services() noexcept {
    try {
        return internal_.services();
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<SimpleBLE::ByteArray> SPeripheral::read(BluetoothUUID const& service,
                                                      BluetoothUUID const& characteristic) noexcept {
    try {
        return internal_.read(service, characteristic);
    } catch (...) {
        return std::nullopt;
    }
}

bool SPeripheral::write_request(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                ByteArray const& data) noexcept {
    try {
        internal_.write_request(service, characteristic, data);
        return true;
    } catch (...) {
        return false;
    }
}

bool SPeripheral::write_command(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                ByteArray const& data) noexcept {
    try {
        internal_.write_command(service, characteristic, data);
        return true;
    } catch (...) {
        return false;
    }
}

bool SPeripheral::notify(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                         std::function<void(ByteArray payload)> callback) noexcept {
    try {
        internal_.notify(service, characteristic, std::move(callback));
        return true;
    } catch (...) {
        return false;
    }
}

bool SPeripheral::indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                           std::function<void(ByteArray payload)> callback) noexcept {
    try {
        internal_.indicate(service, characteristic, std::move(callback));
        return true;
    } catch (...) {
        return false;
    }
}

bool SPeripheral::unsubscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic) noexcept {
    try {
        internal_.unsubscribe(service, characteristic);
        return true;
    } catch (...) {
        return false;
    }
}

std::optional<SimpleBLE::ByteArray> SPeripheral::read(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                                      BluetoothUUID const& descriptor) noexcept {
    try {
        return internal_.read(service, characteristic, descriptor);
    } catch (...) {
        return std::nullopt;
    }
}

bool SPeripheral::write(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                        BluetoothUUID const& descriptor, ByteArray const& data) noexcept {
    try {
        internal_.write(service, characteristic, descriptor, data);
        return true;
    } catch (...) {
        return false;
    }
}

bool SPeripheral::set_callback_on_connected(std::function<void()> on_connected) noexcept {
    try {
        internal_.set_callback_on_connected(std::move(on_connected));
        return true;
    } catch (...) {
        return false;
    }
}

bool SPeripheral::set_callback_on_disconnected(std::function<void()> on_disconnected) noexcept {
    try {
        internal_.set_callback_on_disconnected(std::move(on_disconnected));
        return true;
    } catch (...) {
        return false;
    }
}
