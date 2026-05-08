#pragma once

#include <simpledbuslegacy/advanced/Proxy.h>
#include <simpledbuslegacy/interfaces/ObjectManager.h>

#include <simplebluezlegacy/BluezRoot.h>
#include <simplebluezlegacy/Adapter.h>
#include <simplebluezlegacy/Agent.h>

#include <vector>

namespace SimpleBluezLegacy {

class Bluez {
  public:
    Bluez();
    virtual ~Bluez();

    // Delete copy and move operations
    Bluez(const Bluez&) = delete;
    Bluez& operator=(const Bluez&) = delete;
    Bluez(Bluez&&) = delete;
    Bluez& operator=(Bluez&&) = delete;

    void init();
    void run_async();

    std::vector<std::shared_ptr<Adapter>> get_adapters();
    std::shared_ptr<Agent> get_agent();
    void register_agent();

  private:
    std::shared_ptr<SimpleDBusLegacy::Connection> _conn;
    std::shared_ptr<SimpleBluezLegacy::BluezRoot> _bluez_root;
};

}  // namespace SimpleBluezLegacy
