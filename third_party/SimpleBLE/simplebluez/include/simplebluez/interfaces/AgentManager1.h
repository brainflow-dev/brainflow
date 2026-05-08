#pragma once

#include <simpledbus/advanced/Interface.h>
#include <simpledbus/advanced/InterfaceRegistry.h>

#include <string>

namespace SimpleBluez {

class AgentManager1 : public SimpleDBus::Interface {
  public:
    AgentManager1(std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy);
    virtual ~AgentManager1();

    // ----- METHODS -----
    void RegisterAgent(std::string agent, std::string capability);
    void RequestDefaultAgent(std::string agent);
    void UnregisterAgent(std::string agent);

  private:
    static const SimpleDBus::AutoRegisterInterface<AgentManager1> registry;
};

}  // namespace SimpleBluez
