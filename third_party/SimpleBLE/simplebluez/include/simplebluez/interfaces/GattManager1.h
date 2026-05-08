#pragma once
#include <simpledbus/advanced/Interface.h>
#include <simpledbus/advanced/InterfaceRegistry.h>
#include <string>

namespace SimpleBluez {

class GattManager1 : public SimpleDBus::Interface {

  public:

    // ----- TYPES -----

    // ----- CONSTRUCTORS -----

    GattManager1(std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy);
    virtual ~GattManager1();

    // ----- METHODS -----

    void RegisterApplication(std::string application_path);
    void UnregisterApplication(std::string application_path);

    // ----- PROPERTIES -----

  private:
    static const SimpleDBus::AutoRegisterInterface<GattManager1> registry;
  
};

}  // namespace SimpleBluez