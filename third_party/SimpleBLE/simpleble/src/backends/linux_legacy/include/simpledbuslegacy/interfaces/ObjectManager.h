#pragma once

#include <simpledbuslegacy/advanced/Interface.h>
#include <simpledbuslegacy/advanced/InterfaceRegistry.h>

#include <functional>

namespace SimpleDBusLegacy {

class ObjectManager : public Interface {
  public:
    ObjectManager(std::shared_ptr<Connection> conn, std::string bus_name, std::string path);
    virtual ~ObjectManager() = default;

    // Names are made matching the ones from the DBus specification
    Holder GetManagedObjects(bool use_callbacks = false);
    std::function<void(std::string path, Holder options)> InterfacesAdded;
    std::function<void(std::string path, Holder options)> InterfacesRemoved;

    void message_handle(Message& msg) override;

  private:
    static const SimpleDBusLegacy::AutoRegisterInterface<ObjectManager> registry;
};

}  // namespace SimpleDBusLegacy
