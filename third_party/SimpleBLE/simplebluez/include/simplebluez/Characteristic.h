#pragma once

#include <simpledbus/advanced/Proxy.h>

#include <simplebluez/Descriptor.h>
#include <simplebluez/Types.h>
#include <simplebluez/interfaces/GattCharacteristic1.h>

#include <cstdlib>

namespace SimpleBluez {

class Characteristic : public SimpleDBus::Proxy {
  public:
    Characteristic(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~Characteristic();

    std::shared_ptr<Descriptor> get_descriptor(const std::string& uuid);

    // ----- METHODS -----
    ByteArray read();
    void write_request(ByteArray value);
    void write_command(ByteArray value);
    void start_notify();
    void stop_notify();

    // ----- PROPERTIES -----
    std::vector<std::shared_ptr<Descriptor>> descriptors();

    std::string uuid();
    ByteArray value();
    bool notifying();
    std::vector<std::string> flags();
    uint16_t mtu();

    // ----- CALLBACKS -----
    void set_on_value_changed(std::function<void(ByteArray new_value)> callback);
    void clear_on_value_changed();

  private:
    std::shared_ptr<SimpleDBus::Proxy> path_create(const std::string& path) override;
    std::shared_ptr<SimpleDBus::Interface> interfaces_create(const std::string& interface_name) override;

    std::shared_ptr<GattCharacteristic1> gattcharacteristic1();
};

}  // namespace SimpleBluez
