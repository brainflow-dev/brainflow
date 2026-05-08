#pragma once

#include <simpledbus/advanced/Interface.h>
#include <simpledbus/advanced/InterfaceRegistry.h>
#include <string>

namespace SimpleBluez {

class GattService1 : public SimpleDBus::Interface {
  public:
    GattService1(std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy);
    virtual ~GattService1();

    // ----- METHODS -----

    // ----- PROPERTIES -----
    Property<std::string>& UUID = property<std::string>("UUID");
    Property<bool>& Primary = property<bool>("Primary");
    Property<std::vector<std::string>>& Characteristics = property<std::vector<std::string>>("Characteristics");

  private:
    static const SimpleDBus::AutoRegisterInterface<GattService1> registry;
};

}  // namespace SimpleBluez
