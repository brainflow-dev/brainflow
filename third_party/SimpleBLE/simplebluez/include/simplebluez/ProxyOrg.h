#pragma once

#include <simpledbus/advanced/Proxy.h>

#include <simplebluez/Adapter.h>
#include <simplebluez/Agent.h>

namespace SimpleBluez {

class ProxyOrg : public SimpleDBus::Proxy {
  public:
    ProxyOrg(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~ProxyOrg() = default;

    std::vector<std::shared_ptr<Adapter>> get_adapters();
    void register_agent(std::shared_ptr<Agent> agent);

  private:
    std::shared_ptr<SimpleDBus::Proxy> path_create(const std::string& path) override;
    std::shared_ptr<SimpleDBus::Interface> interfaces_create(const std::string& interface_name) override;
};

}  // namespace SimpleBluez
