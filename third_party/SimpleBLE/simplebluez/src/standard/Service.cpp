#include <simplebluez/Exceptions.h>
#include <simplebluez/standard/Characteristic.h>
#include <simplebluez/standard/Service.h>

using namespace SimpleBluez;

Service::Service(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path)
    : Proxy(conn, bus_name, path) {}

void Service::on_registration() {
    // NOTE: We are following the approach of predeclaring all interfaces that might exist here,
    //       so that this same class can fullfill all the necessary functionality for central
    //       and peripheral roles.
    auto service1 = std::make_shared<GattService1>(_conn, shared_from_this());
    _interfaces.emplace(std::make_pair("org.bluez.GattService1", service1));

    auto properties = std::make_shared<SimpleDBus::Interfaces::Properties>(_conn, shared_from_this());
    _interfaces.emplace(std::make_pair("org.freedesktop.DBus.Properties", properties));
}

std::shared_ptr<SimpleDBus::Proxy> Service::path_create(const std::string& path) {
    return Proxy::create<Characteristic>(_conn, _bus_name, path);
}

std::shared_ptr<SimpleDBus::Interfaces::Properties> Service::properties() {
    return std::dynamic_pointer_cast<SimpleDBus::Interfaces::Properties>(
        interface_get("org.freedesktop.DBus.Properties"));
}

std::shared_ptr<GattService1> Service::gattservice1() {
    return std::dynamic_pointer_cast<GattService1>(interface_get("org.bluez.GattService1"));
}

std::vector<std::shared_ptr<Characteristic>> Service::characteristics() { return children_casted<Characteristic>(); }

std::shared_ptr<Characteristic> Service::characteristic_add(const std::string& name) {
    const std::string characteristic_path = _path + "/characteristic_" + name;
    auto characteristic = Proxy::create<Characteristic>(_conn, _bus_name, characteristic_path);
    path_append_child(characteristic_path, std::static_pointer_cast<SimpleDBus::Proxy>(characteristic));

    auto service_characteristic_list = gattservice1()->Characteristics.get();
    service_characteristic_list.push_back(characteristic_path);
    gattservice1()->Characteristics.set(service_characteristic_list);

    characteristic->service(_path);

    return characteristic;
}

void Service::characteristic_remove(const std::string& name) {
    const std::string characteristic_path = _path + "/characteristic_" + name;

    auto service_characteristic_list = std::vector<std::string>();
    for (auto& characteristic : gattservice1()->Characteristics.get()) {
        if (characteristic != characteristic_path) {
            service_characteristic_list.push_back(characteristic);
        }
    }
    gattservice1()->Characteristics.set(service_characteristic_list);
    path_remove_child(characteristic_path);
}

std::shared_ptr<Characteristic> Service::get_characteristic(const std::string& uuid) {
    auto characteristics_all = characteristics();

    for (auto& characteristic : characteristics_all) {
        if (characteristic->uuid() == uuid) {
            return characteristic;
        }
    }

    throw Exception::CharacteristicNotFoundException(uuid);
}

std::string Service::uuid() { return gattservice1()->UUID; }

void Service::uuid(const std::string& uuid) { gattservice1()->UUID.set(uuid); }

bool Service::primary() { return gattservice1()->Primary; }

void Service::primary(bool primary) { gattservice1()->Primary.set(primary); }
