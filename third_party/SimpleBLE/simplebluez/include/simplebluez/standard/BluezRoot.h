#pragma once

#include <simplebluez/standard/Adapter.h>
#include <simplebluez/standard/Agent.h>
#include <simpledbus/advanced/Proxy.h>
#include <simpledbus/interfaces/ObjectManager.h>
#include <vector>

namespace SimpleBluez {

class BluezRoot : public SimpleDBus::Proxy {
  public:
    BluezRoot(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~BluezRoot() = default;

    void load_managed_objects();

    std::vector<std::shared_ptr<Adapter>> get_adapters();
    void register_agent(std::shared_ptr<Agent> agent);

    // ----- INTERNAL CALLBACKS -----
    void on_registration() override;

  private:
    std::shared_ptr<SimpleDBus::Proxy> path_create(const std::string& path) override;
    std::shared_ptr<SimpleDBus::Interfaces::ObjectManager> object_manager();
};

}  // namespace SimpleBluez
