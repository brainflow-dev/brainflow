#pragma once

#include <simpledbus/advanced/Proxy.h>
#include <simplebluez/standard/Descriptor.h>
#include <simplebluez/interfaces/GattCharacteristic1.h>
#include <simplebluez/Types.h>

namespace SimpleBluez {

class Characteristic : public SimpleDBus::Proxy {
  public:
    Characteristic(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~Characteristic();

    std::shared_ptr<Descriptor> descriptor_add(const std::string& name);
    void descriptor_remove(const std::string& name);

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
    void uuid(std::string uuid);

    std::string service();
    void service(const std::string& service);
    
    ByteArray value();
    void value(ByteArray value);
    
    bool notifying();

    std::vector<std::string> flags();
    void flags(std::vector<std::string> flags);

    uint16_t mtu();

    // ----- CALLBACKS -----
    void set_on_value_changed(std::function<void(ByteArray new_value)> callback);
    void clear_on_value_changed();

    void set_on_read_value(std::function<void()> callback);
    void clear_on_read_value();

    void set_on_write_value(std::function<void(ByteArray value)> callback);
    void clear_on_write_value();

    void set_on_notify(std::function<void(bool)> callback);
    void clear_on_notify();

    // ----- INTERNAL CALLBACKS -----
    void on_registration() override;

  private:
    std::shared_ptr<SimpleDBus::Proxy> path_create(const std::string& path) override;

    std::shared_ptr<SimpleDBus::Interfaces::Properties> properties();
    std::shared_ptr<GattCharacteristic1> gattcharacteristic1();
};

}  // namespace SimpleBluez
