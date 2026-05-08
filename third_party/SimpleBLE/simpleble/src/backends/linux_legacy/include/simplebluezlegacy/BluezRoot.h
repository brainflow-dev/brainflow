#pragma once

#include <simpledbuslegacy/advanced/Proxy.h>
#include <simpledbuslegacy/interfaces/ObjectManager.h>

#include <simplebluezlegacy/Adapter.h>
#include <simplebluezlegacy/Agent.h>

#include <vector>

namespace SimpleBluezLegacy {

class BluezRoot : public SimpleDBusLegacy::Proxy {
  public:

    BluezRoot(std::shared_ptr<SimpleDBusLegacy::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~BluezRoot() = default;

    void load_managed_objects();

    std::vector<std::shared_ptr<Adapter>> get_adapters();
    std::shared_ptr<Agent> get_agent();
    void register_agent();

  private:
    std::shared_ptr<SimpleDBusLegacy::Proxy> path_create(const std::string& path) override;
    std::shared_ptr<SimpleDBusLegacy::ObjectManager> object_manager();

    std::shared_ptr<Agent> _agent;
};

}  // namespace SimpleBluez
