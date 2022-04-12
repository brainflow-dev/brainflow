#include <simplebluez/Characteristic.h>
#include <simplebluez/Exceptions.h>
#include <simplebluez/Service.h>

using namespace SimpleBluez;

Service::Service(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path)
    : Proxy(conn, bus_name, path) {}

std::shared_ptr<SimpleDBus::Proxy> Service::path_create(const std::string& path) {
    auto child = std::make_shared<Characteristic>(_conn, _bus_name, path);
    return std::static_pointer_cast<SimpleDBus::Proxy>(child);
}

std::shared_ptr<SimpleDBus::Interface> Service::interfaces_create(const std::string& interface_name) {
    if (interface_name == "org.bluez.GattService1") {
        return std::static_pointer_cast<SimpleDBus::Interface>(std::make_shared<GattService1>(_conn, _path));
    }

    auto interface = std::make_shared<SimpleDBus::Interface>(_conn, _bus_name, _path, interface_name);
    return std::static_pointer_cast<SimpleDBus::Interface>(interface);
}

std::shared_ptr<GattService1> Service::gattservice1() {
    return std::dynamic_pointer_cast<GattService1>(interface_get("org.bluez.GattService1"));
}

std::vector<std::shared_ptr<Characteristic>> Service::characteristics() { return children_casted<Characteristic>(); }

std::shared_ptr<Characteristic> Service::get_characteristic(const std::string& uuid) {
    auto characteristics_all = characteristics();

    for (auto& characteristic : characteristics_all) {
        if (characteristic->uuid() == uuid) {
            return characteristic;
        }
    }

    throw Exception::CharacteristicNotFoundException(uuid);
}

std::string Service::uuid() { return gattservice1()->UUID(); }
