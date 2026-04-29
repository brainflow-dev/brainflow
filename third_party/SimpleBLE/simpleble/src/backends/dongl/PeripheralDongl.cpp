#include "PeripheralDongl.h"

#include "CharacteristicBase.h"
#include "DescriptorBase.h"
#include "ServiceBase.h"

#include <simpleble/Exceptions.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <thread>

#include "CommonUtils.h"
#include "LoggingInternal.h"
#include "fmt/chrono.h"
#include "protocol/simpleble.pb.h"
#include "simpleble/Types.h"

#include <fmt/core.h>

using namespace SimpleBLE;
using namespace std::chrono_literals;

PeripheralDongl::PeripheralDongl(std::shared_ptr<Dongl::Serial::Protocol> serial_protocol,
                                 advertising_data_t advertising_data) {
    _serial_protocol = serial_protocol;
    _address_type = advertising_data.address_type;
    _identifier = advertising_data.identifier;
    _address = advertising_data.mac_address;
    _connectable = advertising_data.connectable;
    update_advertising_data(advertising_data);
}

PeripheralDongl::~PeripheralDongl() {}

void* PeripheralDongl::underlying() const { return nullptr; }

std::string PeripheralDongl::identifier() { return _identifier; }

BluetoothAddress PeripheralDongl::address() { return _address; }

BluetoothAddressType PeripheralDongl::address_type() { return _address_type; }

int16_t PeripheralDongl::rssi() { return _rssi; }

int16_t PeripheralDongl::tx_power() { return _tx_power; }

uint16_t PeripheralDongl::mtu() { return 0; }

void PeripheralDongl::connect() {
    if (is_connected()) {
        return;
    }

    bool connection_successful = false;
    for (int i = 0; i < 10; i++) {
        fmt::print("PeripheralDongl::connect: attempt {}\n", i);
        connection_successful = _attempt_connect();
        fmt::print("PeripheralDongl::connect: attempt {} - success: {}\n", i, connection_successful);
        if (connection_successful) {
            break;
        } else {
            std::this_thread::sleep_for(750ms);
        }
    }

    if (!connection_successful) {
        throw Exception::OperationFailed(fmt::format("Connection failed to be established"));
    }

    SAFE_CALLBACK_CALL(this->_callback_on_connected);
    fmt::print("PeripheralDongl::connect: connected\n");
}

void PeripheralDongl::disconnect() {
    if (!is_connected()) {
        return;
    }

    auto response = _serial_protocol->simpleble_disconnect(_conn_handle);
    if (response.ret_code != 0) {
        throw Exception::OperationFailed(fmt::format("Failed to disconnect: {}", response.ret_code));
    }

    // Wait for the disconnection to be confirmed.
    std::unique_lock<std::mutex> lock(disconnection_mutex_);
    disconnection_cv_.wait_for(lock, 500ms, [this]() { return !is_connected(); });

    if (is_connected()) {
        _conn_handle = BLE_CONN_HANDLE_INVALID;
        throw Exception::OperationFailed(fmt::format("Timeout while waiting for disconnection confirmation"));
    }
    SAFE_CALLBACK_CALL(this->_callback_on_disconnected);
}

bool PeripheralDongl::is_connected() { return _conn_handle != BLE_CONN_HANDLE_INVALID; }

bool PeripheralDongl::is_connectable() { return _connectable; }

bool PeripheralDongl::is_paired() { return false; }

void PeripheralDongl::unpair() {}

SharedPtrVector<ServiceBase> PeripheralDongl::available_services() {
    SharedPtrVector<ServiceBase> service_list;
    for (auto& service : _services) {
        SharedPtrVector<CharacteristicBase> characteristic_list;
        for (auto& characteristic : service.characteristics) {
            SharedPtrVector<DescriptorBase> descriptor_list;
            for (auto& descriptor : characteristic.descriptors) {
                descriptor_list.push_back(std::make_shared<DescriptorBase>(descriptor.uuid));
            }
            characteristic_list.push_back(std::make_shared<CharacteristicBase>(
                characteristic.uuid, descriptor_list, characteristic.can_read, characteristic.can_write_request,
                characteristic.can_write_command, characteristic.can_notify, characteristic.can_indicate));
        }
        service_list.push_back(std::make_shared<ServiceBase>(service.uuid, characteristic_list));
    }

    return service_list;
}

SharedPtrVector<ServiceBase> PeripheralDongl::advertised_services() {
    SharedPtrVector<ServiceBase> service_list;
    for (auto& [service_uuid, data] : _service_data) {
        service_list.push_back(std::make_shared<ServiceBase>(service_uuid, data));
    }

    return service_list;
}

std::map<uint16_t, ByteArray> PeripheralDongl::manufacturer_data() { return _manufacturer_data; }

ByteArray PeripheralDongl::read(BluetoothUUID const& service_uuid, BluetoothUUID const& characteristic_uuid) {
    auto& characteristic = _find_characteristic_from_uuid(service_uuid, characteristic_uuid);

    if (!characteristic.can_read) {
        throw Exception::OperationFailed(fmt::format("Characteristic {} is not readable", characteristic_uuid));
    }

    simpleble_ReadRsp rsp = _serial_protocol->simpleble_read(_conn_handle, characteristic.handle_value);
    if (rsp.ret_code != 0) {
        throw Exception::OperationFailed(
            fmt::format("Failed to read characteristic {} - ret_code: {}", characteristic_uuid, rsp.ret_code));
    }

    return ByteArray(rsp.data.bytes, rsp.data.size);
}

void PeripheralDongl::write_request(BluetoothUUID const& service_uuid, BluetoothUUID const& characteristic_uuid,
                                    ByteArray const& data) {
    auto& characteristic = _find_characteristic_from_uuid(service_uuid, characteristic_uuid);

    if (!characteristic.can_write_request) {
        throw Exception::OperationFailed(fmt::format("Characteristic {} is not writable", characteristic_uuid));
    }

    simpleble_WriteRsp rsp = _serial_protocol->simpleble_write(_conn_handle, characteristic.handle_value,
                                                               simpleble_WriteOperation_WRITE_REQ, data);
    if (rsp.ret_code != 0) {
        throw Exception::OperationFailed(
            fmt::format("Failed to write characteristic {} - ret_code: {}", characteristic_uuid, rsp.ret_code));
    }
}

void PeripheralDongl::write_command(BluetoothUUID const& service_uuid, BluetoothUUID const& characteristic_uuid,
                                    ByteArray const& data) {
    auto& characteristic = _find_characteristic_from_uuid(service_uuid, characteristic_uuid);

    if (!characteristic.can_write_command) {
        throw Exception::OperationFailed(fmt::format("Characteristic {} is not writable", characteristic_uuid));
    }

    simpleble_WriteRsp rsp = _serial_protocol->simpleble_write(_conn_handle, characteristic.handle_value,
                                                               simpleble_WriteOperation_WRITE_CMD, data);
    if (rsp.ret_code != 0) {
        throw Exception::OperationFailed(
            fmt::format("Failed to write characteristic {} - ret_code: {}", characteristic_uuid, rsp.ret_code));
    }
}

void PeripheralDongl::notify(BluetoothUUID const& service_uuid, BluetoothUUID const& characteristic_uuid,
                             std::function<void(ByteArray payload)> callback) {
    auto& characteristic = _find_characteristic_from_uuid(service_uuid, characteristic_uuid);

    if (!characteristic.can_notify) {
        throw Exception::OperationFailed(fmt::format("Characteristic {} is not notifyable", characteristic_uuid));
    }

    if (characteristic.handle_cccd == 0) {
        throw Exception::OperationFailed(fmt::format("Characteristic {} does not have a CCCD", characteristic_uuid));
    }

    _callbacks_on_value_changed[characteristic.handle_value] = std::move(callback);

    ByteArray data = {0x01, 0x00};
    simpleble_WriteRsp rsp = _serial_protocol->simpleble_write(_conn_handle, characteristic.handle_cccd,
                                                               simpleble_WriteOperation_WRITE_REQ, data);
    if (rsp.ret_code != 0) {
        throw Exception::OperationFailed(
            fmt::format("Failed to write characteristic {} - ret_code: {}", characteristic_uuid, rsp.ret_code));
    }
}

void PeripheralDongl::indicate(BluetoothUUID const& service_uuid, BluetoothUUID const& characteristic_uuid,
                               std::function<void(ByteArray payload)> callback) {
    auto& characteristic = _find_characteristic_from_uuid(service_uuid, characteristic_uuid);

    if (!characteristic.can_indicate) {
        throw Exception::OperationFailed(fmt::format("Characteristic {} is not indicateable", characteristic_uuid));
    }

    if (characteristic.handle_cccd == 0) {
        throw Exception::OperationFailed(fmt::format("Characteristic {} does not have a CCCD", characteristic_uuid));
    }

    _callbacks_on_value_changed[characteristic.handle_value] = callback;

    ByteArray data = {0x02, 0x00};
    simpleble_WriteRsp rsp = _serial_protocol->simpleble_write(_conn_handle, characteristic.handle_cccd,
                                                               simpleble_WriteOperation_WRITE_REQ, data);
    if (rsp.ret_code != 0) {
        throw Exception::OperationFailed(
            fmt::format("Failed to write characteristic {} - ret_code: {}", characteristic_uuid, rsp.ret_code));
    }
}

void PeripheralDongl::unsubscribe(BluetoothUUID const& service_uuid, BluetoothUUID const& characteristic_uuid) {
    auto& characteristic = _find_characteristic_from_uuid(service_uuid, characteristic_uuid);

    if (characteristic.handle_cccd == 0) {
        throw Exception::OperationFailed(fmt::format("Characteristic {} does not have a CCCD", characteristic_uuid));
    }

    _callbacks_on_value_changed.erase(characteristic.handle_value);

    ByteArray data = {0x00, 0x00};
    simpleble_WriteRsp rsp = _serial_protocol->simpleble_write(_conn_handle, characteristic.handle_cccd,
                                                               simpleble_WriteOperation_WRITE_REQ, data);
    if (rsp.ret_code != 0) {
        throw Exception::OperationFailed(
            fmt::format("Failed to write characteristic {} - ret_code: {}", characteristic_uuid, rsp.ret_code));
    }
}

ByteArray PeripheralDongl::read(BluetoothUUID const& service_uuid, BluetoothUUID const& characteristic_uuid,
                                BluetoothUUID const& descriptor_uuid) {
    return {};
}

void PeripheralDongl::write(BluetoothUUID const& service_uuid, BluetoothUUID const& characteristic_uuid,
                            BluetoothUUID const& descriptor_uuid, ByteArray const& data) {}

void PeripheralDongl::set_callback_on_connected(std::function<void()> on_connected) {
    if (on_connected) {
        _callback_on_connected.load(std::move(on_connected));
    } else {
        _callback_on_connected.unload();
    }
}

void PeripheralDongl::set_callback_on_disconnected(std::function<void()> on_disconnected) {
    if (on_disconnected) {
        _callback_on_disconnected.load(std::move(on_disconnected));
    } else {
        _callback_on_disconnected.unload();
    }
}

uint16_t PeripheralDongl::conn_handle() const { return _conn_handle; }

void PeripheralDongl::update_advertising_data(advertising_data_t advertising_data) {
    if (advertising_data.identifier != "") {
        _identifier = advertising_data.identifier;
    }
    _rssi = advertising_data.rssi;
    _tx_power = advertising_data.tx_power;
    _manufacturer_data = advertising_data.manufacturer_data;

    advertising_data.service_data.merge(_service_data);
    _service_data = advertising_data.service_data;
}

bool PeripheralDongl::_attempt_connect() {
    if (_conn_handle != BLE_CONN_HANDLE_INVALID) {
        auto response = _serial_protocol->simpleble_disconnect(_conn_handle);
        if (response.ret_code != 0) {
            SIMPLEBLE_LOG_ERROR(fmt::format("Failed to disconnect during connect attempt: {}", response.ret_code));
        }

        // Wait for the disconnection to be confirmed.
        std::unique_lock<std::mutex> lock(disconnection_mutex_);
        disconnection_cv_.wait_for(lock, 500ms, [this]() { return !is_connected(); });
    }

    _conn_handle = BLE_CONN_HANDLE_INVALID;

    auto response = _serial_protocol->simpleble_connect(static_cast<simpleble_BluetoothAddressType>(_address_type),
                                                        _address);
    if (response.ret_code != 0) {
        throw Exception::OperationFailed(fmt::format("Error when attempting to connect: {}", response.ret_code));
    }

    // NOTE: Bluetooth connections are non-acknowledged by the peripheral. The connected event that we get
    //       is just the confirmation that the connection packet was sent, not that it was received.
    //       Our only option is to wait a bit after the connection event is received to see if maybe a
    //       disconnection event arises too, in which case the connection failed.

    // Wait for the connection to be confirmed.
    {
        std::unique_lock<std::mutex> lock(connection_mutex_);
        connection_cv_.wait_for(lock, 5000ms, [this]() { return _conn_handle != BLE_CONN_HANDLE_INVALID; });
        if (_conn_handle == BLE_CONN_HANDLE_INVALID) {
            SIMPLEBLE_LOG_ERROR("Timeout while waiting for connection confirmation");
            return false;
        }
    }

    // Wait for the attributes to be discovered.
    {
        std::unique_lock<std::mutex> lock(attributes_discovered_mutex_);
        attributes_discovered_cv_.wait_for(
            lock, 15000ms, [this]() { return !_services.empty() || _conn_handle == BLE_CONN_HANDLE_INVALID; });
        if (_services.empty()) {
            SIMPLEBLE_LOG_ERROR("Timeout while waiting for attributes to be discovered");
            return false;
        }

        if (_conn_handle == BLE_CONN_HANDLE_INVALID) {
            SIMPLEBLE_LOG_ERROR("Connection lost during attribute discovery");
            return false;
        }
    }

    // Retrieve any missing 128-bit UUIDs.
    for (auto& service : _services) {
        // Fetch the service UUID if missing.
        if (service.uuid.empty()) {
            simpleble_ReadRsp rsp = _serial_protocol->simpleble_read(_conn_handle, service.start_handle);
            if (rsp.ret_code != 0) {
                SIMPLEBLE_LOG_ERROR(fmt::format("Failed to read UUID for service {} - ret_code: {}", service.start_handle, rsp.ret_code));
                continue;
            }

            if (rsp.data.size == 2) {
                service.uuid = _uuid_from_uuid16(rsp.data.bytes[1] << 8 | rsp.data.bytes[0]);
            } else if (rsp.data.size == 16) {
                uint8_t uuid_128[16];
                for (int i = 0; i < 16; i++) {
                    uuid_128[i] = rsp.data.bytes[15 - i];
                }
                service.uuid = _uuid_from_uuid128(uuid_128);
            } else {
                SIMPLEBLE_LOG_ERROR(fmt::format("Unexpected UUID size: {}", rsp.data.size));
                continue;
            }
        }

        for (auto& characteristic : service.characteristics) {
            // Fetch the characteristic UUID if missing.
            if (characteristic.uuid.empty()) {
                simpleble_ReadRsp rsp = _serial_protocol->simpleble_read(_conn_handle, characteristic.handle_decl);
                if (rsp.ret_code != 0) {
                    SIMPLEBLE_LOG_ERROR(fmt::format("Failed to read UUID for characteristic {} - ret_code: {}", characteristic.handle_decl,
                                 rsp.ret_code));
                    continue;
                }

                if (rsp.data.size == 5) {
                    characteristic.uuid = _uuid_from_uuid16(rsp.data.bytes[4] << 8 | rsp.data.bytes[3]);
                } else if (rsp.data.size == 19) {
                    uint8_t uuid_128[16];
                    for (int i = 0; i < 16; i++) {
                        uuid_128[i] = rsp.data.bytes[15 - i + 3];
                    }
                    characteristic.uuid = _uuid_from_uuid128(uuid_128);
                }
            }
        }
    }
    return true;
}

void PeripheralDongl::notify_connected(uint16_t conn_handle) {
    _conn_handle = conn_handle;
    connection_cv_.notify_all();
}

void PeripheralDongl::notify_disconnected() {
    _conn_handle = BLE_CONN_HANDLE_INVALID;
    disconnection_cv_.notify_all();
    attributes_discovered_cv_.notify_all();

    // TODO: Only throw the callback if the disconection was unexpected.
    // SAFE_CALLBACK_CALL(this->_callback_on_disconnected);
}

void PeripheralDongl::notify_service_discovered(simpleble_ServiceDiscoveredEvt const& evt) {
    BluetoothUUID uuid;
    if (evt.has_uuid16) {
        uuid = _uuid_from_uuid16(evt.uuid16.uuid);
    }

    _services.emplace_back(ServiceDefinition{
        uuid,
        evt.start_handle,
        evt.end_handle,
    });
}

void PeripheralDongl::notify_characteristic_discovered(simpleble_CharacteristicDiscoveredEvt const& evt) {
    auto& service = _find_service_from_handle(evt.handle_decl);

    BluetoothUUID uuid;
    if (evt.has_uuid16) {
        uuid = _uuid_from_uuid16(evt.uuid16.uuid);
    }

    service.characteristics.emplace_back(CharacteristicDefinition{
        uuid,
        evt.handle_decl,
        evt.handle_value,
        0,
        evt.props.read,
        evt.props.write,
        evt.props.write_wo_resp,
        evt.props.notify,
        evt.props.indicate,
    });
}

void PeripheralDongl::notify_descriptor_discovered(simpleble_DescriptorDiscoveredEvt const& evt) {
    auto& service = _find_service_from_handle(evt.handle);

    for (auto& characteristic : service.characteristics) {
        // If the descriptor matches the characteristic declaration handle or value handle, we can ignore it.
        if (characteristic.handle_decl == evt.handle || characteristic.handle_value == evt.handle) {
            return;
        }
    }

    // At this point we know we have a real descriptor that we shouldn't ignore.

    auto& characteristic = _find_characteristic_from_handle(evt.handle);
    characteristic.descriptors.emplace_back(DescriptorDefinition{
        _uuid_from_uuid16(evt.uuid16.uuid),
        evt.handle,
    });

    // If the descriptor is a client characteristic configuration descriptor (CCCD),
    // save that handle number for the characteristic.
    if (evt.uuid16.uuid == 0x2902) {
        characteristic.handle_cccd = evt.handle;
    }
}

void PeripheralDongl::notify_attribute_discovery_complete() {
    attributes_discovered_cv_.notify_all();
}

void PeripheralDongl::notify_value_changed(simpleble_ValueChangedEvt const& evt) {
    ByteArray data(evt.data.bytes, evt.data.bytes + evt.data.size);
    std::function<void(ByteArray)> callback = _callbacks_on_value_changed[evt.handle];
    if (callback) {
        callback(data);
    }
}

BluetoothUUID PeripheralDongl::_uuid_from_uuid16(uint16_t uuid16) {
    return BluetoothUUID(fmt::format("0000{:04X}-0000-1000-8000-00805F9B34FB", uuid16));
}

BluetoothUUID PeripheralDongl::_uuid_from_uuid32(uint32_t uuid32) {
    return BluetoothUUID(fmt::format("{:08X}-0000-1000-8000-00805F9B34FB", uuid32));
}

BluetoothUUID PeripheralDongl::_uuid_from_uuid128(const uint8_t id[16]) {
    std::string uuid_str;
    uuid_str.reserve(36);  // Pre-allocate memory for the UUID string

    uuid_str += fmt::format("{:02X}{:02X}{:02X}{:02X}", id[0], id[1], id[2], id[3]);
    uuid_str += "-";
    uuid_str += fmt::format("{:02X}{:02X}", id[4], id[5]);
    uuid_str += "-";
    uuid_str += fmt::format("{:02X}{:02X}", id[6], id[7]);
    uuid_str += "-";
    uuid_str += fmt::format("{:02X}{:02X}", id[8], id[9]);
    uuid_str += "-";
    uuid_str += fmt::format("{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}", id[10], id[11], id[12], id[13], id[14], id[15]);

    return BluetoothUUID(uuid_str);
}

BluetoothUUID PeripheralDongl::_uuid_from_proto(simpleble_UUID const& uuid) {
    switch (uuid.which_uuid) {
        case simpleble_UUID_uuid16_tag:
            return BluetoothUUID(fmt::format("0000{:04X}-0000-1000-8000-00805F9B34FB", uuid.uuid.uuid16.uuid));
        case simpleble_UUID_uuid32_tag:
            return BluetoothUUID(fmt::format("{:08X}-0000-1000-8000-00805F9B34FB", uuid.uuid.uuid32.uuid));
        case simpleble_UUID_uuid128_tag: {
            const auto& bytes = uuid.uuid.uuid128.uuid;
            std::string uuid_str;
            uuid_str.reserve(36);  // Pre-allocate memory for the UUID string

            uuid_str += fmt::format("{:02X}{:02X}{:02X}{:02X}", bytes[0], bytes[1], bytes[2], bytes[3]);
            uuid_str += "-";
            uuid_str += fmt::format("{:02X}{:02X}", bytes[4], bytes[5]);
            uuid_str += "-";
            uuid_str += fmt::format("{:02X}{:02X}", bytes[6], bytes[7]);
            uuid_str += "-";
            uuid_str += fmt::format("{:02X}{:02X}", bytes[8], bytes[9]);
            uuid_str += "-";
            uuid_str += fmt::format("{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}", bytes[10], bytes[11], bytes[12], bytes[13],
                                    bytes[14], bytes[15]);

            return BluetoothUUID(uuid_str);
        }
    }

    // Should not be reached
    throw std::runtime_error(fmt::format("Unknown UUID type: {}", uuid.which_uuid));
}

PeripheralDongl::ServiceDefinition& PeripheralDongl::_find_service_from_handle(uint16_t handle) {
    for (auto& service : _services) {
        if (service.start_handle <= handle && service.end_handle >= handle) {
            return service;
        }
    }

    throw std::runtime_error(fmt::format("Service not found for handle {}", handle));
}

PeripheralDongl::CharacteristicDefinition& PeripheralDongl::_find_characteristic_from_handle(uint16_t handle) {
    for (auto& service : _services) {
        if (service.start_handle <= handle && service.end_handle >= handle) {
            // For the given service handle, loop the characteristics backwards and select the first characteristic
            // where the handle_value is less than the descriptor handle.
            for (auto it = service.characteristics.rbegin(); it != service.characteristics.rend(); ++it) {
                if (it->handle_value < handle) {
                    return *it;
                }
            }
        }
    }

    throw std::runtime_error(fmt::format("Characteristic not found for handle {}", handle));
}

PeripheralDongl::CharacteristicDefinition& PeripheralDongl::_find_characteristic_from_uuid(
    BluetoothUUID const& service_uuid, BluetoothUUID const& characteristic_uuid) {
    for (auto& service : _services) {
        if (service.uuid == service_uuid) {
            for (auto& characteristic : service.characteristics) {
                if (characteristic.uuid == characteristic_uuid) {
                    return characteristic;
                }
            }
        }
    }

    throw std::runtime_error(fmt::format("Characteristic {} not found", characteristic_uuid));
}