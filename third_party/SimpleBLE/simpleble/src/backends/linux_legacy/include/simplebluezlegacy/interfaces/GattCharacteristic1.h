#pragma once

#include <simpledbuslegacy/advanced/Interface.h>
#include <simpledbuslegacy/advanced/InterfaceRegistry.h>

#include "kvn/kvn_safe_callback.hpp"

#include <simplebluezlegacy/Types.h>

#include <string>

namespace SimpleBluezLegacy {

class GattCharacteristic1 : public SimpleDBusLegacy::Interface {
  public:
    typedef enum { REQUEST = 0, COMMAND } WriteType;

    GattCharacteristic1(std::shared_ptr<SimpleDBusLegacy::Connection> conn, std::string path);
    virtual ~GattCharacteristic1();

    // ----- METHODS -----
    void StartNotify();
    void StopNotify();
    void WriteValue(const ByteArray& value, WriteType type);
    ByteArray ReadValue();

    // ----- PROPERTIES -----
    std::string UUID();
    ByteArray Value();
    bool Notifying(bool refresh = true);
    std::vector<std::string> Flags();
    uint16_t MTU();

    // ----- CALLBACKS -----
    kvn::safe_callback<void()> OnValueChanged;

  protected:
    void property_changed(std::string option_name) override;
    void update_value(SimpleDBusLegacy::Holder& new_value);

    std::string _uuid;
    ByteArray _value;

  private:
    static const SimpleDBusLegacy::AutoRegisterInterface<GattCharacteristic1> registry;
};

}  // namespace SimpleBluez
