#include <simplebluez/standard/Adapter.h>
#include <simplebluez/standard/BluezOrgBluez.h>

using namespace SimpleBluez;

BluezOrgBluez::BluezOrgBluez(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name,
                             const std::string& path)
    : Proxy(conn, bus_name, path) {}

std::shared_ptr<SimpleDBus::Proxy> BluezOrgBluez::path_create(const std::string& path) {
    return Proxy::create<Adapter>(_conn, _bus_name, path);
}

std::shared_ptr<AgentManager1> BluezOrgBluez::agentmanager1() {
    return std::dynamic_pointer_cast<AgentManager1>(interface_get("org.bluez.AgentManager1"));
}

void BluezOrgBluez::register_agent(std::shared_ptr<Agent> agent) {
    agentmanager1()->RegisterAgent(agent->path(), agent->capabilities());
}

std::vector<std::shared_ptr<Adapter>> BluezOrgBluez::get_adapters() { return children_casted<Adapter>(); }
