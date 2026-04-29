#pragma once

#include <simpledbus/advanced/Interface.h>
#include <simpledbus/advanced/InterfaceRegistry.h>

#include <simplebluez/Types.h>

#include <string>

namespace SimpleBluez {

class GattCharacteristic1 : public SimpleDBus::Interface {
  public:
    typedef enum { REQUEST = 0, COMMAND } WriteType;

    GattCharacteristic1(std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy);
    virtual ~GattCharacteristic1();

    // ----- METHODS -----
    void StartNotify();
    void StopNotify();
    void WriteValue(const ByteArray& value, WriteType type);
    ByteArray ReadValue();

    // ----- PROPERTIES -----
    Property<std::string>& UUID = property<std::string>("UUID");
    Property<SimpleDBus::ObjectPath>& Service = property<SimpleDBus::ObjectPath>("Service");
    Property<ByteArray>& Value = property<ByteArray>("Value");
    Property<bool>& Notifying = property<bool>("Notifying");
    Property<std::vector<std::string>>& Flags = property<std::vector<std::string>>("Flags", {"read", "write", "notify"});
    Property<uint16_t>& MTU = property<uint16_t>("MTU");

    // ----- CALLBACKS -----
    kvn::safe_callback<void()> OnValueChanged;
    kvn::safe_callback<void(ByteArray value)> OnWriteValue;
    kvn::safe_callback<void()> OnReadValue;
    kvn::safe_callback<void()> OnStartNotify;
    kvn::safe_callback<void()> OnStopNotify;

    void message_handle(SimpleDBus::Message& msg) override;

  private:
    static const SimpleDBus::AutoRegisterInterface<GattCharacteristic1> registry;
};

}  // namespace SimpleBluez
