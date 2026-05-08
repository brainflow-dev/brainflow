#pragma once

#include <simpledbus/advanced/Proxy.h>
#include <simpledbus/interfaces/ObjectManager.h>
#include <simplebluez/standard/Service.h>

#include <vector>

namespace SimpleBluez {

class ServiceManager : public SimpleDBus::Proxy {
  public:

    ServiceManager(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~ServiceManager() = default;

    std::shared_ptr<SimpleBluez::Service> service_add(const std::string& name);
    void service_remove(const std::string& path);

    void on_registration() override;

  private:
    std::shared_ptr<SimpleDBus::Interfaces::ObjectManager> object_manager();
};

}  // namespace SimpleBluez