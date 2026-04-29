#pragma once

#include <simpledbuslegacy/advanced/Interface.h>
#include <simpledbuslegacy/advanced/InterfaceRegistry.h>

#include "kvn/kvn_safe_callback.hpp"

#include <simplebluezlegacy/Types.h>

#include <string>

namespace SimpleBluezLegacy {

class GattDescriptor1 : public SimpleDBusLegacy::Interface {
  public:
    GattDescriptor1(std::shared_ptr<SimpleDBusLegacy::Connection> conn, std::string path);
    virtual ~GattDescriptor1();

    // ----- METHODS -----
    void WriteValue(const ByteArray& value);
    ByteArray ReadValue();

    // ----- PROPERTIES -----
    std::string UUID();
    ByteArray Value();

    // ----- CALLBACKS -----
    kvn::safe_callback<void()> OnValueChanged;

  protected:
    void property_changed(std::string option_name) override;
    void update_value(SimpleDBusLegacy::Holder& new_value);

    std::string _uuid;
    ByteArray _value;

  private:
    static const SimpleDBusLegacy::AutoRegisterInterface<GattDescriptor1> registry;
};

}  // namespace SimpleBluez
