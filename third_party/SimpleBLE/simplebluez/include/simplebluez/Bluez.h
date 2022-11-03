#pragma once

#include <simpledbus/advanced/Proxy.h>
#include <simpledbus/interfaces/ObjectManager.h>

#include <simplebluez/Adapter.h>
#include <simplebluez/Agent.h>

#include <vector>

namespace SimpleBluez {

class Bluez : public SimpleDBus::Proxy {
  public:
    Bluez();
    virtual ~Bluez();

    void init();
    void run_async();

    std::vector<std::shared_ptr<Adapter>> get_adapters();
    std::shared_ptr<Agent> get_agent();
    void register_agent();

  private:
    std::shared_ptr<SimpleDBus::Proxy> path_create(const std::string& path) override;

    std::shared_ptr<SimpleDBus::ObjectManager> object_manager();

    std::shared_ptr<Agent> _agent;
};

}  // namespace SimpleBluez
