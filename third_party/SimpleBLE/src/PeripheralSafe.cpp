#include <simpleble/Exceptions.h>
#include <simpleble/PeripheralSafe.h>

// Note: This file is extremely verbose with the usage of namespace to
// ensure that the functions are not ambiguous.

SimpleBLE::Safe::Peripheral::Peripheral(SimpleBLE::Peripheral& peripheral) : SimpleBLE::Peripheral(peripheral) {}

std::optional<std::string> SimpleBLE::Safe::Peripheral::identifier() noexcept {
    try {
        return SimpleBLE::Peripheral::identifier();
    } catch (const SimpleBLE::Exception::BaseException&) {
        return std::nullopt;
    }
}

std::optional<SimpleBLE::BluetoothAddress> SimpleBLE::Safe::Peripheral::address() noexcept {
    try {
        return SimpleBLE::Peripheral::address();
    } catch (const SimpleBLE::Exception::BaseException&) {
        return std::nullopt;
    }
}

std::optional<int16_t> SimpleBLE::Safe::Peripheral::rssi() noexcept {
    try {
        return SimpleBLE::Peripheral::rssi();
    } catch (const SimpleBLE::Exception::BaseException&) {
        return std::nullopt;
    }
}

bool SimpleBLE::Safe::Peripheral::connect() noexcept {
    try {
        SimpleBLE::Peripheral::connect();
        return true;
    } catch (const SimpleBLE::Exception::BaseException&) {
        return false;
    }
}

bool SimpleBLE::Safe::Peripheral::disconnect() noexcept {
    try {
        SimpleBLE::Peripheral::disconnect();
        return true;
    } catch (const SimpleBLE::Exception::BaseException&) {
        return false;
    }
}

std::optional<bool> SimpleBLE::Safe::Peripheral::is_connected() noexcept {
    try {
        return SimpleBLE::Peripheral::is_connected();
    } catch (const SimpleBLE::Exception::BaseException&) {
        return std::nullopt;
    }
}

std::optional<bool> SimpleBLE::Safe::Peripheral::is_connectable() noexcept {
    try {
        return SimpleBLE::Peripheral::is_connectable();
    } catch (const SimpleBLE::Exception::BaseException&) {
        return std::nullopt;
    }
}

std::optional<bool> SimpleBLE::Safe::Peripheral::is_paired() noexcept {
    try {
        return SimpleBLE::Peripheral::is_paired();
    } catch (const SimpleBLE::Exception::BaseException&) {
        return std::nullopt;
    }
}

bool SimpleBLE::Safe::Peripheral::unpair() noexcept {
    try {
        SimpleBLE::Peripheral::unpair();
        return true;
    } catch (const SimpleBLE::Exception::BaseException&) {
        return false;
    }
}

std::optional<std::map<uint16_t, SimpleBLE::ByteArray>> SimpleBLE::Safe::Peripheral::manufacturer_data() noexcept {
    try {
        return SimpleBLE::Peripheral::manufacturer_data();
    } catch (const SimpleBLE::Exception::BaseException&) {
        return std::nullopt;
    }
}

std::optional<std::vector<SimpleBLE::BluetoothService>> SimpleBLE::Safe::Peripheral::services() noexcept {
    try {
        return SimpleBLE::Peripheral::services();
    } catch (const SimpleBLE::Exception::BaseException&) {
        return std::nullopt;
    }
}

std::optional<SimpleBLE::ByteArray> SimpleBLE::Safe::Peripheral::read(BluetoothUUID const& service,
                                                                      BluetoothUUID const& characteristic) noexcept {
    try {
        return SimpleBLE::Peripheral::read(service, characteristic);
    } catch (const SimpleBLE::Exception::BaseException&) {
        return std::nullopt;
    }
}

bool SimpleBLE::Safe::Peripheral::write_request(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                                ByteArray const& data) noexcept {
    try {
        SimpleBLE::Peripheral::write_request(service, characteristic, data);
        return true;
    } catch (const SimpleBLE::Exception::BaseException&) {
        return false;
    }
}

bool SimpleBLE::Safe::Peripheral::write_command(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                                ByteArray const& data) noexcept {
    try {
        SimpleBLE::Peripheral::write_command(service, characteristic, data);
        return true;
    } catch (const SimpleBLE::Exception::BaseException&) {
        return false;
    }
}

bool SimpleBLE::Safe::Peripheral::notify(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                         std::function<void(ByteArray payload)> callback) noexcept {
    try {
        SimpleBLE::Peripheral::notify(service, characteristic, std::move(callback));
        return true;
    } catch (const SimpleBLE::Exception::BaseException&) {
        return false;
    }
}

bool SimpleBLE::Safe::Peripheral::indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                           std::function<void(ByteArray payload)> callback) noexcept {
    try {
        SimpleBLE::Peripheral::indicate(service, characteristic, std::move(callback));
        return true;
    } catch (const SimpleBLE::Exception::BaseException&) {
        return false;
    }
}

bool SimpleBLE::Safe::Peripheral::unsubscribe(BluetoothUUID const& service,
                                              BluetoothUUID const& characteristic) noexcept {
    try {
        SimpleBLE::Peripheral::unsubscribe(service, characteristic);
        return true;
    } catch (const SimpleBLE::Exception::BaseException&) {
        return false;
    }
}

bool SimpleBLE::Safe::Peripheral::set_callback_on_connected(std::function<void()> on_connected) noexcept {
    try {
        SimpleBLE::Peripheral::set_callback_on_connected(std::move(on_connected));
        return true;
    } catch (const SimpleBLE::Exception::BaseException&) {
        return false;
    }
}

bool SimpleBLE::Safe::Peripheral::set_callback_on_disconnected(std::function<void()> on_disconnected) noexcept {
    try {
        SimpleBLE::Peripheral::set_callback_on_disconnected(std::move(on_disconnected));
        return true;
    } catch (const SimpleBLE::Exception::BaseException&) {
        return false;
    }
}
