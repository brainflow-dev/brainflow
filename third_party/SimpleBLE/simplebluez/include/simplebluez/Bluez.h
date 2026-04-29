#pragma once

#include <simpledbus/advanced/Proxy.h>

#include <simplebluez/standard/Adapter.h>
#include <simplebluez/standard/Agent.h>
#include <simplebluez/standard/BluezRoot.h>
#include <simplebluez/standard/CustomRoot.h>
#include <vector>

namespace SimpleBluez {

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

    std::shared_ptr<CustomRoot> root_custom();
    std::shared_ptr<BluezRoot> root_bluez();

    std::vector<std::shared_ptr<Adapter>> get_adapters();
    void register_agent(std::shared_ptr<Agent> agent);

  private:
    std::shared_ptr<SimpleDBus::Connection> _conn;
    std::shared_ptr<SimpleBluez::BluezRoot> _bluez_root;
    std::shared_ptr<SimpleBluez::CustomRoot> _custom_root;
};

}  // namespace SimpleBluez
