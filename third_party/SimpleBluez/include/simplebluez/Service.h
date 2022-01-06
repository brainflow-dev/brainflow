#pragma once

#include <simpledbus/advanced/Proxy.h>

#include <simplebluez/Characteristic.h>
#include <simplebluez/interfaces/GattService1.h>

namespace SimpleBluez {

class Service : public SimpleDBus::Proxy {
  public:
    Service(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~Service() = default;

    std::shared_ptr<Characteristic> get_characteristic(const std::string& uuid);

    // ----- PROPERTIES -----
    std::vector<std::shared_ptr<Characteristic>> characteristics();

    std::string uuid();

  private:
    std::shared_ptr<SimpleDBus::Proxy> path_create(const std::string& path) override;
    std::shared_ptr<SimpleDBus::Interface> interfaces_create(const std::string& interface_name) override;

    std::shared_ptr<GattService1> gattservice1();
};

}  // namespace SimpleBluez
