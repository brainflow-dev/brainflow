#pragma once

#include <simpledbus/advanced/Interface.h>
#include <simpledbus/advanced/InterfaceRegistry.h>

#include "kvn/kvn_safe_callback.hpp"

#include <string>

namespace SimpleBluez {

class Battery1 : public SimpleDBus::Interface {
  public:
    Battery1(std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy);
    virtual ~Battery1();

    // ----- METHODS -----

    // ----- PROPERTIES -----
    Property<uint8_t>& Percentage = property<uint8_t>("Percentage");

  private:
    static const SimpleDBus::AutoRegisterInterface<Battery1> registry;
};

}  // namespace SimpleBluez
