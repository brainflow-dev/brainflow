#pragma once

#include <simpledbuslegacy/advanced/Proxy.h>

#include <simplebluezlegacy/Adapter.h>
#include <simplebluezlegacy/Agent.h>

namespace SimpleBluezLegacy {

class BluezOrg : public SimpleDBusLegacy::Proxy {
  public:
    BluezOrg(std::shared_ptr<SimpleDBusLegacy::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~BluezOrg() = default;

    std::vector<std::shared_ptr<Adapter>> get_adapters();
    void register_agent(std::shared_ptr<Agent> agent);

  private:
    std::shared_ptr<SimpleDBusLegacy::Proxy> path_create(const std::string& path) override;
};

}  // namespace SimpleBluezLegacy
