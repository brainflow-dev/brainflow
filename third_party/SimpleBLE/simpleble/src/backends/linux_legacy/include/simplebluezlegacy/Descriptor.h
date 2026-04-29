#pragma once

#include <simpledbuslegacy/advanced/Proxy.h>

#include <simplebluezlegacy/Types.h>
#include <simplebluezlegacy/interfaces/GattDescriptor1.h>

#include <cstdlib>

namespace SimpleBluezLegacy {

class Descriptor : public SimpleDBusLegacy::Proxy {
  public:
    Descriptor(std::shared_ptr<SimpleDBusLegacy::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~Descriptor();

    // ----- METHODS -----
    ByteArray read();
    void write(ByteArray value);

    // ----- PROPERTIES -----
    std::string uuid();
    ByteArray value();

    // ----- CALLBACKS -----
    void set_on_value_changed(std::function<void(ByteArray new_value)> callback);
    void clear_on_value_changed();

  private:
    std::shared_ptr<GattDescriptor1> gattdescriptor1();
};

}  // namespace SimpleBluezLegacy
