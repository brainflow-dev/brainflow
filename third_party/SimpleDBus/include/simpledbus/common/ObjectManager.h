#pragma once
#include <functional>
#include <string>

#include <simpledbus/base/Connection.h>
#include <simpledbus/base/Holder.h>
#include <simpledbus/base/Message.h>

namespace SimpleDBus {

class Holder;
class Connection;

class ObjectManager {
  private:
    const std::string _interface;

    std::string _path;
    std::string _service;
    Connection* _conn;

  public:
    ObjectManager(Connection* conn, std::string service, std::string path);
    ~ObjectManager();

    // Names are made matching the ones from the DBus specification
    Holder GetManagedObjects(bool use_callbacks = false);
    std::function<void(std::string path, Holder options)> InterfacesAdded;
    std::function<void(std::string path, Holder options)> InterfacesRemoved;

    bool process_received_signal(Message& message);
};

}  // namespace SimpleDBus
