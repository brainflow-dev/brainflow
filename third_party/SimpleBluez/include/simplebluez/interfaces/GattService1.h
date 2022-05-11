#pragma once

#include <simpledbus/advanced/Interface.h>

#include <string>

namespace SimpleBluez {

class GattService1 : public SimpleDBus::Interface {
  public:
    GattService1(std::shared_ptr<SimpleDBus::Connection> conn, std::string path);
    virtual ~GattService1() = default;

    // ----- METHODS -----

    // ----- PROPERTIES -----
    std::string UUID();

  protected:
    void property_changed(std::string option_name) override;

    std::string _uuid;
};

}  // namespace SimpleBluez
