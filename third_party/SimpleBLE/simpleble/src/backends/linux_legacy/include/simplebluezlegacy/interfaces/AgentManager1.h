#pragma once

#include <simpledbuslegacy/advanced/Interface.h>
#include <simpledbuslegacy/advanced/InterfaceRegistry.h>

#include <string>

namespace SimpleBluezLegacy {

class AgentManager1 : public SimpleDBusLegacy::Interface {
  public:
    AgentManager1(std::shared_ptr<SimpleDBusLegacy::Connection> conn, std::string path);
    virtual ~AgentManager1() = default;

    // ----- METHODS -----
    void RegisterAgent(std::string agent, std::string capability);
    void RequestDefaultAgent(std::string agent);
    void UnregisterAgent(std::string agent);

  private:
    static const SimpleDBusLegacy::AutoRegisterInterface<AgentManager1> registry;
};

}  // namespace SimpleBluezLegacy
