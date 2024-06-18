#include <simplebluez/Characteristic.h>
#include <simplebluez/Descriptor.h>
#include <simplebluez/Exceptions.h>

using namespace SimpleBluez;

Characteristic::Characteristic(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name,
                               const std::string& path)
    : Proxy(conn, bus_name, path) {}

Characteristic::~Characteristic() {}

std::shared_ptr<SimpleDBus::Proxy> Characteristic::path_create(const std::string& path) {
    auto child = std::make_shared<Descriptor>(_conn, _bus_name, path);
    return std::static_pointer_cast<SimpleDBus::Proxy>(child);
}

std::shared_ptr<SimpleDBus::Interface> Characteristic::interfaces_create(const std::string& interface_name) {
    if (interface_name == "org.bluez.GattCharacteristic1") {
        return std::static_pointer_cast<SimpleDBus::Interface>(std::make_shared<GattCharacteristic1>(_conn, _path));
    }

    auto interface = std::make_shared<SimpleDBus::Interface>(_conn, _bus_name, _path, interface_name);
    return std::static_pointer_cast<SimpleDBus::Interface>(interface);
}

std::vector<std::shared_ptr<Descriptor>> Characteristic::descriptors() { return children_casted<Descriptor>(); }

std::shared_ptr<GattCharacteristic1> Characteristic::gattcharacteristic1() {
    return std::dynamic_pointer_cast<GattCharacteristic1>(interface_get("org.bluez.GattCharacteristic1"));
}

bool Characteristic::notifying() { return gattcharacteristic1()->Notifying(); }

std::string Characteristic::uuid() { return gattcharacteristic1()->UUID(); }

ByteArray Characteristic::value() { return gattcharacteristic1()->Value(); }

std::vector<std::string> Characteristic::flags() { return gattcharacteristic1()->Flags(); }

uint16_t Characteristic::mtu() { return gattcharacteristic1()->MTU(); }

ByteArray Characteristic::read() { return gattcharacteristic1()->ReadValue(); }

void Characteristic::write_request(ByteArray value) {
    gattcharacteristic1()->WriteValue(value, GattCharacteristic1::WriteType::REQUEST);
}

void Characteristic::write_command(ByteArray value) {
    gattcharacteristic1()->WriteValue(value, GattCharacteristic1::WriteType::COMMAND);
}

void Characteristic::start_notify() { gattcharacteristic1()->StartNotify(); }

void Characteristic::stop_notify() { gattcharacteristic1()->StopNotify(); }

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
    gattcharacteristic1()->OnValueChanged.load([this, callback]() { callback(gattcharacteristic1()->Value()); });
}

void Characteristic::clear_on_value_changed() { gattcharacteristic1()->OnValueChanged.unload(); }
