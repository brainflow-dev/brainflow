#pragma once

#include <simpledbus/advanced/Interface.h>
#include <simpledbus/advanced/InterfaceRegistry.h>

#include <kvn/kvn_safe_callback.hpp>

namespace SimpleDBus::Interfaces {

class ObjectManager : public Interface {
  public:
    ObjectManager(std::shared_ptr<Connection> conn, std::shared_ptr<Proxy> proxy);
    virtual ~ObjectManager();

    Holder GetManagedObjects();

    // ----- SIGNALS -----
    kvn::safe_callback<void(std::string path, Holder options)> InterfacesAdded;
    kvn::safe_callback<void(std::string path, Holder options)> InterfacesRemoved;

    void message_handle(Message& msg) override;

  private:
    static const SimpleDBus::AutoRegisterInterface<ObjectManager> registry;
};

}  // namespace SimpleDBus::Interfaces
