#include <simplebluez/Adapter.h>
#include <simplebluez/ProxyOrgBluez.h>

using namespace SimpleBluez;

ProxyOrgBluez::ProxyOrgBluez(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name,
                             const std::string& path)
    : Proxy(conn, bus_name, path) {}

std::shared_ptr<SimpleDBus::Proxy> ProxyOrgBluez::path_create(const std::string& path) {
    auto child = std::make_shared<Adapter>(_conn, _bus_name, path);
    return std::static_pointer_cast<SimpleDBus::Proxy>(child);
}

std::shared_ptr<SimpleDBus::Interface> ProxyOrgBluez::interfaces_create(const std::string& interface_name) {
    if (interface_name == "org.bluez.AgentManager1") {
        return std::static_pointer_cast<SimpleDBus::Interface>(std::make_shared<AgentManager1>(_conn, _path));
    }

    auto interface = std::make_shared<SimpleDBus::Interface>(_conn, _bus_name, _path, interface_name);
    return std::static_pointer_cast<SimpleDBus::Interface>(interface);
}

std::shared_ptr<AgentManager1> ProxyOrgBluez::agentmanager1() {
    return std::dynamic_pointer_cast<AgentManager1>(interface_get("org.bluez.AgentManager1"));
}

void ProxyOrgBluez::register_agent(std::shared_ptr<Agent> agent) {
    agentmanager1()->RegisterAgent(agent->path(), agent->capabilities());
}

std::vector<std::shared_ptr<Adapter>> ProxyOrgBluez::get_adapters() { return children_casted<Adapter>(); }
