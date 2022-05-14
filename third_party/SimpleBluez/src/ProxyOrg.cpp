#include <simplebluez/ProxyOrg.h>
#include <simplebluez/ProxyOrgBluez.h>

using namespace SimpleBluez;

ProxyOrg::ProxyOrg(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path)
    : Proxy(conn, bus_name, path) {}

std::vector<std::shared_ptr<Adapter>> ProxyOrg::get_adapters() {
    return std::dynamic_pointer_cast<ProxyOrgBluez>(path_get("/org/bluez"))->get_adapters();
}

void ProxyOrg::register_agent(std::shared_ptr<Agent> agent) {
    std::dynamic_pointer_cast<ProxyOrgBluez>(path_get("/org/bluez"))->register_agent(agent);
}

std::shared_ptr<SimpleDBus::Proxy> ProxyOrg::path_create(const std::string& path) {
    auto child = std::make_shared<ProxyOrgBluez>(_conn, _bus_name, path);
    return std::static_pointer_cast<SimpleDBus::Proxy>(child);
}

std::shared_ptr<SimpleDBus::Interface> ProxyOrg::interfaces_create(const std::string& interface_name) {
    auto interface = std::make_shared<SimpleDBus::Interface>(_conn, _bus_name, _path, interface_name);
    return std::static_pointer_cast<SimpleDBus::Interface>(interface);
}
