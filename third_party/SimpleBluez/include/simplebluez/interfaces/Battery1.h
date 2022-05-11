#pragma once

#include <simpledbus/advanced/Callback.h>
#include <simpledbus/advanced/Interface.h>

#include <string>

namespace SimpleBluez {

class Battery1 : public SimpleDBus::Interface {
  public:
    Battery1(std::shared_ptr<SimpleDBus::Connection> conn, std::string path);
    virtual ~Battery1();

    // ----- METHODS -----

    // ----- PROPERTIES -----
    uint8_t Percentage();

    // ----- CALLBACKS -----
    SimpleDBus::Callback<std::function<void()>> OnPercentageChanged;

  protected:
    void property_changed(std::string option_name) override;
};

}  // namespace SimpleBluez
