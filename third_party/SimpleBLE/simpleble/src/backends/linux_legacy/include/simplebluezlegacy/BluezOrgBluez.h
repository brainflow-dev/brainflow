#pragma once

#include <simpledbuslegacy/advanced/Proxy.h>

#include <simplebluezlegacy/Adapter.h>
#include <simplebluezlegacy/Agent.h>

#include <simplebluezlegacy/interfaces/AgentManager1.h>

namespace SimpleBluezLegacy {

class BluezOrgBluez : public SimpleDBusLegacy::Proxy {
  public:
    BluezOrgBluez(std::shared_ptr<SimpleDBusLegacy::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~BluezOrgBluez() = default;

    void register_agent(std::shared_ptr<Agent> agent);

    std::vector<std::shared_ptr<Adapter>> get_adapters();

  private:
    std::shared_ptr<SimpleDBusLegacy::Proxy> path_create(const std::string& path) override;
    std::shared_ptr<AgentManager1> agentmanager1();
};

}  // namespace SimpleBluezLegacy
