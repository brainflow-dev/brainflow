#pragma once

#include <simplebluez/standard/Characteristic.h>
#include <simplebluez/interfaces/GattService1.h>
#include <simpledbus/interfaces/Properties.h>
#include <simpledbus/advanced/Proxy.h>

namespace SimpleBluez {

class Service : public SimpleDBus::Proxy {
  public:
    Service(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~Service() = default;

    std::shared_ptr<Characteristic> characteristic_add(const std::string& name);
    void characteristic_remove(const std::string& name);

    std::shared_ptr<Characteristic> get_characteristic(const std::string& uuid);

    // ----- PROPERTIES -----
    std::vector<std::shared_ptr<Characteristic>> characteristics();

    std::string uuid();
    void uuid(const std::string& uuid);

    bool primary();
    void primary(bool primary);

    // ----- INTERNAL CALLBACKS -----
    void on_registration() override;

  private:
    std::shared_ptr<SimpleDBus::Proxy> path_create(const std::string& path) override;
    std::shared_ptr<SimpleDBus::Interfaces::Properties> properties();
    std::shared_ptr<GattService1> gattservice1();
};

}  // namespace SimpleBluez
