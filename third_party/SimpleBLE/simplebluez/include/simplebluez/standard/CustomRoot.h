#pragma once

#include <simplebluez/standard/Agent.h>
#include <simpledbus/advanced/Proxy.h>
#include <simpledbus/interfaces/ObjectManager.h>
#include "simplebluez/standard/Advertisement.h"
#include "simplebluez/standard/ServiceManager.h"

namespace SimpleBluez {

class CustomRoot : public SimpleDBus::Proxy {
  public:
    CustomRoot(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~CustomRoot() = default;

    std::shared_ptr<Agent> agent_add(const std::string& name);
    std::shared_ptr<Agent> agent_get(const std::string& name);
    void agent_remove(const std::string& name);

    std::shared_ptr<Advertisement> advertisement_add(const std::string& name);
    std::shared_ptr<Advertisement> advertisement_get(const std::string& name);
    void advertisement_remove(const std::string& name);

    std::shared_ptr<ServiceManager> service_mgr_add(const std::string& name);
    std::shared_ptr<ServiceManager> service_mgr_get(const std::string& name);
    void service_mgr_remove(const std::string& name);

    // ----- INTERNAL CALLBACKS -----
    void on_registration() override;

  private:
    std::shared_ptr<SimpleDBus::Interfaces::ObjectManager> object_manager();
};

}  // namespace SimpleBluez
