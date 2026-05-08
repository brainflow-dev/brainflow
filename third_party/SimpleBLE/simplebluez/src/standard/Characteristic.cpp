#include <simplebluez/Exceptions.h>
#include <simplebluez/standard/Characteristic.h>
#include <simplebluez/standard/Descriptor.h>
#include "simplebluez/Types.h"

using namespace SimpleBluez;

Characteristic::Characteristic(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name,
                               const std::string& path)
    : Proxy(conn, bus_name, path) {}

void Characteristic::on_registration() {
    // NOTE: We are following the approach of predeclaring all interfaces that might exist here,
    //       so that this same class can fullfill all the necessary functionality for central
    //       and peripheral roles.
    auto characteristic1 = std::make_shared<GattCharacteristic1>(_conn, shared_from_this());
    _interfaces.emplace(std::make_pair("org.bluez.GattCharacteristic1", characteristic1));

    auto properties = std::make_shared<SimpleDBus::Interfaces::Properties>(_conn, shared_from_this());
    _interfaces.emplace(std::make_pair("org.freedesktop.DBus.Properties", properties));
}

Characteristic::~Characteristic() {}

std::shared_ptr<SimpleDBus::Proxy> Characteristic::path_create(const std::string& path) {
    return Proxy::create<Descriptor>(_conn, _bus_name, path);
}

std::vector<std::shared_ptr<Descriptor>> Characteristic::descriptors() { return children_casted<Descriptor>(); }

std::shared_ptr<SimpleDBus::Interfaces::Properties> Characteristic::properties() {
    return std::dynamic_pointer_cast<SimpleDBus::Interfaces::Properties>(
        interface_get("org.freedesktop.DBus.Properties"));
}

std::shared_ptr<GattCharacteristic1> Characteristic::gattcharacteristic1() {
    return std::dynamic_pointer_cast<GattCharacteristic1>(interface_get("org.bluez.GattCharacteristic1"));
}

bool Characteristic::notifying() { return gattcharacteristic1()->Notifying.refresh(); }

std::string Characteristic::uuid() { return gattcharacteristic1()->UUID; }
void Characteristic::uuid(std::string uuid) { gattcharacteristic1()->UUID.set(uuid); }

std::string Characteristic::service() { return gattcharacteristic1()->Service; }
void Characteristic::service(const std::string& service) { gattcharacteristic1()->Service.set(service); }

ByteArray Characteristic::value() { return gattcharacteristic1()->Value; }
void Characteristic::value(ByteArray value) { gattcharacteristic1()->Value.set(value).emit(); }

std::vector<std::string> Characteristic::flags() { return gattcharacteristic1()->Flags; }
void Characteristic::flags(std::vector<std::string> flags) { gattcharacteristic1()->Flags(flags); }

uint16_t Characteristic::mtu() { return gattcharacteristic1()->MTU; }

ByteArray Characteristic::read() { return gattcharacteristic1()->ReadValue(); }

void Characteristic::write_request(ByteArray value) {
    gattcharacteristic1()->WriteValue(value, GattCharacteristic1::WriteType::REQUEST);
}

void Characteristic::write_command(ByteArray value) {
    gattcharacteristic1()->WriteValue(value, GattCharacteristic1::WriteType::COMMAND);
}

void Characteristic::start_notify() { gattcharacteristic1()->StartNotify(); }

void Characteristic::stop_notify() { gattcharacteristic1()->StopNotify(); }

std::shared_ptr<Descriptor> Characteristic::descriptor_add(const std::string& name) {
    const std::string descriptor_path = _path + "/descriptor_" + name;
    auto descriptor = Proxy::create<Descriptor>(_conn, _bus_name, descriptor_path);
    path_append_child(descriptor_path, std::static_pointer_cast<SimpleDBus::Proxy>(descriptor));
    return descriptor;
}

void Characteristic::descriptor_remove(const std::string& name) {
    const std::string descriptor_path = _path + "/descriptor_" + name;
    path_remove_child(descriptor_path);
}

std::shared_ptr<Descriptor> Characteristic::get_descriptor(const std::string& uuid) {
    auto descriptors_all = descriptors();

    for (auto& descriptor : descriptors_all) {
        if (descriptor->uuid() == uuid) {
            return descriptor;
        }
    }

    throw Exception::DescriptorNotFoundException(uuid);
}

void Characteristic::set_on_value_changed(std::function<void(ByteArray new_value)> callback) {
    gattcharacteristic1()->Value.on_changed.load(callback);
}

void Characteristic::clear_on_value_changed() { gattcharacteristic1()->Value.on_changed.unload(); }

void Characteristic::set_on_read_value(std::function<void()> callback) {
    gattcharacteristic1()->OnReadValue.load([this, callback]() { callback(); });
}

void Characteristic::clear_on_read_value() { gattcharacteristic1()->OnReadValue.unload(); }

void Characteristic::set_on_write_value(std::function<void(ByteArray value)> callback) {
    gattcharacteristic1()->OnWriteValue.load([this, callback](const ByteArray& value) { callback(value); });
}

void Characteristic::clear_on_write_value() { gattcharacteristic1()->OnWriteValue.unload(); }

void Characteristic::set_on_notify(std::function<void(bool)> callback) {
    gattcharacteristic1()->OnStartNotify.load([this, callback]() { callback(true); });
    gattcharacteristic1()->OnStopNotify.load([this, callback]() { callback(false); });
}

void Characteristic::clear_on_notify() {
    gattcharacteristic1()->OnStartNotify.unload();
    gattcharacteristic1()->OnStopNotify.unload();
}