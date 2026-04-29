#pragma once

#include <simpledbus/advanced/Interface.h>
#include <simpledbus/advanced/InterfaceRegistry.h>

#include <simplebluez/Types.h>

#include <string>

namespace SimpleBluez {

class GattDescriptor1 : public SimpleDBus::Interface {
  public:
    GattDescriptor1(std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy);
    virtual ~GattDescriptor1();

    // ----- METHODS -----
    void WriteValue(const ByteArray& value);
    ByteArray ReadValue();

    // ----- PROPERTIES -----
    Property<std::string>& UUID = property<std::string>("UUID");
    Property<ByteArray>& Value = property<ByteArray>("Value");

  private:
    static const SimpleDBus::AutoRegisterInterface<GattDescriptor1> registry;
};

}  // namespace SimpleBluez
