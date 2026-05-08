#include <simplebluez/standard/ServiceManager.h>
#include <string>

using namespace SimpleBluez;

ServiceManager::ServiceManager(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path)
    : Proxy(conn, bus_name, path) {}

void ServiceManager::on_registration() {
    _interfaces.emplace(
        std::make_pair("org.freedesktop.DBus.ObjectManager",
                       std::make_shared<SimpleDBus::Interfaces::ObjectManager>(_conn, shared_from_this())));
}

std::shared_ptr<SimpleBluez::Service> ServiceManager::service_add(const std::string& name) {
    const std::string service_path = _path + "/service_" + name;
    auto service = Proxy::create<Service>(_conn, _bus_name, service_path);
    path_append_child(service_path, std::static_pointer_cast<SimpleDBus::Proxy>(service));
    return service;
}

void ServiceManager::service_remove(const std::string& path) {
    const std::string service_path = _path + "/" + path;
    path_remove_child(service_path);
}

std::shared_ptr<SimpleDBus::Interfaces::ObjectManager> ServiceManager::object_manager() {
    return std::dynamic_pointer_cast<SimpleDBus::Interfaces::ObjectManager>(
        interface_get("org.freedesktop.DBus.ObjectManager"));
}