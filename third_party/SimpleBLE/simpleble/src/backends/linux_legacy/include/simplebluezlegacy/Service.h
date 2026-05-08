#pragma once

#include <simpledbuslegacy/advanced/Proxy.h>

#include <simplebluezlegacy/Characteristic.h>
#include <simplebluezlegacy/interfaces/GattService1.h>

namespace SimpleBluezLegacy {

class Service : public SimpleDBusLegacy::Proxy {
  public:
    Service(std::shared_ptr<SimpleDBusLegacy::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~Service() = default;

    std::shared_ptr<Characteristic> get_characteristic(const std::string& uuid);

    // ----- PROPERTIES -----
    std::vector<std::shared_ptr<Characteristic>> characteristics();

    std::string uuid();

  private:
    std::shared_ptr<SimpleDBusLegacy::Proxy> path_create(const std::string& path) override;
    std::shared_ptr<GattService1> gattservice1();
};

}  // namespace SimpleBluezLegacy
