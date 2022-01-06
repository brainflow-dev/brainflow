#pragma once

#include <simpledbus/advanced/Callback.h>
#include <simpledbus/advanced/Interface.h>

#include <simplebluez/Types.h>

#include <string>

namespace SimpleBluez {

class GattCharacteristic1 : public SimpleDBus::Interface {
  public:
    typedef enum { REQUEST = 0, COMMAND } WriteType;

    GattCharacteristic1(std::shared_ptr<SimpleDBus::Connection> conn, std::string path);
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

    // ----- CALLBACKS -----
    SimpleDBus::Callback<std::function<void()>> OnValueChanged;

  protected:
    void property_changed(std::string option_name) override;
    void update_value(SimpleDBus::Holder& new_value);

    std::string _uuid;
    ByteArray _value;
};

}  // namespace SimpleBluez
