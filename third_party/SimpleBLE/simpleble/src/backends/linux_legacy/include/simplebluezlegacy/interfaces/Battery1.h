#pragma once

#include <simpledbuslegacy/advanced/Interface.h>
#include <simpledbuslegacy/advanced/InterfaceRegistry.h>

#include "kvn/kvn_safe_callback.hpp"

#include <string>

namespace SimpleBluezLegacy {

class Battery1 : public SimpleDBusLegacy::Interface {
  public:
    Battery1(std::shared_ptr<SimpleDBusLegacy::Connection> conn, std::string path);
    virtual ~Battery1();

    // ----- METHODS -----

    // ----- PROPERTIES -----
    uint8_t Percentage();

    // ----- CALLBACKS -----
    kvn::safe_callback<void()> OnPercentageChanged;

  protected:
    void property_changed(std::string option_name) override;

  private:
    static const SimpleDBusLegacy::AutoRegisterInterface<Battery1> registry;
};

}  // namespace SimpleBluez
