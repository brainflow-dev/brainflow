#include <simplebluez/standard/Descriptor.h>

using namespace SimpleBluez;

Descriptor::Descriptor(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name,
                       const std::string& path)
    : Proxy(conn, bus_name, path) {
}

Descriptor::~Descriptor() {}

void Descriptor::on_registration() {
    // NOTE: We are following the approach of predeclaring all interfaces that might exist here,
    //       so that this same class can fullfill all the necessary functionality for central
    //       and peripheral roles.
    auto descriptor1 = std::make_shared<GattDescriptor1>(_conn, shared_from_this());
    _interfaces.emplace(std::make_pair("org.bluez.GattDescriptor1", descriptor1));

    auto properties = std::make_shared<SimpleDBus::Interfaces::Properties>(_conn, shared_from_this());
    _interfaces.emplace(std::make_pair("org.freedesktop.DBus.Properties", properties));
}

std::shared_ptr<GattDescriptor1> Descriptor::gattdescriptor1() {
    return std::dynamic_pointer_cast<GattDescriptor1>(interface_get("org.bluez.GattDescriptor1"));
}

std::shared_ptr<SimpleDBus::Interfaces::Properties> Descriptor::properties() {
    return std::dynamic_pointer_cast<SimpleDBus::Interfaces::Properties>(interface_get("org.freedesktop.DBus.Properties"));
}

std::string Descriptor::uuid() { return gattdescriptor1()->UUID; }

ByteArray Descriptor::value() { return gattdescriptor1()->Value; }

ByteArray Descriptor::read() { return gattdescriptor1()->ReadValue(); }

void Descriptor::write(ByteArray value) { gattdescriptor1()->WriteValue(value); }

void Descriptor::set_on_value_changed(std::function<void(ByteArray new_value)> callback) {
    gattdescriptor1()->Value.on_changed.load(callback);
}

void Descriptor::clear_on_value_changed() { gattdescriptor1()->Value.on_changed.unload(); }
