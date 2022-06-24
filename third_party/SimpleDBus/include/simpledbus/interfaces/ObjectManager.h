#pragma once

#include <simpledbus/advanced/Interface.h>

#include <functional>

namespace SimpleDBus {

class ObjectManager : public Interface {
  public:
    ObjectManager(std::shared_ptr<Connection> conn, std::string bus_name, std::string path);
    virtual ~ObjectManager() = default;

    // Names are made matching the ones from the DBus specification
    Holder GetManagedObjects(bool use_callbacks = false);
    std::function<void(std::string path, Holder options)> InterfacesAdded;
    std::function<void(std::string path, Holder options)> InterfacesRemoved;

    bool process_received_signal(Message& message);

    void message_handle(Message& msg) override;
};

}  // namespace SimpleDBus
