#pragma once

#include <simpledbuslegacy/advanced/Interface.h>
#include <simpledbuslegacy/advanced/InterfaceRegistry.h>
#include <string>

namespace SimpleBluezLegacy {

class GattService1 : public SimpleDBusLegacy::Interface {
  public:
    GattService1(std::shared_ptr<SimpleDBusLegacy::Connection> conn, std::string path);
    virtual ~GattService1() = default;

    // ----- METHODS -----

    // ----- PROPERTIES -----
    std::string UUID();

  protected:
    void property_changed(std::string option_name) override;

    std::string _uuid;

  private:
    static const SimpleDBusLegacy::AutoRegisterInterface<GattService1> registry;
};

}  // namespace SimpleBluezLegacy
