#pragma once

#include <simpledbus/advanced/Proxy.h>

#include <simplebluez/Characteristic.h>
#include <simplebluez/Service.h>
#include <simplebluez/interfaces/Battery1.h>
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
    std::vector<std::string> uuids();

    std::string address();
    std::string address_type();
    std::string name();
    std::string alias();
    int16_t rssi();
    int16_t tx_power();

    std::map<uint16_t, ByteArray> manufacturer_data();
    std::map<std::string, ByteArray> service_data();

    bool paired();
    bool connected();
    bool services_resolved();

    // ----- METHODS -----
    void connect();
    void disconnect();
    void pair();
    void cancel_pairing();

    // ----- CALLBACKS -----
    void set_on_services_resolved(std::function<void()> callback);
    void clear_on_services_resolved();
    void set_on_disconnected(std::function<void()> callback);
    void clear_on_disconnected();

    // ----- BATTERY INTERFACE -----
    bool has_battery_interface();
    uint8_t battery_percentage();
    void set_on_battery_percentage_changed(std::function<void(uint8_t new_value)> callback);
    void clear_on_battery_percentage_changed();

  private:
    std::shared_ptr<SimpleDBus::Proxy> path_create(const std::string& path) override;
    std::shared_ptr<SimpleDBus::Interface> interfaces_create(const std::string& interface_name) override;

    std::shared_ptr<Device1> device1();
    std::shared_ptr<Battery1> battery1();
};

}  // namespace SimpleBluez
