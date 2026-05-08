#include <simplebluezlegacy/Characteristic.h>
#include <simplebluezlegacy/Exceptions.h>
#include <simplebluezlegacy/Service.h>

using namespace SimpleBluezLegacy;

Service::Service(std::shared_ptr<SimpleDBusLegacy::Connection> conn, const std::string& bus_name, const std::string& path)
    : Proxy(conn, bus_name, path) {}

std::shared_ptr<SimpleDBusLegacy::Proxy> Service::path_create(const std::string& path) {
    auto child = std::make_shared<Characteristic>(_conn, _bus_name, path);
    return std::static_pointer_cast<SimpleDBusLegacy::Proxy>(child);
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
