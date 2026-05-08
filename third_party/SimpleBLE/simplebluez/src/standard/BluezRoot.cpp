#include <simplebluez/standard/BluezOrg.h>
#include <simplebluez/standard/BluezRoot.h>
#include <simpledbus/interfaces/ObjectManager.h>

using namespace SimpleBluez;

BluezRoot::BluezRoot(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path)
    : Proxy(conn, bus_name, path) {}

void BluezRoot::on_registration() {
    _interfaces.emplace(
        std::make_pair("org.freedesktop.DBus.ObjectManager",
                       std::make_shared<SimpleDBus::Interfaces::ObjectManager>(_conn, shared_from_this())));
}

void BluezRoot::load_managed_objects() {
    SimpleDBus::Holder managed_objects = object_manager()->GetManagedObjects();
    for (auto& [path, managed_interfaces] :
         managed_objects.get<std::map<SimpleDBus::ObjectPath, SimpleDBus::Holder>>()) {
        path_add(path, managed_interfaces);
    }
}

std::vector<std::shared_ptr<Adapter>> BluezRoot::get_adapters() {
    return std::dynamic_pointer_cast<BluezOrg>(path_get("/org"))->get_adapters();
}

void BluezRoot::register_agent(std::shared_ptr<Agent> agent) {
    std::dynamic_pointer_cast<BluezOrg>(path_get("/org"))->register_agent(agent);
}

std::shared_ptr<SimpleDBus::Proxy> BluezRoot::path_create(const std::string& path) {
    auto child = std::make_shared<BluezOrg>(_conn, _bus_name, path);
    return std::static_pointer_cast<SimpleDBus::Proxy>(child);
}

std::shared_ptr<SimpleDBus::Interfaces::ObjectManager> BluezRoot::object_manager() {
    return std::dynamic_pointer_cast<SimpleDBus::Interfaces::ObjectManager>(
        interface_get("org.freedesktop.DBus.ObjectManager"));
}
