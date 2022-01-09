#pragma once

#include <simpledbus/advanced/Proxy.h>

#include <simplebluez/Characteristic.h>
#include <simplebluez/Service.h>
#include <simplebluez/interfaces/Device1.h>

namespace SimpleBluez {

class Device : public SimpleDBus::Proxy {
  public:
    Device(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~Device();

    std::shared_ptr<Service> get_service(const std::string& uuid);
    std::shared_ptr<Characteristic> get_characteristic(const std::string& service_uuid,
                                                       const std::string& characteristic_uuid);

    // ----- PROPERTIES -----
    std::vector<std::shared_ptr<Service>> services();

    std::string address();
    std::string name();
    std::string alias();

    std::map<uint16_t, std::vector<uint8_t>> manufacturer_data();

    bool connected();
    bool services_resolved();

    // ----- METHODS -----
    void connect();
    void disconnect();

    // ----- CALLBACKS -----
    void set_on_services_resolved(std::function<void()> callback);
    void clear_on_services_resolved();
    void set_on_disconnected(std::function<void()> callback);
    void clear_on_disconnected();

  private:
    std::shared_ptr<SimpleDBus::Proxy> path_create(const std::string& path) override;
    std::shared_ptr<SimpleDBus::Interface> interfaces_create(const std::string& interface_name) override;

    std::shared_ptr<Device1> device1();
};

}  // namespace SimpleBluez
