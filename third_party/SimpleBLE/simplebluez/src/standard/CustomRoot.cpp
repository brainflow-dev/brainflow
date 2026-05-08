#include <simplebluez/standard/CustomRoot.h>

using namespace SimpleBluez;

CustomRoot::CustomRoot(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name,
                       const std::string& path)
    : Proxy(conn, bus_name, path) {}

void CustomRoot::on_registration() {
    _interfaces.emplace(
        std::make_pair("org.freedesktop.DBus.ObjectManager",
                       std::make_shared<SimpleDBus::Interfaces::ObjectManager>(_conn, shared_from_this())));
}

std::shared_ptr<Agent> CustomRoot::agent_add(const std::string& name) {
    const std::string agent_path = "/agent_" + name;
    auto agent = Proxy::create<Agent>(_conn, _bus_name, agent_path);
    path_append_child(agent_path, std::static_pointer_cast<SimpleDBus::Proxy>(agent));
    // TODO: Have the object manager send the InterfacesAdded signal.
    return agent;
}

std::shared_ptr<Agent> CustomRoot::agent_get(const std::string& name) {
    const std::string agent_path = "/agent_" + name;
    return std::dynamic_pointer_cast<Agent>(path_get(agent_path));
}

void CustomRoot::agent_remove(const std::string& name) {
    const std::string agent_path = "/agent_" + name;
    // TODO: Have the object manager send the InterfacesRemoved signal.
    path_remove_child(agent_path);
}

std::shared_ptr<Advertisement> CustomRoot::advertisement_add(const std::string& name) {
    const std::string advertisement_path = "/advertisement_" + name;
    auto advertisement = Proxy::create<Advertisement>(_conn, _bus_name, advertisement_path);
    path_append_child(advertisement_path, std::static_pointer_cast<SimpleDBus::Proxy>(advertisement));
    // TODO: Have the object manager send the InterfacesAdded signal.
    return advertisement;
}

std::shared_ptr<Advertisement> CustomRoot::advertisement_get(const std::string& name) {
    const std::string advertisement_path = "/advertisement_" + name;
    return std::dynamic_pointer_cast<Advertisement>(path_get(advertisement_path));
}

void CustomRoot::advertisement_remove(const std::string& name) {
    const std::string advertisement_path = "/advertisement_" + name;
    // TODO: Have the object manager send the InterfacesRemoved signal.
    path_remove_child(advertisement_path);
}

std::shared_ptr<ServiceManager> CustomRoot::service_mgr_add(const std::string& name) {
    const std::string service_mgr_path = "/application_" + name;
    auto service_mgr = Proxy::create<ServiceManager>(_conn, _bus_name, service_mgr_path);
    path_append_child(service_mgr_path, std::static_pointer_cast<SimpleDBus::Proxy>(service_mgr));
    // TODO: Have the object manager send the InterfacesAdded signal.
    return service_mgr;
}

std::shared_ptr<ServiceManager> CustomRoot::service_mgr_get(const std::string& name) {
    const std::string service_mgr_path = "/application_" + name;
    return std::dynamic_pointer_cast<ServiceManager>(path_get(service_mgr_path));
}

void CustomRoot::service_mgr_remove(const std::string& name) {
    const std::string service_mgr_path = "/application_" + name;
    // TODO: Have the object manager send the InterfacesRemoved signal.
    path_remove_child(service_mgr_path);
}

std::shared_ptr<SimpleDBus::Interfaces::ObjectManager> CustomRoot::object_manager() {
    return std::dynamic_pointer_cast<SimpleDBus::Interfaces::ObjectManager>(
        interface_get("org.freedesktop.DBus.ObjectManager"));
}
